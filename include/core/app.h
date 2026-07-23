#pragma once

#include "core/arena.h"
#include "core/defs.h"

typedef struct {
  arena_t *arena;
  arena_t *scene_arena;
  app_state_t state;

  scene_type_t           scene;
  scene_type_t           scene_next;
  void                  *scene_state;
  app_scene_transition_t scene_transition;
  
  screen_size_t screen_size;

} app_t;

GLOBAL app_t *g_app;

API app_t* app_ptr();
API bool app_paused();
API void app_emit_hot_sync();
API arena_t* app_scene_arena();
API void app_quit();
API void app_pause(bool paused);
API screen_size_t* app_screen_size();


