#include <string.h>
#include <assert.h>
#include <stdio.h>

#include <X11/Xlib.h>

#include "ssd1306.h"

uint8_t fb[16][128] = {0};
uint8_t page = 0;
uint8_t col = 0;

Display *d;
Window w;
XEvent e;
XImage *img;

static inline void inc_addr()
{
   ++col;
}


void ssd1306_init(void)
{
   memset(fb, 0, sizeof(fb));

   d = XOpenDisplay(NULL);

   if (d == NULL) {
      fprintf(stderr, "Cannot open display\n");
      exit(1);
   }

   int s = DefaultScreen(d);
   w = XCreateSimpleWindow(d, RootWindow(d, s), 100, 100, 48*4, 64*4, 1, 777215, 111111);
   XSelectInput(d, w, ExposureMask | KeyPressMask);
   XMapWindow(d, w);

   char *data = (char*)malloc(256*256*4);

	XImage *img = XCreateImage(display,visual,DefaultDepth(display,screen_num),ZPixmap,
		0,data,256,256,32,0);

   // Start a task
   // TODO
}

void display_task()
{
       while (1) {
        XNextEvent(d, &e);
        if (e.type == Expose) {
           // Redraw the framebuffer
//            XFillRectangle(d, w, DefaultGC(d, s), 20, 20, 100, 100);
//            XDrawString(d, w, DefaultGC(d, s), 230, 250, msg, strlen(msg));

        }
        if (e.type == KeyPress)
            break;
    }

    XCloseDisplay(d);
    return 0;
}

void ssd1306_set_display_start_line_address(uint8_t address)
{
}

void ssd1306_set_page_address(uint8_t address)
{
	address &= 0x0F;
   page = address;
}

void ssd1306_set_column_address(uint8_t address)
{
	address &= 0x7F;
   col = address;
}

void ssd1306_write_data(uint8_t data)
{
   fb[page][col] = data;
   inc_addr();

   //XAddPixel(img,(long)i);

   //XPutImage(display,win,DefaultGC(display,screen_num),img,0,0,0,0,256,256);
}

uint8_t ssd1306_read_data()
{
   uint8_t retval = fb[page][col];
   inc_addr();
   return retval;
}

void ssd1306_print()
{
   uint8_t a, c, r;

   for ( a=0; a<8; ++a)
   {
      for (r=1; r!=0; r<<=1)
      {
         for ( c=0; c<48; ++c )
         {
            putchar( fb[a][c] & r ? 'X' : ' ');
         }

         printf("\n");
      }
   }
}