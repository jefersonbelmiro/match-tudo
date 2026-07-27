#pragma once

#include "core/app.h"
#include "core/arena.h"
#include "core/defs.h"
#include "core/ease.h"
#include "raylib.h"
#include <assert.h>
#include <stdbool.h>

// tween_h - stable generational handle.
//   low byte  = handle id   (index into sparse/generation tables)
//   high byte = generation  (bumped on every release of that id)
typedef u16 tween_h;

#define TWEEN_NONE ((tween_h)UINT16_MAX)

#define TWEEN_FLAG_ACTIVE        (1u << 0)
#define TWEEN_FLAG_PARALLEL      (1u << 1)
#define TWEEN_FLAG_PAUSED        (1u << 2)
#define TWEEN_FLAG_LOOP          (1u << 3)
#define TWEEN_FLAG_COMPLETED     (1u << 4)
#define TWEEN_FLAG_PROCESS_AWAYS (1u << 5)

#define TWEEN__ID(h)  ((u16)((h) & 0xFFu))
#define TWEEN__GEN(h) ((u8)((h) >> 8))

typedef struct {
  u8        flags[MAX_TWEENS];
  u16       tweener_start[MAX_TWEENS];
  u16       tweener_count[MAX_TWEENS];
  u16       tweener_current[MAX_TWEENS];
  u16       dense_id[MAX_TWEENS];

  float    *tw_target[MAX_TWEENERS_TOTAL];
  float     tw_from[MAX_TWEENERS_TOTAL];
  float     tw_to[MAX_TWEENERS_TOTAL];
  float     tw_duration[MAX_TWEENERS_TOTAL];
  float     tw_elapsed[MAX_TWEENERS_TOTAL];
  float     tw_delay[MAX_TWEENERS_TOTAL];
  ease_fn_t tw_ease[MAX_TWEENERS_TOTAL];
  bool      tw_done[MAX_TWEENERS_TOTAL];

  u16       sparse[MAX_TWEENS];
  u8        generation[MAX_TWEENS];
  u16       free_ids[MAX_TWEENS];
  u16       free_count;
  u16       next_id;

  u16 tween_count;
  u16 tweener_used;

  u16 remove_queue[MAX_TWEENS];
  u16 remove_count;
} tween_pool_t;

GLOBAL tween_pool_t *g_tween;

API tween_pool_t *tween_ptr(void) 
{ 
  return g_tween;
}

API u32 tween_memory_size(void)
{ 
  u32 size = 0;
  size += sizeof(tween_pool_t); 
  return size;
}

API void tween_init(arena_t *arena)
{
  g_tween = arena_push_zero(arena, tween_pool_t, 1);
  g_tween->next_id = 1;
  g_tween->generation[0] = 1;
}

// returns dense index, or tween_none if the handle is stale/none
API tween_h tween__dense(tween_h h)
{
  if (h == TWEEN_NONE) return TWEEN_NONE;
  u16 id = TWEEN__ID(h);
  if (id >= MAX_TWEENS) return TWEEN_NONE;
  if (g_tween->generation[id] != TWEEN__GEN(h)) return TWEEN_NONE;
  return g_tween->sparse[id];
}

API tween_h tween__alloc(bool parallel)
{
  assert(g_tween->tween_count < MAX_TWEENS && "MAX_TWEENS exceeded");
  assert(g_tween->tweener_used < MAX_TWEENERS_TOTAL && "MAX_TWEENERS_TOTAL exceeded");

  // acquire stable handle id
  u16 id = (g_tween->free_count > 0) ? g_tween->free_ids[--g_tween->free_count]
                                  : g_tween->next_id++;

  u16 d = g_tween->tween_count++;
  g_tween->sparse[id]   = d;
  g_tween->dense_id[d]  = id;

  g_tween->flags[d]          = TWEEN_FLAG_ACTIVE | (parallel ? TWEEN_FLAG_PARALLEL : 0);
  g_tween->tweener_start[d]  = g_tween->tweener_used;
  g_tween->tweener_count[d]  = 0;
  g_tween->tweener_current[d]= 0;

  return (tween_h)(((u16)g_tween->generation[id] << 8) | id);
}



