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
   int8_t program_number;

public:
   ///< Construct a parser
   explicit Parser( Program &program, etl::istring &error );

   ///< Access the error string
   uint8_t get_error_position() { return distance; }

   ///< Access the program number. Guaranteed since the manual program always exists. Can be zero if the program is off.
   int8_t get_program_number() { return program_number; }

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
   bool parse_program_number( const etl::string_view token );

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