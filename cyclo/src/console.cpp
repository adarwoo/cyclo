/*
 * console_task.cpp
 *
 * Created: 28/08/2021 20:27:07
 *  Author: micro
 */
#include "console.hpp"

#include "asx.h"
#include "parser.hpp"
#include "trace.h"

#include <fx.hpp>

#include <logger.h>

#include "console_server.hpp"
#include "msg_defs.hpp"
#include "program_manager.hpp"

using rtos::tick_t;

// USB static management
namespace
{
   const char *const DOM = "console";

   auto cdc_available_semaphore = rtos::BinarySemaphore{};
   bool cdc_transfert_allowed{ false };
   bool first_time{ true };
   bool usb_mode{ false };
}  // namespace

//
// Hooks for the CDC interface
//
extern "C" bool console_cdc_enabled( uint8_t port )
{
   UNUSED( port );
   cdc_transfert_allowed = true;
   cdc_available_semaphore.give_from_isr( nullptr );
   trace_set( TRACE_INFO );

   return true;
}

extern "C" void console_cdc_disabled( uint8_t port )
{
   UNUSED( port );
   cdc_transfert_allowed = false;
   trace_clear( TRACE_INFO );
}

void console_putc( vt100::char_t c )
{
   udi_cdc_multi_putc( 0, (int)c );
}

/** Create the timer for the splash */
Console::Console( ProgramManager &program_manager )
   : parser{ temp_program, error_buffer }, program_manager{ program_manager }
{
   // Start the thread
   this->run();
}

void Console::show_error()
{
   using T = TTerminal;

   uint8_t err_position = parser.get_error_position();

   // Display the error message
   T::putc( '#' );

   // Insert spaces
   for ( uint8_t i = 0; i <= err_position; ++i )
   {
      T::putc( ' ' );
   }

   T::putc( '^' );
   T::move_to_start_of_next_line();
   print_error( error_buffer.c_str() );
}

void Console::print_error( const char *error )
{
   using T = TTerminal;

   T::putc( '#' );
   T::putc( ' ' );
   T::puts( error );
   T::move_to_start_of_next_line();
}

/**
 * The task entry point
 */
void Console::default_handler()
{
   using T = TTerminal;

   while ( true )
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
         T::print_P( PSTR( "# Welcome to Cyclo! Type 'help' for help.\r\n" ) );
         T::move_to_start_of_next_line();
         first_time = false;
      }

      server.print_prompt();

      while ( ! v )
      {
         v = server.process_input( (vt100::char_t)udi_cdc_getc() );

         rtos::delay( 1 );

         if ( ! cdc_transfert_allowed )
         {
            break;
         }
      }

      // Process the line
      if ( v )
      {
         process( *v );
      }
   }
}

void Console::process( etl::string_view line )
{
   auto res = parser.parse( line );

   switch ( res )
   {
   case Parser::Result::nothing: break;
   case Parser::Result::error: show_error(); break;
   case Parser::Result::program:
      if ( ! usb_mode )
      {
         usb_mode = true;
         fx::publish( msg::USBConnected{} );
      }

      program_manager.load( temp_program );
      // Copy this program, so it can be saved as is
      last_program.assign(line.begin(), line.end());
      break;
   case Parser::Result::help: show_help(); break;
   case Parser::Result::list:
      program_manager.scan();
      show_list();
      break;
   case Parser::Result::quit:
      usb_mode = false;
      fx::publish( msg::USBDisconnected{} );

      break;
   case Parser::Result::del: program_manager.erase( parser.get_program_number() ); break;
   case Parser::Result::run: program_manager.load( parser.get_program_number() ); break;
   case Parser::Result::save:
      if ( last_program.empty() )
      {
         print_error("No valid program to save");
      }
      else
      {
         program_manager.write_pgm_at( parser.get_program_number(), last_program );
      }

      break;
   case Parser::Result::autostart:
      // Revoke current autostart
      // Mark autostart
      program_manager.set_autostart( parser.get_program_number() );
      break;
   default: LOG_ERROR( DOM, "Unexpected" );
   }
}

void Console::show_help()
{
   auto help = PSTR(
      "Commands are separated with spaces. All or just 1 letters can be used.\r\n"
      "\r\n"
      "Basic commands:\r\n"
      "  open  .. Opens the contact\r\n"
      "  close .. Closes the contact\r\n"
      "\r\n"
      "A valid program is a series of open/close with some delay added.\r\n"
      "A minimum delay of 1 second is automatically added if none is specified.\r\n"
      "Delays can have units (no units indicate seconds) as:\r\n"
      "  H .. hour\r\n"
      "  M .. minutes\r\n"
      "  s .. seconds\r\n"
      "  m .. milliseconds\r\n"
      "\r\n"
      ">> Example:\r\n"
      "  o 1 500m c 1H 30M\r\n"
      "To loop a sequence, add '*' at the end.\r\n"
      "  close 250m open 10 c o *\r\n"
      "\r\n"
      "Other commands:\r\n"
      "  list       : List saved programs\n\r"
      "  save [1-9] : Save the last valid program\r\n"
      "  del [1-9]  : Delete the program at the given location\r\n"
      "  run [0-9]  : Run the given program\r\n"
      "  auto [0-9] : Start the program automatically on power-up\r\n"
      "  reset      : Clear the program memory\r\n"
      "  quit       : Leave this shell and re-enable manual mode\r\n"
      "Fast run:\r\n"
      "  [0-9] <*>  : Type a valid program number to run it.\r\n"
      "               Add a '*' to loop it. Ignored for looped programs.\r\n");

   TTerminal::print_P( help );
}

void Console::show_list()
{
   // Iterate the bitset
   uint8_t old, index = 0;

   do
   {
      old   = index;
      index = program_manager.get_next( index );
      // Print the index
      TTerminal::move_forward( 2 );
      TTerminal::putc( '0' + old );
      TTerminal::putc( ':' );
      TTerminal::move_forward( 4 );
      TTerminal::puts( program_manager.get_pgm( old ) );
      TTerminal::move_to_start_of_next_line();
   } while ( old != index );
}
