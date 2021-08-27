#ifndef ui_model_h_included
#define ui_model_h_included

#include <cstdint>
#include <cstdio>

#include <etl/bitset.h>

#include "cyclo_manager.hpp"

#ifdef _POSIX
#   define PROGMEM_STRING( x ) x
#   define snprintf_P          snprintf
#endif


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

   // Set the cached manual value from a internal program
   void from_command( const Command &cmd )
   {
      CommandItem itemClose = cmd.at( 0 );
      CommandItem itemOpen  = cmd.at( 1 );

      on_min = itemClose.delay_ms / 60000;
      on_sec = itemClose.delay_ms % 60000;

      off_min = itemOpen.delay_ms / 60000;
      off_sec = itemOpen.delay_ms % 60000;
   }

   ManualProgram() : on_min{ 1 }, on_sec{ 0 }, off_min{ 0 }, off_sec{ 5 } {}
};


/**
 * The model feeds of the programs manager and adds the manual mode as a struct
 */
class UIModel : public ManualProgram
{
   ///< Direct access rather than through the singleton
   CycloManager &cm;
   ///< The program being displayed
   uint8_t pgm;


public:
   using program_state_t = CycloManager::program_state_t;

   explicit UIModel( CycloManager &cm ) : cm{ cm }, pgm{ cm.get_selected() }
   {
      // Load the manual program to cache the values
      if ( cm.load( 0 ) )
      {
         // Load the auto program values into this model
         from_command( cm.get_command() );
      }
   }

   ///< Get the current program to display
   inline uint8_t get_pgm() { return pgm; }
   ///< Get the next program to display
   inline uint8_t get_next() { return cm.get_next( pgm ); }
   ///< Get the previous program to display
   inline uint8_t get_prev() { return cm.get_prev( pgm ); }
   ///< Set the program shown
   inline void set_pgm( uint8_t newPgm ) { pgm = newPgm; }
   ///< Select the new program (not just shown)
   inline void select_pgm() { cm.set_selected( pgm ); }

   ///< Persist the manual program to the EEProm
   void store_manual_pgm()
   {
      char buffer[ 22 ];  // 'c 00M 01s o 00M 00s *' = 21 + 1 \0
      auto fmt = PROGMEM_STRING( "c %.2dM %.2ds o %.2dM %.2ds *" );
      snprintf_P( buffer, sizeof( buffer ), fmt, on_min, on_sec, off_min, off_sec );

      cm.write_pgm_at( 0, buffer );

      // Reload it
      cm.load( 0 );
   }

   ///< Load the currently selected command
   void load_command() { cm.load( get_pgm() ); }

   inline program_state_t get_state() { return cm.get_state(); }
   inline void            set_state( program_state_t newState ) { cm.set_state( newState ); }

   inline uint16_t get_counter() { return cm.get_counter(); }
   inline void     reset_counter() { cm.set_counter( 0 ); }

   ///< Contact accessor
   inline bool contact_is_open() const { return cm.get_contact().is_open(); }
   inline bool contact_is_no() const { return cm.get_contact().is_no(); }

   ///< Manually override the contact
   void flip_contact() const { cm.get_contact().flip(); }
};


#endif  // ndef ui_model_h_included