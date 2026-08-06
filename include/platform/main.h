#pragma once

#include "core/defs.h"
#include "raylib.h"
#if defined(PLATFORM_WEB)
#include "platform/web.h"
#endif

API void platform_init() 
{
#if defined(PLATFORM_WEB)
  platform_web_init();
#endif
}

API bool platform_is_ready()
{
#if defined(PLATFORM_WEB)
  return platform_web_is_ready();
#endif
  return true;
}

API bool platform_is_mobile()
{
#if defined(PLATFORM_WEB)
  return platform_web_is_mobile();
#endif
  return false;
}

API const char* platform_resolve_data_path(const char *file_name)
{
  return TextFormat("%s%s/%s", GetApplicationDirectory(), APP_STORAGE_PATH, file_name);
}

API const char* platform_resolve_package_path(const char *file_name)
{
#if defined(PLATFORM_WEB)
  return file_name;
#else 
  return TextFormat("%s%s", GetApplicationDirectory(), file_name);
#endif
}

API bool platform_save_file(const char *file_name, const void *data, const int data_size)
{
  const char *base_directory = GetApplicationDirectory();

  if (!DirectoryExists(base_directory)) {
    MakeDirectory(base_directory);
  }

  bool saved = SaveFileData(TextFormat("%s/%s", base_directory, file_name), data, data_size);

#if defined(PLATFORM_WEB)
  platform_web_syncfs();
#endif

  return saved;
}

API unsigned char* platform_load_file(const char *file_name, int *data_size)
{
  const char *path = TextFormat("%s%s", GetApplicationDirectory(), file_name);
  unsigned char *buff = NULL;
  if (FileExists(path)) {
    buff = LoadFileData(path, data_size);
  }
  // @note: caller need to call unload
  // UnloadFileData(buff);
  return buff;
}
