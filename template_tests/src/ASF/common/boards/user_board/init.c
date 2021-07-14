/**
 * \file
 *
 * \brief User board initialization template
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip
 * Support</a>
 */

#include <asf.h>
#include <board.h>
#include <conf_board.h>

void board_init(void)
{
	twi_options_t m_options = {
		.speed     = TWI_SPEED,
		.chip      = TWI_MASTER_ADDR,
		.speed_reg = TWI_BAUD(sysclk_get_cpu_hz(), TWI_SPEED)
	};
	
	/* This function is meant to contain board-specific initialization code
	 * for, e.g., the I/O pins. The initialization can rely on application-
	 * specific board configuration, found in conf_board.h.
	 */

   // Force a zero on output now
	ioport_set_pin_level(RELAY_CTRL, false);

    // Force a zero on output now
    ioport_set_pin_level(RELAY_CTRL, false);

    // Activate the relay command
    ioport_set_pin_dir(RELAY_CTRL, IOPORT_DIR_OUTPUT);

    // Activate pull-ups on the input pin
    ioport_set_pin_dir(JOYSTICK_UP, IOPORT_DIR_INPUT);
    ioport_set_pin_dir(JOYSTICK_DOWN, IOPORT_DIR_INPUT);
    ioport_set_pin_dir(JOYSTICK_PUSH, IOPORT_DIR_INPUT);
    ioport_set_pin_dir(SWITCH_SENSE_NO, IOPORT_DIR_INPUT);
    ioport_set_pin_dir(SWITCH_SENSE_NC, IOPORT_DIR_INPUT);

    // Active the pull-up on all those pins
    ioport_set_pin_mode(JOYSTICK_UP, IOPORT_MODE_PULLUP);
    ioport_set_pin_mode(JOYSTICK_DOWN, IOPORT_MODE_PULLUP);
    ioport_set_pin_mode(JOYSTICK_PUSH, IOPORT_MODE_PULLUP);

    ioport_set_pin_mode(SWITCH_SENSE_NO, IOPORT_MODE_PULLUP);
    ioport_set_pin_mode(SWITCH_SENSE_NC, IOPORT_MODE_PULLUP);

    // Activate trace pins for debug
    ioport_set_pin_dir(TRACE_TICK, IOPORT_DIR_OUTPUT);
    ioport_set_pin_dir(TRACE_IDLE, IOPORT_DIR_OUTPUT);
    ioport_set_pin_dir(TRACE_ERR, IOPORT_DIR_OUTPUT);

    // Activate the i2c for the OLED chip
    TWI_MASTER_PORT.PIN0CTRL = PORT_OPC_WIREDANDPULL_gc;
    TWI_MASTER_PORT.PIN1CTRL = PORT_OPC_WIREDANDPULL_gc;

    irq_initialize_vectors();

    sysclk_init();
    sysclk_enable_peripheral_clock(&TWI_MASTER);
    twi_master_init(&TWI_MASTER, &m_options);
    twi_master_enable(&TWI_MASTER);
    cpu_irq_enable();

    gfx_mono_ssd1306_init();

    sleepmgr_init();
	
	// Start the USB
	//udc_start();
}
