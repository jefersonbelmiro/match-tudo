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

  if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && game->selected_id != ENTITY_NONE) {

    if (game->hover_id != ENTITY_NONE && game->hover_id != game->selected_id && 
        !tween_is_active(board->entity_tween[game->hover_id])
    ) {
      board->cell_entity[game->selected_idx] = game->hover_id;
      board->cell_entity[idx] = game->selected_id;

      board_layer_swap_fg(board, game->hover_id);

      board_grid_snap_animated(board, game->selected_id, idx);
      board_grid_snap_animated(board, game->hover_id, game->selected_idx);

      board_compute_edges(&game->board);

      printn("match count: %d/%d", board_match_count(&game->board), board_grid_size(&game->board));
      if (board_match_count(&game->board) == board_grid_size(&game->board)) {
        printn("GAME COMPLETED!");
      }

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

API void game_cell_flash(board_t *board, grid_idx_t idx)
{
  board->cell_flash[idx] = 1.0f;
}

API void game_flash_matches(board_t *board, grid_idx_t idx)
{
  if (board->cell_visited[idx]) {
    return;
  }
  board->cell_visited[idx] = true;

  u16 cols = board_cols(board);
  u8 edges = board->cell_edges[idx];
  bool matched = false;

  if (edges & EDGE_LEFT)   { matched = true; game_flash_matches(board, idx - 1); }
  if (edges & EDGE_RIGHT)  { matched = true; game_flash_matches(board, idx + 1); }
  if (edges & EDGE_TOP)    { matched = true; game_flash_matches(board, idx - cols); }
  if (edges & EDGE_BOTTOM) { matched = true; game_flash_matches(board, idx + cols); }

  if (matched) {
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
    for (u16 i = 0; i < landed_count; i++) {
      game_flash_matches(board, landed[i]);
    }
  }
}

API void game_init(game_t *game, game_config_t cfg, arena_t *arena)
{
  mem_set_zero(game, sizeof(*game));

  game->arena  = arena;
  game->config = cfg;
  game->hover_id = ENTITY_NONE;
  game->selected_id = ENTITY_NONE;
  game->selected_idx = IDX_NONE;
  board_init(&game->board, cfg.view_port, cfg.cell_size, arena);

  board_sync_size(&game->board);
  game_create_board(game);
  board_compute_edges(&game->board);
}

API void game_process(game_t *game, float delta)
{
  game_update_input(game);
  game_sync_layer_fg(game);

  board_t *board = &game->board;
  float fade = delta / MATCH_FLASH_DURATION;
  for (grid_idx_t idx = 0; idx < board_grid_size(board); idx++) {
    if (board->cell_flash[idx] > 0.0f) {
      board->cell_flash[idx] -= fade;
      if (board->cell_flash[idx] < 0.0f) {
        board->cell_flash[idx] = 0.0f;
      }
    }
  }
}

API void game_draw(game_t *game)
{
  (void) game;
  draw_border();
  draw_board(game);
  draw_fps();
}
