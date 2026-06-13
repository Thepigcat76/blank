#include "../include/blank.h"
#include "lilc/log.h"
#include <pthread.h>
#include <raylib.h>

extern void raylib_backend_init(Blank_Backend *backend);

static void app_run(void) {
  log_debug("Running app");
}

int main(void) {
  log_info("Started blank debug process");

  Blank_InitState init_state = {0};

  blank_window_title(&init_state, "Blank Debug Window");
  blank_window_size(&init_state, 800, 800);
  blank_window_resizeable(&init_state);

  blank_begin(&init_state, raylib_backend_init);

  blank_run(app_run);

  blank_end();
}