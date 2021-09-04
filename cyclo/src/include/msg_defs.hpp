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
#ifndef msg_defs_hpp_included
#define msg_defs_hpp_included
/**
 * Defines all the fx messages
 */
#include <fx.hpp>
#include <rtos.hpp>

namespace msg
{
   // Message types
   FX_MSG( NoNcUpdate ){};
   FX_MSG( ContactUpdate ){};
   FX_MSG( ShowActivity ){};
   FX_MSG( Keypad ) { uint8_t key_code; };
   FX_MSG( EndOfSplash ){};
   FX_MSG( CounterUpdate ){};
   FX_MSG( StartProgram )
   {
      bool from_start;
   public:
       StartProgram( bool r ) : from_start{ r } {}
   };
   FX_MSG( StopProgram ){};
   FX_MSG( ProgramIsStopped ){};
   FX_MSG( USBConnected ){};
   FX_MSG( USBDisconnected ){};
   FX_MSG( SequenceNext ){};

   // Create a universal message type
   using packet_t = etl::message_packet<
      EndOfSplash,
      Keypad,
      NoNcUpdate,
      ContactUpdate,
      CounterUpdate,
      StartProgram,
      StopProgram,
      ProgramIsStopped,
      SequenceNext,
      USBConnected,
      USBDisconnected>;
}  // namespace msg

#endif  // ndef msg_defs_hpp_included