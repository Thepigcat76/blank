#include "../include/blank.h"
#include "../include/blank/ui_elems.h"
#include "../include/blank_backend.h"
#include "lilc/array.h"
#include "lilc/log.h"

#include "blank_internal.h"
#include <lilc/alloc.h>
#include <lilc/panic.h>
#include <pthread.h>

Blank_UiState app_thread_ui_state = {
    ._mutex = PTHREAD_RWLOCK_INITIALIZER,
    .clicked_button = -1,
};

void blank_window_title(Blank_InitState *state, const char *title) {
  state->title = title;
}

void blank_window_size(Blank_InitState *state, i32 width, i32 height) {
  state->width = width;
  state->height = height;
}

void blank_window_resizeable(Blank_InitState *state) {
  state->resizeable = true;
}

inline bool blank_window_closed(Blank_UiState *state) {
  pthread_rwlock_rdlock(&app_thread_ui_state._mutex);
  bool closed = app_thread_ui_state._window_closed;
  pthread_rwlock_unlock(&app_thread_ui_state._mutex);
  return closed;
}

inline bool blank_window_resized(Blank_UiState *state) {
  pthread_rwlock_rdlock(&app_thread_ui_state._mutex);
  bool resized = app_thread_ui_state._window_resized;
  pthread_rwlock_unlock(&app_thread_ui_state._mutex);
  return resized;
}

void blank_ui_begin(Blank_UiState *state, Blank_Color bg_color, Blank_UiLayout initial_layout) {
  pthread_mutex_lock(&submitted_ui_elems._mutex);
  if (submitted_ui_elems.ui_elements == NULL) {
    submitted_ui_elems.ui_elements =
        array_new_capacity(Blank_UiElement, 1024, &HEAP_ALLOCATOR);
  }
  submitted_ui_elems.bg_color = bg_color;
  pthread_mutex_unlock(&submitted_ui_elems._mutex);

  pthread_rwlock_wrlock(&app_thread_ui_state._mutex);

  app_thread_ui_state.ui_layout = initial_layout;
  app_thread_ui_state.building = true;
}

void blank_ui_end(void) {
  app_thread_ui_state.building = false;

  pthread_mutex_lock(&submitted_ui_elems._mutex);

  array_copy(submitted_ui_elems.ui_elements, app_thread_ui_state.elements);
  array_clear(app_thread_ui_state.elements);

  Blank_UiElement *elem;
  array_foreach(submitted_ui_elems.ui_elements, elem) {
    log_debug("Added Elem: %zu", elem->elem_kind);
  }

  submitted_ui_elems.rebuild_layout = true;
  submitted_ui_elems.ui_layout = app_thread_ui_state.ui_layout;

  app_thread_ui_state._window_resized = false;

  pthread_mutex_unlock(&submitted_ui_elems._mutex);

  pthread_rwlock_unlock(&app_thread_ui_state._mutex);
}

void blank_ui_group(Blank_UiElemGroup *group, Blank_UiElement elem) {
  if (group->ui_elems == NULL) {
    group->ui_elems = array_new(Blank_UiElement, &HEAP_ALLOCATOR);
  }

  array_add(group->ui_elems, elem);
}

void blank_ui_submit(Blank_UiElement ui_elem) {
  array_add(app_thread_ui_state.elements, ui_elem);
}

void *blank_app_thread_run(void *args) {
  struct app_thread_args *app_thread_args = args;

  Blank_Backend backend = app_thread_args->backend;

  pthread_rwlock_wrlock(&app_thread_ui_state._mutex);

  app_thread_ui_state._backend = &backend,
  app_thread_ui_state.elements =
      array_new_capacity(Blank_UiElement, 1024, &HEAP_ALLOCATOR),

  pthread_rwlock_unlock(&app_thread_ui_state._mutex);

  app_thread_args->app_run_func(&app_thread_ui_state);

  return NULL;
}

extern void blank_render_button(const Blank_RenderableUiElement *render_elem,
                                Blank_RenderContext render_ctx);

extern Blank_Size blank_min_size_button(const Blank_UiElement *elem,
                                        Blank_Context ctx);

void blank_deinit_button(Blank_UiElement *elem) { heap_dealloc(elem->args); }

