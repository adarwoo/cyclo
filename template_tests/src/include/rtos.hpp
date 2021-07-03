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
  
   namespace tick
   {
      constexpr tick_t infinite = portMAX_DELAY;
      constexpr tick_t per_second = configTICK_RATE_HZ;
      
      constexpr tick_t operator"" _ms ( unsigned long long milliseconds )
         { return static_cast<tick_t>((milliseconds * per_second) / 1000); }

      constexpr tick_t operator"" _s(unsigned long long seconds)
         { return static_cast<tick_t>(per_second * seconds); }

      constexpr tick_t operator"" _M(unsigned long long minutes)
         { return static_cast<tick_t>(per_second * 60 * minutes); }
   }


   /**
    * Delegate to a function/lambda etc.
    */
   struct Delegator
   {
      ///< Type of task handler
      using handler_t = void (*)();
      
      ///< Accessor to the handler
      void invoke() { assert(m_handler); m_handler(); }
      
   protected:      
      /** Hold this function pointer */
      handler_t m_handler = nullptr;
   };
   

   /**
    * C Linkage entrypoint 
    * @param A function pointer to a lambda which calls the real function
    *         with proper arguments
    */
   extern "C" void trampoline(void *thisPtr)
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
   template<
      class TName,
      const size_t TStackSize=0,
      const priority_t TPriority = rtos::priority_t::normal
   >
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
      TaskHandle_t operator *()
      {
         return handle;
      }

      template<typename TLambda>
      void run(TLambda&& handler)
      {
         m_handler = reinterpret_cast<handler_t>(static_cast<handler_t>(handler));
     
         /* Create the task without using any dynamic memory allocation. */
         handle = xTaskCreateStatic(
            &trampoline,      /* Function that implements the task. */
            TName::data(),    /* Text name for the task. */
            stack_size,       /* Number of indexes in the xStack array. */
            (void *)this,      /* Parameter passed into the task. */
            (UBaseType_t)TPriority,        /* Priority at which the task is created. */
            stack,            /* Array to use as the task's stack. */
            &taskBuffer );    /* Variable to hold the task's data structure. */
        }
   };
   
   ///< Start the scheduler and assert on exit
   static void start_scheduler()
   {
      vTaskStartScheduler();
      assert(0);
   }

   static void delay(tick_t ticks)
      { vTaskDelay(ticks); }
   
   /**
    * Static queue wrapper
    */
   template<
      typename T,
      const size_t TQueueLength=8
   >
   class Queue
   {
      
      /** Structure that will hold the TCB of the task being created. */
      StaticQueue_t static_queue;

      /** Local stack storage */
      uint8_t queue_storage_area[TQueueLength * sizeof(T)];
      
      /** Handle to the queue */
      QueueHandle_t handle;
    
   public:
      QueueHandle_t operator *() { return handle; }

      Queue() {
         /* Create a queue capable of containing 10 uint64_t values. */
         handle = xQueueCreateStatic( TQueueLength, sizeof(T), queue_storage_area, &static_queue );
         assert( handle );
      }
      
      bool send(T &what, tick_t tickToWait = tick::infinite )
         { return (xQueueSend(handle, &what, tickToWait) == pdPASS) ? true : false; }

      bool receive(T &what, tick_t tickToWait = tick::infinite )
         { return (xQueueReceive(handle, &what, tickToWait) == pdPASS) ? true : false; }

      bool send_from_isr(T &what)
         { return (xQueueSendFromISR(handle, &what, NULL) == pdPASS) ? true : false; }

      bool receive_from_isr(T &what)
         { return (xQueueReceiveFromISR(handle, &what, NULL) == pdPASS) ? true : false; }
            
      inline T& operator<<(T& what)
         { receive(what); return what; }

      inline T& operator>>(T& what)
         { send(what); return what; }
   };
}


#endif /* RTOS_H_ */