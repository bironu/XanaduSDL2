#include "xanadu.h"

#define FADE_INTERVAL	50

static image_t *fade_clip;
static image_t *fade_image;
static int fade_x;
static int fade_y;
static unsigned fade_R;
static unsigned fade_G;
static unsigned fade_B;
static int fade_step;

typedef uint8_t pixel_table_t[256][256];
static pixel_table_t *fade_pixels; /* for 8bpp */

static void fade_loop(void);
static void fade__8bpp(image_t *dst, int x, int y, image_t *src, int step);
static void fade_16bpp(image_t *dst, int x, int y, image_t *src, int step);
static void fade_24bpp(image_t *dst, int x, int y, image_t *src, int step);
static void fade_32bpp(image_t *dst, int x, int y, image_t *src, int step);

void init_pixel_table(void)
{
  if (graphic_methods.bits_per_pixel == 8 && !fade_pixels) {
    int i, j;
    fade_pixels = (pixel_table_t *)malloc(sizeof(pixel_table_t));
    for (i = 0; i < 256; i++) {
      (*fade_pixels)[i][i] = i;
      for (j = i + 1; j < 256; j++) {
        (*fade_pixels)[i][j] =
          (*fade_pixels)[j][i] =
            find_nearest_color(colors[i].red   & colors[j].red,
                               colors[i].green & colors[j].green,
                               colors[i].blue  & colors[j].blue);
      }
    }
  }
}

int init_fade(image_t *clip, int x, int y, image_t *img, unsigned rgb)
{
  fade_clip = clip;
  fade_x = x;
  fade_y = y;
  fade_image = img;
  fade_R = rgb_R(rgb);
  fade_G = rgb_G(rgb);
  fade_B = rgb_B(rgb);
  fade_step = 0;

  init_pixel_table();
  
  return CONTEXT_FADE;
}

void fade_enter(void)
{
  set_timer(FADE_INTERVAL, fade_loop);
}

void fade_leave(void)
{
  kill_timer();
}

void fade_loop(void)
{
  if (fade_image && fade_step < 11) {
    switch (graphic_methods.bits_per_pixel) {
    case  8: fade__8bpp(fade_clip, fade_x, fade_y, fade_image, fade_step); break;
    case 16: fade_16bpp(fade_clip, fade_x, fade_y, fade_image, fade_step); break;
    case 24: fade_24bpp(fade_clip, fade_x, fade_y, fade_image, fade_step); break;
    case 32: fade_32bpp(fade_clip, fade_x, fade_y, fade_image, fade_step); break;
    }
    fade_step++;
    /* We assumed fade_clip as clip_overall anyway. */
    update_region(fade_x, fade_y, fade_image->width, fade_image->height);
  } else {
    resume_context();
  }
}

void fade__8bpp(image_t *dst, int x, int y, image_t *src, int step)
{
  char *d, *s;
  int W, H;
  int i;
  uint8_t mask = find_nearest_color(fade_R, fade_G, fade_B);

  W = min(dst->width  - x, src->width);
  H = min(dst->height - y, src->height);
  if (W <= 0 || H <= 0) {
    return;
  }
  d = pixel_at(dst, x, y);
  s = pixel_at(src, 0, 0);

  while (H-- > 0) {
    uint8_t *dd = (uint8_t *)d;
    uint8_t *ss = (uint8_t *)s;
    for (i = step; i < W; i += 11) {
      if (dd[i] != ss[i]) {
        dd[i] = (*fade_pixels)[mask][ss[i]];
      }
    }
    d += dst->bytes_per_line;
    s += src->bytes_per_line;
    step = (step + 1) % 11;
  }
}

void fade_16bpp(image_t *dst, int x, int y, image_t *src, int step)
{
  char *d, *s;
  int W, H;
  int i;
  uint16_t mask = ((fade_R & 0xf8) << 8) | ((fade_G & 0xfc) << 3)
    | ((fade_B & 0xf8) >> 3);

  W = min(dst->width  - x, src->width);
  H = min(dst->height - y, src->height);
  if (W <= 0 || H <= 0) {
    return;
  }
  d = pixel_at(dst, x, y);
  s = pixel_at(src, 0, 0);

  while (H-- > 0) {
    uint16_t *dd = (uint16_t *)d;
    uint16_t *ss = (uint16_t *)s;
    for (i = step; i < W; i += 11) {
      if (dd[i] != ss[i]) {
        dd[i] = ss[i] & mask;
      }
    }
    d += dst->bytes_per_line;
    s += src->bytes_per_line;
    step = (step + 1) % 11;
  }
}

