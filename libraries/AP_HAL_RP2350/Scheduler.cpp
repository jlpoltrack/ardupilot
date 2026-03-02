/*
 * FreeRTOS-based scheduler for RP2350
 *
 * date: 2026-02-28
 */

#include <AP_HAL/AP_HAL.h>
#include <AP_Math/AP_Math.h>

#include "Scheduler.h"

#include <FreeRTOS.h>
#include <task.h>
#include <pico/time.h>
#include <hardware/watchdog.h>

#include <stdio.h>

using namespace RP2350;

extern const AP_HAL::HAL& hal;

bool Scheduler::_initialized = false;

// core affinity for dual-core SMP
#define FASTCPU 0
#define SLOWCPU 1

Scheduler::Scheduler()
    : _num_timer_procs(0),
      _num_io_procs(0),
      _in_timer_proc(false),
      _in_io_proc(false)
{
}

void Scheduler::init()
{
    // helper to create a task pinned to a specific core
    auto create_pinned = [](TaskFunction_t fn, const char *name,
                            uint32_t stack, void *param,
                            UBaseType_t prio, TaskHandle_t *handle,
                            UBaseType_t core) {
        xTaskCreate(fn, name, stack, param, prio, handle);
        // set core affinity mask (bit 0 = core 0, bit 1 = core 1)
        vTaskCoreAffinitySet(*handle, 1 << core);
    };

    // pin time-critical tasks to core 0, deferred tasks to core 1
    create_pinned(_main_thread, "APM_MAIN", MAIN_SS, this,
                  MAIN_PRIO, &_main_task_handle, FASTCPU);

    create_pinned(_timer_thread, "APM_TIMER", TIMER_SS, this,
                  TIMER_PRIO, &_timer_task_handle, FASTCPU);

    create_pinned(_uart_thread, "APM_UART", UART_SS, this,
                  UART_PRIO, &_uart_task_handle, FASTCPU);

    create_pinned(_io_thread, "APM_IO", IO_SS, this,
                  IO_PRIO, &_io_task_handle, SLOWCPU);

    create_pinned(_storage_thread, "APM_STORAGE", STORAGE_SS, this,
                  STORAGE_PRIO, &_storage_task_handle, SLOWCPU);
}

void Scheduler::delay(uint16_t ms)
{
    uint64_t start = AP_HAL::micros64();
    while ((AP_HAL::micros64() - start) / 1000 < ms) {
        delay_microseconds(1000);
        if (_min_delay_cb_ms <= ms) {
            if (in_main_thread()) {
                call_delay_cb();
            }
        }
    }
}

void Scheduler::delay_microseconds(uint16_t us)
{
    if (in_main_thread() && us < 100) {
        // busy-wait for short delays on main thread
        busy_wait_us_32(us);
    } else {
        // yield to freertos for longer delays
        uint32_t tick_us = portTICK_PERIOD_MS * 1000;
        vTaskDelay((us + tick_us - 1) / tick_us);
    }
}

void Scheduler::register_timer_process(AP_HAL::MemberProc proc)
{
    for (uint8_t i = 0; i < _num_timer_procs; i++) {
        if (_timer_proc[i] == proc) {
            return;
        }
    }
    if (_num_timer_procs >= RP2350_SCHEDULER_MAX_TIMER_PROCS) {
        printf("out of timer processes\n");
        return;
    }
    _timer_sem.take_blocking();
    _timer_proc[_num_timer_procs] = proc;
    _num_timer_procs++;
    _timer_sem.give();
}

void Scheduler::register_io_process(AP_HAL::MemberProc proc)
{
    _io_sem.take_blocking();
    for (uint8_t i = 0; i < _num_io_procs; i++) {
        if (_io_proc[i] == proc) {
            _io_sem.give();
            return;
        }
    }
    if (_num_io_procs < RP2350_SCHEDULER_MAX_IO_PROCS) {
        _io_proc[_num_io_procs] = proc;
        _num_io_procs++;
    } else {
        printf("out of IO processes\n");
    }
    _io_sem.give();
}

void Scheduler::register_timer_failsafe(AP_HAL::Proc failsafe, uint32_t period_us)
{
    _failsafe = failsafe;
}

void Scheduler::reboot(bool hold_in_bootloader)
{
    printf("rebooting...\n");
    hal.rcout->force_safety_on();
    watchdog_reboot(0, 0, 0);
    while (true) {}
}

bool Scheduler::in_main_thread() const
{
    return _main_task_handle == xTaskGetCurrentTaskHandle();
}

void Scheduler::set_system_initialized()
{
    if (_initialized) {
        AP_HAL::panic("PANIC: scheduler::system_initialized called more than once");
    }
    _initialized = true;
}

bool Scheduler::is_system_initialized()
{
    return _initialized;
}

