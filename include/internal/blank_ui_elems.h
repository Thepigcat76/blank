#pragma once

#include "blank_backend.h"
#include "blank_layouts.h"
#include "blank_shared.h"
#include <pthread.h>
#include <stddef.h>

typedef struct {
  Blank_Backend *backend;
  Blank_RenderCommand **render_commands;
} Blank_RenderContext;

typedef Blank_Size (*MinUiElemSizeFunc)(const struct blank_ui_element *elem);

struct blank_render_ui_elem;

typedef void (*RenderUiElemFunc)(const struct blank_render_ui_elem *elem,
                                 Blank_RenderContext render_ctx);

typedef void (*DeinitUiElemFunc)(struct blank_ui_element *elem);

typedef struct blank_ui_element {
  size_t elem_type;
  void *args;

  MinUiElemSizeFunc min_size_func;
  RenderUiElemFunc render_func;

  DeinitUiElemFunc deinit_func;
} Blank_UiElement;

#define BLANK_ELEM_BUTTON 0
#define BLANK_ELEM_GROUP 1

typedef struct {
  Blank_UiLayout layout;
  Blank_UiElement *ui_elems;
} Blank_UiElemGroup;

Blank_UiElement blank_button(const char *text, bool disabled,
                             OnClickFunc on_click_func);

Blank_UiElement blank_group(Blank_UiElemGroup group);

void blank_ui_group(Blank_UiElemGroup *group, Blank_UiElement elem);

typedef struct {
  const char *text;
  bool disabled;
  OnClickFunc on_click_func;
} Blank_UiElemButton;

#define BLANK_BUTTON()

typedef struct {
  Blank_UiLayout ui_layout;
  Blank_Backend *_backend;

  Blank_UiElement *elements;

  bool _window_closed;
  bool _window_resized;

  pthread_mutex_t _mutex;
} Blank_UiState;

void blank_ui_begin(Blank_UiState *state, Blank_UiLayout initial_layout);

void blank_ui_end(void);

void blank_ui_submit(Blank_UiElement ui_elem);

void blank_ui_button(Blank_UiElement ui_elem, Blank_UiElemButton elem_button);