#pragma once

#include "core/app.h"
#include "core/arena.h"
#include "core/defs.h"
#include "core/draw.h"
#include "core/resources.h"
#include "core/smath.h"
#include "core/tween.h"
#include "game/level.h"
#include "raylib.h"
#include <stdbool.h>

typedef struct {
  entity_id_t *entity_index;
  entity_id_t *index_entity;

  grid_idx_t *idx;
  grid_idx_t *texture_idx;
  Vector2    *position;

  entity_id_t count;
  entity_id_t cap;
} board_layer_t;

typedef struct {
  grid_idx_t *entity_index;
  float      *alpha;
  u16 count;
} board_flash_table_t;

typedef struct {
  Texture2D source;       // level image (crop source)
  float     tex_scale;    // source -> board-canvas scale
  float     tex_origin_x; // source top-left in board-canvas coords
  float     tex_origin_y;

  entity_id_t    *cell_entity;  // grid idx -> entity
  tween_h        *entity_tween; // entity   -> tween
  board_layer_t **entity_layer; // entity   -> layer

  bool        *cell_visited; // grid idx -> bool
  float       *cell_flash;   // grid idx -> alpha (flash target)
  u8          *cell_edges;   // grid idx -> EDGE_* bits (match cache)

  board_flash_table_t *board_flash;

  board_layer_t layer_bg;
  board_layer_t layer_fg;

  bool    completed;
  bool    draw_numbers;

  Vector2 position;
  Vector2 size;
  u16     cell_size;
  float   scale;
} board_t;

typedef enum {
  CELL_HOVERED,
  CELL_SELECTED,
} board_cell_state;

API void board_create(board_t *board, level_t *level);
API void board_sync_position(board_t *board);
API void board_compute_edges(board_t *board);

API void board_layer_init(board_layer_t *layer, arena_t *arena, entity_id_t cap)
{
  layer->entity_index = arena_push(arena, entity_id_t, cap);
  layer->index_entity = arena_push(arena, entity_id_t, cap);
  layer->idx = arena_push(arena, grid_idx_t, cap);
  layer->texture_idx = arena_push(arena, grid_idx_t, cap);
  layer->position = arena_push(arena, Vector2, cap);
  layer->count = 0;
  layer->cap = cap;
}

API void board_init(board_t *board, view_port_t view_port, level_t *level, arena_t *arena) 
{
  u16 cell_size = level->cell_size;
  u16 width = m_step(view_port.width, cell_size);
  u16 height = m_step(view_port.height, cell_size);
  u16 cols = width / cell_size;
  u16 rows = height / cell_size;

  *board = (board_t){
    .cell_entity = arena_push(arena, entity_id_t, rows * cols),
    .entity_tween = arena_push(arena, tween_h, rows * cols),
    .entity_layer = arena_push(arena, board_layer_t*, rows * cols),
    .cell_visited = arena_push(arena, bool, rows * cols),
    .cell_flash = arena_push(arena, float, rows * cols),
    .cell_edges = arena_push(arena, u8, rows * cols),
    .position = {0, 0},
    .size = {width, height},
    .cell_size = cell_size,
    .scale = 1.0,
  };

  board_layer_init(&board->layer_bg, arena, rows * cols);
  board_layer_init(&board->layer_fg, arena, rows * cols);

  mem_set_zero(board->cell_visited, rows * cols);
  mem_set_zero(board->cell_flash, rows * cols * sizeof(float));
  mem_set_zero(board->cell_edges, rows * cols);

  board_sync_position(board);
  board_create(board, level);
  board_compute_edges(board);
}

API void board_layer_append(
  board_layer_t *layer, 
  entity_id_t id,
  grid_idx_t idx,
  Vector2 position,
  grid_idx_t texture_idx) 
{
  assert(id <= layer->cap);
  entity_id_t index = layer->count++;
  layer->entity_index[id] = index;
  layer->index_entity[index] = id;
  layer->idx[index] = idx;
  layer->texture_idx[index] = texture_idx;
  layer->position[index] = position;
}

API void board_layer_erase(board_t *board, board_layer_t *layer, entity_id_t id)
{
  assert(id <= layer->cap);
  entity_id_t index = layer->entity_index[id];
  entity_id_t last = layer->count - 1;
  if (index != last) {
    entity_id_t last_id = layer->index_entity[last];
    tween_h last_tween = board->entity_tween[last_id];
    layer->entity_index[last_id] = index;
    layer->index_entity[index] = last_id;
    layer->idx[index] = layer->idx[last];
    layer->texture_idx[index] = layer->texture_idx[last];
    layer->position[index] = layer->position[last];
    if (tween_is_active(last_tween)) {
      tween_retarget(last_tween, 0, &layer->position[index].x);
      tween_retarget(last_tween, 1, &layer->position[index].y);
    }
  }
  layer->count--;
}

