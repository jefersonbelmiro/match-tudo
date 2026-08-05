#pragma once

#include "core/app.h"
#include "core/app_op.h"
#include "core/arena.h"
#include "core/defs.h"
#include "nodes/ui/button_menu.h"

typedef struct {
  button_menu_t menu;
} menu_scene_t;

static void main_menu_on_play(void *l, u8 t, const void *p) 
{
  (void)l; (void)t; (void)p;
  app_set_scene(SCENE_MAIN);
}

#ifdef DEBUG
static void main_menu_on_editor(void *l, u8 t, const void *p) 
{
  (void)l; (void)t; (void)p;
  app_set_scene(SCENE_EDITOR);
}
#endif

static void main_menu_on_exit(void *l, u8 t, const void *p) 
{
  (void)l; (void)t; (void)p;
  app_quit();
}

API menu_scene_t* menu_scene_init()
{
  arena_t *arena = app_scene_arena();
  menu_scene_t *scene = arena_push(arena, menu_scene_t, 1);

  music_play(RESOURCE_MUSIC_MENU_01);

  button_menu_def_t defs[] = {
    { "PLAY", { main_menu_on_play, NULL } },
#ifdef DEBUG
    { "EDITOR", { main_menu_on_editor, NULL } },
#endif // DEBUG
    { "EXIT", { main_menu_on_exit, NULL } },
  };
  button_menu_cfg_t cfg = { .btn_w = 140, .btn_h = 42.0f, .gap = 12 };
  button_menu_init(&scene->menu, expand_of(defs), cfg);
  button_menu_layout(&scene->menu, 140, (float)GetScreenHeight() * 0.5f);
  button_menu_animate_in(&scene->menu, 1.0f);

  return scene;
}

API bool menu_scene_exiting(UNUSED menu_scene_t *scene)
{
  button_menu_t *m = &scene->menu;
  for (u8 i = 0; i < m->btn_count; i++) {
    if (tween_is_active(m->in_out_tween[i])) return false;
  }
  return true;
}

API bool menu_scene_entering(UNUSED menu_scene_t *scene)
{                  
  return true;
}

API void menu_scene_sync(menu_scene_t *scene, sync_signal_type_t signal)
{
  (void) scene;
  switch (signal) {
    case SYNC_SIGNAL_WINDOW_RESIZED: {
      button_menu_layout(&scene->menu, 140, (float)GetScreenHeight() * 0.5f);
      break;
    }
    case SYNC_SIGNAL_ON_EXIT: {
      float duration = 0.35f;
      button_menu_animate_out(&scene->menu, duration);
      break;
    }

    default: break;
  }
}

API void menu_scene_free(menu_scene_t *scene)
{
  (void) scene;
}

API void menu_scene_process(menu_scene_t *scene, float delta)
{
  if (IsKeyPressed(KEY_ESCAPE)) {
    app_quit();
  }
  button_menu_process(&scene->menu, delta);
}

API void menu_scene_draw(menu_scene_t *scene)
{
  button_menu_draw(&scene->menu);
}
