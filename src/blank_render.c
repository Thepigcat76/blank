#include "blank_internal.h"
#include "lilc/array.h"
#include "lilc/assert.h"
#include "lilc/log.h"
#include <lilc/alloc.h>
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

  pthread_mutex_lock(&ui_state->_mutex);
  ui_state->_window_closed = true;
  pthread_mutex_unlock(&ui_state->_mutex);
}

inline static void blank_set_window_resized(Blank_UiState *ui_state,
                                            bool resized) {
  ASSERT_RENDER_THREAD()

  pthread_mutex_lock(&ui_state->_mutex);
  ui_state->_window_resized = resized;
  pthread_mutex_unlock(&ui_state->_mutex);
}

inline static i32 blank_text_width(Blank_Backend *backend, const char *text,
                                   size_t font_size) {
  return backend->text_width_func(backend, text, font_size);
}

inline static void render_cmd(Blank_RenderCommand **cmds,
                              Blank_RenderCommand cmd) {
  _internal_array_add((void **)cmds, &cmd);
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

void blank_render_button(const Blank_RenderableUiElement *render_elem,
                         Blank_RenderContext render_ctx) {
  ASSERT_RENDER_THREAD()

  blank_render_rect(render_ctx.render_commands, render_elem->x, render_elem->y,
                    render_elem->width, render_elem->height,
                    blank_color_make(255, 0, 0, 255));

  Blank_UiElemButton *btn = render_elem->elem.args;

  i32 text_width = blank_text_width(render_ctx.backend, btn->text, 16);

  blank_render_text(render_ctx.render_commands, btn->text,
                    render_elem->x + (render_elem->width - text_width) / 2,
                    render_elem->y + (render_elem->height - 16) / 2, 16,
                    blank_color_make(0, 0, 0, 255));
}

Blank_Size blank_min_size_button(const Blank_UiElement *elem) {
  ASSERT_RENDER_THREAD();

  Blank_UiElemButton *btn = elem->args;
  Blank_Size size = {
      .width = MeasureText(btn->text, 16),
      .height = 16,
  };

  return size;
}

Blank_Size blank_min_size_group(const Blank_UiElement *elem) {
  ASSERT_RENDER_THREAD()

  Blank_UiElemGroup *group = elem->args;
  Blank_Size size =
      group->layout.min_size_func(&group->layout, group->ui_elems);

  return size;
}

inline static i32 blank_screen_width(Blank_Backend *backend) {
  ASSERT_RENDER_THREAD()
  return backend->screen_width_func(backend);
}

inline static i32 blank_screen_height(Blank_Backend *backend) {
  ASSERT_RENDER_THREAD()
  return backend->screen_height_func(backend);
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

static void _blank_handle_resize(Blank_Backend *backend, i32 *prev_width,
                                 i32 *prev_height) {
  ASSERT(backend != NULL, "Backend is null");
  ASSERT_RENDER_THREAD()

  i32 new_width = blank_screen_width(backend);
  i32 new_height = blank_screen_height(backend);
  if (*prev_width != new_width || *prev_height != new_height) {
    blank_set_window_resized(&app_thread_ui_state, true);
    log_info("Set window resized");
    *prev_width = new_width;
    *prev_height = new_height;
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

  submitted_ui_elems.ui_layout.rearrange_elems_func(
      &submitted_ui_elems.ui_layout, &submitted_ui_elems.ui_elements,
      render_elems,
      (Blank_LayoutContext){
          .backend = backend,
          .container_width = blank_screen_width(backend),
          .container_height = blank_screen_height(backend),
      });
  submitted_ui_elems.rebuild_layout = false;

  pthread_mutex_unlock(&submitted_ui_elems._mutex);
}

static void _blank_render_elems(Blank_Backend *backend,
                                Blank_RenderableUiElement *render_elems,
                                Blank_RenderCommand **render_cmds) {
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
    if (render_elem_func != NULL) {
      render_elem_func(render_elem, render_ctx);
    }
  }
}

static Rectangle *debug_rects = NULL;

void *blank_render_thread_run(void *args) {
  ASSERT_RENDER_THREAD()
  struct render_thread_args *render_args = args;

  debug_rects = array_new(Rectangle, &HEAP_ALLOCATOR);

  Blank_Backend backend = {0};
  blank_backend_init(&backend, render_args->backend,
                     BACKEND_INIT_RENDER_THREAD);

  backend.window_init_func(&backend, &render_args->backend.init_state);

  i32 prev_width = blank_screen_width(&backend);
  i32 prev_height = blank_screen_height(&backend);

  Blank_RenderCommand *render_cmds =
      array_new_capacity(Blank_RenderCommand, 2, &HEAP_ALLOCATOR);
  Blank_RenderableUiElement *render_elems =
      array_new_capacity(Blank_RenderableUiElement, 2, &HEAP_ALLOCATOR);

  log_debug("Arr ptr: %p, ptr ptr: %p", render_cmds, &render_cmds);

  log_debug("cmds: %zu", ((_InternalArrayHeader *)render_cmds - 1)->item_size);

  bool rebuild_layout = false;

  while (!blank_window_should_close(&backend)) {
    array_clear(render_cmds);

    _blank_handle_resize(&backend, &prev_width, &prev_height);

    blank_clear_screen(&render_cmds, blank_color_make(245, 245, 245, 255));

    pthread_mutex_lock(&submitted_ui_elems._mutex);
    rebuild_layout = submitted_ui_elems.rebuild_layout;
    pthread_mutex_unlock(&submitted_ui_elems._mutex);

    if (rebuild_layout) {
      array_clear(debug_rects);
      _blank_rebuild_layout(&backend, &render_elems);
    }

    if (render_elems != NULL) {
      _blank_render_elems(&backend, render_elems, &render_cmds);
    }

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

Blank_Size _blank_impl_linear_layout_min_size(const Blank_UiLayout *layout,
                                              Blank_UiElement *elems) {
  ASSERT_RENDER_THREAD()
  Blank_LayoutOrientation orientation = layout->layout_data.linear.orientation;
  i32 width = 0;
  i32 height = 0;

  Blank_UiElement *elem;
  array_foreach(elems, elem) {
    Blank_Size size = elem->min_size_func(elem);
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

  if (elems_amount == 0)
    return;

  if (context.container_width <= 0)
    context.container_width = 1;
  if (context.container_height <= 0)
    context.container_height = 1;

  log_debug("Linear layout w: %d, h: %d", context.container_width,
            context.container_height);

  Blank_LayoutOrientation orientation = layout->layout_data.linear.orientation;

  log_debug("orientation: %s, container-x: %d, container-width: %d",
            orientation == BLANK_VERTICAL ? "vertical" : "horizontal",
            context.container_x, context.container_width);

  // Calculate minimum layout size for given elements, remainders...
  Blank_Size layout_min_size = layout->min_size_func(layout, *elems);

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

  if (rem_width < 0)
    rem_width = 0;
  if (rem_height < 0)
    rem_height = 0;

  log_debug("Remainder: width %d, height %d", rem_width, rem_height);

  i32 unsplit_rem_width = rem_width % elems_amount;
  i32 unsplit_rem_height = rem_height % elems_amount;

  i32 extra_width = (rem_width - unsplit_rem_width) / elems_amount;
  i32 extra_height = (rem_height - unsplit_rem_height) / elems_amount;

  i32 x_offset = context.container_x;
  i32 y_offset = context.container_y;

  // Scale every element by the calculated amount
  Blank_UiElement *elem;
  array_foreach(*elems, elem) {
    MinUiElemSizeFunc min_elem_size_func = elem->min_size_func;
    Blank_Size min_size = min_elem_size_func(elem);

    Blank_RenderableUiElement render_elem = {
        .elem = *elem,
        .width = min_size.width + extra_width,
        .height = min_size.height + extra_height,
    };

    if (orientation == BLANK_HORIZONTAL) {
      render_elem.width += unsplit_rem_width > 0 ? 1 : 0;
      unsplit_rem_width -= 1;
      render_elem.height = max(render_elem.height, context.container_height);
    }

    if (orientation == BLANK_VERTICAL) {
      render_elem.width = max(render_elem.width, context.container_width);
      render_elem.height += unsplit_rem_height > 0 ? 1 : 0;
      unsplit_rem_height -= 1;
    }

    render_elem.x = x_offset;
    render_elem.y = y_offset;

    if (render_elem.elem.elem_type == BLANK_ELEM_GROUP) {
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
