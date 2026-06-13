#include "../include/blank.h"
#include <pthread.h>
#include "lilc/log.h"
#include <raylib.h>

static Blank_UiRenderer blank_renderer = {0};

void blank_begin(const Blank_InitState *state, void (*backend_init_func)(Blank_Backend *backend)) {
  backend_init_func(&blank_renderer.backend);
  blank_renderer.backend.window_init_func(&blank_renderer.backend, state);
}

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

static void *blank_render_thread_run(void *args) {
  while (!WindowShouldClose()) {
    BeginDrawing();
    {
      log_debug("Hii");
      ClearBackground(RAYWHITE);
      //blank_screen_clear(blank_color_make(0, 255, 0, 255));
    }
    EndDrawing();
  }
  return NULL;
}

static void *blank_app_thread_run(void *arg) {
  void (*app_run_func)(void) = arg;

  app_run_func();

  return NULL;
}

void blank_run(void (*app_run_func)(void)) {
  pthread_t render_thread;
  pthread_t app_thread;

  if (pthread_create(&render_thread, NULL, blank_render_thread_run, NULL)) {
    log_error("[BLANK] Failed to create blank render thread\n");
    return;
  }

  if (pthread_create(&app_thread, NULL, blank_app_thread_run, app_run_func)) {
    log_error("[BLANK] Failed to create blank app thread\n");
    return;
  }

  pthread_join(app_thread, NULL);
  pthread_join(render_thread, NULL);
}

void blank_end() {

}

void blank_screen_clear(Blank_Color color) {
  blank_renderer.backend.clear_screen_func(&blank_renderer.backend, color);
}
