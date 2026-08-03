#pragma once

#include "core/arena.h"

typedef enum {
  EDITOR_ACTION_NONE,
  EDITOR_ACTION_NODE_ADD,
  EDITOR_ACTION_NODE_ERASE,
  EDITOR_ACTION_NODE_LINK,
  EDITOR_ACTION_NODE_MOVE,
  EDITOR_ACTION_COUNT,
} editor_action_t;

typedef struct {
  arena_t *arena;
  editor_action_t action;
} editor_t;

API void editor_init(editor_t *editor, arena_t *arena)
{
  *editor = (editor_t){
    .arena = arena,
    .action = EDITOR_ACTION_NONE
  };
}

API void editor_process(editor_t *editor, float delta)
{
  (void) editor; (void) delta;
}

API void editor_draw(editor_t *editor)
{
  (void) editor;
}
