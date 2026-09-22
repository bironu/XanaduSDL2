#include "scene/battle/BattleScene.h"
#include "battle/BattleState.h"
#include "context.h"
#include "status.h"
#include "equip.h"
#include "use_item.h"
#include "inventory.h"
#include "user_dead.h"
#include "animation.h"
#include "resources/Resources.h"
#include "resources/SoundId.h"
// #include "sdl/LegacyPlatform.h"
#include "app/Application.h"
#include "sdl/SDLMixMixer.h"

// field.cppに定義されている
extern const point_t move_table[10];
extern const int frame_monster[10];
extern const int frame_magic[MAX_SCROLL_TYPE];

// src/battle/BattleState.cppに定義されている(point_t型はxanadu.h由来)
extern const point_t room_door_position[4];

namespace {

// 魔法詠唱SE。scrollTypeはSCROLL_NEEDLE(0)..SCROLL_DEATH(8)(goods.h参照)
SoundId resolveCastSound(int scrollType)
{
	static constexpr SoundId kCastSounds[MAX_SCROLL_TYPE] = {
		SoundId::c_needle, SoundId::c_mittar, SoundId::c_deluge,
		SoundId::c_fire,   SoundId::c_thunder, SoundId::c_poison,
		SoundId::c_corros, SoundId::c_tilte,   SoundId::c_death,
	};
	if (scrollType < 0 || MAX_SCROLL_TYPE <= scrollType) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "resolveCastSound: scroll_type out of range %d\n", scrollType);
		return SoundId::invoke;
	}
	return kCastSounds[scrollType];
}

constexpr SoundId kBattleSoundIds[] = {
	SoundId::dead, SoundId::magic, SoundId::failed, SoundId::lost_key,
	SoundId::attack, SoundId::trapped, SoundId::treasure, SoundId::get,
	SoundId::poison, SoundId::encount,
	SoundId::c_needle, SoundId::c_mittar, SoundId::c_deluge, SoundId::c_fire,
	SoundId::c_thunder, SoundId::c_poison, SoundId::c_corros, SoundId::c_tilte,
	SoundId::c_death,
};

} // namespace

// モンスター行動パターンテーブル(旧battle.cpp unknownA_table)。
// BattleState::activityPatternと同じインデックスを共有する
const BattleScene::ActivityProc BattleScene::kActivityTable[13] = {
	{ &BattleScene::monstersMoveWalker, &BattleScene::changeDirChaser  }, // 0: 追跡型 A
	{ &BattleScene::monstersMoveWalker, &BattleScene::changeDirChaser  }, // 1: 追跡型 B
	{ &BattleScene::monstersMoveWalker, &BattleScene::changeDirChaser  }, // 2: 追跡型 C
	{ &BattleScene::monstersMoveWalker, &BattleScene::changeDirNeutral }, // 3: 中立型 A
	{ &BattleScene::monstersMoveWalker, &BattleScene::changeDirNeutral }, // 4: 中立型 B
	{ &BattleScene::monstersMoveWalker, &BattleScene::changeDirDancer  }, // 5: 舞踏型 A
	{ &BattleScene::monstersMoveRooted, &BattleScene::changeDirNeutral }, // 6: 固定型
	{ &BattleScene::monstersMoveVision, &BattleScene::changeDirNeutral }, // 7: ワープ型
	{ &BattleScene::monstersMoveWalker, &BattleScene::changeDirDancer  }, // 8: 舞踏型 B
	{ &BattleScene::monstersMoveWalker, &BattleScene::changeDirChaser  }, // 9: 追跡型 D
	{ &BattleScene::monstersMoveWalker, &BattleScene::changeDirDancer  }, // 10: 舞踏型 C
	{ &BattleScene::monstersMoveWalker, &BattleScene::changeDirEscapee }, // 11: 逃走型 A
	{ &BattleScene::monstersMoveWalker, &BattleScene::changeDirEscapee }, // 12: 逃走型 B
};

BattleScene::BattleScene()
	: GameScene(CONTEXT_BATTLE)
{
}

// ---- ヘルパー(旧マクロ関数) ----

bool BattleScene::userGetSkillfull() const
{
	return random_integer(4) == 0;
}

void BattleScene::userSkillUp(int type, int n)
{
	user.inventory[type][user.equipment[type]].skill =
		min(kSkillMax[type], user.inventory[type][user.equipment[type]].skill + n);
}

int BattleScene::userError(int p)
{
	return (p * (random_integer(8) + 30)) / 32;
}

int BattleScene::monsterError(int p)
{
	return (p * (random_integer(8) + 12)) / 16;
}

int BattleScene::monsterTime() const
{
	return BattleState::instance().interval;
}

// ---- ライフサイクル ----

void BattleScene::onCreate(uint32_t /*tick*/)
{
	auto &res = Resources::instance();
	auto &mixer = Application::instance().getMixer();
	for (SoundId id : kBattleSoundIds) {
		res.loadSound(mixer, id);
	}
	needEncountSound_ = true;
}

void BattleScene::onDestroy(uint32_t /*tick*/)
{
	auto &res = Resources::instance();
	for (SoundId id : kBattleSoundIds) {
		res.unloadSound(id);
	}
}

void BattleScene::onEnter()
{
	// 画面の描画。遭遇音の待機中もフィールドの残像ではなく戦闘画面が
	// 見えるよう、SEを鳴らすより先に更新しておく
	updateBackground();
	status_refresh(in_battle());

	if (needEncountSound_) {
		needEncountSound_ = false;

		// 遭遇音が鳴り終わるまでゲーム進行・操作を止め、いきなり戦闘が
		// 始まらないようにする。その間BGMはミュートする
		const int channel = playSound(SoundId::encount);
		Application::instance().getMixer().setMusicGain(0.0f);
		wait(
			[channel]() { return !Application::instance().getMixer().isChannelPlaying(channel); },
			[this]() {
				Application::instance().getMixer().setMusicGain(1.0f);
				battleEnterContinue();
			});
		return;
	}

	battleEnterContinue();
}

