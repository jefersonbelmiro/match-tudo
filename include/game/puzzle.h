#pragma once

#include "core/defs.h"
#include "core/arena.h"

typedef struct {
  u16        cell_size;
  grid_idx_t texture_idx;
} puzzle_t;

typedef struct {
  puzzle_t *array;
  u16 cap;
  u16 count;
} puzzle_table_t;

void puzzle_table_init(puzzle_table_t *table, u16 cap, arena_t *arena)
{
  *table = (puzzle_table_t) {
    .array = arena_push(arena, puzzle_t, cap),
    .cap = cap,
    .count = 0,
  };
}
