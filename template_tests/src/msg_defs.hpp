#ifndef msg_defs_hpp__included
#define msg_defs_hpp__included
/*
 * Fx messages
 * Created: 04/07/2021 14:35:24
 * Author: software@arreckx.com
 */
#include "etl/message.h"

namespace msg
{
   // All message require a unique ID
   enum : etl::message_id_t
   {
      NO_NC_UPDATE,
      SET_RELAY,
      SHOW_ACTIVITY,
      KEYPAD_EVENT,
      CDC_CHAR_EVENT,
      END_OF_SPLASH,
      COUNTER_UPDATE,
      START_PGM,
      STOP_PGM,
      USB_CONNECTED,
      USB_DISCONNECTED,
      NEXT_SEQUENCE,
      CONTACT_UPDATE,
   };

   // Message types
   struct NoNcUpdate          : etl::message<NO_NC_UPDATE>     {};
   struct ContactUpdate       : etl::message<CONTACT_UPDATE>   {};
   struct SetRelay            : etl::message<SET_RELAY>        { bool invert; };
   struct ShowActivity        : etl::message<SHOW_ACTIVITY>    {};
   struct Keypad              : etl::message<KEYPAD_EVENT>     { uint8_t key_code; };
   struct CDCChar             : etl::message<CDC_CHAR_EVENT>   { char c; };
   struct EndOfSplash         : etl::message<END_OF_SPLASH>    {};
   struct CounterUpdate       : etl::message<COUNTER_UPDATE>   {};
   struct StartProgram        : etl::message<START_PGM>        {};
   struct StopProgram         : etl::message<STOP_PGM>         {};
   struct USBConnected        : etl::message<USB_CONNECTED>    {};
   struct USBDisconnected     : etl::message<USB_DISCONNECTED> {};
   struct SequenceNext        : etl::message<NEXT_SEQUENCE>    {};

   // Create a universal message type
   using ui_packet_t = etl::message_packet<
      EndOfSplash,
      Keypad,
      NoNcUpdate,
      ContactUpdate      
   >;
   
   using console_packet_t = etl::message_packet<
      CDCChar
   >;
   
   using sequencer_packet_t = etl::message_packet<
      NoNcUpdate   
   >;
}

#endif // ndef msg_defs_hpp__included