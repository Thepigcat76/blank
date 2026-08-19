#include "../include/blank.h"
#include "../include/blank/ui_elems.h"
#include "../include/blank_backend.h"
#include "blank_internal.h"
#include "lilc/array.h"
#include "lilc/assert.h"
#include "lilc/log.h"
#include <lilc/alloc.h>
#include <lilc/numbers.h>
#include <pthread.h>
#include <raylib.h>

#define ASSERT_RENDER_THREAD()                                                 \
  ASSERT(pthread_equal(pthread_self(), render_thread),                         \
         "Called render function on non-render thread");

extern pthread_t render_thread;

SubmittedUiElements submitted_ui_elems = {._mutex = PTHREAD_MUTEX_INITIALIZER};

extern Blank_UiState app_thread_ui_state;

inline static void blank_set_window_closed(Blank_UiState *ui_state) {
  ASSERT_RENDER_THREAD()

  pthread_rwlock_wrlock(&ui_state->_mutex);
  ui_state->_window_closed = true;
  pthread_rwlock_unlock(&ui_state->_mutex);
}

inline static void blank_set_window_resized(Blank_UiState *ui_state,
                                            bool resized) {
  ASSERT_RENDER_THREAD()

  pthread_rwlock_wrlock(&ui_state->_mutex);
  ui_state->_window_resized = resized;
  pthread_rwlock_unlock(&ui_state->_mutex);
}

inline static i32 blank_text_width(Blank_Backend *backend, const char *text,
                                   size_t font_size) {
  return backend->text_width_func(backend, text, font_size);
}

inline static void render_cmd(Blank_RenderCommand **cmds,
                              Blank_RenderCommand cmd) {
  _internal_array_add((void **)cmds, &cmd);
}

inline static Blank_Pos blank_mouse_pos(Blank_Backend *backend) {
  return backend->mouse_pos_func(backend);
}

inline static void blank_clear_screen(Blank_RenderCommand **cmds,
                                      Blank_Color color) {
  ASSERT_RENDER_THREAD()

  Blank_RenderCommand cmd = {
      .cmd_type = BLANK_CLEAR_SCREEN_COMMAND,
      .cmd.cmd_cs.color = color,
  };
  render_cmd(cmds, cmd);
}

static inline void blank_render_image(Blank_RenderCommand **cmds,
                                      const char *img_path, i32 x, i32 y,
                                      i32 width, i32 height,
                                      Blank_Color tint_color) {
  ASSERT_RENDER_THREAD()

  Blank_RenderCommand cmd = {
      .cmd_type = BLANK_RENDER_IMAGE_COMMAND,
      .cmd.cmd_ri =
          {
              .img_path = img_path,
              .x = x,
              .y = y,
              .width = width,
              .height = height,
              .tint_color = tint_color,
          },
  };
  render_cmd(cmds, cmd);
}

inline static void blank_render_rect(Blank_RenderCommand **cmds, i32 x, i32 y,
                                     i32 width, i32 height, Blank_Color color) {
  ASSERT_RENDER_THREAD()

  Blank_RenderCommand cmd = {
      .cmd_type = BLANK_RENDER_RECTANGLE_COMMAND,
      .cmd.cmd_rr =
          {
              .x = x,
              .y = y,
              .width = width,
              .height = height,
              .color = color,
          },
  };
  render_cmd(cmds, cmd);
}

inline static void blank_render_text(Blank_RenderCommand **cmds,
                                     const char *text, i32 x, i32 y,
                                     size_t font_size, Blank_Color color) {
  ASSERT_RENDER_THREAD()

  Blank_RenderCommand cmd = {
      .cmd_type = BLANK_RENDER_TEXT_COMMAND,
      .cmd.cmd_rt =
          {
              .x = x,
              .y = y,
              .text = text,
              .font_size = font_size,
              .color = color,
          },
  };
  render_cmd(cmds, cmd);
}

