#include "xanadu.h"
#include "user_dead.h"
#include "status.h"
#include "animation.h"
#include "resources/Resources.h"
#include "resources/SoundId.h"
#include "app/Application.h"

#define STATE_SWOON		0	// 昏倒
#define STATE_GRAVE		1	// 埋葬

#define STATE_EXIT		-1

static int user_dead_state;
static int user_x;
static int user_y;
static void (*thunk_update_background)(void);
static int user_revived;
static void restore_context(void);

int init_user_dead(int x, int y, void (*update_background)(void))
{
  user_dead_state = STATE_SWOON;
  user_x = x;
  user_y = y;
  thunk_update_background = update_background;
  user_revived = 0;

  // ユーザーを表示しない
  user_hidden = 1;

  return CONTEXT_USER_DEAD;
}

void user_dead_enter(void)
{
  Resources::instance().loadSound(Application::instance().getMixer(), SoundId::elixer);

  switch (user_dead_state) {
  case STATE_SWOON:
    {
      static animation_frame_t swoon_frames[8];
      int i, n;
      extern const int battle_frame_user[10]; // battle.c
      
      user_dead_state = STATE_GRAVE;

      // 昏倒フレームを作成
      for (i = 0, n = user.dir; i < 8; i++) {
        switch (n) {
        case 6: case 3: case 9: n = 8; break;
        case 4: case 1: case 7: n = 2; break;
        case 8: case 5:         n = 4; break;
        case 2: default:        n = 6; break;
        }
        swoon_frames[i].image = frame_user[battle_frame_user[n]];
        swoon_frames[i].x = user_x;
        swoon_frames[i].y = user_y;
      }
      
      // 昏倒シーン
      extend_context(init_animation(clip_main, swoon_frames, 8,
                                    thunk_update_background));
    }
    break;
    
  case STATE_GRAVE:
    {
      user_dead_state = STATE_EXIT;

      // 墓標を表示
      (*thunk_update_background)();
      draw_sprite(clip_main, user_x, user_y, frame_specials[SPECIAL_GRAVE]);

      // 霊薬を持っている？
      if (user.status.ELX > 0) {
        user_revived = 1;
        emit_message("Hit to use Elixer");
        extend_context(init_enter_buffer(CONTEXT_ENTER_CHARACTER, NULL));
      } else {
        emit_message("Hit to restart");
        extend_context(init_enter_buffer(CONTEXT_ENTER_CHARACTER, NULL));
      }
    }
    break;
    
  default:
    // ユーザーは復活した？
    if (user_revived) {
      user.status.HP = user.status.max_HP;
      user.status.ELX--;

      // SE
      playSound(SoundId::elixer);

      status_update_HP(SDL_::Color::WHITE);
      restore_context();
    } else {
      reset_context();
    }
  }
}

void user_dead_leave(void)
{
  Resources::instance().unloadSound(SoundId::elixer);
}

void restore_context(void)
{
  user_hidden = 0; // ユーザーをまた見えるようにする
  resume_context();
}
