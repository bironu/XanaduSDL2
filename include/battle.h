#ifndef battle_H
#define battle_H

#include "dungeon.h"
#include <functional>

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

// monster_t::stateに入る値。BattleState/BattleScene両方から使うためここに置く
constexpr short MONSTER_ALIVE     = 0; // 生きている
constexpr short MONSTER_KILLED    = 1; // 殺された
constexpr short MONSTER_DEAD      = 3; // 死んだ
constexpr short MONSTER_DISAPPEAR = 4; // 消滅した
constexpr short MONSTER_RIP       = 5; // 永眠中

extern const int battle_frame_user[10];

// 部屋の四辺のドアの位置(point_t型はxanadu.h由来のためここでは宣言できない。
// 実体はsrc/battle/BattleState.cpp。使う側はuse_item.cppのように
// ローカルにextern宣言すること)

// 初期化関数。実体はBattleState(include/battle/BattleState.h)への
// 薄いブリッジ(src/battle.cpp)。field.cpp/tower.cppからBattleScene生成前に、
// init_battle_monsters/replace_battle_mapはtower.cpp/use_item.cppから
// BattleSceneの生存期間と無関係に直接呼ばれるため、この形で残している
extern int init_battle(room_t *room, const battle_t *suspended,
                       std::function<int(int)> thunk_escape);
extern void init_battle_monsters(const battle_t *suspended);
extern void save_battle_monsters(void);

// 戦場マップの修正
extern int replace_battle_map(int x, int y, int tile);

#endif // battle_H
