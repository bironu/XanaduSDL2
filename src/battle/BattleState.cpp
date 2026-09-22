#include "battle/BattleState.h"

// field.cppに定義されている
extern const int frame_monster[10];

namespace {

// unknownA_table(旧battle.cpp)の軽量版。move_proc/change_dirは
// BattleScene::kActivityTable(メンバ関数ポインタ)側が持つため、
// ここではsensitive/isCasterのみを同じインデックスで保持する
struct ActivityInfo {
	int sensitive;
	bool isCaster;
};

constexpr ActivityInfo kActivityInfo[] = {
	{  8, false }, // 0: 追跡型 A
	{  4, false }, // 1: 追跡型 B
	{  1, false }, // 2: 追跡型 C
	{ 16, false }, // 3: 中立型 A
	{ 16, false }, // 4: 中立型 B
	{  1, false }, // 5: 舞踏型 A
	{  8, false }, // 6: 固定型
	{  8, true  }, // 7: ワープ型
	{  1, true  }, // 8: 舞踏型 B
	{  4, true  }, // 9: 追跡型 D
	{  1, true  }, // 10: 舞踏型 C
	{  4, true  }, // 11: 逃走型 A
	{  4, false }, // 12: 逃走型 B
};

} // namespace

// 部屋の四辺のドアの位置。use_item.cppから直接externで参照される
extern const point_t room_door_position[4] = {
	{ 4, 8 }, { 0, 4 }, { 8, 4 }, { 4, 0 }
};

BattleState *BattleState::instance_ = nullptr;

BattleState::BattleState()
{
	instance_ = this;
}

BattleState::~BattleState()
{
	instance_ = nullptr;
}

int BattleState::init(room_t *room_, const battle_t *suspended, std::function<int(int)> escapeCallback_)
{
	room = room_;
	escapeCallback = std::move(escapeCallback_);

	monsterImage = frame_monsters[room->monster_id / 4];
	monsterStatus = &monster_data[room->monster_id];

	int n = monsterStatus->unknownA;
	if (n < 0 || static_cast<int>(sizeof(kActivityInfo) / sizeof(kActivityInfo[0])) <= n) {
		n = 0;
	}
	activityPattern = n;
	sensitive = kActivityInfo[n].sensitive;
	isCaster = kActivityInfo[n].isCaster;

	// 魔法を唱えられるようにする(特殊ケース: 固定型かつ知力を持つモンスター)
	if (monsterStatus->unknownA == 6 &&
	    monsterStatus->INT > 0 && room->monster_id % MAX_VARIETY > 1) {
		isCaster = true;
	}

	// モンスターの行動間隔
	if (monsterStatus->AGL == 0) {
		interval = 0;
	} else {
		interval = user.status.AGL / monsterStatus->AGL;
	}

	resetMonsters(suspended);
	constructMap();

	// 戦闘中を示すフラグをセット
	user.environment.in_battle = numMonsters > 0;

	return CONTEXT_BATTLE;
}

void BattleState::resetMonsters(const battle_t *suspended)
{
	if (!suspended) {
		// モンスターを初期化
		int n = 0;
		for (int i = 0; i < MAX_MEMBER; i++) {
			member_t *mm = &room->members[i];

			if (member_monster(mm)) {
				int dir = random_direction();

				if (monsterStatus->activity & ACTIVITY_VIVID)
					mm->frame = frame_monster[dir] + random_integer(2);
				else
					mm->frame = frame_monster[dir];

				// モンスター識別番号
				mm->value = n;

				// モンスターの個体に関する情報
				monster_t *mo = &monsters[n];
				mo->member        = mm;
				mo->state         = MONSTER_ALIVE;
				mo->HP            = monsterStatus->max_HP * 100;
				mo->dir           = dir;
				mo->monster_timer = interval - random_integer(4) + 2;
				mo->phantom_timer = 20 + random_integer(8); // phantom_time()相当
				mo->magic         = &magics[n];

				n++;
			}
		}
		maxMonsters = numMonsters = n;

		// 魔法を初期化
		for (int i = 0; i < MAX_MEMBER + 1; i++) {
			magics[i].lifetime = 0;
		}
	} else {
		// 保存されている情報を復元
		maxMonsters = suspended->max_monsters;
		numMonsters = 0;
		memcpy(monsters, suspended->monsters, sizeof(monsters));
		memcpy(magics, suspended->magics, sizeof(magics));

		// 生きているモンスターを構成員情報とリンクする
		for (int i = 0; i < MAX_MEMBER; i++) {
			member_t *mm = &room->members[i];
			if (member_monster(mm)) {
				monsters[mm->value].member = mm;
				numMonsters++; // 生きているモンスターの数
			}
		}
		// 死んでいるモンスターのmemberの内容はゴミであることに注意！
		// 魔法情報とリンクする
		for (int i = 0; i < maxMonsters; i++) {
			monsters[i].magic = &magics[i];
		}
	}
}

void BattleState::constructMap()
{
	// フィールド？
	if (!user.environment.in_tower) {
		// 床
		int *p = reinterpret_cast<int *>(map);
		for (int i = 0; i < 81; i++, p++)
			*p = i % 2 == 0 ? tile_data.pattern0 : tile_data.pattern1;

		// 外壁
		for (int i = 0; i < 9; i++) {
			if (room->barrier[0] == BARRIER_WALL)
				map[8][i] = tile_data.bricks;
			if (room->barrier[1] == BARRIER_WALL)
				map[i][0] = tile_data.bricks;
			if (room->barrier[2] == BARRIER_WALL)
				map[i][8] = tile_data.bricks;
			if (room->barrier[3] == BARRIER_WALL)
				map[0][i] = tile_data.bricks;
		}
	} else {
		// 床
		int *p = reinterpret_cast<int *>(map);
		for (int i = 0; i < 81; i++, p++)
			*p = tile_data.floor;

		// 外壁
		for (int i = 0; i < 8; i++) {
			map[0    ][i] = tile_data.marble;
			map[i    ][8] = tile_data.marble;
			map[i + 1][0] = tile_data.marble;
			map[8][i + 1] = tile_data.marble;
		}
		// 出入り口
		for (int i = 0; i < 4; i++) {
			int tile;
			switch (room->barrier[i]) {
			case BARRIER_OPEN: tile = tile_data.floor;    break;
			case BARRIER_LOCK: tile = tile_data.locked;   break;
			case BARRIER_EXIT: tile = tile_data.pattern1; break;
			default: continue;
			}
			int x = room_door_position[i].x;
			int y = room_door_position[i].y;
			map[y][x] = tile;
		}
	}
}

bool BattleState::replaceMap(int x, int y, int tile)
{
	if (0 <= x && x < 9 && 0 <= y && y < 9) {
		map[y][x] = tile;
		return true;
	}
	return false;
}

void BattleState::save(battle_t &out) const
{
	out.max_monsters = maxMonsters;
	memcpy(out.monsters, monsters, sizeof(out.monsters));
	memcpy(out.magics, magics, sizeof(out.magics));
}