void fade_24bpp(image_t *dst, int x, int y, image_t *src, int step)
{
  char *d, *s;
  int W, H;
  int i;

  W = min(dst->width  - x, src->width);
  H = min(dst->height - y, src->height);
  if (W <= 0 || H <= 0) {
    return;
  }
  d = pixel_at(dst, x, y);
  s = pixel_at(src, 0, 0);

  while (H-- > 0) {
    uint24_t *dd = (uint24_t *)d;
    uint24_t *ss = (uint24_t *)s;
    for (i = step; i < W; i += 11) {
      if (dd[i].c[0] != ss[i].c[0] ||
	  dd[i].c[1] != ss[i].c[1] ||
	  dd[i].c[2] != ss[i].c[2]) {
        dd[i].c[0] = ss[i].c[0] & fade_B;
	dd[i].c[1] = ss[i].c[1] & fade_G;
	dd[i].c[2] = ss[i].c[2] & fade_R;
      }
    }
    d += dst->bytes_per_line;
    s += src->bytes_per_line;
    step = (step + 1) % 11;
  }
}

void fade_32bpp(image_t *dst, int x, int y, image_t *src, int step)
{
  char *d, *s;
  int W, H;
  int i;
  uint32_t mask = (fade_R << 16) | (fade_G << 8) | fade_B;

  W = min(dst->width  - x, src->width);
  H = min(dst->height - y, src->height);
  if (W <= 0 || H <= 0) {
    return;
  }
  d = pixel_at(dst, x, y);
  s = pixel_at(src, 0, 0);

  while (H-- > 0) {
    uint32_t *dd = (uint32_t *)d;
    uint32_t *ss = (uint32_t *)s;
    for (i = step; i < W; i += 11) {
      if (dd[i] != ss[i]) {
        dd[i] = ss[i] & mask;
      }
    }
    d += dst->bytes_per_line;
    s += src->bytes_per_line;
    step = (step + 1) % 11;
  }
}

#if 0
static void fade__8bpp(image_t *img, unsigned R, unsigned G, unsigned B);
static void fade_16bpp(image_t *img, unsigned R, unsigned G, unsigned B);
static void fade_24bpp(image_t *img, unsigned R, unsigned G, unsigned B);
static void fade_32bpp(image_t *img, unsigned R, unsigned G, unsigned B);

static void fade_plane(image_t *img, unsigned R, unsigned G, unsigned B)
{
  switch (graphic_methods.bits_per_pixel) {
  case  8: fade__8bpp(img, R, G, B); break;
  case 16: fade_16bpp(img, R, G, B); break;
  case 24: fade_24bpp(img, R, G, B); break;
  case 32: fade_32bpp(img, R, G, B); break;
  }
}

void fade__8bpp(image_t *img, unsigned R, unsigned G, unsigned B)
{
  pixel_t pixels[256];
  int i, w, h;
  char *d;
  
  for (i = 0; i < 256; i++) {
    pixels[i] = find_nearest_color(R & colors[i].red,
                                   G & colors[i].green,
                                   B & colors[i].blue);
  }
  w = img->width;
  h = img->height;
  d = img->data;
  while (h-- > 0) {
    for (i = 0; i < w; i++) {
      d[i] = pixels[d[i]];
    }
    d += img->bytes_per_line;
  }
}

void fade_16bpp(image_t *img, unsigned R, unsigned G, unsigned B)
{
  int i, w, h;
  char *d;
  uint16_t mask = ((R & 0x1f) << 10) | ((G & 0x1f) << 5) | (B & 0x1f);

  w = img->width;
  h = img->height;
  d = img->data;
  while (h-- > 0) {
    uint16_t *dd = (uint16_t *)d;
    for (i = 0; i < w; i++, dd++) {
      *dd = *dd & mask;
    }
    d += img->bytes_per_line;
  }
}

static void fade_24bpp(image_t *img, unsigned R, unsigned G, unsigned B)
{
  int i, w, h;
  char *d;

  w = img->width;
  h = img->height;
  d = img->data;
  while (h-- > 0) {
    char *dd = d;
    for (i = 0; i < w; i++, dd += 3) {
      dd[0] = dd[0] & R;
      dd[1] = dd[1] & G;
      dd[2] = dd[2] & B;
    }
    d += img->bytes_per_line;
  }
}

static void fade_32bpp(image_t *img, unsigned R, unsigned G, unsigned B)
{
  int i, w, h;
  char *d;
  uint32_t mask = ((R & 0xff) << 16) | ((G & 0xff) << 8) | (B & 0xff);

  w = img->width;
  h = img->height;
  d = img->data;
  while (h-- > 0) {
    uint32_t *dd = (uint32_t *)d;
    for (i = 0; i < w; i++, dd++) {
      *dd = *dd & mask;
    }
    d += img->bytes_per_line;
  }
}
#endif
