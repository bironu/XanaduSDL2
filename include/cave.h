#ifndef cave_H
#define cave_H

// コンテキスト保護関数
extern void cave_enter(void);
extern void cave_leave(void);

// 初期化関数
extern int init_cave(int to_level);

#endif // cave_H