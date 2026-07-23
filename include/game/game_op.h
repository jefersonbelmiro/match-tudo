#pragma once

#include "core/app.h"
#include "core/defs.h"
#include "core/mem.h"
#include "core/resources.h"
#include "core/smath.h"
#include "game/board.h"
#include "game/game.h"
#include "game/game_draw.h"
#include "raylib.h"

API void game_create_board(game_t *game)
{
  Texture2D target_texture = resource_texture(RESOURCE_TEXTURE_001);
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

  u16 grid_size = rows * cols;
  for (u16 idx = 0; idx < grid_size; idx++) {
    u16 row = idx / cols;
    u16 col = idx % cols;

    board->value[idx] = idx;
    board->cell_position[idx] = (Vector2){
      cell_offset_x + cell_size * col,
      cell_offset_y + cell_size * row
    };
  }
  // fisher-yates/knuth shuffle
  for (u16 i = grid_size - 1; i > 0; i--) {
    u16 j = m_rand32(0, i);
    u16 tmp = board->value[i];
    board->value[i] = board->value[j];
    board->value[j] = tmp;
  }

  hot_sync({
    printn("\n\n");
    printn("idx size: %d", rows * cols);
    printn("target_texture: %d/%d", target_texture.width, target_texture.height);
    printn("target_texture scale: %g", texture_scale);
    printn("target_texture offset_top: %g", texture_offset_top);
    printn("board: w:%d h:%d", board->size.x, board->size.y);
    printn("\n\n");
  });
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
  game->hover = IDX_NONE;
  game->selected = IDX_NONE;
  game->board = (board_t){
    .value = arena_push(arena, grid_idx_t, rows * cols),
    .atlas = arena_push(arena, atlas_t, 1),
    .cell_position = arena_push(arena, Vector2, rows * cols),
    .cell_size = cfg.cell_size,
    .size = {width, height},
    .position = {0, 0},
    .scale = 1.0,
  };

  board_sync_resize(&game->board);
  game_create_board(game);
}

API void game_sync_resize(game_t *game)
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

  board_sync_resize(&game->board);
  printn("board w: %d h:%d scale: %g", board->size.x, board->size.y, board->scale);
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

  game->hover = IDX_NONE;
  if (in_board_bounds) {
    game->hover = idx;
  }

  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && game->hover != IDX_NONE) {
    printn("left presed: %d", game->hover);
    // Vector2 cell_position = {
    //   board->position.x + cell_size * 0.5 + cell_size * col,
    //   board->position.y + cell_size * 0.5 + cell_size * row
    // };
    Vector2 cell_position = game->board.cell_position[game->hover];

    game->selected = idx;
    game->selected_offset = (Vector2){ 
      GetMouseX() - cell_position.x, 
      GetMouseY() - cell_position.y
    };
  }
  else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && game->selected != IDX_NONE) {
    printn("left released: selected: %d hover: %d", game->selected, idx);

    if (game->hover != IDX_NONE) {
      grid_idx_t *value = game->board.value;
      grid_idx_t hover_value = value[game->hover];
      value[game->hover] = value[game->selected];
      value[game->selected] = hover_value;

      // Vector2 *positions = board->cell_position;
      // Vector2 hover_pos = positions[game->hover];
      // value[game->hover] = value[game->selected];
      // value[game->selected] = hover_value;



      // Vector2 *pos = game->board.cell_position;
      // Vector2 sel_target = pos[game->hover];
      // Vector2 hov_target = board_idx_to_world(board, game->selected);
      // tween_h h = tween_create_parallel();
      // tween_add(h, &pos[game->selected].x, sel_target.x, 0.2f, ease_out_quad);
      // tween_add(h, &pos[game->selected].y, sel_target.y, 0.2f, ease_out_quad);
      // tween_add(h, &pos[game->hover].x, hov_target.x, 0.2f, ease_out_quad);
      // tween_add(h, &pos[game->hover].y, hov_target.y, 0.2f, ease_out_quad);
    } else {
      // Vector2 *pos = game->board.cell_position;
      // Vector2 target = board_idx_to_world(board, game->selected);
      // tween_h h = tween_create_parallel();
      // tween_add(h, &pos[game->selected].x, target.x, 0.2f, ease_out_quad);
      // tween_add(h, &pos[game->selected].y, target.y, 0.2f, ease_out_quad);
    }

    game->selected = IDX_NONE;
  } 

  if (game->selected != IDX_NONE) {
    game->board.cell_position[game->selected] = (Vector2){ 
      GetMouseX() - game->selected_offset.x, 
      GetMouseY() - game->selected_offset.y
    };
  }
}


API void game_process(game_t *game, float delta)
{
  (void) delta;
  game_update_input(game);
}

API void game_draw(game_t *game)
{
  (void) game;
  draw_border();
  draw_board(game);
  draw_hover(game);
}