void BattleScene::battleEnterContinue()
{
	if (in_battle()) {
		// 戦闘時のステータス画面
		auto &state = BattleState::instance();
		int i;

		status_draw_text(7, 0, state.monsterStatus->name, SDL_::Color::RED);

		for (i = 0; i < state.maxMonsters; i++) {
			int row = i + kStatusMonsterHpLine;
			if (state.monsters[i].HP >= 0)
				status_draw_integer(row, 5, state.monsters[i].HP, SDL_::Color::WHITE);
			else
				status_draw_text(row, 5, "Dead !!", SDL_::Color::RED);
		}
		for (; i < MAX_MEMBER; i++) {
			status_erase_line(i + kStatusMonsterHpLine);
		}
		bgm_tempo(1); // テンポを早くする
	}

	// タイマーの設定
	setGameTimer(kBattleInterval, [this]() { battleLoop(); });
}

void BattleScene::onLeave()
{
	killGameTimer();
}

// ---- メインループ ----

void BattleScene::battleLoop()
{
	auto &state = BattleState::instance();
	int (BattleScene::*moveProc)(int dir);
	int interval, update = 0;

	if (doping_AGL()) {
		mirrorWait_ = (mirrorWait_ + 1) % 2;
		interval = (kBattleInterval / 2);
	} else {
		mirrorWait_ = 0;
		interval = kBattleInterval;
	}

	// 時間の経過
	user_time_elapse(interval);
	setGameTimer(interval, [this]() { battleLoop(); });

	// 死んだ？
	if (user.status.HP < 0) {
		userDamage_ = nullptr;
		play_user_dead(user.x, user.y, [this]() { updateBackground(); }, [this]() { onEnter(); });
		return;
	}

	// 前のターンでユーザーは何かを攻撃した？
	if (userAttack_ != nullptr) {
		userAttack_ = nullptr;
		update = 1;
		goto do_monster;
	}

	// 前のターンでユーザーは攻撃された？
	if (userDamage_ != nullptr) {
		userDamage_ = nullptr;
		update = 1;
		status_update_HP(SDL_::Color::WHITE);
		goto do_magic;
	}

	if (isCtrlDown()) {
		// Ctrl-S: サウンド
		if (isKeyDown(SDL_SCANCODE_S)) {
			if (bgm_mute()) {
				emit_message("Sound Off");
			} else {
				emit_message("Sound On");
			}
			pauseFor(SDL_SCANCODE_S, [this]() { onEnter(); });
			return;
		}
		// Ctrl-Q: 保存
		if (isKeyDown(SDL_SCANCODE_Q)) {
			state.save(user.battle);
			// フィールドの場合、戦闘中でなくても戦闘中として保存する
			user.environment.in_battle = 1;
			save_user();
			user.environment.in_battle = state.numMonsters > 0;
			switch_context(CONTEXT_START_MENU);
			return;
		}
	} else {
		// ENTER: アイテム使用
		if (isReturnDown() &&
		    user.equipment[GOODS_MAGIC_ITEM] < MAX_GOODS) {
			play_use_item([this]() { updateBackground(); }, state.room, [this]() { onEnter(); });
			return;
		}

		// S: ステータス表示
		if (isKeyDown(SDL_SCANCODE_S)) {
			status_user_status();
			emit_message("Hit any key");
			extend_context(init_enter_buffer(CONTEXT_ENTER_CHARACTER, NULL));
			return;
		}
		// I: 在庫表示
		if (isKeyDown(SDL_SCANCODE_I)) {
			extend_context(init_inventory());
			return;
		}
		// E: 装備
		if (isKeyDown(SDL_SCANCODE_E) && !in_battle()) {
			extend_context(init_equip());
			return;
		}
	}

	// Shiftキー状態
	keystateShift_ = isShiftDown();

	// CTRLキーが押されている？
	if (isCtrlDown()) {
		moveProc = &BattleScene::battleControlUserMagic;
	} else {
		moveProc = &BattleScene::battleMoveUser;
	}

	// SPACE: 魔法念唱
	if (isKeyDown(SDL_SCANCODE_SPACE)) {
		magic_t *userMagic = &state.magics[MAX_MEMBER];
		if (userMagic->lifetime == 0) {
			battleCastSpell(userMagic,
			                user.equipment[GOODS_SCROLL],
			                user_INT(),
			                user.x, user.y, user.dir);
			update = 1;
		}
	}
	else if (isKeyDown(SDL_SCANCODE_DOWN))  update = (this->*moveProc)(2);
	else if (isKeyDown(SDL_SCANCODE_LEFT))  update = (this->*moveProc)(4);
	else if (isKeyDown(SDL_SCANCODE_RIGHT)) update = (this->*moveProc)(6);
	else if (isKeyDown(SDL_SCANCODE_UP))    update = (this->*moveProc)(8);

	// 中断？
	if (update < 0) {
		return;
	}

	// 何か攻撃した？
	if (userAttack_ != nullptr) {
		goto done;
	}

do_monster:
	if (in_battle() && !mirrorWait_ && !using_hourglass()) {
		// モンスターの移動
		update |= (this->*(kActivityTable[state.activityPattern].moveProc))();

		// ユーザーを攻撃した？
		if (userDamage_ != nullptr) {
			update |= 1;
			goto do_magic;
		}
	}

do_magic:
	// ユーザーの直接攻撃に対して同期を取らなければならない
	if (!mirrorWait_) {
		update |= battleMoveMagic();
	}

done:
	// 更新する？
	if (update)
		updateBackground();

	if (userAttack_ != nullptr) {
		// ダメージ個所を反転
		inverse_image(clip_main, userAttack_->x, userAttack_->y, mask_damaged);
		setGameTimer(kBattleInterval, [this]() { battleLoopAttack(); });
	}
}

