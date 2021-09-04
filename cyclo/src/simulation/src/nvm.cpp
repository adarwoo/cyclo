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
#include <algorithm>
#include <cstring>
#include <fstream>

// For the CRC
#include "asx.h"

#include <etl/crc.h>

#include <logger.h>

namespace
{
   // Logger domain
   const char *const DOM = "nvm";

   // EEProm content
   uint8_t eeprom_memory[ 2048 ];

   // EEProm page buffer
   uint8_t eeprom_page_buffer[ EEPROM_PAGE_SIZE ];
}  // namespace

// EEProm simulated memory
const uintptr_t MAPPED_EEPROM_START = (uintptr_t)eeprom_memory;

extern "C"
{
   //
   // EEProm emulation
   //
   void nvm_wait_until_ready() {}

   void nvm_init() { std::fill_n( eeprom_memory, sizeof( eeprom_memory ), 0xff ); }

   void eeprom_enable_mapping()
   {
      using ifs_t = std::ifstream;

      // Read the whole file into the buffer
      auto ifs = ifs_t( "/tmp/eeprom.bin", ifs_t::binary );

      if ( ifs )
      {
         ifs.read( reinterpret_cast<char *>( eeprom_memory ), sizeof( eeprom_memory ) );
      }
      else
      {
         LOG_WARN( DOM, "Failed to read eeprom.bin" );
      }
   }

   void nvm_eeprom_atomic_write_page( uint8_t page )
   {
      LOG_TRACE(DOM, "Writing page %d", page);
      using ofs_t = std::ofstream;

      // Transfer the buffer to the main memory
      memcpy( eeprom_memory + EEPROM_PAGE_SIZE * page, eeprom_page_buffer, EEPROM_PAGE_SIZE );

      // Write to whole buffer back
      auto ofs = ofs_t( "/tmp/eeprom.bin", ofs_t::binary | ofs_t::out );

      if ( ! ofs.good() )
      {
         LOG_ERROR( DOM, "Failed to create eeprom.bin" );
         return;
      }

      ofs.write( reinterpret_cast<char *>( eeprom_memory ), sizeof( eeprom_memory ) );
   }

   void nvm_eeprom_load_page_to_buffer( const uint8_t *values )
   {
      memcpy( eeprom_page_buffer, values, EEPROM_PAGE_SIZE );
   }

   // CRC Emulation
   uint32_t crc_io_checksum( void *data, uint16_t len, enum crc_16_32_t crc_16_32 )
   {
      auto crc = etl::crc16{};

      // Write data to DATAIN register
      while ( len-- )
      {
         crc.add( *(uint8_t *)data );
         data = (uint8_t *)data + 1;
      }

      return crc.value();
   }

   /**
    * \brief Erase EEPROM page.
    *
    * This function erases one EEPROM page, so that every location reads 0xFF.
    *
    * \param page_addr EEPROM Page address, between 0 and EEPROM_SIZE/EEPROM_PAGE_SIZE
    */
   void nvm_eeprom_erase_page( uint8_t page_addr )
   {
      memset( eeprom_memory + EEPROM_PAGE_SIZE * page_addr, 0xff, EEPROM_PAGE_SIZE );
   }
}