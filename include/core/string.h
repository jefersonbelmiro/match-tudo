#pragma once

#include "core/defs.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

// size of internal static buffers used on some functions:
#ifndef MAX_TEXT_BUFFER_LENGTH
  #define MAX_TEXT_BUFFER_LENGTH 256
#endif

// maximum number of static buffers for text formatting
#ifndef MAX_TEXTFORMAT_BUFFERS
  #define MAX_TEXTFORMAT_BUFFERS 4
#endif

// @note: we don't have to worry about str being shorter than pre because
// according to the c standard (7.21.4.4/2):
//    The strncmp function compares not more than n characters (characters that
//    follow a null character are not compared) from the array pointed to by s1
//    to the array pointed to by s2."
API bool start_with(char *str, char *pre)
{
  return memcmp(pre, str, strlen(pre)) == 0;
}

API bool start_with_n(char *str, char *pre, size_t pre_length)
{
  return memcmp(pre, str, pre_length) == 0;
}

API bool is_alpha_num(char c) 
{
  return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') ||
         (c >= 'a' && c <= 'z');
}

API bool is_empty(char c) 
{
  return c == ' ' || c == '\t' || c == '\n';
}

API bool is_space(char c)
{
  return c == ' ' || c == '\t';
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
