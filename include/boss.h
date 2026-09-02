#ifndef boss_H
#define boss_H

/* コンテキスト保護関数 */
extern void boss_enter(void);
extern void boss_leave(void);

/* 初期化関数 */
extern int init_boss(int boss_id);

#endif /* boss_H */