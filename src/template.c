#include "graphics.h"
#include "bitmap.h"

static image_t *tmpl_create(int width, int height)
{
  int bytes_per_line = ((width + 3) & ~3) * sizeof(T);
  image_t *img;

  img = (image_t *)malloc(sizeof(image_t) + bytes_per_line * height);
  if (!img) {
    fprintf(stderr, "Memory exhausted.\n");
    exit(EXIT_FAILURE);
  }
  img->width = width;
  img->height = height;
  img->bytes_per_line = bytes_per_line;
  img->mask = 0;
  img->data = (char *)&img[1];
  return img;
}

static char *tmpl_pixel_at(image_t *dst, int x, int y)
{
  return dst->data + x * sizeof(T) + y * dst->bytes_per_line;
}

static image_t *tmpl_load(const char *filename)
{
  BITMAPFILEHEADER bmpfile;
  BITMAPINFOHEADER bmpinfo;
  FILE *fp;
  image_t *img = NULL;
  unsigned char *data, *flip = NULL;
  int i;

  fp = fopen(filename, "rb");
  if (!fp) {
    perror(filename);
    return NULL;
  }
  if (fread(&bmpfile, sizeof(bmpfile), 1, fp) != 1 ||
      bmpfile.bfType != 0x4d42 ||
      fread(&bmpinfo, sizeof(bmpinfo), 1, fp) != 1) {
    fprintf(stderr, "%s: doesn't seems to be a .bmp file.\n", filename);
    goto done;
  }
  if (bmpinfo.biCompression != BI_RGB ||
      (bmpinfo.biBitCount != 8 &&
       bmpinfo.biBitCount != 4)) {
    fprintf(stderr, "%s: not RGB or 4bpp/8bpp format.\n", filename);
    goto done;
  }
  img = tmpl_create(bmpinfo.biWidth, bmpinfo.biHeight);
  if (bmpinfo.biBitCount == 4) {
    /* 16色ビットマップ */
    pixel_t pixels[16];
    unsigned rgb[16];
    int H = bmpinfo.biHeight;

    if (fread(rgb, sizeof(rgb), 1, fp) != 1) {
      fprintf(stderr, "%s: can't read RGB table.\n", filename);
      goto done;
    }
    flip = (unsigned char *)malloc(bmpinfo.biWidth * H);
    if (!flip) {
      fprintf(stderr, "%s: memory exhausted.\n", filename);
      exit(EXIT_FAILURE);
    }
    fseek(fp, bmpfile.bfOffBits, SEEK_SET);
    if (fread(flip, H * bmpinfo.biWidth / 2, 1, fp) != 1) {
      fprintf(stderr, "%s: can't read bitmap data.\n", filename);
      goto done;
    }
    for (i = 0; i < 16; i++) {
      unsigned R = (rgb[i] >> 16) & 0xff;
      unsigned G = (rgb[i] >>  8) & 0xff;
      unsigned B = (rgb[i] >>  0) & 0xff;
      pixels[i] = find_nearest_color(R, G, B);
    }
    flip += H * bmpinfo.biWidth / 2;
    data = img->data;
    while (H-- > 0) {
      flip -= bmpinfo.biWidth / 2;
      for (i = 0; i < bmpinfo.biWidth / 2; i++) {
#if (BITS_PER_PIXEL != 24)
	((T *)data)[i*2+0] = (T)pixels[(flip[i]>>4) & 0x0f];
	((T *)data)[i*2+1] = (T)pixels[(flip[i]   ) & 0x0f];
#else
	((T *)data)[i*2+0] = ((uu_t *)(&pixels[(flip[i]>>4) & 0x0f]))->uint24;
	((T *)data)[i*2+1] = ((uu_t *)(&pixels[(flip[i]   ) & 0x0f]))->uint24;
#endif
      }
      data += img->bytes_per_line;
    }
  } else {
    /* 256色ビットマップ */
    int H = bmpinfo.biHeight;
    flip = (unsigned char *)malloc(bmpinfo.biWidth * H);
    if (!flip) {
      fprintf(stderr, "%s: memory exhausted.\n", filename);
      exit(EXIT_FAILURE);
    }
    fseek(fp, bmpfile.bfOffBits, SEEK_SET);
    if (fread(flip, bmpinfo.biWidth * bmpinfo.biHeight, 1, fp) != 1) {
      fprintf(stderr, "%s: can't read bitmap data.\n", filename);
      goto done;
    }
    flip += bmpinfo.biWidth * H;
    data = img->data;
    while (H-- > 0) {
      flip -= bmpinfo.biWidth;
      for (i = 0; i < bmpinfo.biWidth; i++) {
#if (BITS_PER_PIXEL != 24)
	((T *)data)[i] = (T)colors[flip[i]].pixel;
#else
	((T *)data)[i] = ((uu_t *)(&colors[flip[i]].pixel))->uint24;
#endif
      }
      data += img->bytes_per_line;
    }
  }
#if (BITS_PER_PIXEL != 24)
  img->mask = (unsigned)(((T *)img->data)[0]);
#else
  {
    uu_t t;
    t.uint32 = 0;
    t.uint24 = ((T *)img->data)[0];
    img->mask = t.uint32;
  }
#endif
done:
  free(flip);
  fclose(fp);
  return img;
}

