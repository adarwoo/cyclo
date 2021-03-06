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

/*
 * rtos.cpp
 *
 * Created: 27/08/2021 01:34:22
 *  Author: micro
 */

#include <rtos.hpp>

namespace rtos
{
   /**
    *  Acquire (take) a semaphore.
    *
    *  Example of blocking indefinitely:
    *      aSemaphore.Take();
    *
    *  Example of blocking for 100 ticks:
    *      aSemaphore.Take(100);
    *
    *  @param Timeout How long to wait to get the Lock until giving up.
    *  @return true if the Semaphore was acquired, false if it timed out.
    */
   bool Semaphore::take( tick_t timeout )
   {
      BaseType_t success;

      success= xSemaphoreTake( handle, timeout );

      return success == pdTRUE ? true : false;
   }

   /**
    *  Release (give) a semaphore.
    *
    *  @return true if the Semaphore was released, false if it failed.
    */
   bool Semaphore::give()
   {
      BaseType_t success;

      success= xSemaphoreGive( handle );

      return success == pdTRUE ? true : false;
   }

   /**
    *  Aquire (take) a semaphore from ISR context.
    *
    *  @param pxHigherPriorityTaskWoken Did this operation result in a
    *         rescheduling event.
    *  @return true if the Semaphore was acquired, false if it timed out.
    */
   bool Semaphore::take_from_isr( BaseType_t *pxHigherPriorityTaskWoken )
   {
      BaseType_t success;

      success= xSemaphoreTakeFromISR( handle, pxHigherPriorityTaskWoken );

      return success == pdTRUE ? true : false;
   }

   /**
    *  Release (give) a semaphore from ISR context.
    *
    *  @param pxHigherPriorityTaskWoken Did this operation result in a
    *         rescheduling event.
    *  @return true if the Semaphore was released, false if it failed.
    */
   bool Semaphore::give_from_isr( BaseType_t *pxHigherPriorityTaskWoken )
   {
      BaseType_t success;

      success= xSemaphoreGiveFromISR( handle, pxHigherPriorityTaskWoken );

      return success == pdTRUE ? true : false;
   }

   Semaphore::~Semaphore() {}

   Semaphore::Semaphore() {}

   BinarySemaphore::BinarySemaphore( bool set )
   {
      handle= xSemaphoreCreateBinaryStatic( &xSemaphoreBuffer );

      if ( handle == NULL )
      {
         configASSERT( ! "BinarySemaphore Constructor Failed" );
      }

      if ( set )
      {
         xSemaphoreGive( handle );
      }
   }

#if ( ( configUSE_COUNTING_SEMAPHORES == 1 ) && ( configSUPPORT_STATIC_ALLOCATION == 1 ) )
   /**
    *  Constructor to create a counting semaphore.
    *  This ctor throws a SemaphoreCreateException on failure.
    *
    *  @param maxCount Must be greater than 0.
    *  @param initialCount Must not be greater than maxCount.
    *  @throws SemaphoreCreateException on failure.
    *  @return Instance of a CountingSemaphore.
    */
   CountingSemaphore::CountingSemaphore( UBaseType_t maxCount, UBaseType_t initialCount )
   {
      if ( maxCount == 0 )
      {
         configASSERT( ! "CountingSemaphore Constructor bad maxCount" );
      }

      if ( initialCount > maxCount )
      {
         configASSERT( ! "CountingSemaphore Constructor bad initialCount" );
      }

      handle= xSemaphoreCreateCountingStatic( maxCount, initialCount, &xSemaphoreBuffer );

      if ( handle == NULL )
      {
         configASSERT( ! "CountingSemaphore constructor Failed" );
      }
   }
#endif

#if ( ( configUSE_MUTEXES == 1 ) && ( configSUPPORT_STATIC_ALLOCATION == 1 ) )
   Mutex::Mutex()
   {
      handle= xSemaphoreCreateMutexStatic( &xSemaphoreBuffer );
      assert(handle);
   }
#endif


   /**
    *  Constructor
    *  @note Do not construct inside an ISR! This includes creating
    *  local instances of this object.
    */
   Tasklet::Tasklet()
   {
      DtorLock = xSemaphoreCreateBinaryStatic( &xSemaphoreBuffer );

      assert( DtorLock );

      xSemaphoreGive( DtorLock );
   }

   /**
    *  Destructor
    *  @note Do not delete inside an ISR! This includes the automatic
    *  deletion of local instances of this object when leaving scope.
    */
   Tasklet::~Tasklet() {}

   /**
    *  Schedule this Tasklet to run.
    *
    *  @param parameter Value passed to your Run method.
    *  @param CmdTimeout How long to wait to send this command to the
    *         timer daemon.
    *  @returns true if this command will be sent to the timer daemon,
    *           false if it will not (i.e. timeout).
    */
   bool Tasklet::schedule( uint32_t parameter, tick_t CmdTimeout )
   {
      BaseType_t rc;

      xSemaphoreTake( DtorLock, portMAX_DELAY );

      rc= xTimerPendFunctionCall( TaskletAdapterFunction, this, parameter, CmdTimeout );

      if ( rc == pdPASS )
      {
         return true;
      }
      else
      {
         xSemaphoreGive( DtorLock );
         return false;
      }
   }

   /**
    *  Schedule this Tasklet to run from ISR context.
    *  This allows FreeRTOS ISRs to defer processing from the ISR
    *  into a task context.
    *
    *  @param parameter Value passed to your Run method.
    *  @param pxHigherPriorityTaskWoken Did this operation result in a
    *         rescheduling event.
    *  @returns true if this command will be sent to the timer daemon,
    *           false if it will not (i.e. timeout).
    */
   bool Tasklet::schedule_from_isr( uint32_t parameter )
   {
      BaseType_t pxHigherPriorityTaskWoken;
      BaseType_t rc;

      xSemaphoreTake( DtorLock, portMAX_DELAY );

      rc= xTimerPendFunctionCallFromISR(
         TaskletAdapterFunction, this, parameter, &pxHigherPriorityTaskWoken );

      if ( rc == pdPASS )
      {
         if ( pxHigherPriorityTaskWoken )
         {
            taskYIELD();
         }

         return true;
      }
      else
      {
         xSemaphoreGive( DtorLock );
         return false;
      }
   }

   /**
    *  You must call this in your dtor, to synchronize between
    *  being called and being deleted.
    */
   void Tasklet::check_for_safe_delete()
   {
      xSemaphoreTake( DtorLock, portMAX_DELAY );
      vSemaphoreDelete( DtorLock );
   }

   /**
    *  Adapter function that allows you to write a class
    *  specific run() function that interfaces with FreeRTOS.
    *  Look at the implementation of the constructors and this
    *  code to see how the interface between C and C++ is performed.
    */
   void Tasklet::TaskletAdapterFunction( void *ref, uint32_t parameter )
   {
      Tasklet *tasklet= static_cast< Tasklet * >( ref );
      tasklet->run( parameter );
      xSemaphoreGive( tasklet->DtorLock );
   }
}  // namespace rtos