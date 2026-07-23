#pragma once

#include "hot_code.h"
#include "hot_code_setup.h"
// #include "hot_resouce.h"

static inline
void hot_init(void)
{
  hot_code_setup();
  // hot_resource_init();
}

static inline
void hot_process(float delta)
{
  if (IsKeyPressed(KEY_F5)) {
    hot_code_reload_all();
  }

#if HOT_RELOAD_UPDATE_ON_SAVE
  hot_code_process(delta);
#endif

  // hot_resource_process(delta);
}
