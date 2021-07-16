#ifndef msg_defs_hpp__included
#define msg_defs_hpp__included
/*
 * Fx messages
 * Created: 04/07/2021 14:35:24
 *  Author: micro
 */
#include "etl/message.h"

namespace msg
{
   enum : etl::message_id_t
   {
      NO_NC_UPDATE,
      SET_RELAY,
      REFRESH_UI,
      KEYPAD_EVENT,
      CDC_CHAR_EVENT,
      END_OF_SPLASH
   };

   struct Keypad : etl::message<KEYPAD_EVENT>
   {
      uint8_t key_code;
   };

   struct NoNcUpdate : etl::message<NO_NC_UPDATE>
   {
      bool is_no;
   };

   struct SetRelay : etl::message<SET_RELAY>
   {
      ///< Turn no of off (irrespective of the NO/NC status. True to turn ON.
      bool turn_on;
   };
   
   struct EndOfSplash : etl::message<END_OF_SPLASH>
   {
   };

   struct CDCChar : etl::message<CDC_CHAR_EVENT>
   {
      char c;
   };

   using packet = etl::message_packet<EndOfSplash, Keypad, NoNcUpdate, SetRelay>;
  
   using cdc = etl::message_packet<CDCChar>;
}

#endif // ndef msg_defs_hpp__included