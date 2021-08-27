/*-
 *****************************************************************************
 * Force the library initialisation
 *
 * @author gpa
 *****************************************************************************
 */

#ifdef _MSC_VER
/*
 * lpvReserved yields an unreferenced formal parameter;
 * ignore it
 */
#pragma warning( disable : 4100 )
#endif

#include <stdbool.h>

/**
 *****************************************************************************
 * Force DSO initialisation
 *
 *****************************************************************************
 */
extern void _log_init( bool splitPatternOff );

_DLL_InitTerm(HMODULE modhandle, DWORD fdwReason, LPVOID lpvReserved)
{	
    WSADATA			Data;	
    switch (fdwReason)	
    {		
        case DLL_PROCESS_ATTACH:		
        if (_CRT_init() == -1)			
           return 0L;
          /* start with initialization code */		
         _log_init( false );
          break;	
    case DLL_PROCESS_DETACH:		
         /* Start with termination code*/
         _CRT_term();		
         break;
    default:		
          /* unexpected flag value - return error indication */		
            return 0UL;	
    }	return 1UL;		/* success */
}


/* ---------------------------- End of file ------------------------------- */

