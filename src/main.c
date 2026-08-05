#include <raylib.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "core/defs.h"
#if HOT_RELOAD
#include "hot/reload.h"
#endif
#include "core/app_op.h"

int main()
{
  srand(time(NULL));

  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_TOPMOST | FLAG_WINDOW_RESIZABLE);
  SetTraceLogLevel(LOG_WARNING);
  InitWindow(800, 600, "match-puzzle");
  SetExitKey(KEY_NULL);

#if HOT_RELOAD
  hot_init();
#endif

  app_init();
  app_start();
  app_set_scene(SCENE_MENU);

  app_t *app = app_ptr();
  while (app->state != APP_EXITED) {
    if (WindowShouldClose()) app_quit();

#if HOT_RELOAD
    hot_process(GetFrameTime());
#endif

    app_process(GetFrameTime());

    // fullscreen toggle
    if (IsKeyPressed(KEY_F11)) {
      if (!IsWindowFullscreen()) {
        int monitor = GetCurrentMonitor();
        SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
        ToggleFullscreen();
      } else {
        ToggleFullscreen();
        SetWindowSize(APP_WINDOW_WIDTH, APP_WINDOW_HEIGHT);
      }
    }

    BeginDrawing();
    ClearBackground(BLACK);

    app_draw();

    EndDrawing();
  }

#if DEBUG_MEMORY_USAGE
  arena_print_stats(app->arena->debug_id);
  // arena_print_track(app->arena->debug_id, false);
#endif
  app_fini();
#if DEBUG_MEMORY_USAGE
  mem_print_stats();
#endif
  CloseWindow();

  return 0;
}