API tween_h tween_create(void)          
{ 
  return tween__alloc(false); 
}

API tween_h tween_create_parallel(void)
{ 
  return tween__alloc(true);
}

API u8 tween_add(tween_h h, float *target, float to, float duration, ease_fn_t ease)
{
  tween_h d = tween__dense(h);
  assert(d != TWEEN_NONE && "tween_add: stale handle");
  assert(g_tween->tweener_used < MAX_TWEENERS_TOTAL && "MAX_TWEENERS_TOTAL exceeded");
  u8  idx = g_tween->tweener_count[d]++;
  u32 ti  = g_tween->tweener_start[d] + idx;
  g_tween->tw_target[ti]   = target;
  g_tween->tw_from[ti]     = *target;
  g_tween->tw_to[ti]       = to;
  g_tween->tw_duration[ti] = duration;
  g_tween->tw_elapsed[ti]  = 0.0f;
  g_tween->tw_delay[ti]    = 0.0f;
  g_tween->tw_ease[ti]     = ease;
  g_tween->tw_done[ti]     = false;
  g_tween->tweener_used++;
  return idx;
}

API void tween_process_mode(tween_h h, process_mode_t mode)
{
  tween_h d = tween__dense(h);
  if (d == TWEEN_NONE) return;
  if (mode == PROCESS_MODE_ALWAYS) {
    g_tween->flags[d] |= TWEEN_FLAG_PROCESS_AWAYS;
  } else {
    g_tween->flags[d] &= ~TWEEN_FLAG_PROCESS_AWAYS;
  }
}

// set delay (in seconds) for a specific tweener index within the tween.
// call after tween_add. tweener_idx is 0-based within this tween
API void tween_set_delay(tween_h h, u8 tweener_idx, float delay)
{
  tween_h d = tween__dense(h);
  if (d == TWEEN_NONE) return;
  assert(tweener_idx < g_tween->tweener_count[d] && "tweener_idx out of range");
  u32 ti = g_tween->tweener_start[d] + tweener_idx;
  g_tween->tw_delay[ti]   = delay;
  g_tween->tw_elapsed[ti] = -delay;
}

API void tween_set_loop(tween_h h, bool loop)
{
  tween_h d = tween__dense(h);
  if (d == TWEEN_NONE) return;
  if (loop)
    g_tween->flags[d] |=  TWEEN_FLAG_LOOP;
  else
    g_tween->flags[d] &= ~TWEEN_FLAG_LOOP;
}

API void tween_pause(tween_h h)
{
  tween_h d = tween__dense(h);
  if (d == TWEEN_NONE) return;
  g_tween->flags[d] |= TWEEN_FLAG_PAUSED;
}

API void tween_play(tween_h h)
{
  tween_h d = tween__dense(h);
  if (d == TWEEN_NONE) return;
  g_tween->flags[d] &= ~TWEEN_FLAG_PAUSED;
}

API bool tween_is_paused(tween_h h)
{
  tween_h d = tween__dense(h);
  return d != TWEEN_NONE && (g_tween->flags[d] & TWEEN_FLAG_PAUSED) != 0;
}

API void tween_kill(tween_h h)
{
  tween_h d = tween__dense(h);
  if (d == TWEEN_NONE) return;
  if (!(g_tween->flags[d] & TWEEN_FLAG_ACTIVE)) return;
  g_tween->flags[d] &= ~TWEEN_FLAG_ACTIVE;
  g_tween->remove_queue[g_tween->remove_count++] = (u16)d;
}

API bool tween_is_active(tween_h h)
{
  tween_h d = tween__dense(h);
  return d != TWEEN_NONE && (g_tween->flags[d] & TWEEN_FLAG_ACTIVE) != 0;
}

