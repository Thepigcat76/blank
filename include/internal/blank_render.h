#pragma once

#include "blank_backend.h"
#include "blank_shared.h"
#include "blank_ui_elems.h"
#include <pthread.h>
#include <stddef.h>

typedef struct blank_render_ui_elem {
  Blank_UiElement elem;

  i32 x;
  i32 y;
  i32 width;
  i32 height;
} Blank_RenderableUiElement;

typedef void (*RenderUiElemFunc)(const Blank_RenderableUiElement *elem, Blank_RenderContext render_ctx);

extern RenderUiElemFunc ui_elem_render_functions[];
