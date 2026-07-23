#pragma once

// Easing functions - input t in [0,1], output in [0,1]
// Based on Robert Penner's easing equations
// Reference: https://easings.net

#include "core/defs.h"
#include <math.h>

typedef float (*ease_fn_t)(float t);

// ---------------------------------------------------------------------------
// Linear
// ---------------------------------------------------------------------------

API float ease_linear(float t) { return t; }

// ---------------------------------------------------------------------------
// Quad
// ---------------------------------------------------------------------------

API float ease_in_quad(float t)     { return t * t; }
API float ease_out_quad(float t)    { return t * (2.0f - t); }
API float ease_in_out_quad(float t) {
  return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
}

// ---------------------------------------------------------------------------
// Cubic
// ---------------------------------------------------------------------------

API float ease_in_cubic(float t)     { return t * t * t; }
API float ease_out_cubic(float t)    { float u = 1.0f - t; return 1.0f - u * u * u; }
API float ease_in_out_cubic(float t) {
  return t < 0.5f ? 4.0f * t * t * t : 1.0f - (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) / 2.0f;
}

// ---------------------------------------------------------------------------
// Quart
// ---------------------------------------------------------------------------

API float ease_in_quart(float t)     { return t * t * t * t; }
API float ease_out_quart(float t)    { float u = 1.0f - t; return 1.0f - u * u * u * u; }
API float ease_in_out_quart(float t) {
  float u = -2.0f * t + 2.0f;
  return t < 0.5f ? 8.0f * t * t * t * t : 1.0f - u * u * u * u / 2.0f;
}

// ---------------------------------------------------------------------------
// Sine
// ---------------------------------------------------------------------------

API float ease_in_sine(float t)     { return 1.0f - cosf(t * (3.14159265f / 2.0f)); }
API float ease_out_sine(float t)    { return sinf(t * (3.14159265f / 2.0f)); }
API float ease_in_out_sine(float t) { return -(cosf(3.14159265f * t) - 1.0f) / 2.0f; }

// ---------------------------------------------------------------------------
// Expo
// ---------------------------------------------------------------------------

API float ease_in_expo(float t)  { return t == 0.0f ? 0.0f : powf(2.0f, 10.0f * t - 10.0f); }
API float ease_out_expo(float t) { return t == 1.0f ? 1.0f : 1.0f - powf(2.0f, -10.0f * t); }
API float ease_in_out_expo(float t) {
  if (t == 0.0f) return 0.0f;
  if (t == 1.0f) return 1.0f;
  return t < 0.5f
    ? powf(2.0f,  20.0f * t - 10.0f) / 2.0f
    : (2.0f - powf(2.0f, -20.0f * t + 10.0f)) / 2.0f;
}

// ---------------------------------------------------------------------------
// Back (slight overshoot)
// ---------------------------------------------------------------------------

API float ease_in_back(float t) {
  float c = 1.70158f;
  return (c + 1.0f) * t * t * t - c * t * t;
}
API float ease_out_back(float t) {
  float c = 1.70158f;
  float u = t - 1.0f;
  return 1.0f + (c + 1.0f) * u * u * u + c * u * u;
}
API float ease_in_out_back(float t) {
  float c = 1.70158f * 1.525f;
  return t < 0.5f
    ? (powf(2.0f * t, 2.0f) * ((c + 1.0f) * 2.0f * t - c)) / 2.0f
    : (powf(2.0f * t - 2.0f, 2.0f) * ((c + 1.0f) * (2.0f * t - 2.0f) + c) + 2.0f) / 2.0f;
}

// ---------------------------------------------------------------------------
// Elastic
// ---------------------------------------------------------------------------

API float ease_out_elastic(float t) {
  if (t == 0.0f) return 0.0f;
  if (t == 1.0f) return 1.0f;
  float c = (2.0f * 3.14159265f) / 3.0f;
  return powf(2.0f, -10.0f * t) * sinf((t * 10.0f - 0.75f) * c) + 1.0f;
}
API float ease_in_elastic(float t) {
  if (t == 0.0f) return 0.0f;
  if (t == 1.0f) return 1.0f;
  float c = (2.0f * 3.14159265f) / 3.0f;
  return -powf(2.0f, 10.0f * t - 10.0f) * sinf((t * 10.0f - 10.75f) * c);
}
API float ease_in_out_elastic(float t) {
  if (t == 0.0f) return 0.0f;
  if (t == 1.0f) return 1.0f;
  float c = (2.0f * 3.14159265f) / 4.5f;
  return t < 0.5f
    ? -(powf(2.0f,  20.0f * t - 10.0f) * sinf((20.0f * t - 11.125f) * c)) / 2.0f
    :  (powf(2.0f, -20.0f * t + 10.0f) * sinf((20.0f * t - 11.125f) * c)) / 2.0f + 1.0f;
}

// ---------------------------------------------------------------------------
// Bounce
// ---------------------------------------------------------------------------

API float ease_out_bounce(float t) {
  float n = 7.5625f, d = 2.75f;
  if (t < 1.0f / d)       return n * t * t;
  if (t < 2.0f / d)       { t -= 1.5f   / d; return n * t * t + 0.75f; }
  if (t < 2.5f / d)       { t -= 2.25f  / d; return n * t * t + 0.9375f; }
                           { t -= 2.625f / d; return n * t * t + 0.984375f; }
}
API float ease_in_bounce(float t)     { return 1.0f - ease_out_bounce(1.0f - t); }
API float ease_in_out_bounce(float t) {
  return t < 0.5f
    ? (1.0f - ease_out_bounce(1.0f - 2.0f * t)) / 2.0f
    : (1.0f + ease_out_bounce(2.0f * t - 1.0f)) / 2.0f;
}
