#pragma once

#include "core/app.h"
#include "core/defs.h"
#include "core/resources.h"
#include "raylib.h"

typedef struct {
  Texture *texture;
  atlas_t *atlas;
  grid_idx_t *value; // cell -> texture grid idx
  Vector2 *cell_position;
  u16     cell_size;

  // @TODO: 
  Vector2 position;
  Vector2 size;

  float scale;
} board_t;

typedef enum {
  CELL_HOVERED,
  CELL_SELECTED,
  CELL_DRAGGING,
} board_cell_state;

API void board_sync_resize(board_t *board)
{
  screen_size_t *screen = app_screen_size();
  u16 width = board->size.x * board->scale;
  u16 height = board->size.y * board->scale;
  float left = screen->x * 0.5 - width  * 0.5;
  float top  = screen->y * 0.5 - height * 0.5;
  board->position = (Vector2){ left, top };
}

API Vector2 board_idx_to_world(board_t *board, grid_idx_t idx)
{
  u16 cell_size = board->cell_size * board->scale;
  u16 width = board->size.x * board->scale;
  u16 cols = width / cell_size;
  u16 row = idx / cols;
  u16 col = idx % cols;
  return (Vector2){
    board->position.x + cell_size * col + cell_size * 0.5,
    board->position.y + cell_size * row + cell_size * 0.5
  };
}
