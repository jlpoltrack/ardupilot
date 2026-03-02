/*
 * main entry point for RP2350 ArduPilot build
 * called by pico sdk startup, hands off to AP_HAL
 *
 * date: 2026-02-28
 */

#include <pico/stdlib.h>

extern void setup(void);
extern void loop(void);

int main(void)
{
    // pico sdk initialization (clocks, gpio, usb)
    stdio_init_all();

    // brief delay for usb enumeration
    sleep_ms(500);

    // ardupilot entry point (calls HAL::run which starts FreeRTOS tasks)
    setup();

    // should never reach here - freertos scheduler takes over
    while (1) {
        loop();
    }
    return 0;
}
