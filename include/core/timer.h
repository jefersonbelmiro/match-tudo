#pragma once

#include "core/app.h"
#include "core/arena.h"
#include "core/defs.h"
#include <assert.h>
#include <stdbool.h>

// timer_h - stable generational handle
//   low byte  = handle id   (index into sparse/generation tables)
//   high byte = generation  (bumped on every release of that id)
typedef u16 timer_h;

#define TIMER_NONE ((timer_h)UINT16_MAX)

#define TIMER_FLAG_PAUSED        (1u << 0)
#define TIMER_FLAG_ONE_SHOT      (1u << 1)
#define TIMER_FLAG_ACTIVE        (1u << 2)
#define TIMER_FLAG_TICKED        (1u << 3)
#define TIMER_FLAG_PROCESS_AWAYS (1u << 4)

#define TIMER__ID(h)  ((u16)((h) & 0xFFu))
#define TIMER__GEN(h) ((u8)((h) >> 8))

// timer data lives in dense arrays [0..count) compacted via swap-and-pop.
// the sparse table maps handle id -> dense index, so handles stay valid when
// timers move inside the dense arrays. a stale handle (dead or reused slot)
// fails the generation check and every api call on it becomes a safe no-op.
typedef struct {
  // dense arrays - index 0..count, compacted via swap-and-pop
  float     timeout[MAX_TIMERS];
  float     elapsed[MAX_TIMERS];
  u8        flags[MAX_TIMERS];
  u16       dense_id[MAX_TIMERS];        // dense index -> handle id

  // handle tables - index = handle id
  u16       sparse[MAX_TIMERS];          // handle id -> dense index
  u8        generation[MAX_TIMERS];      // handle id -> current generation
  u16       free_ids[MAX_TIMERS];        // released handle ids
  u16       free_count;
  u16       next_id;

  u16       count;
} timer_pool_t;

GLOBAL timer_pool_t *g_timer;

API timer_pool_t *timer_ptr(void)
{
  return g_timer;
}

API u32 timer_memory_size(void)
{ 
  u32 size = 0;
  size += sizeof(timer_pool_t); 
  return size;
}

API void timer_init(arena_t *arena)
{
  g_timer = arena_push_zero(arena, timer_pool_t, 1);
  g_timer->next_id = 1;
  g_timer->generation[0] = 1;
}

// returns dense index, or TIMER_NONE if the handle is stale/none
API timer_h timer__dense(timer_h h)
{
  if (h == TIMER_NONE) return TIMER_NONE;
  u16 id = TIMER__ID(h);
  if (id >= MAX_TIMERS) return TIMER_NONE;
  if (g_timer->generation[id] != TIMER__GEN(h)) return TIMER_NONE;
  return g_timer->sparse[id];
}

API timer_h timer__alloc(void)
{
  assert(g_timer->count < MAX_TIMERS && "MAX_TIMERS exceeded");

  u16 id = (g_timer->free_count > 0) ? g_timer->free_ids[--g_timer->free_count]
                                  : g_timer->next_id++;

  u16 d = g_timer->count++;
  g_timer->sparse[id]  = d;
  g_timer->dense_id[d] = id;

  g_timer->flags[d]   = TIMER_FLAG_ACTIVE;
  g_timer->elapsed[d] = 0.0f;
  g_timer->timeout[d] = 0.0f;

  return (timer_h)(((u16)g_timer->generation[id] << 8) | id);
}

// remove timer at dense slot i via swap-and-pop
// releases its handle id (bumps generation -> stale handles become no-ops)
// and remaps the moved timer's id so its handle stays valid
API void timer__remove(timer_pool_t *pool, u16 i)
{
  u16 removed_id = pool->dense_id[i];
  pool->generation[removed_id]++;
  pool->free_ids[pool->free_count++] = removed_id;

  u16 last = --pool->count;

  if (i != last) {
    pool->timeout[i]    = pool->timeout[last];
    pool->elapsed[i]    = pool->elapsed[last];
    pool->flags[i]      = pool->flags[last];

    u16 moved_id = pool->dense_id[last];
    pool->sparse[moved_id] = i;
    pool->dense_id[i]      = moved_id;
  }
}

API timer_h timer_new(void)
{
  return timer__alloc();
}

