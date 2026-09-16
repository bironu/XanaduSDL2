#include "xanadu.h"
#include <SDL3/SDL_surface.h>
#include <algorithm>
#include <cstdint>

#define FADE_INTERVAL	50

static std::shared_ptr<SDL_::Image> fade_clip;
static std::shared_ptr<SDL_::Image> fade_image;
/* fade_imageをfade_clipと同じピクセルフォーマットへ変換したもの。
   生ピクセル比較のためフォーマットを揃える必要がある */
static std::shared_ptr<SDL_::Image> fade_source;
static int fade_x;
static int fade_y;
static unsigned fade_R;
static unsigned fade_G;
static unsigned fade_B;
static int fade_step;

static void fade_loop(void);
static void fade_apply(std::shared_ptr<SDL_::Image> dst, int x, int y,
                       std::shared_ptr<SDL_::Image> src, int step);

int init_fade(std::shared_ptr<SDL_::Image> clip, int x, int y, std::shared_ptr<SDL_::Image> img, unsigned rgb)
{
  fade_clip = clip;
  fade_x = x;
  fade_y = y;
  fade_image = img;
  fade_source = nullptr;
  if (clip && img) {
    SDL_Surface *converted = SDL_ConvertSurface(img->get(), clip->get()->format);
    if (converted) {
      fade_source = std::make_shared<SDL_::Image>(converted);
    }
  }
  fade_R = (rgb >> 16) & 0xff;
  fade_G = (rgb >>  8) & 0xff;
  fade_B = (rgb      ) & 0xff;
  fade_step = 0;

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
    fade_apply(fade_clip, fade_x, fade_y, fade_source ? fade_source : fade_image, fade_step);
    fade_step++;
    // We assumed fade_clip as clip_overall anyway.
    update_region(fade_x, fade_y, fade_image->getWidth(), fade_image->getHeight());
  } else {
    resume_context();
  }
}

/* 11ステップに分けて市松状にfade_R/G/Bでマスクした色へ寄せていく
   (元のfade__8bpp/fade_16bpp/fade_24bpp/fade_32bppを1本化したもの) */
void fade_apply(std::shared_ptr<SDL_::Image> dst, int x, int y,
                std::shared_ptr<SDL_::Image> src, int step)
{
  if (!dst || !src) {
    return;
  }

  const int W = min(dst->getWidth() - x, src->getWidth());
  const int H = min(dst->getHeight() - y, src->getHeight());
  if (W <= 0 || H <= 0) {
    return;
  }

  dst->lock();
  src->lock();

  SDL_Surface *dstSurf = dst->get();
  SDL_Surface *srcSurf = src->get();
  const SDL_PixelFormatDetails *format = SDL_GetPixelFormatDetails(dstSurf->format);
  const Uint32 mask = (fade_R << format->Rshift) | (fade_G << format->Gshift)
                     | (fade_B << format->Bshift) | format->Amask;

  auto *drow = static_cast<uint8_t *>(dstSurf->pixels) + y * dstSurf->pitch + x * 4;
  auto *srow = static_cast<uint8_t *>(srcSurf->pixels);

  for (int row = 0; row < H; ++row) {
    auto *dd = reinterpret_cast<Uint32 *>(drow);
    auto *ss = reinterpret_cast<Uint32 *>(srow);
    for (int i = step; i < W; i += 11) {
      if (dd[i] != ss[i]) {
        dd[i] = ss[i] & mask;
      }
    }
    drow += dstSurf->pitch;
    srow += srcSurf->pitch;
    step = (step + 1) % 11;
  }

  src->unlock();
  dst->unlock();
}