// --- thread functions ---

void Scheduler::_main_thread(void *arg)
{
    Scheduler *sched = (Scheduler *)arg;

    hal.rcout->init();
    sched->callbacks->setup();
    sched->set_system_initialized();

    // enable watchdog with 3 second timeout
    watchdog_enable(3000, true);

    while (true) {
        sched->callbacks->loop();
        sched->delay_microseconds(250);
        watchdog_update();
    }
}

void Scheduler::_timer_thread(void *arg)
{
    Scheduler *sched = (Scheduler *)arg;

    while (!_initialized) {
        sched->delay_microseconds(1000);
    }

    while (true) {
        sched->delay_microseconds(1000);
        sched->_run_timers();
    }
}

void Scheduler::_run_timers()
{
    if (_in_timer_proc) {
        return;
    }
    _in_timer_proc = true;

    int num_procs = 0;
    _timer_sem.take_blocking();
    num_procs = _num_timer_procs;
    _timer_sem.give();

    for (int i = 0; i < num_procs; i++) {
        if (_timer_proc[i]) {
            _timer_proc[i]();
        }
    }

    if (_failsafe != nullptr) {
        _failsafe();
    }

    _in_timer_proc = false;
}

void Scheduler::_uart_thread(void *arg)
{
    Scheduler *sched = (Scheduler *)arg;

    while (!_initialized) {
        sched->delay_microseconds(2000);
    }

    while (true) {
        sched->delay_microseconds(1000);
        for (uint8_t i = 0; i < hal.num_serial; i++) {
            hal.serial(i)->_timer_tick();
        }
        hal.console->_timer_tick();
    }
}

void Scheduler::_io_thread(void *arg)
{
    Scheduler *sched = (Scheduler *)arg;

    while (!_initialized) {
        sched->delay_microseconds(1000);
    }

    while (true) {
        sched->delay_microseconds(1000);
        sched->_run_io();
    }
}

void Scheduler::_run_io()
{
    if (_in_io_proc) {
        return;
    }
    _in_io_proc = true;

    int num_procs = 0;
    _io_sem.take_blocking();
    num_procs = _num_io_procs;
    _io_sem.give();

    for (int i = 0; i < num_procs; i++) {
        if (_io_proc[i]) {
            _io_proc[i]();
        }
    }
    _in_io_proc = false;
}

void Scheduler::_storage_thread(void *arg)
{
    Scheduler *sched = (Scheduler *)arg;

    while (!_initialized) {
        sched->delay_microseconds(10000);
    }

    while (true) {
        sched->delay_microseconds(10000);
        hal.storage->_timer_tick();
    }
}

void Scheduler::thread_create_trampoline(void *ctx)
{
    AP_HAL::MemberProc *t = (AP_HAL::MemberProc *)ctx;
    (*t)();
    free(t);
    vTaskDelete(NULL);
}

bool Scheduler::thread_create(AP_HAL::MemberProc proc, const char *name,
                              uint32_t requested_stack_size,
                              priority_base base, int8_t priority)
{
    AP_HAL::MemberProc *tproc = (AP_HAL::MemberProc *)calloc(1, sizeof(proc));
    if (!tproc) {
        return false;
    }
    *tproc = proc;

    uint8_t thread_priority = IO_PRIO;
    static const struct {
        priority_base base;
        uint8_t p;
    } priority_map[] = {
        { PRIORITY_BOOST, MAIN_PRIO},
        { PRIORITY_MAIN, MAIN_PRIO},
        { PRIORITY_SPI, SPI_PRIORITY},
        { PRIORITY_I2C, I2C_PRIORITY},
        { PRIORITY_CAN, IO_PRIO},
        { PRIORITY_TIMER, TIMER_PRIO},
        { PRIORITY_RCIN, RCIN_PRIO},
        { PRIORITY_RCOUT, RCOUT_PRIO},
        { PRIORITY_IO, IO_PRIO},
        { PRIORITY_UART, UART_PRIO},
        { PRIORITY_STORAGE, STORAGE_PRIO},
        { PRIORITY_SCRIPTING, IO_PRIO},
        { PRIORITY_NET, IO_PRIO},
    };
    for (uint8_t i = 0; i < ARRAY_SIZE(priority_map); i++) {
        if (priority_map[i].base == base) {
            thread_priority = constrain_int16(priority_map[i].p + priority, 1, configMAX_PRIORITIES - 1);
            break;
        }
    }

    uint32_t actual_stack_size = requested_stack_size + 1024;

    TaskHandle_t xhandle;
    BaseType_t ret = xTaskCreate(thread_create_trampoline, name,
                                 actual_stack_size, tproc,
                                 thread_priority, &xhandle);
    if (ret != pdPASS) {
        free(tproc);
        return false;
    }
    return true;
}