void BattleScene::battleLoopAttack()
{
	auto &state = BattleState::instance();
	member_t *mm = userAttack_;
	int update = 0;

	if (mm->type == MEMBER_BOX || mm->type == MEMBER_GOODS) {
		emit_message("Unlucky!");
		mm->type = MEMBER_UNUSED;
		setGameTimer(kBattleInterval, [this]() { battleLoop(); }); // メインループに戻る
		update = 1;
	}
	// ごくまれに消える瞬間にヒットすることがある
	else if (member_monster(mm)) {
		monster_t *mo = &state.monsters[mm->value];

		switch (mo->state) {
		case MONSTER_ALIVE:
			status_draw_integer(mm->value + kStatusMonsterHpLine, 5,
			                    mo->HP, SDL_::Color::WHITE);
			setGameTimer(kBattleInterval, [this]() { battleLoop(); }); // メインループに戻る
			return;

		case MONSTER_KILLED:
			mo->monster_timer = kDeadTime;
			mo->state = MONSTER_DEAD;
			update = 1;
			playSound(SoundId::dead);
			break;

		case MONSTER_DEAD:
			if (--mo->monster_timer < 0) {
				mo->state = MONSTER_DISAPPEAR;
				update = 1;
			}
			break;

		case MONSTER_DISAPPEAR:
			if (--mo->monster_timer < 0) {
				mo->state = MONSTER_RIP;

				// 宝箱
				mm->type = MEMBER_BOX;
				mm->frame = 3;
				mm->value = killedUserMagic_ || in_battle() ? 0 : 1;
				killedUserMagic_ = 0;

				setGameTimer(kBattleInterval, [this]() { battleLoop(); }); // メインループに戻る
			}
			break;
		}
	}
	// 更新する？
	if (update)
		updateBackground();
}

void BattleScene::battleLoopMagic()
{
	auto &state = BattleState::instance();
	int update = 0;

	switch (userDegPhase_) {
	case kDegPhaseHit:
		{
			int someoneDead = 0;
			monster_t *mo = state.monsters;

			for (int i = 0; i < state.maxMonsters; i++, mo++) {
				if (mo->state == MONSTER_KILLED) {
					mo->state = MONSTER_DEAD;
					someoneDead = 1;
				}
				else if (mo->state == MONSTER_ALIVE) {
					status_draw_integer(i + kStatusMonsterHpLine, 5,
					                    mo->HP, SDL_::Color::WHITE);
				}
			}

			if (!someoneDead) {
				userDegPhase_ = kDegPhaseInactive;
			} else {
				// 誰か死んだ
				magicLoopTimer_ = kDeadTime;
				userDegPhase_ = kDegPhaseDead;
			}
			playSound(SoundId::magic);
			update = 1;
		}
		break;

	case kDegPhaseDead:
		if (--magicLoopTimer_ < 0) {
			monster_t *mo = state.monsters;

			for (; mo < &state.monsters[state.maxMonsters]; mo++) {
				if (mo->state == MONSTER_DEAD) {
					mo->state = MONSTER_DISAPPEAR;
				}
			}
			magicLoopTimer_ = kDeadTime;
			userDegPhase_ = kDegPhaseDisappear;
			playSound(SoundId::dead);
			update = 1;
		}
		break;

	case kDegPhaseDisappear:
		if (--magicLoopTimer_ < 0) {
			monster_t *mo = state.monsters;

			for (; mo < &state.monsters[state.maxMonsters]; mo++) {
				if (mo->state == MONSTER_DISAPPEAR) {
					mo->state = MONSTER_RIP;

					// 宝箱
					mo->member->type = MEMBER_BOX;
					mo->member->frame = 3;
					mo->member->value = 0;
				}
			}
			userDegPhase_ = kDegPhaseInactive;
			update = 1;
		}
		break;
	}

	if (userDegPhase_ == kDegPhaseInactive) {
		setGameTimer(kBattleInterval, [this]() { battleLoop(); }); // メインループに戻る
	}
	if (update)
		updateBackground();
}

void BattleScene::updateBackground()
{
	auto &state = BattleState::instance();
	member_t *mm;
	magic_t *ma;
	int i, j;

	// 背景
	if (in_darkness()) {
		fill_image(clip_main, 0, 0, clip_main->getWidth(), clip_main->getHeight(), SDL_::Color::BLACK);
	} else {
		int *map = reinterpret_cast<int *>(state.map);
		for (i = 0; i < 360; i += 40) {
			for (j = 0; j < 360; j += 40) {
				draw_image(clip_main, j, i, frame_tiles[*map++]);
			}
		}
	}

	// ユーザー
	if (!user_hidden) {
		draw_sprite(clip_main, user.x, user.y, frame_user[user.frame]);
	}

	// モンスター等
	mm = state.room->members;
	for (; mm < &state.room->members[MAX_MEMBER]; mm++) {
		switch (mm->type) {
		case MEMBER_MONSTER:
			{
				monster_t *mo = &state.monsters[mm->value];
				switch (mo->state) {
				case MONSTER_ALIVE:
				case MONSTER_KILLED:
					draw_sprite(clip_main, mm->x, mm->y, state.monsterImage[mm->frame]);
					break;
				case MONSTER_DEAD:
					draw_sprite(clip_main, mm->x, mm->y, frame_specials[SPECIAL_DEAD]);
					break;
				case MONSTER_DISAPPEAR:
					draw_sprite(clip_main, mm->x, mm->y, frame_specials[SPECIAL_DISAPPEAR]);
					break;
				}
			}
			break;

		case MEMBER_BOX:
			draw_sprite(clip_main, mm->x, mm->y,
			            (mm->value ?
			             frame_brownbox[mm->frame] :
			             frame_whitebox[mm->frame]));
			break;

		case MEMBER_GOODS:
			draw_sprite(clip_main, mm->x, mm->y, frame_goods[mm->frame]);
			break;
		}
	}

	// 魔法
	ma = state.magics;
	for (; ma < &state.magics[MAX_MEMBER + 1]; ma++) {
		if (ma->lifetime > 0) {
			draw_sprite(clip_main, ma->x, ma->y, frame_magics[ma->frame]);
		}
	}

	// ダメージ
	if (userDegPhase_ == kDegPhaseHit) {
		mm = state.room->members;

		for (; mm < &state.room->members[MAX_MEMBER]; mm++) {
			if (mm->type == MEMBER_MONSTER) {
				inverse_image(clip_main, mm->x, mm->y, mask_damaged);
			}
		}
	}
	if (userDamage_ != nullptr) {
		inverse_image(clip_main, user.x, user.y, mask_damaged);
	}
	update_region(rect_main.x, rect_main.y, rect_main.width, rect_main.height);
}

// ---- ユーザー移動・入力 ----

