#pragma once

#include "../blank.h"
#include <stddef.h>

#define BLANK_LAYOUT_ID_CUSTOM 0
#define BLANK_LAYOUT_ID_LINEAR 1

void _blank_impl_linear_layout_rearrange_elems(
    const Blank_UiLayout *layout, Blank_UiElement **elems,
    Blank_RenderableUiElement **renderable_elems, Blank_LayoutContext context);

Blank_Size _blank_impl_linear_layout_min_size_elems(const Blank_UiLayout *layout,
                                              Blank_UiElement *elems);

#define BLANK_LINEAR_LAYOUT(...)                                               \
  (Blank_UiLayout) {                                                           \
    .rearrange_elems_func = _blank_impl_linear_layout_rearrange_elems,         \
    .min_size_elems_func = _blank_impl_linear_layout_min_size_elems,                       \
    __VA_OPT__(.layout_data.linear = __VA_ARGS__)                              \
  }