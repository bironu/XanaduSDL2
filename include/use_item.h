#ifndef use_item_H
#define use_item_H

#include "dungeon.h"

/* コンテキスト保護関数 */
extern void use_item_enter(void);
extern void use_item_leave(void);

/* 初期化関数 */
extern int init_use_item(void (*update_background)(void), room_t *room);

#endif /* use_item_H */