static int tmpl_save(const char *filename, image_t *image)
{
#if 1
  return 0;
#else
  BITMAPFILEHEADER bmpfile;
  BITMAPINFOHEADER bmpinfo;
  FILE *fp;
  int i, image_size, error = 0;
  char *bits, *flip;

  fp = fopen(filename, "wb");
  if (fp == NULL) {
    perror(filename);
    return 1;
  }

  image_size = image->width * image->height;
  
  bmpfile.bfType = 0x4d42;
  bmpfile.bfSize = image_size;
  bmpfile.bfReserved1 = 0;
  bmpfile.bfReserved2 = 0;
  bmpfile.bfOffBits = sizeof(bmpfile)
                    + sizeof(bmpinfo)
                    + sizeof(unsigned) * 256;

  bmpinfo.biSize   = sizeof(bmpinfo);
  bmpinfo.biWidth  = image->width;
  bmpinfo.biHeight = image->height;
  bmpinfo.biPlanes = 1;
  bmpinfo.biBitCount = 8;
  bmpinfo.biCompression = BI_RGB;
  bmpinfo.biSizeImage = 0;
  bmpinfo.biXPelsPerMeter = 0;
  bmpinfo.biYPelsPerMeter = 0;
  bmpinfo.biClrUsed = 256;
  bmpinfo.biClrImportant = 0;

  flip = (char *)malloc(image_size);
  bits = image->bits;
  
  /* 上下反転する */
  for (i = 0; i < bmpinfo.biHeight; i++)
    {
      char *dst = flip + bmpinfo.biWidth * i;
      char *src = bits + bmpinfo.biWidth * (bmpinfo.biHeight - i - 1);
      memcpy(dst, src, bmpinfo.biWidth);
    }
  
  /* ファイルへの書き込み */
  if (fwrite(&bmpfile, sizeof(bmpfile), 1, fp) != 1 ||
      fwrite(&bmpinfo, sizeof(bmpinfo), 1, fp) != 1 ||
      fwrite(__colormap, sizeof(unsigned) * 256, 1, fp) != 1 ||
      fwrite(flip, image_size, 1, fp) != 1) {
    error = 1;
  }
  free(flip);
  fclose(fp);
  return error;
#endif
}

static void tmpl_fill(image_t *dst, int x, int y, int w, int h, pixel_t pixel)
{
  char *d;

  w = min(dst->width  - x, w);
  h = min(dst->height - y, h);
  if (w <= 0 || h <= 0) {
    return;
  }
  d = dst->data + x * sizeof(T) + y * dst->bytes_per_line;

  while (h-- > 0) {
    int i;
    for (i = 0; i < w; i++) {
#if (BITS_PER_PIXEL != 24)
      ((T *)d)[i] = (T)pixel;
#else
      uu_t t;
      t.uint32 = (unsigned)pixel;
      ((T *)d)[i] = t.uint24;
#endif
    }
    d += dst->bytes_per_line;
  }
}

static void tmpl_draw(image_t *dst, int x, int y, image_t *src)
{
  int dx, dy;
  int sx, sy;
  int W, H;
  char *d, *s;

  if (x < 0) { sx = -x; dx = 0; } else { dx = x; sx = 0; }
  if (y < 0) { sy = -y; dy = 0; } else { dy = y; sy = 0; }
  W = min(dst->width  - dx, src->width  - sx);
  H = min(dst->height - dy, src->height - sy);
  if (W <= 0 || H <= 0) {
    return;
  }
  d = dst->data + dx * sizeof(T) + dy * dst->bytes_per_line;
  s = src->data + sx * sizeof(T) + sy * src->bytes_per_line;

  while (H-- > 0) {
    T *dd = (T *)d;
    T *ss = (T *)s;
    int i;
    for (i = 0; i < W; i++, dd++, ss++) {
      *dd = *ss;
    }
    d += dst->bytes_per_line;
    s += src->bytes_per_line;
  }
}

