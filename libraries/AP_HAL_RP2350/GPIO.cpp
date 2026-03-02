/*
 * minimal GPIO driver for RP2350
 *
 * date: 2026-03-02
 */

#include <AP_HAL/AP_HAL.h>
#include "GPIO.h"

#include <hardware/gpio.h>
#include <pico/stdio_usb.h>

using namespace RP2350;

GPIO::GPIO()
{
}

void GPIO::init()
{
    // initialize onboard LED (GP25 on Pico 2)
    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);
}

void GPIO::pinMode(uint8_t pin, uint8_t output)
{
    gpio_init(pin);
    gpio_set_dir(pin, output ? GPIO_OUT : GPIO_IN);
}

uint8_t GPIO::read(uint8_t pin)
{
    return gpio_get(pin) ? 1 : 0;
}

void GPIO::write(uint8_t pin, uint8_t value)
{
    gpio_put(pin, value);
}

void GPIO::toggle(uint8_t pin)
{
    gpio_xor_mask(1u << pin);
}

bool GPIO::usb_connected()
{
    // use pico sdk's stdio_usb_connected() instead of direct tinyusb API
    return stdio_usb_connected();
}
