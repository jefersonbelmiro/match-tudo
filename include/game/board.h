#pragma once

#include "core/app.h"
#include "core/arena.h"
#include "core/defs.h"
#include "core/resources.h"
#include "core/tween.h"
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
  atlas_t *atlas;

  entity_id_t    *cell_entity;  // grid idx -> entity
  tween_h        *entity_tween; // entity   -> tween
  board_layer_t **entity_layer; // entity   -> layer

  bool        *cell_visited; // grid idx -> bool
  float       *cell_flash;   // grid idx -> alpha (flash target)

  board_layer_t layer_bg;
  board_layer_t layer_fg;

  Vector2 position;
  Vector2 size;
  u16     cell_size;
  float   scale;
} board_t;

typedef enum {
  CELL_HOVERED,
  CELL_SELECTED,
} board_cell_state;

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

API Vector2 board_idx_to_world(board_t *board, grid_idx_t idx)
{
  u16 cell_size = board->cell_size * board->scale;
  u16 width = board->size.x * board->scale;
  u16 cols = width / cell_size;
  u16 row = idx / cols;
  u16 col = idx % cols;
  return (Vector2){
    board->position.x + cell_size * col + cell_size * 0.5,
    board->position.y + cell_size * row + cell_size * 0.5
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

API bool board_cells_match_dir(board_t *board, grid_idx_t source_idx, grid_idx_t target_idx, int diff_inc)
{
  grid_idx_t source_texture_idx = board_entity_texture(board, board->cell_entity[source_idx]);
  grid_idx_t target_texture_idx = board_entity_texture(board, board->cell_entity[target_idx]);

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

API void board_sync_size(board_t *board)
{
  screen_size_t *screen = app_screen_size();
  u16 width = board->size.x * board->scale;
  u16 height = board->size.y * board->scale;
  float left = screen->x * 0.5 - width  * 0.5;
  float top  = screen->y * 0.5 - height * 0.5;
  board->position = (Vector2){ left, top };
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

