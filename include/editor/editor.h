#pragma once

#include "core/arena.h"

typedef struct {
  arena_t *arena;
} editor_t;

API void editor_init(editor_t *editor, arena_t *arena)
{
  mem_set_zero(editor, sizeof(*editor));
  editor->arena = arena;
}
