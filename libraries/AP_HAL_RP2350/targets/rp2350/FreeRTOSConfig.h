/*
 * FreeRTOS SMP configuration for RP2350 (dual Cortex-M33)
 * uses raspberrypi/FreeRTOS-Kernel RP2350_ARM_NTZ port
 *
 * date: 2026-03-02
 */
#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

// --- ARM Cortex-M33 port-specific ---
#define configENABLE_FPU                        1   // RP2350 has single-precision FPU
#define configENABLE_MPU                        0   // not using MPU
#define configENABLE_TRUSTZONE                  0   // no TrustZone

// --- core configuration ---
#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION  0
#define configUSE_TICKLESS_IDLE                  0
#define configCPU_CLOCK_HZ                      150000000
#define configTICK_RATE_HZ                      1000
#define configMAX_PRIORITIES                    25
#define configMINIMAL_STACK_SIZE                256
#define configMAX_TASK_NAME_LEN                 16
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES   3

// --- SMP (dual-core) ---
#define configNUMBER_OF_CORES                   2
#define configRUN_MULTIPLE_PRIORITIES            1
#define configUSE_CORE_AFFINITY                 1
#define configTICK_CORE                         0
#define configUSE_PASSIVE_IDLE_HOOK             0

// --- SMP spinlocks (RP2350-specific) ---
// PICO_SPINLOCK_ID_OS1 = 14, PICO_SPINLOCK_ID_OS2 = 15
// using numeric values to avoid including hardware/sync.h here
// (that header depends on pico platform macros not yet available at config time)
#define configSMP_SPINLOCK_0                    14
#define configSMP_SPINLOCK_1                    15

// --- memory ---
#define configTOTAL_HEAP_SIZE                   (200 * 1024)
#define configSUPPORT_STATIC_ALLOCATION         1
#define configSUPPORT_DYNAMIC_ALLOCATION         1
#define configAPPLICATION_ALLOCATED_HEAP         0
#define configSTACK_DEPTH_TYPE                  uint32_t

// --- synchronization ---
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1
#define configQUEUE_REGISTRY_SIZE               8
#define configUSE_QUEUE_SETS                    0

// --- timers ---
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            1024

// --- hooks ---
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configUSE_MALLOC_FAILED_HOOK            1
#define configCHECK_FOR_STACK_OVERFLOW          2

// --- stats ---
#define configGENERATE_RUN_TIME_STATS           0
#define configUSE_TRACE_FACILITY                0
#define configUSE_STATS_FORMATTING_FUNCTIONS    0

// --- co-routines (not used) ---
#define configUSE_CO_ROUTINES                   0
#define configMAX_CO_ROUTINE_PRIORITIES         1

// --- interrupt priorities for ARM Cortex-M33 ---
#define configKERNEL_INTERRUPT_PRIORITY         (0xFF)
#define configMAX_SYSCALL_INTERRUPT_PRIORITY     (0x20)

// --- API includes ---
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_xResumeFromISR                  1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_uxTaskGetStackHighWaterMark     1
#define INCLUDE_xTaskGetIdleTaskHandle          1
#define INCLUDE_eTaskGetState                   1
#define INCLUDE_xTimerPendFunctionCall          1
#define INCLUDE_xTaskAbortDelay                 0
#define INCLUDE_xTaskGetHandle                  0
#define INCLUDE_xTaskResumeFromISR              1

// --- assertion ---
#include <assert.h>
#define configASSERT(x) assert(x)

#endif /* FREERTOS_CONFIG_H */
