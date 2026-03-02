/*
 * minimal GPIO driver for RP2350
 *
 * date: 2026-02-28
 */
#pragma once

#include <AP_HAL/GPIO.h>
#include "HAL_RP2350_Namespace.h"

class RP2350::GPIO : public AP_HAL::GPIO
{
public:
    GPIO();
    void init() override;
    void pinMode(uint8_t pin, uint8_t output) override;
    uint8_t read(uint8_t pin) override;
    void write(uint8_t pin, uint8_t value) override;
    void toggle(uint8_t pin) override;

    // channel operations (stubbed)
    AP_HAL::DigitalSource* channel(uint16_t n) override { return nullptr; }
    bool usb_connected() override;
};
