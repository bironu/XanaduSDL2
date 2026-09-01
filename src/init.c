#include "xanadu.h"

static image_t _clip_main;
static image_t _clip_message;
static image_t _clip_status;
static image_t _clip_shrine;
static image_t _clip_user_guage;
static image_t _clip_boss_guage;
static image_t _clip_endingroll;

image_t *clip_overall;
image_t *clip_main    = &_clip_main;
image_t *clip_message = &_clip_message;
image_t *clip_status  = &_clip_status;
image_t *clip_shrine  = &_clip_shrine;
image_t *clip_user_guage = &_clip_user_guage;
image_t *clip_boss_guage = &_clip_boss_guage;
image_t *clip_endingroll = &_clip_endingroll;

const rectangle_t rect_overall    = {   0,   0, 640, 400 };
const rectangle_t rect_main       = {  16,  16, 360, 360 };
const rectangle_t rect_message    = { 392, 304, 240,  80 };
const rectangle_t rect_status     = { 392,  16, 240, 272 };
const rectangle_t rect_shrine     = {  16,  96, 608, 240 };
const rectangle_t rect_user_guage = {  56,  32, 240,  40 };
const rectangle_t rect_boss_guage = { 384,  32, 240,  40 };
const rectangle_t rect_endingroll = {  80,  96, 560, 240 };

image_t fonts[128];
image_t frame_user[10];
image_t frame_monsters[N_MONSTERS][4];
image_t frame_magics[N_MAGICS * 2];
image_t frame_tiles[N_TILES];
image_t frame_goods[N_GOODS];
image_t frame_brownbox[4];
image_t frame_whitebox[4];
image_t frame_specials[N_SPECIALS];
image_t mask_damaged;
image_t pattern_guage;
image_t pattern_status;
image_t *visual_image;

int draw_text(image_t *dst, int x, int y, const char *s, pixel_t pixel)
{
  int w = dst->width;
  int n = 0;
  for (; x < w && *s; x += 16, s++, n++) {
    if ((*s & 0x7f) > 0x20) {
      draw_plane(dst, x, y, &fonts[*(unsigned char *)s], pixel);
    }
  }
  return max(1, n);
}

/* colors[]とclip_overallの初期化はプラットフォーム依存 */
static int init(void)
{
  image_t *font_base, *magic_base, *goods_base, *pattern_base, *special_base;
  image_t *damage_mask;
  int i;

  if (!clip_overall) {
    return 1;
  }
  
  black_pixel = find_nearest_color(0x00, 0x00, 0x00);
  white_pixel = find_nearest_color(0xff, 0xff, 0xff);
  red_pixel   = find_nearest_color(0xff, 0x00, 0x00);
  blue_pixel  = find_nearest_color(0x00, 0x00, 0xff);
  
  font_base = load_image("../bmp/user/font.bmp");
  if (!font_base) {
    return 1;
  }
  for (i = 0; i < 128; i++) {
    subsection_image(font_base, i * 16, 0, 16, 16, &fonts[i]);
  }
  
  subsection_image(clip_overall,  16,  16, 360, 360, clip_main);
  subsection_image(clip_overall, 392, 304, 240,  80, clip_message);
  subsection_image(clip_overall, 392,  16, 240, 272, clip_status);
  subsection_image(clip_overall,  16,  96, 608, 240, clip_shrine);
  subsection_image(clip_overall,  56,  32, 240,  40, clip_user_guage);
  subsection_image(clip_overall, 384,  32, 240,  40, clip_boss_guage);
  subsection_image(clip_overall,  80,  96, 560, 240, clip_endingroll);
  
  magic_base = load_image(IMAGE_DIR "/user/magic.bmp");
  for (i = 0; i < N_MAGICS * 2; i++) {
    subsection_image(magic_base, i * 16, 0, 16, 16, &frame_magics[i]);
  }

  special_base = load_image(IMAGE_DIR "/user/effect.bmp");
  for (i = 0; i < N_SPECIALS; i++) {
    subsection_image(special_base, i * 40, 0, 40, 40, &frame_specials[i]);
  }
  
  goods_base = load_image(IMAGE_DIR "/user/goods.bmp");
  for (i = 0; i < N_GOODS; i++) {
    subsection_image(goods_base, i * 40, 0, 40, 40, &frame_goods[i]);
  }
  for (i = 0; i < 4; i++) {
    int n;
    n = index_whitebox[i];
    subsection_image(goods_base, n * 40, 0, 40, 40, &frame_whitebox[i]);
    n = index_brownbox[i];
    subsection_image(goods_base, n * 40, 0, 40, 40, &frame_brownbox[i]);
  }
  
  pattern_base = load_image(IMAGE_DIR "/user/pattern.bmp");
  subsection_image(pattern_base,  0, 0, 16, 16, &pattern_status);
  subsection_image(pattern_base, 16, 0, 16, 16, &pattern_guage);
  
  damage_mask = load_image(IMAGE_DIR "/user/damage.bmp");
  subsection_image(damage_mask,  0,  0, 40, 40, &mask_damaged);
  
  if (init_bgm() == 0) {
  }
  
  if (init_se() == 0) {
    se_load(SE_USER_HIT,	se_data.user_hit[0]);
    se_load(SE_MAGIC_HIT,	se_data.magic_hit);
    se_load(SE_MAGIC_FAILED,	se_data.magic_failed);
    se_load(SE_DAMAGED,		se_data.damaged);
    se_load(SE_TRAPPED,		se_data.trapped);
    se_load(SE_MONSTER_DEAD,	se_data.monster_dead);
    se_load(SE_OPEN_BOX,	se_data.open_box);
    se_load(SE_TREASURE,	se_data.treasure);
    se_load(SE_GET,		se_data.get);
    se_load(SE_GET_POISON,	se_data.get_poison);
    se_load(SE_LOST_KEY,	se_data.lost_key);
    se_load(SE_CAST_NEEDLE,	se_data.cast[0]);
    se_load(SE_CAST_MITTAR,	se_data.cast[1]);
    se_load(SE_CAST_DELUGE,	se_data.cast[2]);
    se_load(SE_CAST_FIRE,	se_data.cast[3]);
    se_load(SE_CAST_THUNDER,	se_data.cast[4]);
    se_load(SE_CAST_POISON,	se_data.cast[5]);
    se_load(SE_CAST_CORROSION,	se_data.cast[6]);
    se_load(SE_CAST_TILTE,	se_data.cast[7]);
    se_load(SE_CAST_DEATH,	se_data.cast[8]);
    se_load(SE_USE_ITEM,	se_data.item[0]);
  }
  
  load_user_image();
  init_level(0, NULL);
  
  load_background(IMAGE_DIR "/user/frame.bmp");

  return 0;
}
