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
#include "contact.hpp"
#include "parser.hpp"
#include "program.hpp"
#include "trace.h"

#include <etl/bitset.h>
#include <etl/string_view.h>

#include "conf_cyclo.hpp"



/**
 * Storage in NV of the various persistable items
 * This manages the programs and the state of the system. It  is a model only.
 * It does not emit messages.
 */
class ProgramManager
{
public:
   /** A bitset which determine which slot contains a valid program */
   using Pgms = etl::bitset<10>;

   /** The state of the active program  */
   enum program_state_t : uint8_t { stopped, paused, running, usb };

   /** The program mode */
   enum class mode_t : uint8_t { normal=0, autostart=0x50, lastused=0x0A };

   /** The storage for the program must fit 2 pages of eeprom - header, spare and crc (2 bytes) */
   static constexpr size_t STORAGE_MAX_LENGTH = ( EEPROM_PAGE_SIZE * 2 ) - 4;

private:
   ///< Current selected program. 0 is auto. -1 is none.
   int8_t selected;

   ///< Which program should be started right away or -1
   int8_t auto_start;

   ///< Which program was last used - so we make it the default (unless autostart)
   int8_t last_used;

   ///< Current state of the active program
   program_state_t state;

   /** Current value of the counter. uint16_max for undefined */
   int32_t counter;

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
   inline int8_t get_selected() { return selected; }
   inline void   set_selected( int8_t pgm ) { selected = pgm; }

   inline program_state_t get_state() { return state; }
   inline void            set_state( program_state_t newState ) { state = newState; }

   ///< Grab the execution counter
   inline int32_t get_counter() { return counter; }
   inline void     set_counter( int32_t newValue ) { counter = newValue; }

   // @return true if a program starts automatically
   inline bool starts_automatically() const { return ( auto_start >= 0 ); }

   // @return The index of the program to start automatically or -1
   inline int8_t get_autostart_index() const { return auto_start; }

   // @return The index of the last used program
   inline int8_t get_lastused_index() const { return last_used; }

   // Grab the contact manager
   inline Contact &get_contact() { return contact; }

   // Grab the program
   inline Program &get_active_program() { return active_program; }

   // Grab the map
   inline Pgms get_map() { return occupancy_map; }

   /** Grab the next available slot from the given position */
   int8_t get_next( int8_t from );

   /** Grab the prev available slot from the given position*/
   int8_t get_prev( int8_t from );

   /** Get the program string at the given index */
   const char *get_pgm( uint8_t index );

   /** Write the given program at the given slot. 0 is the auto slot */
   void write_pgm_at( uint8_t pos, etl::string_view view);

   /** Load a program from the eeprom - and start it */
   void load( uint8_t pgmIndex );

   /** Load a program and start it */
   void load( const Program &pgm );

   /** Scan the whole eeprom for valid programs */
   void scan();

   /** Set a program has auto start */
   void set_autostart( int8_t index );

   /** Set a program has last used */
   void set_lastused( int8_t index );

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

   eeprom_addr_t pgm_at( const size_t index ) { return EEPROM_PAGE_SIZE * index * 2; }
};


#endif  // command_manager_header_included
