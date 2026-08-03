#pragma once

#include "core/arena.h"
#include "core/defs.h"
#include "game/board.h"
#include "raylib.h"

typedef struct {
  Vector2    view_port;
  u16        cell_size;
  grid_idx_t texture_idx;
} game_config_t;

typedef struct {
  arena_t       *arena;
  game_config_t config;
  board_t       board;
  Vector2       selected_offset;
  entity_id_t   selected_id;
  grid_idx_t    selected_idx;
  entity_id_t   hover_id;
} game_t;
