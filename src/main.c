#include "../include/backends/raylib_backend.h"
#include "../include/blank.h"
#include "../include/blank/ui_elems.h"
#include "../include/blank/ui_layouts.h"
#include "lilc/log.h"
#include <pthread.h>

#define FONT_SIZE 16

typedef struct {
  i32 pos;
} AppState;

static void app_ui_rebuild(AppState *app, Blank_UiState *state) {
  blank_ui_begin(state, BLANK_LINEAR_LAYOUT({
                            .padding = 10,
                            .orientation = BLANK_HORIZONTAL,
                        }));

  blank_ui_submit(blank_button("First Button", false, NULL));

  Blank_UiElemGroup example_group = {
      .layout = BLANK_LINEAR_LAYOUT({
          .padding = 5,
          .orientation = BLANK_VERTICAL,
      }),
  };

  blank_ui_group(&example_group, blank_button("Third Button", false, NULL));
  blank_ui_group(&example_group, blank_button("rawr Button", false, NULL));
  blank_ui_group(&example_group, blank_button("Another Button", false, NULL));
  blank_ui_group(&example_group, blank_button("rat Button", false, NULL));
  blank_ui_group(&example_group, blank_button("rate Button", false, NULL));

  Blank_UiElemGroup sec_group = {
      .layout = BLANK_LINEAR_LAYOUT({
          .padding = 5,
          .orientation = BLANK_HORIZONTAL,
      }),
  };

  blank_ui_group(&sec_group, blank_button("tz Button", false, NULL));
  blank_ui_group(&sec_group, blank_button("zt Button", false, NULL));

  Blank_UiElemGroup third_group = {
      .layout = BLANK_LINEAR_LAYOUT({
          .padding = 5,
          .orientation = BLANK_VERTICAL,
      }),
  };

  blank_ui_group(&third_group, blank_button("inner tz Button", false, NULL));
  blank_ui_group(&third_group, blank_button("inner zt Button", false, NULL));

  Blank_UiElemGroup fourth_group = {
      .layout = BLANK_LINEAR_LAYOUT({
          .padding = 5,
          .orientation = BLANK_HORIZONTAL,
      }),
  };

  blank_ui_group(&fourth_group,
                 blank_button("inner 2x tz Button", false, NULL));
  blank_ui_group(&fourth_group,
                 blank_button("inner 2x zt Button", false, NULL));

  blank_ui_group(&third_group, blank_group(fourth_group));

  blank_ui_group(&sec_group, blank_group(third_group));

  blank_ui_group(&example_group, blank_group(sec_group));

  blank_ui_submit(blank_group(example_group));

  blank_ui_submit(blank_button("Second Button", false, NULL));

  blank_ui_submit(blank_button("Third Button", false, NULL));

  blank_ui_end();
}

static void app_run(Blank_UiState *state) {
  AppState app = {0};

  app_ui_rebuild(&app, state);

  while (!blank_window_closed(state)) {
    if (blank_window_resized(state)) {
      log_info("Window resized");
      app_ui_rebuild(&app, state);
    }

    blank_wait(1000);
  }
}

int main(void) {
  log_info("Started blank debug process");

  Blank_InitState init_state = {0};

  blank_window_title(&init_state, "Blank Debug Window");
  blank_window_size(&init_state, 800, 800);
  blank_window_resizeable(&init_state);

  blank_start(init_state, raylib_backend_init, app_run);
}