#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#include <stdarg.h>
#include <stdio.h>

#ifndef DEBUG_PRINT_LOC
#define DEBUG_PRINT_LOC 0
#endif

#define LOG_LEVEL_NONE  0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARN  2
#define LOG_LEVEL_INFO  3
#define LOG_LEVEL_DEBUG 4
#define LOG_LEVEL_ALL   5

#ifndef LOG_LEVEL
#define LOG_LEVEL 3
#endif

#ifndef DEBUG_MEMORY_USAGE
#define DEBUG_MEMORY_USAGE 0
#endif

#ifndef GAME_ANIMATED_STATIC_ENTITIES
#define GAME_ANIMATED_STATIC_ENTITIES 0
#endif

#ifndef APP_CUSTOM_CURSOR
#define APP_CUSTOM_CURSOR 0
#endif
#ifndef APP_FULL_SCREEN
#define APP_FULL_SCREEN 0
#endif
#ifndef APP_WINDOW_TOPMOST
#define APP_WINDOW_TOPMOST 0
#endif
#ifndef APP_WINDOW_MONITOR
#define APP_WINDOW_MONITOR -1
#endif
#ifndef APP_WINDOW_UNDECORATED
#define APP_WINDOW_UNDECORATED 0
#endif
#ifndef APP_WINDOW_TRANSPARENT
#define APP_WINDOW_TRANSPARENT 0
#endif
#ifndef APP_WINDOW_WIDTH
#define APP_WINDOW_WIDTH 1024
#endif
#ifndef APP_WINDOW_HEIGHT
#define APP_WINDOW_HEIGHT 720
#endif
#ifndef APP_STORAGE_PATH
#define APP_STORAGE_PATH "data"
#endif
#ifndef APP_PACKAGE_RESOURCE
#define APP_PACKAGE_RESOURCE 1
#endif

#ifndef HOT_RELOAD
#define HOT_RELOAD 0
#endif

#ifndef HOT_RELOAD_UPDATE_ON_SAVE
#define HOT_RELOAD_UPDATE_ON_SAVE 1
#endif

#if HOT_RELOAD
  #define API __attribute__((noinline)) __attribute__((aligned(16))) 
  #ifdef MODULE_BUILD
    #define GLOBAL extern
  #else
    #define GLOBAL
  #endif
#else
  #define API      static inline
  #define GLOBAL   static
#endif

#define MB(size) ((size) * 1024 * 1024)
#define KB(size) ((size) * 1024)
#define UNUSED __attribute__((unused))
#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)

#define countof(a)    (u32)(sizeof(a) / sizeof((a)[0]))
#define expand_of(a)  (a), countof(a)

typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;

typedef u16 entity_id_t;
typedef u16 grid_idx_t;

typedef u32 ui_flag_t;
typedef u8 input_layer_t;

typedef struct {
  float x;
  float y;
} screen_size_t;

typedef struct {
  float x;
  float y;
  float width;
  float height;
} view_port_t;

typedef enum {
  SYNC_SIGNAL_WINDOW_RESIZED,
  SYNC_SIGNAL_ON_ENTER,  // called once before entering loop starts
  SYNC_SIGNAL_ON_EXIT,   // called once before exiting loop starts
  SYNC_SIGNAL_HOT_SYNC,
  SYNC_SIGNAL_COUNT,
} sync_signal_type_t;

typedef enum {
  APP_RUNNING,
  APP_PAUSED,
  APP_EXITING,
  APP_EXITED,
} app_state_t;

typedef enum {
  SCENE_NONE,
  SCENE_MENU,
  SCENE_MAIN,
  SCENE_EDITOR,
  SCENE_COUNT,
} scene_type_t;

typedef enum {
  SCENE_TRANSITION_NONE,
  SCENE_TRANSITION_ENTERING,
  SCENE_TRANSITION_EXITING,
} app_scene_transition_t;

typedef enum {
  PROCESS_MODE_DEFAULT,
  PROCESS_MODE_ALWAYS,
} process_mode_t;

enum {
  UI_STATE_HOVER     = 1u << 0,
  UI_STATE_PRESSED   = 1u << 1,
  UI_STATE_FOCUS     = 1u << 2,
  UI_STATE_ACTIVE    = 1u << 3,
  UI_STATE_DISABLED  = 1u << 4,
};

// Input layer bitmask - multiple layers can be active simultaneously
// Cleared to 0 every frame by input_process.
#define INPUT_LAYER_NONE   0u
#define INPUT_LAYER_GAME   (1u << 0)
#define INPUT_LAYER_HUD    (1u << 1)
#define INPUT_LAYER_HUD_FG (1u << 2)

#define APP_ARENA_SIZE          KB(64)
#define APP_SCENE_ARENA_SIZE    KB(32)

#define MAX_TWEENS              32
#define MAX_TWEENERS_TOTAL      96

#define MAX_TIMERS              32

#define IDX_NONE    ((grid_idx_t)UINT16_MAX)  // sentinel: no valid cell index
#define ENTITY_NONE ((entity_id_t)UINT16_MAX) // sentinel: no valid entity id

#define MATCH_FLASH_DURATION 0.4f

// match cache: per-cell bits telling which of its 4 edges are matched
#define EDGE_TOP     (1u << 0)
#define EDGE_RIGHT   (1u << 1)
#define EDGE_BOTTOM  (1u << 2)
#define EDGE_LEFT    (1u << 3)

API void printn(const char* format, ...) 
{
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  printf("\n");
  va_end(args);
}

#if LOG_LEVEL >= LOG_LEVEL_ERROR
#define log_error(...) printn("[error]: " __VA_ARGS__)
#else
#define log_error(...) (void)0
#endif

#if LOG_LEVEL >= LOG_LEVEL_WARN
#define log_warn(...) printn("[warn]: " __VA_ARGS__)
#else
#define log_warn(...) (void)0
#endif

#if LOG_LEVEL >= LOG_LEVEL_INFO
#define log_info(...) printn("" __VA_ARGS__)
#define log_info_inline(...) printf("" __VA_ARGS__)
#else
#define log_info(...) (void)0
#define log_info_inline(...) (void)0
#endif

#if DEBUG
  #if DEBUG_PRINT_LOC
    #define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
    #define debug_print(...) printn("[DEBUG %s:%d]: " __VA_ARGS__, __FILENAME__, __LINE__)
  #else
    #define debug_print(...) printn("[debug]: " __VA_ARGS__)
  #endif
#else
  // #define printn(...) (void)0 
  #define debug_print(...) (void)0  
#endif

#if LOG_LEVEL >= 4
#define log_debug(...) debug_print("[debug]: " __VA_ARGS__)
#else
#define log_debug(...) (void)0
#endif

#if HOT_RELOAD
#define hot_sync(block)            \
    do {                           \
      static int _hot_sync_fired;  \
      if (!_hot_sync_fired) {      \
        _hot_sync_fired = 1;       \
        do {                       \
          block;                   \
        } while (0);               \
      }                            \
    } while (0)
#else
#define hot_sync(block)
#endif

