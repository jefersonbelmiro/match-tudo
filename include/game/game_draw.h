#pragma once

#include "core/app.h"
#include "core/defs.h"
#include "core/draw.h"
#include "core/resources.h"
#include "core/style.h"
#include "game/board.h"
#include "game/game.h"
#include "raylib.h"

API void draw_border()
{
  DrawRectangleLinesEx(
    (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()}, 
    4, DARKGRAY
  );
}

API void draw_cell_state(game_t *game, Vector2 position, board_cell_state state)
{
  board_t *board = &game->board;
  u16 cell_size = board->cell_size * board->scale;

  Color color;
  switch (state) {
    case CELL_HOVERED:
      color = ColorAlpha(WHITE, 0.5);
    break;
    case CELL_SELECTED:
      color = ColorAlpha(WHITE, 0.4);
    break;
    case CELL_DRAGGING:
      color = ColorAlpha(ORANGE, 0.4);
    break;
  }

  Rectangle rec = {
    position.x - cell_size * 0.5 - 1,
    position.y - cell_size * 0.5 - 1,
    cell_size + 2, 
    cell_size + 2
  };
  float line_tick = 4.0;
  DrawRectangleLinesEx(rec, line_tick, color);
}

API void draw_hover(game_t *game)
{
  board_t *board = &game->board;

  if (game->hover != IDX_NONE) {
    Vector2 position = board_idx_to_world(board, game->hover);
    draw_cell_state(game, position, CELL_HOVERED);
  }

  if (game->selected != IDX_NONE) {
    u16 value = board->value[game->selected];
    Vector2 position = board->cell_position[game->selected];
    draw_atlas_fliph(board->atlas, value, position, board->scale, 0, WHITE);
    draw_cell_state(game, position, CELL_DRAGGING);
  }
}

API void draw_board(game_t *game)
{
  board_t *board = &game->board;
  atlas_t *atlas = board->atlas;

  u16 cols = board->size.x / board->cell_size;
  u16 rows = board->size.y / board->cell_size;
  u16 width = board->size.x * board->scale;
  u16 height = board->size.y * board->scale;

  u16 cell_size = board->cell_size * board->scale;

  u16 grid_size = rows * cols;
  for (u16 idx = 0; idx < grid_size; idx++) {
    if (idx == game->selected) {
      continue;
    }
    u16 value = board->value[idx];
    Vector2 cell_position = board->cell_position[idx];
    draw_atlas_fliph(atlas, value, cell_position, board->scale, 0, WHITE);
  }

  float margin_left = board->position.x;
  float margin_top  = board->position.y;

  float line_tick = 4.0;
  Color line_color = COLOR_GRID_LINE;

  for (u16 col = 0; col <= cols; col++) {
    Vector2 start = {
      col * cell_size + margin_left, 
      margin_top
    };
    Vector2 end = {
      col * cell_size + margin_left,
      height + margin_top
    };
    if (col == 0) {
      start.y -= line_tick * 0.5;
      end.y += line_tick * 0.5;
    } if (col == cols) {
      start.y -= line_tick * 0.5;
      end.y += line_tick * 0.5;
    }
    DrawLineEx(start, end, line_tick, line_color);
  }
  for (u16 row = 0; row <= rows; row++) {
    Vector2 start = {
      margin_left,
      row * cell_size + margin_top
    };
    Vector2 end = {
      width + margin_left,
      cell_size * row + margin_top
    };
    DrawLineEx(start, end, line_tick, line_color);
  }
}

