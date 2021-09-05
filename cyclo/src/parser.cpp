#include "parser.hpp"

#include "program.hpp"

#include <etl/algorithm.h>
#include <etl/basic_string.h>
#include <etl/string.h>
#include <etl/string_view.h>
#include <etl/tokenizer.h>
#include <etl/vector.h>

#include <cctype>

#include <logger.h>

// Construct a parser
Parser::Parser( Program &program, etl::istring &error )
   : live_{ program }, err_( error ), buffer_{ nullptr }
{}

/**
 * Check if the token is a delay.
 * An error can be triggered
 *
 * @return 0 if not, otherwise the correct delay in ms
 */
bool Parser::get_delay( uint32_t &value, const etl::string_view token )
{
   using Item  = etl::pair<const char *, uint32_t>;
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

      // Is it a program number rather?
      if ( number < 10 and unit.size() == 0 )
      {
         program_number = number;
      }

      auto foundit =
         etl::find_if( etl::begin( unit_lut ), etl::end( unit_lut ), [ unit ]( const Item &i ) {
            return unit == i.first;
         } );

      if ( foundit != etl::end( unit_lut ) )
      {
         value  = number * foundit->second;
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

/**
 * @param token String containing the value to convert
 * @return true on success. If false, err_ contains the error descruption
 */
bool Parser::get_program_number( const etl::string_view token )
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
      retval         = true;
   }

   return retval;
}

/**
 * @param c Command to insert into the program
 * @param token Actual command element
 */
void Parser::safe_insert( Command::command_t c, etl::string_view token )
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
            LOG_DEBUG( "parser", "Adding 1s delay" );
            live_.back().delay_ms = 1000;
         }
      }

      LOG_DEBUG( "parser", "Adding item: '%c'", c );

      // Insert a new command
      live_.push_back( Command( c ) );
   }
}

/**
 * @param buffer The buffer to parse
 * @return A parsing can return an program, a interactive command, nothing
 *          or an error. The calling entity should switch case the result
 *          to process the result.
 *         If the result is error, the supplied error string will contain
 *          the error text description, along with the method get_error_position
 *          which can point to the actual location of the error in the supplied string.
 *         If the result is a command, and the command requires a program number,
 *          the accessor get_program_number will provide the value.
 *         An empty (or space only) string returns 'nothing'.
 *         Finally, a return value 'program' indicate the program is available from
 *          the supplied program object.
 */
Parser::Result Parser::parse( const etl::string_view &buffer )
{
   enum : uint8_t { more, no_more, program, program_1_to_9 } expects = more;


   auto retval = Result::program; // Default is to expect a program

   // Reset all to allow multiple calls
   buffer_     = buffer.begin();
   live_.clear();
   err_.clear();

   // Invalidate the program number
   program_number = 255;

   LOG_INFO( "parser", "Parsing: %s", etl::string<60>( buffer ).c_str() );

   for ( auto token : etl::tokenizer( buffer, etl::char_separator( " ," ) ) )
   {
      // Lambdas to scope things a bit
      auto is_command = [ token ]( etl::string_view command ) {
         return command.starts_with( token );
      };

      // Check extra args
      if ( expects == no_more )
      {
         err_ = "Unexpected extra arg(s): '";
         err_.append( token.data(), token.size() );
         err_ += "\'";
         error( token );
      }

      // Break on error
      if ( not err_.empty() )
      {
         break;
      }

      uint32_t number;

      if ( expects == program or expects == program_1_to_9 )
      {
         if ( not get_program_number() )
         {
            err_ = "Expecting a number ";

            if ( expects == program )
            {
               err_ += '0';
            }
            else
            {
               err_ += '1';
            }

            err_ += " to 9. Got '";
            err_.append( token.data(), token.size() );
            err_ += "\'";
            error( token );
         }
         else
         {
            expects = no_more;
         }
      }
      else if ( get_delay( number, token ) )
      {
         if ( not live_.empty() )
         {
            LOG_DEBUG( "parser", "Adding %dms delay", number );

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
               retval  = Result::help;
               expects = no_more;
            }
            else if ( is_command( "list" ) )
            {
               retval  = Result::list;
               expects = no_more;
            }
            else if ( is_command( "quit" ) )
            {
               retval  = Result::quit;
               expects = no_more;
            }
            else if ( is_command( "auto" ) )
            {
               retval  = Result::quit;
               expects = program;
            }
            else if ( is_command( "save" ) )
            {
               retval  = Result::save;
               expects = program_1_to_9;
            }
            else if ( is_command( "run" ) )
            {
               retval  = Result::run;
               expects = program;
            }
            else if ( is_command( "delete" ) )
            {
               retval  = Result::del;
               expects = program_1_to_9;
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

   // Any errors? - Return an error
   if ( not err_.empty() )
   {
      retval = Result::error;
   }
   else if ( retval == Result::program )
   {
      if ( live_.empty() )
      {
         retval = Result::nothing;
      }
      else
      {
         // If the command only has a delay - check if it is a program number
         if ( live_.size() == 1 and live_[ 0 ].command == Command::delay )
         {
            // Check the number
            if ( program_number == 255 )
            {
               err_ = "A delay must be followed by 'open' or 'close'";
            }
            else
            {
               // It is in fact a run command
               retval = Result::run;
            }
         }
      }
   }
   // else a interactive command was entered

   return retval;
}
