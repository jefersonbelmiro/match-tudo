#pragma once

#include "core/arena.h"
#include "core/defs.h"
#include "raylib.h"

#define ACTION_UP                 (1u <<  0)
#define ACTION_DOWN               (1u <<  1)
#define ACTION_LEFT               (1u <<  2)
#define ACTION_RIGHT              (1u <<  3)
#define ACTION_CONFIRM            (1u <<  4)
#define ACTION_CANCEL             (1u <<  5)
#define ACTION_MAP_SELECT_PRESSED (1u <<  6)
#define ACTION_MAP_TARGET_PRESSED (1u <<  7)
#define ACTION_SHIFT              (1u <<  8)
#define ACTION_ALT                (1u <<  9)

typedef struct input_t input_t;

struct input_t {
  Vector2 mouse_position;
  u32 action_curr;
  u32 action_prev;
  float scroll;
  input_layer_t layer;
};

GLOBAL input_t *g_input;

API input_t *input_ptr(void) 
{ 
  return g_input;
}

API u32 input_memory_size(void)
{ 
  u32 size = 0;
  size += sizeof(input_t); 
  return size;
}

API void input_init(arena_t *arena)
{
  g_input = arena_push_zero(arena, input_t, 1);
}

API void input_action_down(u32 action)
{
  g_input->action_curr |= action;
}

API void input_action_up(u32 action)
{
  g_input->action_curr &= ~action;
}

API bool input_action_pressed(u32 action)
{
  return (g_input->action_curr & action) != 0;
}

API bool input_action_just_pressed(u32 action)
{
  return (g_input->action_curr & ~g_input->action_prev & action) != 0;
}

API bool input_action_just_released(u32 action)
{
  return (~g_input->action_curr & g_input->action_prev & action) != 0;
}

API bool input_layer_handled(input_layer_t mask)
{
  return (input_ptr()->layer & mask) != 0;
}

API bool input_layer_any_hud_handled()
{
  input_layer_t mask = INPUT_LAYER_HUD | INPUT_LAYER_HUD_FG;
  return (input_ptr()->layer & mask) != 0;
}

API void input_layer_set(input_layer_t mask)
{
  input_ptr()->layer |= mask;
}

API void input_process()
{
  input_t *input = input_ptr();

  input->layer = INPUT_LAYER_NONE;

  input->mouse_position = GetMousePosition();

  input->action_prev = input->action_curr;

  input->scroll = GetMouseWheelMove();

  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
    input_action_down(ACTION_MAP_SELECT_PRESSED);
  }
  if (IsMouseButtonUp(MOUSE_BUTTON_LEFT)) {
    input_action_up(ACTION_MAP_SELECT_PRESSED);
  }
  if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
    input_action_down(ACTION_MAP_TARGET_PRESSED);
  }
  if (IsMouseButtonUp(MOUSE_BUTTON_RIGHT)) {
    input_action_up(ACTION_MAP_TARGET_PRESSED);
  }

  if (IsKeyPressed(KEY_LEFT_SHIFT) || IsKeyPressed(KEY_RIGHT_SHIFT)) {
    input_action_down(ACTION_SHIFT);
  }
  if (IsKeyReleased(KEY_LEFT_SHIFT) || IsKeyReleased(KEY_RIGHT_SHIFT)) {
    input_action_up(ACTION_SHIFT);
  }
  if (IsKeyPressed(KEY_LEFT_ALT) || IsKeyPressed(KEY_RIGHT_ALT)) {
    input_action_down(ACTION_ALT);
  }
  if (IsKeyReleased(KEY_LEFT_ALT) || IsKeyReleased(KEY_RIGHT_ALT)) {
    input_action_up(ACTION_ALT);
  }
}
