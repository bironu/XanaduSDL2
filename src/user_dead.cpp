#include "xanadu.h"
#include "user_dead.h"
#include "status.h"
#include "animation.h"
#include "resources/Resources.h"
#include "resources/SoundId.h"
#include "app/Application.h"

namespace {

int g_x;
int g_y;
std::function<void()> g_updateBackground;
std::function<void()> g_onRevive;
bool g_revived;

// 墓標表示後、任意のキー入力で復活/リセットへ進む(旧CONTEXT_ENTER_CHARACTER
// 経由のMessageEnterSceneを介さず、thunk_key_eventを直接使う)
void onDeadKey(int /*c*/)
{
  thunk_key_event = nullptr;
  Resources::instance().unloadSound(SoundId::elixer);

  if (g_revived) {
    user.status.HP = user.status.max_HP;
    user.status.ELX--;

    // SE
    playSound(SoundId::elixer);

    status_update_HP(SDL_::Color::WHITE);
    user_hidden = 0; // ユーザーをまた見えるようにする

    // 死亡演出中に止めていたゲームループ(タイマー・描画)を再開する
    if (g_onRevive) {
      g_onRevive();
    }
  } else {
    reset_context();
  }
}

// 昏倒アニメーション完了後、墓標を表示してキー入力待ちに入る
void showGraveAndWaitKey()
{
  g_updateBackground();
  draw_sprite(clip_main, g_x, g_y, frame_specials[SPECIAL_GRAVE]);

  // 霊薬を持っている？
  g_revived = user.status.ELX > 0;
  if (g_revived) {
    emit_message("Hit to use Elixer");
  } else {
    emit_message("Hit to restart");
  }

  thunk_key_event = onDeadKey;
}

} // namespace

void play_user_dead(int x, int y, std::function<void()> update_background,
                    std::function<void()> onRevive)
{
  static animation_frame_t swoon_frames[8];
  extern const int battle_frame_user[10]; // battle.c
  int i, n;

  g_x = x;
  g_y = y;
  g_updateBackground = std::move(update_background);
  g_onRevive = std::move(onRevive);

  // ユーザーを表示しない
  user_hidden = 1;

  Resources::instance().loadSound(Application::instance().getMixer(), SoundId::elixer);

  // 昏倒フレームを作成
  for (i = 0, n = user.dir; i < 8; i++) {
    switch (n) {
    case 6: case 3: case 9: n = 8; break;
    case 4: case 1: case 7: n = 2; break;
    case 8: case 5:         n = 4; break;
    case 2: default:        n = 6; break;
    }
    swoon_frames[i].image = frame_user[battle_frame_user[n]];
    swoon_frames[i].x = x;
    swoon_frames[i].y = y;
  }

  // 昏倒シーン
  play_animation(clip_main, swoon_frames, 8, g_updateBackground, showGraveAndWaitKey);
}
