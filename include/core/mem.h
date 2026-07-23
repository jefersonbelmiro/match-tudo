#pragma once

#include "core/defs.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG

typedef struct {
  uint64_t alloc_count;     /**< mem_alloc calls.                       */
  uint64_t calloc_count;    /**< mem_calloc calls.                      */
  uint64_t realloc_count;   /**< mem_realloc calls.                     */
  uint64_t free_count;      /**< mem_free calls on non-NULL pointers.   */
  uint64_t bytes_requested; /**< Total bytes passed to alloc + calloc.  */
} mem_stats_t;

static mem_stats_t _mem_stats;

static
inline void mem_print_stats(void)
{
  int64_t active = (_mem_stats.alloc_count + _mem_stats.calloc_count) - _mem_stats.free_count;
  printf("[mem]\n");
  printf("  alloc           : %llu\n", (unsigned long long)_mem_stats.alloc_count);
  printf("  calloc          : %llu\n", (unsigned long long)_mem_stats.calloc_count);
  printf("  realloc         : %llu\n", (unsigned long long)_mem_stats.realloc_count);
  printf("  free            : %llu\n", (unsigned long long)_mem_stats.free_count);
  printf("  active          : %lli\n", (unsigned long long)active);
  printf("  bytes requested : %llu\n", (unsigned long long)_mem_stats.bytes_requested);
  fflush(stdout);
}
#else

#  define mem_print_stats(void) (void)0

#endif // DEBUG


// ---------------------------------------------------------------------------
// Wrappers
// ---------------------------------------------------------------------------

static
inline void * mem_alloc(size_t size)
{
#ifdef DEBUG
  _mem_stats.alloc_count++;
  _mem_stats.bytes_requested += size;
#endif
  return malloc(size);
}

static
inline void * mem_calloc(size_t count, size_t size)
{
#ifdef DEBUG
  _mem_stats.calloc_count++;
  _mem_stats.bytes_requested += count * size;
#endif
  return calloc(count, size);
}

static
inline void * mem_realloc(void *ptr, size_t size)
{
#ifdef DEBUG
  if (!ptr) {
    return mem_alloc(size);
  }
  _mem_stats.realloc_count++;
#endif
  return realloc(ptr, size);
}

static
inline void mem_free(void *ptr)
{
#ifdef DEBUG
  if (ptr) _mem_stats.free_count++;
#endif
  free(ptr);
}

/**
 * @brief Allocates size bytes aligned to align bytes.
 *
 * align must be a power of two and a multiple of sizeof(void *).
 */
// static
// inline void * mem_alloc_aligned(size_t size, size_t align)
// {
// #ifdef DEBUG
//   _mem_stats.alloc_count++;
//   _mem_stats.bytes_requested += size;
// #endif
// #if defined(_MSC_VER)
//   return _aligned_malloc(size, align);
// #else
//   size = (size + align - 1) & ~(align - 1);
//   return aligned_alloc(align, size);
// #endif
// }

/**
 * @brief Frees a pointer returned by mem_alloc_aligned().
 *
 * Do not mix with mem_free() on MSVC (_aligned_free vs free differ).
 */
static
inline void mem_free_aligned(void *ptr)
{
#ifdef DEBUG
  if (ptr) _mem_stats.free_count++;
#endif
#if defined(_MSC_VER)
  _aligned_free(ptr);
#else
  free(ptr);
#endif
}

#define mem_set_t(ptr, c, size) mem_set(ptr, c, size)
#define mem_set_zero_t(ptr, type) mem_set_zero(ptr, sizeof(type))

API void mem_set(void *ptr, int c, size_t size)
{
  memset(ptr, c, size);
}

API void mem_set_zero(void *ptr, size_t size)
{
  memset(ptr, 0, size);
}

API void mem_copy(void *source, void *target, size_t size)
{
  memcpy(target, source, size);
}
