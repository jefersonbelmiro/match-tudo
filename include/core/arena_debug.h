#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//  logger
// Redefine ARENA_LOG before including arena.h to redirect output:
//   file:  #define ARENA_LOG(...) fprintf(logfile, __VA_ARGS__)
//   buff:  #define ARENA_LOG(...) pos += snprintf(buf+pos, size-pos, __VA_ARGS__)

#ifndef ARENA_LOG
#define ARENA_LOG(...) printf(__VA_ARGS__)
#endif

#define arena__log(...) do { ARENA_LOG(__VA_ARGS__); ARENA_LOG("\n"); } while(0)

#ifndef ARENA_PRINT_MAX_ITEMS
#define ARENA_PRINT_MAX_ITEMS 5
#endif

#define ARENA_DEBUG_MAX   64
#define ARENA_MAX_CHILD   10
#define ARENA_TRACK_MAX 4096

typedef struct {
  size_t       offset;
  const char  *type_name;
  const char  *tag;
  const char  *file;
  int          line;
  uint32_t     count;
  uint16_t     elem_size;
  uint8_t      flags;
} arena_alloc_rec_t;

typedef enum {
  ARENA_VAL_U8,
  ARENA_VAL_U16,
  ARENA_VAL_U32,
  ARENA_VAL_U64,
  ARENA_VAL_FLOAT,
  ARENA_VAL_DOUBLE,
  ARENA_VAL_BOOL,
  ARENA_VAL_COLOR,
  ARENA_VAL_VECTOR2,
  ARENA_VAL_VECTOR3,
  ARENA_VAL_RECTANGLE,
  ARENA_VAL_UNKNOWN,
} arena_val_type_t;

typedef struct {
  arena_val_type_t type;
  const char      *type_name;
  uint32_t         index;
  uint32_t         count;
  uint16_t         elem_size;
  union {
    uint8_t       u8;
    uint16_t      u16;
    uint32_t      u32;
    uint64_t      u64;
    float         f32;
    double        f64;
    bool          b;
    struct { uint8_t r,g,b,a; }        color;
    struct { float x,y; }              v2;
    struct { float x,y,z; }            v3;
    struct { float x,y,w,h; }          rect;
  } as;
} arena_val_t;

// global table
static uint8_t g_init;
static uint32_t g_arena_debug_count;
static uint32_t g_child_ids[ARENA_DEBUG_MAX][ARENA_MAX_CHILD];
static uint32_t g_child_count[ARENA_DEBUG_MAX];
static uint64_t g_alloc_count[ARENA_DEBUG_MAX];
static uint64_t g_fb_count[ARENA_DEBUG_MAX];
static uint64_t g_bytes_alloced[ARENA_DEBUG_MAX];
static uint64_t g_bytes_fb[ARENA_DEBUG_MAX];
static uint64_t g_wasted[ARENA_DEBUG_MAX];
static size_t   g_peak[ARENA_DEBUG_MAX];
static size_t   g_cap[ARENA_DEBUG_MAX];
static size_t   g_off[ARENA_DEBUG_MAX];
static uintptr_t g_buf[ARENA_DEBUG_MAX];
static const char *g_tag[ARENA_DEBUG_MAX];
static arena_alloc_rec_t g_track[ARENA_DEBUG_MAX][ARENA_TRACK_MAX];
static uint32_t g_track_count[ARENA_DEBUG_MAX];

static uint32_t arena_debug_create(size_t cap, uintptr_t buf, const char *tag)
{
  if (!g_init) { memset(g_child_count, 0, sizeof(g_child_count)); g_init = 1; }
  uint32_t id = g_arena_debug_count++;
  g_cap[id]  = cap;
  g_buf[id]  = buf;
  g_tag[id]  = tag;
  g_off[id]  = 0;
  g_track_count[id] = 0;
  return id;
}

static inline uint32_t arena_debug_sub_create(uint32_t parent, size_t cap,
                                        uintptr_t buf, const char *tag)
{
  uint32_t id = arena_debug_create(cap, buf, tag);
  assert(g_child_count[parent] < ARENA_MAX_CHILD);
  if (g_child_count[parent] < ARENA_MAX_CHILD)
    g_child_ids[parent][g_child_count[parent]++] = id;
  return id;
}

