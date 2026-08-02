#include "../../include/blank_backend.h"
#include "raylib.h"
#include <lilc/alloc.h>
#include <lilc/array.h>
#include <lilc/eq.h>
#include <lilc/hash.h>
#include <lilc/hashmap0.h>
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

  Hashmap img_textures; // const char * -> Texture2D

  Bump ui_elem_bump;
  Allocator ui_elem_allocator;
  Bump ui_render_elem_bump;
  Allocator ui_render_elem_allocator;
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

static bool texture_load(Blank_Backend *backend, const char *img_path,
                         Texture2D *tex) {
  if (FileExists(img_path)) {
    *tex = LoadTexture(img_path);

    RaylibBackend *rl_backend = backend->backend_context;
    hashmap_insert(&rl_backend->img_textures, &img_path, tex);

    return true;
  }
  return false;
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
      DrawRectangleLinesEx(
          (Rectangle){cmd_rr.x, cmd_rr.y, cmd_rr.width, cmd_rr.height}, 3.0f,
          rl_color(cmd_rr.color));
    } break;
    case BLANK_RENDER_IMAGE_COMMAND: {
      struct blank_cmd_render_img cmd_ri = cmd->cmd.cmd_ri;

      RaylibBackend *rl_backend = backend->backend_context;
      Texture2D *tex =
          hashmap_value(&rl_backend->img_textures, &cmd_ri.img_path);
      if (tex == NULL) {
        Texture2D _tex = {0};
        if (texture_load(backend, cmd_ri.img_path, &_tex)) {
          tex = &_tex;
        }
      }

      if (tex != NULL) {
        DrawTexturePro(*tex,
                       (Rectangle){.width = tex->width, .height = tex->height},
                       (Rectangle){
                           .x = cmd_ri.x,
                           .y = cmd_ri.y,
                           .width = cmd_ri.width,
                           .height = cmd_ri.height,
                       },
                       (Vector2){0}, 0, rl_color(cmd_ri.tint_color));
      }
    } break;
    }
  }

  EndDrawing();
}

static i32 rl_text_width(Blank_Backend *backend, const char *text,
                         size_t font_size) {
  return MeasureText(text, font_size);
}

static Blank_Size rl_screen_size(Blank_Backend *backend) {
  return (Blank_Size){
      .width = GetScreenWidth(),
      .height = GetScreenHeight(),
  };
}

static Blank_Pos rl_mouse_pos(Blank_Backend *backend) {
  Vector2 pos = GetMousePosition();
  return (Blank_Pos){
      .x = pos.x,
      .y = pos.y,
  };
}

static void rl_mouse_button_down(Blank_Backend *backend,
                                 bool buttons[_amount_mouse_button]) {
  buttons[BLANK_MOUSE_BUTTON_LEFT] = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
  buttons[BLANK_MOUSE_BUTTON_MIDDLE] = IsMouseButtonDown(MOUSE_BUTTON_MIDDLE);
  buttons[BLANK_MOUSE_BUTTON_RIGHT] = IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
}

static void rl_mouse_button_pressed(Blank_Backend *backend,
                                    bool buttons[_amount_mouse_button]) {
  buttons[BLANK_MOUSE_BUTTON_LEFT] = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
  buttons[BLANK_MOUSE_BUTTON_MIDDLE] =
      IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE);
  buttons[BLANK_MOUSE_BUTTON_RIGHT] = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
}

static void rl_mouse_button_released(Blank_Backend *backend,
                                     bool buttons[_amount_mouse_button]) {
  buttons[BLANK_MOUSE_BUTTON_LEFT] = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
  buttons[BLANK_MOUSE_BUTTON_MIDDLE] =
      IsMouseButtonReleased(MOUSE_BUTTON_MIDDLE);
  buttons[BLANK_MOUSE_BUTTON_RIGHT] = IsMouseButtonReleased(MOUSE_BUTTON_RIGHT);
}

static bool rl_window_should_close(Blank_Backend *backend) {
  return WindowShouldClose();
}

static bool rl_custom_log_level = false;

void raylib_backend_set_loglevel(u32 log_level) {
  SetTraceLogLevel(log_level);
  rl_custom_log_level = true;
}

static bool rl_img_metadata(Blank_Backend *backend, const char *img_path,
                            Blank_ImageMetadata *metdata) {
  RaylibBackend *rl_backend = backend->backend_context;

  Texture2D *tex = hashmap_value(&rl_backend->img_textures, &img_path);
  if (tex == NULL) {
    Texture2D _tex = {0};
    if (texture_load(backend, img_path, &_tex)) {
      tex = &_tex;
    } else {
      return false;
    }
  }
  metdata->path = img_path;
  metdata->width = tex->width;
  metdata->height = tex->height;

  return true;
}

void raylib_backend_init(Blank_Backend *backend) {
  if (!rl_custom_log_level) {
    SetTraceLogLevel(LOG_NONE);
  }

  RaylibBackend _rl_backend = {0};

  hashmap_init(&_rl_backend.img_textures, &HEAP_ALLOCATOR, const char *,
               Texture2D, str_ptrv_hash, str_ptrv_eq, NULL);

  bump_init(&_rl_backend.ui_elem_bump, 32000);
  bump_init(&_rl_backend.ui_render_elem_bump, 32000);

  bump_allocator_init(&_rl_backend.ui_elem_allocator,
                      &_rl_backend.ui_elem_bump);
  bump_allocator_init(&_rl_backend.ui_render_elem_allocator,
                      &_rl_backend.ui_render_elem_bump);

  backend->backend_context = heap_alloc(sizeof(RaylibBackend));
  memcpy(backend->backend_context, &_rl_backend, sizeof(RaylibBackend));

  RaylibBackend *rl_backend = backend->backend_context;

  backend->ui_elem_allocator = &rl_backend->ui_elem_allocator;
  pthread_mutex_init(&backend->ui_elem_alloc_mutex, NULL);
  backend->ui_render_elem_allocator = &rl_backend->ui_render_elem_allocator;

  backend->window_init_func = rl_window_init;
  backend->window_should_close_func = rl_window_should_close;

  backend->render_cmds_func = rl_render_cmds;

  backend->text_width_func = rl_text_width;
  backend->screen_size_func = rl_screen_size;
  backend->mouse_pos_func = rl_mouse_pos;

  backend->mouse_button_down_func = rl_mouse_button_down;
  backend->mouse_button_pressed_func = rl_mouse_button_pressed;
  backend->mouse_button_released_func = rl_mouse_button_released;

  backend->img_metadata_func = rl_img_metadata;
}

void raylib_backend_deinit(Blank_Backend *backend) {
  RaylibBackend *rl_backend = backend->backend_context;

  bump_free(&rl_backend->ui_elem_bump);
  bump_free(&rl_backend->ui_render_elem_bump);
}
