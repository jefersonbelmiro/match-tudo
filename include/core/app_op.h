#pragma once

#include "core/app.h"
#include "core/arena.h"
#include "core/defs.h"
#include "core/resources.h"
#include "core/timer.h"
#include "core/tween.h"
#include "raylib.h"
#include "scenes/main.h"

API app_t* app_ptr()
{
  return g_app;
}

API bool app_paused()
{
  app_t *app = app_ptr();
  return app->state == APP_PAUSED;
}

API screen_size_t* app_screen_size()
{
  return &app_ptr()->screen_size;
}

API void app_set_scene(scene_type_t type)
{
  app_t *app = app_ptr();
  app->scene_next = type;
}

API arena_t* app_scene_arena()
{
  app_t *app = app_ptr();
  return app->scene_arena;
}

API void app_init(void)
{
  assert(!g_app);
  arena_t *arena = arena_create(APP_ARENA_SIZE, "app");
  app_t *app = arena_push_zero(arena, app_t, 1);
  g_app = app;

  resource_init(arena_create_sub(arena, resources_memory_size(), "resource"));
  tween_init(arena_create_sub(arena, tween_memory_size(), "tween"));
  timer_init(arena_create_sub(arena, timer_memory_size(), "timer"));

  app->scene = SCENE_NONE;
  app->arena = arena;
  // keep scene arena to end, for cache locality(i think)
  app->scene_arena = arena_create_sub(arena, APP_SCENE_ARENA_SIZE, "scene");
}

API void app_fini()
{
  app_t *app = app_ptr();
  resource_unload();
  arena_fini(app->arena);
  g_app = NULL;
}

API void app_start(void)
{
  resource_start();

  app_t *app = app_ptr();
  app->state = APP_RUNNING;
}

API void app_quit()
{
  app_t *app = app_ptr();
  if (app->state != APP_EXITING) {
    app->state = APP_EXITING;
  }
}

API void app_pause(bool paused)
{
  app_t *app = app_ptr();
  if (app->state == APP_EXITING || app->state == APP_EXITED) {
    return;
  }
  app->state = paused ? APP_PAUSED : APP_RUNNING;
}

static bool app__scene_entering() 
{
  app_t *app = app_ptr();
  switch (app->scene) {
  case SCENE_MAIN:
    return main_scene_entering(app->scene_state);
  default: return true;
  }
}

static bool app__scene_exiting() 
{
  app_t *app = app_ptr();
  switch (app->scene) {
    case SCENE_MAIN:
      return main_scene_exiting(app->scene_state);
    default: return true;
  }
}

static void app__scene_init() 
{
  app_t *app = app_ptr();
  switch (app->scene) {
    case SCENE_MAIN:
      app->scene_state = main_scene_init();
    break;
    default: break;
  }
}

static void app__scene_process(float delta) 
{
  app_t *app = app_ptr();
  switch (app->scene) {
    case SCENE_MAIN: 
      main_scene_process(app->scene_state, delta);
    break;
    default: break;
  }
}

static void app__scene_draw() 
{
  app_t *app = app_ptr();
  switch (app->scene) {
    case SCENE_MAIN: 
      main_scene_draw(app->scene_state);
    break;
    default: break;
  }
}

static void app__scene_free() 
{
  app_t *app = app_ptr();
  switch (app->scene) {
    case SCENE_MAIN: 
      main_scene_free(app->scene_state);
    break;
    default: break;
  }
}

API void app__scene_sync(scene_type_t scene, sync_signal_type_t signal)
{
  switch (scene) {
    case SCENE_MAIN:
      main_scene_sync(app_ptr()->scene_state, signal);
    break;
    default: break;
  }
}

API void app_emit_hot_sync()
{
  app__scene_sync(app_ptr()->scene, SYNC_SIGNAL_HOT_SYNC);
}

API void app_process(float delta)
{
  assert(g_app && "app not initialized");
  app_t *app = app_ptr();
  if (app->state == APP_EXITED) return;

  app->screen_size = (screen_size_t) {
    .x = GetScreenWidth(),
    .y = GetScreenHeight(),
  };

  if (app->scene_next) {
    if (app->scene && app->scene_transition != SCENE_TRANSITION_EXITING) {
      app__scene_sync(app->scene_next, SYNC_SIGNAL_ON_EXIT);
      app->scene_transition = SCENE_TRANSITION_EXITING;
    }

    // exiting phase: current scene
    if (app->scene_transition == SCENE_TRANSITION_EXITING && !app__scene_exiting()) {
      return;
    }

    if (app->scene) {
      app__scene_free();
      app->scene = SCENE_NONE;
    }

    app->scene_transition = SCENE_TRANSITION_ENTERING;
    app->scene = app->scene_next;
    app->scene_next = SCENE_NONE;
    arena_reset(app->scene_arena);
    app__scene_sync(app->scene, SYNC_SIGNAL_ON_ENTER);
    app__scene_init();
  }

  tween_process(delta);
  timer_process(delta);

  if (app->state == APP_EXITING) {
    if (app->scene_transition != SCENE_TRANSITION_EXITING) {
      app->scene_transition = SCENE_TRANSITION_EXITING;
      app__scene_sync(app->scene, SYNC_SIGNAL_ON_EXIT);
    }
    if (app__scene_exiting()) {
      app__scene_free();
      app->state = APP_EXITED;
      app->scene = SCENE_NONE;
      app->scene_transition = SCENE_TRANSITION_NONE;
    }
    return;
  }

  if (app->scene_transition == SCENE_TRANSITION_ENTERING && app__scene_entering()) {
    app->scene_transition = SCENE_TRANSITION_NONE;
    return;
  }

  if (IsWindowResized()) {
    app__scene_sync(app->scene, SYNC_SIGNAL_WINDOW_RESIZED);
  }

  app__scene_process(delta);
}

API void app_draw()
{
  app__scene_draw();
}

