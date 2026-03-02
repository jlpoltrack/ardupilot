/*
 * system functions for RP2350
 * provides timing, panic handler, and HAL singleton
 *
 * date: 2026-02-28
 */

#include <AP_HAL/AP_HAL.h>
#include "HAL_RP2350_Class.h"
#include <AP_Math/div1000.h>

#include <pico/time.h>
#include <stdio.h>
#include <stdarg.h>

namespace AP_HAL
{

void panic(const char *errormsg, ...)
{
    va_list ap;
    va_start(ap, errormsg);
    vprintf(errormsg, ap);
    va_end(ap);
    printf("\n");

    while (true) {
        // halt
    }
}

uint32_t micros()
{
    return micros64();
}

uint32_t millis()
{
    return millis64();
}

uint64_t micros64()
{
    return time_us_64();
}

uint64_t millis64()
{
    return uint64_div1000(micros64());
}

} // namespace AP_HAL

static HAL_RP2350 hal_rp2350;

const AP_HAL::HAL& AP_HAL::get_HAL()
{
    return hal_rp2350;
}

AP_HAL::HAL& AP_HAL::get_HAL_mutable()
{
    return hal_rp2350;
}
