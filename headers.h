#ifndef cmonk_headers_h
#define cmonk_headers_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define createObject(type) \
    ({ \
        type* t = (type*)malloc(sizeof(type)); \
        if (t == NULL) { \
            fprintf(stderr, "ERROR: not enough memory.\n"); \
            exit(74); \
        } \
        t; \
    })

#endif