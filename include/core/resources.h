#pragma once

#include "core/arena.h"
#include "core/defs.h"
#include "raylib.h"
#include <assert.h>
#include <string.h>

typedef enum  {
  RESOURCE_ATLAS_COUNT,
} resource_atlas_id_t;

typedef struct  {
  Vector2 cell_size;
} resource_atlas_config_t;

typedef enum  {
  RESOURCE_TEXTURE_001,
  RESOURCE_TEXTURE_002,
  RESOURCE_TEXTURE_003,
  RESOURCE_TEXTURE_004,

  RESOURCE_TEXTURE_COUNT,
} resource_texture_id_t;

typedef enum  {
  RESOURCE_FONT_MONOGRAM_32,

  RESOURCE_FONT_COUNT,
} resource_font_id_t;

typedef struct  {
  float font_size;
} resource_font_config_t;

typedef enum  {
  RESOURCE_SOUND_COUNT,
} resource_sound_id_t;

typedef struct  {
  float volume;
  u8    max_active;
} resource_sound_config_t;

typedef enum {
  RESOURCE_MUSIC_COUNT,
} resource_music_id_t;

typedef struct  {
  float volume;
} resource_music_config_t;

typedef enum {
  RESOURCE_SHADER_COUNT,
} resource_shader_id_t;

typedef struct {
  // const char atlas_array   [RESOURCE_ATLAS_COUNT][64];
  const char texture_array [RESOURCE_TEXTURE_COUNT][64];
  const char font_array    [RESOURCE_FONT_COUNT][64];
  // const char sound_array   [RESOURCE_SOUND_COUNT][64];
  // const char music_array   [RESOURCE_MUSIC_COUNT][64];
  // const char shader_array  [RESOURCE_SHADER_COUNT][64];
} resources_path_t;

// static resource_atlas_config_t g_atlas_config[RESOURCE_ATLAS_COUNT] = {
//   // [RESOURCE_ATLAS_1_64] = { .cell_size = {64, 64}, },
// };

static resource_font_config_t g_font_config[RESOURCE_FONT_COUNT] = {
  [RESOURCE_FONT_MONOGRAM_32] = { .font_size = 64 * 4 },
};

// static resource_sound_config_t g_sound_config[RESOURCE_SOUND_COUNT] = {
//   // [RESOURCE_SOUND_FLEET_HIT]      = { 0.20f, 3 },
// };

// static resource_music_config_t g_music_config[RESOURCE_SOUND_COUNT] = {
//   // [RESOURCE_MUSIC_MENU_01] = { 0.2 },
// };
//
static resources_path_t g_resources_path = {
  .texture_array[RESOURCE_TEXTURE_001] = "resources/texture/001.jpg",
  .texture_array[RESOURCE_TEXTURE_002] = "resources/texture/002.jpg",
  .texture_array[RESOURCE_TEXTURE_003] = "resources/texture/003.jpg",
  .texture_array[RESOURCE_TEXTURE_004] = "resources/texture/004.jpg",

  .font_array[RESOURCE_FONT_MONOGRAM_32] = "resources/font/monogram.ttf"
};

typedef struct {
  Texture2D texture;
  Vector2 cell_size;
} atlas_t;

typedef struct {
  // atlas_t   atlas_array   [RESOURCE_ATLAS_COUNT];
  Texture2D texture_array [RESOURCE_TEXTURE_COUNT];
  Font      font_array    [RESOURCE_FONT_COUNT];
  // Sound     sound_array   [RESOURCE_SOUND_COUNT];
  // Music     music_array   [RESOURCE_MUSIC_COUNT];
  // Shader    shader_array  [RESOURCE_SHADER_COUNT];
} resources_t;

GLOBAL resources_t *g_resource;

API resources_t *resource_ptr(void) 
{
  assert(g_resource);
  return g_resource;
}

u32 resources_memory_size()
{
  u32 size = 0;
  size += sizeof(resources_t);
  return size;
}

API void resource_init(arena_t *arena)
{
  g_resource = arena_push_zero(arena, resources_t, 1);
}
 
