#pragma once

// incrementing id allocator with freelist recycling (no generation)
//
// ids are entity_id_t
// freed slots are recycled via a freelist so ids stay compact

#include "core/defs.h"
#include "core/arena.h"
#include <assert.h>

typedef struct {
  entity_id_t *freelist;  // recycled IDs ready to be reacquired
  entity_id_t  next_id;   // next never-used ID (grows monotonically)
  entity_id_t  freelist_count;
#ifdef DEBUG
  entity_id_t  freelist_capacity;
#endif
} id_pool_t;

API void id_pool_init(id_pool_t *ep, arena_t *arena, entity_id_t capacity)
{
  ep->next_id = 1;
  ep->freelist = arena_push(arena, entity_id_t, capacity);
  ep->freelist_count = 0;
#ifdef DEBUG
  ep->freelist_capacity = capacity;
#endif
}

API void id_pool_reset(id_pool_t *ep)
{
  ep->next_id = 1;
  ep->freelist_count = 0;
}

// no-op: memory is owned by the game arena, freed all at once by game_fini
API void id_pool_fini(id_pool_t *ep)
{
  (void)ep;
}

// Acquire a new ID. Reuses a freed ID when available, otherwise increments
API entity_id_t id_pool_acquire(id_pool_t *ep)
{
  if (ep->freelist_count > 0) {
    return ep->freelist[--ep->freelist_count];
  }
  return ep->next_id++;
}

// Release an ID back to the pool for future reuse.
// NOTE: No generation bump — any external copies of this ID remain valid.
// Caller is responsible for ensuring the ID is no longer reachable before releasing.
API void id_pool_release(id_pool_t *ep, entity_id_t id) {
#ifdef DEBUG
  assert(ep->freelist_count < ep->freelist_capacity && "id_pool freelist overflow: increase cap");
#endif
  ep->freelist[ep->freelist_count++] = id;
}
