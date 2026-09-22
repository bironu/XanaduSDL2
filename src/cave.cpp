#include "cave.h"
#include "user.h"
#include "animation.h"
#include "resources/Resources.h"
#include "resources/ImageId.h"

namespace {

void update_background(void)
{
  if (!visual_image) {
    fill_image(clip_main, 0, 0, clip_main->getWidth(), clip_main->getHeight(), SDL_::Color::BLACK);
  } else {
    draw_image(clip_main, 0, 0, visual_image);
  }
  update_region(rect_main.x, rect_main.y, rect_main.width, rect_main.height);
}

} // namespace

void play_cave(int to_level, std::function<void()> onResume)
{
  save_user();
  user.environment.dungeon_level = to_level;
  init_level(to_level, user_path.empty() ? nullptr : user_path.c_str());

  Resources::instance().loadImage(ImageId::picture_cave);
  visual_image = Resources::instance().getImage(ImageId::picture_cave);

  emit_message("Enter-Cave");

  // 歩いていくユーザーの後ろ姿
  static animation_frame_t frames[20];
  for (int i = 0; i < 20; i++) {
    frames[i].image = frame_user[battle_frame_user[8] + i % 2];
    frames[i].x = 160;
    frames[i].y = 320 - i * 8;
  }

  play_animation(clip_main, frames, 20, update_background, [onResume]() {
    Resources::instance().unloadImage(ImageId::picture_cave);
    if (onResume) {
      onResume();
    }
  });
}
