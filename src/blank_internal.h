#pragma once

#include "../include/blank.h"
#include "../include/blank_backend.h"
#include <pthread.h>

typedef struct {
  Blank_UiElement *ui_elements;
  Blank_UiLayout ui_layout;
  bool rebuild_layout;

  Blank_Color bg_color;

  pthread_mutex_t _mutex;
} SubmittedUiElements;

struct blank_ui_state {
  Blank_UiLayout ui_layout;
  Blank_Backend *_backend;

  Blank_UiElement *elements;

  Blank_UiElement clicked_elem;
  i32 clicked_button;

  bool building;

  bool _window_closed;
  bool _window_resized;

  pthread_mutex_t _mutex;
};

extern SubmittedUiElements submitted_ui_elems;

typedef void (*BackendInitFunc)(Blank_Backend *backend);
typedef void (*BackendDeinitFunc)(Blank_Backend *backend);
typedef void (*AppRunFunc)(Blank_UiState *state);

struct render_thread_args {
  Blank_Backend backend;
  Blank_InitState init_state;
};

struct app_thread_args {
  Blank_Backend backend;
  AppRunFunc app_run_func;
};

#define BLANK_SIZE_DYNAMIC_ID 0
#define BLANK_SIZE_FIXED_ID 1
#define BLANK_SIZE_MIN_ID 2
