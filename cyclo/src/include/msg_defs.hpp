#ifndef msg_defs_hpp__included
#define msg_defs_hpp__included
/*
 * Fx messages
 * Created: 04/07/2021 14:35:24
 * Author: software@arreckx.com
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
      // TODO -> Need to make sure no race possible between sending and copying rtos::BinarySemaphore &read;
   public:
      //StartProgram( bool r, rtos::BinarySemaphore &read ) : from_start{ r }, read {read} {}
       StartProgram( bool r ) : from_start{ r } {}
      //void ack() { read.give(); }
      //void wait_for_ack() { read.take(); }
   };
   FX_MSG( StopProgram ){};
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
      SequenceNext,
      USBConnected>;
}  // namespace msg

#endif  // ndef msg_defs_hpp__included