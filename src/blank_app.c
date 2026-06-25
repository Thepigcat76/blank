#include "../include/blank.h"
#include "lilc/array.h"
#include "lilc/log.h"

#include "blank_internal.h"
#include <pthread.h>

Blank_UiState app_thread_ui_state = {._mutex = PTHREAD_MUTEX_INITIALIZER};

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
  pthread_mutex_lock(&app_thread_ui_state._mutex);
  bool closed = app_thread_ui_state._window_closed;
  pthread_mutex_unlock(&app_thread_ui_state._mutex);
  return closed;
}

inline bool blank_window_resized(Blank_UiState *state) {
  pthread_mutex_lock(&app_thread_ui_state._mutex);
  bool resized = app_thread_ui_state._window_resized;
  pthread_mutex_unlock(&app_thread_ui_state._mutex);
  return resized;
}

void blank_ui_begin(Blank_UiState *state, Blank_UiLayout initial_layout) {
  // if (_internal_current_ui_state != NULL) {
  //   log_error(
  //       "[BLANK] Cannot begin submitting a ui, last one was not submitted");
  //   exit(1);
  // }

  pthread_mutex_lock(&submitted_ui_elems._mutex);
  if (submitted_ui_elems.ui_elements == NULL) {
    submitted_ui_elems.ui_elements =
        array_new_capacity(Blank_UiElement, 1024, &HEAP_ALLOCATOR);
  }
  pthread_mutex_unlock(&submitted_ui_elems._mutex);

  pthread_mutex_lock(&app_thread_ui_state._mutex);

  app_thread_ui_state.ui_layout = initial_layout;
}

void blank_ui_end(void) {
  // if (_internal_current_ui_state == NULL) {
  //   log_error("[BLANK] Cannot submit ui, no ui submission was started");
  //   exit(1);
  // }

  pthread_mutex_lock(&submitted_ui_elems._mutex);

  array_copy(submitted_ui_elems.ui_elements, app_thread_ui_state.elements);
  array_clear(app_thread_ui_state.elements);

  Blank_UiElement *elem;
  array_foreach(submitted_ui_elems.ui_elements, elem) {
    log_debug("Added Elem: %zu", elem->elem_type);
  }

  submitted_ui_elems.rebuild_layout = true;
  submitted_ui_elems.ui_layout = app_thread_ui_state.ui_layout;

  app_thread_ui_state._window_resized = false;

  pthread_mutex_unlock(&submitted_ui_elems._mutex);

  pthread_mutex_unlock(&app_thread_ui_state._mutex);
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

  Blank_Backend backend = {0};
  blank_backend_init(&backend, app_thread_args->backend,
                     BACKEND_INIT_APP_THREAD);

  pthread_mutex_lock(&app_thread_ui_state._mutex);
  app_thread_ui_state = (Blank_UiState){
      ._backend = &backend,
      .elements = array_new_capacity(Blank_UiElement, 1024, &HEAP_ALLOCATOR),
  };
  pthread_mutex_unlock(&app_thread_ui_state._mutex);

  app_thread_args->app_run_func(&app_thread_ui_state);

  return NULL;
}
