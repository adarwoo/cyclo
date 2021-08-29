#include "logger.h"

int testLog( int level )
{
   switch( level )
   {
      case 0: LOG_ERROR( "C", "An error message" ); break;
      case 1: LOG_WARN( "C", "An warn message" ); break;
      case 2: LOG_MILE( "C", "An mile message" ); break;
      case 3: LOG_INFO( "C", "An info message" ); break;
      case 4: LOG_TRACE( "C", "An trace message" ); break;
      case 5: LOG_DEBUG( "C", "A debug message" ); break;
      default:
         LOG_ERROR("C", "Too high");
   }

   return level;

}