API void timer_set(timer_h h, float timeout, bool one_shot)
{
  timer_h d = timer__dense(h);
  if (d == TIMER_NONE) return;
  g_timer->timeout[d] = timeout;
  g_timer->elapsed[d] = 0.0f;
  if (one_shot)
    g_timer->flags[d] |=  TIMER_FLAG_ONE_SHOT;
  else
    g_timer->flags[d] &= ~TIMER_FLAG_ONE_SHOT;
}

API void timer_process_mode(timer_h h, process_mode_t mode)
{
  timer_h d = timer__dense(h);
  if (d == TIMER_NONE) return;
  if (mode == PROCESS_MODE_ALWAYS) {
    g_timer->flags[d] |= TIMER_FLAG_PROCESS_AWAYS;
  } else {
    g_timer->flags[d] &= ~TIMER_FLAG_PROCESS_AWAYS;
  }
}

API void timer_pause(timer_h h)
{
  timer_h d = timer__dense(h);
  if (d == TIMER_NONE) return;
  g_timer->flags[d] |= TIMER_FLAG_PAUSED;
}

API void timer_play(timer_h h)
{
  timer_h d = timer__dense(h);
  if (d == TIMER_NONE) return;
  g_timer->flags[d] &= ~TIMER_FLAG_PAUSED;
}

API bool timer_is_paused(timer_h h)
{
  timer_h d = timer__dense(h);
  return d != TIMER_NONE && (g_timer->flags[d] & TIMER_FLAG_PAUSED) != 0;
}

// Mark inactive - removed by timer_process on next frame
API void timer_kill(timer_h h)
{
  timer_h d = timer__dense(h);
  if (d == TIMER_NONE) return;
  g_timer->flags[d] &= ~TIMER_FLAG_ACTIVE;
}

API bool timer_is_active(timer_h h)
{
  timer_h d = timer__dense(h);
  return d != TIMER_NONE && (g_timer->flags[d] & TIMER_FLAG_ACTIVE) != 0;
}

// frame based: true when timer is ticked on the last timer_process call
API bool timer_ticked(timer_h h)
{
  timer_h d = timer__dense(h);
  return d != TIMER_NONE && (g_timer->flags[d] & TIMER_FLAG_TICKED) != 0;
}

API timer_h timer_once(float timeout)
{
  timer_h h = timer_new();
  timer_set(h, timeout, true);
  return h;
}

API timer_h timer_once_always(float timeout)
{
  timer_h h = timer_new();
  timer_set(h, timeout, true);
  timer_process_mode(h, PROCESS_MODE_ALWAYS);
  return h;
}

API timer_h timer_every(float timeout)
{
  timer_h h = timer_new();
  timer_set(h, timeout, false);
  return h;
}

API void timer_process(float delta)
{
  timer_pool_t *pool = g_timer;
  bool paused = app_paused();
  u16 i = 0;
  while (i < pool->count) {
    u8 f = pool->flags[i];

    if (!(f & TIMER_FLAG_ACTIVE)) {
      timer__remove(pool, i);
      continue;
    }

    pool->flags[i] &= ~TIMER_FLAG_TICKED;

    if (f & TIMER_FLAG_PAUSED) { i++; continue; }
    if (!(f & TIMER_FLAG_PROCESS_AWAYS) && paused) { i++; continue; }

    pool->elapsed[i] += delta;

    if (pool->elapsed[i] >= pool->timeout[i]) {
      pool->flags[i] |= TIMER_FLAG_TICKED;

      if (pool->flags[i] & TIMER_FLAG_ONE_SHOT) {
        pool->elapsed[i] = pool->timeout[i];
        pool->flags[i]  &= ~TIMER_FLAG_ACTIVE;
      } else if (pool->timeout[i] > 0.0f) {
        do {
          pool->elapsed[i] -= pool->timeout[i];
        } while (pool->elapsed[i] >= pool->timeout[i]);
      }
    }
    i++;
  }
}

API void timer_cancel_all(void)
{
  timer_pool_t *pool = g_timer;
  for (u16 id = 0; id < MAX_TIMERS; id++) pool->generation[id]++;
  pool->count      = 0;
  pool->free_count = 0;
  pool->next_id    = 1;
}
