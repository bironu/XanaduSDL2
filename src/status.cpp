#include "xanadu.h"
#include "status.h"

#define MAX_STATUS_LINE		17	/* ステータス画面の行数 */

#define status_draw_string(row, col, s, pixel) \
  (draw_text(clip_status, (col) * 16, (row) * 16, (s), (pixel)))

void status_erase_line(int row)
{
  int status_width = rect_status.width;
  int x, y = row * 16;
  for (x = 0; x < status_width; x += 16) {
    draw_image(clip_status, x, y, &pattern_status);
  }
  update_region(rect_status.x, rect_status.y + y, status_width, 16);
}

void status_refresh(int in_battle)
{
  status_draw_text(0, 0, user.status.name, red_pixel);
  status_update_rank();
  status_draw_text(2, 0, "Hit Point", red_pixel);
  status_update_HP(white_pixel);
  status_draw_text(4, 0, "Gold", red_pixel);
  status_update_gold();

  /* 戦闘中？ */
  if (in_battle) {
    status_erase_line(6);
  } else {
    status_draw_text(6, 0, "Food", red_pixel);
    status_update_food();
    status_draw_text(8, 0, "Experience", red_pixel);
    status_update_experience();
    status_draw_text(11, 0, "Equipment", red_pixel);
    status_update_equipment();
  }
}

void status_update_rank(void)
{
  const char *name = user.status.fighter.rank >= user.status.wizard.rank
    ? fighter_rank[user.status.fighter.rank].name
    : wizard_rank[user.status.wizard.rank].name;
  
  status_draw_text(1, 0, name, white_pixel);
}

void status_update_HP(pixel_t pixel)
{
  status_draw_integer(3, 5, user.status.HP, pixel);
}

void status_update_gold(void)
{
  status_draw_integer(5, 5, user.status.gold, white_pixel);
}

void status_update_food(void)
{
  status_draw_integer(7, 5, user.status.food, white_pixel);
}

void status_update_experience(void)
{
  status_draw_integer( 9, 5, user.status.fighter.EXP, white_pixel);
  status_draw_integer(10, 5, user.status.wizard.EXP, white_pixel);
}

void status_update_equipment(void)
{
  int i;
  
  for (i = 0; i < 5; i++) {
    const char *name = goods_data[i][user.equipment[i]].name;
    status_draw_text(i + 12, 0, name, white_pixel);
  }
}

void status_draw_integer(int row, int col, int n, pixel_t pixel)
{
  if (n >= 0) {
    char buf[16];
    sprintf(buf, "%07d", n);
    status_draw_text(row, col, buf, pixel);
  } else {
    status_draw_text(row, col, "0000000", red_pixel);
  }
}

void status_draw_text(int row, int col, const char *s, pixel_t pixel)
{
  status_erase_line(row);
  draw_text(clip_status, col * 16, row * 16, s, pixel);
}

/* 品物のリストを表示する */
void status_list_goods(int goods_type, unsigned display_bits)
{
  const goods_t *gd;
  int i;
  int row = 0;

  gd = goods_data[goods_type];

  for (i = 0; i < MAX_GOODS; i++, gd++) {
    status_erase_line(i);
    if (display_bits & (1u << i)) {
      char buf[3];
      buf[0] = 'A' + i;
      buf[1] = ':';
      buf[2] = '\0';
      status_draw_string(row, 0, buf, red_pixel);
      status_draw_string(row, 2, gd->name, white_pixel);
      row++;
    }
  }
}

/* ユーザーの在庫リストを表示する */
void status_user_goods(int goods_type)
{
  unsigned bits = 0u;
  int i;

  for (i = 0; i < MAX_GOODS; i++)
    if (user.inventory[goods_type][i].stock > 0)
      bits |= 1u << i;

  status_list_goods(goods_type, bits);
}

#define status_draw_long(row, col, n, color) {	\
  sprintf(buf, "%07d", (n));			\
  status_draw_string(row, col, buf, color);	\
}

#define status_draw_short(row, col, n, color) {	\
  sprintf(buf, "%03d", (n));			\
  status_draw_string(row, col, buf, color);	\
}

/* ユーザーのステータスを表示する */
void status_user_status(void)
{
  char buf[BUFSIZ];
  int i;

  for (i = 0; i < MAX_STATUS_LINE; i++)
    status_erase_line(i);
  
  status_draw_string(0, 0, user.status.name, white_pixel);
  status_draw_string(1, 0.5,
                     fighter_rank[user.status.fighter.rank].name, red_pixel);
  status_draw_string(2, 0.5,
                     wizard_rank[user.status.wizard.rank].name, red_pixel);

  status_draw_string(4, 0, "HP",     red_pixel);
  status_draw_string(5, 0, "Max.HP", red_pixel);
  status_draw_string(6, 0, "Gold",   red_pixel);
  status_draw_string(7, 0, "STRENG", red_pixel);
  status_draw_string(8, 0, "MAGIC",  red_pixel);
  status_draw_string(9, 0, "DEFEND", red_pixel);
  
  status_draw_long(4, 7.5, user.status.HP, white_pixel);
  status_draw_long(5, 7.5, user.status.max_HP, white_pixel);
  status_draw_long(6, 7.5, user.status.gold, white_pixel);
  status_draw_long(7, 7.5, user_attack_point(), white_pixel);
  status_draw_long(8, 7.5, user_magic_point(0), white_pixel);
  status_draw_long(9, 7.5, user_defend_point(GUARD_FRONT), white_pixel);

  status_draw_string(11, 0.5, "STR", red_pixel);
  status_draw_string(11, 8.5, "INT", red_pixel);
  status_draw_string(12, 0.5, "WIS", red_pixel);
  status_draw_string(12, 8.5, "CHR", red_pixel);
  status_draw_string(13, 0.5, "DEX", red_pixel);
  status_draw_string(13, 8.5, "AGL", red_pixel);
  status_draw_string(14, 0.5, "MGR", red_pixel);
  status_draw_string(14, 8.5, "KRM", red_pixel);
  
  status_draw_short(11,  3.5, user_STR(), white_pixel);
  status_draw_short(11, 11.5, user_INT(), white_pixel);
  status_draw_short(12,  3.5, user_WIS(), white_pixel);
  status_draw_short(12, 11.5, user_CHR(), white_pixel);
  status_draw_short(13,  3.5, user_DEX(), white_pixel);
  status_draw_short(13, 11.5, user_AGL(), white_pixel);
  status_draw_short(14,  3.5, user_MGR(), white_pixel);
  status_draw_short(14, 11.5, user_KRM(), white_pixel);

  status_draw_string(15, 8.5, "CRN", red_pixel);
  status_draw_short(15, 11.5, user.status.CRN, white_pixel);

  if (!in_scenario2()) {
    status_draw_string(15, 0.5, "KEY", red_pixel);
    status_draw_string(16, 0.5, "ELX", red_pixel);
    status_draw_short(15, 3.5, user.status.KEY, white_pixel);
    status_draw_short(16, 3.5, user.status.ELX, white_pixel);
  } else {
    /* scenario 2 */
    status_draw_string(15, 0.5, "ELX", red_pixel);
    status_draw_short(15, 3.5, user.status.ELX, white_pixel);
  }
}
