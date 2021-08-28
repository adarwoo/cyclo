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

   on_min = itemClose.delay_ms / 60000;
   on_sec = itemClose.delay_ms % 60000;

   off_min = itemOpen.delay_ms / 60000;
   off_sec = itemOpen.delay_ms % 60000;
}

UIModel::UIModel( ProgramManager &pm ) : program_manager{ pm }
{
   // Get the index of the selected program
   program_index = program_manager.get_selected();

   // Load the manual program to cache the values
   if ( program_manager.load( 0 ) )
   {
      // Load the auto program values into this model
      import_program( program_manager.get_program() );
   }
   else // Store the default program
   {
      store_manual_pgm();
   }
}


///< Persist the manual program to the EEProm
void UIModel::store_manual_pgm()
{
   char buffer[ 22 ];  // 'c 00M 01s o 00M 00s *' = 21 + 1 \0
   auto fmt = PROGMEM_STRING( "c %.2dM %.2ds o %.2dM %.2ds *" );
   snprintf_P( buffer, sizeof( buffer ), fmt, on_min, on_sec, off_min, off_sec );

   program_manager.write_pgm_at( 0, buffer );

   // Reload it
   program_manager.load( 0 );
}
