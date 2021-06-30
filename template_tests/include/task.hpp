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

namespace taskname
{
   template <char... Chrs>
   struct string<char, Chrs...> {
      using type = string;
      static auto c_str() {
         static constexpr char str[] = {Chrs..., 0};
         return str;
      }
   };

   template <class T>
   struct string<T> {
      using type = T;
      static auto c_str() { return c_str_impl((T *)0); }
      template <class U>
      static decltype(U::c_str()) c_str_impl(U *) {
         return U::c_str();
      }
      static auto c_str_impl(...) { return get_type_name<T>(); }
   };
}

template <class T, T... Chrs>
constexpr auto operator""_taskname() {
   return string<T, Chrs...>{};
}


template<
   typename TName,
   const size_t TStackSize=255,
   const priority_t TPriority=priority_t::normal,
   typename TArg = void
> 
class Task
{
   constexpr stack_size = configMINIMAL_STACK_SIZE + TStackSize;
   using RunCb_t = etl::delegate<void(typename TArg *)>;
   
   /** Structure that will hold the TCB of the task being created. */
   StaticTask_t taskBuffer;

   /** Local stack storage */
   StackType_t stack[ STACK_SIZE ];

   /** Handle to the task */   
   TaskHandle_t handle_;
   
   /** Store the parameter arg */
   TArg *arg_ptr_;
      
   /** Trampoline entry point for C */
   static extern "C" entrypoint(Task *arg)
   {
      EntryPoint(arg);
   }
   

public:
   TaskHandle_t operator *()
   {
      return handle_;
   }
   
   template< class Function, class... Args >
   explicit Task( Function&& f, Args&&... args );   
   
   Task() : handle_(nullptr)
   {
   }
   
   void start(TArg *arg) 
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