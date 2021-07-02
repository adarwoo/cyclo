/*
 * Example application running on the Cyclo project
 *
 * Created: 29/06/2021 21:26:21
 * Author : software@arreckx.com
 */ 
#include "rtos.hpp"
#include "trace.h"


int main(void)
{
   auto led1_task = rtos::Task<typestring_is("led1")>();
   auto led2_task = rtos::Task<typestring_is("led2")>();

   board_init();
  
   led1_task.run([]() {for(;;) trace_tgl(TRACE_IDLE);});
   led2_task.run([]() {for(;;) trace_tgl(TRACE_TICK);});

   rtos::start_scheduler();
}
