#ifndef animation_H
#define animation_H

#include "graphics.h"
#include "dungeon.h"

// アニメーションフレームに関する情報を保持する構造体
typedef struct {
  SDL_::SubImage image;			// イメージ
  short		x;			// 水平座標
  short		y;			// 垂直座標
} animation_frame_t;

// コンテキスト保護関数
extern void animation_enter(void);
extern void animation_leave(void);

// 初期化関数
extern int init_animation(std::shared_ptr<SDL_::Image> clip, animation_frame_t *frames, int n_frames,
                          void (*update_background)(void));

// 地形タイルアニメーションの初期化
int init_animation_tile(map_t *tiles, int n_frames, int x, int y,
                        void (*update_background)(void));

#endif // animation_H