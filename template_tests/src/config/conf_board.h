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
#define TRACE_TICK IOPORT_CREATE_PIN(PORTD, 0)
#define TRACE_IDLE IOPORT_CREATE_PIN(PORTD, 1)
#define TRACE_ERR  IOPORT_CREATE_PIN(PORTD, 2)


#endif // CONF_BOARD_H
