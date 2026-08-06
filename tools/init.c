#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CC "gcc"
#define INC "-I./include -I./libs/raylib/src"
#define DEPS "-L./libs/raylib/build/raylib -lraylib -lm -lX11"

#define MAX_TEXT_BUFFER_LENGTH 256
#define MAX_TEXTFORMAT_BUFFERS 4 

#define printn_clear(...) printf("\33[2K\r" __VA_ARGS__);fflush(stdout)

void extract_build_path(const char *bin_path, char *build_path, size_t build_path_max);
void printn(const char* format, ...);
const char* format_text(const char *format, ...);
void run(const char *format, ...);
void compile(const char *source, const char *output);

static char g_builld_path[64];

int main(int argc, char *argv[])
{
  (void) argc; (void) argv;
  printn("dev init");

  extract_build_path(argv[0], g_builld_path, sizeof(g_builld_path));
  compile("lvl_parser.c", "lvl_parser");
  run("./build/lvl_parser");

  printn_clear("");
  return EXIT_SUCCESS;
}

void compile(const char *source, const char *output)
{
  assert(g_builld_path[0] != '\0' && "build_path not created: use extract_build_path(...)" );
  const char format[] = {
    CC " tools/%s " INC " " DEPS " -o %s/%s"
  };
  run(format, source, g_builld_path, output);
}

void run(const char *format, ...)
{
  assert(format);

  char buffer[MAX_TEXT_BUFFER_LENGTH];

  va_list args;
  va_start(args, format);
  int required_byte_count = vsnprintf(buffer, MAX_TEXT_BUFFER_LENGTH, format, args);
  if (required_byte_count >= MAX_TEXT_BUFFER_LENGTH) {
    printn("[ERROR] cmd(%d chars) >= MAX_TEXT_BUFFER_LENGTH(%d chars)", required_byte_count, MAX_TEXT_BUFFER_LENGTH);
    exit(EXIT_FAILURE);
  }
  va_end(args);

  printn("run: %s", buffer);
  int error = system(buffer);
  if (error) {
    exit(EXIT_FAILURE);
  }
}

void extract_build_path(const char *bin_path, char *build_path, size_t build_path_max)
{
  size_t bin_path_len = strlen(bin_path);
  size_t build_path_index = 0;
  bool first_slash = false;
  for (size_t i = bin_path_len - 1; i >= 0; i--) {
    if (bin_path[i] == '\0' || build_path_index >= build_path_max) {
      break;
    }
    if (!first_slash && bin_path[i] != '/') {
      continue;
    }
    else if (!first_slash && bin_path[i] == '/') {
      first_slash = true;
      continue;
    }
    build_path_index += 1;
    build_path[i] = bin_path[i];
  }
  build_path[build_path_index] = '\0';
}

void printn(const char* format, ...) 
{
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  printf("\n");
  va_end(args);
}

const char* format_text(const char *format, ...)
{
  static char buffers[MAX_TEXTFORMAT_BUFFERS][MAX_TEXT_BUFFER_LENGTH] = { 0 };
  static int index = 0;

  char *buffer = buffers[index];
  memset(buffer, 0, MAX_TEXT_BUFFER_LENGTH); // Clear buffer before using

  if (format != NULL) {
    va_list args;
    va_start(args, format);
    int required_byte_count = vsnprintf(buffer, MAX_TEXT_BUFFER_LENGTH, format, args);
    va_end(args);

    // if required_byte_count is larger than the max_text_buffer_length, then overflow occurred
    if (required_byte_count >= MAX_TEXT_BUFFER_LENGTH) {
      // inserting "..." at the end of the string to mark as truncated
      char *trunc_buffer = buffers[index] + MAX_TEXT_BUFFER_LENGTH - 4; // Adding 4 bytes = "...\0"
      snprintf(trunc_buffer, 4, "...");
      printf("[WARN] previous log messat truncated %d chars", required_byte_count - (int)sizeof(buffer) + 1);
    }

    index += 1;     // Move to next buffer for next function call
    if (index >= MAX_TEXTFORMAT_BUFFERS) index = 0;
  }

  return buffer;
}

