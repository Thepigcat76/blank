#include "../include/backends/raylib_backend.h"
#include "../include/blank.h"
#include "../include/blank/ui_elems.h"
#include "../include/blank/ui_layouts.h"
#include "lilc/log.h"
#include <pthread.h>

#define FONT_SIZE 16

typedef struct {
  u32 cur_page;
} AppState;

#define RAWR_BUTTON_UID 1

static void app_ui_rebuild(AppState *app, Blank_UiState *state) {
  blank_ui_begin(state, BLANK_COZY_BLACK,
                 BLANK_LINEAR_LAYOUT({
                     .padding = 10,
                     .orientation = BLANK_VERTICAL,
                 }));

  Blank_UiElemGroup header_group = {.layout = BLANK_LINEAR_LAYOUT({
                                        .padding = 10,
                                        .orientation = BLANK_HORIZONTAL,
                                    })};

  Blank_Color btn_bg_color = blank_color_make(235, 235, 34, 255);

  blank_ui_group(&header_group, BLANK_BUTTON(RAWR_BUTTON_UID, "First Button",
                                             false, BLANK_WHITE, btn_bg_color));

  blank_ui_group(&header_group, BLANK_BUTTON(0, app->cur_page == 0 ? "Third Button" : "Other button", false,
                                             BLANK_WHITE, btn_bg_color));
  blank_ui_group(&header_group,
                 BLANK_IMAGE(0, "assets/image.png", true, 0.25f));
  blank_ui_group(&header_group, BLANK_BUTTON(0, "rawr Button", false,
                                             BLANK_WHITE, btn_bg_color));

  blank_ui_submit(blank_group(header_group));

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

    u64 clicked_elem;
    if (blank_elem_clicked(state, BLANK_MOUSE_BUTTON_RIGHT, &clicked_elem)) {
      log_debug("elem clicked: %zu", clicked_elem);
      app.cur_page = (app.cur_page + 1) % 3;
      app_ui_rebuild(&app, state);
    }

    blank_wait(100);
  }
}

int main(void) {
  log_info("Started blank debug process");

  Blank_InitState init_state = {0};

  blank_window_title(&init_state, "Blank Debug Window");
  blank_window_size(&init_state, 800, 800);
  blank_window_resizeable(&init_state);

  blank_start(init_state, raylib_backend_init, raylib_backend_deinit, app_run);
}