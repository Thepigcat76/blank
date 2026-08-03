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

// clang-format off

static Blank_Key blank_key_from_rl(KeyboardKey rl_key) {
  switch (rl_key) {
    case KEY_NULL: return BLANK_KEY_NULL;
    // Letters
    case KEY_A: return BLANK_KEY_A;
    case KEY_B: return BLANK_KEY_B;
    case KEY_C: return BLANK_KEY_C;
    case KEY_D: return BLANK_KEY_D;
    case KEY_E: return BLANK_KEY_E;
    case KEY_F: return BLANK_KEY_F;
    case KEY_G: return BLANK_KEY_G;
    case KEY_H: return BLANK_KEY_H;
    case KEY_I: return BLANK_KEY_I;
    case KEY_J: return BLANK_KEY_J;
    case KEY_K: return BLANK_KEY_K;
    case KEY_L: return BLANK_KEY_L;
    case KEY_M: return BLANK_KEY_M;
    case KEY_N: return BLANK_KEY_N;
    case KEY_O: return BLANK_KEY_O;
    case KEY_P: return BLANK_KEY_P;
    case KEY_Q: return BLANK_KEY_Q;
    case KEY_R: return BLANK_KEY_R;
    case KEY_S: return BLANK_KEY_S;
    case KEY_T: return BLANK_KEY_T;
    case KEY_U: return BLANK_KEY_U;
    case KEY_V: return BLANK_KEY_V;
    case KEY_W: return BLANK_KEY_W;
    case KEY_X: return BLANK_KEY_X;
    case KEY_Y: return BLANK_KEY_Y;
    case KEY_Z: return BLANK_KEY_Z;

    // Numbers
    case KEY_ZERO:  return BLANK_KEY_0;
    case KEY_ONE:   return BLANK_KEY_1;
    case KEY_TWO:   return BLANK_KEY_2;
    case KEY_THREE: return BLANK_KEY_3;
    case KEY_FOUR:  return BLANK_KEY_4;
    case KEY_FIVE:  return BLANK_KEY_5;
    case KEY_SIX:   return BLANK_KEY_6;
    case KEY_SEVEN: return BLANK_KEY_7;
    case KEY_EIGHT: return BLANK_KEY_8;
    case KEY_NINE:  return BLANK_KEY_9;

    // Function keys
    case KEY_F1:  return BLANK_KEY_F1;
    case KEY_F2:  return BLANK_KEY_F2;
    case KEY_F3:  return BLANK_KEY_F3;
    case KEY_F4:  return BLANK_KEY_F4;
    case KEY_F5:  return BLANK_KEY_F5;
    case KEY_F6:  return BLANK_KEY_F6;
    case KEY_F7:  return BLANK_KEY_F7;
    case KEY_F8:  return BLANK_KEY_F8;
    case KEY_F9:  return BLANK_KEY_F9;
    case KEY_F10: return BLANK_KEY_F10;
    case KEY_F11: return BLANK_KEY_F11;
    case KEY_F12: return BLANK_KEY_F12;

    // Arrow keys
    case KEY_UP:    return BLANK_KEY_UP;
    case KEY_DOWN:  return BLANK_KEY_DOWN;
    case KEY_LEFT:  return BLANK_KEY_LEFT;
    case KEY_RIGHT: return BLANK_KEY_RIGHT;

    // Modifiers
    case KEY_LEFT_SHIFT:  return BLANK_KEY_LEFT_SHIFT;
    case KEY_RIGHT_SHIFT: return BLANK_KEY_RIGHT_SHIFT;
    case KEY_LEFT_CONTROL:  return BLANK_KEY_LEFT_CTRL;
    case KEY_RIGHT_CONTROL: return BLANK_KEY_RIGHT_CTRL;
    case KEY_LEFT_ALT:  return BLANK_KEY_LEFT_ALT;
    case KEY_RIGHT_ALT: return BLANK_KEY_RIGHT_ALT;
    case KEY_LEFT_SUPER:  return BLANK_KEY_LEFT_SUPER;
    case KEY_RIGHT_SUPER: return BLANK_KEY_RIGHT_SUPER;

    // Control keys
    case KEY_ESCAPE:        return BLANK_KEY_ESCAPE;
    case KEY_ENTER:         return BLANK_KEY_ENTER;
    case KEY_TAB:           return BLANK_KEY_TAB;
    case KEY_BACKSPACE:     return BLANK_KEY_BACKSPACE;
    case KEY_DELETE:        return BLANK_KEY_DELETE;
    case KEY_INSERT:        return BLANK_KEY_INSERT;
    case KEY_HOME:          return BLANK_KEY_HOME;
    case KEY_END:           return BLANK_KEY_END;
    case KEY_PAGE_UP:       return BLANK_KEY_PAGE_UP;
    case KEY_PAGE_DOWN:     return BLANK_KEY_PAGE_DOWN;
    case KEY_CAPS_LOCK:     return BLANK_KEY_CAPS_LOCK;
    case KEY_PRINT_SCREEN:  return BLANK_KEY_PRINT_SCREEN;
    case KEY_SCROLL_LOCK:   return BLANK_KEY_SCROLL_LOCK;
    case KEY_PAUSE:         return BLANK_KEY_PAUSE;
    case KEY_SPACE:         return BLANK_KEY_SPACE;

    // Punctuation
    case KEY_APOSTROPHE:    return BLANK_KEY_APOSTROPHE;
    case KEY_COMMA:         return BLANK_KEY_COMMA;
    case KEY_MINUS:         return BLANK_KEY_MINUS;
    case KEY_PERIOD:        return BLANK_KEY_PERIOD;
    case KEY_SLASH:         return BLANK_KEY_SLASH;
    case KEY_SEMICOLON:     return BLANK_KEY_SEMICOLON;
    case KEY_EQUAL:         return BLANK_KEY_EQUAL;
    case KEY_LEFT_BRACKET:  return BLANK_KEY_LEFT_BRACKET;
    case KEY_BACKSLASH:     return BLANK_KEY_BACKSLASH;
    case KEY_RIGHT_BRACKET: return BLANK_KEY_RIGHT_BRACKET;
    case KEY_GRAVE:         return BLANK_KEY_GRAVE;

    // Numpad
    case KEY_NUM_LOCK:        return BLANK_KEY_NUM_LOCK;
    case KEY_KP_0:            return BLANK_KEY_NUMPAD_0;
    case KEY_KP_1:            return BLANK_KEY_NUMPAD_1;
    case KEY_KP_2:            return BLANK_KEY_NUMPAD_2;
    case KEY_KP_3:            return BLANK_KEY_NUMPAD_3;
    case KEY_KP_4:            return BLANK_KEY_NUMPAD_4;
    case KEY_KP_5:            return BLANK_KEY_NUMPAD_5;
    case KEY_KP_6:            return BLANK_KEY_NUMPAD_6;
    case KEY_KP_7:            return BLANK_KEY_NUMPAD_7;
    case KEY_KP_8:            return BLANK_KEY_NUMPAD_8;
    case KEY_KP_9:            return BLANK_KEY_NUMPAD_9;
    case KEY_KP_ADD:          return BLANK_KEY_NUMPAD_ADD;
    case KEY_KP_SUBTRACT:     return BLANK_KEY_NUMPAD_SUBTRACT;
    case KEY_KP_MULTIPLY:     return BLANK_KEY_NUMPAD_MULTIPLY;
    case KEY_KP_DIVIDE:       return BLANK_KEY_NUMPAD_DIVIDE;
    case KEY_KP_DECIMAL:      return BLANK_KEY_NUMPAD_DECIMAL;
    case KEY_KP_ENTER:        return BLANK_KEY_NUMPAD_ENTER;

    default: return -1; // unknown key
  }

}

