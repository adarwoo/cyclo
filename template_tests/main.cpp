/*
 * template_tests.cpp
 *
 * Created: 29/06/2021 21:26:21
 * Author : micro
 */ 

#include <avr/io.h>
#include <etl/string.h>

extern "C" {
#include "FreeRTOS.h"
#include "task.h"
};

#include "trace.h"

/**
 * As configSUPPORT_STATIC_ALLOCATION is set to 1, 
 * the application must provide an implementation of 
 * vApplicationGetIdleTaskMemory() to provide the memory that is used by the Idle task. 
 */
void vApplicationGetIdleTaskMemory( 
   StaticTask_t **ppxIdleTaskTCBBuffer,
   StackType_t **ppxIdleTaskStackBuffer,
   uint32_t *pulIdleTaskStackSize )
{
   /* If the buffers to be provided to the Idle task are declared inside this
   function then they must be declared static - otherwise they will be allocated on
   the stack and so not exists after this function exits. */
   static StaticTask_t xIdleTaskTCB;
   static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

   /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
   state will be stored. */
   *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

   /* Pass out the array that will be used as the Idle task's stack. */
   *ppxIdleTaskStackBuffer = uxIdleTaskStack;

   /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
   Note that, as the array is necessarily of type StackType_t,
   configMINIMAL_STACK_SIZE is specified in words, not bytes. */
   *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/* Dimensions of the buffer that the task being created will use as its stack.
    NOTE:  This is the number of words the stack will hold, not the number of
    bytes.  For example, if each stack item is 32-bits, and this is set to 100,
    then 400 bytes (100 * 32-bits) will be allocated. */
    #define STACK_SIZE 200

    /* Structure that will hold the TCB of the task being created. */
    StaticTask_t xTaskBuffer;
    StaticTask_t xTaskBuffer2;

    /* Buffer that the task being created will use as its stack.  Note this is
    an array of StackType_t variables.  The size of StackType_t is dependent on
    the RTOS port. */
    StackType_t xStack[ STACK_SIZE ];
    StackType_t xStack2[ STACK_SIZE ];

    /* Function that implements the task being created. */
    void vTaskCode( void * pvParameters )
    {
        /* The parameter value is expected to be 1 as 1 is passed in the
        pvParameters value in the call to xTaskCreateStatic(). */
        configASSERT( ( uint32_t ) pvParameters == 1UL );

        for( ;; )
        {
            trace_tgl(TRACE_TICK);
        }
    }

    /* Function that implements the task being created. */
    void vTaskCode2( void * pvParameters )
    {
        /* The parameter value is expected to be 1 as 1 is passed in the
        pvParameters value in the call to xTaskCreateStatic(). */
        configASSERT( ( uint32_t ) pvParameters == 1UL );

        for( ;; )
        {
            trace_tgl(TRACE_IDLE);
        }
    }

    /* Function that creates a task. */
    void vOtherFunction( void )
    {
        TaskHandle_t xHandle = NULL;

        /* Create the task without using any dynamic memory allocation. */
        xHandle = xTaskCreateStatic(
                      vTaskCode,       /* Function that implements the task. */
                      "T1",          /* Text name for the task. */
                      STACK_SIZE,      /* Number of indexes in the xStack array. */
                      ( void * ) 1,    /* Parameter passed into the task. */
                      tskIDLE_PRIORITY+1,/* Priority at which the task is created. */
                      xStack,          /* Array to use as the task's stack. */
                      &xTaskBuffer );  /* Variable to hold the task's data structure. */

        /* Create the task without using any dynamic memory allocation. */
        xHandle = xTaskCreateStatic(
                      vTaskCode2,       /* Function that implements the task. */
                      "T2",          /* Text name for the task. */
                      STACK_SIZE,      /* Number of indexes in the xStack array. */
                      ( void * ) 1,    /* Parameter passed into the task. */
                      tskIDLE_PRIORITY+1,/* Priority at which the task is created. */
                      xStack2,          /* Array to use as the task's stack. */
                      &xTaskBuffer2 );  /* Variable to hold the task's data structure. */

        /* puxStackBuffer and pxTaskBuffer were not NULL, so the task will have
        been created, and xHandle will be the task's handle.  Use the handle
        to suspend the task. */
        //vTaskSuspend( xHandle );
    }

int main(void)
{
   board_init();
   trace_set(TRACE_IDLE);
   
   vOtherFunction();
   trace_clear(TRACE_IDLE);
   vTaskStartScheduler();
}

