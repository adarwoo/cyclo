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

namespace manual_program
{
   namespace default_values
   {
      namespace on
      {
         constexpr auto minutes = 1;
         constexpr auto seconds = 0;
      }  // namespace on

      namespace off
      {
         constexpr auto minutes = 0;
         constexpr auto seconds = 5;
      }  // namespace off
   }     // namespace default_values
}  // namespace manual_program

using namespace manual_program::default_values;


/** Contruct a manual program as o 1 minute and off 5 seconds */
ManualProgram::ManualProgram()
   : on_min{ on::minutes }, on_sec{ on::seconds }, off_min{ off::minutes }, off_sec{ off::seconds }
{}


// Set the cached manual value from a internal program
void ManualProgram::import_program( const Program &pgm )
{
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
}

UIModel::UIModel( ProgramManager &pm ) : program_manager{ pm }
{
   // Get the index of the selected program
   program_index = program_manager.get_selected();
}


///< Persist the manual program to the EEProm
void UIModel::store_manual_pgm()
{
   char buffer[ 22 ];  // 'c 00M 01s o 00M 00s *' = 21 + 1 \0
   auto fmt = PROGMEM_STRING( "c %.2hhuM %.2hhus o %.2hhuM %.2hhus *" );
   snprintf_P( buffer, sizeof( buffer ), fmt, on_min, on_sec, off_min, off_sec );

   program_manager.write_pgm_at( 0, buffer );

   // Reload it
   program_manager.load( 0 );
}
