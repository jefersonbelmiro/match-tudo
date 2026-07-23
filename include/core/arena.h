#pragma once

#include "core/defs.h"
#include "core/mem.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if DEBUG_MEMORY_USAGE
#  include "core/arena_debug.h"
#endif

#define ARENA_DEFAULT_ALIGNMENT 16

#ifndef ARENA_FALLBACK_MALLOC
#define ARENA_FALLBACK_MALLOC 1
#endif

typedef struct arena {
  size_t        capacity;
  size_t        offset;
  uintptr_t    *buffer;
#if DEBUG_MEMORY_USAGE
  uint32_t      debug_id;
#endif
} arena_t;

API size_t arena_offset(const arena_t *arena, const void *ptr)
{
  return (size_t)((uintptr_t)ptr - (uintptr_t)arena->buffer);
}

API arena_t *arena_create(size_t capacity, const char *tag)
{
  (void)tag;
  arena_t *arena = (arena_t *)mem_alloc(sizeof(arena_t) + capacity);
  if (!arena) return NULL;
  arena->buffer   = (uintptr_t *)(arena + 1);
  arena->capacity = capacity;
  arena->offset   = 0;
#if DEBUG_MEMORY_USAGE
  arena->debug_id = arena_debug_create(capacity, (uintptr_t)arena->buffer, tag);
#endif
  return arena;
}

API void arena_init(arena_t *arena, size_t capacity, const char *tag)
{
  *arena = *arena_create(capacity, tag);
}

API void *arena_alloc_aligned(arena_t *arena, size_t size, size_t alignment)
{
  uintptr_t current = (uintptr_t)arena->buffer + arena->offset;
  uintptr_t aligned = (current + (alignment - 1u)) & ~(alignment - 1u);
  size_t    padding = (size_t)(aligned - current);
  size_t    total   = padding + size;

  if (arena->offset + total > arena->capacity) {
#if ARENA_FALLBACK_MALLOC
#  if DEBUG_MEMORY_USAGE
    arena_debug_fallback(arena->debug_id, size);
#  endif
    return mem_alloc(total);
#else
    return NULL;
#endif
  }

  arena->offset += total;

#if DEBUG_MEMORY_USAGE
  arena_debug_alloc_stats(arena->debug_id, size, padding, arena->offset);
#endif

  return (void *)aligned;
}

API void *arena_alloc_zero_aligned(arena_t *arena, size_t size, size_t alignment)
{
  void *p = arena_alloc_aligned(arena, size, alignment);
  if (p) memset(p, 0, size);
  return p;
}

API void *arena_alloc(arena_t *arena, size_t size)
{
  return arena_alloc_aligned(arena, size, ARENA_DEFAULT_ALIGNMENT);
}

API void *arena_alloc_zero(arena_t *arena, size_t size)
{
  void *p = arena_alloc_aligned(arena, size, ARENA_DEFAULT_ALIGNMENT);
  if (p) memset(p, 0, size);
  return p;
}

API size_t arena_used(const arena_t *arena) { return arena->offset; }

API size_t arena_remaining(const arena_t *arena)
{
  return arena->capacity - arena->offset;
}

API void arena_reset(arena_t *arena)
{
  arena->offset = 0;
#if DEBUG_MEMORY_USAGE
  arena_debug_reset(arena->debug_id);
#endif
}

API void arena_fini(arena_t *arena)
{
#if DEBUG_MEMORY_USAGE
  arena_debug_destroy(arena->debug_id);
#endif
  mem_free(arena);
}

API arena_t *arena_create_sub(arena_t *parent, size_t capacity, const char *tag)
{
  (void)tag;
  arena_t *sub = (arena_t *)arena_alloc_aligned(parent, sizeof(arena_t), _Alignof(arena_t));
  if (!sub) return NULL;
  sub->buffer   = (uintptr_t *)arena_alloc_aligned(parent, capacity, ARENA_DEFAULT_ALIGNMENT);
  sub->capacity = capacity;
  sub->offset   = 0;
#if DEBUG_MEMORY_USAGE
  sub->debug_id = arena_debug_sub_create(parent->debug_id, capacity,
                                          (uintptr_t)sub->buffer, tag);
#endif
  return sub;
}

API void *arena_alloc_tracked(arena_t *arena, size_t elem_size, uint32_t count,
                               size_t alignment, const char *type_name,
                               int zero, const char *tag,
                               const char *file, int line)
{
  (void)type_name; (void)tag; (void)file; (void)line;
  size_t total = (size_t)elem_size * count;
  void *ptr  = zero ? arena_alloc_zero_aligned(arena, total, alignment)
                    : arena_alloc_aligned(arena, total, alignment);
#if DEBUG_MEMORY_USAGE
  arena_debug_alloc_track(arena->debug_id, type_name, count, (uint16_t)elem_size,
                           zero, tag, file, line, arena_offset(arena, ptr));
#endif
  return ptr;
}

#define arena_push(arena, type, count) \
  arena_push_tag(arena, type, count, NULL)

#define arena_push_tag(arena, type, count, tag) \
  ((type *)arena_alloc_tracked((arena), sizeof(type), (count), _Alignof(type), \
    #type, 0, (tag), __FILE__, __LINE__))

#define arena_push_zero(arena, type, count) \
  arena_push_zero_tag(arena, type, count, NULL)

#define arena_push_zero_tag(arena, type, count, tag) \
  ((type *)arena_alloc_tracked((arena), sizeof(type), (count), _Alignof(type), \
    #type, 1, (tag), __FILE__, __LINE__))

#define arena_push_stride(arena, type, count, stride) \
  ((type *)arena_alloc_tracked((arena), (stride), (count), _Alignof(type), \
    #type, 0, NULL, __FILE__, __LINE__))

#define arena_push_zero_stride(arena, type, count, stride) \
  ((type *)arena_alloc_tracked((arena), (stride), (count), _Alignof(type), \
    #type, 1, NULL, __FILE__, __LINE__))
