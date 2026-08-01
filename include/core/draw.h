#pragma once

#include "core/resources.h"
#include "core/defs.h"
#include "raylib.h"
#include "raymath.h"

API void draw_atlas(atlas_t *atlas, u32 idx, Vector2 pos, float scale,
                              float rotation, Color tint) 
{
  u32 cols = atlas->texture.width / atlas->cell_size.x;
  u32 col  = idx % cols;
  u32 row  = idx / cols;

  Rectangle source = {
    (float)(col * atlas->cell_size.x),
    (float)(row * atlas->cell_size.y),
    (float)atlas->cell_size.x,
    (float)atlas->cell_size.y,
  };
  float w = atlas->cell_size.x * scale;
  float h = atlas->cell_size.y * scale;
  Rectangle dest = { pos.x, pos.y, w, h };
  Vector2 origin = { w * 0.5f, h * 0.5f };
  DrawTexturePro(atlas->texture, source, dest, origin, rotation, tint);
}

API void draw_atlas_fliph(atlas_t *atlas, u32 idx, Vector2 pos, float scale,
                              float rotation, Color tint) 
{
  u32 cols = atlas->texture.width / atlas->cell_size.x;
  u32 col  = idx % cols;
  u32 row  = idx / cols;

  // printn(" - idx: %d row: %d col: %d", idx, row, col);

  Rectangle source = {
    (float)(col * atlas->cell_size.x),
    -(float)((row+1)* atlas->cell_size.y),
    (float)atlas->cell_size.x,
    -(float)atlas->cell_size.y,
  };
  float w = atlas->cell_size.x * scale;
  float h = atlas->cell_size.y * scale;
  Rectangle dest = { pos.x, pos.y, w, h };
  Vector2 origin = { w * 0.5f, h * 0.5f };
  DrawTexturePro(atlas->texture, source, dest, origin, rotation, tint);
}

void draw_render_texture(Texture2D *texture, float x, float y, float rotation,
                         float scale, Color tint)
{
  /* Negative source height flips Y — required for render textures in raylib
     (OpenGL bottom-left origin vs screen top-left). */
  Rectangle source = { 0.0f, 0.0f, (float)texture->width, -(float)texture->height };
  Rectangle dest = { x - texture->width * scale * 0.5f, y - texture->height * scale * 0.5f,
    texture->width * scale, texture->height * scale };
  Vector2 origin = { 0, 0 };
  DrawTexturePro(*texture, source, dest, origin, rotation, tint);
}

void draw_texture(
  Texture2D *texture, 
  float x, float y,
  float rotation, float scale,
  Color tint)
{
  Rectangle source = {0.0f, 0.0f, (float)texture->width,
                      (float)texture->height};
  Rectangle dest = {x - texture->width * scale * 0.5f,
                    y - texture->height * scale * 0.5f, texture->width * scale,
                    texture->height * scale};
  Vector2 origin = {0, 0};
  DrawTexturePro(*texture, source, dest, origin, rotation, tint);
}

void draw_fps()
{
  Color color = LIME; // Good FPS
  int fps = GetFPS();

#if !defined(PLATFORM_WEB)
  int target_fps = GetMonitorRefreshRate(GetCurrentMonitor());
  if (fps < target_fps * 0.80) color = RED; // Low FPS
  else if (fps < target_fps * 0.95) color = ORANGE;  // Warning FPS
#endif

  const char *text = TextFormat("%2i FPS", fps);
  u32 text_width = MeasureText(text, 20);
  DrawText(text, GetScreenWidth() - text_width - 10, 10, 20, color);

  // {
  //   const char* target_fps_text = TextFormat("%2i HZ", target_fps);
  //   DrawText(target_fps_text, GetScreenWidth() - MeasureText(target_fps_text, 20) - 10, 30, 20, color);
  // }
}

void draw_line_dashed_thick(
  Vector2 start, 
  Vector2 end, 
  float thickness, 
  float dash_length, 
  Color color
) 
{
  float distance = Vector2Distance(start, end);

  // Se a distância for muito curta, desenha um ponto fixo centralizado
  if (distance < 0.1f) return;

  Vector2 direction = Vector2Normalize(Vector2Subtract(end, start));
  float current_length = 0.0f;

  // Alterna entre desenhar o traço (dash) e pular o espaço (blank)
  bool draw_dash = true;

  while (current_length < distance) {
    // Garante que o último traço não passe do ponto final
    float step = dash_length;
    if (current_length + step > distance) {
      step = distance - current_length;
    }

    Vector2 segment_start = Vector2Add(start, Vector2Scale(direction, current_length));
    Vector2 segment_end = Vector2Add(segment_start, Vector2Scale(direction, step));

    if (draw_dash) {
      // Desenha o segmento grosso. Se thickness == dash_length, vira um quadrado perfeito.
      DrawLineEx(segment_start, segment_end, thickness, color);
    }

    current_length += step;
    draw_dash = !draw_dash; // Alterna o estado para criar o espaço em branco
  }
}

API void draw_npatch(resource_texture_id_t id, float x, float y, float width, float height, float scale, Color tint)
{
  Texture2D texture = resource_texture(id);
  int border = 8;
  float ox = (1.0f - scale) * width * 0.5f;
  float oy = (1.0f - scale) * height * 0.5f;
  NPatchInfo info = {
    .source = (Rectangle){ 0, 0, (float)texture.width, (float)texture.height },
    .left = border,
    .top = border,
    .right = border,
    .bottom = border,
    .layout = NPATCH_NINE_PATCH
  };
  Rectangle dest = { roundf(x + ox), roundf(y + oy), roundf(width * scale), roundf(height * scale) };
  DrawTextureNPatch(texture, info, dest, (Vector2){0}, 0.0f, tint);
}
