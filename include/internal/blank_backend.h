#pragma once

#include "lilc/numbers.h"
#include "lilc/alloc.h"
#include <stdbool.h>
#include "blank_shared.h"
#include "blank_init.h"

typedef enum {
  BACKEND_INIT_INITIAL,
  BACKEND_INIT_APP_THREAD,
  BACKEND_INIT_RENDER_THREAD,
} Blank_BackendInitStage;

#define BLANK_CLEAR_SCREEN_COMMAND 0
#define BLANK_RENDER_CUSTOM_COMMAND 1
#define BLANK_RENDER_TEXT_COMMAND 2
#define BLANK_RENDER_RECTANGLE_COMMAND 3

struct blank_cmd_clear_screen {
  Blank_Color color;
};

struct blank_cmd_render_text {
  Blank_Color color;
  i32 x;
  i32 y;

  size_t font_size;
  const char *text;
};

struct blank_cmd_render_rectangle {
  Blank_Color color;
  i32 x;
  i32 y;
  
  i32 width;
  i32 height;
};

typedef struct {
  u32 cmd_type;
  union {
    struct blank_cmd_clear_screen cmd_cs;
    struct blank_cmd_render_rectangle cmd_rr;
    struct blank_cmd_render_text cmd_rt;
  } cmd;
} Blank_RenderCommand;

typedef struct blank_backend {
  // -- Init functions --
  // Init the window based on the configured initial state. Called on render thread
  void (*window_init_func)(struct blank_backend *, const Blank_InitState *init_state);
  // -- Render functions --
  void (*render_cmds_func)(struct blank_backend *, Blank_RenderCommand *render_command);
  // -- Screen functions --
  i32 (*screen_width_func)(struct blank_backend *);
  i32 (*screen_height_func)(struct blank_backend *);
  // -- Window functions --
  bool (*window_should_close_func)(struct blank_backend *);
  // -- Text functions --
  i32 (*text_width_func)(struct blank_backend *, const char *text, size_t font_size);

  // Allocator for ui_elements. WILL BE CALLED FROM MULTIPLE THREADS
  Allocator *ui_elem_allocator;
  // Allocator for temporary ui_state. Thread local
  Allocator *ui_state_allocator;

  // Custom context depending on the backend
  void *backend_context;
  
} Blank_Backend;
