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
  u16      cap;
  u16      count;
} level_pack_t;

void level_pack_init(level_pack_t *pack, u16 cap, arena_t *arena)
{
  *pack = (level_pack_t) {
    .levels = arena_push(arena, level_t, cap),
    .cap = cap,
    .count = 0,
  };
}

level_pack_t *level_pack_load(arena_t *arena) 
{
  level_t levels[] = {
    { .texture_idx = RESOURCE_TEXTURE_001, .cell_size = 128 },
    { .texture_idx = RESOURCE_TEXTURE_002, .cell_size = 128 },
    { .texture_idx = RESOURCE_TEXTURE_003, .cell_size = 96  },
  };
  level_pack_t *pack = arena_push(arena, level_pack_t, 1);
  *pack = (level_pack_t){
    .name = "Ragnarok Online",
    .levels = arena_push(arena, level_t, countof(levels)),
    .count = countof(levels),
    .cap = countof(levels),
  };
  mem_copy(levels, pack->levels, sizeof(level_t) * countof(levels));
  return pack;
}