Blank_UiElement blank_ui_button(u64 uid, Blank_SizeConfig size_config, Blank_UiElemButton ui_elem_button) {
  if (!app_thread_ui_state.building) {
    panic("Tried constructing a button while not building ui.");
  }

  if (uid != 0) {
    log_debug("non-zero uid: %zu", uid);
  }

  pthread_mutex_lock(&app_thread_ui_state._backend->ui_elem_alloc_mutex);
  Allocator *ui_elem_alloc = app_thread_ui_state._backend->ui_elem_allocator;
  Blank_UiElemButton *args =
      HEAP_ALLOCATOR.alloc(&HEAP_ALLOCATOR, sizeof(Blank_UiElemButton));
  pthread_mutex_unlock(&app_thread_ui_state._backend->ui_elem_alloc_mutex);

  if (ui_elem_button.text_color == 0) {
    ui_elem_button.text_color = BLANK_BLACK;
  }

  if (ui_elem_button.bg_color == 0) {
    ui_elem_button.bg_color = BLANK_RED;
  }

  *args = ui_elem_button;

  Blank_UiElement ui_elem = {
      .uid = uid,
      .size_config = size_config,
      .elem_kind = BLANK_ELEM_BUTTON,
      .args = args,
      .render_func = blank_render_button,
      .deinit_func = blank_deinit_button,
      .min_size_func = blank_min_size_button,
  };

  return ui_elem;
}

extern void
blank_render_image_elem(const Blank_RenderableUiElement *render_elem,
                        Blank_RenderContext render_ctx);

extern Blank_Size blank_min_size_image_elem(const Blank_UiElement *elem,
                                            Blank_Context ctx);

extern void blank_deinit_image_elem(Blank_UiElement *elem);

Blank_UiElement blank_ui_image(u64 uid, Blank_UiElemImage ui_elem_image) {
  if (!app_thread_ui_state.building) {
    panic("Tried constructing a button while not building ui.");
  }

  if (uid != 0) {
    log_debug("non-zero uid: %zu", uid);
  }

  pthread_mutex_lock(&app_thread_ui_state._backend->ui_elem_alloc_mutex);
  Allocator *ui_elem_alloc = app_thread_ui_state._backend->ui_elem_allocator;
  Blank_UiElemImage *args =
      HEAP_ALLOCATOR.alloc(&HEAP_ALLOCATOR, sizeof(Blank_UiElemImage));
  pthread_mutex_unlock(&app_thread_ui_state._backend->ui_elem_alloc_mutex);

  *args = ui_elem_image;

  Blank_UiElement ui_elem = {
      .uid = uid,
      .elem_kind = BLANK_ELEM_IMAGE,
      .args = args,
      .render_func = blank_render_image_elem,
      .deinit_func = blank_deinit_image_elem,
      .min_size_func = blank_min_size_image_elem,
  };

  return ui_elem;
}

bool blank_elem_clicked(Blank_UiState *state, Blank_MouseButton mouse_button,
                        u64 *clicked_elem_uid) {
  pthread_rwlock_wrlock(&app_thread_ui_state._mutex);
  Blank_MouseButton btn = app_thread_ui_state.clicked_button;

  if (app_thread_ui_state.clicked_button != -1) {
    log_debug("Button: %d", app_thread_ui_state.clicked_button);
  }

  app_thread_ui_state.clicked_button = -1;

  *clicked_elem_uid = app_thread_ui_state.clicked_elem.uid;

  pthread_rwlock_unlock(&app_thread_ui_state._mutex);

  return btn != -1 && btn == mouse_button;
}

Blank_UiElement blank_button(const char *text, bool disabled) {
  Blank_UiElemButton ui_elem_btn = {
      .text = text,
      .disabled = disabled,
  };
  Blank_UiElement ui_elem = {
      .elem_kind = BLANK_ELEM_BUTTON,
      .args = heap_alloc(sizeof(Blank_UiElemButton)),
      .render_func = blank_render_button,
      .deinit_func = blank_deinit_button,
      .min_size_func = blank_min_size_button,
  };
  memcpy(ui_elem.args, &ui_elem_btn, sizeof(Blank_UiElemButton));
  return ui_elem;
}

extern Blank_Size blank_min_size_group(const Blank_UiElement *elem,
                                       Blank_Context ctx);

void blank_deinit_group(Blank_UiElement *elem) { heap_dealloc(elem->args); }

Blank_UiElement blank_group(Blank_UiElemGroup group) {
  Blank_UiElement ui_elem = {
      .elem_kind = BLANK_ELEM_GROUP,
      .args = heap_alloc(sizeof(Blank_UiElemGroup)),
      .min_size_func = blank_min_size_group,
      .deinit_func = blank_deinit_group,
  };
  memcpy(ui_elem.args, &group, sizeof(Blank_UiElemGroup));
  return ui_elem;
}
