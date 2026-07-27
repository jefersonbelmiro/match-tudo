#include <raylib.h>
#include <stdint.h>
#include <stdio.h>
#include "core/defs.h"
#if HOT_RELOAD
#include "hot/reload.h"
#endif
#include "core/app_op.h"

int main()
{
  printf("main()\n");

  SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_TOPMOST | FLAG_WINDOW_RESIZABLE);
  SetTraceLogLevel(LOG_WARNING);
  InitWindow(800, 600, "match-puzzle");

#if HOT_RELOAD
  hot_init();
#endif

  app_init();
  app_start();
  app_set_scene(SCENE_MAIN);

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

  app_fini();
  CloseWindow();

  return 0;
}
