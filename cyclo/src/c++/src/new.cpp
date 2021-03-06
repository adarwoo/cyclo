
#include <cstdlib>
#include <new>
#include <cassert>

#ifdef _POSIX
   #include "logger.h"

   void operator delete(void *, unsigned int)
   {
      LOG_ERROR(0, "Delete invalid");
      assert(0);
   }

#else
   void *operator new(size_t size_) { return malloc(size_); }

   /**
    * Global placement operator new
    */

   void *operator new(size_t size_, void *ptr_) { return ptr_; }

   /**
    * Global operator delete
    */

   void operator delete(void *ptr_) { free(ptr_); }

   /** Delete a void */
   void operator delete(void *, unsigned int)
   {
      asm volatile("break");
   }

   /** Called if a pure virtual is called */
   extern "C" void __cxa_pure_virtual()
   {
      asm volatile("break");
      assert(0);
   }
#endif