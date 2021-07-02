/*
 * rtos.h
 *
 * Created: 01/07/2021 12:19:42
 *  Author: micro
 */ 


#ifndef RTOS_H_
#define RTOS_H_


#include "FreeRTOS.h"
#include "task.h"

#include "etl/algorithm.h"
#include "etl/delegate.h"

#include "typestring.hh"

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
   
}


#endif /* RTOS_H_ */