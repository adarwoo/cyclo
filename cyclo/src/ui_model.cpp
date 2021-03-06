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

/**
 * @file
 * @{
 * This is the M in MVC. It accesses the data accross the application and serves as
 *  a unique proxy to the program_manager, and the contact object.
 * @author software@arreckx.com
 */
#include "ui_model.hpp"

#include "asx.h"

#include <etl/format_spec.h>
#include <etl/string.h>
#include <etl/to_string.h>


/** Contruct a manual program as o 1 minute and off 5 seconds */
ManualProgram::ManualProgram() : on_min{ 0 }, on_sec{ 0 }, off_min{ 0 }, off_sec{ 0 }
{}

UIModel::UIModel( ProgramManager &pm ) : program_manager{ pm }
{
   // Load the manual program. The programs manager guarantees it exists
   program_manager.load( 0 );

   // Import the manual program
   const Program &pgm = program_manager.get_active_program();

   Command itemClose = pgm.at( 0 );
   Command itemOpen  = pgm.at( 1 );

   auto to_min_sec = []( uint32_t ms, uint8_t &minutes ) {
      // Discard ms
      uint16_t seconds = ms / 1000;
      minutes          = seconds / 60;
      return seconds % 60;
   };

   // Work with seconds (not ms)
   on_sec  = to_min_sec( itemClose.delay_ms, on_min );
   off_sec = to_min_sec( itemOpen.delay_ms, off_min );

   // Get the index of the selected program
   program_index = program_manager.get_selected();
}

///< Persist the manual program to the EEProm
void UIModel::store_manual_pgm()
{
   etl::string<22>  pgmStr{ "c " };  // 'c 00M 01s o 00M 00s *' = 21 + 1 \0
   etl::format_spec format;

   format.fill( '0' ).width( 2 ).decimal();

   etl::to_string( on_min, pgmStr, format, true );
   pgmStr += "M ";
   etl::to_string( on_sec, pgmStr, format, true );
   pgmStr += "s o ";
   etl::to_string( off_min, pgmStr, format, true );
   pgmStr += "M ";
   etl::to_string( off_sec, pgmStr, format, true );
   pgmStr += "s *";

   program_manager.write_pgm_at( 0, pgmStr.c_str() );

   // Reload it
   program_manager.load( 0 );
}

void UIModel::select_pgm()
{
   // If the program is back - reverse back to the current
   if ( program_index < 0 )
   {
      program_index = program_manager.get_selected();
   }
   else
   {
      // TO DO -> Overwrite the pgm to mark it as the default selected
      program_manager.set_selected( program_index );
   }
}