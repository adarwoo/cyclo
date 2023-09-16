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
#ifndef ui_model_h_included
#define ui_model_h_included

#include "asx.h"

#include <etl/bitset.h>

#include <cstdint>
#include <cstdio>

#include "program_manager.hpp"


struct ManualProgram
{
   ///< Read as an array or as a struct
   union
   {
      uint8_t value[ 2 ][ 2 ];
      struct
      {
         uint8_t on_min;
         uint8_t on_sec;
         uint8_t off_min;
         uint8_t off_sec;
      };
   };

   ManualProgram();
};


/**
 * The model feeds of the programs manager and adds the manual mode as a struct
 */
class UIModel : public ManualProgram
{
   ///< Direct access rather than through the singleton
   ProgramManager &program_manager;
   ///< The program being displayed
   int8_t program_index;

public:
   using program_state_t = ProgramManager::program_state_t;

   explicit UIModel( ProgramManager &pm );

   ///< Get the current program to display
   inline int8_t get_pgm() { return program_index; }
   ///< Get the next program to display
   inline int8_t get_next() { return program_manager.get_next( program_index ); }
   ///< Get the previous program to display
   inline int8_t get_prev() { return program_manager.get_prev( program_index ); }
   ///< Set the program shown
   inline void set_pgm( int8_t newPgm ) { program_index = newPgm; }
   ///< Select the new program (not just shown)
   void select_pgm();

   ///< Persist the manual program to the EEProm
   void store_manual_pgm();

   ///< Load the currently selected command
   void load_command() { program_manager.load( get_pgm() ); }

   inline program_state_t get_state() { return program_manager.get_state(); }
   inline void set_state( program_state_t newState ) { program_manager.set_state( newState ); }

   inline void set_last_used() { program_manager.set_lastused( program_index ); }


   inline int32_t get_counter() { return program_manager.get_counter(); }
   inline void    reset_counter( bool invalidate = false )
   {
      program_manager.set_counter( invalidate ? -1 : 0 );
   }

   ///< Contact accessor
   inline bool contact_is_open() const { return program_manager.get_contact().is_open(); }
   inline bool contact_is_no() const { return program_manager.get_contact().is_no(); }

   ///< Manually override the contact
   void flip_contact() const { program_manager.get_contact().flip(); }
};


#endif  // ndef ui_model_h_included