API void board_layer_swap(board_t *board, board_layer_t *source, board_layer_t *target, entity_id_t id)
{
  assert(source != target);
  entity_id_t source_index = source->entity_index[id];
  board_layer_append(target, id, source->idx[source_index], source->position[source_index], source->texture_idx[source_index]);
  board_layer_erase(board, source, id);
  board->entity_layer[id] = target;
}

API void board_layer_swap_bg(board_t *board, entity_id_t id)
{
  board_layer_t *source = &board->layer_fg;
  board_layer_t *target = &board->layer_bg;
  board_layer_t *current = board->entity_layer[id];
  if (current == target) {
    return;
  }
  board_layer_swap(board, source, target, id);
}

API void board_layer_swap_fg(board_t *board, entity_id_t id)
{
  board_layer_t *source = &board->layer_bg;
  board_layer_t *target = &board->layer_fg;
  board_layer_t *current = board->entity_layer[id];
  if (current == target) {
    return;
  }
  board_layer_swap(board, source, target, id);
}

API Rectangle board_cell_source_rect(board_t *board, grid_idx_t texture_idx)
{
  u16 cols = board->size.x / board->cell_size;
  u16 col = texture_idx % cols;
  u16 row = texture_idx / cols;
  float s = board->tex_scale;
  float cell = board->cell_size;
  return (Rectangle){
    (col * cell - board->tex_origin_x) / s,
    (row * cell - board->tex_origin_y) / s,
    cell / s,
    cell / s,
  };
}

API Vector2 board_idx_to_world(board_t *board, grid_idx_t idx)
{
  u16 cell_size = board->cell_size * board->scale;
  u16 width = board->size.x * board->scale;
  u16 cols = width / cell_size;
  u16 row = idx / cols;
  u16 col = idx % cols;
  return (Vector2){
    m_floor(board->position.x + cell_size * col + cell_size * 0.5),
    m_floor(board->position.y + cell_size * row + cell_size * 0.5)
  };
}

API void board_grid_snap(board_t *board, entity_id_t id, grid_idx_t idx)
{
  board_layer_t *layer = board->entity_layer[id];
  entity_id_t index = layer->entity_index[id];
  layer->position[index] = board_idx_to_world(board, idx);
}

API void board_grid_snap_animated(board_t *board, entity_id_t id, grid_idx_t target_idx)
{
  board_layer_t *layer = board->entity_layer[id];
  entity_id_t index = layer->entity_index[id];

  Vector2 *position = &layer->position[index];
  Vector2 target = board_idx_to_world(board, target_idx);

  tween_kill(board->entity_tween[id]);

  tween_h h = tween_create_parallel();
  tween_add(h, &position->x, target.x, 0.2f, ease_out_quad);
  tween_add(h, &position->y, target.y, 0.2f, ease_out_quad);
  board->entity_tween[id] = h;

  layer->idx[index] = target_idx;
}

API u16 board_cols(board_t *board)
{
  return board->size.x / board->cell_size;
}

API u16 board_rows(board_t *board)
{
  return board->size.y / board->cell_size;
}

API u16 board_grid_size(board_t *board)
{
  return board_rows(board) * board_cols(board);
}

API grid_idx_t board_entity_texture(board_t *board, entity_id_t id)
{
  board_layer_t *layer = board->entity_layer[id];
  return layer->texture_idx[layer->entity_index[id]];
}

// directional match: the target neighbor's texture is one step ahead of source
//   right  -> texture(source) + 1   == texture(target)
//   left   -> texture(source) - 1   == texture(target)
//   bottom -> texture(source) + cols == texture(target)
//   top    -> texture(source) - cols == texture(target)
API bool board_cells_match_dir(board_t *board, grid_idx_t source_idx, grid_idx_t target_idx, int diff_inc)
{
  grid_idx_t source_texture_idx = board_entity_texture(board, board->cell_entity[source_idx]);
  grid_idx_t target_texture_idx = board_entity_texture(board, board->cell_entity[target_idx]);
  u16 cols = board_cols(board);

  if (diff_inc == 1 || diff_inc == -1) {
    // horizontal: same image row, consecutive columns
    return source_texture_idx + diff_inc == target_texture_idx
        && source_texture_idx / cols == target_texture_idx / cols;
  }
  // vertical: texture diff == cols implies same image column
  return source_texture_idx + diff_inc == target_texture_idx;
}

API bool board_cells_match_left(board_t *board, grid_idx_t source_idx, grid_idx_t target_idx)
{
  return board_cells_match_dir(board, source_idx, target_idx, -1);
}

