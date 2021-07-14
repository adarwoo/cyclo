
#include <udi_cdc.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include "term.hpp"
#include "parser.hpp"

using char_t = vt100::char_t;

// Configuration
constexpr size_t max_line_length = 16;

//
// Static queue setup
//
namespace
{
   constexpr auto queue_size = 8;
   constexpr auto size_of_char_t = sizeof(char_t);
   static StaticQueue_t console_queue_storage;
   static QueueHandle_t console_queue;

   ///< Storage for the queue. Using bytes.
   uint8_t console_input_queue[queue_size * size_of_char_t];

   //
   // Static task setup
   //
   const char *const consoleTaskName = "con";

   constexpr auto stack_size = configMINIMAL_STACK_SIZE + 255;
   StackType_t console_task_stack[stack_size];
   StaticTask_t console_task_storage;
}

/**
   * CDC has received a character
   * Add to the queue
   */
extern "C" void console_cdc_rx_notify_callback(uint8_t port)
{
   // Get the character
   char_t data = (char_t)udi_cdc_multi_getc(0);

   // Send to the task
   xQueueSendFromISR(console_queue, &data, nullptr);
}

void console_cdc_putc(char_t c)
{
   udi_cdc_multi_putc(0, (int)c);
}

using TTerminal = vt100::Terminal<console_cdc_putc>;

//
// CDC interface
//
// Specify a console which output using the CDC

using TConsole = Console<TTerminal, max_line_length>;
using TParser = Parser<max_line_length>;

extern "C" void console_task(void *unused)
{
   auto console = TConsole();

   auto interact = [](Interact i)
   {
      //printf("Interact : %c", (char)i);
   };

   auto show_command = [](const TParser::Command &cmd) {
   };

   auto show_error = [](const etl::string_view msg, uint_least8_t distance)
   {
      for (auto i = 0; i < distance + 2; ++i)
      {
         TTerminal::putc('-');
         TTerminal::putc('^');
         //Terminal::print_P(PSTR("! Error: %s\n", msg.begin());
      }
   };

   auto parser = TParser(interact, show_command, show_error);

   TConsole::optional_buffer_view_t line;

   for (;;)
   {
      console.print_prompt();

      do
      {
         TConsole::char_t c;
         xQueueReceive(console_queue, (void *)&c, portMAX_DELAY);
         line = console.process_input(c);
      } while (line);

      // Now parse the context
      parser.parse(*line);
   }
}

/**
 * To be called before initializing the CDC since the CDC posts in the queue this function creates
 */
void console_init()
{
   console_queue = xQueueCreateStatic(
       queue_size, size_of_char_t, &(console_input_queue[0]), &console_queue_storage);

   // Fire the task
   xTaskCreateStatic(
       &console_task,
       consoleTaskName,
       stack_size,
       nullptr,
       tskIDLE_PRIORITY + 1,
       &(console_task_stack[0]),
       &console_task_storage);
}
