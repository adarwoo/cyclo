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
#include <etl/delegate.h>
#include <etl/string.h>
#include <etl/string_view.h>
#include <etl/tokenizer.h>
#include <etl/vector.h>

#include "program.hpp"

///< Possible nom command interactions
enum Interact : char { help = 'h', list = 'l', save = 's', del = 'd' };

using InteractHandler = etl::delegate<void( Interact )>;


/**
 * Parser for the command line.
 * Size correctly using the template args.
 * The command created from the interpretation is internal to parser and can therefore
 *  be used before any new parsing.
 */
template<const size_t MAX_SIZE = 16, const size_t MAX_ERROR_BUFFER = 40>
class Parser
{
   using ErrorString = etl::string<MAX_ERROR_BUFFER>;

   Program                          live_;
   ErrorString                      err_;
   etl::string_view::const_iterator buffer_{ nullptr };

   ///< Position of the error
   uint_least8_t distance;

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
   Parser() = default;

   // Access the parser command. Use only within a thread after a successfull parse
   Program &get_program() { return live_; }

   // Access the error string
   const char *get_error( uint_least8_t &position )
   {
      position = this->distance;
      return err_;
   }

   /**
    * Parse a single line passed as a string_view buffer
    * Since all the callbacks are in place, will call either:
    *  interact to list or show the help
    *  the command handler to handle the command
    * @return true on success and the handler
    */
   bool parse(
      const etl::string_view &buffer, const InteractHandler &interact_handler = []( Interact ) {} )
   {
      buffer_     = buffer.begin();
      live_.clear();
      err_.clear();

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
                  interact_handler( Interact::help );
               }
               else if ( is_command( "list" ) )
               {
                  interact_handler( Interact::list );
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

      return err_.empty();
   }
};


#endif  // ndef parser_hpp_was_included