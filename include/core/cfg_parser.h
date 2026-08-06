#pragma once

#include "core/defs.h"
#include "core/string.h"
#include <stddef.h>

#define CFG_TOKEN_MAX_NAME_SIZE 32
#define CFG_TOKEN_MAX_VALUE_SIZE 64
#define CFG_TOKEN_MAX_ARRAY_CHILD_COUNT 16
#define CFG_TOKEN_MAX_KEY_VALUE_COUNT 16
#define CFG_TOKEN_MAX_ARRAY_COUNT 16

typedef struct {
  char name[CFG_TOKEN_MAX_NAME_SIZE];
  char value[CFG_TOKEN_MAX_VALUE_SIZE];
} cfg_token_key_value_t;

typedef struct {
  char name[CFG_TOKEN_MAX_NAME_SIZE];
  char values[CFG_TOKEN_MAX_ARRAY_CHILD_COUNT][CFG_TOKEN_MAX_VALUE_SIZE];
  u16  count;
} cfg_token_array_t;

typedef struct {
  cfg_token_key_value_t  key_value_tokens[CFG_TOKEN_MAX_KEY_VALUE_COUNT];
  cfg_token_array_t      array_tokens[CFG_TOKEN_MAX_ARRAY_COUNT];
  u16                    key_value_tokens_count;
  u16                    array_tokens_count;
} cfg_tokens_t;

API void cfg_print_tokens(cfg_tokens_t *tokens)
{
  printn("[tokens]");
  printn("");
  for (int i = 0; i < tokens->key_value_tokens_count; i++) {
    printn("  %s: %s", tokens->key_value_tokens[i].name, tokens->key_value_tokens[i].value);
  }

  printn("");
  for (int i = 0; i < tokens->array_tokens_count; i++) {
    printn("  %s:", tokens->array_tokens[i].name);
    for (int ci = 0; ci < tokens->array_tokens[i].count; ci++) {
      printn("    - %s", tokens->array_tokens[i].values[ci]);
    }
  }
}

API bool cfg__is_key_value(const char *line)
{
  while (*line != '\0') {
    if (*line == ':') {
      line++;
      while (is_empty(*line)) {
        line++;
        continue;
      }
      if (is_alpha_num(*line)) {
        return true;
      }
      break;
    }
    line++;
  }
  return false;
}

API void cfg__sanitize_line(const char **line)
{
  size_t len = strlen(*line);
  if (len == 0) return;
  size_t index = len - 1;
  while ((*line)[index] == ' ') {
    ((char *)(*line))[index] = '\0'; // overwrid the space
    if (index == 0) break;
    index--;
  }
}

API void cfg__make_key_value(const char *line, cfg_token_key_value_t *token)
{
  int char_index = 0;
  while (*line != '\0') {
    while (is_empty(*line)) {
      line++;
      continue;
    }

    while (is_alpha_num(*line) || is_space(*line)) {
      token->name[char_index++] = *line;
      line++;
      continue;
    }

    token->name[char_index] = '\0';

    while (is_empty(*line)) {
      line++;
      continue;
    }

    char_index = 0;

    if (*line == ':') {
      line++;

      while (is_empty(*line)) {
        line++;
        continue;
      }

      while (*line != '\0') {
        token->value[char_index++] = *line;
        line++;
        continue;
      }

      token->value[char_index] = '\0';
    } 
    line++;
  }
}

API void cfg__make_array(char lines[256][64], int line_count, int index, cfg_token_array_t *token)
{
  const char *line = lines[index];
  int char_index = 0;
  while (*line != '\0') {
    while (is_empty(*line)) {
      line++;
      continue;
    }
    while (is_alpha_num(*line)) {
      token->name[char_index++] = *line;
      line++;
      continue;
    }

    token->name[char_index] = '\0';
    index++;
    break;
  }

  if (token->name[0] == '\0' || index >= line_count) {
    return;
  }

  for (int li = index; li < line_count; li++) {
    char_index = 0;
    line = lines[li];

    if (*line != ' ' && *(line+1) != ' ') {
      break;
    }

    while (is_empty(*line)) {
      line++;
      continue;
    }

    while (*line != '\0') {
      token->values[token->count][char_index++] = *line;
      line++;
      continue;
    }

    token->values[token->count][char_index] = '\0';
    token->count++;
  }
}

API bool cfg__is_array(const char *line)
{
  bool found = false;
  while (*line != '\0') {
    if (*line == ':') {
      found = true;
      line++;
      while (is_empty(*line)) {
        line++;
        continue;
      }
      if (*line == '\0') {
        return true;
      }
      return false;
    }
    line++;
  }
  return found;
}

API void cfg_parse(unsigned char *file_content, cfg_tokens_t *tokens)
{
  char lines[256][64];
  int line_count = 0;
  int cursor_index = 0;
  unsigned char *cursor = file_content;

  while (*cursor != '\0') {
    if (*cursor == '\n') {
      lines[line_count][cursor_index] = '\0';
      line_count++;
      cursor++;
      cursor_index = 0;
      continue;
    }

    lines[line_count][cursor_index] = *cursor;
    cursor++;
    cursor_index++;
  }

  for (int i = 0; i < line_count; i++) {

    const char *line = lines[i];
    while (*line != '\0' && is_empty(*line)) {
      line++;
    }
    if (line[0] == '\0') {
      continue;
    }

    cfg__sanitize_line(&line);
    if (cfg__is_key_value(line)) {
      cfg_token_key_value_t token = {0};
      cfg__make_key_value(line, &token);
      tokens->key_value_tokens[tokens->key_value_tokens_count++] = token;
    }
    else if (cfg__is_array(line)) {
      cfg_token_array_t token = {0};
      cfg__make_array(lines, line_count, i, &token);
      tokens->array_tokens[tokens->array_tokens_count++] = token;
    }

  }
}

// API void cfg_file_parse(const char *path, cfg_tokens_t *tokens)
// {
//   int data_size = 0;
//   unsigned char *buffer = platform_load_file(path, &data_size);
//
//   if (!data_size) {
//     printn("[ERROR][cfg_file_parse] invalid data from '%s'", path);
//     return;
//   }
//
//   cfg_parse(buffer, tokens);
// }
