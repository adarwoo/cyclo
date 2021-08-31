#ifndef parser_hpp_was_included
#define parser_hpp_was_included
/*
 * Parser for the relay control expressions
 *
 * Grammar:
 *   Separator is space or ,
 *   o<pen> / c<lose> commands
 *   h<elp>, l<ist>
 *   [0-9]+<HMsm> delay
 * open close
 * o 10 c
 *
 */
#include <cctype>

#include <logger.h>

#include <etl/algorithm.h>
#include <etl/basic_string.h>
#include <etl/string.h>
#include <etl/string_view.h>
#include <etl/tokenizer.h>
#include <etl/vector.h>

#include "program.hpp"


/**
 * Parser for the command line.
 * Size correctly using the template args.
 * The command created from the interpretation is internal to parser and can therefore
 *  be used before any new parsing.
 * Takes 8 bytes off the stack
 */
template<const size_t MAX_SIZE = 16, const size_t MAX_ERROR_BUFFER = 40>
class Parser
{
public:
   ///< Possible nom command interactions
   enum Interact : char ;

   ///< Result of the parsing
   enum Result : uint8_t {
      error=0,
      pgm = 255,
      help = 'h',
      list = 'l',
      save = 's',
      del = 'd',
      run = 'r' };

// Local data
protected:
   Program                          &live_;
   etl::istring                     &err_;
   etl::string_view::const_iterator buffer_{ nullptr };

   ///< Position of the error
   uint8_t distance;

   ///< Number of the program for program commands
   uint8_t program_number;

// Helpers
protected:
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

   /**
    * Check if the token is a delay.
    * An error can be triggered.
    * @return 0 if not, otherwise the correct delay in ms
    */
   bool get_delay( uint32_t &value, const etl::string_view token )
   {
      using Item = etl::pair<const char *, uint32_t>;
      bool retval = false;

      static constexpr Item unit_lut[] = {
         { "H", 60ul * 60ul * 1000ul },
         { "M", 60ul * 1000ul },
         { "s", 1000 },
         { "m", 1 },
         { "", 1000 } };

      // Check if the first char is a digit before perusing
      if ( ::isdigit( token.front() ) )
      {
         // Convert
         auto unit_start = token.begin();
         auto number     = strtoul( unit_start, const_cast<char **>( &unit_start ), 10 );
         auto unit       = etl::string_view( unit_start, token.end() );

         auto foundit =
            etl::find_if( etl::begin( unit_lut ), etl::end( unit_lut ), [ unit ]( const Item &i ) {
               return unit == i.first;
            } );

         if ( foundit != etl::end( unit_lut ) )
         {
            value = number * foundit->second;
            retval = true;
         }
         else
         {
            err_ = "Invalid unit: '";
            err_.append( unit.data(), unit.size() );
            err_ += "\'";
            error( unit_start );
         }
      }

      return retval;
   }

   bool get_program_number( const etl::string_view token )
   {
      auto retval = false;

      // Must be a single char 0->9
      if ( not ::isdigit( token.front() ) or token.size() > 1 )
      {
         err_ = "Expecting a program number";
         error( token.front() );
      }
      else
      {
         program_number = token.front() - '0';
         retval = true;
      }

      return retval;
   }

   void safe_insert( Command::command_t c, etl::string_view token )
   {
      if ( live_.full() )
      {
         err_.assign( "Too many items" );
         error( token );
      }
      else if ( live_.back().command == Command::loop )
      {
         err_.assign( "No commands allowed past *" );
         error( token );
      }
      else if ( live_.empty() and c == Command::loop )
      {
         err_.assign( "Loop not allowed as first action" );
         error( token );
      }
      else
      {
         if ( c != Command::delay )
         {
            // Force a 1 second delay unless given
            if ( not live_.empty() and live_.back().delay_ms == 0 )
            {
               LOG_DEBUG("parser", "Adding 1s delay");
               live_.back().delay_ms = 1000;
            }
         }

         LOG_DEBUG("parser", "Adding item: '%c'", c);

         // Insert a new command
         live_.push_back( Command( c ) );
      }
   }

public:
   // Construct a parser
   explicit Parser( Program &program, etl::istring &error) :  live_{program}, err_(error)
   {}

   // Access the error string
   uint8_t get_error_position() { return this->distance; }

   /**
    * Parse a single line passed as a string_view buffer
    * @return The parser result
    */
   Result parse(const etl::string_view &buffer)
   {
      auto retval = Result{error};
      buffer_     = buffer.begin();
      live_.clear();
      err_.clear();
      int8_t items_to_follow{0}; // 0 is open, -1 is no more, n is n

      LOG_INFO("parser", "Parsing: %s", etl::string<60>(buffer).c_str());

      for ( auto token : etl::tokenizer( buffer, etl::char_separator( " ," ) ) )
      {
         // Break on error
         if ( not err_.empty() )
            break;

         // Lambdas to scope things a bit
         auto is_command = [ token ]( etl::string_view command ) {
            return command.starts_with( token );
         };

         uint32_t number;

         if ( get_delay( number, token ) )
         {
            if ( not live_.empty() )
            {
               LOG_DEBUG("parser", "Adding %dms delay", number);

               live_.back().delay_ms += number;
            }
            else
            {
               safe_insert( Command::delay, token );
            }
         }
         else if ( err_.empty() )
         {
            if ( is_command( "open" ) )
            {
               safe_insert( Command::open, token );
            }
            else if ( is_command( "close" ) )
            {
               safe_insert( Command::close, token );
            }
            else if ( token == "*" )
            {
               // Loop - must be the last command
               safe_insert( Command::loop, token );
            }
            else
            {
               if ( not live_.empty() )
               {
                  err_ = "Unexpected: '";
                  err_.append( token.data(), token.size() );
                  err_ += "\'";
                  error( token );
               }
               else if ( is_command( "help" ) )
               {
                  last_item = true;

               }
               else if ( is_command( "list" ) )
               {
                  return list;
               }
               else
               {
                  err_ = "Unexpected: '";
                  err_.append( token.data(), token.size() );
                  err_ += "\'";
                  error( token );
               }
            }
         }
      }

      if ( not live_.empty )

      return retval;
   }
};


#endif  // ndef parser_hpp_was_included