/*
 * board header for RP2350 (Raspberry Pi Pico 2) with FreeRTOS
 *
 * date: 2026-02-28
 */
#pragma once

#include <hwdef.h>

// board identity
#ifndef HAL_BOARD_NAME
#define HAL_BOARD_NAME "RP2350"
#endif
#define HAL_CPU_CLASS HAL_CPU_CLASS_150
#define HAL_MEM_CLASS HAL_MEM_CLASS_300

#define O_CLOEXEC 0

// storage
#ifndef HAL_STORAGE_SIZE
#define HAL_STORAGE_SIZE (16384)
#endif

#ifndef HAL_PROGRAM_SIZE_LIMIT_KB
#define HAL_PROGRAM_SIZE_LIMIT_KB 4096
#endif

// semaphore types
#ifdef __cplusplus
#include <AP_HAL_RP2350/Semaphores.h>
#define HAL_Semaphore RP2350::Semaphore
#define HAL_BinarySemaphore RP2350::BinarySemaphore
#endif

// hardware capabilities (not present)
#define HAL_WITH_DRONECAN 0
#define HAL_WITH_UAVCAN 0
#define HAL_NUM_CAN_IFACES 0
#define HAL_MAX_CAN_PROTOCOL_DRIVERS 0
#define HAL_HAVE_SAFETY_SWITCH 0
#define HAL_HAVE_BOARD_VOLTAGE 0
#define HAL_HAVE_SERVO_VOLTAGE 0
#define HAL_WITH_IO_MCU 0

// cortex-m33 has single-precision FPU only
#ifndef HAL_HAVE_HARDWARE_DOUBLE
#define HAL_HAVE_HARDWARE_DOUBLE 0
#endif

#ifndef HAL_WITH_EKF_DOUBLE
#define HAL_WITH_EKF_DOUBLE HAL_HAVE_HARDWARE_DOUBLE
#endif

// endianness
#define __LITTLE_ENDIAN  1234
#define __BYTE_ORDER     __LITTLE_ENDIAN

// ram function attribute (pico sdk equivalent)
#define __RAMFUNC__ __attribute__((section(".time_critical")))

// servo channels
#define NUM_SERVO_CHANNELS 16

// generous overtime margin during bring-up
#define AP_SCHEDULER_OVERTIME_MARGIN_US 50000UL

// filesystem and logging (no SD card, no POSIX filesystem)
#define AP_FILESYSTEM_POSIX_ENABLED 0
#define AP_FILESYSTEM_FATFS_ENABLED 0
#define AP_FILESYSTEM_LITTLEFS_ENABLED 0
#define AP_FILESYSTEM_ROMFS_ENABLED 0
#define AP_FILESYSTEM_SYS_ENABLED 0
#define AP_FILESYSTEM_PARAM_ENABLED 0
#define AP_FILESYSTEM_FILE_READING_ENABLED 0
#define AP_FILESYSTEM_FILE_WRITING_ENABLED 0
#define AP_FILESYSTEM_FORMAT_ENABLED 0
#define HAL_LOGGING_ENABLED 0
#define HAL_LOGGING_BACKENDS_DEFAULT 0
#define AP_MAVLINK_FTP_ENABLED 0

// peripherals (stubbed in phase 1)
#ifndef AP_COMPASS_BACKEND_DEFAULT_ENABLED
#define AP_COMPASS_BACKEND_DEFAULT_ENABLED 0
#endif
#define HAL_INS_TEMPERATURE_CAL_ENABLE 0
#define AP_EXTERNAL_AHRS_ENABLED 0
#define AP_BATTMONITOR_TORQEEDO_ENABLED 0
#define AP_BATTERY_TORQEEDO_ENABLED 0
#define HAL_TORQEEDO_ENABLED 0

// vehicle features (too large / not applicable)
#define HAL_QUADPLANE_ENABLED 0
#define HAL_GYROFFT_ENABLED 0
#define HAL_SOARING_ENABLED 0
#define AP_ADVANCEDFAILSAFE_ENABLED 0
#define AP_ICENGINE_ENABLED 0
#define HAL_GENERATOR_ENABLED 0
#define HAL_PARACHUTE_ENABLED 0
#define AP_STATS_ENABLED 0
#define AP_OPENDRONEID_ENABLED 0
#define AP_DRONECAN_ENABLED 0

// sensors and peripherals
#define AP_OPTICALFLOW_ENABLED 0
#define AP_RPM_ENABLED 0
#define AP_AIRSPEED_AUTOCAL_ENABLE 0
#define AP_AIRSPEED_ENABLED 0
#define AP_RANGEFINDER_ENABLED 0
#define AP_BEACON_ENABLED 0
#define AP_PROXIMITY_ENABLED 0

// accessories and payloads
#define HAL_MOUNT_ENABLED 0
#define AP_CAMERA_ENABLED 0
#define AP_GRIPPER_ENABLED 0
#define AP_LANDINGGEAR_ENABLED 0
#define HAL_ADSB_ENABLED 0
#define HAL_BUTTON_ENABLED 0
#define AP_SERVORELAYEVENTS_ENABLED 0
#define HAL_SPRAYER_ENABLED 0

// navigation features
#define AP_AVOIDANCE_ENABLED 0
#define AP_FENCE_ENABLED 0
#define MODE_FOLLOW_ENABLED 0
#define AP_OAPATHPLANNER_ENABLED 0
#define AP_TERRAIN_AVAILABLE 0

// scripting and extras
#define AP_SCRIPTING_ENABLED 0
#define HAL_RALLY_ENABLED 0
#define HAL_DISPLAY_ENABLED 0
#define HAL_LOGGING_MAVLINK_ENABLED 0

// telemetry protocols (depend on battery/GPS/etc)
#define AP_FRSKY_TELEM_ENABLED 0
#define HAL_CRSF_TELEM_ENABLED 0
#define AP_GHST_TELEM_ENABLED 0
#define AP_SERIALMANAGER_NUM_PORTS 3

#ifndef AP_NOTIFY_BUZZER_ENABLED
#define AP_NOTIFY_BUZZER_ENABLED 0
#endif
