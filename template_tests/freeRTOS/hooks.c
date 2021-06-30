#include <FreeRTOS.h>
#include <task.h>

/** Implied in heap_1 */
extern void vApplicationMallocFailedHook( void );

void vApplicationMallocFailedHook( void )
{
	asm("break");
	while(1);
}

void vApplicationStackOverflowHook( TaskHandle_t xTask, char * pcTaskName )
{
	while(1);
}
