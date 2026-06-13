#include "backend.h"
#include "internal/blank_render.h"
#include "internal/blank_init.h"

typedef struct {
  size_t id;
} Blank_UiElement;

typedef struct {
  Blank_Backend backend;

  Blank_UiElement *ui_elements;
} Blank_UiRenderer;

void blank_begin(const Blank_InitState *state, void (*backend_init_func)(Blank_Backend *backend));

void blank_run(void (*app_run_func)(void));

void blank_end(void);