static KeyboardKey rl_key_from_blank(Blank_Key blank_key) {
  switch (blank_key) {
    case BLANK_KEY_NULL: return KEY_NULL;
    // Letters
    case BLANK_KEY_A: return KEY_A;
    case BLANK_KEY_B: return KEY_B;
    case BLANK_KEY_C: return KEY_C;
    case BLANK_KEY_D: return KEY_D;
    case BLANK_KEY_E: return KEY_E;
    case BLANK_KEY_F: return KEY_F;
    case BLANK_KEY_G: return KEY_G;
    case BLANK_KEY_H: return KEY_H;
    case BLANK_KEY_I: return KEY_I;
    case BLANK_KEY_J: return KEY_J;
    case BLANK_KEY_K: return KEY_K;
    case BLANK_KEY_L: return KEY_L;
    case BLANK_KEY_M: return KEY_M;
    case BLANK_KEY_N: return KEY_N;
    case BLANK_KEY_O: return KEY_O;
    case BLANK_KEY_P: return KEY_P;
    case BLANK_KEY_Q: return KEY_Q;
    case BLANK_KEY_R: return KEY_R;
    case BLANK_KEY_S: return KEY_S;
    case BLANK_KEY_T: return KEY_T;
    case BLANK_KEY_U: return KEY_U;
    case BLANK_KEY_V: return KEY_V;
    case BLANK_KEY_W: return KEY_W;
    case BLANK_KEY_X: return KEY_X;
    case BLANK_KEY_Y: return KEY_Y;
    case BLANK_KEY_Z: return KEY_Z;

    // Numbers
    case BLANK_KEY_0:  return KEY_ZERO;
    case BLANK_KEY_1:   return KEY_ONE;
    case BLANK_KEY_2:   return KEY_TWO;
    case BLANK_KEY_3: return KEY_THREE;
    case BLANK_KEY_4:  return KEY_FOUR;
    case BLANK_KEY_5:  return KEY_FIVE;
    case BLANK_KEY_6:   return KEY_SIX;
    case BLANK_KEY_7: return KEY_SEVEN;
    case BLANK_KEY_8: return KEY_EIGHT;
    case BLANK_KEY_9:  return KEY_NINE;

    // Function keys
    case BLANK_KEY_F1:  return KEY_F1;
    case BLANK_KEY_F2:  return KEY_F2;
    case BLANK_KEY_F3:  return KEY_F3;
    case BLANK_KEY_F4:  return KEY_F4;
    case BLANK_KEY_F5:  return KEY_F5;
    case BLANK_KEY_F6:  return KEY_F6;
    case BLANK_KEY_F7:  return KEY_F7;
    case BLANK_KEY_F8:  return KEY_F8;
    case BLANK_KEY_F9:  return KEY_F9;
    case BLANK_KEY_F10: return KEY_F10;
    case BLANK_KEY_F11: return KEY_F11;
    case BLANK_KEY_F12: return KEY_F12;

    // Arrow keys
    case BLANK_KEY_UP:    return KEY_UP;
    case BLANK_KEY_DOWN:  return KEY_DOWN;
    case BLANK_KEY_LEFT:  return KEY_LEFT;
    case BLANK_KEY_RIGHT: return KEY_RIGHT;

    // Modifiers
    case BLANK_KEY_LEFT_SHIFT:  return KEY_LEFT_SHIFT;
    case BLANK_KEY_RIGHT_SHIFT: return KEY_RIGHT_SHIFT;
    case BLANK_KEY_LEFT_CTRL:  return KEY_LEFT_CONTROL;
    case BLANK_KEY_RIGHT_CTRL: return KEY_RIGHT_CONTROL;
    case BLANK_KEY_LEFT_ALT:  return KEY_LEFT_ALT;
    case BLANK_KEY_RIGHT_ALT: return KEY_RIGHT_ALT;
    case BLANK_KEY_LEFT_SUPER:  return KEY_LEFT_SUPER;
    case BLANK_KEY_RIGHT_SUPER: return KEY_RIGHT_SUPER;

    // Control keys
    case BLANK_KEY_ESCAPE:        return KEY_ESCAPE;
    case BLANK_KEY_ENTER:         return KEY_ENTER;
    case BLANK_KEY_TAB:           return KEY_TAB;
    case BLANK_KEY_BACKSPACE:     return KEY_BACKSPACE;
    case BLANK_KEY_DELETE:        return KEY_DELETE;
    case BLANK_KEY_INSERT:        return KEY_INSERT;
    case BLANK_KEY_HOME:          return KEY_HOME;
    case BLANK_KEY_END:           return KEY_END;
    case BLANK_KEY_PAGE_UP:       return KEY_PAGE_UP;
    case BLANK_KEY_PAGE_DOWN:     return KEY_PAGE_DOWN;
    case BLANK_KEY_CAPS_LOCK:     return KEY_CAPS_LOCK;
    case BLANK_KEY_PRINT_SCREEN:  return KEY_PRINT_SCREEN;
    case BLANK_KEY_SCROLL_LOCK:   return KEY_SCROLL_LOCK;
    case BLANK_KEY_PAUSE:         return KEY_PAUSE;
    case BLANK_KEY_SPACE:         return KEY_SPACE;

    // Punctuation
    case BLANK_KEY_APOSTROPHE:    return KEY_APOSTROPHE;
    case BLANK_KEY_COMMA:         return KEY_COMMA;
    case BLANK_KEY_MINUS:         return KEY_MINUS;
    case BLANK_KEY_PERIOD:        return KEY_PERIOD;
    case BLANK_KEY_SLASH:         return KEY_SLASH;
    case BLANK_KEY_SEMICOLON:     return KEY_SEMICOLON;
    case BLANK_KEY_EQUAL:         return KEY_EQUAL;
    case BLANK_KEY_LEFT_BRACKET:  return KEY_LEFT_BRACKET;
    case BLANK_KEY_BACKSLASH:     return KEY_BACKSLASH;
    case BLANK_KEY_RIGHT_BRACKET: return KEY_RIGHT_BRACKET;
    case BLANK_KEY_GRAVE:         return KEY_GRAVE;

    // Numpad
    case BLANK_KEY_NUM_LOCK:        return KEY_NUM_LOCK;
    case BLANK_KEY_NUMPAD_0:        return KEY_KP_0;
    case BLANK_KEY_NUMPAD_1:        return KEY_KP_1;
    case BLANK_KEY_NUMPAD_2:        return KEY_KP_2;
    case BLANK_KEY_NUMPAD_3:        return KEY_KP_3;
    case BLANK_KEY_NUMPAD_4:        return KEY_KP_4;
    case BLANK_KEY_NUMPAD_5:        return KEY_KP_5;
    case BLANK_KEY_NUMPAD_6:        return KEY_KP_6;
    case BLANK_KEY_NUMPAD_7:        return KEY_KP_7;
    case BLANK_KEY_NUMPAD_8:        return KEY_KP_8;
    case BLANK_KEY_NUMPAD_9:        return KEY_KP_9;
    case BLANK_KEY_NUMPAD_ADD:      return KEY_KP_ADD;
    case BLANK_KEY_NUMPAD_SUBTRACT: return KEY_KP_SUBTRACT;
    case BLANK_KEY_NUMPAD_MULTIPLY: return KEY_KP_MULTIPLY;
    case BLANK_KEY_NUMPAD_DIVIDE:   return KEY_KP_DIVIDE;
    case BLANK_KEY_NUMPAD_DECIMAL:  return KEY_KP_DECIMAL;
    case BLANK_KEY_NUMPAD_ENTER:    return KEY_KP_ENTER;

    default: return -1; // unknown key
  }

}

