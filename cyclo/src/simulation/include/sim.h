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


// Wdt
//! Watchdog window period setting
enum wdt_window_period_t {
	WDT_WINDOW_PERIOD_8CLK = (0x00),
	WDT_WINDOW_PERIOD_16CLK = (0x01),
	WDT_WINDOW_PERIOD_32CLK = (0x02),
	WDT_WINDOW_PERIOD_64CLK = (0x03),
	WDT_WINDOW_PERIOD_125CLK = (0x04),
	WDT_WINDOW_PERIOD_250CLK = (0x05),
	WDT_WINDOW_PERIOD_500CLK = (0x06),
	WDT_WINDOW_PERIOD_1KCLK = (0x07),
	WDT_WINDOW_PERIOD_2KCLK = (0x08),
	WDT_WINDOW_PERIOD_4KCLK = (0x09),
	WDT_WINDOW_PERIOD_8KCLK = (0x0A),
};

enum wdt_timeout_period_t {
	//! Timeout period = 8 cycles or 8 ms @ 3.3V
	WDT_TIMEOUT_PERIOD_8CLK = (0x00),
	//! Timeout period = 16 cycles or 16 ms @ 3.3V
	WDT_TIMEOUT_PERIOD_16CLK = (0x01),
	//! Timeout period = 32 cycles or 32m s @ 3.3V
	WDT_TIMEOUT_PERIOD_32CLK = (0x02),
	//! Timeout period = 64 cycles or 64ms @ 3.3V
	WDT_TIMEOUT_PERIOD_64CLK = (0x03),
	//! Timeout period = 125 cycles or 125ms @ 3.3V
	WDT_TIMEOUT_PERIOD_125CLK = (0x04),
	//! 250 cycles or 250ms @ 3.3V)
	WDT_TIMEOUT_PERIOD_250CLK = (0x05),
	//! Timeout period = 500 cycles or 500ms @ 3.3V
	WDT_TIMEOUT_PERIOD_500CLK = (0x06),
	//! Timeout period =1K cycles or 1s @ 3.3V
	WDT_TIMEOUT_PERIOD_1KCLK = (0x07),
	//! Timeout period = 2K cycles or 2s @ 3.3V
	WDT_TIMEOUT_PERIOD_2KCLK = (0x08),
	//! Timeout period = 4K cycles or 4s @ 3.3V
	WDT_TIMEOUT_PERIOD_4KCLK = (0x09),
	//! Timeout period = 8K cycles or 8s @ 3.3V
	WDT_TIMEOUT_PERIOD_8KCLK = (0x0A),
};

void wdt_enable(void);
void wdt_set_timeout_period(enum wdt_timeout_period_t to_period);
void wdt_reset();

#ifdef __cplusplus
}
#endif

// Include the board.h now the functions/macros are defined
#include "conf_board.h"
