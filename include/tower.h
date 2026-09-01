#ifndef tower_H
#define tower_H

#include "xanadu.h"

/* コンテキスト保護関数 */
extern void tower_enter(void);
extern void tower_leave(void);

/* 初期化関数 */
extern int init_tower(void);

#endif /* tower_H */