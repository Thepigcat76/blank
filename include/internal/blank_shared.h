#pragma once

#include "lilc/numbers.h"

typedef u32 Blank_Color;

Blank_Color blank_color_make(u8 red, u8 green, u8 blue, u8 alpha);

u8 blank_color_red(Blank_Color color);
u8 blank_color_green(Blank_Color color);
u8 blank_color_blue(Blank_Color color);
u8 blank_color_alpha(Blank_Color color);