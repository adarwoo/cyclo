#ifndef KEYPAD_H_
#define KEYPAD_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Callback called when a key is pushed */
typedef void (*keypad_handler_t)(uint8_t key, void *param);

/** Initialise the keypad library */
void keypad_init(void);

/** 
 * Regsiter a callback for one or many key
 * key_masks Mask of the keys to handle
 * handler Callback function (from isr)
 * param Optional data to pass along
 */
void keypad_register_callback(uint8_t key_masks, keypad_handler_t handler, void *param);

#include "conf_keypad.h"

#ifdef __cplusplus
}
#endif

#endif /* KEYPAD_H_ */
