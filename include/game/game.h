#pragma once

#include "core/arena.h"
#include "core/defs.h"
#include "game/board.h"
#include "game/level.h"
#include "raylib.h"

typedef enum {
  GAME_ORIENTATION_AUTO,
  GAME_ORIENTATION_PORTRAIT,
  GAME_ORIENTATION_LANDSCAPE,
} game_orientation_t;

typedef struct {
  arena_t           *arena;
  arena_t           *board_arena;
  level_pack_t      *lvl_pack;
  view_port_t       view_port;
  game_orientation_t orientation;         // efetiva (concreta)
  game_orientation_t orientation_setting; // AUTO / manual (settings futuro)
  board_t           board;
  Vector2           selected_offset;
  entity_id_t       selected_id;
  grid_idx_t        selected_idx;
  entity_id_t       hover_id;
} game_t;
