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

  _amount_mouse_button,
} Blank_MouseButton;

typedef enum {
  BLANK_KEY_NULL,
  // Letters
  BLANK_KEY_A, BLANK_KEY_B, BLANK_KEY_C, BLANK_KEY_D, BLANK_KEY_E,
  BLANK_KEY_F, BLANK_KEY_G, BLANK_KEY_H, BLANK_KEY_I, BLANK_KEY_J,
  BLANK_KEY_K, BLANK_KEY_L, BLANK_KEY_M, BLANK_KEY_N, BLANK_KEY_O,
  BLANK_KEY_P, BLANK_KEY_Q, BLANK_KEY_R, BLANK_KEY_S, BLANK_KEY_T,
  BLANK_KEY_U, BLANK_KEY_V, BLANK_KEY_W, BLANK_KEY_X, BLANK_KEY_Y,
  BLANK_KEY_Z,

  // Numbers (top row)
  BLANK_KEY_0, BLANK_KEY_1, BLANK_KEY_2, BLANK_KEY_3, BLANK_KEY_4,
  BLANK_KEY_5, BLANK_KEY_6, BLANK_KEY_7, BLANK_KEY_8, BLANK_KEY_9,

  // Function keys
  BLANK_KEY_F1,  BLANK_KEY_F2,  BLANK_KEY_F3,  BLANK_KEY_F4,
  BLANK_KEY_F5,  BLANK_KEY_F6,  BLANK_KEY_F7,  BLANK_KEY_F8,
  BLANK_KEY_F9,  BLANK_KEY_F10, BLANK_KEY_F11, BLANK_KEY_F12,

  // Arrow keys
  BLANK_KEY_UP, BLANK_KEY_DOWN, BLANK_KEY_LEFT, BLANK_KEY_RIGHT,

  // Modifier keys
  BLANK_KEY_LEFT_SHIFT,  BLANK_KEY_RIGHT_SHIFT,
  BLANK_KEY_LEFT_CTRL,   BLANK_KEY_RIGHT_CTRL,
  BLANK_KEY_LEFT_ALT,    BLANK_KEY_RIGHT_ALT,
  BLANK_KEY_LEFT_SUPER,  BLANK_KEY_RIGHT_SUPER,

  // Control keys
  BLANK_KEY_ESCAPE,
  BLANK_KEY_ENTER,
  BLANK_KEY_TAB,
  BLANK_KEY_BACKSPACE,
  BLANK_KEY_DELETE,
  BLANK_KEY_INSERT,
  BLANK_KEY_HOME,
  BLANK_KEY_END,
  BLANK_KEY_PAGE_UP,
  BLANK_KEY_PAGE_DOWN,
  BLANK_KEY_CAPS_LOCK,
  BLANK_KEY_PRINT_SCREEN,
  BLANK_KEY_SCROLL_LOCK,
  BLANK_KEY_PAUSE,
  BLANK_KEY_SPACE,

  // Punctuation / symbols
  BLANK_KEY_APOSTROPHE,    // '
  BLANK_KEY_COMMA,         // ,
  BLANK_KEY_MINUS,         // -
  BLANK_KEY_PERIOD,        // .
  BLANK_KEY_SLASH,         // /
  BLANK_KEY_SEMICOLON,     // ;
  BLANK_KEY_EQUAL,         // =
  BLANK_KEY_LEFT_BRACKET,  // [
  BLANK_KEY_BACKSLASH,     // '\'
  BLANK_KEY_RIGHT_BRACKET, // ]
  BLANK_KEY_GRAVE,         // `

  // Numpad
  BLANK_KEY_NUM_LOCK,
  BLANK_KEY_NUMPAD_0, BLANK_KEY_NUMPAD_1, BLANK_KEY_NUMPAD_2,
  BLANK_KEY_NUMPAD_3, BLANK_KEY_NUMPAD_4, BLANK_KEY_NUMPAD_5,
  BLANK_KEY_NUMPAD_6, BLANK_KEY_NUMPAD_7, BLANK_KEY_NUMPAD_8,
  BLANK_KEY_NUMPAD_9,
  BLANK_KEY_NUMPAD_ADD,
  BLANK_KEY_NUMPAD_SUBTRACT,
  BLANK_KEY_NUMPAD_MULTIPLY,
  BLANK_KEY_NUMPAD_DIVIDE,
  BLANK_KEY_NUMPAD_DECIMAL,
  BLANK_KEY_NUMPAD_ENTER,

  _amount_blank_keys,
} Blank_Key;

