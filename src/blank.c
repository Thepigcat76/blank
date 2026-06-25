#include <raylib.h>
#define _POSIX_C_SOURCE 199309L
#include "../include/blank.h"
#include "lilc/log.h"
#include "lilc/numbers.h"
#include <pthread.h>
#include <time.h>

#include "blank_internal.h"

inline Blank_Color blank_color_make(u8 red, u8 green, u8 blue, u8 alpha) {
  return (red << 24) | (green << 16) | (blue << 8) | alpha;
}

inline u8 blank_color_red(Blank_Color color) { return (color >> 24) & 0xFF; }

inline u8 blank_color_green(Blank_Color color) { return (color >> 16) & 0xFF; }

inline u8 blank_color_blue(Blank_Color color) { return (color >> 8) & 0xFF; }

inline u8 blank_color_alpha(Blank_Color color) { return color & 0xFF; }

inline void blank_wait(int miliseconds) {
  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = miliseconds * 1000000L;
  nanosleep(&ts, NULL);
}

inline void blank_backend_init(Blank_Backend *backend,
                                      BackendPrototype proto_backend,
                                      Blank_BackendInitStage stage) {
  proto_backend.backend_init_func(backend, stage);
}

pthread_t render_thread;

extern void *blank_app_thread_run(void *args);

extern void *blank_render_thread_run(void *args);

void blank_start(Blank_InitState state, BackendInitFunc backend_init_func,
                 AppRunFunc app_run_func) {
  pthread_t app_thread;

  BackendPrototype proto_backend = {
      .init_state = state,
      .backend_init_func = backend_init_func,
  };

  struct render_thread_args render_thread_arg = {.backend = proto_backend};

  if (pthread_create(&render_thread, NULL, blank_render_thread_run,
                     &render_thread_arg)) {
    log_error("[BLANK] Failed to create blank render thread\n");
    return;
  }

  log_info("Render thread created with id: %zu", render_thread);

  struct app_thread_args app_thread_arg = {
      .backend = proto_backend,
      .app_run_func = app_run_func,
  };

  if (pthread_create(&app_thread, NULL, blank_app_thread_run,
                     &app_thread_arg)) {
    log_error("[BLANK] Failed to create blank app thread\n");
    return;
  }

  log_info("App thread created with id: %zu", app_thread);

  pthread_join(app_thread, NULL);
  pthread_join(render_thread, NULL);
}