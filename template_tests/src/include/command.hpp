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
struct CommandItem
{
   ///< Command for the engine
   enum command_t : char { open = 'o', close = 'c', delay = 'd', loop = 'l' } command;

   ///< Delay after the command execution
   uint32_t delay_ms;

   ///< Simple constructor
   explicit CommandItem( command_t type, uint32_t delay = 0 ) : command{ type }, delay_ms{ delay }
   {}
};


/**
 * Holds a complete program already parsed
 */
class Command : public etl::vector<CommandItem, cyclo::max_items_per_command>
{
   const_iterator it;

public:
   // Command
   Command() : it{ begin() } {}

   /**
    * Get the next item. This is the first item following a reset.
    * Automatically loops the sequence for looped commands
    * @returns A pointer to the next command to execute or nullptr if
    *           the command has ended.
    */
   auto iterate()
   {
      const CommandItem *retval = nullptr;

      if ( it != end() )
      {
         retval = &( *it++ );
      }

      return retval;
   }

   void start() { it = begin(); }
};


#endif  // ndef command_hpp_was_included