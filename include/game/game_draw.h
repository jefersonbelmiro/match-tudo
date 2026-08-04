#pragma once

#include "core/defs.h"
#include "core/draw.h"
#include "core/resources.h"
#include "core/style.h"
#include "game/board.h"
#include "game/game.h"
#include "raylib.h"

#define MATCH_HOVER_ALPHA 0.1f

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

API void draw_cell_state(game_t *game, Vector2 position, board_cell_state state, float alpha)
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
  DrawRectangleLinesEx(rec, line_tick, ColorAlpha(color, alpha));
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
  grid_idx_t idx = layer->idx[index];

  float alpha = board_cell_is_matched(board, idx) ? MATCH_HOVER_ALPHA : 1.0f;
  draw_cell_state(game, position, CELL_HOVERED, alpha);
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
  draw_cell_state(game, position, CELL_SELECTED, 1.0f);
}

// border tiles from atlas_01_64: 0 top, 1 right, 2 bottom, 3 left
#define BORDER_TILE_TOP    0
#define BORDER_TILE_RIGHT  1
#define BORDER_TILE_BOTTOM 2
#define BORDER_TILE_LEFT   3

API void draw_board_borders(game_t *game)
{
  board_t *board = &game->board;
  atlas_t *borders = resource_atlas_ptr(RESOURCE_ATLAS_1_64);

  u16 cols = board->size.x / board->cell_size;
  u16 rows = board->size.y / board->cell_size;
  float tile_scale = board->scale * ((float)board->cell_size / 64.0f);

  for (u16 row = 0; row < rows; row++) {
    for (u16 col = 0; col < cols; col++) {
      grid_idx_t idx = row * cols + col;
      Vector2 center = board_idx_to_world(board, idx);
      u8 edges = board->cell_edges[idx];

      if (!(edges & EDGE_TOP))  draw_atlas(borders, BORDER_TILE_TOP,  center, tile_scale, 0, WHITE);
      if (!(edges & EDGE_LEFT)) draw_atlas(borders, BORDER_TILE_LEFT, center, tile_scale, 0, WHITE);

      if (col + 1 == cols) draw_atlas(borders, BORDER_TILE_RIGHT,  center, tile_scale, 0, WHITE);
      if (row + 1 == rows) draw_atlas(borders, BORDER_TILE_BOTTOM, center, tile_scale, 0, WHITE);
    }
  }
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
  draw_board_borders(game);
  draw_board_cell_hover(game);
  draw_board_layer_fg(game);
  draw_board_match_flash(game);
  draw_board_cell_selected(game);
}