typedef struct {
  i32 x;
  i32 y;
} Blank_Pos;

typedef struct {
  i32 width;
  i32 height;
} Blank_Size;

/* Misc functions*/

void blank_wait(i32 miliseconds);

// Blank_Color is the 32 bit rgba color representation
// used by Blank. The individual color components can be
// obtained using the corresponding red, green, blue, alpha
// functions
typedef u32 Blank_Color;

/* Color functions */

#define blank_color_make(red, green, blue, alpha)                              \
  (Blank_Color)((red) << 24) | ((green) << 16) | ((blue) << 8) | (alpha)

u8 blank_color_red(Blank_Color color);

u8 blank_color_green(Blank_Color color);

u8 blank_color_blue(Blank_Color color);

u8 blank_color_alpha(Blank_Color color);

#define BLANK_WHITE blank_color_make(255, 255, 255, 255)

#define BLANK_COZY_WHITE blank_color_make(239, 232, 222, 255)

#define BLANK_BLACK blank_color_make(0, 0, 0, 255)

#define BLANK_COZY_BLACK blank_color_make(16, 23, 33, 255)

#define BLANK_RED blank_color_make(235, 25, 34, 255)

#define BLANK_CLEAR_SCREEN_COMMAND 0
#define BLANK_RENDER_CUSTOM_COMMAND 1
#define BLANK_RENDER_TEXT_COMMAND 2
#define BLANK_RENDER_RECTANGLE_COMMAND 3
#define BLANK_RENDER_IMAGE_COMMAND 4

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

struct blank_cmd_render_img {
  const char *img_path;
  
  Blank_Color tint_color;
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
    struct blank_cmd_render_img cmd_ri;
  } cmd;
} Blank_RenderCommand;

typedef struct {
  Blank_Backend *backend;
  Blank_RenderCommand **render_commands;
} Blank_RenderContext;

typedef struct {
  Blank_Backend *backend;
} Blank_Context;

typedef Blank_Size (*MinUiElemSizeFunc)(const struct blank_ui_elem *elem, Blank_Context ctx);

typedef void (*RenderUiElemFunc)(const struct blank_render_ui_elem *elem,
                                 Blank_RenderContext render_ctx);

typedef void (*DeinitUiElemFunc)(struct blank_ui_elem *elem);

typedef struct blank_ui_elem {
  // Unique identifier that can be used to identify and group elements
  u64 uid;

  size_t elem_kind;
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

typedef Blank_Size (*LayoutMinSizeElemsFunc)(
    const struct blank_ui_layout *layout, Blank_UiElement *elems, Blank_Context ctx);

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
                 void (*backend_deinit_func)(Blank_Backend *backend),
                 void (*app_run_func)(struct blank_ui_state *state));

/* Initialization-State config functions */

void blank_window_title(Blank_InitState *init_state, const char *title);

void blank_window_size(Blank_InitState *init_state, i32 width, i32 height);

void blank_window_resizeable(Blank_InitState *init_state);

/* UI-State functions */

bool blank_window_closed(Blank_UiState *state);

bool blank_window_resized(Blank_UiState *state);

bool blank_elem_clicked(Blank_UiState *state, Blank_MouseButton mouse_button,
                        u64 *clicked_elem_uid);

/* UI-Building functions */

void blank_ui_begin(Blank_UiState *state, Blank_Color bg_color, Blank_UiLayout initial_layout);

void blank_ui_end(void);

void blank_ui_submit(Blank_UiElement ui_elem);
