/**
 * @file
 * @{
 * C++ abstraction of FreeRTOS.
 * 
 *
 * Created: 01/07/2021 12:19:42
 *  Author: micro
 */

#ifndef RTOS_H_
#define RTOS_H_

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "message_buffer.h"

#include "etl/algorithm.h"
#include "etl/delegate.h"

#include "typestring.hpp"

#include "trace.h"

namespace rtos
{
   // Priority level C++ way. Based on the FreeRTOS configuration
   enum class priority_t : UBaseType_t
   {
      idle = tskIDLE_PRIORITY,
      low = (configMAX_PRIORITIES >> 3),
      normal = (configMAX_PRIORITIES >> 2),
      high = (configMAX_PRIORITIES - 1)
   };

   /** TickType_t becomes rtos::tick_t */
   using tick_t = TickType_t;

   /** In lieu of BaseType_t */
   using type_t = BaseType_t;

   namespace tick
   {
      constexpr tick_t infinite = portMAX_DELAY;
      constexpr tick_t per_second = configTICK_RATE_HZ;

      constexpr tick_t operator"" _ms(unsigned long long milliseconds)
      {
         return static_cast<tick_t>((milliseconds * per_second) / 1000);
      }

      constexpr tick_t operator"" _s(unsigned long long seconds)
      {
         return static_cast<tick_t>(per_second * seconds);
      }

      constexpr tick_t operator"" _s(long double seconds)
      {
         return static_cast<tick_t>(per_second * seconds);
      }

      constexpr tick_t operator"" _M(unsigned long long minutes)
      {
         return static_cast<tick_t>(per_second * 60 * minutes);
      }
   }

   /**
    *
    *  Base wrapper class around FreeRTOS's implementation of semaphores.
    *
    *  It is not expected that an application will derive from this class.
    *
    *  Note that we distinguish between Semaphore, Binary Semaphores,
    *  Counting Semaphores, and Mutexes. Mutexes, while implemented as a kind
    *  of semaphore in FreeRTOS, are conceptually very different in use and
    *  behavior from semaphores. We acknowledge this difference in the class
    *  heirarchy, implementing mutextes as a completely different class heirarchy.
    */
   class Semaphore
   {
   public:
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
      bool take(tick_t timeout = tick::infinite)
      {
         BaseType_t success;

         success = xSemaphoreTake(handle, timeout);

         return success == pdTRUE ? true : false;
      }

      /**
       *  Release (give) a semaphore.
       *
       *  @return true if the Semaphore was released, false if it failed.
       */
      bool give()
      {
         BaseType_t success;

         success = xSemaphoreGive(handle);

         return success == pdTRUE ? true : false;
      }

      /**
       *  Aquire (take) a semaphore from ISR context.
       *
       *  @param pxHigherPriorityTaskWoken Did this operation result in a
       *         rescheduling event.
       *  @return true if the Semaphore was acquired, false if it timed out.
       */
      bool take_from_isr(BaseType_t *pxHigherPriorityTaskWoken)
      {
         BaseType_t success;

         success = xSemaphoreTakeFromISR(handle, pxHigherPriorityTaskWoken);

         return success == pdTRUE ? true : false;
      }

      /**
       *  Release (give) a semaphore from ISR context.
       *
       *  @param pxHigherPriorityTaskWoken Did this operation result in a
       *         rescheduling event.
       *  @return true if the Semaphore was released, false if it failed.
       */
      bool give_from_isr(BaseType_t *pxHigherPriorityTaskWoken)
      {
         BaseType_t success;

         success = xSemaphoreGiveFromISR(handle, pxHigherPriorityTaskWoken);

         return success == pdTRUE ? true : false;
      }

      /**
       *  Our destructor
       */
      virtual ~Semaphore() {}

   protected:
      StaticSemaphore_t xSemaphoreBuffer;

      /**
       *  FreeRTOS semaphore handle.
      */
      SemaphoreHandle_t handle;

      /**
       *  We do not want a Semaphore ctor. This class should never be
       *  directly created, this is a base class only.
       */
      Semaphore() {}
   };