static inline void arena_debug_alloc_stats(uint32_t id, size_t sz, size_t pad,
                                     size_t new_off)
{
  g_alloc_count[id]++;
  g_bytes_alloced[id] += sz;
  g_wasted[id] += pad;
  g_off[id] = new_off;
  if (new_off > g_peak[id]) g_peak[id] = new_off;
}

static inline void arena_debug_fallback(uint32_t id, size_t sz)
{
  g_fb_count[id]++;
  g_bytes_fb[id] += sz;
}

static inline void arena_debug_alloc_track(uint32_t id, const char *type,
                                     uint32_t count, uint16_t esz,
                                     int zero, const char *tag,
                                     const char *file, int line,
                                     size_t offset)
{
  if (g_track_count[id] >= ARENA_TRACK_MAX) return;
  uint32_t i = g_track_count[id]++;
  g_track[id][i].offset    = offset;
  g_track[id][i].type_name = type;
  g_track[id][i].tag       = tag;
  g_track[id][i].file      = file;
  g_track[id][i].line      = line;
  g_track[id][i].count     = count;
  g_track[id][i].elem_size = esz;
  g_track[id][i].flags     = zero ? 1u : 0u;
}

static inline void arena_debug_reset(uint32_t id)
{
  g_off[id] = 0;
  g_track_count[id] = 0;
  for (uint32_t i = 0; i < g_child_count[id]; i++)
    arena_debug_reset(g_child_ids[id][i]);
  g_child_count[id] = 0;
}

static inline void arena_debug_destroy(uint32_t id)
{
  // no-op: global slots are never freed
  (void)id;
}

// value extraction

static arena_val_type_t arena_alloc_val(const arena_alloc_rec_t *r,
                                         const void *data,
                                         uint32_t index,
                                         arena_val_t *out)
{
  out->type_name = r->type_name;
  out->index     = index;
  out->count     = r->count;
  out->elem_size = r->elem_size;

  if (strcmp(r->type_name, "float") == 0) {
    out->type = ARENA_VAL_FLOAT; out->as.f32 = ((const float *)data)[index];
  } else if (strcmp(r->type_name, "double") == 0) {
    out->type = ARENA_VAL_DOUBLE; out->as.f64 = ((const double *)data)[index];
  } else if (strcmp(r->type_name, "bool") == 0 || strcmp(r->type_name, "_Bool") == 0) {
    out->type = ARENA_VAL_BOOL; out->as.b = ((const bool *)data)[index];
  } else if (strcmp(r->type_name, "u8") == 0 && r->elem_size == 1) {
    out->type = ARENA_VAL_U8; out->as.u8 = ((const uint8_t *)data)[index];
  } else if ((strcmp(r->type_name, "u16") == 0 || strcmp(r->type_name, "int") == 0) && r->elem_size == 2) {
    out->type = ARENA_VAL_U16; out->as.u16 = ((const uint16_t *)data)[index];
  } else if (strcmp(r->type_name, "u32") == 0 && r->elem_size == 4) {
    out->type = ARENA_VAL_U32; out->as.u32 = ((const uint32_t *)data)[index];
  } else if (strcmp(r->type_name, "u64") == 0 && r->elem_size == 8) {
    out->type = ARENA_VAL_U64; out->as.u64 = ((const uint64_t *)data)[index];
  } else if (strcmp(r->type_name, "entity_id_t") == 0 && r->elem_size == 2) {
    out->type = ARENA_VAL_U16; out->as.u16 = ((const uint16_t *)data)[index];
  } else if (strcmp(r->type_name, "grid_idx_t") == 0 && r->elem_size == 2) {
    out->type = ARENA_VAL_U16; out->as.u16 = ((const uint16_t *)data)[index];
  } else if (strcmp(r->type_name, "Color") == 0 && r->elem_size == 4) {
    out->type = ARENA_VAL_COLOR;
    const uint8_t *c = &((const uint8_t *)data)[index * 4];
    out->as.color.r = c[0]; out->as.color.g = c[1];
    out->as.color.b = c[2]; out->as.color.a = c[3];
  } else if (strcmp(r->type_name, "Vector2") == 0 && r->elem_size == 8) {
    out->type = ARENA_VAL_VECTOR2;
    const float *v = &((const float *)data)[index * 2];
    out->as.v2.x = v[0]; out->as.v2.y = v[1];
  } else if (strcmp(r->type_name, "Vector3") == 0 && r->elem_size == 12) {
    out->type = ARENA_VAL_VECTOR3;
    const float *v = &((const float *)data)[index * 3];
    out->as.v3.x = v[0]; out->as.v3.y = v[1]; out->as.v3.z = v[2];
  } else if (strcmp(r->type_name, "Rectangle") == 0 && r->elem_size == 16) {
    out->type = ARENA_VAL_RECTANGLE;
    const float *v = &((const float *)data)[index * 4];
    out->as.rect.x = v[0]; out->as.rect.y = v[1];
    out->as.rect.w = v[2]; out->as.rect.h = v[3];
  } else {
    out->type = ARENA_VAL_UNKNOWN;
  }
  return out->type;
}

