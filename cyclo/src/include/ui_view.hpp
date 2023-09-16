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
#ifndef ui_view_hpp__included
#define ui_view_hpp__included
/*
 * UI View
 *
 * Created: 29/06/2021 21:26:21
 * Author : software@arreckx.com
 */
#include "ui_model.hpp"


class UIView
{
   UIModel &model;

public:
   ///< Use with manual_program_draw_digit
   enum show_digit_t : uint8_t { first_normal, first_highlight, next_highlight, back_to_normal };

   explicit UIView( UIModel &model );

   void draw();
   void draw_splash();
   void draw_prog( bool highlight = false );
   void draw_contact();
   void draw_nonc();
   void draw_counter();
   void draw_walkman( uint8_t select = 0 );
   void draw_box();
   void draw_cursor( uint8_t pos );
   void erase_cursor( uint8_t pos );
   void draw_curson_hint( uint8_t pos );
   void erase_curson_hint( uint8_t pos );
   void draw_program_setup_dialog();
   void manual_program_draw_digit( show_digit_t show, uint8_t row, uint8_t column );
   void draw_usb();
};


#endif  // ndef ui_view_hpp__included
