#ifndef console_worker_hpp__included
#define console_worker_hpp__included
/*
 * UI Woker
 */
#include <rtos.hpp>
#include <typestring.hpp>

#include "console_server.hpp"
#include "program_manager.hpp"
#include "parser.hpp"

extern void console_putc(vt100::char_t);

class Console : public rtos::Task<typestring_is("console"), 256>
{
   using TTerminal = vt100::Terminal<console_putc>;
   using TConsoleServer = ConsoleServer<TTerminal>;
   using optional_buffer_view_t = TConsoleServer::optional_buffer_view_t;

   ///< Console error buffer
   etl::string<80> error_buffer;

   ///< Console own program place holder during parsing
   Program temp_program;

   ///< Console parser
   Parser parser;

   ///< Console server
   ConsoleServer<TTerminal> server;

   ///< THE program manager
   ProgramManager &program_manager;

public:
   explicit Console( ProgramManager &);
   virtual void default_handler() final;

protected:
   ///< Display the parsing error
   void show_error();

   ///< Process a full command line
   void process(etl::string_view line);

   void show_help();
   void show_list();
};


#endif  // ndef console_worker_hpp__included
