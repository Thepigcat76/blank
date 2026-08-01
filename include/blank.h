#pragma once

#include "lilc/numbers.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct blank_backend Blank_Backend;
struct blank_ui_elem;
struct blank_render_ui_elem;
struct blank_ui_layout;

typedef enum {
  BLANK_MOUSE_BUTTON_LEFT,
  BLANK_MOUSE_BUTTON_MIDDLE,
  BLANK_MOUSE_BUTTON_RIGHT,
} Blank_MouseButton;

typedef struct {
  i32 width;
  i32 height;
} Blank_Size;

struct blank_ui_elem;

typedef void (*OnClickFunc)(struct blank_ui_elem *elem, Blank_MouseButton mouse_button);

typedef u32 Blank_Color;

Blank_Color blank_color_make(u8 red, u8 green, u8 blue, u8 alpha);

u8 blank_color_red(Blank_Color color);

u8 blank_color_green(Blank_Color color);

u8 blank_color_blue(Blank_Color color);

u8 blank_color_alpha(Blank_Color color);

void blank_wait(i32 miliseconds);

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

typedef struct blank_render_cmd {
  u32 cmd_type;
  union {
    struct blank_cmd_clear_screen cmd_cs;
    struct blank_cmd_render_rectangle cmd_rr;
    struct blank_cmd_render_text cmd_rt;
  } cmd;
} Blank_RenderCommand;

typedef struct {
  Blank_Backend *backend;
  Blank_RenderCommand **render_commands;
} Blank_RenderContext;

typedef Blank_Size (*MinUiElemSizeFunc)(const struct blank_ui_elem *elem);

typedef void (*RenderUiElemFunc)(const struct blank_render_ui_elem *elem,
                                 Blank_RenderContext render_ctx);

typedef void (*DeinitUiElemFunc)(struct blank_ui_elem *elem);

typedef struct blank_ui_elem {
  size_t elem_type;
  void *args;

  MinUiElemSizeFunc min_size_func;
  RenderUiElemFunc render_func;

  DeinitUiElemFunc deinit_func;
} Blank_UiElement;

typedef struct blank_render_ui_elem {
  Blank_UiElement elem;

  i32 x;
  i32 y;
  i32 width;
  i32 height;
} Blank_RenderableUiElement;

typedef struct {
  i32 container_x;
  i32 container_y;
  // Width and height of current container,
  // If no container-like ui element was
  // created, window size is used here
  i32 container_width;
  i32 container_height;

  Blank_Backend *backend;
} Blank_LayoutContext;

typedef enum {
  BLANK_VERTICAL,
  BLANK_HORIZONTAL,
} Blank_LayoutOrientation;

typedef void (*LayoutRearrangeElemsFunc)(
    const struct blank_ui_layout *layout, Blank_UiElement **elems,
    Blank_RenderableUiElement **renderable_elems, Blank_LayoutContext context);

typedef Blank_Size (*LayoutMinSizeElemsFunc)(const struct blank_ui_layout *layout,
                                       Blank_UiElement *elems);

typedef struct blank_ui_layout {
  LayoutRearrangeElemsFunc rearrange_elems_func;
  LayoutMinSizeElemsFunc min_size_elems_func;

  u64 layout_id;
  union {
    void *custom;
    struct {
      u32 padding;
      Blank_LayoutOrientation orientation;
    } linear;
  } layout_data;
} Blank_UiLayout;

// Blank_InitState has config values used for the
// initialization of the ui window.
typedef struct blank_init_state {
  const char *title;
  i32 width;
  i32 height;
  bool resizeable;
} Blank_InitState;

// Blank_UiState stores the current state of the ui
// Its state can be accessed using the UI-State functions.
// Note: The type is opaque since its shared among threads
// and needs to be thread safe.
typedef struct blank_ui_state Blank_UiState;

/* Start function */

// Starts the app and render thread based on a given initialization state.
void blank_start(Blank_InitState state,
                 void (*backend_init_func)(Blank_Backend *backend),
                 void (*app_run_func)(struct blank_ui_state *state));

/* Initialization-State config functions */

void blank_window_title(Blank_InitState *init_state, const char *title);

void blank_window_size(Blank_InitState *init_state, i32 width, i32 height);

void blank_window_resizeable(Blank_InitState *init_state);

/* UI-State functions */

bool blank_window_closed(Blank_UiState *state);

bool blank_window_resized(Blank_UiState *state);

/* UI-Building functions */

void blank_ui_begin(Blank_UiState *state, Blank_UiLayout initial_layout);

void blank_ui_end(void);

void blank_ui_submit(Blank_UiElement ui_elem);
