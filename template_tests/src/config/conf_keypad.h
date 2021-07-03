#ifndef CONF_KEYPAD_H_
#define CONF_KEYPAD_H_
/*
 * conf_keypad.h
 *
 * Configuration for the keypad
 */ 

// Define the key codes defines
#define KEY_UP 1<<0
#define KEY_DOWN 1<<1
#define KEY_SELECT 1<<2

// Define the matching pins. The pins must be configured to yield a '0' on a push
#define KEYPAD_PINS JOYSTICK_UP, JOYSTICK_DOWN, JOYSTICK_PUSH

// Select the timer to use
#define KEYPAD_TC TCD0


#endif /* CONF_KEYPAD_H_ */