API void resource_start()
{
  // atlas_t   *atlas_array   = g_resource->atlas_array;
  Texture2D *texture_array = g_resource->texture_array;
  Font      *font_array    = g_resource->font_array;
  // Sound     *sound_array   = g_resource->sound_array;
  // Music     *music_array   = g_resource->music_array;
  // Shader    *shader_array  = g_resource->shader_array;

  // for (u8 i = 0; i < RESOURCE_ATLAS_COUNT; i++) {
  //   atlas_array[i] = (atlas_t) {
  //     .texture = LoadTexture(g_resources_path.atlas_array[i]),
  //     .cell_size = g_atlas_config[i].cell_size,
  //   };
  // }

  for (u8 i = 0; i < RESOURCE_TEXTURE_COUNT; i++) {
    texture_array[i] = LoadTexture(g_resources_path.texture_array[i]);
    if (!IsTextureValid(texture_array[i])) {
      log_error("invalid texture #%d '%s' ", i, g_resources_path.texture_array[i]);
    }
  }

  for (u8 i = 0; i < RESOURCE_FONT_COUNT; i++) {
    float font_size = g_font_config[i].font_size;
    font_array[i] = LoadFontEx(g_resources_path.font_array[i], font_size, NULL, 0);
    if (!IsFontValid(font_array[i])) {
      log_error("invalid font #%d '%s' error", i, g_resources_path.font_array[i]);
    }
  }

  // for (u8 i = 0; i < RESOURCE_SHADER_COUNT; i++) {
  //   shader_array[i] = LoadShader(0, g_resources_path.shader_array[i]);
  // }

  // InitAudioDevice();
  //
  // if (!IsAudioDeviceReady()) {
  //   return;
  // }

  // for (u8 i = 0; i < RESOURCE_SOUND_COUNT; i++) {
  //   sound_array[i] = LoadSound(g_resources_path.sound_array[i]);
  //   if (!IsSoundValid(sound_array[i])) {
  //     printn("ERROR: on load sound: %d", i);
  //   }
  // }

  // @NOTE: use LoadSoundAlias on duplicates
  // for (u8 i = 0; i < RESOURCE_MUSIC_COUNT; i++) {
  //   music_array[i] = LoadMusicStream(g_resources_path.music_array[i]);
  // }
}

// API atlas_t resource_atlas(resource_atlas_id_t id)
// {
//   assert(g_resource && id < RESOURCE_ATLAS_COUNT);
//   return g_resource->atlas_array[id];
// }
//
// API atlas_t* resource_atlas_ptr(resource_atlas_id_t id)
// {
//   assert(g_resource && id < RESOURCE_ATLAS_COUNT);
//   return &g_resource->atlas_array[id];
// }

API Texture2D resource_texture(resource_texture_id_t id)
{
  assert(g_resource && id < RESOURCE_TEXTURE_COUNT);
  return g_resource->texture_array[id];
}

API Texture2D* resource_texture_ptr(resource_texture_id_t id)
{
  assert(g_resource && id < RESOURCE_TEXTURE_COUNT);
  return &g_resource->texture_array[id];
}

API Font resource_font(resource_font_id_t id)
{
  assert(g_resource && id < RESOURCE_FONT_COUNT);
  return g_resource->font_array[id];
}

API Font* resource_font_ptr(resource_font_id_t id)
{
  assert(g_resource && id < RESOURCE_FONT_COUNT);
  return &g_resource->font_array[id];
}

// API Sound resource_sound(resource_sound_id_t id)
// {
//   assert(g_resource && id < RESOURCE_SOUND_COUNT);
//   return g_resource->sound_array[id];
// }
//
// API Sound* resource_sound_ptr(resource_sound_id_t id)
// {
//   assert(g_resource && id < RESOURCE_SOUND_COUNT);
//   return &g_resource->sound_array[id];
// }
//
// API Music resource_music(resource_music_id_t id)
// {
//   assert(g_resource && id < RESOURCE_MUSIC_COUNT);
//   return g_resource->music_array[id];
// }
//
// API Music* resource_music_ptr(resource_music_id_t id)
// {
//   assert(g_resource && id < RESOURCE_MUSIC_COUNT);
//   return &g_resource->music_array[id];
// }
//
// API Shader resource_shader(resource_shader_id_t id)
// {
//   assert(g_resource && id < RESOURCE_SHADER_COUNT);
//   return g_resource->shader_array[id];
// }
//
// API Shader* resource_shader_ptr(resource_shader_id_t id)
// {
//   assert(g_resource && id < RESOURCE_SHADER_COUNT);
//   return &g_resource->shader_array[id];
// }

API void resource_unload()
{
  if (!g_resource) return;

  // for (u8 i = 0; i < RESOURCE_ATLAS_COUNT; i++) {
  //   UnloadTexture(resource_atlas_ptr(i)->texture);
  // }

  for (u8 i = 0; i < RESOURCE_TEXTURE_COUNT; i++) {
    UnloadTexture(resource_texture(i));
  }

  for (u8 i = 0; i < RESOURCE_FONT_COUNT; i++) {
    UnloadFont(resource_font(i));
  }

  // for (u8 i = 0; i < RESOURCE_SOUND_COUNT; i++) {
  //   UnloadSound(resource_sound(i));
  // }

  // for (u8 i = 0; i < RESOURCE_MUSIC_COUNT; i++) {
  //   UnloadMusicStream(resource_music(i));
  // }

  // for (u8 i = 0; i < RESOURCE_SHADER_COUNT; i++) {
  //   UnloadShader(resource_shader(i));
  // }

  g_resource = NULL;

  // CloseAudioDevice();
}
