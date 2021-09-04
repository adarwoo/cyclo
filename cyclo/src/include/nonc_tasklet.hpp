/******************************************************************************
The MIT License(MIT)
https://github.com/adarwoo/cyclo

Copyright(c) 2021 Guillaume ARRECKX - software@arreckx.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/
#ifndef nonc_tasklet_hpp_included
#define nonc_tasklet_hpp_included

#include "contact.hpp"


#ifndef NONC_SAMPLE_FREQUENCY_HZ
#   define NONC_SAMPLE_FREQUENCY_HZ 100
#endif

/**
 * Tasklet which posts the status of the NoNc switch
 *  and reports further changes as message
 */
class NoNcTasklet : public rtos::Tasklet
{
   inline static NoNcTasklet *this_ = nullptr;

   // Cache the last result
   inline static bool was_no = true;

   ///< Wait for a first status
   inline static bool initialised = false;

   using no_nc_t = Contact::no_nc_t;

   // Contact
   Contact &contact;

public:
   NoNcTasklet( Contact &contact );

   static void read_nonc();

   virtual void run( uint32_t no_readback ) override;
};


#endif  // ndef nonc_tasklet_hpp_included