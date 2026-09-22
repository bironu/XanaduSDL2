#ifndef BATTLESTATE_H_
#define BATTLESTATE_H_

#include "xanadu.h"
#include "battle.h"
#include "dungeon.h"
#include "misc/Uncopyable.h"
#include "sdl/SDLImage.h"
#include <functional>

// 戦闘中のコアデータ(部屋・戦場マップ・モンスター・魔法)を保持するクラス。
//
// init_battle()/init_battle_monsters()はBattleSceneが生成される前
// (field.cpp/tower.cppから、switch_context(init_battle(...))として)に、
// save_battle_monsters()/replace_battle_map()はBattleSceneの生存期間と
// 無関係なタイミング(tower.cppの塔内移動処理、use_item.cppのアイテム効果)に
// 直接呼ばれるため、これらが操作するデータはBattleSceneのメンバにできない。
// Resources(include/resources/Resources.h)と同じ「旧Cスタイル関数から
// instance()経由でアクセスする」パターンを踏襲する。
class BattleState final
{
public:
	UNCOPYABLE(BattleState);
	BattleState();
	~BattleState();

	static BattleState &instance() { return *instance_; }

	room_t *room = nullptr;
	int map[9][9] = {};
	monster_t monsters[MAX_MEMBER] = {};
	magic_t magics[MAX_MEMBER + 1] = {};
	int maxMonsters = 0;   // 合計のモンスター数(固定)
	int numMonsters = 0;   // 現在のモンスター数(変動)
	monster_status_t *monsterStatus = nullptr;
	SDL_::SubImage *monsterImage = nullptr;

	// monster_status->unknownAから求めた行動パターン番号(0〜12)。
	// 実際の行動関数(移動/方向転換)はBattleSceneのメンバ関数ポインタ
	// テーブル(BattleScene::kActivityTable、同じインデックスを共有)として
	// 持つため、ここではsensitive/isCasterの導出値のみ保持する
	int activityPattern = 0;
	int sensitive = 0;     // 敏感さ
	bool isCaster = false; // 魔法を使用する？
	int interval = 0;      // モンスター行動間隔

	// BattleScene脱出時に呼ぶコールバック(旧thunk_battle_escape)
	std::function<int(int)> escapeCallback;

	// field.cpp/tower.cppから、BattleScene生成前に呼ばれる。CONTEXT_BATTLEを返す
	int init(room_t *room, const battle_t *suspended, std::function<int(int)> escapeCallback);

	// tower.cppから、BattleScene終了後(塔内でボス部屋に隣接した際)に
	// モンスター/魔法情報だけをリセットするために直接呼ばれる
	void resetMonsters(const battle_t *suspended);

	// 戦場マップをroomの情報から再構築する
	void constructMap();

	// 戦場マップの1マスを差し替える。use_item.cppから
	bool replaceMap(int x, int y, int tile);

	// 現在のモンスター/魔法情報をuser.battleへ保存する(Ctrl-Q保存用)
	void save(battle_t &out) const;

private:
	static BattleState *instance_;
};

#endif // BATTLESTATE_H_