int BattleScene::battleMoveUser(int dir)
{
	auto &state = BattleState::instance();
	member_t *something;
	int x, y;
	int update = 0;

	// シフトキーが押されていないならば方向を変える
	if (!keystateShift_ && user.dir != dir) {
		user.dir = dir;
		update = 1;
	}

	// その方向へ一歩進む
	x = user.x + move_table[dir].x * kStepUser;
	y = user.y + move_table[dir].y * kStepUser;

	// 離脱しようとしている？
	if (x < 0 || x > 360 - 40 || y < 0 || y > 360 - 40) {
		if (update) {
			// フレーム更新
			user.frame = battle_frame_user[user.dir] + (user.frame + 1) % 2;
		}
		if (userAttemptEscape(dir)) {
			return kUpdateAbort;
		}
		goto done;
	}

	// 進もうとしている位置の障害物を調べる
	switch (dir) {
	case 2: something = battleCanThrough(x, y + 39, x + 39, y + 39); break;
	case 4: something = battleCanThrough(x, y, x, y + 39); break;
	case 6: something = battleCanThrough(x + 39, y, x + 39, y + 39); break;
	case 8: something = battleCanThrough(x, y, x + 39, y); break;
	default: goto do_move;
	}

	if (something == NULL) {
		// 障害物はない
	do_move:
		user.x = x;
		user.y = y;
		update = 1;

	} else {
		if (!keystateShift_ && userAttemptAttack(something)) {
			update = 1;
			goto done;
		}

		// 壁？
		if (something == &memberWall_ || something == &memberLock_) {
			// 外套
			if (using_mantle() && (!in_scenario2() ||
			                       (something->value & TILE_WALL_MARBLE) == 0))
				goto do_move;

			// 最初からめり込んでいた？
			if (dir == 4 && user.x < 40)
				goto do_move;
			if (dir == 6 && user.x > 360 - 2 * 40)
				goto do_move;
			if (dir == 8 && user.y < 40)
				goto do_move;
			if (dir == 2 && user.y > 360 - 2 * 40)
				goto do_move;
		}

		// 扉？
		if (in_tower() && something == &memberLock_ && user_use_key()) {
			int i;

			// memberLock_には便利な値が書き込まれている
			x = memberLock_.x;
			y = memberLock_.y;
			state.map[y][x] = tile_data.floor;

			for (i = 0; i < 4; i++) {
				if (room_door_position[i].x == x &&
				    room_door_position[i].y == y) {

					// 鍵を開ける
					state.room->barrier[i] = BARRIER_OPEN;
					break;
				}
			}

			emit_message("Lost key");
			playSound(SoundId::lost_key);

			play_animation_tile(tile_data.tower_open, 3,
			                    x * 40, y * 40,
			                    [this]() { updateBackground(); }, [this]() { onEnter(); });
			update = kUpdateAbort;
		}
	}
done:
	if (update) {
		// フレーム更新
		user.frame = battle_frame_user[user.dir] + (user.frame + 1) % 2;
	}
	return update;
}

int BattleScene::battleControlUserMagic(int dir)
{
	BattleState::instance().magics[MAX_MEMBER].dir = dir;
	return 0; // 方向を変えるだけ
}

// モンスターの移動。静止
int BattleScene::monstersMoveRooted()
{
	auto &state = BattleState::instance();
	monster_t *mo, *mo_end;
	int update = 0;

	for (mo = state.monsters, mo_end = mo + state.maxMonsters;
	     mo < mo_end; mo++) {
		if (mo->state == MONSTER_ALIVE && --mo->monster_timer < 0) {

			mo->monster_timer = monsterTime();

			// 隣にユーザーがいる？
			if (monsterCaptureUser(mo) == 0) {
				battleAttackUser(mo->member);
				goto update_frame;
			}

			// フレーム更新
			if ((state.monsterStatus->activity & ACTIVITY_VIVID) != 0) {
			update_frame:
				mo->member->frame = frame_monster[mo->dir]
					+ (mo->member->frame + 1) % 2;
				update = 1;
			}
		}
	}
	return update;
}

// モンスターの移動。瞬間移動
int BattleScene::monstersMoveVision()
{
	auto &state = BattleState::instance();
	monster_t *mo, *mo_end;
	int update = 0;

	for (mo = state.monsters, mo_end = mo + state.maxMonsters;
	     mo < mo_end; mo++) {
		member_t *mm = mo->member;
		int dir;

		// 幻影時間はモンスターの素早さに関わらず時を刻む
		mo->phantom_timer--;

		switch (mm->type) {
		case MEMBER_MONSTER:
			{
				// もうすぐ消える？
				if ((state.monsterStatus->activity & ACTIVITY_FADEOUT) != 0 &&
				    mo->phantom_timer == 4) {
					mm->frame = 1;
					goto update_frame;
				}

				// 消える時間だ？
				if (mo->phantom_timer < 0) {
					mo->phantom_timer = kPhantomTime;
					mm->type = MEMBER_UNSEEN;
					update = 1;
				}
				// 行動する時間だ？
				else if (--mo->monster_timer < 0) {
					mo->monster_timer = monsterTime();

					dir = monsterCaptureUser(mo);
					if (dir == 0) {
						// 攻撃
						battleAttackUser(mm);
						update = 1;
					}
					else if (dir > 0) {
						mo->dir = dir;
						goto update_frame;
					}
				}
				// フレーム更新
				if ((state.monsterStatus->activity & ACTIVITY_FADEOUT) == 0) {
					mm->frame++;
				update_frame:
					mm->frame = frame_monster[mo->dir] + mm->frame % 2;
					update = 1;
				}
			}
			break;

		case MEMBER_UNSEEN:
			if (mo->phantom_timer < 0 && monsterCanAppear(mm)) {
				// 現れる場所を確保できた
				mo->phantom_timer = kPhantomTime;
				mo->dir = random_direction();
				mm->type = MEMBER_MONSTER;
				mm->frame = frame_monster[mo->dir];
				update = 1;
			}
			break;
		}
	}
	return update;
}

