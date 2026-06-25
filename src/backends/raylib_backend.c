#include "../../include/internal/blank_backend.h"

#include "raylib.h"

#include <lilc/alloc.h>
#include <lilc/array.h>
#include <lilc/log.h>
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
  RaylibWindow *rl_window = &rl_backend->window;

  rl_backend->window.title = init_state->title;
  rl_backend->window.width = init_state->width;
  rl_backend->window.height = init_state->height;
  rl_backend->window.resizeable = init_state->resizeable;

  if (rl_window->resizeable) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  }
  InitWindow(rl_window->width, rl_window->height, rl_window->title);
}

static void rl_render_cmds(Blank_Backend *backend, Blank_RenderCommand *cmds) {
  BeginDrawing();

  Blank_RenderCommand *cmd;
  array_foreach(cmds, cmd) {
    switch (cmd->cmd_type) {
    case BLANK_CLEAR_SCREEN_COMMAND: {
      struct blank_cmd_clear_screen cmd_cs = cmd->cmd.cmd_cs;
      ClearBackground(rl_color(cmd_cs.color));
    } break;
    case BLANK_RENDER_CUSTOM_COMMAND: {

    } break;
    case BLANK_RENDER_TEXT_COMMAND: {
      struct blank_cmd_render_text cmd_rt = cmd->cmd.cmd_rt;
      DrawText(cmd_rt.text, cmd_rt.x, cmd_rt.y, cmd_rt.font_size,
               rl_color(cmd_rt.color));
    } break;
    case BLANK_RENDER_RECTANGLE_COMMAND: {
      struct blank_cmd_render_rectangle cmd_rr = cmd->cmd.cmd_rr;
      DrawRectangle(cmd_rr.x, cmd_rr.y, cmd_rr.width, cmd_rr.height,
                    rl_color(cmd_rr.color));
    } break;
    }
  }

  EndDrawing();
}

static i32 rl_text_width(Blank_Backend *backend, const char *text,
                         size_t font_size) {
  return MeasureText(text, font_size);
}

static i32 rl_screen_width(Blank_Backend *backend) { return GetScreenWidth(); }

static i32 rl_screen_height(Blank_Backend *backend) {
  return GetScreenHeight();
}

static bool rl_window_should_close(Blank_Backend *backend) {
  return WindowShouldClose();
}

static bool rl_custom_log_level = false;

void raylib_backend_set_loglevel(u32 log_level) {
  SetTraceLogLevel(log_level);
  rl_custom_log_level = true;
}

void raylib_backend_init(Blank_Backend *backend,
                         Blank_BackendInitStage init_stage) {
  if (!rl_custom_log_level) {
    SetTraceLogLevel(LOG_NONE);
  }

  RaylibBackend _rl_backend = {0};
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

  backend->window_init_func = rl_window_init;
  backend->window_should_close_func = rl_window_should_close;

  backend->render_cmds_func = rl_render_cmds;

  backend->text_width_func = rl_text_width;
  backend->screen_width_func = rl_screen_width;
  backend->screen_height_func = rl_screen_height;
}
