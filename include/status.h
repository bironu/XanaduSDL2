#ifndef status_H
#define status_H

#include "graphics.h"

/* ステータス画面全体の更新 */
extern void status_refresh(int in_battle);
extern void status_user_status(void);
extern void status_user_goods(int goods_type);
extern void status_list_goods(int goods_type, unsigned display_bits);

/* 特定のエントリを更新する */
extern void status_update_rank(void);
extern void status_update_HP(pixel_t pixel);
extern void status_update_gold(void);
extern void status_update_food(void);
extern void status_update_experience(void);
extern void status_update_equipment(void);

/* 数値または文字列を描画する */
extern void status_draw_integer(int row, int col, int n, pixel_t pixel);
extern void status_draw_text(int row, int col, const char *s, pixel_t pixel);

/* 特定の行を消去する */
extern void status_erase_line(int row);

#endif /* status_H */