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
  Color line_color = RED;

  // internal vertical edges, skipped between two matched cells
  for (u16 row = 0; row < rows; row++) {
    for (u16 col = 0; col + 1 < cols; col++) {
      grid_idx_t left_idx = row * cols + col;
      if (board->cell_matches[left_idx] && board->cell_matches[left_idx + 1]) {
        continue;
      }
      float x = margin_left + cell_size * (col + 1);
      DrawLineEx(
        (Vector2){ x, margin_top + cell_size * row },
        (Vector2){ x, margin_top + cell_size * (row + 1) },
        line_tick, line_color
      );
    }
  }

  // internal horizontal edges, skipped between two matched cells
  for (u16 row = 0; row + 1 < rows; row++) {
    for (u16 col = 0; col < cols; col++) {
      grid_idx_t top_idx = row * cols + col;
      if (board->cell_matches[top_idx] && board->cell_matches[top_idx + cols]) {
        continue;
      }
      float y = margin_top + cell_size * (row + 1);
      DrawLineEx(
        (Vector2){ margin_left + cell_size * col, y },
        (Vector2){ margin_left + cell_size * (col + 1), y },
        line_tick, line_color
      );
    }
  }

  // outer frame
  DrawRectangleLinesEx(
    (Rectangle){ margin_left, margin_top, width, height },
    line_tick, line_color
  );
}

API void draw_board_match_flash(game_t *game)
{
  board_t *board = &game->board;

  u16 cols = board->size.x / board->cell_size;
  u16 rows = board->size.y / board->cell_size;
  u16 cell_size = board->cell_size * board->scale;

  float margin_left = board->position.x;
  float margin_top  = board->position.y;

  for (u16 idx = 0; idx < rows * cols; idx++) {
    float flash = board->cell_flash[idx];
    if (flash <= 0.0f) {
      continue;
    }
    u16 row = idx / cols;
    u16 col = idx % cols;
    Rectangle rec = {
      margin_left + cell_size * col,
      margin_top + cell_size * row,
      cell_size,
      cell_size
    };
    DrawRectangleRec(rec, ColorAlpha(WHITE, flash * 0.5f));
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

  for (entity_id_t i = layer->count; i > 0; i--) {
    entity_id_t index = i - 1;
    grid_idx_t texture_idx = layer->texture_idx[index];
    Vector2 position = layer->position[index];
    draw_atlas_fliph(atlas, texture_idx, position, board->scale, 0, WHITE);
  }
}

API void draw_board(game_t *game)
{
  draw_board_layer_bg(game);
  draw_board_match_flash(game);
  draw_board_grid(game);
  draw_board_cell_hover(game);
  draw_board_layer_fg(game);
  draw_board_cell_selected(game);
}
