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
   using TParser = Parser<>;
   using TConsoleServer = ConsoleServer<TTerminal>;
   
   using optional_buffer_view_t = TConsoleServer::optional_buffer_view_t;

   ConsoleServer<TTerminal> server;
   TParser parser;
   ProgramManager &program_manager;

public:
   explicit Console( ProgramManager &program_manager );
   virtual void default_handler() final;
   void interact(Interact i);
   
protected:
   void show_error();
};


#endif  // ndef console_worker_hpp__included
