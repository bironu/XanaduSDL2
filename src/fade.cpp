#include "fade.h"
#include <SDL3/SDL_surface.h>
#include <algorithm>
#include <cstdint>

namespace {

/* 11ステップに分けて市松状にfade_R/G/Bでマスクした色へ寄せていく
   (元のfade__8bpp/fade_16bpp/fade_24bpp/fade_32bppを1本化したもの) */
void fade_apply(std::shared_ptr<SDL_::Image> dst, int x, int y,
                std::shared_ptr<SDL_::Image> src, unsigned fade_R, unsigned fade_G, unsigned fade_B, int step)
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

} // namespace

XanaduFade::XanaduFade()
	: dst_()
	, x_(0)
	, y_(0)
	, source_()
	, r_(0)
	, g_(0)
	, b_(0)
	, step_(kSteps)
{
}

void XanaduFade::start(std::shared_ptr<SDL_::Image> dst, int x, int y,
                        std::shared_ptr<SDL_::Image> img, unsigned rgb)
{
	dst_ = dst;
	x_ = x;
	y_ = y;
	source_ = nullptr;
	if (dst && img) {
		SDL_Surface *converted = SDL_ConvertSurface(img->get(), dst->get()->format);
		if (converted) {
			source_ = std::make_shared<SDL_::Image>(converted);
		}
	}
	if (!source_) {
		source_ = img;
	}
	r_ = (rgb >> 16) & 0xff;
	g_ = (rgb >>  8) & 0xff;
	b_ = (rgb      ) & 0xff;
	step_ = 0;
}

bool XanaduFade::step()
{
	if (isDone() || !source_) {
		return false;
	}
	fade_apply(dst_, x_, y_, source_, r_, g_, b_, step_);
	++step_;
	return !isDone();
}
