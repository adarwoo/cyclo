#include <cassert>
#include <cstdlib>
#include <thread>
#include <algorithm>
#include <fstream>

// For the getch
#include <unistd.h>
#include <termios.h>

// For the CRC
#include <etl/crc.h>

#include <logger.h>

// For the logger
#include <FreeRTOS.h>
#include <task.h>
#include <logger/logger_os.h>

#include "asx.h"

#include "keypad.h"

namespace
{
    // Logger domain
    const char * const DOM = "sim";

    // EEProm content
    uint8_t eeprom_memory[2048];

    // EEProm page buffer
    uint8_t eeprom_page_buffer[EEPROM_PAGE_SIZE];
}

// EEProm simulated memory
uintptr_t MAPPED_EEPROM_START = (uintptr_t)eeprom_memory;

extern "C" 
{
    uint8_t ports[8] = {0};

    void board_init()
    {
        std::fill_n(eeprom_memory, sizeof(eeprom_memory), 0xff);

        // TODO -> Init the GUI?
    }

    bool ioport_get_pin_level(ioport_pin_t pin)
    {
        uint8_t bitpos = pin & 0b111;
        uint8_t bitmask = 1 << bitpos;
        uint8_t port = pin >> 8;
        char portLetter = 'A' + port;

        bool retval = ports[port] & bitmask;

        LOG_TRACE(DOM, "PORT%c,%d : %s", portLetter, bitpos, retval ? "on" : "off" );

        return retval;
    }

    void ioport_set_pin_level(ioport_pin_t pin, bool level)
    {
        uint8_t bitpos = pin & 0b111;
        uint8_t bitmask = 1 << bitpos;
        uint8_t port = pin >> 8;
        char portLetter = 'A' + port;

        bool from = ports[port] & bitmask;

        LOG_TRACE(DOM, "PORT%c,%d %s > %s", portLetter, bitpos, from ? "on" : "off", level ? "ON" : "OFF");

        // Clear the bit
        ports[port] &=  ~bitmask;

        // Mask with value
        ports[port] |= bitmask;
    }

    void ioport_toggle_pin_level(ioport_pin_t pin)
    {
        uint8_t bitpos = pin & 0b111;
        uint8_t bitmask = 1 << bitpos;
        uint8_t port = pin >> 8;
        char portLetter = 'A' + port;

        LOG_TRACE(DOM, "PORT%c,%d %s", portLetter, bitpos, ports[port] & bitmask ? "on > OFF" : "off > ON");

        // Toggle the bit
        ports[port] ^= bitmask;
    }

    //
    // Trace
    //
    void trace_assert(bool test, ioport_pin_t pin)
    {
        assert(test);
    }

    //
    // Keypad
    //
    struct keypad_key_t
    {
        uint8_t mask;             ///< ID of the key
        keypad_handler_t handler; ///< Callback to call on a push or repeat
        void *param;              ///< Additonal param to pass along
    };

    // Register a timer for sampling the keypad_pins
    constexpr ioport_pin_t keypad_pins[] = {KEYPAD_PINS};
    constexpr uint8_t KEYPAD_NUMBER_OF_KEYS = sizeof(keypad_pins) / sizeof(ioport_pin_t);
    static keypad_key_t keypad_keys[KEYPAD_NUMBER_OF_KEYS] = {{KEY_UP}, {KEY_DOWN}, {KEY_SELECT}};

    char getch()
    {
        char buf = 0;
        struct termios old = {0};
        int ret;
        
        tcgetattr(0, &old);
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 10;
        tcsetattr(0, TCSANOW, &old);
        read(0, &buf, 1);
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        tcsetattr(0, TCSADRAIN, &old);

        return buf;
    }

    void scan_keys()
    {
        char c = 0;

        while (c != 'q')
        {
            uint8_t key = 0;

            switch (c)
            {
            case 'w': key = KEY_UP; break;
            case 's': key = KEY_DOWN; break;
            case '\n':  key = KEY_SELECT; break;
            default: break;
            }

            if ( key != 0 )
            {
                for (size_t i = 0; i < KEYPAD_NUMBER_OF_KEYS; ++i)
                {
                    if ( keypad_keys[i].mask &= key )
                    {
                        // Call the handler
                        keypad_keys[i].handler(key, keypad_keys[i].param);
                    }
                }
            }

            c = getch();
        }

        exit(0);
    }

    /** Initialise the keypad library */
    void keypad_init(void)
    {
        // Start a thread which scan the keypad and calls the callback
        auto background = std::thread( &scan_keys );

        background.detach();
    }

    /** 
     * Regsiter a callback for one or many key
     * key_masks Mask of the keys to handle
     * handler Callback function (from isr)
     * param Optional data to pass along
     */
    void keypad_register_callback(uint8_t key_masks, keypad_handler_t handler, void *param)
    {
        uint8_t i;

        for (i = 0; i < KEYPAD_NUMBER_OF_KEYS; ++i)
        {
            if (keypad_keys[i].mask &= key_masks)
            {
                keypad_keys[i].handler = handler;
                keypad_keys[i].param = param;
            }
        }
    }

    //
    // EEProm emulation
    //
   void nvm_wait_until_ready() {}
   void eeprom_enable_mapping() {}
   
   void nvm_eeprom_atomic_write_page(uint8_t page_addr)
   {
       using ofs_t = std::ofstream;

       // Write to whole buffer back
       auto ofs = ofs_t("eeprom.bin", ofs_t::binary | ofs_t::out);

       if ( ! ofs.good() )
       {
           LOG_ERROR(DOM, "Failed to create eeprom.bin");
           return;
       }

       ofs.write(reinterpret_cast<char*>(eeprom_memory), sizeof(eeprom_memory));
   }

   void nvm_eeprom_load_page_to_buffer(const uint8_t *values) 
   {
       memcpy(eeprom_page_buffer, values, EEPROM_PAGE_SIZE);
   }

   // CRC Emulation
   uint32_t crc_io_checksum(void *data, uint16_t len, enum crc_16_32_t crc_16_32)
   {
       auto crc = etl::crc16 {};

       	// Write data to DATAIN register
    	while (len--) {
	    	crc.add(*(uint8_t*)data);
		    data = (uint8_t*)data + 1;
        }

        return crc.value();
	}

    //
    // Logger - enhance
    //

    /**
     * Get the name of the thread instead
     */
    void _log_copy_thread_id( char *buffer, size_t count, _LOG_THREAD_ID id)
    {
        TaskHandle_t h = xTaskGetCurrentTaskHandle();

        if ( h != 0 )
        {
           snprintf( buffer, count, " %s ", pcTaskGetName(h) );
        }
        else
        {
           snprintf( buffer, count, " main ");
        }
    }

}