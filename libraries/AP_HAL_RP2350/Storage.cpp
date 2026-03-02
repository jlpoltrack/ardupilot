/*
 * RAM-only storage for RP2350 (phase 1)
 *
 * date: 2026-02-28
 */

#include <AP_HAL/AP_HAL.h>
#include "Storage.h"

#include <string.h>

using namespace RP2350;

Storage::Storage()
    : _dirty(false)
{
    memset(_buffer, 0, sizeof(_buffer));
}

void Storage::init()
{
    // ram storage is ready immediately
}

void Storage::read_block(void *dst, uint16_t loc, size_t n)
{
    if (loc >= sizeof(_buffer)) return;
    if (loc + n > sizeof(_buffer)) {
        n = sizeof(_buffer) - loc;
    }
    _sem.take_blocking();
    memcpy(dst, &_buffer[loc], n);
    _sem.give();
}

void Storage::write_block(uint16_t loc, const void *src, size_t n)
{
    if (loc >= sizeof(_buffer)) return;
    if (loc + n > sizeof(_buffer)) {
        n = sizeof(_buffer) - loc;
    }
    _sem.take_blocking();
    memcpy(&_buffer[loc], src, n);
    _dirty = true;
    _sem.give();
}

void Storage::_timer_tick(void)
{
    // in phase 1 (ram-only), nothing to flush
    // phase 2 will write dirty pages to flash here
}