// 当たり判定
member_t *BattleScene::battleHitTest(int x, int y)
{
	auto &state = BattleState::instance();
	if (0 <= x && x < 360 && 0 <= y && y < 360) {
		member_t *mm;
		int i;
		int map;

		for (i = 0, mm = state.room->members; i < MAX_MEMBER; i++, mm++) {
			// モンスターまたはお宝？
			if (mm->type > 0) {
				if (mm->x <= x && x < mm->x + 40 && mm->y <= y && y < mm->y + 40)
					return mm;
			}
		}

		// 地形タイルを調べる
		map = state.map[y / 40][x / 40];

		// 扉？
		if (map == tile_data.locked) {
			memberLock_.x = x / 40;
			memberLock_.y = y / 40;
			return &memberLock_;
		}

		// 壁？
		if (tile_data.flags[map] & TILE_WALL) {
			memberWall_.value = tile_data.flags[map];
			return &memberWall_;
		}

		return NULL;
	}
	return &memberWall_;
}

member_t *BattleScene::monsterCanThrough(int x1, int y1, int dir)
{
	member_t *um;
	int x2 = x1;
	int y2 = y1;
	switch (dir) {
	case 2: y1 += 39; x2 += 39; y2 = y1; break;
	case 4: y2 += 39; break;
	case 6: x1 += 39; x2 = x1; y2 += 39; break;
	case 8: x2 += 39; break;
	default: return &memberWall_;
	}
	// ユーザーと接触する？
	if ((user.x <= x1 && x1 < user.x + 40 && user.y <= y1 && y1 < user.y + 40) ||
	    (user.x <= x2 && x2 < user.x + 40 && user.y <= y2 && y2 < user.y + 40)) {
		return &memberWall_;
	}
	return (um = battleHitTest(x1, y1)) != NULL ? um
		: battleHitTest(x2, y2);
}

member_t *BattleScene::battleCanThrough(int x1, int y1, int x2, int y2)
{
	member_t *um1 = battleHitTest(x1, y1);
	member_t *um2 = battleHitTest(x2, y2);

	if (um1 == NULL)
		return um2;
	if (um2 == NULL)
		return um1;

	// モンスターを優先的に返す
	if (um1->type == MEMBER_MONSTER) return um1;
	if (um2->type == MEMBER_MONSTER) return um2;

	// 壁でない方を優先的に返す
	if (um1->type != MEMBER_UNUSED) return um1;
	if (um2->type != MEMBER_UNUSED) return um2;

	// 最終的に壁でない方を返す
	return um1 == &memberWall_ ? um2 : um1;
}

int BattleScene::monsterCanAppear(member_t *mm)
{
	// 現れる場所を決める(ユーザーの歩幅にあわせる)
	int x = random_integer(240 / kStepUser) * kStepUser + 40;
	int y = random_integer(240 / kStepUser) * kStepUser + 40;

	// そこには誰もいない？
	if (battleHitTest(x,      y     ) == NULL &&
	    battleHitTest(x + 39, y + 39) == NULL &&
	    battleHitTest(x + 39, y     ) == NULL &&
	    battleHitTest(x,      y + 39) == NULL &&
	    // プレイヤーの軸上に現れる
	    !((user.x - 40 < x && x < user.x + 40) &&
	      (user.y - 40 < y && y < user.y + 40))
	    ) {
		mm->x = x;
		mm->y = y;
		return 1;
	}
	return 0;
}

/* モンスターの現在の位置から見て、ユーザーのいる方向を返す。
 * 実際のところ、この関数を通過しなければモンスターはユーザーに危害を加えない。
 * -1	ユーザーは水平方向、垂直方向のどちらにもに見つからない。
 *  0	ユーザーは隣にいる。mo->dirはユーザーのいる方向に修正される。
 * 2/8	ユーザーは垂直方向に見つかった。
 * 4/6	ユーザーは水平方向に見つかった。*/
int BattleScene::monsterCaptureUser(monster_t *mo)
{
	auto &state = BattleState::instance();
	int dir = 0;
	int diff_x;
	int diff_y;

	// 見えない
	if (using_demons_ring())
		return -1;

	diff_x = user.x - mo->member->x;
	diff_y = user.y - mo->member->y;

	// 水平座標がほぼ等しい？
	if (-40 < diff_x && diff_x < +40) {
		// 垂直座標を調べる
		if (diff_y <= 0) {
			if (diff_y < -40) dir = 8; else mo->dir = 8;
		} else if (diff_y > 0) {
			if (diff_y > +40) dir = 2; else mo->dir = 2;
		}
	}
	// 垂直座標がほぼ等しい？
	else if (-40 < diff_y && diff_y < +40) {
		// 水平座標を調べる
		if (diff_x <= 0) {
			if (diff_x < -40) dir = 4; else mo->dir = 4;
		} else if (diff_x > 0) {
			if (diff_x > +40) dir = 6; else mo->dir = 6;
		}
	} else {
		return -1; // モンスターはユーザーの位置を捕捉できない
	}

	// モンスターは魔法を使える？
	if (state.isCaster &&
	    state.monsterStatus->INT > 0 && mo->magic->lifetime == 0) {
		// ユーザーを捕捉済み
		if (dir > 0 && mo->dir == dir) {
			battleCastSpell(mo->magic, state.monsterStatus->magic,
			                state.monsterStatus->INT,
			                mo->member->x, mo->member->y, dir);
		}
	}

	// 骸骨？
	if (using_candle()) {
		return mo->dir;
	} else {
		return dir;
	}
}

// 離脱する？
int BattleScene::userAttemptEscape(int dir)
{
	int escape;

	bgm_tempo(0); // 元のテンポに戻す

	auto &callback = BattleState::instance().escapeCallback;
	escape = callback && callback(dir);
	if (!escape && in_battle()) {
		bgm_tempo(1);
	}
	return escape;
}

// 攻撃してみる？
int BattleScene::userAttemptAttack(member_t *um)
{
	switch (um->type) {
	case MEMBER_MONSTER:
		if (!using_candle()) {
			battleAttackMonster(um);
		}
		return 1;
	case MEMBER_BOX:
		battleOpenBox(um);
		return 1;
	case MEMBER_GOODS:
		battleGetGoods(um);
		return 1;
	}
	return 0;
}

/* 攻撃の方向を返す。
 * (x1, y1)攻撃者の位置。(x2, y2)対象者の位置。d2対象者の向き */
