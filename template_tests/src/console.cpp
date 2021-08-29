/*
 * console_task.cpp
 *
 * Created: 28/08/2021 20:27:07
 *  Author: micro
 */ 
#include <fx.hpp>

#include "console_server.hpp"
#include "program_manager.hpp"
#include "parser.hpp"
#include "console.hpp"
#include "msg_defs.hpp"
#include "trace.h"

using rtos::tick_t;

namespace
{
   auto cdc_available_semaphore = rtos::BinarySemaphore{};
   bool cdc_transfert_allowed{false};
   bool first_time{true};
   bool usb_mode{false};
}

//
// Hooks for the CDC interface
//
extern "C" bool console_cdc_enabled(uint8_t port)
{
   UNUSED(port);
   cdc_transfert_allowed = true;
   cdc_available_semaphore.give_from_isr(nullptr);
   trace_set(TRACE_INFO);
   
   return true;
}

extern "C" void console_cdc_disabled(uint8_t port) 
{
   UNUSED(port);
   cdc_transfert_allowed = false;
   trace_clear(TRACE_INFO);
}

void console_putc(vt100::char_t c)
{
   udi_cdc_multi_putc(0, (int)c);
}

/** Create the timer for the splash */
Console::Console(ProgramManager &program_manager) : program_manager(program_manager)
{
   // Start the thread
   this->run();
}

/**
 * The console is requesting interaction
 */
void Console::interact(Interact i)
{
   
}

void Console::show_error()
{
   using T = TTerminal;

   uint_least8_t err_position;
   auto err_message = parser.get_error(err_position);
            
   // Display the error message
   T::putc('#');
   
   // Insert spaces         
   for (uint8_t i=0; i<=err_position; ++i)
   {
      T::putc(' ');
   }               
   
   T::putc('^');
   T::move_to_start_of_next_line();
   T::putc('#');
   T::putc(' ');
   T::puts(err_message);            
   T::move_to_start_of_next_line();
}


/**
 * The task entry point
 */
void Console::default_handler()
{
   using T = TTerminal;
   
   while (true)
   {
      optional_buffer_view_t v;
      
      // Wait for the cdc to become available
      if ( ! cdc_transfert_allowed )
      {
         server.reset();
         cdc_available_semaphore.take();
      }

      if ( first_time )
      {
         T::print_P( PSTR("# Welcome to Cyclo! Type 'help' for help.\r\n"));
         T::move_to_start_of_next_line();
         first_time = false;
      }

      server.print_prompt();
      
      while ( ! v )
      {
         v = server.process_input((vt100::char_t)udi_cdc_getc());
         
         rtos::delay(1);

         if ( ! cdc_transfert_allowed )
         {
            break;
         }
      }
      
      // Process the line
      if ( v )
      {
         bool valid = parser.parse(*v);
         
         if ( valid )
         {
            if ( ! usb_mode )
            {
               // TODO -> Only if command mode (interactive  does not trigger usb)
               // First valid command - disable manual mode
               usb_mode = true;
               fx::publish(msg::USBConnected{});
            }
            // Execute with the sequencer
           
            T::print_P( PSTR("# Good!"));
            T::move_to_start_of_next_line();
         }
         else
         {
            show_error();
         }
      }         
   }
}
