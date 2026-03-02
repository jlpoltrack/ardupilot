/*
 * generated hwdef header for rp2350-minimal
 * based on hwdef.dat
 *
 * date: 2026-02-28
 */
#pragma once

#define HAL_BOARD_NAME "rp2350-minimal"

// no IMU in phase 1
#define HAL_INS_DEFAULT HAL_INS_NONE

// no barometer in phase 1
#define HAL_BARO_DEFAULT HAL_BARO_NONE

// status LED
#define HAL_GPIO_LED_PIN 25

// storage size
#ifndef HAL_STORAGE_SIZE
#define HAL_STORAGE_SIZE 16384
#endif

// logging disabled (no SD card)
#define HAL_LOGGING_BACKENDS_DEFAULT 0
#define HAL_LOGGING_STACK_SIZE 2048

// program size limit
#ifndef HAL_PROGRAM_SIZE_LIMIT_KB
#define HAL_PROGRAM_SIZE_LIMIT_KB 4096
#endif

// board subtype
#ifndef CONFIG_HAL_BOARD_SUBTYPE
#define CONFIG_HAL_BOARD_SUBTYPE HAL_BOARD_SUBTYPE_NONE
#endif