int BattleScene::whichDirection(int x1, int y1, int x2, int y2, int d2)
{
	static const int direction_table[4][4] = {
		{ GUARD_BACK,  GUARD_SIDE,  GUARD_SIDE,  GUARD_FRONT },
		{ GUARD_SIDE,  GUARD_BACK,  GUARD_FRONT, GUARD_SIDE  },
		{ GUARD_SIDE,  GUARD_FRONT, GUARD_BACK,  GUARD_SIDE  },
		{ GUARD_FRONT, GUARD_SIDE,  GUARD_SIDE,  GUARD_BACK  }
	};
	int dx = x2 - x1;
	int dy = y2 - y1;

	switch (d2) {
	case 2: case 3: d2 = 0; break;
	case 1: case 4: d2 = 1; break;
	case 6: case 9: d2 = 2; break;
	case 7: case 8: d2 = 3; break;
	default:
		return 0; // どこを向いているか分からない
	}
	if (-40  < dx && dx < +40) {
		return direction_table[dy <= 0 ? 3 : 0][d2];
	}
	if (-40 < dy && dy < +40) {
		return direction_table[dx <= 0 ? 1 : 2][d2];
	}
	return 0;
}

// ユーザーがモンスターを攻撃
void BattleScene::battleAttackMonster(member_t *mm)
{
	auto &state = BattleState::instance();
	// 命中？
	int hit = random_integer(user.equipment[GOODS_WEAPON] + 1)
		<= user.status.fighter.rank;

	if (hit || using_hourglass()) {
		int guard;
		int damage;

		guard = whichDirection(user.x, user.y,
		                       mm->x, mm->y,
		                       state.monsters[mm->value].dir);

		damage = userError(user_attack_point())
			- monster_defend_point(state.monsterStatus, guard);

		if (damage >= 0) {
			// 命中
			decrementMonsterHP(mm, damage, 0);

			// 武器の熟練度を上げる
			if (userGetSkillfull()) {
				userSkillUp(GOODS_WEAPON, 1);
			}
			userAttack_ = mm;
			playSound(SoundId::attack);
		}
	}
}

// 魔法がモンスターを直撃。0: ミス。1: 当たった
int BattleScene::magicAttackMonster(member_t *mm, magic_t *ma, int deg)
{
	auto &state = BattleState::instance();
	int damage, killed;

	damage = userError(user_magic_point(state.monsterStatus->MGR[ma->scroll_type]));
	if (damage > 0) {
		killed = decrementMonsterHP(mm, damage, 1);

		if (!deg) {
			// 全体魔法ではない
			killedUserMagic_ = killed; // 魔法でやっつけたことを示すフラグ

			// 魔法の熟練度を上げる
			if (userGetSkillfull()) {
				userSkillUp(GOODS_SCROLL, 1);
			}

			userAttack_ = mm;
			playSound(SoundId::magic);
		}
		return 1;
	} else {
		if (!deg) {
			playSound(SoundId::failed);
			emit_message("Failed!"); // Deg系でなければ、一回ごと
		}
		return 0;
	}
}

// モンスター全滅
void BattleScene::battleMonstersDestroyed()
{
	user.environment.in_battle = 0;
	status_refresh(0);
	bgm_tempo(0); // 元のテンポに戻す
}

// モンスターのヒットポイントを減少させる
int BattleScene::decrementMonsterHP(member_t *mm, int howMany, int byMagic)
{
	auto &state = BattleState::instance();
	monster_t *mo;

	format_message("HIT-%d", howMany);

	mo = &state.monsters[mm->value];
	mo->HP -= howMany;

	// 死んだ？
	if (mo->HP < 0) {
		mo->state = MONSTER_KILLED;

		// カルマキャラクター
		if (state.monsterStatus->KRM > 0) {
			user.status.KRM += user.status.fighter.rank
				+ user.status.wizard.rank + 1;
		}

		if (!byMagic) {
			// 戦士の経験を積む
			user.status.fighter.EXP += state.monsterStatus->EXP;
		} else {
			// 魔法使いの経験を積む
			user.status.wizard.EXP += state.monsterStatus->EXP;
		}
		format_message("Killed!+%dexp", state.monsterStatus->EXP);

		// 全滅した？
		if (--state.numMonsters == 0) {
			battleMonstersDestroyed();
		}
		else
			status_draw_text(mm->value + kStatusMonsterHpLine, 5, "Dead !!",
			                 SDL_::Color::RED);
		return 1;
	}
	// ステータス画面更新
	status_draw_integer(mm->value + kStatusMonsterHpLine, 5,
	                    mo->HP, SDL_::Color::RED);
	return 0;
}

// モンスターがユーザーを攻撃
void BattleScene::battleAttackUser(member_t *mm)
{
	auto &state = BattleState::instance();
	int guard;
	int damage;

	// 命中率は100%

	guard = whichDirection(mm->x, mm->y, user.x, user.y, user.dir);

	damage = monsterError(monster_attack_point(state.monsterStatus));
	damage -= user_defend_point(guard);
	if (damage >= 0) {
		decrementUserHP(damage, 0);

		// 防具の熟練度を上げる
		// 正面？
		if (guard == GUARD_FRONT && userGetSkillfull()) {
			userSkillUp(GOODS_SHIELD, 1);
		}
		if (userGetSkillfull()) {
			userSkillUp(GOODS_ARMOUR, 1);
		}

		if (userDamage_ == nullptr) {
			playSound(SoundId::attack);
		}
		userDamage_ = mm; // 1ターンに複数回攻撃されることがある
	}
}

// 魔法がユーザーを直撃
void BattleScene::magicAttackUser(magic_t *ma)
{
	auto &state = BattleState::instance();
	int damage;

	damage = monsterError(monster_magic_point(state.monsterStatus, user_MGR()));
	if (damage > 0) {
		decrementUserHP(damage, 0);

		// 防具の熟練度を上げる
		if (userGetSkillfull()) {
			userSkillUp(GOODS_ARMOUR, 1);
			userSkillUp(GOODS_SHIELD, 1);
		}

		if (userDamage_ == nullptr) {
			playSound(SoundId::trapped);
		}
		userDamage_ = ma;
	}
}