   /**
    *  Wrapper class for Binary Semaphores.
    */
   class BinarySemaphore : public Semaphore
   {
   public:
      explicit BinarySemaphore(bool set = false)
      {
         handle = xSemaphoreCreateBinaryStatic(&xSemaphoreBuffer);

         if (handle == NULL)
         {
            configASSERT(!"BinarySemaphore Constructor Failed");
         }

         if (set)
         {
            xSemaphoreGive(handle);
         }
      }
   };

   /**
    *  Wrapper class for Counting Semaphores.
    */
   class CountingSemaphore : public Semaphore
   {
   public:
      /**
       *  Constructor to create a counting semaphore.
       *  This ctor throws a SemaphoreCreateException on failure.
       *
       *  @param maxCount Must be greater than 0.
       *  @param initialCount Must not be greater than maxCount.
       *  @throws SemaphoreCreateException on failure.
       *  @return Instance of a CountingSemaphore.
       */
      CountingSemaphore(UBaseType_t maxCount, UBaseType_t initialCount)
      {
         if (maxCount == 0)
         {
            configASSERT(!"CountingSemaphore Constructor bad maxCount");
         }

         if (initialCount > maxCount)
         {
            configASSERT(!"CountingSemaphore Constructor bad initialCount");
         }

         handle = xSemaphoreCreateCountingStatic(maxCount, initialCount, &xSemaphoreBuffer);

         if (handle == NULL)
         {
            configASSERT(!"CountingSemaphore Constructor Failed");
         }
      }
   };

   /**
    *  A FreeRTOS wrapper for its concept of a Pended Function.
    *  In Linux, one permutation of this would be a Tasklet, or
    *  bottom half processing from an ISR.
    *
    *  This is an abstract base class.
    *  To use this, you need to subclass it. All of your Tasklets should
    *  be derived from the Tasklet class. Then implement the virtual Run
    *  function. This is a similar design to Java threading.
    */
   class Tasklet
   {

   public:
      /**
       *  Constructor
       *  @note Do not construct inside an ISR! This includes creating 
       *  local instances of this object.
       */
      Tasklet()
      {
         DtorLock = xSemaphoreCreateBinaryStatic(&xSemaphoreBuffer);

         if (DtorLock == NULL)
         {
            configASSERT(!"Tasklet Constructor Failed");
         }

         xSemaphoreGive(DtorLock);
      }

      /**
       *  Destructor
       *  @note Do not delete inside an ISR! This includes the automatic 
       *  deletion of local instances of this object when leaving scope.
       */
      virtual ~Tasklet() {}

