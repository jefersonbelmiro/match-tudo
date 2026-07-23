#pragma once

#include "core/defs.h"
#include "core/mem.h"
#include "core/string.h"
#include "raylib.h"
#include <assert.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#define HOT_MAX_FILES 96
#define HOT_MAX_IGNORE 10
#define HOT_MAX_IGNORE_PATH 128

extern void app_emit_hot_sync();

typedef struct {
  const char  *file;
  char       **funcs;
  int          func_count;
  void       **orig_addrs;
  void        *lib;
  long        mtime;
} hot_code_file_t;

static hot_code_file_t  g_hot_code_files[HOT_MAX_FILES];
static int              g_hot_code_file_count = 0;

static char g_ignore_list[HOT_MAX_IGNORE][HOT_MAX_IGNORE_PATH];
static u8   g_ignore_list_count = 0;

static inline
void hot_code__set_ignore(u8 count, const char **paths)
{
  g_ignore_list_count = 0;
  for (u8 i = 0; i < count; i++) {
    mem_copy((char*)paths[i], g_ignore_list[g_ignore_list_count], HOT_MAX_IGNORE_PATH - 1);
    g_ignore_list[g_ignore_list_count][HOT_MAX_IGNORE_PATH - 1] = '\0';
    g_ignore_list_count++;
    assert(g_ignore_list_count <= HOT_MAX_IGNORE);
  }
}

static inline
char *hot_code__strdup(const char *s)
{
  size_t l = strlen(s) + 1;
  char  *d = malloc(l);
  if (d) memcpy(d, s, l);
  return d;
}

static inline
bool hot_code__ignore(const char *path)
{
  u8 count = g_ignore_list_count;
  for (u8 i = 0; i < count; i++) {
    if (start_with((char*)path, (char*)g_ignore_list[i])) {
      return true;
    }
  }

  return false;
}

static inline
void hot_code__register(u32 count, const char **args)
{
  assert(g_hot_code_file_count < HOT_MAX_FILES);

  const char *file = args[0];
  u32 func_count = count - 1;
  int idx = g_hot_code_file_count++;

  g_hot_code_files[idx].file       = hot_code__strdup(file);
  g_hot_code_files[idx].funcs      = calloc(func_count, sizeof(char*));
  g_hot_code_files[idx].orig_addrs = calloc(func_count, sizeof(void*));
  g_hot_code_files[idx].func_count = func_count;
  g_hot_code_files[idx].lib        = NULL;

  for (u32 i = 0; i < func_count; i++) {
    const char *f = args[i + 1];
    g_hot_code_files[idx].funcs[i]      = hot_code__strdup(f);
    g_hot_code_files[idx].orig_addrs[i] = dlsym(RTLD_DEFAULT, f);
  }

  g_hot_code_files[idx].mtime = GetFileModTime(file);
}

static inline
void hot_code__patch_func(void *old, void *new_fn)
{
  if (!old || !new_fn || old == new_fn) return;

  // Write 12-byte absolute JMP directly at the function entry:
  //   mov rax, imm64
  //   jmp rax
  unsigned char jmp[] = {
    0x48, 0xB8,
    (uint64_t)new_fn >>  0, (uint64_t)new_fn >>  8,
    (uint64_t)new_fn >> 16, (uint64_t)new_fn >> 24,
    (uint64_t)new_fn >> 32, (uint64_t)new_fn >> 40,
    (uint64_t)new_fn >> 48, (uint64_t)new_fn >> 56,
    0xFF, 0xE0,
  };

  uintptr_t start = (uintptr_t)old & ~0xFFF;
  uintptr_t end   = ((uintptr_t)old + 11) & ~0xFFF;
  mprotect((void*)start, (end - start) + 0x1000,
           PROT_READ | PROT_WRITE | PROT_EXEC);
  memcpy(old, jmp, 12);
  __builtin___clear_cache((char*)old, (char*)old + 12);
}

static inline
bool hot_code__reload_file(int idx)
{
  char so_path[64];
  snprintf(so_path, sizeof(so_path), "./build/hot_%d.so", idx);

  char cmd[512];
  snprintf(cmd, sizeof(cmd), "./build.sh --hot-build %s %s",
           g_hot_code_files[idx].file, so_path);
  int ret = system(cmd);
  if (ret != 0) {
    printn("[hot] build failed (exit %d)", ret);
    return true; // try once: to ignore dev errors
  }

  if (g_hot_code_files[idx].lib) {
    dlclose(g_hot_code_files[idx].lib);
  };

  void *lib = dlopen(so_path, RTLD_NOW |RTLD_GLOBAL);
  if (!lib) {
    printn("[hot] dlopen %s", dlerror());
    return true; // try once: to ignore dev errors
  }

  for (int i = 0; i < g_hot_code_files[idx].func_count; i++) {
    void *new = dlsym(lib, g_hot_code_files[idx].funcs[i]);
    if (!new) {
      printn("[hot] dlsym failed for %s", g_hot_code_files[idx].funcs[i]);
      continue;
    }
    hot_code__patch_func(g_hot_code_files[idx].orig_addrs[i], new);
  }

  g_hot_code_files[idx].lib = lib;
  return true;
}