// ユーザーのヒットポイントを減少させる
int BattleScene::decrementUserHP(int howMany, int noEcho)
{
	user.status.HP -= howMany;
	status_update_HP(SDL_::Color::RED);
	if (!noEcho) {
		format_message("DMG-%d", howMany);
	}
	return 0;
}

// 宝箱を開ける
void BattleScene::battleOpenBox(member_t *um)
{
	auto &state = BattleState::instance();
	if (random_integer(50 + user.environment.dungeon_level * 10)
	    < user_DEX()) {
		// 開いた？
		if (--um->frame < 0) {
			um->type = MEMBER_GOODS;

			if (um->value == 1) {
				// ウエイト: 危険なものが入っているかもしれない
				pauseFor(SDL_SCANCODE_SPACE, [this]() { onEnter(); });
				um->value = state.monsterStatus->goods; // 赤箱
			} else {
				um->value = state.monsterStatus->goods == GOODS_FOOD
					? GOODS_FOOD : GOODS_GOLD;
			}
			um->frame = index_goods[um->value];

			playSound(SoundId::treasure);
		} else {
			// 宝箱を開ける音はget.wavを使う
			playSound(SoundId::get);
		}
	}
}

// お宝ゲット
void BattleScene::battleGetGoods(member_t *gm)
{
	auto &state = BattleState::instance();
	int goods_type, goods_numb;

	playSound(SoundId::get);

	gm->type = MEMBER_UNUSED;

	goods_type = gm->value / GOODS_FACTOR;
	goods_numb = gm->value % GOODS_FACTOR;

	if (goods_type == GOODS_OTHER_ITEM) {
		// 即座に効果の出るアイテム
		switch (goods_data[GOODS_OTHER_ITEM][goods_numb].type) {
		case OTHER_CROWN:    user.status.CRN++; break;
		case OTHER_KEY:      user.status.KEY++; break;
		case OTHER_ELIXIR:   user.status.ELX++; break;
		case OTHER_MUSHROOM: user.status.max_HP += user.status.max_HP * 0.1; break;
		case OTHER_POTION:
			user.status.KRM = max(0, user.status.KRM - 5);
			goto its_poison;
		case OTHER_HAMMER:      user.status.STR += 10; break;
		case OTHER_PENDANT:     user.status.INT += 10; break;
		case OTHER_HOLY_BIBLE:  user.status.WIS += 10; break;
		case OTHER_BOOTS:       user.status.DEX += 10; break;
		case OTHER_MAGIC_GLOVE: userSkillUp(GOODS_WEAPON, 10); break;
		case OTHER_ROD:         userSkillUp(GOODS_SCROLL, 10); break;
		case OTHER_CRYSTAL:     user.status.MGR += 10; break;
		case OTHER_POTION2:
		its_poison:
			decrementUserHP(user.status.HP / 2, 1);
			emit_message("It's poison");
			userDamage_ = gm;
			playSound(SoundId::poison);
			return;
		}
	} else if (goods_type < GOODS_OTHER_ITEM) {
		// 在庫に加算
		user.inventory[goods_type][goods_numb].stock++;
	} else {
		int amount;

		if (gm->value == GOODS_GOLD) {
			if (state.monsterStatus->goods == GOODS_GOLD) {
				amount = state.monsterStatus->amount;
			} else {
				amount = (user.environment.dungeon_level + 1)
					* random_integer(10) * 2;
			}
			user.status.gold += amount;
			status_update_gold();
			format_message("Get-%dGold", amount);
		}
		else if (gm->value == GOODS_FOOD) {
			amount = state.monsterStatus->amount;
			user.status.food += amount;
			if (!in_battle())
				status_update_food();
			format_message("Get-%dFood", amount);
		}
		return;
	}
	format_message("Get %s", goods_data[goods_type][goods_numb].name);
}

void BattleScene::battleCastSpell(magic_t *ma, int scrollId, int INT,
                                  int x, int y, int dir)
{
	auto &state = BattleState::instance();
	int scroll_type, scroll_attribute;

	if (scrollId < 0 || MAX_GOODS <= scrollId || INT == 0)
		return;

	scroll_type = scroll_data()[scrollId].type;
	scroll_attribute = scroll_data()[scrollId].attribute;

	ma->lifetime    = INT;
	ma->scroll_type = scroll_type;
	ma->x           = x + 12;
	ma->y           = y + 12;
	ma->dir         = dir;
	ma->frame       = frame_magic[scroll_type];

	// 全体魔法？
	if (scroll_attribute) {
		ma->lifetime = 0;

		if (ma == &state.magics[MAX_MEMBER]) {
			// 術者はユーザー
			if (in_battle()) {
				int someone_hit = 0;
				member_t *mm = state.room->members;

				for (; mm < &state.room->members[MAX_MEMBER]; mm++) {
					if (mm->type == MEMBER_MONSTER) {
						someone_hit |= magicAttackMonster(mm, &state.magics[MAX_MEMBER], 1);
					}
				}
				if (someone_hit) {
					// 魔法の熟練度を上げる
					if (userGetSkillfull()) {
						userSkillUp(GOODS_SCROLL, 1);
					}
					userDegPhase_ = kDegPhaseHit;
					setGameTimer(kBattleInterval, [this]() { battleLoopMagic(); }); // Deg系魔法ループ
				} else
					goto failed;
			} else {
			failed:
				// 失敗
				playSound(SoundId::failed);
				emit_message("Failed!");
				return;
			}
		} else {
			// 術者はモンスター
			if (userDamage_ != nullptr)
				return;

			magicAttackUser(ma); // 1ターンに二回以上攻撃しない
		}
	}
	playSound(resolveCastSound(scroll_type));
}

member_t *BattleScene::moveMagic(magic_t *ma)
{
	int x = ma->x;
	int y = ma->y;

	switch (ma->dir) {
	case 2: y += kStepMagic; break;
	case 4: x -= kStepMagic; break;
	case 6: x += kStepMagic; break;
	case 8: y -= kStepMagic; break;
	}

	if (0 <= x && x < 360 - 16 && 0 <= y && y < 360 - 16) {
		ma->x = x;
		ma->y = y;
		ma->frame = frame_magic[ma->scroll_type] + (ma->frame + 1) % 2;
	} else {
		ma->lifetime = 0;
		return NULL;
	}
	return battleCanThrough(x, y, x + 15, y + 15);
}

