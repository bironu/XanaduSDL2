#include "xanadu.h"
#include "battle.h"
#include "battle/BattleState.h"

// 戦闘のロジック本体はsrc/scene/battle/BattleScene.cppへ移動した。
// ここに残るのは、BattleScene生成前後にも直接呼ばれるためメンバ関数化
// できない4関数(include/battle/BattleState.h参照)を、BattleStateへ
// 委譲するブリッジのみ

extern const int battle_frame_user[10] = { 8, 0, 8, 2, 0, 6, 2, 0, 6, 2 };

int init_battle(room_t *room, const battle_t *suspended, std::function<int(int)> thunk_escape)
{
  return BattleState::instance().init(room, suspended, std::move(thunk_escape));
}

void init_battle_monsters(const battle_t *suspended)
{
  BattleState::instance().resetMonsters(suspended);
}

void save_battle_monsters(void)
{
  BattleState::instance().save(user.battle);
}

int replace_battle_map(int x, int y, int tile)
{
  return BattleState::instance().replaceMap(x, y, tile) ? 1 : 0;
}
