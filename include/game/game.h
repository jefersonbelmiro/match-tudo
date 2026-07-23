#pragma once

#include "core/arena.h"
#include "core/defs.h"
#include "core/tween.h"
#include "game/board.h"
#include "raylib.h"

typedef struct {
  u16 cell_size;
  Vector2 view_port;
} game_config_t;

typedef struct {
  arena_t       *arena;
  game_config_t config;
  board_t       board;
  Vector2       selected_offset;
  // Vector2       selected_position;
  grid_idx_t    selected;
  tween_h       selected_tween;
  grid_idx_t    hover;
  tween_h       hover_tween;
} game_t;

