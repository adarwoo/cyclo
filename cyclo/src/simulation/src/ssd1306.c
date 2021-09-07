#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "ssd1306.h"

uint8_t fb[16][128] = {0};
uint8_t page = 0;
uint8_t col = 0;

static inline void inc_addr()
{
   ++col;
}

void ssd1306_init(void)
{
   memset(fb, 0, sizeof(fb));
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