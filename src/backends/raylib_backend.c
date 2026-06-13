#include "../../include/backend.h"

#include "raylib.h"

#include <lilc/alloc.h>
#include <string.h>

typedef struct {
  i32 width;
  i32 height;
  const char *title;
  bool resizeable;
} RaylibWindow;

typedef struct {
  RaylibWindow window;

  Bump ui_elem_bump;
  Allocator ui_elem_allocator;
  Bump ui_state_bump;
  Allocator ui_state_allocator;
} RaylibBackend;

static inline Color rl_color(Blank_Color color) {
  return (Color){
      .r = blank_color_red(color),
      .g = blank_color_green(color),
      .b = blank_color_blue(color),
      .a = blank_color_alpha(color),
  };
}

static void rl_window_init(Blank_Backend *backend,
                           const Blank_InitState *init_state) {
  RaylibBackend *rl_backend = backend->backend_context;
  rl_backend->window.title = init_state->title;
  rl_backend->window.width = init_state->width;
  rl_backend->window.height = init_state->height;
  rl_backend->window.resizeable = init_state->resizeable;

  InitWindow(init_state->width, init_state->height, init_state->title);
  if (init_state->resizeable) {
    SetWindowState(FLAG_WINDOW_RESIZABLE);
  }
}

static void rl_screen_clear(Blank_Backend *backend, Blank_Color color) {
  ClearBackground(rl_color(color));
}

void raylib_backend_init(Blank_Backend *backend) {
  RaylibBackend _rl_backend = {0};
  bump_init(&_rl_backend.ui_elem_bump, 32000);
  bump_init(&_rl_backend.ui_state_bump, 32000);

  bump_allocator_init(&_rl_backend.ui_state_allocator,
                      &_rl_backend.ui_state_bump);
  bump_allocator_init(&_rl_backend.ui_elem_allocator,
                      &_rl_backend.ui_elem_bump);

  backend->backend_context = heap_alloc(sizeof(RaylibBackend));
  memcpy(backend->backend_context, &_rl_backend, sizeof(RaylibBackend));

  RaylibBackend *rl_backend = backend->backend_context;

  backend->ui_elem_allocator = &rl_backend->ui_elem_allocator;
  backend->ui_state_allocator = &rl_backend->ui_state_allocator;

  backend->clear_screen_func = rl_screen_clear;
  backend->window_init_func = rl_window_init;
  backend->clear_screen_func = rl_screen_clear;
}
