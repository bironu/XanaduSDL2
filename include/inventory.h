#ifndef inventory_H
#define inventory_H

/* コンテキスト保護関数 */
extern void inventory_enter(void);
extern void inventory_leave(void);

/* 初期化関数 */
extern int init_inventory(void);

#endif /* inventory_H */