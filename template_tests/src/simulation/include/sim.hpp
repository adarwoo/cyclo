#include <stdbool.h>
#include <stdint.h>

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
typedef uint16_t eeprom_addr_t;
extern const uintptr_t MAPPED_EEPROM_START;

//
// APIs
//
extern "C"
{
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

   // CRC Emulation
   enum crc_16_32_t
   {
      CRC_16BIT,
      CRC_32BIT,
   };
   uint32_t crc_io_checksum( void *data, uint16_t len, enum crc_16_32_t crc_16_32 );
}

// Include the board.h now the functions/macros are defined
#include "conf_board.h"
