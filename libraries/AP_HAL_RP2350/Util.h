/*
 * utility class for RP2350
 *
 * date: 2026-03-02
 */
#pragma once

#include <AP_HAL/Util.h>
#include "HAL_RP2350_Namespace.h"

class RP2350::Util : public AP_HAL::Util
{
public:
    // return available heap memory in bytes
    uint32_t available_memory(void) override;

    // hardware RTC (not available on RP2350, stub only)
    void set_hw_rtc(uint64_t time_utc_usec) override {}
    uint64_t get_hw_rtc() const override { return 0; }
};
