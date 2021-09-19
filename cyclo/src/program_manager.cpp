///\file

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

// clang-format off
#include "asx.h"
#include "program_manager.hpp"

#include <logger.h>
// clang-format on

namespace
{
   const char *const DOM = "cyclo_man";

   // Set the storage to use the maximum space, matching an EEProm page size
   constexpr auto STORAGE_MAX_LENGTH = ( EEPROM_PAGE_SIZE * 2 ) - 5;

   ///< Flags for the program
   constexpr auto ALL_ONES  = '\xff';
   constexpr auto PGMSLOT   = 'P';
   constexpr auto AUTOSTART = 'A';
   constexpr auto LASTUSED  = 'L';

   struct PgmStorage
   {
      char     marker;  ///< Marker, must be a value other than 255 to be analysed
      char     auto_start;
      char     last_used;
      char     pgm[ STORAGE_MAX_LENGTH ];  ///< Actual program in ASCII storage area
      uint16_t crc;
   };

   // Actual length of the data - excluding the CRC
   constexpr size_t PGM_STORAGE_DATA_SIZE_NO_CRC = sizeof( PgmStorage ) - sizeof( uint16_t );

   // Zero string
   etl::string<0> zs;

   // Default manual program string
   const char *DEFAULT_MANUAL_PGM = "c 1M 0s o 0M 5s *";
};  // namespace

ProgramManager::ProgramManager()
   : selected{ 0 }
   , auto_start{ -1 }
   , last_used{ -1 }
   , state{ stopped }
   , counter{ -1 }
   , parser{ active_program, zs }
{
   LOG_HEADER( DOM );

   // Scan the eeprom for all programs
   scan();

   if ( auto_start >= 0 )
   {
      selected = auto_start;
   }
   else if ( last_used >= 0 )
   {
      selected = last_used;
   }

   // The program 0 must exists - create on if nothing
   if ( not occupancy_map[ 0 ] )
   {
      write_pgm_at( 0, DEFAULT_MANUAL_PGM );
   }
}

void ProgramManager::scan()
{
   LOG_HEADER( DOM );

   nvm_wait_until_ready();
   eeprom_enable_mapping();

   // Read the pages, looking for entries
   for ( uint8_t i = 0; i < 10; ++i )
   {
      // Address of the block
      PgmStorage *pgm = pgm_mapped_at<PgmStorage>( i );

      if ( pgm->marker == PGMSLOT )
      {
         // Compute the crc for the slot. Add one for the marker
         uint16_t crc = crc_io_checksum( pgm, PGM_STORAGE_DATA_SIZE_NO_CRC, CRC_16BIT );

         if ( crc == pgm->crc )
         {
            occupancy_map.set( i );

            // Is it the auto start?
            if ( pgm->auto_start == AUTOSTART )
            {
               // Indicate the program is auto_start
               auto_start = i;
            }

            // Is is the default (for the GUI)
            if ( pgm->last_used == LASTUSED )
            {
               // Indicate the program is auto_start
               last_used = i;
            }
         }
      }
   }
}

/**
 * @param from The position to start from. 0 is OK as 0 always exists
 * @returns The next available position - or -1 if none found
 */
int8_t ProgramManager::get_next( int8_t from )
{
   uint8_t pos = from;

   while ( ++pos != occupancy_map.size() )
   {
      if ( occupancy_map[ pos ] )
      {
         return pos;
      }
   }

   return -1;
}

/**
 * @param from The position to start from. 0 is OK as 0 always exists
 * @returns The previous valid position or the same if no previous was found
 */
int8_t ProgramManager::get_prev( int8_t from )
{
   if ( from < 0 )
   {
      // Go for the last possible
      from = occupancy_map.size();
   }

   while ( from > 0 )
   {
      if ( occupancy_map[ --from ] )
      {
         return from;
      }
   }

   return -1;
}

/**
 * @param index Index of the program to look for
 * @return a pointer to the program at the given index
 */
const char *ProgramManager::get_pgm( uint8_t index )
{
   LOG_HEADER( DOM );

   nvm_wait_until_ready();
   eeprom_enable_mapping();

   auto *pgm = pgm_mapped_at<PgmStorage>( index );

   // Skip the marker
   return pgm->pgm;
}

/**
 * @param pos Index of the program
 * @stirng The content to write. The CRC is automatically added
 * @return a pointer to the program at the given index
 */
