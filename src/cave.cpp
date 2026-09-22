#include "cave.h"
#include "user.h"
#include "animation.h"
#include "resources/Resources.h"
#include "resources/ImageId.h"

static void update_background(void);

int init_cave(int to_level)
{
  save_user();
  user.environment.dungeon_level = to_level;
  init_level(to_level, user_path.empty() ? nullptr : user_path.c_str());

  Resources::instance().loadImage(ImageId::picture_cave);
  visual_image = Resources::instance().getImage(ImageId::picture_cave);
  
  emit_message("Enter-Cave");
  return CONTEXT_CAVE;
}

void cave_enter(void)
{
  static animation_frame_t frames[20];
  int i;
  
  // 歩いていくユーザーの後ろ姿
  for (i = 0; i < 20; i++) {
    frames[i].image = frame_user[battle_frame_user[8] + i % 2];
    frames[i].x = 160;
    frames[i].y = 320 - i * 8;
  }
  switch_context(init_animation(clip_main, frames, 20, update_background));
}

void cave_leave(void)
{
  Resources::instance().unloadImage(ImageId::picture_cave);
}

void update_background(void)
{
  if (!visual_image) {
    fill_image(clip_main, 0, 0, clip_main->getWidth(), clip_main->getHeight(), SDL_::Color::BLACK);
  } else {
    draw_image(clip_main, 0, 0, visual_image);
  }
  update(rect_main);
}
