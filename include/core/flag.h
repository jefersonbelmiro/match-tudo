#pragma once

#include "core/defs.h"

API bool flag_has(u32 flags, u32 flag)
{
  return (flags & flag) != 0;
}

API void flag_set(u32 *flags, u32 flag)
{
  *flags |= flag;
}

API void flag_clear(u32 *flags, u32 flag)
{
  *flags &= ~flag;
}

API void flag_toggle(u32 *flags, u32 flag)
{
  *flags ^= flag;
}

API void flag_set_to(u32 *flags, u32 flag, bool value)
{
  if (value) {
    *flags |= flag;
  } else {
    *flags &= ~flag;
  }
}

API bool flag_has_all(u32 flags, u32 mask)
{
  return (flags & mask) == mask;
}

API bool flag_has_any(u32 flags, u32 mask)
{
  return (flags & mask) != 0;
}

API bool flag_has_none(u32 flags, u32 mask)
{
  return (flags & mask) == 0;
}
