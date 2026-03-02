/*
 * RAM-only storage for RP2350 (phase 1)
 * parameters do not persist across reboot
 *
 * date: 2026-02-28
 */
#pragma once

#include <AP_HAL/Storage.h>
#include "HAL_RP2350_Namespace.h"
#include "Semaphores.h"

class RP2350::Storage : public AP_HAL::Storage
{
public:
    Storage();
    void init() override;
    bool healthy(void) override { return true; }
    void read_block(void *dst, uint16_t loc, size_t n) override;
    void write_block(uint16_t loc, const void *src, size_t n) override;
    void _timer_tick(void) override;

private:
    // ram-backed storage buffer
    uint8_t _buffer[HAL_STORAGE_SIZE];
    volatile bool _dirty;
    Semaphore _sem;
};
