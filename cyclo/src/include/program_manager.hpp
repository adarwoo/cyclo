#ifndef command_manager_header_included
#define command_manager_header_included
/*
 * Manage the commands
 * =============================
 * Manages parsing the commands
 * Manages the current command
 * Manages the eeprom storage
 * -------------------------
 * The EEProm is divided into a entry allocation table made of fixed sized items, pointing
 *  to variable size items
 *
 * Created: 17/07/2021 18:22:20
 *  Author: micro
 */
#include <etl/bitset.h>
#include <etl/string_view.h>

#include "program.hpp"
#include "conf_cyclo.hpp"
#include "contact.hpp"
#include "parser.hpp"
#include "trace.h"


/**
 * Storage in NV of the various persistable items
 * This manages the programs and the state of the system. It  is a model only.
 * It does not emit messages.
 */
class ProgramManager
{
public:
   /** A bitset which determine which slot contains a valid program */
   using Pgms                                 = etl::bitset<10>;

   /** The state of the active program  */
   enum program_state_t : uint8_t { stopped, paused, running };

   /** The storage for the program must fit 2 pages of eeprom - header, spare and crc (2 bytes) */
   constexpr size_t STORAGE_MAX_LENGTH = (EEPROM_PAGE_SIZE * 2) - 4;

private:
   ///< Current selected program. 0 is auto. 255 is none.
   uint8_t selected;

   ///< Should the program starts right away?
   bool auto_start;

   ///< Current state of the active program
   program_state_t state;

   /** Current value of the counter. uint16_max for undefined */
   uint16_t counter;

   /** Bit field containing program slots taken */
   Pgms occupancy_map;

   // Create the contact manager
   Contact contact;

   ///< Avoid a race between the UI, the console and the sequencer
   rtos::Mutex lock;

   ///< Copy of the active program
   Program active_program;

   ///< Parser instance used when loading a program. Allow reuse, and save the stack
   Parser parser;

public:
   ProgramManager();

   //////////////////////////////////////////////////////////////////////////
   // Accessors
   //////////////////////////////////////////////////////////////////////////
public:
   inline uint8_t get_selected() { return selected; }
   inline void    set_selected( uint8_t pgm ) { selected = pgm; }

   inline program_state_t get_state() { return state; }
   inline void            set_state( program_state_t newState ) { state = newState; }

   ///< Grab the execution counter
   inline uint16_t get_counter() { return counter; }
   inline void     set_counter( uint16_t newValue ) { counter = newValue; }

   // @return true if a program starts automatically
   inline bool starts_automatically() const { return auto_start; }

   // Grab the contact manager
   inline Contact &get_contact() { return contact; }

   // Grab the program
   inline Program &get_active_program();

   /** Grab the next available slot from the given position */
   uint8_t get_next( uint8_t from );

   /** Grab the prev available slot from the given position*/
   uint8_t get_prev( uint8_t from );

   /** Get the program string at the given index */
   const char *get_pgm( uint8_t index );

   /** Write the given program at the given slot. 0 is the auto slot */
   void write_pgm_at( uint8_t pos, const char *string );

   /** Load a program from the eeprom - and start it */
   void load( uint8_t pgmIndex );

   /** Load a program and start it */
   void load( const Program &pgm )

   /** Set a program has auto start */
   void set_autostart(uint8_t index );

   //
   // Drive the sequencer
   //
   void stop();
   void resume();

   ///< Erase a program
   void erase( uint8_t pgmIndex );

protected:
   template<typename T>
   T *pgm_mapped_at( size_t pgmIndex )
   {
      void *address = reinterpret_cast<void *>( pgm_at( pgmIndex ) + MAPPED_EEPROM_START );

      return static_cast<T *>( address );
   }

   eeprom_addr_t pgm_at( const size_t index )
   {
      return EEPROM_PAGE_SIZE * index * ( STORAGE_MAX_LENGTH / EEPROM_PAGE_SIZE );
   }
};


#endif  // command_manager_header_included
