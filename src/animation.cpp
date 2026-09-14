#include "animation.h"
#include "user.h"

static animation_frame_t *anime_frames;
static int anime_n_frames;
static int anime_current;
static std::shared_ptr<SDL_::Image> anime_clip;
static int anime_as_tile;

static void (*thunk_update_background)(void);

static void animation_loop(void);

int init_animation(std::shared_ptr<SDL_::Image> clip, animation_frame_t *frames, int n_frames,
                   void (*update_background)(void))
{
  anime_frames = frames;
  anime_n_frames = n_frames;
  anime_current = 0;
  anime_clip = clip;
  thunk_update_background = update_background;
  return CONTEXT_ANIMATION;
}

// 地形タイルアニメーションの初期化
int init_animation_tile(map_t *tiles, int n_frames, int x, int y,
                        void (*update_background)(void))
{
  static animation_frame_t *frames;
  int i;

  delete[] frames;
  frames = new animation_frame_t[n_frames];

  for (i = 0; i < n_frames; i++) {
    frames[i].image = frame_tiles[tiles[i]];
    frames[i].x = x;
    frames[i].y = y;
  }
  anime_as_tile = 1;
  return init_animation(clip_main, frames, n_frames, update_background);
}

void animation_enter(void)
{
  set_timer(85, animation_loop);
}

void animation_leave(void)
{
  kill_timer();
  anime_as_tile = 0;
}

void animation_loop(void)
{
  if (anime_current < anime_n_frames) {
    SDL_::SubImage img = anime_frames[anime_current].image;
    int x = anime_frames[anime_current].x;
    int y = anime_frames[anime_current].y;
    
    // 背景を更新
    (*thunk_update_background)();
    
    // フレームの描画
    if (anime_as_tile) {
      draw_image(anime_clip, x, y, img);
    } else {
      draw_sprite(anime_clip, x, y, img);
    }
    anime_current++;
  } else {
    // コンテキストを復帰
    resume_context();
  }
}
