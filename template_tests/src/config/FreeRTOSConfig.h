#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H
/**
 * Configuration for the RTOS for the AVR
 */

/*
 * #define TCB_t to avoid conflicts between the
 * FreeRTOS task control block type (TCB_t) and the
 * AVR Timer Counter B type (TCB_t)
 */
#define TCB_t avrTCB_t
#include <avr/io.h>
#undef TCB_t

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE. 
 *
 * See http://www.freertos.org/a00110.html.
 *----------------------------------------------------------*/

#define configUSE_PREEMPTION					1
#define configUSE_IDLE_HOOK					1
#define configUSE_TICK_HOOK					0

// For the AVR, do not over switch as each context switch costs a fair bit
#define configTICK_RATE_HZ						100

// atxmega  32A4 has a 2 byte program counter and 2 byte stack pointer, push/pop increments/decrements by 2
// atxmega 128A1 has a 3 byte program counter and 2 byte stack pointer, push/pop increments/decrements by 3
// atxmega 256a3 has a 3 byte program counter and 2 byte stack pointer, push/pop increments/decrements by 3
//
#define config24BITADDRESSING					1 // 0 for 32A4 ; 1 for 128A1,256A3

// controls whether the registers RAMPD,RAMPX,RAMPZ are saved in the context of a task.
// The gcc compiler does it, but I think that most applications don't need it. So leaving
// this to 0 makes the context switch faster and uses less stack space. If you suspect you need this,
// or just want to be on the safe side, set this value to 1. Rethink your decision if you are using 
// external RAM or a CPU with large program flash (>128k).
//
#define configEXTENDED_ADRESSING				0

// This enables round robin scheduling of low-level interrupts.
// When this option is 0, low-level interrupts are priority controlled.
// This option is not available for med- and high-level interrupts (the xmega hardware does not support this).
//
#define configENABLE_ROUND_ROBIN				1

// This defines the interrupt level on which the kernel is running (tick-interrupt).
// This shall be the lowest possible value: 0.
//
#define configKERNEL_INTERRUPT_PRIORITY	0	// kernel interrupt level is low-level, don't change!

// This value defines which ISRs are allowed to call the light-weight
// api functions (the fromISR... ones).
// ISRs running in a level < configKERNEL_INTERRUPT_PRIORITY are never allowed to call the 
// light-weight api functions.  
#define configMAX_SYSCALL_INTERRUPT_PRIORITY	2	// 0=low-level ,1=medium-level, 2=high-level

#if (configMAX_SYSCALL_INTERRUPT_PRIORITY < configKERNEL_INTERRUPT_PRIORITY)
    #error configMAX_SYSCALL_INTERRUPT_PRIORITY must be >= configKERNEL_INTERRUPT_PRIORITY !
#endif

// 4 levels of priorities only to reduce the eval cost during context switch
#define configMAX_PRIORITIES					   4
#define configMINIMAL_STACK_SIZE				   128
#define configTOTAL_HEAP_SIZE					   2048
#ifdef DEBUG
    #define configUSE_MALLOC_FAILED_HOOK		1
#else
    #define configUSE_MALLOC_FAILED_HOOK		0
#endif
#define configMAX_TASK_NAME_LEN					8
   #define configUSE_TRACE_FACILITY				0
#define configUSE_16_BIT_TICKS					0    // 0 means 32bit ticks
#define configIDLE_SHOULD_YIELD					1
#define configQUEUE_REGISTRY_SIZE				0
#define configCHECK_FOR_STACK_OVERFLOW 		2    // use both method 1 and method 2
#define configAPPLICATION_ALLOCATED_HEAP		0

/* Memory allocation related definitions. */
#define configSUPPORT_STATIC_ALLOCATION		1
#define configSUPPORT_DYNAMIC_ALLOCATION		0

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES 					0
#define configMAX_CO_ROUTINE_PRIORITIES		2
#define configNUMBER_OF_COROUTINES				6
#define configUSE_TASK_NOTIFICATIONS			1
#define configUSE_MUTEXES						   1
#define configUSE_RECURSIVE_MUTEXES				0
#define configUSE_COUNTING_SEMAPHORES			0
#define configUSE_QUEUE_SETS					   0
#define configUSE_TIME_SLICING					1 // Allow task of same prio to share the time
#define configUSE_NEWLIB_REENTRANT				0
/* Software timer related definitions. */
#define configUSE_TIMERS						   1
/* Hook function related definitions. */
#define configUSE_DAEMON_TASK_STARTUP_HOOK	0
#define configENABLE_BACKWARD_COMPATIBILITY	0
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 0
/* Run time and task stats gathering related definitions. */
#define configGENERATE_RUN_TIME_STATS			0
#define configUSE_STATS_FORMATTING_FUNCTIONS	0
#define configTIMER_TASK_PRIORITY				(configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH				   5
#define configTIMER_TASK_STACK_DEPTH			(configMINIMAL_STACK_SIZE * 2)

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

/* Optional functions - most linkers will remove unused functions anyway. */
#define INCLUDE_vTaskPrioritySet			   	0
#define INCLUDE_uxTaskPriorityGet				0
#define INCLUDE_vTaskDelete						1
#define INCLUDE_vTaskSuspend			   		1 // Needed to block on queues
#define INCLUDE_vTaskDelayUntil					1
#define INCLUDE_vTaskDelay					   	1
#define INCLUDE_xTaskGetSchedulerState			0
#define INCLUDE_xTaskGetCurrentTaskHandle		0
#define INCLUDE_uxTaskGetStackHighWaterMark	0
#define INCLUDE_xTaskGetIdleTaskHandle			0
#define INCLUDE_eTaskGetState					   0
#define INCLUDE_xEventGroupSetBitFromISR		0
#define INCLUDE_xTimerPendFunctionCall			1
#define INCLUDE_xTaskAbortDelay					0
#define INCLUDE_xTaskGetHandle					0
#define INCLUDE_xTaskResumeFromISR				0


#endif /* FREERTOS_CONFIG_H */
