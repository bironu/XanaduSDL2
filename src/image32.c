#include "graphics.h"

#define BITS_PER_PIXEL	32

typedef uint32_t T;

#include "template.c"

graphic_methods_t graphic_methods_32bpp = {
  BITS_PER_PIXEL,
  tmpl_create,
  tmpl_load,
  tmpl_save,
  tmpl_pixel_at,
  tmpl_fill,
  tmpl_draw,
  tmpl_sprite,
  tmpl_inverse,
  tmpl_scroll,
  tmpl_plane
};
