#pragma once

#include "core/app.h"
#include "core/app_op.h"
#include "core/arena.h"
#include "core/defs.h"
#include "core/smath.h"
#include "game/game_op.h"

typedef struct {
  game_t *game;
} main_scene_t;

API main_scene_t* main_scene_init()
{
  arena_t *arena = app_scene_arena();
  main_scene_t *scene = arena_push(arena, main_scene_t, 1);
  game_t *game = arena_push(arena, game_t, 1);
  screen_size_t *screen = app_screen_size();

  game_config_t cfg = {
    .cell_size = 128,
    .view_port = { min(screen->x, 400) * 0.9, min(screen->y, 600) * 0.9 }
  };
  game_init(game, cfg, arena);
  scene->game = game;

  printn("main_scene_init");
  return scene;
}

API bool main_scene_exiting(UNUSED main_scene_t *scene)
{
  printn("main_scene_exiting");
  return true;
}

API bool main_scene_entering(UNUSED main_scene_t *scene)
{
  printn("main_scene_entering");
  return true;
}

API void main_scene_sync(main_scene_t *scene, sync_signal_type_t signal)
{
  (void) scene; (void) signal;
  printn("main_scene_sync: %d", signal);
  // game_process(delta);
  switch (signal) {
    case SYNC_SIGNAL_WINDOW_RESIZED:
      game_sync_resize(scene->game);
    break;
    default: break;
  }
}

API void main_scene_free(main_scene_t *scene)
{
  (void) scene;
  printn("main_scene_free");
}

API void main_scene_process(main_scene_t *scene, float delta)
{
  (void) scene; (void) delta;
  game_process(scene->game, delta);
}

API void main_scene_draw(main_scene_t *scene)
{
  game_draw(scene->game);
}