      /**
       *  Schedule this Tasklet to run.
       *
       *  @param parameter Value passed to your Run method.
       *  @param CmdTimeout How long to wait to send this command to the
       *         timer daemon.
       *  @returns true if this command will be sent to the timer daemon,
       *           false if it will not (i.e. timeout).
       */
      bool schedule(uint32_t parameter, tick_t CmdTimeout = tick::infinite)
      {
         BaseType_t rc;

         xSemaphoreTake(DtorLock, portMAX_DELAY);

         rc = xTimerPendFunctionCall(TaskletAdapterFunction, this, parameter, CmdTimeout);

         if (rc == pdPASS)
         {
            return true;
         }
         else
         {
            xSemaphoreGive(DtorLock);
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
      bool schedule_from_isr(uint32_t parameter)
      {
         BaseType_t pxHigherPriorityTaskWoken;
         BaseType_t rc;

         xSemaphoreTake(DtorLock, portMAX_DELAY);
         rc = xTimerPendFunctionCallFromISR(TaskletAdapterFunction,
                                            this, parameter, &pxHigherPriorityTaskWoken);

         if (rc == pdPASS)
         {
            if (pxHigherPriorityTaskWoken)
            {
               taskYIELD();
            }

            return true;
         }
         else
         {
            xSemaphoreGive(DtorLock);
            return false;
         }
      }

   protected:
      /**
       *  Implementation of your actual tasklet code.
       *  You must override this function.
       *
       *  @param parameter Value passed to you from the Schedule() methods.
       */
      virtual void run(uint32_t parameter) = 0;

      /**
       *  You must call this in your dtor, to synchronize between 
       *  being called and being deleted.
       */
      void check_for_safe_delete()
      {
         xSemaphoreTake(DtorLock, portMAX_DELAY);
         vSemaphoreDelete(DtorLock);
      }

   private:
      /**
       *  Adapter function that allows you to write a class
       *  specific run() function that interfaces with FreeRTOS.
       *  Look at the implementation of the constructors and this
       *  code to see how the interface between C and C++ is performed.
       */
      static void TaskletAdapterFunction(void *ref, uint32_t parameter)
      {
         Tasklet *tasklet = static_cast<Tasklet *>(ref);
         tasklet->run(parameter);
         xSemaphoreGive(tasklet->DtorLock);
      }

      /**
       *  Protect against accidental deletion before we were executed.
       */
      SemaphoreHandle_t DtorLock;

   protected:
      StaticSemaphore_t xSemaphoreBuffer;
   };

   /**
    * Delegate to a function/lambda etc.
    */
   struct Delegator
   {
      ///< Type of task handler
      using handler_t = void (*)();

      ///< Accessor to the handler
      virtual void invoke()
      {
         if (m_handler)
         {
            m_handler();
         }
         else
         {
            default_handler();
         }
      }

      virtual void default_handler() {}

   protected:
      /** Hold this function pointer */
      handler_t m_handler = nullptr;
   };

   /**
    * C Linkage entrypoint 
    * @param A function pointer to a lambda which calls the real function
    *         with proper arguments
    */
   extern "C" inline void trampoline(void *thisPtr)
   {
      // Grab this
      auto delegate = (Delegator *)thisPtr;
      delegate->invoke();

      // If the function returns - stop the task
      vTaskDelete(NULL);
   }

   /** 
    * Create a new static task.
    * The templated type must be unique for each task.
    * The TName parameter must use the 'typestring_is' macros which creates a literal string type.
    * This string makes the task unique.
    * The stack size is on top of the minimum recommended.
    * The priority defaults to normal.
    * The run method actually creates the FreeRTOS task and starts the task imediatly.
    * The run take a void (void) entrypoint. You can use lambda to pass other values
    */
   template <
       class TName,
       const size_t TStackSize = 0,
       const priority_t TPriority = rtos::priority_t::normal>
   class Task : Delegator
   {
      ///< The stack size is on top of freeRTOS minimum requirement
      static constexpr auto stack_size = configMINIMAL_STACK_SIZE + TStackSize;

      /** Structure that will hold the TCB of the task being created. */
      StaticTask_t taskBuffer;

      /** Local stack storage */
      StackType_t stack[stack_size];

      /** Handle to the task */
      TaskHandle_t handle;

   public:
      TaskHandle_t operator*()
      {
         return handle;
      }

      template <typename TLambda>
      void run(TLambda &&handler)
      {
         m_handler = reinterpret_cast<handler_t>(static_cast<handler_t>(handler));

         /* Create the task without using any dynamic memory allocation. */
         handle = xTaskCreateStatic(
             &trampoline,            /* Function that implements the task. */
             TName::data(),          /* Text name for the task. */
             stack_size,             /* Number of indexes in the xStack array. */
             (void *)this,           /* Parameter passed into the task. */
             (UBaseType_t)TPriority, /* Priority at which the task is created. */
             stack,                  /* Array to use as the task's stack. */
             &taskBuffer);           /* Variable to hold the task's data structure. */
      }

      void run()
      {
         /* Create the task without using any dynamic memory allocation. */
         handle = xTaskCreateStatic(
             &trampoline,            /* Function that implements the task. */
             TName::data(),          /* Text name for the task. */
             stack_size,             /* Number of indexes in the xStack array. */
             (void *)this,           /* Parameter passed into the task. */
             (UBaseType_t)TPriority, /* Priority at which the task is created. */
             stack,                  /* Array to use as the task's stack. */
             &taskBuffer);           /* Variable to hold the task's data structure. */
      }
   };

   ///< Start the scheduler and assert on exit
   static inline void start_scheduler()
   {
      vTaskStartScheduler();
      assert(0);
   }

   static inline void delay(tick_t ticks)
   {
      vTaskDelay(ticks);
   }

   /**
    * Static queue wrapper
    */
   template <
       typename T,
       const size_t TQueueLength = 8>
   class Queue
   {

      /** Structure that will hold the TCB of the task being created. */
      StaticQueue_t static_queue;

      /** Local stack storage */
      uint8_t queue_storage_area[TQueueLength * sizeof(T)];

      /** Handle to the queue */
      QueueHandle_t handle;

   public:
      QueueHandle_t operator*() { return handle; }

      Queue()
      {
         /* Create a queue capable of containing 10 uint64_t values. */
         handle = xQueueCreateStatic(TQueueLength, sizeof(T), queue_storage_area, &static_queue);
         assert(handle);
      }

      bool send(T &what, tick_t tickToWait = tick::infinite)
      {
         return (xQueueSend(handle, &what, tickToWait) == pdPASS) ? true : false;
      }

      bool receive(T &what, tick_t tickToWait = tick::infinite)
      {
         return (xQueueReceive(handle, &what, tickToWait) == pdPASS) ? true : false;
      }

      bool receive(T *what, tick_t tickToWait = tick::infinite)
      {
         return (xQueueReceive(handle, what, tickToWait) == pdPASS) ? true : false;
      }

      bool send_from_isr(T &what)
      {
         return (xQueueSendFromISR(handle, &what, NULL) == pdPASS) ? true : false;
      }

      bool receive_from_isr(T &what)
      {
         return (xQueueReceiveFromISR(handle, &what, NULL) == pdPASS) ? true : false;
      }
   };

   /**
    * Static message buffer
    */
   template <
       const size_t TBufferSize = 64>
   class MessageBuffer
   {

      /** Structure that will hold the TCB of the task being created. */
      StaticMessageBuffer_t static_message_buffer;

      /** Local stack storage */
      uint8_t storage_area[TBufferSize];

      /** Handle to the queue */
      MessageBufferHandle_t handle;

   public:
      MessageBufferHandle_t operator*() { return handle; }

      MessageBuffer()
      {
         /* Create a queue capable of containing 10 uint64_t values. */
         handle = xMessageBufferCreateStatic(TBufferSize, storage_area, &static_message_buffer);
         assert(handle);
      }

      bool empty() const
      {
         return xMessageBufferIsEmpty(handle) == pdTRUE;
      }

      bool send(void *what, size_t size, tick_t tickToWait = tick::infinite)
      {
         return (xMessageBufferSend(handle, size, what, tickToWait) == pdPASS) ? true : false;
      }

      template <typename T>
      bool send(T &what, tick_t tickToWait = tick::infinite)
      {
         return (xMessageBufferSend(handle, sizeof(T), &what, tickToWait) == pdPASS) ? true : false;
      }

      template <typename T>
      bool receive(T &what, tick_t tickToWait = tick::infinite)
      {
         return (xMessageBufferReceive(handle, sizeof(T), &what, tickToWait) == pdPASS) ? true : false;
      }

      template <typename T>
      bool send_from_isr(T &what)
      {
         return (xMessageBufferSendFromISR(handle, sizeof(T), &what, NULL) == pdPASS) ? true : false;
      }

      template <typename T>
      bool receive_from_isr(T &what)
      {
         return (xMessageBufferReceiveFromISR(handle, sizeof(T), &what, NULL) == pdPASS) ? true : false;
      }

      ///< Wait for something, but leave it in. The nextMsgSize contains the size of the message
      bool peek(size_t &nextMsgSize, tick_t tickToWait = tick::infinite)
      {
         nextMsgSize = xMessageBufferReceive(handle, nullptr, 0, tickToWait);

         return nextMsgSize > 0;
      }
   };

   /**
    *  Wrapper class around FreeRTOS's implementation of a timer.
    *
    *  This is an abstract base class.
    *  To use this, you need to subclass it. All of your timers should
    *  be derived from the Timer class. Then implement the virtual Run
    *  function. This is a similar design to Java threading.
    */

   template <class TName>
   class Timer
   {
   public:
      using delegate_t = etl::delegate<void()>;
      typedef void (*cb_t)();

   private:
      /**
      *  Reference to the underlying timer handle.
      */
      TimerHandle_t handle;
      StaticTimer_t pxTimerBuffer;
      //delegate_t cb;
      cb_t cb;

   public:
      Timer(tick_t PeriodInTicks, bool Periodic = true) : cb(nullptr)
      {
         handle = xTimerCreateStatic(TName::data(), PeriodInTicks, Periodic ? pdTRUE : pdFALSE, this, TimerCallbackFunctionAdapter, &pxTimerBuffer);

         if (handle == NULL)
         {
            configASSERT(!"Timer Constructor Failed");
         }
      }

      Timer(cb_t cb, tick_t PeriodInTicks, bool Periodic = true) : cb(cb)
      {
         handle = xTimerCreateStatic(TName::data(), PeriodInTicks, Periodic ? pdTRUE : pdFALSE, this, TimerCallbackFunctionAdapter, &pxTimerBuffer);

         if (handle == NULL)
         {
            configASSERT(!"Timer Constructor Failed");
         }
      }

      ~Timer()
      {
         xTimerDelete(handle, tick::infinite);
      }

      bool is_active()
      {
         return xTimerIsTimerActive(handle) == pdFALSE ? false : true;
      }

      bool start(tick_t CmdTimeout = tick::infinite)
      {
         return xTimerStart(handle, CmdTimeout) == pdFALSE ? false : true;
      }

      bool start_from_isr()
      {
         bool retval;
         BaseType_t *pxHigherPriorityTaskWoken;

         retval = xTimerStartFromISR(handle, pxHigherPriorityTaskWoken) == pdFALSE ? false : true;

         if (pxHigherPriorityTaskWoken)
         {
            portYIELD();
         }

         return retval;
      }

      bool stop(tick_t CmdTimeout = tick::infinite)
      {
         return xTimerStop(handle, CmdTimeout) == pdFALSE ? false : true;
      }

      bool stop_from_isr()
      {
         bool retval;
         BaseType_t *pxHigherPriorityTaskWoken;

         retval = xTimerStopFromISR(handle, pxHigherPriorityTaskWoken) == pdFALSE ? false : true;

         if (pxHigherPriorityTaskWoken)
         {
            portYIELD();
         }

         return retval;
      }

      bool reset(tick_t CmdTimeout = tick::infinite)
      {
         return xTimerReset(handle, CmdTimeout) == pdFALSE ? false : true;
      }

      bool reset_from_isr()
      {
         bool retval;
         BaseType_t *pxHigherPriorityTaskWoken;

         retval = xTimerResetFromISR(handle, pxHigherPriorityTaskWoken) == pdFALSE ? false : true;

         if (pxHigherPriorityTaskWoken)
         {
            portYIELD();
         }

         return retval;
      }

      bool set_period(tick_t NewPeriod, tick_t CmdTimeout = tick::infinite)
      {
         return xTimerChangePeriod(handle, NewPeriod, CmdTimeout) == pdFALSE ? false : true;
      }

      bool set_period_from_isr(tick_t NewPeriod)
      {
         bool retval;
         BaseType_t *pxHigherPriorityTaskWoken;

         retval = xTimerChangePeriodFromISR(handle, NewPeriod, pxHigherPriorityTaskWoken) == pdFALSE ? false : true;

         if (pxHigherPriorityTaskWoken)
         {
            portYIELD();
         }

         return retval;
      }

   protected:
      cb_t get_callback()
      {
         return cb;
      }

      static void TimerCallbackFunctionAdapter(TimerHandle_t xTimer)
      {
         Timer *timer = static_cast<Timer *>(pvTimerGetTimerID(xTimer));

         cb_t cb = timer->get_callback();

         if (cb != nullptr)
         {
            cb();
         }
         else
         {
            timer->run();
         }
      }

   protected:
      virtual void run()
      {
         // No overload
         assert(0);
      }
   };
}

#endif /* RTOS_H_ */