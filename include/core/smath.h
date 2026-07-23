#pragma once

#include "core/defs.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

API float clampf(float x, float lo, float hi) { return x < lo ? lo : x > hi ? hi : x; }
API float lerpf(float a, float b, float t)    { return a + (b - a) * t; }

API int32_t m_floor(float x) 
{
  int32_t i = (int32_t)x;
  return i - (i > x);
}

API uint32_t m_next_pow2(uint32_t x)
{
  if (x == 0) return 1;
  x--;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  return x + 1;
}

API uint32_t m_log2_pow2(uint32_t x)
{
  // x must be a power of 2
  uint32_t n = 0;
  while (x >>= 1) n++;
  return n;
}

API float m_absf(float x) 
{
  return x < 0.0f ? -x : x;
}

API u32 m_abs32(int x) 
{
  return x < 0 ? -x : x;
}

API float m_step(float value, float step)
{
  return roundf(value / step) * step;
}

API u32 m_rand32(u32 lo, u32 hi)
{
  return lo + (rand() % (hi - lo + 1)); 
}

API float m_randf() 
{
  return (float)rand() / (float)RAND_MAX;
}

