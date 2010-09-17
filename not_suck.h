#ifndef _NOT_SUCK_H_
#define _NOT_SUCK_H_

#include "stdlib.h"

#define true 1
#define false 0
#define null 0

#define NEW(type) (type*)safe_malloc(sizeof(type))
void * safe_malloc(size_t size);

#endif

