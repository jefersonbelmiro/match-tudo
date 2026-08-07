#pragma once

#include "core/app.h"
#include "core/arena.h"
#include "core/defs.h"
#include "core/mem.h"
#include "core/smath.h"
#include "core/tween.h"
#include "game/board.h"
#include "game/game.h"
#include "game/game_draw.h"
#include "game/level.h"
#include "raylib.h"
#include <stdbool.h>

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
  if (in_board_bounds && !board->completed) {
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

      printn("match count: %d/%d solved: %d", board_match_count(&game->board), board_match_edges_total(&game->board), board_is_solved(&game->board));
      if (board_is_solved(&game->board)) {
        printn("GAME COMPLETED!");
        board->completed = true;

        if (!level_pack_is_last(game->lvl_pack)) {
          arena_reset(game->board_arena);
          level_t *level = level_pack_next(game->lvl_pack);
          board_init(board, game->view_port, level, game->board_arena);
        }
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

API void game_init(game_t *game, arena_t *arena)
{
  mem_set_zero(game, sizeof(*game));

  game->arena  = arena;
  game->board_arena = arena_create_sub(arena, 10, "board");
  game->hover_id = ENTITY_NONE;
  game->selected_id = ENTITY_NONE;
  game->selected_idx = IDX_NONE;

  screen_size_t *screen = app_screen_size();
  game->view_port = (Vector2){ min(screen->x, 400) * 0.9, min(screen->y, 600) * 0.9 };

  game->lvl_pack = level_pack_load(arena);
  if (game->lvl_pack->random) {
    game->lvl_pack->index = m_rand32(0, game->lvl_pack->count - 1);
  }

  level_t *lvl = level_pack_current(game->lvl_pack);

  board_init(&game->board, game->view_port, lvl, game->board_arena);
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
