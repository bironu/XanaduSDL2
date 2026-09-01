#ifndef user_dead_H
#define user_dead_H

/* コンテキスト保護関数 */
extern void user_dead_enter(void);
extern void user_dead_leave(void);

/* 初期化関数 */
extern int init_user_dead(int x, int y, void (*update_background)(void));

#endif /* user_dead_H */