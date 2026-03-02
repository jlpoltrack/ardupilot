/*
 * FreeRTOS-based scheduler for RP2350
 *
 * date: 2026-02-28
 */
#pragma once

#include <AP_HAL/AP_HAL.h>
#include "HAL_RP2350_Namespace.h"
#include "Semaphores.h"

#include <FreeRTOS.h>
#include <task.h>

#define RP2350_SCHEDULER_MAX_TIMER_PROCS 10
#define RP2350_SCHEDULER_MAX_IO_PROCS 10

class RP2350::Scheduler : public AP_HAL::Scheduler
{
public:
    Scheduler();

    void init() override;
    void set_callbacks(AP_HAL::HAL::Callbacks *cb) { callbacks = cb; }
    void delay(uint16_t ms) override;
    void delay_microseconds(uint16_t us) override;
    void register_timer_process(AP_HAL::MemberProc) override;
    void register_io_process(AP_HAL::MemberProc) override;
    void register_timer_failsafe(AP_HAL::Proc, uint32_t period_us) override;
    void reboot(bool hold_in_bootloader) override;
    bool in_main_thread() const override;
    void set_system_initialized() override;
    bool is_system_initialized() override;

    static void thread_create_trampoline(void *ctx);
    bool thread_create(AP_HAL::MemberProc, const char *name,
                       uint32_t stack_size, priority_base base,
                       int8_t priority) override;

    // task priorities (configMAX_PRIORITIES = 25)
    static const int MAIN_PRIO    = 24;
    static const int TIMER_PRIO   = 23;
    static const int UART_PRIO    = 22;
    static const int SPI_PRIORITY = 20;
    static const int I2C_PRIORITY = 5;
    static const int RCIN_PRIO    = 5;
    static const int RCOUT_PRIO   = 10;
    static const int IO_PRIO      = 5;
    static const int STORAGE_PRIO = 4;

    // stack sizes (in words, FreeRTOS convention)
    static const int MAIN_SS    = 4096;
    static const int TIMER_SS   = 3072;
    static const int UART_SS    = 2048;
    static const int IO_SS      = 3072;
    static const int STORAGE_SS = 2048;

private:
    AP_HAL::HAL::Callbacks *callbacks;
    AP_HAL::Proc _failsafe;

    AP_HAL::MemberProc _timer_proc[RP2350_SCHEDULER_MAX_TIMER_PROCS];
    uint8_t _num_timer_procs;

    AP_HAL::MemberProc _io_proc[RP2350_SCHEDULER_MAX_IO_PROCS];
    uint8_t _num_io_procs;

    static bool _initialized;

    TaskHandle_t _main_task_handle;
    TaskHandle_t _timer_task_handle;
    TaskHandle_t _uart_task_handle;
    TaskHandle_t _io_task_handle;
    TaskHandle_t _storage_task_handle;

    static void _main_thread(void *arg);
    static void _timer_thread(void *arg);
    static void _uart_thread(void *arg);
    static void _io_thread(void *arg);
    static void _storage_thread(void *arg);

    bool _in_timer_proc;
    void _run_timers();
    Semaphore _timer_sem;

    bool _in_io_proc;
    void _run_io();
    Semaphore _io_sem;
};
