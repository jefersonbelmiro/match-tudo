#pragma once

#include "core/app.h"
#include "core/defs.h"
#include "core/mem.h"
#include "core/resources.h"
#include "core/smath.h"
#include "core/tween.h"
#include "game/board.h"
#include "game/game.h"
#include "game/game_draw.h"
#include "raylib.h"
#include <stdbool.h>

API void game_create_board(game_t *game)
{
  Texture2D target_texture = resource_texture(game->config.texture_idx);
  board_t *board = &game->board;
  float texture_scale = 1.0;

  if (target_texture.height > target_texture.width) {
    texture_scale = (float)board->size.x / target_texture.width;
  } else if (target_texture.width > target_texture.height) {
    if (board->size.y > board->size.x) {
      texture_scale = (float)board->size.y / target_texture.height;
    } else {
      texture_scale = (float)board->size.x / target_texture.width;
    }
  }

  float texture_offset_top = target_texture.height * texture_scale * 0.5;
  if (target_texture.height > board->size.y) {
    texture_offset_top -= (target_texture.height * texture_scale - board->size.y) * 0.5;
  }
  float texture_offset_left = target_texture.width * texture_scale * 0.5;
  if (target_texture.width > board->size.x) {
    texture_offset_left -= (target_texture.width * texture_scale - board->size.x) * 0.5;
  }

  RenderTexture2D render = LoadRenderTexture(board->size.x, board->size.y);
  BeginTextureMode(render);

  draw_texture(&target_texture,
               texture_offset_left,
               texture_offset_top,
               0, texture_scale, WHITE);
  EndTextureMode();
  board->atlas->texture = render.texture;
  board->atlas->cell_size = (Vector2){board->cell_size, board->cell_size};

  u16 cell_size = board->cell_size * board->scale;
  u16 cols = board->size.x / board->cell_size;
  u16 rows = board->size.y / board->cell_size;

  float cell_offset_x = board->position.x + cell_size * 0.5;
  float cell_offset_y = board->position.y + cell_size * 0.5;
  
  board_layer_t *layer_bg = &board->layer_bg;

  u16 grid_size = rows * cols;
  for (u16 idx = 0; idx < grid_size; idx++) {
    u16 row = idx / cols;
    u16 col = idx % cols;
    entity_id_t id = idx;
    board_layer_append(
      layer_bg, 
      id, 
      idx,
      (Vector2){
        cell_offset_x + cell_size * col,
        cell_offset_y + cell_size * row
      },
      idx
    );
    board->entity_layer[id] = layer_bg;
    board->cell_entity[idx] = id;
    board->entity_tween[id] = TWEEN_NONE;
  }

  // fisher-yates/knuth shuffle
  for (u16 i = layer_bg->count - 1; i > 0; i--) {
    u16 j = m_rand32(0, i);
    u16 tmp = layer_bg->texture_idx[i];
    layer_bg->texture_idx[i] = layer_bg->texture_idx[j];
    layer_bg->texture_idx[j] = tmp;
  }
}

API void game_sync_size(game_t *game)
{
  board_t *board = &game->board;
  screen_size_t *screen = app_screen_size();
  board->scale = 1.0f;

  if (board->size.x > screen->x - 40 && board->size.y > screen->y - 40) {
    board->scale = (screen->x - 40) / board->size.x;
  }
  else if (board->size.x > screen->x - 40) {
    board->scale = (screen->x - 40) / board->size.x;
  }
  else if (board->size.y > screen->y - 40) {
    board->scale = (screen->y - 40) / board->size.y;
  }

  board_sync_size(&game->board);
}

