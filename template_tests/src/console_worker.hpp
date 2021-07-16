#ifndef console_worker_hpp__included
#define console_worker_hpp__included
/*
 * UI Woker
 */
#include "fx.hpp"
#include "msg_defs.hpp"


class ConsoleWorker : public fx::Worker<ConsoleWorker, msg::CDCChar>
{
public:
   void on_receive(const msg::CDCChar &msg)
   {
   }
};


#endif // ndef console_worker_hpp__included
