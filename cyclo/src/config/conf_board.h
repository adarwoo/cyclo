#ifndef CONF_BOARD_H
#define CONF_BOARD_H
/**
 * \file
 *
 * \brief User board configuration template
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

// Joystick
#define JOYSTICK_UP IOPORT_CREATE_PIN(PORTA, 0)
#define JOYSTICK_DOWN IOPORT_CREATE_PIN(PORTA, 2)
#define JOYSTICK_PUSH IOPORT_CREATE_PIN(PORTA, 1)

// NO/NC
#define SWITCH_SENSE_NO IOPORT_CREATE_PIN(PORTA, 3)
#define SWITCH_SENSE_NC IOPORT_CREATE_PIN(PORTA, 4)

// Relay
#define RELAY_CTRL IOPORT_CREATE_PIN(PORTA, 7)

// Tracing
#define TRACE_INFO IOPORT_CREATE_PIN(PORTD, 0)
#define TRACE_WARN IOPORT_CREATE_PIN(PORTD, 1)
#define TRACE_ERR  IOPORT_CREATE_PIN(PORTD, 2)

/*
 * Timers allocation
 * TCC0 TCC1 TCD0 TCD1 TCE0 TCE1 TCF0 TCF1
 */
#define FREERTOS_TC TCC0
#define KEYPAD_TC   TCD0
#define NONC_TC     TCE0

/*
 * Keypad defines
 */

// Define the key codes defines
#define KEY_UP 1<<0
#define KEY_DOWN 1<<1
#define KEY_SELECT 1<<2

// Define the matching pins. The pins must be configured to yield a '0' on a push
#define KEYPAD_PINS JOYSTICK_UP, JOYSTICK_DOWN, JOYSTICK_PUSH


#endif // CONF_BOARD_H