void blank_render_image_elem(const Blank_RenderableUiElement *render_elem,
                             Blank_RenderContext render_ctx) {
  ASSERT_RENDER_THREAD()

  Blank_UiElemImage *img = render_elem->elem.args;
  Blank_ImageMetadata metadata = {0};
  bool exists = render_ctx.backend->img_metadata_func(render_ctx.backend,
                                                      img->img_path, &metadata);
  if (exists) {
    i32 img_width = metadata.width * img->scale;
    i32 img_height = metadata.height * img->scale;

    i32 render_width = render_elem->width;
    i32 render_height = render_elem->height;

    if (img->keep_ratio) {
      f32 img_aspect = (f32)img_width / (f32)img_height;
      f32 rect_aspect = (f32)render_width / (f32)render_height;

      if (img_aspect > rect_aspect) {
        // image is wider than container — fit to width
        render_width = render_elem->width;
        render_height = (i32)(render_elem->width / img_aspect);
      } else {
        // image is taller than container — fit to height
        render_height = render_elem->height;
        render_width = (i32)(render_elem->height * img_aspect);
      }

      // center within the render element bounds
      i32 x_offset = (render_elem->width - render_width) / 2;
      i32 y_offset = (render_elem->height - render_height) / 2;

      blank_render_image(render_ctx.render_commands, img->img_path,
                         render_elem->x + x_offset, render_elem->y + y_offset,
                         render_width, render_height, BLANK_WHITE);
    } else {
      blank_render_image(render_ctx.render_commands, img->img_path,
                         render_elem->x, render_elem->y, render_width,
                         render_height, BLANK_WHITE);
    }
  }
}

Blank_Size blank_min_size_image_elem(const Blank_UiElement *elem,
                                     Blank_Context ctx) {
  i32 width = 100;
  i32 height = 100;

  Blank_UiElemImage *img = elem->args;

  Blank_ImageMetadata metadata = {0};
  bool exists =
      ctx.backend->img_metadata_func(ctx.backend, img->img_path, &metadata);

  if (elem->size_config.x_size_kind == BLANK_SIZE_FIXED_ID) {
    width = elem->size_config.width;
  } else if (exists) {
    width = metadata.width * img->scale;
  }

  if (elem->size_config.y_size_kind == BLANK_SIZE_FIXED_ID) {
    height = elem->size_config.height;
  } else if (exists) {
    height = metadata.height * img->scale;
  }

  return (Blank_Size){.width = width, .height = height};
}

void blank_deinit_image_elem(Blank_UiElement *elem) {}

void blank_render_button(const Blank_RenderableUiElement *render_elem,
                         Blank_RenderContext render_ctx) {
  ASSERT_RENDER_THREAD()

  Blank_UiElemButton *btn = render_elem->elem.args;

  blank_render_rect(render_ctx.render_commands, render_elem->x, render_elem->y,
                    render_elem->width, render_elem->height, btn->bg_color);

  i32 text_width = blank_text_width(render_ctx.backend, btn->text, 16);

  blank_render_text(render_ctx.render_commands, btn->text,
                    render_elem->x + (render_elem->width - text_width) / 2,
                    render_elem->y + (render_elem->height - 16) / 2, 16,
                    btn->text_color);
}

Blank_Size blank_min_size_button(const Blank_UiElement *elem,
                                 Blank_Context ctx) {
  ASSERT_RENDER_THREAD();

  Blank_UiElemButton *btn = elem->args;

  i32 width = 0;
  if (elem->size_config.x_size_kind == BLANK_SIZE_FIXED_ID) {
    width = elem->size_config.width;
  } else {
    width = MeasureText(btn->text, 16) + 16;
  }
  i32 height = 0;
  if (elem->size_config.y_size_kind == BLANK_SIZE_FIXED_ID) {
    height = elem->size_config.height;
  } else {
    height = 16;
  }

  Blank_Size size = {
      .width = width,
      .height = height,
  };

  return size;
}

Blank_Size blank_min_size_group(const Blank_UiElement *elem,
                                Blank_Context ctx) {
  ASSERT_RENDER_THREAD()

  Blank_UiElemGroup *group = elem->args;
  Blank_Size size =
      group->layout.min_size_elems_func(&group->layout, group->ui_elems, ctx);

  return size;
}

inline static Blank_Size blank_screen_size(Blank_Backend *backend) {
  ASSERT_RENDER_THREAD()
  return backend->screen_size_func(backend);
}

