#ifndef asx_h_was_included
#define asx_h_was_included
/*
 * asx.h
 * Replace asf but allow simulation
 *
 * Created: 26/08/2021 13:57:53
 *  Author: software@arreckx.com
 */

#ifdef _POSIX
#   include "sim.hpp"
#else
#   include "asf.h"
#endif

#endif /* ndef asx_h_was_included */