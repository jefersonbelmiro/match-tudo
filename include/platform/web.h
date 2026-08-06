#pragma once

#if defined(PLATFORM_WEB)

#include <emscripten/emscripten.h>
#include <emscripten/em_js.h>

static volatile bool g_web_is_mobile = false;

// set to 1 by the js syncfs callback when the indexeddb populate is done
static volatile int g_idbfs_ready = 0;

// web_set_idbfs_ptr() passes the address of g_idbfs_ready so the js
// syncfs callback can set it to 1 when the IndexedDB populate finishes
EM_JS(void, web_set_idbfs_ptr, (volatile int* ptr), {
  Module._g_idbfs_ready_ptr = ptr;
});

EM_JS(void, web_idbfs_mount, (), {
  FS.mkdir('/data');
  FS.mount(IDBFS, { autoPersist: true }, '/data');
  FS.syncfs(true, function(err) {
    if (err) console.error('IDBFS syncfs error:', err);
    if (Module._g_idbfs_ready_ptr) {
      setValue(Module._g_idbfs_ready_ptr, 1, 'i32');
    }
  });
});

EM_JS(int, web_is_mobile_js, (), {
  return (navigator.maxTouchPoints > 0 || /Mobi|Android|iPhone|iPad|iPod/i.test(navigator.userAgent)) ? 1 : 0;
});

API void platform_web_init()
{
  web_set_idbfs_ptr(&g_idbfs_ready);
  web_idbfs_mount();
  g_web_is_mobile = web_is_mobile_js();
}

API bool platform_web_is_ready()
{
  return g_idbfs_ready;
}

API void platform_web_syncfs()
{
  EM_ASM({
    FS.syncfs(/*populate=*/false, function(err) {
      if (err) console.log("syncfs error: ", err);
    });
  });
}

API void platform_web_remove_loading_overlay()
{
  EM_ASM({
    const loadElement = document.getElementById("loading-overlay");
    loadElement.style.display = 'none';
    loadElement.parentElement.removeChild(loadElement);
  });
}

API bool platform_web_is_mobile(void)
{
  return g_web_is_mobile;
}

#endif

