#ifndef field_H
#define field_H

#include "xanadu.h"

// フィールドの一次元座標に変換
#define field_offset_XY(x, y)	((x) + (y) * FIELD_WIDTH)

// ユーザーの視界の左上隅の相対位置
#define user_sight_XY() (in_training_ground()		\
                         ? field_offset_XY(-4, -7)	\
                         : field_offset_XY(-4, -4))

// コンテキスト保護関数
extern void field_enter(void);
extern void field_leave(void);

// 指定された位置のマップの地形を返す
int point_map(int p);

// 初期化関数
extern int init_field(void);
extern int init_training_ground(int scenario);

// 次の階層に行く洞窟を開く
extern void field_cave_open(void);

#endif // field_H