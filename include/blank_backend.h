#pragma once

#include "blank.h"
#include "lilc/alloc.h"
#include "lilc/numbers.h"
#include <stdbool.h>

typedef enum {
  BACKEND_INIT_INITIAL,
  BACKEND_INIT_APP_THREAD,
  BACKEND_INIT_RENDER_THREAD,
} Blank_BackendInitStage;

struct blank_backend {
  // -- Init functions --
  // Init the window based on the configured initial state. Called on render
  // thread
  void (*window_init_func)(struct blank_backend *,
                           const Blank_InitState *init_state);
  // -- Render functions --
  void (*render_cmds_func)(struct blank_backend *,
                           Blank_RenderCommand *render_command);
  // -- Screen functions --
  i32 (*screen_width_func)(struct blank_backend *);
  i32 (*screen_height_func)(struct blank_backend *);
  // -- Window functions --
  bool (*window_should_close_func)(struct blank_backend *);
  // -- Text functions --
  i32 (*text_width_func)(struct blank_backend *, const char *text,
                         size_t font_size);

  // Allocator for ui_elements. WILL BE CALLED FROM MULTIPLE THREADS
  Allocator *ui_elem_allocator;
  // Allocator for temporary ui_state. Thread local
  Allocator *ui_state_allocator;

  // Custom context depending on the backend
  void *backend_context;
};