// Update the target pointer of a tweener within an active tween.
// Used when a layer operation moves an entity's position slot, so the
// existing tween writes to the correct memory address.
API void tween_retarget(tween_h h, u8 tweener_idx, float *new_target)
{
  tween_h d = tween__dense(h);
  if (d == TWEEN_NONE) return;
  assert(tweener_idx < g_tween->tweener_count[d]);
  u32 ti = g_tween->tweener_start[d] + tweener_idx;
  g_tween->tw_target[ti] = new_target;
}

// Returns true if this tween completed on the most recent tween_process call.
API bool tween_completed(tween_h h)
{
  tween_h d = tween__dense(h);
  return d != TWEEN_NONE && (g_tween->flags[d] & TWEEN_FLAG_COMPLETED) != 0;
}

API tween_h tween_f32(float *target, float to, float duration, ease_fn_t ease)
{
  tween_h h = tween_create();
  tween_add(h, target, to, duration, ease);
  return h;
}

API tween_h tween_f32_always(float *target, float to, float duration, ease_fn_t ease)
{
  tween_h h = tween_create();
  tween_process_mode(h, PROCESS_MODE_ALWAYS);
  tween_add(h, target, to, duration, ease);
  return h;
}

API tween_h tween_vec2(Vector2 *v, Vector2 to, float duration, ease_fn_t ease)
{
  tween_h h = tween_create_parallel();
  tween_add(h, &v->x, to.x, duration, ease);
  tween_add(h, &v->y, to.y, duration, ease);
  return h;
}

API void tween__loop_reset(tween_pool_t *pool, u16 i)
{
  u32 start = pool->tweener_start[i];
  u32 count = pool->tweener_count[i];
  for (u32 j = 0; j < count; j++) {
    u32 ti = start + j;
    pool->tw_elapsed[ti] = -pool->tw_delay[ti];
    pool->tw_done[ti]    = false;
  }
  pool->tweener_current[i] = 0;
  // recapture tw_from of first tweener from current target value
  pool->tw_from[start] = *pool->tw_target[start];
}

// reset the tween to the beginning of its sequence and unpause
// use this instead of tween_play when the tween may have been mid-cycle or
// sitting at end-of-sequence (e.g. after being paused mid-loop)
API void tween_restart(tween_h h)
{
  tween_h d = tween__dense(h);
  if (d == TWEEN_NONE) return;
  tween__loop_reset(g_tween, (u16)d);
  g_tween->flags[d] &= ~TWEEN_FLAG_PAUSED;
}

