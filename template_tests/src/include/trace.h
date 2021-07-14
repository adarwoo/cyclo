/*
 * trace.h
 *
 * Created: 26/04/2020 22:39:15
 *  Author: micro
 */

#ifndef TRACE_H_
#define TRACE_H_

#include <asf.h>
#include <ioport.h>

static inline void trace_set(ioport_pin_t pin) { ioport_set_pin_level(pin, true); }
static inline void trace_clear(ioport_pin_t pin) { ioport_set_pin_level(pin, false); }
static inline void trace_tgl(ioport_pin_t pin) { ioport_toggle_pin_level(pin); }

#ifdef __cplusplus
/**
 * RAII style tracing
 */
struct Trace
{
	ioport_pin_t pin;

	Trace(const ioport_pin_t pin) : pin(pin)
	{
		trace_set(pin);
	}

	~Trace()
	{
		trace_clear(pin);
	}
};
#endif

#endif /* TRACE_H_ */