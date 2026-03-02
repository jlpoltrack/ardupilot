/*
 * FreeRTOS semaphore implementations for RP2350
 *
 * date: 2026-02-28
 */

#include <AP_HAL/AP_HAL.h>
#include "Semaphores.h"

#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

using namespace RP2350;

// --- Semaphore (recursive mutex) ---

Semaphore::Semaphore()
{
    _handle = xSemaphoreCreateRecursiveMutex();
}

bool Semaphore::give()
{
    return xSemaphoreGiveRecursive(_handle) == pdTRUE;
}

bool Semaphore::take(uint32_t timeout_ms)
{
    if (timeout_ms == HAL_SEMAPHORE_BLOCK_FOREVER) {
        return xSemaphoreTakeRecursive(_handle, portMAX_DELAY) == pdTRUE;
    }
    return xSemaphoreTakeRecursive(_handle, pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
}

bool Semaphore::take_nonblocking()
{
    return xSemaphoreTakeRecursive(_handle, 0) == pdTRUE;
}

void Semaphore::take_blocking()
{
    xSemaphoreTakeRecursive(_handle, portMAX_DELAY);
}

// --- BinarySemaphore ---

BinarySemaphore::BinarySemaphore(bool initial_state)
    : AP_HAL::BinarySemaphore(initial_state)
{
    _sem = xSemaphoreCreateBinary();
    if (initial_state) {
        xSemaphoreGive(_sem);
    }
}

BinarySemaphore::~BinarySemaphore()
{
    vSemaphoreDelete(_sem);
}

bool BinarySemaphore::wait(uint32_t timeout_us)
{
    // convert microseconds to ticks, rounding up
    TickType_t ticks = pdMS_TO_TICKS((timeout_us + 999) / 1000);
    if (ticks == 0) {
        ticks = 1;
    }
    return xSemaphoreTake(_sem, ticks) == pdTRUE;
}

bool BinarySemaphore::wait_blocking()
{
    return xSemaphoreTake(_sem, portMAX_DELAY) == pdTRUE;
}

void BinarySemaphore::signal()
{
    xSemaphoreGive(_sem);
}

void BinarySemaphore::signal_ISR()
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(_sem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
