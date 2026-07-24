#pragma once

#include "hot_code.h"

void hot_code_setup(void)
{
  hot_code_ignore(
    "include/hot/",
    "include/core/",
    // "include/core/arena_debug.h",
    // "include/core/app.h",
    // "include/core/style.h",
    "include/game/game.h",
  );

  hot_code_register_all();
}
