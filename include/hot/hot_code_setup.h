#pragma once

#include "hot_code.h"

void hot_code_setup(void)
{
  hot_code_ignore(
    "include/hot/",
    //"include/core/",
  );

  hot_code_register_all();
}
