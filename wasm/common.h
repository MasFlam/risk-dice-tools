#pragma once

// These are just the standard library functions what we use.
// Their implementations have to be given to the WASM module.

#define EXPORTED __attribute__((used)) __attribute__((visibility("default")))

/* Typedefs */

//typedef __INT8_TYPE__ int8_t;
//typedef __INT16_TYPE__ int16_t;
//typedef __INT32_TYPE__ int32_t;
//typedef __INT64_TYPE__ int64_t;
//typedef __UINT8_TYPE__ uint8_t;
//typedef __UINT16_TYPE__ uint16_t;
//typedef __UINT32_TYPE__ uint32_t;
//typedef __UINT64_TYPE__ uint64_t;
//typedef __SIZE_TYPE__ size_t;
//typedef __PTRDIFF_TYPE__ ptrdiff_t;

/* Allocator (we use @thi.ng/malloc) */

//extern void *malloc(size_t size);
//
//extern void *calloc(size_t nmemb, size_t size);
//
//extern void *realloc(void *ptr, size_t size);
//
//extern void free(void *ptr);
//
//extern void abort() __attribute__((noreturn));

/* Math */

extern double pow(double x, double y);

#include <stdint.h>
#include <stdlib.h>
