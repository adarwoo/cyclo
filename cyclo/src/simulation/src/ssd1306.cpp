#include "ssd1306.h"

#include <rtos.hpp>

#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xutil.h>

#include <assert.h>
#include <logger.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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

void init_x()
{
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
         if ( keysym == XK_Escape )
            break;
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
}

static inline void inc_addr()
{
   ++col;
}

static bool once = false;

extern "C" void ssd1306_init( void )
{
   if ( once )
      return;

   once = true;

   LOG_HEADER( DOM );

   // Need a task to refresh the UI
   static auto ui_task = rtos::Task<typestring_is( "simui" )>();

   // Reset the framebuffer
   memset( fb, 0, sizeof( fb ) );

   init_x();

   // Start the task to process screen refresh etc.
   ui_task.run( handle_x );
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
