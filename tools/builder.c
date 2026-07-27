// builder
// atempt to remove shell lang
// kind nop.h (tsoding)

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CC "gcc"
#define INC "-I./include -I./libs/raylib/src"
#define DEPS_BASE "-L./libs/raylib/build/raylib -lraylib -lm -lX11"
#define DEPS "$DEPS_BASE"
#define LD_FLAGS ""

#define SRC_DIR    "./src"
#define BUILD_DIR  "./build"
#define OUTPUT_BIN "match-tudo"
#define TARGET     BUILD_DIR "/" OUTPUT_BIN

// Size of internal static buffers used on some functions:
#ifndef MAX_TEXT_BUFFER_LENGTH
  #define MAX_TEXT_BUFFER_LENGTH 256
#endif

// Maximum number of static buffers for text formatting
#ifndef MAX_TEXTFORMAT_BUFFERS
  #define MAX_TEXTFORMAT_BUFFERS 4 
#endif

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

    // If required_byte_count is larger than the MAX_TEXT_BUFFER_LENGTH, then overflow occurred
    if (required_byte_count >= MAX_TEXT_BUFFER_LENGTH) {
      // Inserting "..." at the end of the string to mark as truncated
      char *trunc_buffer = buffers[index] + MAX_TEXT_BUFFER_LENGTH - 4; // Adding 4 bytes = "...\0"
      snprintf(trunc_buffer, 4, "...");
      printf("[WARN] previous log messat truncated %d chars", required_byte_count - (int)sizeof(buffer) + 1);
    }

    index += 1;     // Move to next buffer for next function call
    if (index >= MAX_TEXTFORMAT_BUFFERS) index = 0;
  }

  return buffer;
}

bool exec_build()
{
  printf("Starting build process...\n");
  printf("buld_dir: %s\n", BUILD_DIR);
  printf("call: %s\n", format_text("mkdir -p %s", BUILD_DIR));
  if (system(format_text("mkdir -p %s", BUILD_DIR))) {
    return true;
  }
  return false;
}

int main(int argc, char **argv)
{
  printf("src_dir: %s\n", SRC_DIR);
  printf("TARGET: %s\n", TARGET);

  printf("\nargc: %d\n", argc);
  for (int i = 0; i < argc; i++) {
    printf("%s\n", argv[i]);
  }

  exec_build();

  printf("done\n");
}
