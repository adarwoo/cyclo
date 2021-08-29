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
   void draw_program_setup_dialog();
   void manual_program_draw_digit( show_digit_t show, uint8_t row, uint8_t column );
   void draw_usb();
};


#endif  // ndef ui_view_hpp__included
