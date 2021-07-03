/*
 * task.h
 *
 * Created: 28/06/2021 10:36:35
 *  Author: micro
 */ 


#ifndef TASK_H_
#define TASK_H_

#include <FreeRTOS.h>
#include <task.h>

#include <etl/delegate.h>


enum class priority_t : UBaseType_t
{
   idle = tskIDLE_PRIORITY,
   normal,
   high
};

   /** Trampoline entry point for C */
   static extern "C" entrypoint(Task *arg)
   {
      EntryPoint(arg);
   }


template<
   typename TName,
   const size_t TStackSize=255,
   const priority_t TPriority=priority_t::normal,
> 
class Task
{
   constexpr stack_size = configMINIMAL_STACK_SIZE + TStackSize;
   
   /** Structure that will hold the TCB of the task being created. */
   StaticTask_t taskBuffer;

   /** Local stack storage */
   StackType_t stack[ STACK_SIZE ];

   /** Handle to the task */   
   TaskHandle_t handle_;
  
public:
   TaskHandle_t operator *()
   {
      return handle_;
   }
   
   static constexpr const char *get_name() const
   { return TName::name(); }
  
   
   Task() : handle_(nullptr)
   {
   }
   
   template< class Function, class... Args >
   void start( Function&& f, Args&&... args )
   {
        /* Create the task without using any dynamic memory allocation. */
        handle_ = xTaskCreateStatic(
           [](void *) {this->},    /* Function that implements the task. */
           TName.c_str(),    /* Text name for the task. */
           TStackSize,       /* Number of indexes in the xStack array. */
           ( void * ) arg,   /* Parameter passed into the task. */
           TPriority,        /* Priority at which the task is created. */
           stack,            /* Array to use as the task's stack. */
           &taskBuffer );    /* Variable to hold the task's data structure. */
   }
};


#endif /* TASK_H_ */