inline static bool blank_window_should_close(Blank_Backend *backend) {
  ASSERT_RENDER_THREAD()
  return backend->window_should_close_func(backend);
}

inline static void blank_render_cmds(Blank_Backend *backend,
                                     Blank_RenderCommand *cmds) {
  ASSERT_RENDER_THREAD()
  backend->render_cmds_func(backend, cmds);
}

static void _blank_handle_resize(Blank_Backend *backend, Blank_Size *size) {
  ASSERT(backend != NULL, "Backend is null");
  ASSERT_RENDER_THREAD()

  Blank_Size new_size = blank_screen_size(backend);
  if (size->width != new_size.width || size->height != new_size.height) {
    blank_set_window_resized(&app_thread_ui_state, true);
    log_info("Set window resized");
    size->width = new_size.width;
    size->height = new_size.height;
  }
}

static void _blank_rebuild_layout(Blank_Backend *backend,
                                  Blank_RenderableUiElement **render_elems) {
  ASSERT(backend != NULL, "Backend is null");
  ASSERT_RENDER_THREAD()

  if (render_elems == NULL || *render_elems == NULL)
    return;

  array_clear(*render_elems);

  pthread_mutex_lock(&submitted_ui_elems._mutex);

  Blank_Size size = blank_screen_size(backend);

  submitted_ui_elems.ui_layout.rearrange_elems_func(
      &submitted_ui_elems.ui_layout, &submitted_ui_elems.ui_elements,
      render_elems,
      (Blank_LayoutContext){
          .backend = backend,
          .container_width = size.width,
          .container_height = size.height,
      });
  submitted_ui_elems.rebuild_layout = false;

  pthread_mutex_unlock(&submitted_ui_elems._mutex);
}

static void _blank_render_elems(Blank_Backend *backend,
                                Blank_RenderableUiElement *render_elems,
                                Blank_RenderCommand **render_cmds,
                                Blank_Pos mouse_pos,
                                Blank_RenderableUiElement **hovered_elem) {
  ASSERT(backend != NULL, "Backend is null");
  ASSERT(render_cmds != NULL, "Render Commands is null");
  ASSERT_RENDER_THREAD()

  if (render_elems == NULL)
    return;

  Blank_RenderableUiElement *render_elem;
  array_foreach(render_elems, render_elem) {
    RenderUiElemFunc render_elem_func = render_elem->elem.render_func;
    Blank_RenderContext render_ctx = {
        .render_commands = render_cmds,
        .backend = backend,
    };

    if (render_elem->x < mouse_pos.x &&
        render_elem->x + render_elem->width > mouse_pos.x &&
        render_elem->y < mouse_pos.y &&
        render_elem->y + render_elem->height > mouse_pos.y &&
        render_elem->elem.elem_kind != BLANK_ELEM_GROUP) {
      *hovered_elem = render_elem;
      // log_debug("hovered uid: %zu", render_elem->elem.uid);
    }

    if (render_elem_func != NULL) {
      render_elem_func(render_elem, render_ctx);
    }
  }
}

static void mouse_button_state(Blank_Backend *backend,
                               Blank_KeyState buttons[_amount_mouse_button]) {
  backend->mouse_button_state_func(backend, buttons);
}

static Rectangle *debug_rects = NULL;

