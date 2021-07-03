/*
 * Example application running on the Cyclo project
 *
 * Created: 29/06/2021 21:26:21
 * Author : software@arreckx.com
 */ 
#include "rtos.hpp"
#include "trace.h"
#include "keypad.h"


/** Callback called when a key is pushed */
typedef void (*keypad_handler_t)(uint8_t key, bool repeat);

auto kq = rtos::Queue<uint8_t>();

void key_cb(uint8_t k, bool longPush)
   { kq.send_from_isr(k); }
      
void handle_key_task()
{
   using namespace rtos::tick;
   uint8_t key;
   
   for (;;)
   {
      ioport_pin_t pin;

      kq.receive(key);
      
      switch (key)
      {
         case KEY_UP: pin=TRACE_IDLE; break;
         case KEY_DOWN: pin=TRACE_TICK; break;
         case KEY_SELECT: pin=TRACE_ERR; break;
         default:
            assert(0);
      }
      
      trace_set(pin);
      rtos::delay(100_ms);
      trace_clear(pin);
   }
}


int main(void)
{
//   auto led1_task = rtos::Task<typestring_is("led1")>();
//   auto led2_task = rtos::Task<typestring_is("led2")>();
   auto key_task = rtos::Task<typestring_is("key")>();
   
   board_init();
   keypad_init();
   
   keypad_register_callback(KEY_UP | KEY_DOWN | KEY_SELECT, key_cb);
  
   //led1_task.run([]() {for(;;) trace_tgl(TRACE_IDLE);});
   //led2_task.run([]() {for(;;) trace_tgl(TRACE_TICK);});
   key_task.run(handle_key_task);

   rtos::start_scheduler();
}
