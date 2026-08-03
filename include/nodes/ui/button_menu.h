#pragma once

#include "core/defs.h"
#include "core/ease.h"
#include "core/focus_list.h"
#include "core/resources.h"
#include "core/sound.h"
#include "core/style.h"
#include "core/tween.h"
#include "nodes/ui/button.h"
#include <raylib.h>

typedef struct {
  float btn_w;
  float btn_h;
  float gap;
} button_menu_cfg_t;

typedef struct {
  const char *label;   // assigned to button.label.text
  signal_t    on_pressed;
} button_menu_def_t;

#define BUTTON_MENU_MAX 4

typedef struct {
  button_t           buttons[BUTTON_MENU_MAX];
  tween_h            in_out_tween[BUTTON_MENU_MAX];
  button_menu_cfg_t  cfg;
  float              start_delay;
  float              focus_blink;
  process_mode_t     process_mode;
  tween_h            focus_blink_tween;
  focus_list_t       focus;
  u8                 btn_count;
} button_menu_t;

// ---------------------------------------------------------------------------
// menu_layout — position buttons vertically centered on (anchor_x, anchor_y).
// anchor is the center-bottom of the whole list.
// ---------------------------------------------------------------------------
API void button_menu_layout(button_menu_t *s, float anchor_x, float anchor_y)
{
  float btn_w  = s->cfg.btn_w;
  float btn_h  = s->cfg.btn_h;
  float gap    = s->cfg.gap;
  float total_h = btn_h * s->btn_count + gap * (s->btn_count - 1);
  float x = anchor_x - btn_w * 0.5f;
  float y = anchor_y - total_h * 0.5;

  for (u8 i = 0; i < s->btn_count; i++) {
    s->buttons[i].position = (Vector2){ x, y + i * (btn_h + gap) };
    s->buttons[i].size     = (Vector2){ btn_w, btn_h };
  }
}

API void button_menu_animate_in(button_menu_t *s, float duration)
{  
  tween_kill(s->focus_blink_tween);
  s->focus_blink_tween = TWEEN_NONE;
  s->focus_blink = 0.0f;
  s->start_delay = 1.0f;

  for (u8 i = 0; i < s->btn_count; i++) {
    tween_kill(s->in_out_tween[i]);

    button_t *btn = &s->buttons[i];
    float x_end = btn->position.x;
    btn->position.x -= 40.0f;
    btn->alpha = 0.0f;

    tween_h t = tween_create_parallel();
    tween_process_mode(t, s->process_mode);
    tween_add(t, &btn->position.x, x_end, 0.35f * duration, ease_out_quad);
    tween_add(t, &btn->alpha,      1.0f,          duration, ease_out_quad);
    s->in_out_tween[i] = t;
  }
}

API void button_menu_animate_out(button_menu_t *s, float duration)
{
  for (u8 i = 0; i < s->btn_count; i++) {
    tween_kill(s->in_out_tween[i]);

    button_t *btn = &s->buttons[i];
    float x_end = btn->position.x - 40;
    tween_h t = tween_create_parallel();
    tween_process_mode(t, s->process_mode);
    tween_add(t, &btn->position.x, x_end, duration, ease_in_quad);
    tween_add(t, &btn->alpha,      0.0f,  duration, ease_in_quad);
    s->in_out_tween[i] = t;
  }
  tween_kill(s->focus_blink_tween);
  s->focus_blink = 0.0f;
}

API void button_menu_init(button_menu_t *s, const button_menu_def_t *defs, u8 count, button_menu_cfg_t cfg)
{
  assert(count <= BUTTON_MENU_MAX && "[button_menu_init]: count exceeds BUTTON_MENU_MAX");
  s->btn_count = count;
  s->cfg       = cfg;
  s->focus_blink = 0.0f;
  s->start_delay = 1.0f;
  s->focus_blink_tween = TWEEN_NONE;
  s->process_mode = PROCESS_MODE_ALWAYS;
  for (u8 i = 0; i < count; i++) {
    s->buttons[i] = (button_t){
      .label      = { .align = 1, .font_size = 36.0f },
      .alpha      = 1.0f,
      .on_pressed = defs[i].on_pressed,
    };
    s->buttons[i].label.font = resource_font_ptr(RESOURCE_FONT_MONOGRAM_32);
    s->in_out_tween[i] = TWEEN_NONE;
    button_label_set(&s->buttons[i].label, defs[i].label);
  }
  focus_list_init(&s->focus, count);
}

API void button_menu_process(button_menu_t *s, float delta)
{
  if (s->start_delay > 0) {
    s->start_delay -= delta;
    if (s->start_delay <= 0) {
      s->focus_blink = 1.0;
      tween_h blink_tween = tween_create();
      tween_process_mode(blink_tween, s->process_mode);
      tween_set_loop(blink_tween, true);
      tween_add(blink_tween, &s->focus_blink, 0.0, 0.3, ease_linear);
      tween_set_delay(blink_tween, 0, 0.5);
      tween_add(blink_tween, &s->focus_blink, 1.0, 0.2, ease_linear);
      s->focus_blink_tween = blink_tween;
    }
  }

  u8 current_focus_index = s->focus.index;
  focus_list_process(&s->focus);
  for (u8 i = 0; i < s->btn_count; i++) {
    flag_set_to(&s->buttons[i].flags, UI_STATE_FOCUS, i == s->focus.index);
    button_process(&s->buttons[i]);
  }

  if (current_focus_index != s->focus.index) {
    sound_play(RESOURCE_SOUND_CLICK);
    tween_restart(s->focus_blink_tween);
    s->focus_blink = 1.0f;
  }
}

API void button_menu_draw(button_menu_t *s)
{
  // Draw focus cursor to the left of the focused button
  button_t *foc = &s->buttons[s->focus.index];

  DrawRectangleV((Vector2){ foc->position.x - 6, foc->position.y + 6 }, (Vector2) {4, s->cfg.btn_h - 6}, ColorAlpha(UI_COLOR_WIDGET_FG, foc->alpha * s->focus_blink));

  // u8 a = (u8)(foc->alpha * 255.0f);
  // atlas_t *atlas = resource_atlas_ptr(RESOURCE_ATLAS_3_16);
  //
  // draw_atlas(
  //   atlas, 0,
  //   (Vector2){ foc->position.x - 8, foc->position.y + s->cfg.btn_h * 0.5f },
  //   2.5f, 0,
  //   (Color){ 255, 255, 255, a }
  // );

  for (u8 i = 0; i < s->btn_count; i++) {
    button_draw(&s->buttons[i]);
  }
}
