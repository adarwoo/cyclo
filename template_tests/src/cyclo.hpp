#ifndef cyclo_header_included
#define cyclo_header_included
/*
 * cyclo.cpp
 *
 * Created: 17/07/2021 16:11:18
 *  Author: micro
 */ 
#include <cstdint>
#include <etl/atomic.h>
#include <etl/optional.h>

#include "conf_cyclo.hpp"

namespace cyclo
{
   /** The current program selection */  
   enum pgm_t : uint8_t
   {
      P_USB=0,
      P_1,
      P_2,
      P_3,
      P_4,
      P_5,
      P_6,
      P_7,
      P_8,
      P_9,
      P_MAN
   };
   
   /** Type for the contact state when known */
   enum class contact_t : bool
   {
      opened = 0,
      closed = 1
   };
   
   /** Type for the NO/NC rocker switch when known */
   enum class rocker_toggle_t : bool
   {
      no = 0,
      nc = 1
   };
   
   /** Status of the sequencer */
   enum class seq_status_t : uint8_t
   {
      stopped,
      running ,
      paused  
   };

   /** Easy integer cast */
   inline uint8_t operator*(const pgm_t pgm) { return static_cast<uint8_t>(pgm); }

   /** Counter of running cycles */
   using cycle_counter_t = etl::optional<uint16_t>;
   
   /** Rocker optional shortcut */
   using rocker_toggle_optional_t = etl::optional<rocker_toggle_t>;
   
   
   /*
    * Inline instantiating
    */

   ///< The current program. Defaults to man
   inline static pgm_t pgm_sel = P_MAN;
  
   ///< Contact status
   inline static contact_t contact = contact_t::opened;
   
   ///< No Nc position
   inline static rocker_toggle_optional_t toggle_pos;
   
   ///< Counter
   cycle_counter_t counter;
   
   ///< Status of the sequencer
   inline static seq_status_t seq_status = seq_status_t::stopped;

   /**
    * A single command item to execute.
    * It carries the operation as well as a delay.
    * The delay is to be observed after the command
    */
   struct CommandItem
   {
      ///< Command for the engine
      enum class ECommand : char
      {
         delay = 'd',
         open = 'o',
         close = 'c'
      } command;

      ///< Delay after the command execution
      uint32_t delay_ms;

      ///< Simple constructor
      explicit CommandItem(ECommand type, uint32_t delay = 0) : command(type), delay_ms(delay)
      {
      }

      ///< Simple Enum converter for debug
      constexpr const char *to_string(ECommand c) const
      {
         switch (c)
         {
         case ECommand::open:
            return "Open";
         case ECommand::close:
            return "Close";
         case ECommand::delay:
            return "Delay";
         }

         return "Idle";
      }
   };

   using Command = etl::vector<CommandItem, cyclo::max_items_per_command>;
};


#endif // cyclo_header_included