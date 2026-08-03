#pragma once

#include "core/arena.h"
#include "core/defs.h"
#include "core/draw.h"
#include "core/flag.h"
#include "core/input.h"
#include "core/resources.h"
#include "core/signal.h"
#include "core/sound.h"
#include "core/style.h"
#include "raylib.h"

typedef struct {
  char  *text;
  Font  *font;      // NULL = GetFontDefault()
  float font_size;  // 0 = use default font size
  u16   text_len;
} input_node_label_t;

API void input_label_set(input_node_label_t *l, const char *str)
{
  size_t n = strlen(str);
  if (n > l->text_len) n = l->text_len;
  mem_copy((void *)str, l->text, n);
  l->text[n] = '\0';
}

typedef struct {
  input_node_label_t label;
  Vector2            position;
  Vector2            size;
  float              alpha;
  ui_flag_t          flags;
  signal_t           on_changed; 
  input_layer_t      input_layer; // set   : apply this layer
  input_layer_t      input_mask;  // filter: process ignore layer
  u16                cursor;
} input_node_t;

void input_node_init(input_node_t *s, u16 label_len, arena_t *arena)
{
  *s = (input_node_t){
    .alpha = 1.0f,
    .label = {
      .text_len = label_len,
      .text = arena_push(arena, char, label_len),
      .font_size = UI_FONT_SIZE_SM
    },
    .flags = 0,
    .cursor = 0,
  };
  s->label.text[0] = '\0';
}

API bool input_node_is_hover(const input_node_t *s)    { return flag_has(s->flags, UI_STATE_HOVER);    }
API bool input_node_is_pressed(const input_node_t *s)  { return flag_has(s->flags, UI_STATE_PRESSED);  }
API bool input_node_is_focused(const input_node_t *s)  { return flag_has(s->flags, UI_STATE_FOCUS);    }
API bool input_node_is_active(const input_node_t *s)   { return flag_has(s->flags, UI_STATE_ACTIVE);   }
API bool input_node_is_disabled(const input_node_t *s) { return flag_has(s->flags, UI_STATE_DISABLED); }

API void input_node_on_changed(input_node_t *s, signal_fn fn, void *listener)
{
  signal_connect(&s->on_changed, fn, listener);
}

API void input_node_set_disabled(input_node_t *s, bool disabled)
{
  flag_set_to(&s->flags, UI_STATE_DISABLED, disabled);
  if (disabled)
    flag_clear(&s->flags, UI_STATE_HOVER | UI_STATE_PRESSED | UI_STATE_ACTIVE);
}

API void input_node_process(input_node_t *s)
{
  if (input_node_is_disabled(s)) return;

  flag_clear(&s->flags, UI_STATE_ACTIVE);

  if (s->input_mask && input_layer_handled(s->input_mask)) {
    flag_clear(&s->flags, UI_STATE_HOVER);
    return;
  }

  bool was_hover = input_node_is_hover(s);
  Rectangle rect = { s->position.x, s->position.y, s->size.x, s->size.y };
  flag_set_to(&s->flags, UI_STATE_HOVER, CheckCollisionPointRec(input_ptr()->mouse_position, rect));
  if (!was_hover && input_node_is_hover(s)) {
    sound_play(RESOURCE_SOUND_HOVER_01);
  }

  if (s->input_layer && input_node_is_hover(s)) {
    input_layer_set(s->input_layer);
  }

  if (!input_node_is_pressed(s) && input_node_is_hover(s) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    sound_play(RESOURCE_SOUND_CLICK);
    flag_set(&s->flags, UI_STATE_PRESSED);
  }

  if (input_node_is_pressed(s) && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
    // if (input_node_is_hover(s)) {
    //   flag_set(&s->flags, UI_STATE_ACTIVE);
    //   signal_emit(&s->on_changed, 0, NULL);
    // }
    flag_clear(&s->flags, UI_STATE_PRESSED);
  } else if (input_node_is_focused(s) && (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))) {
    sound_play(RESOURCE_SOUND_CLICK);
    signal_emit(&s->on_changed, 0, NULL);
  }
}

API void input_node_draw(const input_node_t *s)
{
  float a = s->alpha;
  if (input_node_is_disabled(s)) a *= UI_ALPHA_DISABLED;

  Color fg = UI_COLOR_TEXT; 
  if (!input_node_is_disabled(s) && input_node_is_hover(s))
    fg = UI_COLOR_HOVER;

  Color bg = (!input_node_is_disabled(s) && input_node_is_hover(s))
    ? UI_COLOR_WIDGET_FG
    : UI_COLOR_WIDGET_BG;

  if (!input_node_is_disabled(s) && input_node_is_pressed(s)) {
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
  Vector2 text_pos  = {
    s->position.x + 10.0f,
    s->position.y + (s->size.y * 0.5f) - (text_size.y * 0.5f),
  };

  if (font.texture.id == resource_font(RESOURCE_FONT_MONOGRAM_32).texture.id) {
    text_pos.y -= 2.0f;
  }

  DrawTextEx(font, s->label.text, text_pos, font_size, 1, fg);
}