void *blank_render_thread_run(void *args) {
  ASSERT_RENDER_THREAD()
  struct render_thread_args *render_args = args;

  debug_rects = array_new(Rectangle, &HEAP_ALLOCATOR);

  Blank_Backend backend = render_args->backend;

  backend.window_init_func(&backend, &render_args->init_state);

  Blank_Size prev_size = blank_screen_size(&backend);

  Blank_RenderCommand *render_cmds =
      array_new_capacity(Blank_RenderCommand, 2, &HEAP_ALLOCATOR);
  Blank_RenderableUiElement *render_elems =
      array_new_capacity(Blank_RenderableUiElement, 2, &HEAP_ALLOCATOR);

  log_debug("Arr ptr: %p, ptr ptr: %p", render_cmds, &render_cmds);

  log_debug("cmds: %zu", ((_InternalArrayHeader *)render_cmds - 1)->item_size);

  bool rebuild_layout = false;

  Blank_Pos mouse_pos = {0};
  Blank_Color bg_color = BLANK_WHITE;

  while (!blank_window_should_close(&backend)) {
    array_clear(render_cmds);

    _blank_handle_resize(&backend, &prev_size);

    pthread_mutex_lock(&submitted_ui_elems._mutex);
    rebuild_layout = submitted_ui_elems.rebuild_layout;
    bg_color = submitted_ui_elems.bg_color;
    pthread_mutex_unlock(&submitted_ui_elems._mutex);

    blank_clear_screen(&render_cmds, bg_color);

    mouse_pos = blank_mouse_pos(&backend);

    if (rebuild_layout) {
      array_clear(debug_rects);
      _blank_rebuild_layout(&backend, &render_elems);
    }

    Blank_RenderableUiElement *hovered_elem = NULL;
    if (render_elems != NULL) {
      _blank_render_elems(&backend, render_elems, &render_cmds, mouse_pos,
                          &hovered_elem);
    }

    Blank_KeyState buttons[_amount_mouse_button] = {0};
    mouse_button_state(&backend, buttons);

    pthread_rwlock_wrlock(&app_thread_ui_state._mutex);
    if (hovered_elem != NULL && app_thread_ui_state.clicked_button == -1) {
      app_thread_ui_state.clicked_elem = hovered_elem->elem;
      if (buttons[BLANK_MOUSE_BUTTON_LEFT] == KEY_STATE_RELEASED)
        app_thread_ui_state.clicked_button = BLANK_MOUSE_BUTTON_LEFT;
      else if (buttons[BLANK_MOUSE_BUTTON_RIGHT] == KEY_STATE_RELEASED)
        app_thread_ui_state.clicked_button = BLANK_MOUSE_BUTTON_RIGHT;
      else if (buttons[BLANK_MOUSE_BUTTON_MIDDLE] == KEY_STATE_RELEASED)
        app_thread_ui_state.clicked_button = BLANK_MOUSE_BUTTON_MIDDLE;
    }

    pthread_rwlock_unlock(&app_thread_ui_state._mutex);

    Rectangle *rect;
    array_foreach(debug_rects, rect) {
      blank_render_rect(&render_cmds, rect->x, rect->y, rect->width,
                        rect->height, blank_color_make(0, 255, 0, 255));
    }

    blank_render_cmds(&backend, render_cmds);
  }

  blank_set_window_closed(&app_thread_ui_state);

  array_free(render_cmds);
  array_free(render_elems);

  CloseWindow();

  return NULL;
}

Blank_Size _blank_impl_linear_layout_min_size_elems(
    const Blank_UiLayout *layout, Blank_UiElement *elems, Blank_Context ctx) {
  ASSERT_RENDER_THREAD()
  Blank_LayoutOrientation orientation = layout->layout_data.linear.orientation;
  i32 width = 0;
  i32 height = 0;

  Blank_UiElement *elem;
  array_foreach(elems, elem) {
    Blank_Size size = elem->min_size_func(elem, ctx);
    if (orientation == BLANK_HORIZONTAL) {
      width += size.width;
      height = max(height, size.height);
    } else if (orientation == BLANK_VERTICAL) {
      width = max(width, size.width);
      height += size.height;
    }
  }

  return (Blank_Size){
      .width = width,
      .height = height,
  };
}

