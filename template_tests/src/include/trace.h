#ifndef trace_h_was_included
#define trace_h_was_included
/*
 * trace.h
 *
 * Created: 26/04/2020 22:39:15
 *  Author: micro
 */

#include "asx.h"

static inline void trace_set(ioport_pin_t pin) { ioport_set_pin_level(pin, true); }
static inline void trace_clear(ioport_pin_t pin) { ioport_set_pin_level(pin, false); }
static inline void trace_tgl(ioport_pin_t pin) { ioport_toggle_pin_level(pin); }

#ifdef __cplusplus
extern "C"
#endif
void trace_assert(bool test, ioport_pin_t pin);

// Overwrite the assert macro
// TRACE_ERR must be configured
#undef assert
#define assert(cond) trace_assert(cond, TRACE_ERR)

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

#ifdef __cplusplus
extern "C"
#endif

void trace_inspect(const void *p, size_t len);


#endif /* ndef trace_h_was_included */