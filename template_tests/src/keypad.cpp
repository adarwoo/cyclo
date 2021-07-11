#include <FreeRTOS.h>
#include <asf.h>
#include <string.h>

#include "conf_keypad.h"
#include "keypad.h"
#include "trace.h"

struct keypad_key_t 
{
   ioport_pin_t pin; ///< Pin to sample
   uint8_t integrator; ///< Current integrated value on the pin
   uint8_t cycle; ///< Number of consecutive cycle in the given state
   bool output; ///< Consolidated output
   keypad_handler_t handler; ///< Callback to call on a push or repeat
   void *param; ///< Additonal param to pass along
   uint8_t mask; ///< ID of the key
};

//static uint8_t KEYPAD_NUMBER_OF_KEYS;

// Register a timer for sampling the keypad_pins
constexpr ioport_pin_t keypad_pins[] = { KEYPAD_PINS };
constexpr uint8_t KEYPAD_NUMBER_OF_KEYS = sizeof(keypad_pins) / sizeof(ioport_pin_t);
static keypad_key_t keypad_keys_state[KEYPAD_NUMBER_OF_KEYS];

/**
written by Kenneth A. Kuhn
version 1.00
 
This is an algorithm that debounces or removes random or spurious
transistions of a digital signal read as an input by a computer.  This is
particularly applicable when the input is from a mechanical contact.  An
integrator is used to perform a time hysterisis so that the signal must
persistantly be in a logical state (0 or 1) in order for the output to change
to that state.  Random transitions of the input will not affect the output
except in the rare case where statistical clustering is longer than the
specified integration time.
 
The following example illustrates how this algorithm works.  The sequence
labeled, real signal, represents the real intended signal with no noise.  The
sequence labeled, corrupted, has significant random transitions added to the
real signal.  The sequence labled, integrator, represents the algorithm
integrator which is constrained to be between 0 and 3.  The sequence labeled,
output, only makes a transition when the integrator reaches either 0 or 3.
Note that the output signal lags the input signal by the integration time but
is free of spurious transitions.
 
real signal 0000111111110000000111111100000000011111111110000000000111111100000
 
corrupted   0100111011011001000011011010001001011100101111000100010111011100010
integrator  0100123233233212100012123232101001012321212333210100010123233321010
output      0000001111111111100000001111100000000111111111110000000001111111000
 
I have been using this algorithm for years and I show it here as a code
fragment in C.  The algorithm has been around for many years but does not seem
to be widely known.  Once in a rare while it is published in a tech note.  It
 
is notable that the algorithm uses integration as opposed to edge logic
(differentiation).  It is the integration that makes this algorithm so robust
in the presence of noise.
******************************************************************************/

/* The following parameters tune the algorithm to fit the particular
application.  The example numbers are for a case where a computer samples a
mechanical contact 10 times a second and a half-second integration time is
 
used to remove bounce.  Note: DEBOUNCE_TIME is in seconds and SAMPLE_FREQUENCY
is in Hertz */

// Set a default value for the sampling frequency of the keypad
#ifndef KEYPAD_SAMPLE_FREQUENCY_HZ
#define KEYPAD_SAMPLE_FREQUENCY_HZ 100
#endif

// Helper to convert a time into cycles
#define KEYPAD_MS_TO_CYCLES(ms) ((uint8_t)(ms * (KEYPAD_SAMPLE_FREQUENCY_HZ / 1000.0)))

// Set the filter cycles for the integrator
#ifndef KEYPAD_FILTER_DELAY_MS
#define KEYPAD_FILTER_DELAY_MS 40
#endif

// Set the first auto-repeat delay
#ifndef KEYPAD_FIRST_REPEAT_AFTER_MS
#define KEYPAD_FIRST_REPEAT_AFTER_MS 700
#endif

// Set the next auto-repeat delay
#ifndef KEYPAD_NEXT_REPEATS_EVERY_MS
#define KEYPAD_NEXT_REPEATS_EVERY_MS 100
#endif

// Shortcut in cycles
#define KEYPAD_FILTER_CYCLES KEYPAD_MS_TO_CYCLES(KEYPAD_FILTER_DELAY_MS)
#define KEYPAD_FIRST_REPEAT_CYCLES KEYPAD_MS_TO_CYCLES(KEYPAD_FIRST_REPEAT_AFTER_MS)
#define KEYPAD_NEXT_REPEAT_CYCLE (KEYPAD_FIRST_REPEAT_CYCLES + KEYPAD_MS_TO_CYCLES(KEYPAD_NEXT_REPEATS_EVERY_MS))

/** Called with the timer interrupt to sample the keys */
static void keypad_process(void)
{
    uint8_t i;

    for (i = 0; i < KEYPAD_NUMBER_OF_KEYS; ++i) {
        keypad_key_t* key = &(keypad_keys_state[i]);

        if (key->handler) {
            // Get the state of the key
            if (!ioport_get_pin_level(key->pin)) {
                if (key->integrator < KEYPAD_FILTER_CYCLES) {
                    ++key->integrator;

                    if (key->integrator == KEYPAD_FILTER_CYCLES) {
                        key->output = true;
                        key->cycle = 0;
                    }
                }
            } else {
                if (key->integrator) {
                    --key->integrator;

                    if (key->integrator == 0) {
                        key->output = false;
                    }
                }
            }

            // Check the integrator result
            if (key->output) {
                if (key->cycle == 0) {
                    key->handler(key->mask, key->param);
                } else if (key->cycle == KEYPAD_FIRST_REPEAT_CYCLES || key->cycle == KEYPAD_NEXT_REPEAT_CYCLE) {
                    // Repeats
                    key->handler(key->mask, key->param);
                }

                if (key->cycle == KEYPAD_NEXT_REPEAT_CYCLE) {
                    key->cycle = KEYPAD_FIRST_REPEAT_CYCLES;
                }

                ++key->cycle;
            }
        }
    }
}

/** Initialize the keypad library */
extern "C" void keypad_init()
{
    memset(keypad_keys_state, 0, sizeof(keypad_keys_state));

    // Initialize every keys
    for (uint8_t i = 0; i < KEYPAD_NUMBER_OF_KEYS; ++i) {
        keypad_keys_state[i].mask = 1 << i;
        keypad_keys_state[i].pin = keypad_pins[i];
    }

    // Initialise the sampling timer
    tc_enable(&KEYPAD_TC);
    tc_set_wgm(&KEYPAD_TC, TC_WG_NORMAL);
    tc_set_resolution(&KEYPAD_TC, sysclk_get_per_hz() / KEYPAD_SAMPLE_FREQUENCY_HZ);
    tc_write_period(&KEYPAD_TC, tc_get_resolution(&KEYPAD_TC) / KEYPAD_SAMPLE_FREQUENCY_HZ);
    tc_set_overflow_interrupt_callback(&KEYPAD_TC, keypad_process);

    // Low interrupt priority
    tc_set_overflow_interrupt_level(&KEYPAD_TC, TC_INT_LVL_LO);
}

/** Register a callback for one or many keys */
void keypad_register_callback(uint8_t key_masks, keypad_handler_t handler, void *param)
{
    uint8_t i;

    for (i = 0; i < KEYPAD_NUMBER_OF_KEYS; ++i) {
        if (keypad_keys_state[i].mask &= key_masks) {
            keypad_keys_state[i].handler = handler;
            keypad_keys_state[i].param = param;
        }
    }
}