static int arena_snprint_val(char *buf, size_t sz, const arena_val_t *v)
{
  switch (v->type) {
    case ARENA_VAL_U8:     return snprintf(buf,sz,"%u",v->as.u8);
    case ARENA_VAL_U16:    return snprintf(buf,sz,"%u",v->as.u16);
    case ARENA_VAL_U32:    return snprintf(buf,sz,"%u",v->as.u32);
    case ARENA_VAL_U64:    return snprintf(buf,sz,"%llu",(unsigned long long)v->as.u64);
    case ARENA_VAL_FLOAT:  return snprintf(buf,sz,"%g",v->as.f32);
    case ARENA_VAL_DOUBLE: return snprintf(buf,sz,"%g",v->as.f64);
    case ARENA_VAL_BOOL:   return snprintf(buf,sz,"%s",v->as.b?"T":".");
    case ARENA_VAL_COLOR:  return snprintf(buf,sz,"(%u,%u,%u,%u)",
                                            v->as.color.r,v->as.color.g,
                                            v->as.color.b,v->as.color.a);
    case ARENA_VAL_VECTOR2: return snprintf(buf,sz,"(%g,%g)",v->as.v2.x,v->as.v2.y);
    case ARENA_VAL_VECTOR3: return snprintf(buf,sz,"(%g,%g,%g)",v->as.v3.x,v->as.v3.y,v->as.v3.z);
    case ARENA_VAL_RECTANGLE: return snprintf(buf,sz,"(%g,%g,%g,%g)",
                                              v->as.rect.x,v->as.rect.y,
                                              v->as.rect.w,v->as.rect.h);
    default: return snprintf(buf,sz,"?");
  }
}

static inline void arena__print_vals_line(const char *pfx, const char *buf)
{
  arena__log("        %s: %s", pfx, buf);
}

static inline void arena__print_total(uint32_t count, uint16_t esz)
{
  size_t total = (size_t)count * esz;
  arena__log("        total: %u items x %u B = %zu B = %.2f KB",
         count, esz, total, (double)total / 1024.0);
}

static inline void arena__print_values(const arena_alloc_rec_t *r, const void *data)
{
  char buf[1024] = {0};
  int pos = 0;

  if (strcmp(r->type_name, "u8") == 0 && r->elem_size == 1) {
    uint32_t show = r->count < 48 ? r->count : 48;
    for (uint32_t j = 0; j < show && pos < (int)sizeof(buf)-6; j++)
      pos += snprintf(buf+pos,sizeof(buf)-pos,"%02x ",((const uint8_t*)data)[j]);
    if (r->count > 48) snprintf(buf+pos,sizeof(buf)-pos,"...");
    arena__print_vals_line("hex", buf);
  } else if (strcmp(r->type_name,"char")==0 && r->elem_size==1 && r->count>1) {
    const char *s = data;
    size_t slen = 0;
    while (slen < r->count && s[slen]) slen++;
    size_t show = slen < 128 ? slen : 128;
    snprintf(buf,sizeof(buf), show<slen ? "\"%.*s...\"" : "\"%.*s\"",(int)show,s);
    arena__print_vals_line("str", buf);
  } else if (strcmp(r->type_name,"char")==0 && r->elem_size==1) {
    unsigned char c = ((const unsigned char*)data)[0];
    char ch = c >= 32 ? c : '.';
    snprintf(buf,sizeof(buf),"%d (0x%02x '%c')",c,c,ch);
    arena__print_vals_line("val", buf);
  } else {
    uint32_t max = r->count < ARENA_PRINT_MAX_ITEMS ? r->count : ARENA_PRINT_MAX_ITEMS;
    int nv = 0;
    for (uint32_t j = 0; j < max; j++) {
      arena_val_t v;
      if (arena_alloc_val(r,data,j,&v) == ARENA_VAL_UNKNOWN) continue;
      int n = arena_snprint_val(buf+pos,sizeof(buf)-pos,&v);
      if (n<0 || pos+n+3>=(int)sizeof(buf)) break;
      pos += n; buf[pos++]=' '; buf[pos++]=' '; buf[pos]='\0';
      nv++;
    }
    if (r->count > max && pos+5 < (int)sizeof(buf))
      snprintf(buf+pos,sizeof(buf)-pos,"...");
    if (nv > 0) arena__print_vals_line("vals", buf);
  }
  arena__print_total(r->count, r->elem_size);
}

