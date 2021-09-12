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

   ///< Copy of the last valid program string
   TConsoleServer::buffer_t last_program;

public:
   explicit Console( ProgramManager &);
   virtual void default_handler() final;

protected:
   ///< Display the parsing error
   void show_error();

   ///< Print a simple error
   void print_error( const char error[], bool is_pgm_str = true );

   ///< Process a full command line
   void process(etl::string_view line);

   void show_help();
   void show_list();
};


#endif  // ndef console_worker_hpp__included