void ProgramManager::write_pgm_at( uint8_t pos, etl::string_view view )
{
   LOG_HEADER( DOM );

   PgmStorage buffer;

   // Reset the buffer content to all zero
   memset( &buffer, 0, sizeof( buffer ) );

   // Determine the mode from the current settings
   buffer.marker     = PGMSLOT;
   buffer.auto_start = ( auto_start == pos ) ? AUTOSTART : ALL_ONES;
   buffer.last_used  = ( last_used == pos ) ? LASTUSED : ALL_ONES;

   strncpy( buffer.pgm, view.data(), etl::min( view.size(), STORAGE_MAX_LENGTH ) );

   // Compute CRC - the whole buffer excluding the crc itself
   buffer.crc = crc_io_checksum( &buffer, PGM_STORAGE_DATA_SIZE_NO_CRC, CRC_16BIT );

   // Write eeprom
   nvm_eeprom_load_page_to_buffer( reinterpret_cast<const uint8_t *>( &buffer ) );
   nvm_eeprom_atomic_write_page( pos * 2 );
   nvm_eeprom_load_page_to_buffer(
      reinterpret_cast<const uint8_t *>( &buffer ) + EEPROM_PAGE_SIZE );
   nvm_eeprom_atomic_write_page( ( pos * 2 ) + 1 );

   // Mark as available
   occupancy_map.set( pos );
}

/**
 * The program is loaded, parsed. If OK, the get_program() method gives access
 * to the command.
 * Errors are ignored, but logged.
 * If OK, start the sequencer
 * @param index Index of the program to load
 * @return true if all ok
 */
void ProgramManager::load( uint8_t pgmIndex )
{
   LOG_HEADER( DOM );
   bool loaded = false;

   // As different tasks using this method, make it safe
   rtos::Lock_guard{ lock };

   // Must have a program
   if ( occupancy_map[ pgmIndex ] )
   {
      // Get the string
      etl::string_view pgm{ get_pgm( pgmIndex ) };

      // Parse
      auto res = parser.parse( pgm );
      loaded   = ( res == Parser::Result::program );
   }

   // Force a default to avoid the system going ape
   if ( not loaded )
   {
      active_program.clear();

      // Insert items
      active_program.push_back( Command{ Command::open, 60000 } );
      active_program.push_back( Command{ Command::close, 5000 } );
      active_program.push_back( Command{ Command::loop } );
   }
}

/**
 * Load a program into the active program and start
 * The given program is copied.
 * @param The program to load.
 */
void ProgramManager::load( const Program &pgm )
{
   LOG_HEADER( DOM );

   // As different tasks using this method, make it safe
   rtos::Lock_guard{ lock };

   // Make a copy
   active_program.assign( pgm.begin(), pgm.end() );
   active_program.start();

   // Let the sequencer know
   fx::publish( msg::StartProgram{ true } );
}

void ProgramManager::stop()
{
   // Let the sequencer know
   fx::publish( msg::StopProgram{} );
}

void ProgramManager::resume()
{
   // Let the sequencer know
   fx::publish( msg::StartProgram{ false } );
}

void ProgramManager::erase( uint8_t pgmIndex )
{
   nvm_eeprom_erase_page( pgmIndex * 2 );
   occupancy_map.set( pgmIndex, false );
}

void ProgramManager::set_autostart( int8_t pgmIndex )
{
   auto prev = auto_start;

   // Make this program the new auto-start
   auto_start = pgmIndex;

   if ( prev != auto_start )
   {
      // Could be -1 to simply turn off auto-start
      if ( pgmIndex >= 0 )
      {
         write_pgm_at( pgmIndex, get_pgm( pgmIndex ) );
      }

      // Re-write the previous program to remove its attribute
      if ( prev >= 0 )
      {
         write_pgm_at( prev, get_pgm( prev ) );
      }
   }
}

void ProgramManager::set_lastused( int8_t pgmIndex )
{
   auto prev = last_used;

   // Make this program the new auto-start
   last_used = pgmIndex;

   if ( prev != last_used )
   {
      if ( pgmIndex >= 0 )
      {
         write_pgm_at( pgmIndex, get_pgm( pgmIndex ) );
      }

      // Re-write the previous program to remove its attribute
      if ( prev >= 0 )
      {
         write_pgm_at( prev, get_pgm( prev ) );
      }
   }
}