API bool board_cells_match_right(board_t *board, grid_idx_t source_idx, grid_idx_t target_idx)
{
  return board_cells_match_dir(board, source_idx, target_idx, +1);
}

API bool board_cells_match_bottom(board_t *board, grid_idx_t source_idx, grid_idx_t target_idx)
{
  return board_cells_match_dir(board, source_idx, target_idx, board_cols(board));
}

API bool board_cells_match_top(board_t *board, grid_idx_t source_idx, grid_idx_t target_idx)
{
  return board_cells_match_dir(board, source_idx, target_idx, -board_cols(board));
}

API void board_compute_edges(board_t *board)
{
  u16 cols = board_cols(board);
  u16 rows = board_rows(board);
  for (u16 row = 0; row < rows; row++) {
    for (u16 col = 0; col < cols; col++) {
      grid_idx_t idx = row * cols + col;
      u8 edges = 0;
      if (col > 0        && board_cells_match_left(board, idx, idx - 1))      edges |= EDGE_LEFT;
      if (col + 1 < cols && board_cells_match_right(board, idx, idx + 1))     edges |= EDGE_RIGHT;
      if (row > 0        && board_cells_match_top(board, idx, idx - cols))    edges |= EDGE_TOP;
      if (row + 1 < rows && board_cells_match_bottom(board, idx, idx + cols)) edges |= EDGE_BOTTOM;
      board->cell_edges[idx] = edges;
    }
  }
}

API u16 board_match_count(board_t *board)
{
  u16 cols = board_cols(board);
  u16 rows = board_rows(board);
  u16 count = 0;
  for (u16 row = 0; row < rows; row++) {
    for (u16 col = 0; col < cols; col++) {
      u8 edges = board->cell_edges[row * cols + col];
      if (edges & EDGE_RIGHT)  count += 1;
      if (edges & EDGE_BOTTOM) count += 1;
    }
  }
  return count;
}

API u16 board_match_edges_total(board_t *board)
{
  u16 cols = board_cols(board);
  u16 rows = board_rows(board);
  return rows * (cols - 1) + cols * (rows - 1);
}

API bool board_is_solved(board_t *board)
{
  u16 rows = board_rows(board);
  u16 cols = board_cols(board);
  for (u16 idx = 0; idx < rows * cols; idx++) {
    if (board_entity_texture(board, board->cell_entity[idx]) != idx) {
      return false;
    }
  }
  return true;
}

API bool board_cell_is_matched(board_t *board, grid_idx_t idx)
{
  return board->cell_edges[idx] != 0;
}

API void board_sync_position(board_t *board)
{
  // screen_size_t *screen = app_screen_size();
  // u16 width = board->size.x * board->scale;
  // u16 height = board->size.y * board->scale;
  // float left = m_floor(screen->x * 0.5 - width  * 0.5);
  // float top  = m_floor(screen->y * 0.5 - height * 0.5);
  // board->position = (Vector2){ left, top };
  {
    board_layer_t *layer = &board->layer_bg;
    for (entity_id_t i = 0; i < layer->count; i++) {
      grid_idx_t idx = layer->idx[i];
      layer->position[i] = board_idx_to_world(board, idx);
    }
  }
  {
    board_layer_t *layer = &board->layer_fg;
    for (entity_id_t i = 0; i < layer->count; i++) {
      grid_idx_t idx = layer->idx[i];
      layer->position[i] = board_idx_to_world(board, idx);
    }
  }
}


API void board_create(board_t *board, level_t *level)
{
  board->source = resource_texture(level->texture_idx);
  float texture_scale = 1.0;

  if (board->source.height > board->source.width) {
    texture_scale = (float)board->size.x / board->source.width;
  } else if (board->source.width > board->source.height) {
    if (board->size.y > board->size.x) {
      texture_scale = (float)board->size.y / board->source.height;
    } else {
      texture_scale = (float)board->size.x / board->source.width;
    }
  }

  float texture_offset_top = board->source.height * texture_scale * 0.5;
  if (board->source.height > board->size.y) {
    texture_offset_top -= (board->source.height * texture_scale - board->size.y) * 0.5;
  }
  float texture_offset_left = board->source.width * texture_scale * 0.5;
  if (board->source.width > board->size.x) {
    texture_offset_left -= (board->source.width * texture_scale - board->size.x) * 0.5;
  }

  board->tex_scale = texture_scale;
  board->tex_origin_x = texture_offset_left - board->source.width  * texture_scale * 0.5f;
  board->tex_origin_y = texture_offset_top  - board->source.height * texture_scale * 0.5f;

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

