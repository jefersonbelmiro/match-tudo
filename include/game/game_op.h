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

#define GAME_VIEW_PORT_WIDTH   400
#define GAME_VIEW_PORT_HEIGHT  600
#define GAME_HUD_TOPBAR_HEIGHT 48
#define GAME_HUD_FOOTER_HEIGHT 48

API view_port_t game_view_port_max()
{
  return (view_port_t){
    .x = 0,
    .y = 0,
    .width = GAME_VIEW_PORT_WIDTH,
    .height = GAME_VIEW_PORT_HEIGHT,
  };
}

API view_port_t game_view_port_board(view_port_t vp)
{
  return (view_port_t){
    .x = vp.x,
    .y = vp.y + GAME_HUD_TOPBAR_HEIGHT,
    .width = vp.width,
    .height = vp.height - GAME_HUD_TOPBAR_HEIGHT - GAME_HUD_FOOTER_HEIGHT,
  };
}

API void game_sync_view_port(game_t *game)
{
  view_port_t max_vp = game_view_port_max();
  screen_size_t *screen = app_screen_size();
  float width = min(screen->x, max_vp.width);
  float height = min(screen->y, max_vp.height);
  game->view_port = (view_port_t){ 
    .x = m_floor(screen->x * 0.5 - width  * 0.5),
    .y = 0,
    .width = width,
    .height = height,
  };
}

API void game_sync_size(game_t *game)
{
  board_t *board = &game->board;
  game_sync_view_port(game);

  view_port_t board_vp = game_view_port_board(game->view_port);
  if (board_vp.width < 1.0f || board_vp.height < 1.0f) {
    return;
  }

  board->scale = 1.0f;
  if (board->size.x > board_vp.width || board->size.y > board_vp.height) {
    float scale_x = board_vp.width  / board->size.x;
    float scale_y = board_vp.height / board->size.y;
    board->scale = min(scale_x, scale_y);
  }

  float scaled_width  = board->size.x * board->scale;
  float scaled_height = board->size.y * board->scale;
  board->position = (Vector2){
    m_floor(board_vp.x + (board_vp.width  - scaled_width)  * 0.5f),
    m_floor(board_vp.y + (board_vp.height - scaled_height) * 0.5f),
  };

  printn("[game_sync_size]:");
  printn(" - vp: (%g, %g, %g, %g)", game->view_port.x, game->view_port.y, game->view_port.width, game->view_port.height);
  printn(" - board vp: (%g, %g, %g, %g)", board_vp.x, board_vp.y, board_vp.width, board_vp.height);
  printn(" - size: (%g, %g)", board->size.x, board->size.y);
  printn(" - scale: %g", board->scale);
  printn(" - position: (%g, %g)", board->position.x, board->position.y);

  board_sync_position(&game->board);
}

API void game_update_input(game_t *game)
{
  board_t *board = &game->board;
  float cell_size = board->cell_size * board->scale;
  Vector2 mouse = GetMousePosition();
  float width = board->size.x * board->scale;
  float height = board->size.y * board->scale;

  float left = board->position.x;
  float top  = board->position.y;

  bool in_board_bounds = true;
  if (mouse.x < left || mouse.x >= left + width) {
    in_board_bounds = false;
  } 
  else if (mouse.y < top || mouse.y >= top + height) {
    in_board_bounds = false;
  }

  u16 cols = board_cols(board);
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

        // @todo: 
        //  #1 - start timer to show win effects
        //  #2 - recreate board from "next" button
        //  #2.2 - on "next" show level map selector
        //  #2.3 - recreate board from "level map selector"
        if (!level_pack_is_last(game->lvl_pack)) {
          level_t *level = level_pack_next(game->lvl_pack);
          arena_reset(game->board_arena);
          view_port_t board_vp = game_view_port_board(game_view_port_max());
          board_init(board, board_vp, level, game->board_arena);
          game_sync_size(game);

          game->hover_id = ENTITY_NONE;
          // game->selected_id = ENTITY_NONE;
          // game->selected_idx = IDX_NONE;
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

  game->lvl_pack = level_pack_load(arena);
  if (game->lvl_pack->random) {
    game->lvl_pack->index = m_rand32(0, game->lvl_pack->count - 1);
  }

  level_t *lvl = level_pack_current(game->lvl_pack);

  view_port_t board_vp = game_view_port_board(game_view_port_max());
  board_init(&game->board, board_vp, lvl, game->board_arena);
  game_sync_size(game);
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
