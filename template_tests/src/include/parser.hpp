#ifndef parser_hpp_is_included
#define parser_hpp_is_included
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

#include <etl/vector.h>
#include <etl/string_view.h>
#include <etl/string.h>
#include <etl/algorithm.h>
#include <etl/basic_string.h>
#include <etl/delegate.h>
#include <etl/tokenizer.h>

/**
 * A single command item to execute.
 * It carries the operation as well as a delay.
 * The delay is to be observed after the command
 */
struct CommandItem
{
   ///< Command for the engine
   enum class ECommand: char
   {
      delay='d', open = 'o', close = 'c' 
   } command;

   ///< Delay after the command execution
   uint32_t delay_ms;
   ///< Cycle counter duing execution
   uint32_t cycle {0};

   ///< Simple constructor
   explicit CommandItem(ECommand type, uint32_t delay = 0) : 
      command(type), delay_ms(delay)
   {}

   ///< Simple Enum converter for debug
   constexpr const char*to_string(ECommand c) const
   {
      switch (c)
      {
         case ECommand::open: return "Open";
         case ECommand::close: return "Close";
         case ECommand::delay: return "Delay";
      }

      return "Idle";
   }
};


///< Possible nom command interactions
enum Interact: char { help='h', list='l' };

/**
 * Parser for the command line.
 * Size correctly using the template args.
 * The command created from the interpretation is internal to parser and can therefore
 *  be used before any new parsing.
 */
template<const size_t MAX_SIZE=16, const size_t MAX_ERROR_BUFFER=40>
class Parser
{
public:
   using Command = etl::vector<CommandItem, MAX_SIZE>;
   using InteractHandler = etl::delegate<void(Interact)>;
   using ErrorHandler = etl::delegate<void(const etl::string_view, uint_least8_t)>;
   using CommandHandler = etl::delegate<void(const Command &)>;

private:
   using ErrorString = etl::string<MAX_ERROR_BUFFER>;

   Command live_;
   ErrorString err_;
   InteractHandler interact_handler_;
   CommandHandler command_handler_;
   ErrorHandler error_handler_;
   etl::string_view::const_iterator buffer_ {nullptr};

   void safe_insert(CommandItem::ECommand c, etl::string_view token )
   {
      if ( live_.full() )
      {
         err_.assign("Too many items");
         error(token);
      }
      else
      {
         if (c != CommandItem::ECommand::delay)
         {
            // Force a 1 second delay unless given
            if (not live_.empty() and live_.back().delay_ms == 0)
            {
               live_.back().delay_ms = 1000;
            }
         }

         // Insert a new command
         live_.push_back(CommandItem(c));
      }
   }

   template <typename T>
   void error(T where)
   {
      uint_least8_t distance;
      
      if constexpr(etl::is_same_v<T, etl::string_view>)
         distance = where.begin() - buffer_;
      else if constexpr(etl::is_pointer_v<T>)
         distance = where - buffer_;

      this->error_handler_(err_, distance);
   }

   /**
    * Check if the token is a delay.
    * An error can be triggered.
    * @return 0 if not, otherwise the correct delay in ms
    */
   uint32_t get_delay(const etl::string_view token)
   {
      auto retval = 0ul;

      using Item = etl::pair<const char*, uint32_t>;

      static constexpr Item unit_lut[] = {
         {"H", 60ul*60ul*1000ul}, {"M", 60ul*1000ul}, {"s", 1000}, {"m", 1}, {"", 1000}
      };

      // Check if the first char is a digit before perusing
      if ( ::isdigit(token.front()) )
      {
         // Convert
         auto unit_start = token.begin();
         auto number = strtoul(unit_start, const_cast<char**>(&unit_start), 10);
         auto unit = etl::string_view(unit_start, token.end());

         auto foundit = etl::find_if(etl::begin(unit_lut), etl::end(unit_lut),
            [unit](const Item &i) {return unit == i.first;}
         );

         if (foundit != etl::end(unit_lut))
         {
            retval =  number * foundit->second;
         }
         else
         {
            err_ ="Invalid unit: '";
            err_.append(unit.data(), unit.size());
            err_ +="\'";
            error(unit_start);
         }
      }

      return retval;
   }

public:
   Parser(
      InteractHandler interact_handler,
      CommandHandler command_handler,
      ErrorHandler error_handler
   ) :
      interact_handler_(interact_handler), 
      command_handler_(command_handler),
      error_handler_(error_handler)
   {}
   
   const Command &get_command() const { return live_; }

   /**
    * Parse a single line passed as a string_view buffer
    * Since all the callbacks are in place, will call either:
    *  error callback to display the error
    *  interact to list or show the help
    *  the command handler to handle the command
    */
   void parse(const etl::string_view &buffer)
   {
      buffer_ = buffer.begin();
      live_.clear();
      err_.clear();

      for (auto token : etl::tokenizer(buffer, etl::char_separator(" ,")))
      {
         // Break on error
         if (not err_.empty())
            break;

         // Lambdas to scope things a bit
         auto is_command = [token](etl::string_view command)
         { 
            return command.starts_with(token); 
         };

         if ( auto number = get_delay(token) )
         {
            if ( not live_.empty() )
            {
               live_.back().delay_ms += number;
            }
            else
            {
               safe_insert(CommandItem::ECommand::delay, token);
            }
         }
         else if (err_.empty())
         {
            if (is_command("open"))
            {
               safe_insert(CommandItem::ECommand::open, token);
            }
            else if (is_command("close"))
            {
               safe_insert(CommandItem::ECommand::close, token);
            }
            else
            {
               if (not live_.empty())
               {
                  err_ = "Unexpected: '";
                  err_.append(token.data(), token.size());
                  err_ +="\'";
                  error(token);
               }
               else if (is_command("help"))
               {
                  this->interact_handler_(Interact::help);
               }
               else if (is_command("list"))
               {
                  this->interact_handler_(Interact::list);
               }
               else
               {
                  err_ = "Unexpected: '";
                  err_.append(token.data(), token.size());
                  err_ +="\'";
                  error(token);
               }
            }
         }
      }

      // If the last item is not idle - and there is no pause, add 1 second worth
      if ( err_.empty() and not live_.empty() )
      {
         auto &item = live_.back();

         if (item.command != CommandItem::ECommand::delay and item.delay_ms == 0 )
         {
            item.delay_ms = 1000;
         }

         this->command_handler_(live_);
      }
   }
};

#endif // ndef parser_hpp_is_included