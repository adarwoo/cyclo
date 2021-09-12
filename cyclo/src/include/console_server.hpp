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
#ifndef console_server_hpp_was_included
#define console_server_hpp_was_included
/**
 * Class to act as a Terminal server.
 * The physical input/output must be provided.
 * This terminal server supports a VT100 terminal, and provides history, delete, insertion etc.
 */

#include "logger.h"

#include <cassert>
#include <cstdint>
#include <cstdio>

// include project-specific configuration
#include "vt100.hpp"

#include <etl/circular_buffer.h>
#include <etl/optional.h>
#include <etl/string.h>
#include <etl/string_view.h>

//
// Constant definitions
//

template<typename TTerminal, const size_t TBufferSize = 40, const size_t THistorySize = 10>
class ConsoleServer
{
public:
   /** Native char type */
   using char_t = vt100::char_t;

   /** String type to hold a single line */
   using buffer_t = etl::string<TBufferSize>;

   /** Returned optional buffer */
   using optional_buffer_view_t = etl::optional<etl::string_view>;

protected:
   /** History stack */
   using history_t = etl::circular_buffer<buffer_t, THistorySize>;

   /** Control of the history */
   enum class history_e : uint8_t {
      save,
      prev,
      next,
   };

   /** VT100 input state management */
   enum class input_state_t : uint8_t { normal, esc, cmd };

protected:
   /** Input buffer storage */
   buffer_t input_buffer;

   /** input_buffer_position within the current line */
   typename buffer_t::iterator input_buffer_position;

   /** Holds the history */
   history_t history_buffer;

   /** Hold the current input_buffer_position in the history */
   typename history_t::iterator history_position;

   /** State of the input for control character */
   input_state_t input_state;

   /** To cope with CR and LF in different shells, skip if the char is this value */
   char_t skip_first_if;

public:
   ConsoleServer()
      : input_buffer_position( input_buffer.begin() )
      , history_position()
      , input_state( input_state_t::normal )
      , skip_first_if( '\0' )
   {}

   virtual void print_prompt() { TTerminal::print_P( PSTR( "> " ) ); }

   etl::string_view get_line() { return etl::string_view( history_buffer.back() ); }

   /**
    * Reset the internal state machine if the input is interrupted
    */
   void reset()
   {
      input_state = input_state_t::normal;
      input_buffer.clear();
      skip_first_if = 0;
   }

