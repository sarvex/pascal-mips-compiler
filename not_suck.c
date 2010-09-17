#include "not_suck.h"

#include "stdio.h"

void * safe_malloc(size_t size) {
    void * object = malloc(size);
    if (object == null) {
        fprintf(stderr, "ERROR: malloc failed.\n");
        exit(1);
    }
    return object;
}

