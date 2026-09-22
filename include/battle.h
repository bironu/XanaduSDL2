#ifndef battle_H
#define battle_H

#include "dungeon.h"

// 戦闘時の魔法に関する情報を保持する構造体
typedef struct {
  short		lifetime;		// 寿命
  short		x;			// 水平位置
  short		y;			// 垂直位置
  short		frame;			// フレーム
  short		dir;			// 方向
  short		scroll_type;		// 魔法種別
} magic_t;

// 戦闘時のモンスターに関する情報を保持する構造体
typedef struct {
  member_t *	member;			// メンバー情報
  int		HP;			// ヒットポイント
  short		state;			// 状態
  short		dir;			// 方向
  short		monster_timer;		// タイマー
  short		phantom_timer;		// テレポートタイマー
  magic_t *	magic;			// 魔法
} monster_t;

// サスペンドした戦闘時の情報を保持する構造体
typedef struct {
  int		max_monsters;		// モンスターの数
  monster_t	monsters[MAX_MEMBER];	// モンスター情報
  magic_t	magics[MAX_MEMBER + 1];	// 魔法情報
} battle_t;

extern const int battle_frame_user[10];

// コンテキスト保護関数
extern void battle_create(void);
extern void battle_destroy(void);
extern void battle_enter(void);
extern void battle_leave(void);

// 初期化関数
extern int init_battle(room_t *room, const battle_t *suspended,
                       int (*thunk_escape)(int dir));
extern void init_battle_monsters(const battle_t *suspended);
extern void save_battle_monsters(void);

// 戦場マップの修正
extern int replace_battle_map(int x, int y, int tile);

#endif // battle_H
