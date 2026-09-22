#include "animation.h"
#include "user.h"

namespace {

struct PlayAnimationState {
  animation_frame_t *frames = nullptr;
  int n_frames = 0;
  int current = 0;
  std::shared_ptr<SDL_::Image> clip;
  bool as_tile = false;
  std::function<void()> update_background;
  std::function<void()> onComplete;
};

PlayAnimationState g_playAnim; // 同時に1つしか再生しない

void playAnimationStep()
{
  if (g_playAnim.current >= g_playAnim.n_frames) {
    kill_timer();
    auto onComplete = std::move(g_playAnim.onComplete);
    g_playAnim.onComplete = nullptr;
    if (onComplete) {
      onComplete();
    }
    return;
  }

  const SDL_::SubImage &img = g_playAnim.frames[g_playAnim.current].image;
  const int x = g_playAnim.frames[g_playAnim.current].x;
  const int y = g_playAnim.frames[g_playAnim.current].y;

  g_playAnim.update_background();

  if (g_playAnim.as_tile) {
    draw_image(g_playAnim.clip, x, y, img);
  } else {
    draw_sprite(g_playAnim.clip, x, y, img);
  }
  g_playAnim.current++;
}

void playAnimationImpl(std::shared_ptr<SDL_::Image> clip, animation_frame_t *frames, int n_frames,
                       std::function<void()> update_background, std::function<void()> onComplete, bool as_tile)
{
  g_playAnim.frames = frames;
  g_playAnim.n_frames = n_frames;
  g_playAnim.current = 0;
  g_playAnim.clip = clip;
  g_playAnim.as_tile = as_tile;
  g_playAnim.update_background = std::move(update_background);
  g_playAnim.onComplete = std::move(onComplete);

  set_timer(85, playAnimationStep);
}

} // namespace

void play_animation(std::shared_ptr<SDL_::Image> clip, animation_frame_t *frames, int n_frames,
                    std::function<void()> update_background, std::function<void()> onComplete)
{
  playAnimationImpl(clip, frames, n_frames, std::move(update_background), std::move(onComplete), false);
}

void play_animation_tile(map_t *tiles, int n_frames, int x, int y,
                         std::function<void()> update_background, std::function<void()> onComplete)
{
  static animation_frame_t *frames;

  delete[] frames;
  frames = new animation_frame_t[n_frames];

  for (int i = 0; i < n_frames; i++) {
    frames[i].image = frame_tiles[tiles[i]];
    frames[i].x = x;
    frames[i].y = y;
  }

  playAnimationImpl(clip_main, frames, n_frames, std::move(update_background), std::move(onComplete), true);
}
