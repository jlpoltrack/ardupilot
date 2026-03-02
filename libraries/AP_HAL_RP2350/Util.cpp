/*
 * utility class for RP2350
 *
 * date: 2026-02-28
 */

#include <AP_HAL/AP_HAL.h>
#include "Util.h"

#include <FreeRTOS.h>

using namespace RP2350;

uint32_t Util::available_memory(void)
{
    return xPortGetFreeHeapSize();
}
