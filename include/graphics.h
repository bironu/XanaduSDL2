#ifndef graphics_H
#define graphics_H

#include "sdl/SDLImage.h"
#include "sdl/SDLColor.h"
#include "geo/Rect.h"
#include <memory>

#ifdef __GNUC__
#define min(a, b)	((a) < (b) ? (a) : (b))
#define max(a, b)	((a) < (b) ? (b) : (a))
#endif

/* ビットマップの生成・読み込み・描画。旧image_t/graphic_methods_t(8/16/24/32bpp別の
   関数ポインタテーブル)は廃止し、SDL_::Image(SDL_Surfaceのラッパ)を直接使う */

std::shared_ptr<SDL_::Image> create_image(int width, int height);
std::shared_ptr<SDL_::Image> load_image(const char *filename);

void draw_image(std::shared_ptr<SDL_::Image> dst, int x, int y, std::shared_ptr<SDL_::Image> src);
void fill_image(std::shared_ptr<SDL_::Image> dst, int x, int y, int w, int h, const SDL_::Color &color);
void draw_sprite(std::shared_ptr<SDL_::Image> dst, int x, int y, std::shared_ptr<SDL_::Image> src);
void scroll_image(std::shared_ptr<SDL_::Image> dst, int dot);
void inverse_image(std::shared_ptr<SDL_::Image> dst, int x, int y, std::shared_ptr<SDL_::Image> mask);

int draw_text(std::shared_ptr<SDL_::Image> dst, int x, int y, const char *s, const SDL_::Color &color);

#endif /* graphics_H */
