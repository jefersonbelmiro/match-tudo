#pragma once

#include "core/app.h"
#include "core/arena.h"
#include "core/defs.h"
#include "core/resources.h"
#include "core/tween.h"
#include "raylib.h"

typedef struct {
  entity_id_t *entity_index;
  entity_id_t *index_entity;

  grid_idx_t *idx;
  grid_idx_t *target_idx;
  grid_idx_t *texture_idx;
  Vector2    *position;

  entity_id_t count;
  entity_id_t cap;
} board_layer_t;

typedef struct {
  atlas_t *atlas;

  entity_id_t    *cell_id; // idx -> id
  tween_h        *cell_tween; // entity -> tween
  board_layer_t **cell_layer; // entity -> layer

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
  layer->target_idx = arena_push(arena, grid_idx_t, cap);
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
  layer->target_idx[index] = IDX_NONE;
  layer->texture_idx[index] = texture_idx;
  layer->position[index] = position;
}

API void board_layer_erase(board_layer_t *layer, entity_id_t id)
{
  assert(id <= layer->cap);
  entity_id_t index = layer->entity_index[id];
  entity_id_t last = layer->count - 1;
  if (index != last) {
    entity_id_t last_id = layer->index_entity[last];
    layer->entity_index[last_id] = index;
    layer->index_entity[index] = last_id;
    layer->idx[index] = layer->idx[last];
    layer->target_idx[index] = layer->target_idx[last];
    layer->texture_idx[index] = layer->texture_idx[last];
    layer->position[index] = layer->position[last];
  }
  layer->count--;
}

API void board_layer_swap(board_layer_t *source, board_layer_t *target, entity_id_t id)
{
  if (source == target) return;

  entity_id_t source_index = source->entity_index[id];
  board_layer_append(target, id, source->idx[source_index], source->position[source_index], source->texture_idx[source_index]);
  board_layer_erase(source, id);
}

API void board_sync_size(board_t *board)
{
  screen_size_t *screen = app_screen_size();
  u16 width = board->size.x * board->scale;
  u16 height = board->size.y * board->scale;
  float left = screen->x * 0.5 - width  * 0.5;
  float top  = screen->y * 0.5 - height * 0.5;
  board->position = (Vector2){ left, top };
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
  board_layer_t *layer = board->cell_layer[id];
  entity_id_t index = layer->entity_index[id];
  layer->position[index] = board_idx_to_world(board, idx);
}

API void board_grid_snap_animated(board_t *board, entity_id_t id, grid_idx_t target_idx)
{
  board_layer_t *layer = board->cell_layer[id];
  entity_id_t index = layer->entity_index[id];

  Vector2 *position = &layer->position[index];
  Vector2 target = board_idx_to_world(board, target_idx);

  tween_kill(board->cell_tween[id]);

  tween_h h = tween_create_parallel();
  tween_add(h, &position->x, target.x, 0.2f, ease_out_quad);
  tween_add(h, &position->y, target.y, 0.2f, ease_out_quad);
  board->cell_tween[id] = h;

  layer->target_idx[index] = target_idx;
}