#define hot_code_register(...)                                                 \
  do {                                                                         \
    const char *_arr[] = {__VA_ARGS__};                                        \
    hot_code__register(countof(_arr), _arr);                                   \
  } while (0)

#define hot_code_ignore(...)                                                   \
  do {                                                                         \
    const char *_arr[] = {__VA_ARGS__};                                        \
    hot_code__set_ignore(countof(_arr), _arr);                                 \
  } while (0)

static inline
void hot_code_register_auto(const char *file)
{
  FILE *f = fopen(file, "r");
  if (!f) {
    printn("[hot] error opening file: %s", file);
    return;
  };

  char line[1024], funcs[128][128];
  int count = 0;

  while (fgets(line, sizeof(line), f) && count < 128) {
    char *p = line;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '\0' || *p == '#' || (*p == '/' && p[1] == '/')) continue;
    if (strncmp(p, "API", 3) != 0) continue;
    char c = p[3];
    if (c != ' ' && c != '\t' && c != '\0' && c != '\n' && c != '\r') continue;

    char *after = p + 3;
    while (*after == ' ' || *after == '\t' || *after == '\n' || *after == '\r') after++;

    // API alone on the line -> read next
    if (*after == '\0') {
      if (!fgets(line, sizeof(line), f)) break;
      after = line;
      while (*after == ' ' || *after == '\t') after++;
    }

    // after = "type name(args)"
    char *paren = strchr(after, '(');
    if (!paren) continue;

    char *end = paren - 1;
    while (end >= after && (*end == ' ' || *end == '\t' || *end == '*')) end--;
    char *start = end;
    while (start >= after && *start != ' ' && *start != '\t' && *start != '*') start--;
    start++;

    int nlen = end - start + 1;
    if (nlen <= 0 || nlen >= 128) continue;
    if (strncmp(start, "hot_", 4) == 0) continue;

    // skip forward declarations (end with ; not {)
    {
      char *scan = paren + 1;
      int depth = 1;
      while (*scan && depth > 0) {
        if (*scan == '(') depth++;
        else if (*scan == ')') depth--;
        if (depth > 0) scan++;
      }
      if (depth == 0) {
        scan++;
        while (*scan == ' ' || *scan == '\t') scan++;
        if (*scan == ';') continue;
      }
    }

    strncpy(funcs[count], start, nlen);
    funcs[count][nlen] = '\0';
    count++;
  }

  fclose(f);
  if (count == 0) {
    printn("[hot] no functions found in %s", file);
    return;
  };

  const char *args[130];
  args[0] = file;
  for (int i = 0; i < count; i++) args[i + 1] = funcs[i];
  hot_code__register(count + 1, args);
}

static inline
void hot_code_register_all()
{
  FilePathList list = LoadDirectoryFilesEx("include", "FILES*", true);

  if (list.count > HOT_MAX_FILES) {
    printn("[hot] file count(%d) > HOT_MAX_FILES(%d)", list.count, HOT_MAX_FILES);
    return;
  }

  for (unsigned int i = 0; i < list.count; i++) {
    char *path = list.paths[i];
    if (hot_code__ignore(path)) {
      continue;
    }
    hot_code_register_auto(path);
  }
}

static inline
void hot_code_reload_all()
{
  for (int i = 0; i < g_hot_code_file_count; i++) {
    if (hot_code__reload_file(i)) {
      g_hot_code_files[i].mtime = GetFileModTime(g_hot_code_files[i].file);
    }
  }
  app_emit_hot_sync();
}

static inline
void hot_code_process(float delta)
{
  static float watch_timer = 0.0f;
  static float watch_delay = 0.3f;
  static float debounce_delay = 0.3f;

  static long pending_mtime[HOT_MAX_FILES] = {0};
  static float debounce_elapsed[HOT_MAX_FILES] = {0};

  bool updated = false;

  watch_timer += delta;
  if (watch_timer < watch_delay) {
    return;
  };

  for (int i = 0; i < g_hot_code_file_count; i++) {
    long cur = GetFileModTime(g_hot_code_files[i].file);

    // not changed
    if (cur == g_hot_code_files[i].mtime) {
      pending_mtime[i] = 0;
      debounce_elapsed[i] = 0.0f;
      continue;
    }

    // new save
    if (cur != pending_mtime[i]) {
      pending_mtime[i] = cur;
      debounce_elapsed[i] = 0.0f;
      continue;
    }

    if (!pending_mtime[i]) continue;

    debounce_elapsed[i] += delta;

    if (debounce_elapsed[i] >= debounce_delay) {
      if (hot_code__reload_file(i)) {
        g_hot_code_files[i].mtime = cur;
      }
      pending_mtime[i] = 0;
      debounce_elapsed[i] = 0.0f;
      updated = true;
    }
  }

  if (updated) {
    app_emit_hot_sync();
  }
}

