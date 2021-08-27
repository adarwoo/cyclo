/**
 * @file
 * @{
 * Test the embedded version of the logger where all is turned off
 */
#include "logger.h"

LOG_ONLY(const char * DOM = "X");

int main()
{
   LOG_INIT();
   LOG_SETLEVEL(LOG_LEVEL_DEBUG);
   LOG_PRINT("Hello world! in %d", 2020);
   LOG_ERROR(DOM, "ERROR");
   LOG_WARN(DOM, "ERROR");
   LOG_MILE(DOM, "ERROR");
   LOG_INFO(DOM, "ERROR");
   LOG_TRACE(DOM, "ERROR");
   LOG_DEBUG(DOM, "ERROR");
   LOG_MASK("X");
   LOG_NOTMASK("X");
   LOG_SETLEVEL(LOG_LEVEL_INFO);
   LOG_ONLY(int r = LOG_GETLEVEL());
   LOG_ONLY(int r = LOG_GETLIMIT("X"));
   LOG_ASSERT(0);

   return 0;
}