API void game_update_input(game_t *game)
{
  board_t *board = &game->board;
  u16 cell_size = board->cell_size * board->scale;
  Vector2 mouse = GetMousePosition();
  screen_size_t *screen = app_screen_size();
  u16 width = board->size.x * board->scale;
  u16 height = board->size.y * board->scale;

  float left = screen->x * 0.5 - width  * 0.5;
  float top  = screen->y * 0.5 - height * 0.5;

  bool in_board_bounds = true;
  if (mouse.x < left || mouse.x >= left + width) {
    in_board_bounds = false;
  } 
  else if (mouse.y < top || mouse.y >= top + height) {
    in_board_bounds = false;
  }

  u16 cols = width / cell_size;
  u16 col = m_floor((mouse.x - left) / cell_size);
  u16 row = m_floor((mouse.y - top) / cell_size);
  u16 idx = row * cols + col;

  game->hover_id = ENTITY_NONE;
  if (in_board_bounds) {
    game->hover_id = board->cell_entity[idx];
  }

  DrawText(TextFormat("mouse   idx: %d\n", idx),                10, 40, 20, LIME);
  DrawText(TextFormat("hover    id: %d\n", game->hover_id),     10, 60, 20, LIME);
  DrawText(TextFormat("selected id: %d\n", game->selected_id),  10, 80, 20, LIME);
  if (game->hover_id != ENTITY_NONE)
    DrawText(
        TextFormat("texture idx: %d\n", board->layer_bg.texture_idx[board->layer_bg.entity_index[game->hover_id]]),
        10, 100, 20, LIME);

  if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && game->selected_id != ENTITY_NONE) {

    if (game->hover_id != ENTITY_NONE && game->hover_id != game->selected_id && 
        !tween_is_active(board->entity_tween[game->hover_id])
    ) {
      board->cell_entity[game->selected_idx] = game->hover_id;
      board->cell_entity[idx] = game->selected_id;

      board_layer_swap_fg(board, game->hover_id);

      board_grid_snap_animated(board, game->selected_id, idx);
      board_grid_snap_animated(board, game->hover_id, game->selected_idx);
    } else {
      board_grid_snap_animated(board, game->selected_id, game->selected_idx);
    }

    game->selected_id = ENTITY_NONE;
    game->selected_idx = IDX_NONE;
  }

  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && game->hover_id != ENTITY_NONE && 
    !tween_is_active(board->entity_tween[game->hover_id]) &&
    board->entity_layer[game->hover_id] == &board->layer_bg
  ) {
    board_layer_swap_fg(board, game->hover_id);

    board_layer_t *layer = board->entity_layer[game->hover_id];
    entity_id_t cell_index = layer->entity_index[game->hover_id];
    Vector2 cell_position = layer->position[cell_index];

    game->selected_idx = idx;
    game->selected_id = game->hover_id;
    game->selected_offset = (Vector2){ 
      GetMouseX() - cell_position.x, 
      GetMouseY() - cell_position.y
    };
  }

  if (game->selected_id != ENTITY_NONE) {
    board_layer_t *selected_layer = board->entity_layer[game->selected_id];
    entity_id_t selected_index = selected_layer->entity_index[game->selected_id];
    Vector2 *position = &selected_layer->position[selected_index];
    *position = (Vector2){ 
      GetMouseX() - game->selected_offset.x, 
      GetMouseY() - game->selected_offset.y
    };
  }
}

#define MATCH_FLASH_DURATION 0.4f
#define MATCH_MIN_RUN        2

API void game_cell_flash(board_t *board, grid_idx_t idx)
{
  board->cell_flash[idx] = 1.0f;
  tween_f32_always(&board->cell_flash[idx], 0.0f, MATCH_FLASH_DURATION, ease_out_cubic);
}

