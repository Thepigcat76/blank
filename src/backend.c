#include "../include/backend.h"

Blank_Color blank_color_make(u8 red, u8 green, u8 blue, u8 alpha) {
  return (red << 24) | (green << 16) | (blue << 8) | alpha;
}

u8 blank_color_red(Blank_Color color) {
  return (color >> 24) & 0xFF;
}

u8 blank_color_green(Blank_Color color) {
  return (color >> 16) & 0xFF;
}

u8 blank_color_blue(Blank_Color color) {
  return (color >> 8) & 0xFF;
}

u8 blank_color_alpha(Blank_Color color) {
  return color & 0xFF;
}
