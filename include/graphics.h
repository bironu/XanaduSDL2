#ifndef graphics_H
#define graphics_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __GNUC__
#define min(a, b)	((a) < (b) ? (a) : (b))
#define max(a, b)	((a) < (b) ? (b) : (a))

#define PACKED __attribute__((packed))
#else
#define PACKED
#endif

#ifdef __FreeBSD__
#include <sys/types.h>
#else
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;			
typedef unsigned int uint32_t;
#endif

typedef struct { char c[3]; } PACKED uint24_t;

typedef union {
  uint24_t	uint24;
  uint32_t	uint32;
} uu_t;

typedef unsigned long pixel_t;

/* イメージ(兼クリップ用ビューポート)の定義 */
typedef struct {
  int		width;
  int		height;
  int		bytes_per_line;
  unsigned	mask;
  char *	data;
} image_t;

/* カラーの定義(RGB各成分は8ビット精度) */
typedef struct {
  unsigned	red;
  unsigned	green;
  unsigned	blue;
  pixel_t	pixel;
} color_t;

extern pixel_t black_pixel;
extern pixel_t white_pixel;
extern pixel_t red_pixel;
extern pixel_t blue_pixel;

/* グラフィクス処理関数 */
typedef struct {
  unsigned bits_per_pixel;
  image_t *(*create)(int width, int height);
  image_t *(*load)(const char *filename);
  int (*save)(const char *filename, image_t *s);
  char *(*pixel_at)(image_t *dest, int x, int y);
  void (*fill)(image_t *dest, int x, int y, int w, int h, pixel_t pixel);
  void (*draw)(image_t *dest, int x, int y, image_t *s);
  void (*sprite)(image_t *dest, int x, int y, image_t *s);
  void (*inverse)(image_t *dest, int x, int y, image_t *s);
  void (*scroll)(image_t *dest, int dot);
  void (*plane)(image_t *dest, int x, int y, image_t *s, pixel_t pixel);
} graphic_methods_t;

extern color_t colors[256];
extern graphic_methods_t graphic_methods;

extern image_t *null_image;

extern int init_graphics(int bits_per_pixel);

extern pixel_t find_nearest_color(unsigned red, unsigned green, unsigned blue);
extern void subsection_image(image_t *s, int x, int y, int w, int h, image_t *dest);

extern int draw_text(image_t *dst, int x, int y, const char *s, pixel_t pixel);

#define create_image(width, height) \
  ((*graphic_methods.create)((width), (height)))
#define load_image(filename) \
  ((*graphic_methods.load)(filename))
#define save_image(filename, img) \
  ((*graphic_methods.save)((filename), (img))
#define pixel_at(dst, x, y) \
  ((*graphic_methods.pixel_at)((dst), (x), (y)))
#define draw_image(dst, x, y, src) \
  ((*graphic_methods.draw)((dst), (x), (y), (src)))
#define fill_image(dst, x, y, w, h, pixel) \
  ((*graphic_methods.fill)((dst), (x), (y), (w), (h), (pixel)))
#define draw_sprite(dst, x, y, src) \
  ((*graphic_methods.sprite)((dst), (x), (y), (src)))
#define scroll_image(dst, dot) \
  ((*graphic_methods.scroll)((dst), (dot)))
#define inverse_image(dst, x, y, mask) \
  ((*graphic_methods.inverse)((dst), (x), (y), (mask)))
#define draw_plane(dst, x, y, s, pixel) \
  ((*graphic_methods.plane)((dst), (x), (y), (s), (pixel)))

/* ある種のピクセル形式からRGB成分を取り出すマクロ */
#define rgb_R(n)	(((n) >> 16) & 0xff)
#define rgb_G(n)	(((n) >>  8) & 0xff)
#define rgb_B(n)	(((n)      ) & 0xff)

#define rgb2pixel16(rgb) \
  (((rgb_R(rgb) & 0xf8) << 7) |\
   ((rgb_G(rgb) & 0xf8) << 2) |\
   ((rgb_B(rgb) & 0xf8) >> 3))

#define rgb2pixel24(rgb) (rgb)
#define rgb2pixel32(rgb) (rgb)

#endif /* graphics_H */
