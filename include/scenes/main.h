#pragma once

#include "core/app.h"
#include "core/app_op.h"
#include "core/arena.h"
#include "core/defs.h"
#include "core/resources.h"
#include "core/smath.h"
#include "core/sound.h"
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

  grid_idx_t textures[] = {
    RESOURCE_TEXTURE_001,
    RESOURCE_TEXTURE_002,
    RESOURCE_TEXTURE_003,
    RESOURCE_TEXTURE_004,
    RESOURCE_TEXTURE_005,
  };
  grid_idx_t texture_idx = textures[m_rand32(0, countof(textures) - 1)];

  game_config_t cfg = {
    .cell_size = 128,
    .view_port = { min(screen->x, 400) * 0.9, min(screen->y, 600) * 0.9 },
    .texture_idx = texture_idx,
  };
  game_init(game, cfg, arena);
  scene->game = game;

  music_play(RESOURCE_MUSIC_LOOP_01);

  return scene;
}

API bool main_scene_exiting(UNUSED main_scene_t *scene)
{
  return true;
}

API bool main_scene_entering(UNUSED main_scene_t *scene)
{
  return true;
}

API void main_scene_sync(main_scene_t *scene, sync_signal_type_t signal)
{
  switch (signal) {
    case SYNC_SIGNAL_WINDOW_RESIZED:
      game_sync_size(scene->game);
    break;
    case SYNC_SIGNAL_ON_EXIT: {
      tween_cancel_all();
      timer_cancel_all();
      break;
    }

    default: break;
  }
}

API void main_scene_free(main_scene_t *scene)
{
  (void) scene;
}

API void main_scene_process(main_scene_t *scene, float delta)
{
  if (IsKeyPressed(KEY_ESCAPE)) {
    app_set_scene(SCENE_MENU);
    return;
  }
  game_process(scene->game, delta);
}

API void main_scene_draw(main_scene_t *scene)
{
  game_draw(scene->game);
}
