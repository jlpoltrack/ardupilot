/*
 * HAL class for RP2350 - wires up all driver instances
 *
 * date: 2026-02-28
 */

#include <AP_HAL/AP_HAL.h>
#include <AP_HAL_Empty/AP_HAL_Empty_Private.h>

#include "HAL_RP2350_Class.h"
#include "Scheduler.h"
#include "UARTDriver.h"
#include "GPIO.h"
#include "Storage.h"
#include "Semaphores.h"
#include "Util.h"

// serial 0 = usb cdc (console/mavlink)
static RP2350::UARTDriver cons(0);
// serial 1 = uart0 (telem1)
static RP2350::UARTDriver serial1Driver(1);
// serial 2 = uart1 (gps)
static RP2350::UARTDriver serial2Driver(2);
// remaining serials are empty stubs
static Empty::UARTDriver serial3Driver;
static Empty::UARTDriver serial4Driver;
static Empty::UARTDriver serial5Driver;
static Empty::UARTDriver serial6Driver;
static Empty::UARTDriver serial7Driver;
static Empty::UARTDriver serial8Driver;
static Empty::UARTDriver serial9Driver;

// stubbed device managers (phase 2)
static Empty::SPIDeviceManager spiDeviceManager;
static Empty::I2CDeviceManager i2cDeviceManager;
static Empty::AnalogIn analogIn;
static Empty::Flash flashDriver;
static Empty::OpticalFlow opticalFlowDriver;

// real drivers
static RP2350::GPIO gpioDriver;
static RP2350::Storage storageDriver;
static Empty::RCOutput rcoutDriver;
static Empty::RCInput rcinDriver;
static RP2350::Scheduler schedulerInstance;
static RP2350::Util utilInstance;

#if HAL_WITH_DSP
static Empty::DSP dspDriver;
#endif

extern const AP_HAL::HAL& hal;

HAL_RP2350::HAL_RP2350() :
    AP_HAL::HAL(
        &cons,            // console/mavlink
        &serial1Driver,   // telem 1
        &serial2Driver,   // gps 1
        &serial3Driver,   // gps 2
        &serial4Driver,   // extra 1
        &serial5Driver,   // extra 2
        &serial6Driver,   // extra 3
        &serial7Driver,   // extra 4
        &serial8Driver,   // extra 5
        &serial9Driver,   // extra 6
        &i2cDeviceManager,
        &spiDeviceManager,
        nullptr,          // wspi
        &analogIn,
        &storageDriver,
        &cons,            // console
        &gpioDriver,
        &rcinDriver,
        &rcoutDriver,
        &schedulerInstance,
        &utilInstance,
        &opticalFlowDriver,
        &flashDriver,
#if AP_SIM_ENABLED
        nullptr,          // simstate
#endif
#if HAL_WITH_DSP
        &dspDriver,
#endif
        nullptr           // can ifaces
    )
{}

void HAL_RP2350::run(int argc, char * const argv[], Callbacks* callbacks) const
{
    ((RP2350::Scheduler *)hal.scheduler)->set_callbacks(callbacks);
    hal.scheduler->init();

    // should never reach here - freertos tasks take over
    while (true) {}
}

void AP_HAL::init()
{
}
