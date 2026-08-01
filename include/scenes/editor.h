#pragma once

#include "core/app.h"
#include "core/app_op.h"
#include "core/arena.h"
#include "core/defs.h"
#include "editor/editor.h"

typedef struct {
  editor_t *editor;
} editor_scene_t;

API editor_scene_t* editor_scene_init()
{
  arena_t *arena = app_scene_arena();
  editor_scene_t *scene = arena_push(arena, editor_scene_t, 1);
  editor_t *editor = arena_push(arena, editor_t, 1);
  editor_init(editor, arena);
  scene->editor = editor;

  return scene;
}

API bool editor_scene_exiting(UNUSED editor_scene_t *scene)
{
  return true;
}

API bool editor_scene_entering(UNUSED editor_scene_t *scene)
{
  return true;
}

API void editor_scene_sync(editor_scene_t *scene, sync_signal_type_t signal)
{
  (void) scene;
  switch (signal) {
    case SYNC_SIGNAL_WINDOW_RESIZED:
      // editor_sync_size(scene->editor);
      break;
    default: break;
  }
}

API void editor_scene_free(editor_scene_t *scene)
{
  (void) scene;
}

API void editor_scene_process(editor_scene_t *scene, float delta)
{
  (void) scene; (void) delta;
}

API void editor_scene_draw(editor_scene_t *scene)
{
  (void) scene;
}
