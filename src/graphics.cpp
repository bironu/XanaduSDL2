#include "graphics.h"
#include <SDL3/SDL_surface.h>
#include <algorithm>
#include <cstring>
#include <cstdint>

std::shared_ptr<SDL_::Image> create_image(int width, int height)
{
  return std::make_shared<SDL_::Image>(width, height);
}

void draw_image(std::shared_ptr<SDL_::Image> dst, int x, int y, std::shared_ptr<SDL_::Image> src)
{
  if (!dst || !src) {
    return;
  }
  // draw_imageは常に不透明合成(旧tmpl_drawはmaskを一切見ない)。
  // srcにcolorkeyが設定済みでも、一時的に無効化してから合成する
  Uint32 savedKey;
  const bool hadKey = SDL_GetSurfaceColorKey(src->get(), &savedKey);
  if (hadKey) {
    SDL_SetSurfaceColorKey(src->get(), false, 0);
  }
  dst->blit(src, x, y);
  if (hadKey) {
    SDL_SetSurfaceColorKey(src->get(), true, savedKey);
  }
}

void fill_image(std::shared_ptr<SDL_::Image> dst, int x, int y, int w, int h, const SDL_::Color &color)
{
  if (dst) {
    dst->fillRect(Rect(x, y, w, h), color);
  }
}

void draw_sprite(std::shared_ptr<SDL_::Image> dst, int x, int y, std::shared_ptr<SDL_::Image> src)
{
  // frame_specials等、まだローダが未移植で常にnullptrのシートが存在するため、
  // draw_image()と同様にnullを許容する(未ロードのスプライトは単に描かれない)
  if (!dst || !src) {
    return;
  }
  dst->blit(src, x, y);
}

void draw_image(std::shared_ptr<SDL_::Image> dst, int x, int y, const SDL_::SubImage &src)
{
  if (!dst || !src.sheet) {
    return;
  }
  // draw_image(shared_ptr<Image>版)と同様、常に不透明合成する
  Uint32 savedKey;
  const bool hadKey = SDL_GetSurfaceColorKey(src.sheet->get(), &savedKey);
  if (hadKey) {
    SDL_SetSurfaceColorKey(src.sheet->get(), false, 0);
  }
  dst->blit(src.sheet, src.rect, x, y);
  if (hadKey) {
    SDL_SetSurfaceColorKey(src.sheet->get(), true, savedKey);
  }
}

void draw_sprite(std::shared_ptr<SDL_::Image> dst, int x, int y, const SDL_::SubImage &src)
{
  if (!dst || !src.sheet) {
    return;
  }
  dst->blit(src.sheet, src.rect, x, y);
}

void scroll_image(std::shared_ptr<SDL_::Image> dst, int dot)
{
  if (!dst || dot == 0) {
    return;
  }

  SDL_Surface *surf = dst->get();
  const int width = surf->w;
  const int height = surf->h;
  const int pitch = surf->pitch;

  dst->lock();
  uint8_t *pixels = static_cast<uint8_t *>(surf->pixels);
  if (dot > 0) {
    std::memmove(pixels + dot * pitch, pixels, (height - dot) * pitch);
  } else {
    const int shift = -dot;
    std::memmove(pixels, pixels + shift * pitch, (height - shift) * pitch);
  }
  dst->unlock();

  if (dot > 0) {
    fill_image(dst, 0, 0, width, dot, SDL_::Color::BLACK);
  } else {
    fill_image(dst, 0, height + dot, width, -dot, SDL_::Color::BLACK);
  }
}

void inverse_image(std::shared_ptr<SDL_::Image> dst, int x, int y, std::shared_ptr<SDL_::Image> mask)
{
  if (!dst || !mask) {
    return;
  }

  const int w = min(dst->getWidth() - x, mask->getWidth());
  const int h = min(dst->getHeight() - y, mask->getHeight());
  if (w <= 0 || h <= 0) {
    return;
  }

  const Uint32 colorKey = mask->getColorKey();

  dst->lock();
  mask->lock();
  SDL_Surface *dstSurf = dst->get();
  SDL_Surface *maskSurf = mask->get();
  for (int row = 0; row < h; ++row) {
    auto *dd = reinterpret_cast<Uint32 *>(static_cast<uint8_t *>(dstSurf->pixels)
                                           + (y + row) * dstSurf->pitch) + x;
    auto *ss = reinterpret_cast<Uint32 *>(static_cast<uint8_t *>(maskSurf->pixels)
                                           + row * maskSurf->pitch);
    for (int col = 0; col < w; ++col) {
      if (ss[col] != colorKey) {
        /* アルファは保持したままRGBのみ反転する(SDL_::Image(int,int)が作る
           サーフェスのバイト順は下位からR,G,B,Aなのでアルファは最上位バイト) */
        dd[col] = (dd[col] & 0xFF000000u) | (~dd[col] & 0x00FFFFFFu);
      }
    }
  }
  mask->unlock();
  dst->unlock();
}
