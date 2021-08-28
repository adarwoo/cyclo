#ifndef ui_model_h_included
#define ui_model_h_included

#include <cstdint>
#include <cstdio>

#include <etl/bitset.h>

#include "program_manager.hpp"
#include "asx.h"


struct ManualProgram
{
   bool repeat;  ///< If true, the sequence is looped

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

   // Set the cached manual value from a internal program
   void import_program( const Program &program_index );
};


/**
 * The model feeds of the programs manager and adds the manual mode as a struct
 */
class UIModel : public ManualProgram
{
   ///< Direct access rather than through the singleton
   ProgramManager &program_manager;
   ///< The program being displayed
   uint8_t program_index;


public:
   using program_state_t = ProgramManager::program_state_t;

   explicit UIModel( ProgramManager &pm );

   ///< Get the current program to display
   inline uint8_t get_pgm() { return program_index; }
   ///< Get the next program to display
   inline uint8_t get_next() { return program_manager.get_next( program_index ); }
   ///< Get the previous program to display
   inline uint8_t get_prev() { return program_manager.get_prev( program_index ); }
   ///< Set the program shown
   inline void set_pgm( uint8_t newPgm ) { program_index = newPgm; }
   ///< Select the new program (not just shown)
   inline void select_pgm() { program_manager.set_selected( program_index ); }

   ///< Persist the manual program to the EEProm
   void store_manual_pgm();

   ///< Load the currently selected command
   void load_command() { program_manager.load( get_pgm() ); }

   inline program_state_t get_state() { return program_manager.get_state(); }
   inline void            set_state( program_state_t newState ) { program_manager.set_state( newState ); }

   inline uint16_t get_counter() { return program_manager.get_counter(); }
   inline void     reset_counter() { program_manager.set_counter( 0 ); }

   ///< Contact accessor
   inline bool contact_is_open() const { return program_manager.get_contact().is_open(); }
   inline bool contact_is_no() const { return program_manager.get_contact().is_no(); }

   ///< Manually override the contact
   void flip_contact() const { program_manager.get_contact().flip(); }
};


#endif  // ndef ui_model_h_included