API void game_find_matches(board_t *board, grid_idx_t idx)
{
  printn("game_find_matches: %d visited: %d", idx, board->cell_visited[idx]);
  if (board->cell_visited[idx]) {
    return;
  };
  u16 cols = board_cols(board);
  u16 rows = board_rows(board);
  u16 row = idx / cols;
  u16 col = idx % cols;
  bool found = false;

  printn(" - find match idx: %d col: %d row: %d", idx, col, row);

  // horizontal
  if (col > 0) {
    u16 left = col - 1;
    grid_idx_t left_idx = board_idx_at(board, row, left);
    bool match = board_cells_match_left(board, idx, left_idx);
    printn(" - left match: %d", match);
    board->cell_matches[left_idx] = match;
    board->cell_visited[left_idx] = true;
    if (match) {
      found = true;
      game_cell_flash(board, left_idx);
      game_find_matches(board, left_idx);
    }
  }
  if (col + 1 < cols) {
    u16 right = col + 1;
    grid_idx_t right_idx = board_idx_at(board, row, right);
    bool match = board_cells_match_right(board, idx, right_idx);
    board->cell_matches[right_idx] = match;
    board->cell_visited[right_idx] = true;
    if (match) {
      found = true;
      game_cell_flash(board, right_idx);
      game_find_matches(board, right_idx);
    }
  }

  // vertical
  if (row > 0) {
    u16 top = row - 1;
    grid_idx_t top_idx = board_idx_at(board, top, col);
    bool match = board_cells_match_top(board, idx, top_idx);
    board->cell_matches[top_idx] = match;
    board->cell_visited[top_idx] = true;
    if (match) {
      found = true;
      game_cell_flash(board, top_idx);
      game_find_matches(board, top_idx);
    }
  }
  if (row + 1 < rows) {
    u16 bottom = row + 1;
    grid_idx_t bottom_idx = board_idx_at(board, bottom, col);
    bool match = board_cells_match_bottom(board, idx, bottom_idx);
    board->cell_matches[bottom_idx] = match;
    board->cell_visited[bottom_idx] = true;
    if (match) {
      found = true;
      game_cell_flash(board, bottom_idx);
      game_find_matches(board, bottom_idx);
    }
  }

  board->cell_visited[idx] = true;
  board->cell_matches[idx] = found;
  if (found) {
    game_cell_flash(board, idx);
  }
}

API void game_sync_layer_fg(game_t *game)
{
  board_t *board = &game->board;
  board_layer_t *fg = &board->layer_fg;

  grid_idx_t landed[2];
  u16 landed_count = 0;

  for (entity_id_t i = fg->count; i > 0; i--) {
    entity_id_t index = i - 1;
    entity_id_t id = fg->index_entity[index];
    if (!tween_completed(board->entity_tween[id])) {
      continue;
    }
    board_layer_swap_bg(board, id);
    landed[landed_count++] = fg->idx[index];
  }

  if (landed_count > 1) {
    mem_set_zero(board->cell_visited, sizeof(bool) * board_grid_size(board));
    mem_set_zero(board->cell_matches, sizeof(bool) * board_grid_size(board));
    for (u16 i = 0; i < landed_count; i++) {
      game_find_matches(board, landed[i]);
    }
  }

}

API void game_init(game_t *game, game_config_t cfg, arena_t *arena)
{
  mem_set_zero(game, sizeof(*game));

  u16 width = m_step(cfg.view_port.x, cfg.cell_size);
  u16 height = m_step(cfg.view_port.y, cfg.cell_size);
  u16 cell_size = cfg.cell_size;
  u16 cols = width / cell_size;
  u16 rows = height / cell_size;

  game->arena  = arena;
  game->config = cfg;
  game->hover_id = ENTITY_NONE;
  game->selected_id = ENTITY_NONE;
  game->selected_idx = IDX_NONE;
  game->board = (board_t){
    .atlas = arena_push(arena, atlas_t, 1),
    .cell_entity = arena_push(arena, entity_id_t, rows * cols),
    .entity_tween = arena_push(arena, tween_h, rows * cols),
    .entity_layer = arena_push(arena, board_layer_t*, rows * cols),
    .cell_matches = arena_push(arena, bool, rows * cols),
    .cell_visited = arena_push(arena, bool, rows * cols),
    .cell_flash = arena_push(arena, float, rows * cols),
    .position = {0, 0},
    .size = {width, height},
    .cell_size = cfg.cell_size,
    .scale = 1.0,
  };

  board_layer_init(&game->board.layer_bg, arena, rows * cols);
  board_layer_init(&game->board.layer_fg, arena, rows * cols);

  mem_set_zero(game->board.cell_matches, rows * cols);
  mem_set_zero(game->board.cell_visited, rows * cols);
  mem_set_zero(game->board.cell_flash, rows * cols * sizeof(float));

  board_sync_size(&game->board);
  game_create_board(game);
}

API void game_process(game_t *game, float delta)
{
  (void) delta;
  game_update_input(game);
  game_sync_layer_fg(game);
}

API void game_draw(game_t *game)
{
  (void) game;
  draw_border();
  draw_board(game);
  draw_fps();
}
