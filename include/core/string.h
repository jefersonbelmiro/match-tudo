#pragma once

#include "core/defs.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

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
