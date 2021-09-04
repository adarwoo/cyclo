/******************************************************************************
The MIT License(MIT)
https://github.com/adarwoo/cyclo

Copyright(c) 2021 Guillaume ARRECKX - software@arreckx.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>

#include <logger.h>


void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
   LOG_ERROR( NULL, "Stack overflow" );

#ifndef _POSIX
   asm( "break" );
#endif
   while ( 1 )
      ;
}

/**
 * As configSUPPORT_STATIC_ALLOCATION is set to 1,
 * the application must provide an implementation of
 * vApplicationGetIdleTaskMemory() to provide the memory that is used by the Idle task.
 */
void vApplicationGetIdleTaskMemory(
   StaticTask_t **ppxIdleTaskTCBBuffer,
   StackType_t ** ppxIdleTaskStackBuffer,
   uint32_t *     pulIdleTaskStackSize )
{
   /* If the buffers to be provided to the Idle task are declared inside this
   function then they must be declared static - otherwise they will be allocated on
   the stack and so not exists after this function exits. */
   static StaticTask_t xIdleTaskTCB;
   static StackType_t  uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

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


void vApplicationGetTimerTaskMemory(
   StaticTask_t **ppxTimerTaskTCBBuffer,
   StackType_t ** ppxTimerTaskStackBuffer,
   uint32_t *     pulTimerTaskStackSize )
{
   static StaticTask_t xTimerTaskTCB;
   static StackType_t  uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

   /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
   state will be stored. */
   *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

   /* Pass out the array that will be used as the Idle task's stack. */
   *ppxTimerTaskStackBuffer = uxTimerTaskStack;

   /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
   Note that, as the array is necessarily of type StackType_t,
   configMINIMAL_STACK_SIZE is specified in words, not bytes. */
   *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

#ifdef _POSIX
void vApplicationIdleHook( void )
{}
#endif