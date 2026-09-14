#ifndef bitmap_H
#define bitmap_H

#ifndef _WIN32

#ifdef __GNUC__
#define PACKED __attribute__((packed))
#else
#define PACKED
#endif

typedef struct {
  unsigned short	bfType;
  unsigned		bfSize;
  unsigned short	bfReserved1;
  unsigned short	bfReserved2;
  unsigned		bfOffBits;
} PACKED BITMAPFILEHEADER;

typedef struct {
  unsigned		biSize;
  long			biWidth;
  long			biHeight;
  unsigned short	biPlanes;
  unsigned short	biBitCount;
  unsigned		biCompression;
  unsigned		biSizeImage;
  long			biXPelsPerMeter;
  long			biYPelsPerMeter;
  unsigned		biClrUsed;
  unsigned		biClrImportant;
} PACKED BITMAPINFOHEADER;

#define BI_RGB		0L
#define BI_RLE8		1L
#define BI_RLE4		2L
#define BI_BITFIELDS	3L

#else

#include <windows.h>

#endif // _WIN32

#endif // bitmap_H
