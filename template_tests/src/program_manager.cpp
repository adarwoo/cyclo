#include "program_manager.hpp"

#include "asx.h"

#include <logger.h>


namespace
{
   const char *const DOM = "cyclo_man";

   struct PgmStorage
   {
      char     marker; ///< Marker, must be a value other than 255 to be analysed
      char     pgm[ ProgramManager::STORAGE_MAX_LENGTH ]; ///< Actual program in ASCII storage area
      char     spare; 
      uint16_t crc;
   };

   constexpr size_t PGM_STORAGE_DATA_SIZE_NO_CRC = sizeof(PgmStorage) - sizeof(uint16_t);
};  // namespace

ProgramManager::ProgramManager() : selected{ 0 }, auto_start{ false }, state{ stopped }, counter{ 0 }
{
   LOG_HEADER( DOM );

   nvm_wait_until_ready();
   eeprom_enable_mapping();

   // Read the pages, looking for entries
   for ( uint8_t i = 0; i < 10; ++i )
   {
      // Address of the block
      uint8_t *marker_loc = pgm_mapped_at<uint8_t>( i );

      if ( *marker_loc != 0xFF )
      {
         // Compute the crc for the slot. Add one for the marker
         uint16_t crc = crc_io_checksum( marker_loc, PGM_STORAGE_DATA_SIZE_NO_CRC, CRC_16BIT );

         uint16_t actual_crc = *( (uint16_t *)( marker_loc + PGM_STORAGE_DATA_SIZE_NO_CRC ) );

         if ( crc == actual_crc )
         {
            occupancy_map.set( i );

            // Is it the default?
            if ( *marker_loc == '*' )
            {
               // Make it the default program
               selected = i;

               // Indicate the program is auto_start
               auto_start = true;
            }
         }
      }
   }
}

/**
 * @param from The position to start from. 0 is OK as 0 always exists
 * @returns The next available position - which could be the same
 */
uint8_t ProgramManager::get_next( uint8_t from )
{
   uint8_t pos = from;

   while ( ++pos != occupancy_map.size() )
   {
      if ( occupancy_map[ pos ] )
      {
         return pos;
      }
   }

   return from;
}

/**
 * @param from The position to start from. 0 is OK as 0 always exists
 * @returns The previous valid position or the same if no previous was found
 */
uint8_t ProgramManager::get_prev( uint8_t from )
{
   while ( from != 0 )
   {
      if ( occupancy_map[ --from ] )
      {
         return from;
      }
   }

   return 0;
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

   const char *retval = pgm_mapped_at<const char>( index );

   // Skip the marker
   return ( retval + 1 );
}

/**
 * @param index Index of the program to look for
 * @return a pointer to the program at the given index
 */
void ProgramManager::write_pgm_at( uint8_t pos, const char *string )
{
   LOG_HEADER( DOM );

   static PgmStorage buffer;

   // Reset the buffer content to all zero
   memset( &buffer, 0, sizeof( buffer ) );

   buffer.marker = 'A';
   strncpy( buffer.pgm, string, STORAGE_MAX_LENGTH );

   // Compute CRC - the whole buffer excluding the crc itself
   buffer.crc = crc_io_checksum( &buffer, PGM_STORAGE_DATA_SIZE_NO_CRC, CRC_16BIT );

   // Write eeprom
   nvm_eeprom_load_page_to_buffer( reinterpret_cast<const uint8_t *>( &buffer ) );
   nvm_eeprom_atomic_write_page( pos );
   nvm_eeprom_load_page_to_buffer(
      reinterpret_cast<const uint8_t *>( &buffer ) + EEPROM_PAGE_SIZE );
   nvm_eeprom_atomic_write_page( pos + 1 );
}

/**
 * The program is loaded, parsed. If OK, the get_program() method gives access
 * to the command.
 * @param index Index of the program to load
 * @return true if all ok
 */
bool ProgramManager::load( uint8_t pgmIndex )
{
   bool retval = false;
   LOG_HEADER( DOM );

   // Must have a program
   if ( occupancy_map[ pgmIndex ] )
   {
      // Get the string
      etl::string_view pgm{ get_pgm( pgmIndex ) };

      // Parse
      retval = parser.parse( pgm );
   }

   return retval;
}
