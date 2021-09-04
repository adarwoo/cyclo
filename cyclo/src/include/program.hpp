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
#ifndef command_hpp_was_included
#define command_hpp_was_included
/*
 * Defines the command sequence from parsed commands
 * Used by the parser, but also by the model and the sequencer
 *
 * Created: 06/08/2021 23:15:28
 *  Author: software@arreckx.com
 */
#include <cstdint>

#include <etl/vector.h>

#include "conf_cyclo.hpp"


/**
 * A single command item to execute.
 * It carries the operation as well as a delay.
 * The delay is to be observed after the command
 */
struct Command
{
   ///< Program for the engine
   enum command_t : char { open = 'o', close = 'c', delay = 'd', loop = 'l' } command;

   ///< Delay after the command execution
   uint32_t delay_ms;

   ///< Simple constructor
   explicit Command( command_t type, uint32_t delay = 0 ) : command{ type }, delay_ms{ delay }
   {}
};


/**
 * Holds a complete program already parsed
 */
class Program : public etl::vector<Command, cyclo::max_items_per_command>
{
   // Easy iterator
   const_iterator it;

public:
   // Program
   Program() : it{ begin() } {}

   /**
    * Get the next item. This is the first item following a reset.
    * Automatically loops the sequence for looped commands
    * @returns A pointer to the next command to execute or nullptr if
    *           the command has ended.
    */
   auto iterate()
   {
      const Command *retval = nullptr;

      if ( it != end() )
      {
         retval = &( *it++ );
      }

      return retval;
   }

   void start() { it = begin(); }
};


#endif  // ndef command_hpp_was_included