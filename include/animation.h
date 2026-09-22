#ifndef animation_H
#define animation_H

#include "graphics.h"
#include "dungeon.h"
#include <functional>

// アニメーションフレームに関する情報を保持する構造体
typedef struct {
  SDL_::SubImage image;			// イメージ
  short		x;			// 水平座標
  short		y;			// 垂直座標
} animation_frame_t;

// clip上にframesを1コマずつ(85ms間隔)描画するアニメーションを即座に再生する。
// 画面遷移を伴わないため、専用Sceneには依存しない(GameScene::setGameTimerを
// 使う)。完了したらonComplete(省略可)を呼ぶ。update_backgroundは各コマの
// 描画前に呼ばれる(背景の再描画)
void play_animation(std::shared_ptr<SDL_::Image> clip, animation_frame_t *frames, int n_frames,
                    std::function<void()> update_background, std::function<void()> onComplete = nullptr);

// 地形タイルアニメーション版(clip_main上にdraw_imageで描画)
void play_animation_tile(map_t *tiles, int n_frames, int x, int y,
                         std::function<void()> update_background, std::function<void()> onComplete = nullptr);

#endif // animation_H