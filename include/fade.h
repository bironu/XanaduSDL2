#ifndef fade_H
#define fade_H

#include "graphics.h"

extern void fade_enter(void);
extern void fade_leave(void);

extern int init_fade(std::shared_ptr<SDL_::Image> clip, int x, int y, std::shared_ptr<SDL_::Image> img, unsigned rgb);

#endif /* fade_H */