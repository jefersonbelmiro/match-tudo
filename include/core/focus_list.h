#pragma once

#include "core/defs.h"
#include <raylib.h>

typedef struct {
  u8 count;
  u8 index;
} focus_list_t;

API void focus_list_init(focus_list_t *fl, u8 count)
{
  fl->count = count;
  fl->index = 0;
}

API void focus_list_next(focus_list_t *fl)
{
  if (fl->count == 0) return;
  fl->index = (fl->index + 1) % fl->count;
}

API void focus_list_prev(focus_list_t *fl)
{
  if (fl->count == 0) return;
  fl->index = (fl->index + fl->count - 1) % fl->count;
}

// Call from the owner's process — handles arrow keys + vim keys (j/k).
API void focus_list_process(focus_list_t *fl)
{
  if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_J)) focus_list_next(fl);
  if (IsKeyPressed(KEY_UP)   || IsKeyPressed(KEY_K)) focus_list_prev(fl);
}
