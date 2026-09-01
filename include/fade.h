#ifndef fade_H
#define fade_H

extern void fade_enter(void);
extern void fade_leave(void);

extern int init_fade(image_t *clip, int x, int y, image_t *img, unsigned rgb);
extern void init_pixel_table(void);

#endif /* fade_H */