// clang-format on

static Blank_KeyState mouse_btn_state_for_btn(MouseButton btn) {
  if (IsMouseButtonDown(btn)) {
    return KEY_STATE_DOWN;
  }
  if (IsMouseButtonPressed(btn)) {
    return KEY_STATE_PRESSED;
  } else if (IsMouseButtonReleased(btn)) {
    return KEY_STATE_RELEASED;
  }
  return KEY_STATE_UP;
}

static void
rl_mouse_button_state(Blank_Backend *backend,
                      Blank_KeyState buttons[_amount_mouse_button]) {
  buttons[BLANK_MOUSE_BUTTON_LEFT] = mouse_btn_state_for_btn(MOUSE_BUTTON_LEFT);
  buttons[BLANK_MOUSE_BUTTON_MIDDLE] =
      mouse_btn_state_for_btn(MOUSE_BUTTON_MIDDLE);
  buttons[BLANK_MOUSE_BUTTON_RIGHT] =
      mouse_btn_state_for_btn(MOUSE_BUTTON_RIGHT);
}

static Blank_Key rl_key_pressed(Blank_Backend *backend) {
  KeyboardKey rl_key = GetKeyPressed();
  return blank_key_from_rl(rl_key);
}

static Blank_KeyState rl_key_state(Blank_Backend *backend, Blank_Key key) {
  KeyboardKey rl_key = rl_key_from_blank(key);

  if (IsKeyPressed(rl_key)) {
    return KEY_STATE_PRESSED;
  }
  if (IsKeyReleased(rl_key)) {
    return KEY_STATE_RELEASED;
  }

  if (IsKeyDown(rl_key)) {
    return KEY_STATE_DOWN;
  }

  return KEY_STATE_UP;
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

  backend->mouse_button_state_func = rl_mouse_button_state;

  backend->img_metadata_func = rl_img_metadata;

  backend->key_pressed_func = rl_key_pressed;
}

void raylib_backend_deinit(Blank_Backend *backend) {
  RaylibBackend *rl_backend = backend->backend_context;

  bump_free(&rl_backend->ui_elem_bump);
  bump_free(&rl_backend->ui_render_elem_bump);
}
