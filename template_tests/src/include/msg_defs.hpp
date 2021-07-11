#ifndef msg_defs_hpp__included
#define msg_defs_hpp__included
/*
 * User interface message router
 * Created: 04/07/2021 14:35:24
 *  Author: micro
 */ 
#include "etl/message.h"

#include "typestring.hpp"


namespace msg
{
   enum : etl::message_id_t
   {
      NO_NC_UPDATE,
      SET_RELAY,
      REFRESH_UI,
      KEYPAD_EVENT
   };

   struct Keypad : etl::message<KEYPAD_EVENT>
   {
      uint8_t key_code;
   };

   struct NoNcUpdate : etl::message<NO_NC_UPDATE>
   {
      // No data. The value is atomic - better read it from the source
   };

   struct RefreshUI : etl::message<REFRESH_UI>
   {
      // Command message. No data
   };

   struct SetRelay : etl::message<SET_RELAY>
   {
      ///< Turn no of off (irrespective of the NO/NC status. True to turn ON.
      bool turn_on;
   };
   
   using packet = etl::message_packet<Keypad, NoNcUpdate, RefreshUI, SetRelay>;

   namespace router
   {
      enum : etl::message_router_id_t
      {
         UI,
         TERMINAL,
         SEQUENCER,
      };  
   }
}


#endif // ndef msg_defs_hpp__included