void _blank_impl_linear_layout_rearrange_elems(
    const Blank_UiLayout *layout, Blank_UiElement **elems,
    Blank_RenderableUiElement **renderable_elems, Blank_LayoutContext context) {
  ASSERT_RENDER_THREAD()
  if (elems == NULL || *elems == NULL)
    return;

  size_t elems_amount = array_len(*elems);

  size_t x_sizeable_elems_amount = 0;
  size_t y_sizeable_elems_amount = 0;

  Blank_UiElement *elem;
  array_foreach(*elems, elem) {
    if (elem->size_config.x_size_kind == BLANK_SIZE_DYNAMIC_ID) {
      x_sizeable_elems_amount++;
    }

    if (elem->size_config.y_size_kind == BLANK_SIZE_DYNAMIC_ID) {
      y_sizeable_elems_amount++;
    }
  }

  if (elems_amount == 0)
    return;

  if (context.container_width <= 0)
    context.container_width = 1;
  if (context.container_height <= 0)
    context.container_height = 1;

  Blank_Context ctx = {.backend = context.backend};

  log_debug("Linear layout w: %d, h: %d", context.container_width,
            context.container_height);

  Blank_LayoutOrientation orientation = layout->layout_data.linear.orientation;

  log_debug("orientation: %s, container-x: %d, container-width: %d",
            orientation == BLANK_VERTICAL ? "vertical" : "horizontal",
            context.container_x, context.container_width);

  // Calculate minimum layout size for given elements, remainders...
  Blank_Size layout_min_size = layout->min_size_elems_func(layout, *elems, ctx);

  u32 padding = layout->layout_data.linear.padding;
  u32 total_padding = (padding * (elems_amount - 1));

  i32 rem_width = context.container_width - layout_min_size.width;
  if (orientation == BLANK_HORIZONTAL) {
    rem_width -= total_padding;
  }
  i32 rem_height = context.container_height - layout_min_size.height;
  if (orientation == BLANK_VERTICAL) {
    rem_height -= total_padding;
  }

  log_debug("Remainder: width %d, height %d", rem_width, rem_height);

  i32 unsplit_rem_width = rem_width % x_sizeable_elems_amount;
  i32 unsplit_rem_height = rem_height % y_sizeable_elems_amount;

  i32 extra_width = (rem_width - unsplit_rem_width) / x_sizeable_elems_amount;
  i32 extra_height =
      (rem_height - unsplit_rem_height) / y_sizeable_elems_amount;

  i32 x_offset = context.container_x;
  i32 y_offset = context.container_y;

  // Scale every element by the calculated amount
  Blank_UiElement *elem1;
  array_foreach(*elems, elem1) {
    MinUiElemSizeFunc min_elem_size_func = elem1->min_size_func;
    Blank_Size min_size = min_elem_size_func(elem1, ctx);

    Blank_RenderableUiElement render_elem = {
        .elem = *elem1,
        .width = min_size.width,
        .height = min_size.height,
    };

    if (elem1->size_config.x_size_kind == BLANK_SIZE_DYNAMIC_ID) {
      render_elem.width += extra_width;
    }

    if (elem1->size_config.y_size_kind == BLANK_SIZE_DYNAMIC_ID) {
      render_elem.height += extra_height;
    }

    if (orientation == BLANK_HORIZONTAL) {
      if (elem1->size_config.x_size_kind == BLANK_SIZE_DYNAMIC_ID) {
        render_elem.width += unsplit_rem_width > 0 ? 1 : 0;
        unsplit_rem_width -= 1;
      }
      render_elem.height = max(render_elem.height, context.container_height);
    }

    if (orientation == BLANK_VERTICAL) {
      if (elem1->size_config.y_size_kind == BLANK_SIZE_DYNAMIC_ID) {
        render_elem.height += unsplit_rem_height > 0 ? 1 : 0;
        unsplit_rem_height -= 1;
      }
      render_elem.width = max(render_elem.width, context.container_width);
    }

    render_elem.x = x_offset;
    render_elem.y = y_offset;

    if (render_elem.elem.elem_kind == BLANK_ELEM_GROUP) {
      log_debug("Group rearranging");

      Blank_UiElemGroup *group = render_elem.elem.args;
      group->layout.rearrange_elems_func(
          &group->layout, &group->ui_elems, renderable_elems,
          (Blank_LayoutContext){
              .backend = context.backend,
              .container_x = x_offset,
              .container_y = y_offset,
              .container_width = render_elem.width,
              .container_height = render_elem.height,
          });
      log_debug("Group end");
    }

    if (orientation == BLANK_HORIZONTAL) {
      x_offset += render_elem.width + layout->layout_data.linear.padding;
    }

    if (orientation == BLANK_VERTICAL) {
      y_offset += render_elem.height + layout->layout_data.linear.padding;
    }

    array_add(*renderable_elems, render_elem);
  }
}
