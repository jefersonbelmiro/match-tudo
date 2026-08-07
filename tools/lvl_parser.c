#include "core/cfg_parser.h"
#include "core/defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define FILE_DIRECTORY "resources/levels"
#define FILE_EXTENSION "cfg"

typedef struct {
  char base_path[64];
  char name[64];
  char path[128];
} file_entry_t;

typedef struct {
  file_entry_t array[8];
  u16 count;
} file_entries_t;

file_entries_t *recursive_find_ext(const char *base_path, const char *ext, file_entries_t *entries) 
{
  char path[128];
  DIR *dir = opendir(base_path);
  if (!dir) {
    return NULL;
  }

  if (entries == NULL) {
    return NULL;
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }

    snprintf(path, sizeof(path), "%s/%s", base_path, entry->d_name);

    struct stat statbuf;

    if (stat(path, &statbuf) != 0) {
      continue;
    }

    if (S_ISDIR(statbuf.st_mode)) {
      recursive_find_ext(path, ext, entries);
      continue;
    }

    size_t len_name = strlen(entry->d_name);
    size_t len_ext = strlen(ext);

    if (len_name >= len_ext &&
        strcmp(entry->d_name + len_name - len_ext, ext) == 0) {
      file_entry_t *file_entry = &entries->array[entries->count++];
      strcpy(file_entry->base_path, base_path);
      strcpy(file_entry->path, path);
      strcpy(file_entry->name, entry->d_name);
    }
  }
  closedir(dir);

  return entries;
}

unsigned char *read_file_content(const char *path) 
{
  static unsigned char buffer[1024];

  FILE *file = fopen(path, "rb");
  if (!file) {
    printn("[error][read_file_content] error on open file '%s'");
    return NULL;
  }

  unsigned long readed = fread(&buffer, sizeof(char), sizeof(buffer), file);
  printn("bytes read: %d", readed);
  fclose(file);
  return buffer;
}

void make_lvl_pack(file_entry_t *file, cfg_tokens_t tokens)
{
  (void) file;  (void) tokens;
}

int main()
{
  file_entries_t files = {0};

  recursive_find_ext(FILE_DIRECTORY, FILE_EXTENSION, &files);

  for (u16 i = 0; i < files.count; i++) {
    file_entry_t *entry = &files.array[i];
    unsigned char *buffer = read_file_content(entry->path);
    cfg_tokens_t tokens = {0};
    cfg_parse(buffer, &tokens);
    printn("");
    cfg_print_tokens(&tokens);
    printn("");
  }

  return EXIT_SUCCESS;
}
