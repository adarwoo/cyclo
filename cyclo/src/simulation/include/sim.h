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
#include <stdbool.h>
#include <stdint.h>

#include <progmem.h>
#include <compiler.h>
#include <gfx_mono.h>
#include <sysfont.h>


//
// Defines
//
#define PORTA 0
#define PORTB 1
#define PORTC 2
#define PORTD 3
#define PORTE 4
#define PORTF 5

#define IOPORT_CREATE_PIN( port, pin ) ( port << 8 | pin )

#define EEPROM_PAGE_SIZE 32

//
// Types
//

// Simulate the port pin type
typedef uint16_t ioport_pin_t;

// EEProm address
typedef uint16_t       eeprom_addr_t;
extern const uintptr_t MAPPED_EEPROM_START;

//
// APIs
//
#ifdef __cplusplus
extern "C"
{
#endif
   // General
   void board_init();

   // IOport
   bool ioport_get_pin_level( ioport_pin_t pp );
   void ioport_set_pin_level( ioport_pin_t pin, bool level );
   void ioport_toggle_pin_level( ioport_pin_t pin );

   // EEProm emulation
   void nvm_wait_until_ready();
   void eeprom_enable_mapping();
   void nvm_eeprom_atomic_write_page( uint8_t page_addr );
   void nvm_eeprom_load_page_to_buffer( const uint8_t *values );
   void nvm_eeprom_erase_page( uint8_t page_addr );

   // UDI
   int udi_cdc_multi_putc( uint8_t port, int value );
   int udi_cdc_getc( void );

   // CRC Emulation
   enum crc_16_32_t {
      CRC_16BIT,
      CRC_32BIT,
   };
   uint32_t crc_io_checksum( void *data, uint16_t len, enum crc_16_32_t crc_16_32 );

#ifdef __cplusplus
}
#endif

// Include the board.h now the functions/macros are defined
#include "conf_board.h"