   // process the received character
   optional_buffer_view_t process_input( char_t c )
   {
      etl::optional<etl::string_view> retval;

      // Once - Skip a LF following a CR or vice versa
      if ( c == skip_first_if )
      {
         // Reset
         c = '\0';
         return retval;
      }

      // Be deterministic and reset
      skip_first_if = '\0';

      // Are we processing a VT100 command?
      switch ( input_state )
      {
      case input_state_t::cmd:
         // We have already received ESC and [ - now process the vt100 code
         switch ( c )
         {
         case vt100::arrow::up: do_history( history_e::prev ); break;
         case vt100::arrow::down: do_history( history_e::next ); break;
         case vt100::arrow::right:
            // if the edit input_buffer_position less than current string length
            if ( input_buffer_position < input_buffer.end() )
            {
               ++input_buffer_position;
               TTerminal::move_forward();
            }
            else
            {
               // else, ring the bell
               TTerminal::putc( ascii::bel );
            }
            break;
         case vt100::arrow::left:
            // if the edit input_buffer_position is non-zero
            if ( input_buffer_position != input_buffer.begin() )
            {
               --input_buffer_position;
               TTerminal::move_back();
            }
            else
            {
               TTerminal::ring_bell();
            }
            break;
         case vt100::arrow::home:
            // if the edit input_buffer_position is non-zero
            if ( input_buffer_position != input_buffer.begin() )
            {
               TTerminal::move_back( input_buffer_position - input_buffer.begin() );
               input_buffer_position = input_buffer.begin();
            }
            else
            {
               TTerminal::ring_bell();
            }
            break;
         case vt100::arrow::end:
            // if the edit input_buffer_position is non-zero
            if ( input_buffer_position != input_buffer.end() )
            {
               TTerminal::move_forward( input_buffer.end() - input_buffer_position );
               input_buffer_position = input_buffer.end();
            }
            else
            {
               TTerminal::ring_bell();
            }
            break;
         default: break;
         }

         // done, reset state
         input_state = input_state_t::normal;

         return retval;
      case input_state_t::esc:
         // we last received [ESC]
         if ( c == '[' )
         {
            input_state = input_state_t::cmd;
            return retval;
         }
         else
         {
            input_state = input_state_t::normal;
         }
         break;
      default:
         // anything else, reset state
         input_state = input_state_t::normal;
      }

      // Regular handling
      if ( ( c >= '\x20' ) && ( c < '\x7f' ) )
      {
         // character is printable
         // is this a simple append
         if ( input_buffer.available() )
         {
            if ( input_buffer_position == input_buffer.end() )
            {
               // echo character to the output
               TTerminal::putc( c );

               // add it to the command line buffer
               input_buffer.push_back( c );

               // Move the position along
               ++input_buffer_position;
            }
            else
            {
               // edit/cursor input_buffer_position != end of buffer
               // we're inserting characters at a mid-line edit input_buffer_position
               // make room at the insert point
               input_buffer.insert( input_buffer_position, c );

               // Move the position along
               ++input_buffer_position;

               // repaint
               repaint();

               // reposition cursor
               TTerminal::move_back( input_buffer.end() - input_buffer_position );
            }
         }
         else
         {
            TTerminal::ring_bell();
         }
      }
      // handle special characters - LineFeed or CarriageReturn?
      else if ( c == ascii::lf or c == ascii::cr )
      {
         // user pressed [ENTER] - echo CR and LF to terminal
         TTerminal::move_to_start_of_next_line();

         // Skip the incomming opposite char
         skip_first_if = c == ascii::lf ? c == ascii::cr : ascii::lf;

         if ( not input_buffer.empty() )
         {
            // Store the last (non-empty) line in the history
            do_history( history_e::save );
            retval = get_line();
         }
         else
         {
            // Return always - even an empty string
            retval = etl::string_view();
         }

         // reset buffer
         return retval;
      }
      else if ( c == ascii::del or c == ascii::bs )
      {
         if ( input_buffer_position == input_buffer.begin() )
         {
            TTerminal::ring_bell();
         }
         else
         {
            // is this a simple delete (off the end of the line)
            if ( input_buffer_position == input_buffer.end() )
            {
               TTerminal::move_back_with_erase();

               // Pop the last char
               input_buffer.pop_back();

               // decrement our buffer length and edit input_buffer_position
               input_buffer_position = input_buffer.end();
            }
            else
            {
               // edit/cursor input_buffer_position != end of buffer
               input_buffer.erase( --input_buffer_position );

               // repaint
               repaint();

               // add space to clear leftover characters
               TTerminal::putc( ' ' );

               // reposition cursor
               TTerminal::move_back( input_buffer.end() - input_buffer_position + 1 /*blank*/ );
            }
         }
      }
      else if ( c == ascii::esc )
      {
         input_state = input_state_t::esc;
      }

      return retval;
   }

protected:
   void repaint( size_t erase_up_to = 0 )
   {
      TTerminal::move_to_start();

      // print fresh prompt
      print_prompt();

      // print the new command line buffer
      for ( auto c : input_buffer )
      {
         TTerminal::putc( c );
      }

      // Erase end of line
      if ( erase_up_to > input_buffer.size() )
      {
         TTerminal::erase_right( erase_up_to - input_buffer.size() );
      }
   }

   void do_history( history_e action )
   {
      if ( action == history_e::save )
      {
         // Don't add if already in the buffer - rather move last
         // TODO
         // auto it = history_buffer.find(input_buffer);
         // if ( it == history_buffer.end() )
         // Move history back

         // Save
         history_buffer.push( input_buffer );

         // Points to the next available slot
         history_position = history_buffer.end();

         // Reset the buffer
         input_buffer.clear();

         // Reset the history position
         input_buffer_position = input_buffer.begin();
      }
      else
      {
         if ( action == history_e::next )
         {
            if ( history_position != history_buffer.end() )
            {
               ++history_position;
            }
            else
            {
               TTerminal::ring_bell();
               return;
            }
         }
         else  // prev
         {
            if ( history_position != history_buffer.begin() )
            {
               --history_position;
            }
            else
            {
               TTerminal::ring_bell();
               return;
            }
         }

         // Grab the number of characters to erase
         size_t char_erase_count = input_buffer.size();

         // Copy the content into the buffer
         input_buffer = *history_position;

         // Repaint and move cursor to the end
         input_buffer_position = input_buffer.end();
         repaint( char_erase_count );
      }
   }
};

#endif  // ndef console_server_hpp_was_included