API void tween_process(float delta)
{
  tween_pool_t *pool = g_tween;

  if (pool->remove_count) {
    u16 write_index = 0;
    u32 write_tweener = 0;
    
    for (u16 read_index = 0; read_index < pool->tween_count; read_index++) {
      if (pool->flags[read_index] & TWEEN_FLAG_ACTIVE) {
        if (write_index != read_index) {
          pool->flags          [write_index] = pool->flags          [read_index];
          pool->tweener_count  [write_index] = pool->tweener_count  [read_index];
          pool->tweener_current[write_index] = pool->tweener_current[read_index];
        }

        u32 source_start = pool->tweener_start[read_index];
        u16 tweener_count = pool->tweener_count[read_index];
        pool->tweener_start[write_index] = write_tweener;

        for (u32 j = 0; j < (u32)tweener_count; j++) {
          pool->tw_target  [write_tweener + j] = pool->tw_target  [source_start + j];
          pool->tw_from    [write_tweener + j] = pool->tw_from    [source_start + j];
          pool->tw_to      [write_tweener + j] = pool->tw_to      [source_start + j];
          pool->tw_duration[write_tweener + j] = pool->tw_duration[source_start + j];
          pool->tw_elapsed [write_tweener + j] = pool->tw_elapsed [source_start + j];
          pool->tw_delay   [write_tweener + j] = pool->tw_delay   [source_start + j];
          pool->tw_ease    [write_tweener + j] = pool->tw_ease    [source_start + j];
          pool->tw_done    [write_tweener + j] = pool->tw_done    [source_start + j];
        }

        u16 handle_id = pool->dense_id[read_index];
        pool->sparse[handle_id] = write_index;
        pool->dense_id[write_index] = handle_id;

        write_tweener += (u32)tweener_count;
        write_index++;
      } else {
        u16 handle_id = pool->dense_id[read_index];
        pool->generation[handle_id]++;
        pool->free_ids[pool->free_count++] = handle_id;
      }
    }

    pool->tween_count = write_index;
    pool->tweener_used = write_tweener;
    if (pool->tween_count == 0) pool->tweener_used = 0;
    pool->remove_count = 0;
  }

  bool paused = app_paused();

  for (u16 i = 0; i < pool->tween_count; i++) {
    if (!(pool->flags[i] & TWEEN_FLAG_ACTIVE)) {
      // pool->remove_queue[pool->remove_count++] = i;
      continue;
    }
    if ((pool->flags[i] & TWEEN_FLAG_COMPLETED)) {
      pool->flags[i] &= ~TWEEN_FLAG_ACTIVE;
      // pool->flags[i] &= ~TWEEN_FLAG_COMPLETED;
      pool->remove_queue[pool->remove_count++] = i;
      continue;
    }

    // pool->flags[i] &= ~TWEEN_FLAG_COMPLETED;

    if (pool->flags[i] & TWEEN_FLAG_PAUSED) continue;
    if (!(pool->flags[i] & TWEEN_FLAG_PROCESS_AWAYS) && paused) continue;

    bool done = false;

    if (pool->flags[i] & TWEEN_FLAG_PARALLEL) {
      bool all_done = true;
      u32 start = pool->tweener_start[i];
      u32 count = pool->tweener_count[i];
      for (u32 j = 0; j < count; j++) {
        u32 ti = start + j;
        if (pool->tw_done[ti]) continue;
        pool->tw_elapsed[ti] += delta;
        if (pool->tw_elapsed[ti] < 0.0f) { all_done = false; continue; }
        float tt = pool->tw_elapsed[ti] / pool->tw_duration[ti];
        if (tt >= 1.0f) { tt = 1.0f; pool->tw_done[ti] = true; }
        else all_done = false;
        *pool->tw_target[ti] = pool->tw_from[ti]
          + (pool->tw_to[ti] - pool->tw_from[ti]) * pool->tw_ease[ti](tt);
      }
      done = all_done;
    } else {
      u32 start = pool->tweener_start[i];
      u32 count = pool->tweener_count[i];
      while (pool->tweener_current[i] < count) {
        u32 ti = start + pool->tweener_current[i];
        if (pool->tw_done[ti]) { pool->tweener_current[i]++; continue; }
        pool->tw_elapsed[ti] += delta;
        if (pool->tw_elapsed[ti] < 0.0f) break;
        float tt = pool->tw_elapsed[ti] / pool->tw_duration[ti];
        if (tt >= 1.0f) {
          tt = 1.0f;
          pool->tw_done[ti] = true;
          pool->tweener_current[i]++;
          if (pool->tweener_current[i] < count) {
            u32 next_ti = start + pool->tweener_current[i];
            pool->tw_from[next_ti] = *pool->tw_target[next_ti];
          }
        }
        *pool->tw_target[ti] = pool->tw_from[ti]
          + (pool->tw_to[ti] - pool->tw_from[ti]) * pool->tw_ease[ti](tt);
        break;
      }
      done = pool->tweener_current[i] >= count;
    }

    if (done) {
      if (pool->flags[i] & TWEEN_FLAG_LOOP) {
        tween__loop_reset(pool, i);
      } else {
        pool->flags[i] |= TWEEN_FLAG_COMPLETED;
        // pool->flags[i] &= ~TWEEN_FLAG_ACTIVE;
        // pool->remove_queue[pool->remove_count++] = i;
      }
    }
  }
}

API void tween_cancel_all(void)
{
  tween_pool_t *pool = g_tween;
  // invalidate every outstanding handle
  for (u16 id = 0; id < MAX_TWEENS; id++) {
    pool->generation[id]++;
  }
  pool->tween_count  = 0;
  pool->tweener_used = 0;
  pool->free_count   = 0;
  pool->next_id      = 1;
  pool->remove_count = 0;
}
