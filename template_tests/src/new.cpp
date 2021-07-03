#include <stdlib.h>

#include <new>

void *operator new(size_t size_) { return malloc(size_); }

/**
 * Global placement operator new
 */

void *operator new(size_t size_, void *ptr_) { return ptr_; }

/**
 * Global operator delete
 */

void operator delete(void *ptr_) { free(ptr_); }
