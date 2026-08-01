#pragma once

#include "core/defs.h"
#include "core/draw.h"
#include "core/resources.h"
#include "core/style.h"
#include "game/board.h"
#include "game/game.h"
#include "raylib.h"

API void draw_border()
{
  if (IsWindowFullscreen()) {
    return;
  }
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
      color = COLOR_GRID_HOVER;
    break;
    case CELL_SELECTED:
      color = COLOR_GRID_SELECTED;
    break;
  }

  Rectangle rec = {
    position.x - cell_size * 0.5 - 1,
    position.y - cell_size * 0.5 - 1,
    cell_size + 2, 
    cell_size + 2
  };
  float line_tick = GRID_LINE_TICK;
  DrawRectangleLinesEx(rec, line_tick, color);
}

API void draw_board_cell_hover(game_t *game)
{
  if (game->hover_id == IDX_NONE) {
    return;
  }
  board_t *board = &game->board;
  board_layer_t *layer = board->entity_layer[game->hover_id];
  entity_id_t index = layer->entity_index[game->hover_id];
  Vector2 position = layer->position[index];
  // Vector2 position = board_idx_to_world(board, layer->idx[index]);
  draw_cell_state(game, position, CELL_HOVERED);
  // DrawText(TextFormat("id: %d\nindex: %d", game->hover_id, index), position.x - 40, position.y - 40, 20, RED);
}

API void draw_board_cell_selected(game_t *game)
{
  if (game->selected_id == IDX_NONE) {
    return;
  }
  board_t *board = &game->board;
  board_layer_t *layer = board->entity_layer[game->selected_id];
  entity_id_t index = layer->entity_index[game->selected_id];
  Vector2 position = layer->position[index];
  draw_cell_state(game, position, CELL_SELECTED);
}

API void draw_board_grid(game_t *game)
{
  board_t *board = &game->board;

  u16 cols = board->size.x / board->cell_size;
  u16 rows = board->size.y / board->cell_size;
  u16 width = board->size.x * board->scale;
  u16 height = board->size.y * board->scale;

  u16 cell_size = board->cell_size * board->scale;

  float margin_left = board->position.x;
  float margin_top  = board->position.y;

  float line_tick = GRID_LINE_TICK;
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

API void draw_board_layer_bg(game_t *game)
{
  board_t *board = &game->board;
  atlas_t *atlas = board->atlas;
  board_layer_t *layer = &board->layer_bg;

  for (entity_id_t i = 0; i < layer->count; i++) {
    grid_idx_t texture_idx = layer->texture_idx[i];
    Vector2 position = layer->position[i];
    draw_atlas_fliph(atlas, texture_idx, position, board->scale, 0, WHITE);
  }
}

API void draw_board_layer_fg(game_t *game)
{
  board_t *board = &game->board;
  atlas_t *atlas = board->atlas;
  board_layer_t *layer = &board->layer_fg;

  for (entity_id_t i = 0; i < layer->count; i++) {
    grid_idx_t texture_idx = layer->texture_idx[i];
    Vector2 position = layer->position[i];
    draw_atlas_fliph(atlas, texture_idx, position, board->scale, 0, WHITE);
  }
}

API void draw_board(game_t *game)
{
  draw_board_layer_bg(game);
  draw_board_grid(game);
  draw_board_cell_hover(game);
  draw_board_layer_fg(game);
  draw_board_cell_selected(game);
}
