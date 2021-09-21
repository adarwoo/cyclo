#include "asx.h"

#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xutil.h>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <logger.h>

#include "keypad.h"
#include "ssd1306.h"

#include <rtos.hpp>


const char *const DOM = "X";

uint8_t  fb[ 16 ][ 128 ] = { 0 };
uint8_t  page            = 0;
uint8_t  col             = 0;
Display *dpy;
Window   win;
GC       gc;

int     scr;
Atom    WM_DELETE_WINDOW;
XImage *img;

extern "C" void sim_key_press(uint8_t key);

void init_x()
{
   // Allow threads
   XInitThreads();

   dpy = XOpenDisplay( NULL );

   if ( dpy == NULL )
   {
      fprintf( stderr, "Cannot open display\n" );
      exit( 1 );
   }

   scr = DefaultScreen( dpy );
   win = XCreateSimpleWindow(
      dpy, RootWindow( dpy, scr ), 10, 10, 200, 200, 1, BlackPixel( dpy, scr ),
      WhitePixel( dpy, scr ) );
   XSelectInput( dpy, win, ExposureMask | KeyPressMask );
   XMapWindow( dpy, win );

   XStoreName( dpy, win, "Cyclo" );

   gc = DefaultGC( dpy, scr );

   WM_DELETE_WINDOW = XInternAtom( dpy, "WM_DELETE_WINDOW", False );
   XSetWMProtocols( dpy, win, &WM_DELETE_WINDOW, 1 );

   int   dplanes = DisplayPlanes( dpy, scr );
   char *data    = (char *)malloc( 256 * 256 * 4 );
   img = XCreateImage( dpy, DefaultVisual( dpy, scr ), dplanes, ZPixmap, 0, data, 48, 64, 8, 0 );
}

void handle_x()
{
   XEvent e;

   while ( 1 )
   {
      XNextEvent( dpy, &e );

      if ( e.type == Expose )
      {
         uint8_t a, c, r;

         for ( a = 0; a < 8; ++a )
         {
            for ( r = 0; r < 8; ++r )
            {
               for ( c = 0; c < 48; ++c )
               {
                  XPutPixel(
                     img, c, a * 8 + r,
                     fb[ a ][ c ] & ( 1 << r ) ? WhitePixel( dpy, scr ) : BlackPixel( dpy, scr ) );
               }
            }
         }

         XPutImage( dpy, win, gc, img, 0, 0, 20, 20, 48, 64 );
      }

      if ( e.type == KeyPress )
      {
         char   buf[ 128 ] = { 0 };
         KeySym keysym;
         XLookupString( &e.xkey, buf, sizeof buf, &keysym, NULL );

         // Esc ends the event loop and exits
         if ( keysym == XK_Escape )
            break;

         switch ( keysym )
         {
         case XK_KP_Enter:
         case XK_Return:
            sim_key_press(KEY_SELECT); break;
         case XK_Down: sim_key_press(KEY_DOWN); break;
         case XK_Up: sim_key_press(KEY_UP); break;
         default:          break;
         }
      }

      if (
         ( e.type == ClientMessage )
         && ( static_cast<unsigned int>( e.xclient.data.l[ 0 ] ) == WM_DELETE_WINDOW ) )
      {
         break;
      }
   }

   XDestroyWindow( dpy, win );
   XCloseDisplay( dpy );

   // Close the application
   exit(0);
}

static inline void inc_addr()
{
   ++col;
}

static bool once = false;

// Need a task to refresh the UI
static auto ui_task = rtos::Task<typestring_is( "simui" )>(
   etl::delegate<void()>::create<handle_x>()
);

extern "C" void ssd1306_init( void )
{
   if ( once )
      return;

   once = true;

   LOG_HEADER( DOM );

   // Reset the framebuffer
   memset( fb, 0, sizeof( fb ) );

   init_x();
}


extern "C" void ssd1306_set_display_start_line_address( uint8_t address )
{}

extern "C" void ssd1306_set_page_address( uint8_t address )
{
   address &= 0x0F;
   page = address;
}

extern "C" void ssd1306_set_column_address( uint8_t address )
{
   address &= 0x7F;
   col = address;
}

extern "C" void ssd1306_write_data( uint8_t data )
{
   fb[ page ][ col ] = data;

   for ( int r = 0; r < 8; ++r )
   {
      XPutPixel(
         img, col, page * 8 + r,
         fb[ page ][ col ] & ( 1 << r ) ? WhitePixel( dpy, scr ) : BlackPixel( dpy, scr ) );
   }

   XPutImage( dpy, win, gc, img, 0, 0, 20, 20, 48, 64 );

   inc_addr();
}

extern "C" uint8_t ssd1306_read_data()
{
   uint8_t retval = fb[ page ][ col ];
   inc_addr();
   return retval;
}

extern "C" void ssd1306_write_data_buffer(const uint8_t *data, uint8_t size)
{
   while ( size-- )
   {
      if ( data == NULL )
      {
         fb[ page ][ col ] = 0;
      }
      else
      {
         fb[ page ][ col ] = *data++;
      }

      for ( int r = 0; r < 8; ++r )
      {
         XPutPixel(
            img, col, page * 8 + r,
            fb[ page ][ col ] & ( 1 << r ) ? WhitePixel( dpy, scr ) : BlackPixel( dpy, scr ) );
      }

      inc_addr();
   }

   XPutImage( dpy, win, gc, img, 0, 0, 20, 20, 48, 64 );
}