#include "graphics.h"

color_t colors[256];
graphic_methods_t graphic_methods;

pixel_t black_pixel;
pixel_t white_pixel;
pixel_t red_pixel;
pixel_t blue_pixel;

static image_t _null_image;
image_t *null_image = &_null_image;

void subsection_image(image_t *orig, int x, int y, int w, int h, image_t *img)
{
  if (!orig || !orig->data) {
    *img = *null_image;
  } else {
    *img = *orig;
    x = max(0, x);
    y = max(0, y);
    w = min(orig->width  - x, w);
    h = min(orig->height - y, h);
    if (w <= 0 || h <= 0) {
      *img = *null_image;
    } else {
      img->width  = w;
      img->height = h;
      img->data = pixel_at(orig, x, y);
    }
  }
}

pixel_t find_nearest_color(unsigned red, unsigned green, unsigned blue)
{
  unsigned min_error = ~0;
  unsigned long pixel = 0;
  int i;
  for (i = 0; i < 256; i++) {
    unsigned error = abs(colors[i].red - red) + abs(colors[i].green - green)
      + abs(colors[i].blue - blue);
    if (error == 0) {
      pixel = colors[i].pixel;
      break;
    } else if (error < min_error) {
      min_error = error;
      pixel = colors[i].pixel;
    }
  }
  return pixel;
}

int init_graphics(int bits_per_pixel)
{
  extern graphic_methods_t graphic_methods__8bpp;
  extern graphic_methods_t graphic_methods_16bpp;
  extern graphic_methods_t graphic_methods_24bpp;
  extern graphic_methods_t graphic_methods_32bpp;

  switch (bits_per_pixel) {
  case  8: graphic_methods = graphic_methods__8bpp; break;
  case 16: graphic_methods = graphic_methods_16bpp; break;
  case 24: graphic_methods = graphic_methods_24bpp; break;
  case 32: graphic_methods = graphic_methods_32bpp; break;
  default: return 1;
  }
  return 0;
}
