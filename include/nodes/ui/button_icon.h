#pragma once

#include "core/defs.h"
#include "core/draw.h"
#include "core/flag.h"
#include "core/resources.h"
#include "core/signal.h"
#include "core/sound.h"
#include "core/style.h"
#include "raylib.h"

#define BUTTON_ICON_SIZE_SM   24.0f
#define BUTTON_ICON_SIZE_MD   36.0f
#define BUTTON_ICON_SIZE_LG   48.0f

#define BUTTON_ICON_SCALE_SM  0.50f
#define BUTTON_ICON_SCALE_MD  1.00f
#define BUTTON_ICON_SCALE_LG  1.00f

typedef struct {
  u32       atlas_idx;
  Vector2   position;
  Vector2   size;
  float     scale;
  float     alpha;
  ui_flag_t flags;
  signal_t  on_pressed;
} button_icon_t;

API bool button_icon_is_hover(const button_icon_t *s)    { return flag_has(s->flags, UI_STATE_HOVER);    }
API bool button_icon_is_pressed(const button_icon_t *s)  { return flag_has(s->flags, UI_STATE_PRESSED);  }
API bool button_icon_is_active(const button_icon_t *s)   { return flag_has(s->flags, UI_STATE_ACTIVE);   }
API bool button_icon_is_disabled(const button_icon_t *s) { return flag_has(s->flags, UI_STATE_DISABLED); }

API void button_icon_set_disabled(button_icon_t *s, bool disabled)
{
  flag_set_to(&s->flags, UI_STATE_DISABLED, disabled);
  if (disabled)
    flag_clear(&s->flags, UI_STATE_HOVER | UI_STATE_PRESSED | UI_STATE_ACTIVE);
}

API void button_icon_init(button_icon_t *s, u32 atlas_idx, float size, float scale)
{
  *s = (button_icon_t){
    .atlas_idx = atlas_idx,
    .size      = (Vector2){ size, size },
    .scale     = scale,
    .alpha     = 1.0f,
  };
}

API void button_icon_process(button_icon_t *s)
{
  if (button_icon_is_disabled(s)) return;

  bool was_hover = button_icon_is_hover(s);

  Rectangle rect = { s->position.x, s->position.y, s->size.x, s->size.y };
  flag_set_to(&s->flags, UI_STATE_HOVER, CheckCollisionPointRec(GetMousePosition(), rect));
  flag_clear(&s->flags, UI_STATE_ACTIVE);

  if (!was_hover && button_icon_is_hover(s)) {
    sound_play(RESOURCE_SOUND_HOVER_01);
  }

  if (!button_icon_is_pressed(s) && button_icon_is_hover(s) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    sound_play(RESOURCE_SOUND_CLICK);
    flag_set(&s->flags, UI_STATE_PRESSED);
  }

  if (button_icon_is_pressed(s) && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
    if (button_icon_is_hover(s)) {
      flag_set(&s->flags, UI_STATE_ACTIVE);
      signal_emit(&s->on_pressed, 0, NULL);
    }
    flag_clear(&s->flags, UI_STATE_PRESSED);
  }
}

API void button_icon_draw(button_icon_t *s)
{
  float a = s->alpha;
  if (button_icon_is_disabled(s)) a *= UI_ALPHA_DISABLED;

  Color tint = (Color){ 200, 200, 200, (u8)(a * 255.0f) };
  if (!button_icon_is_disabled(s) && button_icon_is_hover(s))
    tint = ColorAlpha(UI_COLOR_HOVER, a);

  Color bg = (!button_icon_is_disabled(s) && button_icon_is_hover(s))
    ? ColorAlpha(UI_COLOR_WIDGET_FG, a)
    : ColorAlpha(UI_COLOR_WIDGET_BG, a);

  if (!button_icon_is_disabled(s) && button_icon_is_pressed(s)) {
    bg   = ColorBrightness(bg,   0.2f);
    tint = ColorBrightness(tint, 0.2f);
  }

  draw_npatch(RESOURCE_TEXTURE_NPATCH_32X32_3, s->position.x, s->position.y, s->size.x, s->size.y, 1.0f, bg);

  atlas_t *atlas  = resource_atlas_ptr(RESOURCE_ATLAS_1_64);
  Vector2  center = {
    s->position.x + s->size.x * 0.5f,
    s->position.y + s->size.y * 0.5f,
  };
  draw_atlas(atlas, s->atlas_idx, center, s->scale, 0.0f, tint);
}
