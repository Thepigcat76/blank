#pragma once

#include "blank_backend.h"
#include "blank_shared.h"

typedef struct {
  i32 container_x;
  i32 container_y;
  // Width and height of current container,
  // If no container-like ui element was
  // created, window size is used here
  i32 container_width;
  i32 container_height;

  Blank_Backend *backend;
} Blank_LayoutContext;

struct blank_ui_element;
struct blank_render_ui_elem;

typedef enum {
  BLANK_VERTICAL,
  BLANK_HORIZONTAL,
} Blank_LayoutOrientation;

typedef struct blank_ui_layout {
  void (*rearrange_elems_func)(const struct blank_ui_layout *layout,
                               struct blank_ui_element *elems,
                               struct blank_render_ui_elem *renderable_elems,
                               Blank_LayoutContext context);
  Blank_Size (*min_size_func)(const struct blank_ui_layout *layout,
                              struct blank_ui_element *elems);
  size_t layout_id;
  union {
    void *custom;
    struct {
      size_t padding;
      Blank_LayoutOrientation orientation;
    } linear;
  } layout_data;
} Blank_UiLayout;

#define BLANK_LAYOUT_ID_CUSTOM 0
#define BLANK_LAYOUT_ID_LINEAR 1

void _blank_impl_linear_layout_rearrange_elems(
    const Blank_UiLayout *layout, struct blank_ui_element *elems,
    struct blank_render_ui_elem *renderable_elems, Blank_LayoutContext context);

Blank_Size _blank_impl_linear_layout_min_size(const Blank_UiLayout *layout,
                                              struct blank_ui_element *elems);

#define BLANK_LINEAR_LAYOUT(...)                                               \
  (Blank_UiLayout) {                                                           \
    .rearrange_elems_func = _blank_impl_linear_layout_rearrange_elems,         \
    .min_size_func = _blank_impl_linear_layout_min_size,                       \
    __VA_OPT__(.layout_data.linear = __VA_ARGS__)                              \
  }