static inline void arena__stats_block(uint32_t id, uint32_t depth)
{
  char ind[32] = {0};
  for (uint32_t i = 0; i < depth && i < 15; i++)
    ind[i*2] = ' ';
  double pct = g_cap[id] ? 100.0*(double)g_off[id]/(double)g_cap[id] : 0;

  arena__log("%s------------------------------------------", ind);
  arena__log("%s  %s  (id=%u)", ind, g_tag[id] ? g_tag[id] : "untagged", id);
  arena__log("%s------------------------------------------", ind);
  arena__log("%s  capacity   %10zu B  (%7.2f KB)", ind, g_cap[id],
         (double)g_cap[id]/1024.0);
  arena__log("%s  used       %10zu B  (%7.2f KB)  %5.1f%%", ind, g_off[id],
         (double)g_off[id]/1024.0, pct);
  arena__log("%s  peak       %10zu B  (%7.2f KB)", ind, g_peak[id],
         (double)g_peak[id]/1024.0);
  arena__log("%s  alloc      %10llu  (%7.2f KB)", ind,
         (unsigned long long)g_bytes_alloced[id],
         (double)g_bytes_alloced[id]/1024.0);
  arena__log("%s  calls      %10llu", ind, (unsigned long long)g_alloc_count[id]);
  arena__log("%s  wasted     %10llu B  (%7.2f KB)", ind,
         (unsigned long long)g_wasted[id],
         (double)g_wasted[id]/1024.0);
  if (g_fb_count[id])
    arena__log("%s  fallback   %10llu  (%7.2f KB)", ind,
           (unsigned long long)g_fb_count[id],
           (double)g_bytes_fb[id]/1024.0);

  for (uint32_t i = 0; i < g_child_count[id]; i++)
    arena__stats_block(g_child_ids[id][i], depth + 1);
}

static inline void arena_print_stats(uint32_t id)
{
  arena__stats_block(id, 0);
}

static inline void arena_print_track(uint32_t id, bool show_values)
{
  if (g_track_count[id] > 0) {
    arena__log("------------------------------------------");
    arena__log("  %s  -  %u allocation(s)",
           g_tag[id] ? g_tag[id] : "untagged",
           (unsigned)g_track_count[id]);
    arena__log("------------------------------------------");

    for (uint32_t i = 0; i < g_track_count[id]; i++) {
      const arena_alloc_rec_t *r = &g_track[id][i];
      if (r->tag)
        arena__log("  [#%u]  %s[%u]  tag:%s  %s:%d",
               i, r->type_name, r->count, r->tag, r->file, r->line);
      else
        arena__log("  [#%u]  %s[%u]  %s:%d",
               i, r->type_name, r->count, r->file, r->line);
      arena__log("        count: %-6u  elem: %-3u B  offset: %zu",
             r->count, r->elem_size, r->offset);

      if (show_values) {
        const void *data = (const void*)(g_buf[id] + r->offset);
        arena__print_values(r, data);
      }
    }
  }

  for (uint32_t i = 0; i < g_child_count[id]; i++)
    arena_print_track(g_child_ids[id][i], show_values);
}
