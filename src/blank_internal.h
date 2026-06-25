#pragma once

#include "../include/blank.h"

typedef struct {
  Blank_UiElement *ui_elements;
  Blank_UiLayout ui_layout;
  bool rebuild_layout;

  pthread_mutex_t _mutex;
} SubmittedUiElements;

extern SubmittedUiElements submitted_ui_elems;

typedef void (*BackendInitFunc)(Blank_Backend *backend,
                                Blank_BackendInitStage stage);
typedef void (*AppRunFunc)(Blank_UiState *state);

typedef struct {
  BackendInitFunc backend_init_func;
  Blank_InitState init_state;
} BackendPrototype;

void blank_backend_init(Blank_Backend *backend,
                                      BackendPrototype proto_backend,
                                      Blank_BackendInitStage stage);

struct render_thread_args {
  BackendPrototype backend;
};

struct app_thread_args {
  BackendPrototype backend;
  AppRunFunc app_run_func;
};
