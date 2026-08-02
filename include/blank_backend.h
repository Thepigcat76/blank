#pragma once

#include "blank.h"
#include "blank/ui_elems.h"
#include "lilc/alloc.h"
#include "lilc/numbers.h"
#include <stdbool.h>
#include <pthread.h>

typedef enum {
  BACKEND_INIT_INITIAL,
  BACKEND_INIT_APP_THREAD,
  BACKEND_INIT_RENDER_THREAD,
} Blank_BackendInitStage;

typedef struct {
  const char *path;

  i32 width;
  i32 height;
} Blank_ImageMetadata;

struct blank_backend {
  // -- Init functions --
  // Init the window based on the configured initial state. Called on render
  // thread
  void (*window_init_func)(Blank_Backend *, const Blank_InitState *init_state);
  // -- Render functions --
  void (*render_cmds_func)(Blank_Backend *,
                           Blank_RenderCommand *render_command);
  // -- Screen functions --
  Blank_Size (*screen_size_func)(Blank_Backend *);
  // -- Window functions --
  bool (*window_should_close_func)(Blank_Backend *);
  // -- Text functions --
  i32 (*text_width_func)(Blank_Backend *, const char *text, size_t font_size);
  // -- Mouse functions --
  Blank_Pos (*mouse_pos_func)(Blank_Backend *);
  void (*mouse_button_down_func)(Blank_Backend *, bool buttons[_amount_mouse_button]);
  void (*mouse_button_pressed_func)(Blank_Backend *, bool buttons[_amount_mouse_button]);
  void (*mouse_button_released_func)(Blank_Backend *, bool buttons[_amount_mouse_button]);
  // -- Image functions --
  bool (*img_metadata_func)(Blank_Backend *, const char *img_path, Blank_ImageMetadata *metadata);

  // Allocator for ui_elements. WILL BE CALLED FROM MULTIPLE THREADS
  Allocator *ui_elem_allocator;
  pthread_mutex_t ui_elem_alloc_mutex;
  // Allocator for renderable ui elements. Thread-local
  Allocator *ui_render_elem_allocator;
  // Allocator for render commands. Thread-local
  Allocator *ui_render_cmd_allocator;

  // Custom context depending on the backend
  void *backend_context;
};
