/*
 * contact.cpp
 *
 * Created: 23/08/2021 22:38:18
 *  Author: micro
 */
#include "contact.hpp"

#include "asx.h"


bool Contact::is_open() const
{
   return is_no() != ioport_get_pin_level( RELAY_CTRL );
}

void Contact::set_relay( bool close_contact )
{
   ioport_set_pin_level( RELAY_CTRL, close_contact );
}