static void tmpl_sprite(image_t *dst, int x, int y, image_t *src)
{
  int dx, dy;
  int sx, sy;
  int W, H;
  char *d, *s;
  unsigned mask;

  if (x < 0) { sx = -x; dx = 0; } else { dx = x; sx = 0; }
  if (y < 0) { sy = -y; dy = 0; } else { dy = y; sy = 0; }
  W = min(dst->width  - dx, src->width  - sx);
  H = min(dst->height - dy, src->height - sy);
  if (W <= 0 || H <= 0) {
    return;
  }
  d = dst->data + dx * sizeof(T) + dy * dst->bytes_per_line;
  s = src->data + sx * sizeof(T) + sy * src->bytes_per_line;
  mask = src->mask;
  
  while (H-- > 0) {
    T *dd = (T *)d;
    T *ss = (T *)s;
    int i;
    for (i = 0; i < W; i++, dd++, ss++) {
#if (BITS_PER_PIXEL != 24)
      if (mask != (unsigned)*ss) {
        *dd = *ss;
      }
#else
      uu_t t;
      t.uint32 = 0;
      t.uint24 = *ss;
      if (mask != t.uint32) {
        *dd = t.uint24;
      }
#endif
    }
    d += dst->bytes_per_line;
    s += src->bytes_per_line;
  }
}

static void tmpl_inverse(image_t *dst, int x, int y, image_t *src)
{
  int dx, dy;
  int sx, sy;
  int W, H;
  char *d, *s;
  unsigned mask;

  if (x < 0) { sx = -x; dx = 0; } else { dx = x; sx = 0; }
  if (y < 0) { sy = -y; dy = 0; } else { dy = y; sy = 0; }
  W = min(dst->width  - dx, src->width  - sx);
  H = min(dst->height - dy, src->height - sy);
  if (W <= 0 || H <= 0) {
    return;
  }
  d = dst->data + dx * sizeof(T) + dy * dst->bytes_per_line;
  s = src->data + sx * sizeof(T) + sy * src->bytes_per_line;
  mask = src->mask;
  
  while (H-- > 0) {
    T *dd = (T *)d;
    T *ss = (T *)s;
    int i;
    for (i = 0; i < W; i++, dd++, ss++) {
#if (BITS_PER_PIXEL != 24)
      if (mask != (unsigned)*ss) {
        *dd = ~*dd;
      }
#else
      uu_t t;
      t.uint32 = 0;
      t.uint24 = *ss;
      if (mask != t.uint32) {
        t.uint32 = ~t.uint32;
        *dd = t.uint24;
      }
#endif
    }
    d += dst->bytes_per_line;
    s += src->bytes_per_line;
  }
}

static void tmpl_scroll(image_t *dst, int dot)
{
  if (dot == 0) {
    return;
  }
  tmpl_draw(dst, 0, dot, dst);
  if (dot < 0) {
    tmpl_fill(dst, 0, dst->height + dot, dst->width, -dot, black_pixel);
  } else {
    tmpl_fill(dst, 0, 0, dst->width, dot, black_pixel);
  }
}

static void tmpl_plane(image_t *dst, int x, int y, image_t *src, pixel_t pixel)
{
  int dx, dy;
  int sx, sy;
  int W, H;
  char *d, *s;
  unsigned mask;
  
  if (x < 0) { sx = -x; dx = 0; } else { dx = x; sx = 0; }
  if (y < 0) { sy = -y; dy = 0; } else { dy = y; sy = 0; }
  W = min(dst->width  - dx, src->width  - sx);
  H = min(dst->height - dy, src->height - sy);
  if (W <= 0 || H <= 0) {
    return;
  }
  d = dst->data + dx * sizeof(T) + dy * dst->bytes_per_line;
  s = src->data + sx * sizeof(T) + sy * src->bytes_per_line;
  mask = src->mask;
  
  while (H-- > 0) {
    T *dd = (T *)d;
    T *ss = (T *)s;
    int i;
    for (i = 0; i < W; i++, dd++, ss++) {
#if (BITS_PER_PIXEL != 24)
      if (mask != (unsigned)*ss) {
        *dd = (T)pixel;
      }
#else
      uu_t t;
      t.uint32 = 0;
      t.uint24 = *ss;
      if (mask != t.uint32) {
	t.uint32 = (uint32_t)pixel;
        *dd = t.uint24;
      }
#endif
    }
    d += dst->bytes_per_line;
    s += src->bytes_per_line;
  }
}
