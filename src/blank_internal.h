#pragma once

#include "../include/blank.h"
#include <pthread.h>

typedef struct {
  Blank_UiElement *ui_elements;
  Blank_UiLayout ui_layout;
  bool rebuild_layout;

  pthread_mutex_t _mutex;
} SubmittedUiElements;

struct blank_ui_state {
  Blank_UiLayout ui_layout;
  Blank_Backend *_backend;

  Blank_UiElement *elements;

  bool _window_closed;
  bool _window_resized;

  pthread_mutex_t _mutex;
};

extern SubmittedUiElements submitted_ui_elems;

typedef void (*BackendInitFunc)(Blank_Backend *backend);
typedef void (*AppRunFunc)(Blank_UiState *state);

typedef struct {
  BackendInitFunc backend_init_func;
  Blank_InitState init_state;
} BackendPrototype;

void blank_backend_init(Blank_Backend *backend, BackendPrototype proto_backend);

struct render_thread_args {
  BackendPrototype backend;
};

struct app_thread_args {
  BackendPrototype backend;
  AppRunFunc app_run_func;
};
