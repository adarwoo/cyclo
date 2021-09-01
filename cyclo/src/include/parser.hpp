#ifndef parser_hpp_was_included
#define parser_hpp_was_included
/*
 * Parser for the relay control expressions
 * See the readme for the syntax
 */
#include <etl/algorithm.h>
#include <etl/string.h>
#include <etl/string_view.h>

#include "program.hpp"


/**
 * Parser instance for parsing a program or the command line
 * The parser object can be reused, but will overwrite the
 *  supplied program and error.
 */
class Parser
{
public:
   ///< Result of the parsing
   enum class Result : int8_t {
      error   = -1,
      nothing = 0,
      program = 1,
      help    = 'h',
      list    = 'l',
      save    = 's',
      del     = 'd',
      run     = 'r',
      quit    = 'q',
      autostart = 'a',
   };

// Local data
protected:
   ///< Reference to the program to construct
   Program                          &live_;

   ///< Storage for the error message
   etl::istring                     &err_;

   ///< Internal buffer
   etl::string_view::const_iterator buffer_;

   ///< Position of the error
   uint8_t distance;

   ///< Number of the program for program commands
   uint8_t program_number;

public:
   ///< Construct a parser
   explicit Parser( Program &program, etl::istring &error );

   ///< Access the error string
   uint8_t get_error_position() { return distance; }

   ///< Access the program number
   uint8_t get_program_number() { return program_number; }

   /**
    * Parse a single line passed as a string_view buffer
    * @return The parser result
    */
   Result parse(const etl::string_view &buffer);

// Helpers
protected:
   ///< Check if the token is a delay
   bool get_delay( uint32_t &value, const etl::string_view token );

   ///< Get the expected program number from the token
   bool get_program_number( const etl::string_view token );

   ///< Build the program checking for error conditions
   void safe_insert( Command::command_t c, etl::string_view token );

   ///< Compute the error distance
   template<typename T>
   void error( T where )
   {
      if constexpr ( etl::is_same_v<T, etl::string_view> )
      {
         distance = where.begin() - buffer_;
      }
      else if constexpr ( etl::is_pointer_v<T> )
      {
         distance = where - buffer_;
      }
   }
};


#endif  // ndef parser_hpp_was_included