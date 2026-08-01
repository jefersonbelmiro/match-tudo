#pragma once

#include "core/defs.h"
#include "core/draw.h"
#include "core/flag.h"
#include "core/input.h"
#include "core/resources.h"
#include "core/signal.h"
#include "core/sound.h"
#include "core/style.h"
#include "core/mem.h"
#include "raylib.h"

// ---------------------------------------------------------------------------
// button_label_t
// ---------------------------------------------------------------------------

typedef struct {
  char  text[16];   // inline buffer — update with button_label_set()
  Font  *font;       // zero-init = GetFontDefault()
  float font_size;  // 0 = use default font size. animation (ui_effect.h) tweens this.
  u8    align;      // 0 = CENTER (default/zero-init), 1 = LEFT
} button_label_t;

API void button_label_set(button_label_t *l, const char *str)
{
  size_t n = strlen(str);
  if (n > 15) n = 15;
  mem_copy((void *)str, l->text, n);
  l->text[n] = '\0';
}

// ---------------------------------------------------------------------------
// button_t
// ---------------------------------------------------------------------------

typedef struct {
  button_label_t label;
  Vector2        position;
  Vector2        size;
  float          alpha;
  ui_flag_t      flags;
  signal_t       on_pressed;
  input_layer_t  input_layer; // set   : apply this layer
  input_layer_t  input_mask;  // filter: process ignore layer
} button_t;

void button_init(button_t *s)
{
  *s = (button_t){
    .label      = { .align = 1, .font_size = UI_FONT_SIZE_SM },
    .alpha      = 1.0f,
  };
}

API bool button_is_hover(const button_t *s)    { return flag_has(s->flags, UI_STATE_HOVER);    }
API bool button_is_pressed(const button_t *s)  { return flag_has(s->flags, UI_STATE_PRESSED);  }
API bool button_is_focused(const button_t *s)  { return flag_has(s->flags, UI_STATE_FOCUS);    }
API bool button_is_active(const button_t *s)   { return flag_has(s->flags, UI_STATE_ACTIVE);   }
API bool button_is_disabled(const button_t *s) { return flag_has(s->flags, UI_STATE_DISABLED); }

API void button_on_pressed(button_t *s, signal_fn fn, void *listener)
{
  signal_connect(&s->on_pressed, fn, listener);
}

API void button_set_disabled(button_t *s, bool disabled)
{
  flag_set_to(&s->flags, UI_STATE_DISABLED, disabled);
  if (disabled)
    flag_clear(&s->flags, UI_STATE_HOVER | UI_STATE_PRESSED | UI_STATE_ACTIVE);
}

API void button_process(button_t *s)
{
  if (button_is_disabled(s)) return;

  flag_clear(&s->flags, UI_STATE_ACTIVE);

  if (s->input_mask && input_layer_handled(s->input_mask)) {
    flag_clear(&s->flags, UI_STATE_HOVER);
    return;
  }

  bool was_hover = button_is_hover(s);
  Rectangle rect = { s->position.x, s->position.y, s->size.x, s->size.y };
  flag_set_to(&s->flags, UI_STATE_HOVER, CheckCollisionPointRec(input_ptr()->mouse_position, rect));
  if (!was_hover && button_is_hover(s)) {
    sound_play(RESOURCE_SOUND_HOVER_01);
  }

  if (s->input_layer && button_is_hover(s)) {
    input_layer_set(s->input_layer);
  }

  if (!button_is_pressed(s) && button_is_hover(s) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    sound_play(RESOURCE_SOUND_CLICK);
    flag_set(&s->flags, UI_STATE_PRESSED);
  }

  if (button_is_pressed(s) && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
    if (button_is_hover(s)) {
      flag_set(&s->flags, UI_STATE_ACTIVE);
      signal_emit(&s->on_pressed, 0, NULL);
    }
    flag_clear(&s->flags, UI_STATE_PRESSED);
  } else if (button_is_focused(s) && (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))) {
    sound_play(RESOURCE_SOUND_CLICK);
    signal_emit(&s->on_pressed, 0, NULL);
  }
}

API void button_draw(const button_t *s)
{
  float a = s->alpha;
  if (button_is_disabled(s)) a *= UI_ALPHA_DISABLED;

  Color fg = UI_COLOR_TEXT; 
  if (!button_is_disabled(s) && button_is_hover(s))
    fg = UI_COLOR_HOVER;

  Color bg = (!button_is_disabled(s) && button_is_hover(s))
    ? UI_COLOR_WIDGET_FG
    : UI_COLOR_WIDGET_BG;

  if (!button_is_disabled(s) && button_is_pressed(s)) {
    bg = ColorBrightness(bg, 0.2f);
    fg = ColorBrightness(fg, 0.2f);
  }

  // sync button alpha
  fg.a *= a;
  bg.a *= a;

  draw_npatch(RESOURCE_TEXTURE_NPATCH_32X32_3, s->position.x, s->position.y, s->size.x, s->size.y, 1.0f, bg);

  if (!s->label.text[0]) return;

  float font_size = s->label.font_size > 0.0f ? s->label.font_size : UI_FONT_SIZE_SM;
  Font  font     = s->label.font ? *s->label.font : GetFontDefault();
  Vector2 text_size = MeasureTextEx(font, s->label.text, font_size, 1);
  float   text_x    = (s->label.align == 1)
    ? s->position.x + 10.0f
    : s->position.x + s->size.x * 0.5f - text_size.x * 0.5f;
  Vector2 text_pos  = {
    text_x,
    s->position.y + (s->size.y * 0.5f) - (text_size.y * 0.5f),
  };

  if (font.texture.id == resource_font(RESOURCE_FONT_MONOGRAM_32).texture.id) {
    text_pos.y -= 2.0f;
  }

  DrawTextEx(font, s->label.text, text_pos, font_size, 1, fg);
}
