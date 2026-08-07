#pragma once

#include "core/defs.h"
#include "core/arena.h"
#include "core/mem.h"
#include "core/resources.h"

#define LEVEL_PACK_NAME_MAX 64
#define LEVEL_PACK_CATEGORY_MAX 64

typedef struct {
  u16         cell_size;
  grid_idx_t  texture_idx;
  u16         count;
} level_t;

typedef struct {
  level_t *levels;
  char     name[LEVEL_PACK_NAME_MAX];
  char     category[LEVEL_PACK_CATEGORY_MAX];
  u16      index;
  u16      cap;
  u16      count;
  bool     random;
} level_pack_t;

void level_pack_init(level_pack_t *pack, u16 cap, arena_t *arena)
{
  *pack = (level_pack_t) {
    .levels = arena_push(arena, level_t, cap),
    .random = false,
    .index = 0,
    .cap = cap,
    .count = 0,
  };
}

level_pack_t *level_pack_load(arena_t *arena) 
{
  level_t levels[] = {
    { .texture_idx = RESOURCE_TEXTURE_001, .cell_size = 128 },
    { .texture_idx = RESOURCE_TEXTURE_002, .cell_size = 128 },
    { .texture_idx = RESOURCE_TEXTURE_003, .cell_size = 128+64  },
  };
  u16 count = countof(levels);
  level_pack_t *pack = arena_push(arena, level_pack_t, 1);
  *pack = (level_pack_t){
    .name = "Ragnarok Online",
    .random = true,
    .index = 0,
    .levels = arena_push(arena, level_t, count),
    .count = count,
    .cap = count,
  };
  mem_copy(levels, pack->levels, sizeof(level_t) * count);
  return pack;
}

level_t *level_pack_current(level_pack_t *pack)
{
  return &pack->levels[pack->index];
}

bool level_pack_is_last(level_pack_t *pack)
{
  return pack->index + 1 == pack->count;
}

level_t *level_pack_next(level_pack_t *pack)
{
  assert(pack->index + 1 < pack->count);
  return &pack->levels[++pack->index];
}
