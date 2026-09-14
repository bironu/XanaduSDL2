#include "tower.h"
#include "battle.h"
#include "user.h"
#include "boss.h"

static int tower_battle_escape(int);

// 初期化関数: menu.c から呼ばれる
int init_tower(void)
{
  // BGM
  bgm_play(bgm_data.dungeon[user.environment.scenario]
           .tower[user.environment.dungeon_level]);
  bgm_tempo(0); // テンポ
  
  // 戦闘中だった？
  if (in_battle())
    return init_battle(&level_data.tower[user.point],
                       &user.battle,
                       tower_battle_escape);
  else
    return CONTEXT_TOWER;
}

void tower_enter(void)
{
  room_t *room = &level_data.tower[user.point];

  // ボス？
  if (room->boss_id >= 0) {
    int boss_id = room->boss_id;
    
    room->boss_id = -1; // 生還した場合に備えて退治されたことにする
    extend_context(init_boss(boss_id));
  } else {
    // BGM
    bgm_play(bgm_data.dungeon[user.environment.scenario]
             .tower[user.environment.dungeon_level]);
    bgm_tempo(0); // テンポ

    user.environment.in_tower = 1;
    
    switch_context(init_battle(room, NULL, tower_battle_escape));
  }
}

void tower_leave(void)
{
}

#define TOWER_WIDTH1		4
#define TOWER_WIDTH2		8
int tower_battle_escape(int dir)
{
  room_t *room = &level_data.tower[user.point];
  const int *move;
  // 移動表
  static const int move1[4] = {
    TOWER_WIDTH1, TOWER_SIZE - 1, 1, TOWER_SIZE - TOWER_WIDTH1 };
  static const int move2[4] = {
    TOWER_WIDTH2, TOWER_SIZE - 1, 1, TOWER_SIZE - TOWER_WIDTH2 };

  if (!in_scenario2())
    move = move1;
  else
    move = move2;
  
  // 塔から抜ける？
  if (room->barrier[dir / 2 - 1] == BARRIER_EXIT) {
    user.point = room->entrance_point; // フィールド上の位置
    if (user.environment.lighting > 0) {
      user.environment.lighting--;     // ランプを消す
    }
    switch_context(CONTEXT_FIELD);
    
  } else {
    int point = (user.point + move[dir / 2 - 1]) % TOWER_SIZE;
    room = &level_data.tower[point];
    
    if (room->boss_id >= 0) {
      init_battle_monsters(NULL); // battle.c
      save_user(); // ボス戦に敗北した場合に後戻りする状態
    }

    // 隣の部屋に移動: 保存する前に更新してはいけない
    user.point = point;
    
    switch (dir) {
    case 2: user.y = 0;        break;
    case 4: user.x = 360 - 40; break;
    case 6: user.x = 0;        break;
    case 8: user.y = 360 - 40; break;
    default: return 0; // たぶんエラー
    }
    
    if (room->boss_id >= 0) {
      // CONTEXT_TOWER に戻る: 現在はおそらく CONTEXT_BATTLE
      switch_context(CONTEXT_TOWER);
    } else {
      switch_context(init_battle(room, NULL, tower_battle_escape));
    }
  }
  return 1;
}
