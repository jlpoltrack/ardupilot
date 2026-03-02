cmake_minimum_required(VERSION 3.16)

# pico sdk must be available
set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
if(NOT PICO_SDK_PATH)
    get_filename_component(PICO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/../../../../modules/pico-sdk" ABSOLUTE)
endif()

# freertos kernel path
set(FREERTOS_KERNEL_PATH $ENV{FREERTOS_KERNEL_PATH})
if(NOT FREERTOS_KERNEL_PATH)
    get_filename_component(FREERTOS_KERNEL_PATH "${CMAKE_CURRENT_LIST_DIR}/../../../../modules/FreeRTOS-Kernel" ABSOLUTE)
endif()

# target RP2350 (ARM, not RISC-V)
set(PICO_BOARD pico2)
set(PICO_PLATFORM rp2350-arm-s)

# import pico sdk
include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# import FreeRTOS-Kernel using the RP2040/RP2350 SMP import cmake
# this auto-selects the RP2350_ARM_NTZ port based on PICO_PLATFORM
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(ardupilot_rp2350 C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# hard float ABI to match waf-compiled ArduPilot objects
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfloat-abi=hard -mfpu=fpv5-sp-d16")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfloat-abi=hard -mfpu=fpv5-sp-d16")

# initialize pico sdk
pico_sdk_init()

# --- ArduPilot executable ---
set(elf_file ardupilot)

add_executable(${elf_file} main.c)

# ardupilot libraries (built externally by waf, linked in)
if(NOT DEFINED ARDUPILOT_CMD)
    set(ARDUPILOT_CMD "none")
endif()

message("ARDUPILOT_CMD=${ARDUPILOT_CMD}")

IF(${ARDUPILOT_CMD} STREQUAL "plane")
    message("building for plane")
    set(ARDUPILOT_LIBS
        "${ARDUPILOT_BIN}/libarduplane.a"
        "${ARDUPILOT_LIB}/libArduPlane_libs.a")
ELSEIF(${ARDUPILOT_CMD} STREQUAL "copter")
    message("building for copter")
    set(ARDUPILOT_LIBS
        "${ARDUPILOT_BIN}/libarducopter.a"
        "${ARDUPILOT_LIB}/libArduCopter_libs.a")
ENDIF()

# link all libraries in a group to resolve circular dependencies
# between ArduPilot code and Pico SDK/TinyUSB/FreeRTOS
target_link_libraries(${elf_file}
    -Wl,--start-group
    -Wl,--whole-archive
    ${ARDUPILOT_LIBS}
    -Wl,--no-whole-archive
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
    pico_stdlib
    pico_time
    pico_multicore
    pico_atomic
    hardware_uart
    hardware_spi
    hardware_i2c
    hardware_gpio
    hardware_pwm
    hardware_adc
    hardware_flash
    hardware_watchdog
    tinyusb_device
    pico_stdio_usb
    stdc++
    supc++
    -Wl,--end-group
)

# enable usb cdc output
pico_enable_stdio_usb(${elf_file} 1)
pico_enable_stdio_uart(${elf_file} 0)

# generate uf2 output
pico_add_extra_outputs(${elf_file})

# board defines
target_compile_definitions(${elf_file} PRIVATE
    PICO_RP2350=1
    CONFIG_HAL_BOARD=14
    CONFIG_HAL_BOARD_SUBTYPE=-1
)

# include path for FreeRTOSConfig.h
target_include_directories(${elf_file} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# map file
set(mapfile "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.map")
target_link_options(${elf_file} PRIVATE "-Wl,--Map=${mapfile}")
target_link_options(${elf_file} PRIVATE "-Wl,--cref")
# allow link to complete despite ARM/Thumb relocation warnings
# waf objects are pure Thumb but lack full interwork mapping symbols
target_link_options(${elf_file} PRIVATE "-Wl,--noinhibit-exec")
