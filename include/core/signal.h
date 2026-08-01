#pragma once

#include "core/defs.h"
#include <assert.h>

#ifndef SIGNAL_MAX_LISTENERS
#define SIGNAL_MAX_LISTENERS 4
#endif

// ---------------------------------------------------------------------------
// signal - single listener
// ---------------------------------------------------------------------------

typedef void (*signal_fn)(void *listener, u8 type, const void *payload);

typedef struct {
  signal_fn  fn;
  void      *listener;
} signal_t;

API void signal_connect(signal_t *s, signal_fn fn, void *listener)
{
  s->fn       = fn;
  s->listener = listener;
}

API void signal_disconnect(signal_t *s)
{
  s->fn       = NULL;
  s->listener = NULL;
}

API bool signal_connected(const signal_t *s)
{
  return s->fn != NULL;
}

API void signal_emit(signal_t *s, u8 type, const void *payload)
{
  if (s->fn) s->fn(s->listener, type, payload);
}

// ---------------------------------------------------------------------------
// signal bus - multiple listeners
// ---------------------------------------------------------------------------

typedef struct {
  signal_fn  fn;
  void      *listener;
} signal_listener_t;

typedef struct {
  signal_listener_t listeners[SIGNAL_MAX_LISTENERS];
  u8                count;
} signal_bus_t;

API void signal_bus_connect(signal_bus_t *bus, signal_fn fn, void *listener)
{
  assert(bus->count < SIGNAL_MAX_LISTENERS);
  bus->listeners[bus->count++] = (signal_listener_t){ fn, listener };
}

API void signal_bus_disconnect_all(signal_bus_t *bus)
{
  bus->count = 0;
}

API bool signal_bus_has_listeners(const signal_bus_t *bus)
{
  return bus->count > 0;
}

API void signal_bus_emit(signal_bus_t *bus, u8 type, const void *payload)
{
  for (u8 i = 0; i < bus->count; i++) {
    bus->listeners[i].fn(bus->listeners[i].listener, type, payload);
  }
}
