#include "xanadu.h"
#include "status.h"

#define MAX_STATUS_LINE		17	// ステータス画面の行数

#define status_draw_string(row, col, s, pixel) \
  (draw_text(clip_status, (col) * 16, (row) * 16, (s), (pixel)))

void status_erase_line(int row)
{
  int status_width = rect_status.width;
  int x, y = row * 16;
  for (x = 0; x < status_width; x += 16) {
    draw_image(clip_status, x, y, pattern_status);
  }
  update_region(rect_status.x, rect_status.y + y, status_width, 16);
}

void status_refresh(int in_battle)
{
  status_draw_text(0, 0, user.status.name, SDL_::Color::RED);
  status_update_rank();
  status_draw_text(2, 0, "Hit Point", SDL_::Color::RED);
  status_update_HP(SDL_::Color::WHITE);
  status_draw_text(4, 0, "Gold", SDL_::Color::RED);
  status_update_gold();

  // 戦闘中？
  if (in_battle) {
    status_erase_line(6);
  } else {
    status_draw_text(6, 0, "Food", SDL_::Color::RED);
    status_update_food();
    status_draw_text(8, 0, "Experience", SDL_::Color::RED);
    status_update_experience();
    status_draw_text(11, 0, "Equipment", SDL_::Color::RED);
    status_update_equipment();
  }
}

void status_update_rank(void)
{
  const char *name = user.status.fighter.rank >= user.status.wizard.rank
    ? fighter_rank[user.status.fighter.rank].name
    : wizard_rank[user.status.wizard.rank].name;
  
  status_draw_text(1, 0, name, SDL_::Color::WHITE);
}

void status_update_HP(SDL_::Color pixel)
{
  status_draw_integer(3, 5, user.status.HP, pixel);
}

void status_update_gold(void)
{
  status_draw_integer(5, 5, user.status.gold, SDL_::Color::WHITE);
}

void status_update_food(void)
{
  status_draw_integer(7, 5, user.status.food, SDL_::Color::WHITE);
}

void status_update_experience(void)
{
  status_draw_integer( 9, 5, user.status.fighter.EXP, SDL_::Color::WHITE);
  status_draw_integer(10, 5, user.status.wizard.EXP, SDL_::Color::WHITE);
}

void status_update_equipment(void)
{
  int i;
  
  for (i = 0; i < 5; i++) {
    const char *name = goods_data[i][user.equipment[i]].name;
    status_draw_text(i + 12, 0, name, SDL_::Color::WHITE);
  }
}

void status_draw_integer(int row, int col, int n, SDL_::Color pixel)
{
  if (n >= 0) {
    char buf[16];
    sprintf(buf, "%07d", n);
    status_draw_text(row, col, buf, pixel);
  } else {
    status_draw_text(row, col, "0000000", SDL_::Color::RED);
  }
}

void status_draw_text(int row, int col, const char *s, SDL_::Color pixel)
{
  status_erase_line(row);
  draw_text(clip_status, col * 16, row * 16, s, pixel);
}

// 品物のリストを表示する
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
      status_draw_string(row, 0, buf, SDL_::Color::RED);
      status_draw_string(row, 2, gd->name, SDL_::Color::WHITE);
      row++;
    }
  }
}

// ユーザーの在庫リストを表示する
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

// ユーザーのステータスを表示する
void status_user_status(void)
{
  char buf[BUFSIZ];
  int i;

  for (i = 0; i < MAX_STATUS_LINE; i++)
    status_erase_line(i);
  
  status_draw_string(0, 0, user.status.name, SDL_::Color::WHITE);
  status_draw_string(1, 0.5,
                     fighter_rank[user.status.fighter.rank].name, SDL_::Color::RED);
  status_draw_string(2, 0.5,
                     wizard_rank[user.status.wizard.rank].name, SDL_::Color::RED);

  status_draw_string(4, 0, "HP",     SDL_::Color::RED);
  status_draw_string(5, 0, "Max.HP", SDL_::Color::RED);
  status_draw_string(6, 0, "Gold",   SDL_::Color::RED);
  status_draw_string(7, 0, "STRENG", SDL_::Color::RED);
  status_draw_string(8, 0, "MAGIC",  SDL_::Color::RED);
  status_draw_string(9, 0, "DEFEND", SDL_::Color::RED);
  
  status_draw_long(4, 7.5, user.status.HP, SDL_::Color::WHITE);
  status_draw_long(5, 7.5, user.status.max_HP, SDL_::Color::WHITE);
  status_draw_long(6, 7.5, user.status.gold, SDL_::Color::WHITE);
  status_draw_long(7, 7.5, user_attack_point(), SDL_::Color::WHITE);
  status_draw_long(8, 7.5, user_magic_point(0), SDL_::Color::WHITE);
  status_draw_long(9, 7.5, user_defend_point(GUARD_FRONT), SDL_::Color::WHITE);

  status_draw_string(11, 0.5, "STR", SDL_::Color::RED);
  status_draw_string(11, 8.5, "INT", SDL_::Color::RED);
  status_draw_string(12, 0.5, "WIS", SDL_::Color::RED);
  status_draw_string(12, 8.5, "CHR", SDL_::Color::RED);
  status_draw_string(13, 0.5, "DEX", SDL_::Color::RED);
  status_draw_string(13, 8.5, "AGL", SDL_::Color::RED);
  status_draw_string(14, 0.5, "MGR", SDL_::Color::RED);
  status_draw_string(14, 8.5, "KRM", SDL_::Color::RED);
  
  status_draw_short(11,  3.5, user_STR(), SDL_::Color::WHITE);
  status_draw_short(11, 11.5, user_INT(), SDL_::Color::WHITE);
  status_draw_short(12,  3.5, user_WIS(), SDL_::Color::WHITE);
  status_draw_short(12, 11.5, user_CHR(), SDL_::Color::WHITE);
  status_draw_short(13,  3.5, user_DEX(), SDL_::Color::WHITE);
  status_draw_short(13, 11.5, user_AGL(), SDL_::Color::WHITE);
  status_draw_short(14,  3.5, user_MGR(), SDL_::Color::WHITE);
  status_draw_short(14, 11.5, user_KRM(), SDL_::Color::WHITE);

  status_draw_string(15, 8.5, "CRN", SDL_::Color::RED);
  status_draw_short(15, 11.5, user.status.CRN, SDL_::Color::WHITE);

  if (!in_scenario2()) {
    status_draw_string(15, 0.5, "KEY", SDL_::Color::RED);
    status_draw_string(16, 0.5, "ELX", SDL_::Color::RED);
    status_draw_short(15, 3.5, user.status.KEY, SDL_::Color::WHITE);
    status_draw_short(16, 3.5, user.status.ELX, SDL_::Color::WHITE);
  } else {
    // scenario 2
    status_draw_string(15, 0.5, "ELX", SDL_::Color::RED);
    status_draw_short(15, 3.5, user.status.ELX, SDL_::Color::WHITE);
  }
}