int BattleScene::battleMoveMagic()
{
	auto &state = BattleState::instance();
	member_t *something;
	magic_t *ma;
	int i, update = 0;

	// モンスターの魔法
	for (i = 0, ma = state.magics; i < state.maxMonsters; i++, ma++) {
		if (ma->lifetime > 0) {
			ma->lifetime--;

			something = moveMagic(ma);

			if (ma->x < user.x + 40 && user.x < ma->x + 16 &&
			    ma->y < user.y + 40 && user.y < ma->y + 16) {
				// ユーザーに命中
				ma->lifetime = 0;
				magicAttackUser(ma);
			}
			else if (something != state.monsters[i].member &&
			         something != NULL &&
			         something != &memberWall_ &&
			         something != &memberLock_) {
				// 自分以外の壁や扉でないものに命中
				ma->lifetime = 0;
			}
			update = 1;
		}
	}

	// ユーザーの魔法
	ma = &state.magics[MAX_MEMBER];
	if (ma->lifetime > 0) {
		ma->lifetime--;

		something = moveMagic(ma);

		if (something != NULL) {
			if (something->type == MEMBER_MONSTER) {
				// モンスターに命中
				ma->lifetime = 0;
				magicAttackMonster(something, ma, 0);
			}
			else if (something->type == MEMBER_BOX ||
			         something->type == MEMBER_GOODS) {
				// 宝箱、お宝に命中
				ma->lifetime = 0;
				userAttack_ = something;
				playSound(SoundId::attack);
			}
		}
		update = 1;
	}
	return update;
}

int BattleScene::monstersMoveWalker()
{
	auto &state = BattleState::instance();
	monster_t *mo, *mo_end;
	int update = 0;

	for (mo = state.monsters, mo_end = mo + state.maxMonsters;
	     mo < mo_end; mo++) {
		if (mo->state == MONSTER_ALIVE && --mo->monster_timer < 0) {
			int x, y;
			int retry;

			mo->monster_timer = monsterTime();

			if (monsterCaptureUser(mo) == 0) {
				// 攻撃
				battleAttackUser(mo->member);
				goto update_frame;
			}

			// 機敏に方向を変える？
			if (random_integer(state.sensitive) == 0) {
				mo->dir = (this->*(kActivityTable[state.activityPattern].changeDir))(mo);
			}

			for (retry = 0; retry == 0; retry++) {
				x = mo->member->x;
				y = mo->member->y;

				// 進む
				switch (mo->dir) {
				case 2: y += kStepMonster; break;
				case 4: x -= kStepMonster; break;
				case 6: x += kStepMonster; break;
				case 8: y -= kStepMonster; break;
				default: goto done_move;
				}

				// 障害物を調べる
				if (monsterCanThrough(x, y, mo->dir) != NULL) {
					mo->dir = changeDirDancer(mo);
				} else {
					// 移動
					mo->member->x = x;
					mo->member->y = y;
					mo->magic->dir = mo->dir;
					update = 1;
					break;
				}
			}
		done_move:
			if (state.monsterStatus->activity & ACTIVITY_VIVID) {
			update_frame:
				update = 1;
				mo->member->frame++;
			}
			mo->member->frame = frame_monster[mo->dir] + mo->member->frame % 2;
		}
	}
	return update;
}

// 追跡型
int BattleScene::changeDirChaser(const monster_t *mo)
{
	// ユーザーの姿は見えない？
	if (using_demons_ring()) {
		return changeDirNeutral(mo);
	}
	if (random_integer(4) == 0) {
		return mo->dir;
	} else {
		int diff_x = user.x - mo->member->x;
		int diff_y = user.y - mo->member->y;
		int dx = (diff_x <= -40  ? -1 :
		          diff_x >= +40  ? +1 : 0);
		int dy = (diff_y <= -40 ? +3 :
		          diff_y >= +40 ? -3 : 0);
		int dir = 5 + dx + dy;

		switch (dir) {
		case 1: dir = random_integer(2) == 0 ? 4 : 2; break;
		case 3: dir = random_integer(2) == 0 ? 6 : 2; break;
		case 7: dir = random_integer(2) == 0 ? 4 : 8; break;
		case 9: dir = random_integer(2) == 0 ? 6 : 8; break;
		}
		return dir;
	}
}

// 逃走型
int BattleScene::changeDirEscapee(const monster_t *mo)
{
	// ユーザーを捕捉せずに恣意的に進む？
	if (random_integer(8) == 0) {
		return random_direction();
	} else {
		int diff_x = user.x - mo->member->x;
		int diff_y = user.y - mo->member->y;
		int dx = (diff_x <= -40 ? -1 :
		          diff_x >= +40 ? +1 : 0);
		int dy = (diff_y <= -40 ? +3 :
		          diff_y >= +40 ? -3 : 0);
		int dir = 5 - dx - dy;

		switch (dir) {
		case 1: dir = mo->dir != 4 ? 4 : 2; break;
		case 3: dir = mo->dir != 6 ? 6 : 2; break;
		case 7: dir = mo->dir != 4 ? 4 : 8; break;
		case 9: dir = mo->dir != 6 ? 6 : 8; break;
		}
		switch (dir) {
		case 2: case 8: if (mo->dir == dir) dir = random_integer(2) == 0 ? 4 : 6;
		case 4: case 6: if (mo->dir == dir) dir = random_integer(2) == 0 ? 2 : 8;
		}
		return dir;
	}
}

// 舞踏型
int BattleScene::changeDirDancer(const monster_t *mo)
{
	static const int table28[2] = { 4, 6 };
	static const int table46[2] = { 2, 8 };
	switch (mo->dir) {
	case 4:
	case 6: return table46[random_integer(2)];
	case 2:
	case 8: return table28[random_integer(2)];
	default: return random_direction();
	}
}

// 中立型
int BattleScene::changeDirNeutral(const monster_t *mo)
{
	int dir = random_direction();
	switch (dir) {
	case 2: case 1: return 2;
	case 4: case 7: return 4;
	case 6: case 3: return 6;
	case 8: case 9: return 8;
	}
	return dir;
}
