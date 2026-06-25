#include "blank_internal.h"
#include "lilc/array.h"
#include "lilc/assert.h"
#include "lilc/log.h"
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

static void blank_render_button(const Blank_RenderableUiElement *render_elem,
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

static Blank_Size blank_min_size_button(const Blank_UiElement *elem) {
  ASSERT_RENDER_THREAD();

  Blank_UiElemButton *btn = elem->args;
  Blank_Size size = {
      .width = MeasureText(btn->text, 16),
      .height = 16,
  };

  return size;
}

static Blank_Size blank_min_size_group(const Blank_UiElement *elem) {
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

RenderUiElemFunc ui_elem_render_functions[] = {
    [BLANK_ELEM_BUTTON] = blank_render_button,
};
MinUiElemSizeFunc ui_elem_min_size_functions[] = {
    [BLANK_ELEM_BUTTON] = blank_min_size_button,
    [BLANK_ELEM_GROUP] = blank_min_size_group,
};

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
                                  Blank_RenderableUiElement *render_elems) {
  ASSERT(backend != NULL, "Backend is null");
  ASSERT_RENDER_THREAD()

  if (render_elems == NULL)
    return;

  array_clear(render_elems);

  pthread_mutex_lock(&submitted_ui_elems._mutex);

  submitted_ui_elems.ui_layout.rearrange_elems_func(
      &submitted_ui_elems.ui_layout, submitted_ui_elems.ui_elements,
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
    RenderUiElemFunc render_elem_func =
        ui_elem_render_functions[render_elem->elem.elem_type];
    Blank_RenderContext render_ctx = {
        .render_commands = render_cmds,
        .backend = backend,
    };
    if (render_elem_func != NULL) {
      render_elem_func(render_elem, render_ctx);
    }
  }
}

void *blank_render_thread_run(void *args) {
  ASSERT_RENDER_THREAD()
  struct render_thread_args *render_args = args;

  Blank_Backend backend = {0};
  blank_backend_init(&backend, render_args->backend,
                     BACKEND_INIT_RENDER_THREAD);

  backend.window_init_func(&backend, &render_args->backend.init_state);

  i32 prev_width = blank_screen_width(&backend);
  i32 prev_height = blank_screen_height(&backend);

  static Blank_RenderCommand *render_cmds;
  render_cmds = array_new_capacity(Blank_RenderCommand, 2, &HEAP_ALLOCATOR);
  static Blank_RenderableUiElement *render_elems;
  render_elems =
      array_new_capacity(Blank_RenderableUiElement, 2, &HEAP_ALLOCATOR);

  log_debug("Arr ptr: %p, ptr ptr: %p", render_cmds, &render_cmds);

  log_debug("cmds: %zu", ((_InternalArrayHeader *)render_cmds - 1)->item_size);

  while (!blank_window_should_close(&backend)) {
    array_clear(render_cmds);

    _blank_handle_resize(&backend, &prev_width, &prev_height);

    blank_clear_screen(&render_cmds, blank_color_make(245, 245, 245, 255));

    if (submitted_ui_elems.rebuild_layout) {
      _blank_rebuild_layout(&backend, render_elems);
    }

    if (render_elems != NULL) {
      _blank_render_elems(&backend, render_elems, &render_cmds);
    }

    blank_render_cmds(&backend, render_cmds);
  }

  blank_set_window_closed(&app_thread_ui_state);

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
    Blank_Size size = ui_elem_min_size_functions[elem->elem_type](elem);
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
    const Blank_UiLayout *layout, Blank_UiElement *elems,
    Blank_RenderableUiElement *renderable_elems, Blank_LayoutContext context) {
  ASSERT_RENDER_THREAD()
  if (elems == NULL)
    return;

  size_t render_elems_amount = array_len(elems);

  if (render_elems_amount == 0)
    return;

  if (context.container_width <= 0)
    context.container_width = 1;
  if (context.container_height <= 0)
    context.container_height = 1;

  Blank_LayoutOrientation orientation = layout->layout_data.linear.orientation;

  log_debug("orientation: %d, container-x: %d, container-width: %d",
            orientation, context.container_x, context.container_width);

  i32 max_elem_height = context.container_height;
  if (orientation == BLANK_VERTICAL) {
    max_elem_height /= render_elems_amount;
  }
  i32 max_elem_width = context.container_width;
  if (orientation == BLANK_HORIZONTAL) {
    max_elem_width /= render_elems_amount;
  }

  f32 min_width_share = 0;
  f32 min_height_share = 0;

  Blank_Size min_layout_size = layout->min_size_func(layout, elems);

  Blank_UiElement *elem;
  array_foreach(elems, elem) {
    MinUiElemSizeFunc min_elem_size_func =
        ui_elem_min_size_functions[elem->elem_type];
    Blank_Size min_size = min_elem_size_func(elem);

    f32 width_share = min(1.0f, (f32)min_size.width / context.container_width);
    f32 height_share =
        min(1.0f, (f32)min_size.height / context.container_height);

    min_width_share += width_share;
    min_height_share += height_share;

    log_debug("Elem: %zu, Min size - width: %d, height: %d - height share: %f "
              "of container height: %d",
              elem->elem_type, min_size.width, min_size.height, height_share,
              context.container_height);
  }

  // if (orientation == BLANK_HORIZONTAL) {
  //   if (min_width_share < 1.0f) {
  //     f32 unused_width_share = 1.0f - min_width_share;
  //
  //    log_debug("Used width share: %f, Unused width share: %f, Base width: %d,
  //    "
  //              "Scaled width: %d, Container width: %d",
  //              min_width_share, unused_width_share, min_layout_size.width,
  //              (i32)(min_layout_size.width / min_width_share),
  //              context.container_width);
  //  }
  //}

  if (min_width_share <= 0.0001f)
    min_width_share = 1.0f;
  if (min_height_share <= 0.0001f)
    min_height_share = 1.0f;

  i32 x_offset = context.container_x;
  i32 y_offset = context.container_y;

  array_foreach(elems, elem) {
    MinUiElemSizeFunc min_elem_size_func =
        ui_elem_min_size_functions[elem->elem_type];
    Blank_Size min_size = min_elem_size_func(elem);

    log_debug("! - Elem scaled width: %d",
              (i32)(min_size.width / min_width_share));

    f32 width_share = min(1.0f, (f32)min_size.width / context.container_width);
    f32 height_share =
        min(1.0f, (f32)min_size.height / context.container_height);

    Blank_RenderableUiElement render_elem = {
        .elem = *elem,
        .width = min_size.width / min_width_share,
        .height = min_size.height / min_height_share,
    };

    if (orientation == BLANK_HORIZONTAL) {
      render_elem.height = max(render_elem.height, context.container_height);
    }

    if (orientation == BLANK_VERTICAL) {
      render_elem.width = max(render_elem.width, context.container_width);
    }

    render_elem.x = x_offset;
    render_elem.y = y_offset;

    if (render_elem.elem.elem_type == BLANK_ELEM_GROUP) {
      if (width_share <= 0.0001f)
        width_share = 1.0f;
      if (height_share <= 0.0001f)
        height_share = 1.0f;

      log_debug("Height share: %f", height_share);

      log_debug("Group rearranging");
      Blank_UiElemGroup *group = render_elem.elem.args;
      group->layout.rearrange_elems_func(
          &group->layout, group->ui_elems, renderable_elems,
          (Blank_LayoutContext){
              .backend = context.backend,
              .container_x = x_offset,
              .container_y = y_offset,
              .container_width = min_size.width / min_width_share,
              .container_height = min_size.height / min_height_share,
          });
      log_debug("Container height: %d, min height: %d, height share: %f, "
                "scaled width: %d, scaled height: %d",
                context.container_height, min_size.height, min_height_share,
                (i32)(min_size.width / width_share),
                (i32)(min_size.height / min_height_share));
      log_debug("Group end");
    }

    if (orientation == BLANK_HORIZONTAL) {
      x_offset +=
          min_size.width / min_width_share + layout->layout_data.linear.padding;
    }

    if (orientation == BLANK_VERTICAL) {
      y_offset += min_size.height / min_height_share +
                  layout->layout_data.linear.padding;
    }

    array_add(renderable_elems, render_elem);
  }
}
