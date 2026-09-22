#include "xanadu.h"
#include "battle.h"
#include "status.h"
#include "equip.h"
#include "use_item.h"
#include "inventory.h"
#include "user_dead.h"
#include "animation.h"
#include "resources/Resources.h"
#include "resources/SoundId.h"
#include "sdl/LegacyPlatform.h"
#include "app/Application.h"

#define UPDATE_ABORT		-1

// 魔法詠唱SE。scroll_typeはSCROLL_NEEDLE(0)..SCROLL_DEATH(8)(goods.h参照)
namespace
{
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
}

#define BATTLE_INTERVAL		70	// 戦闘時インターバル

#define monster_time()		(monster_interval)
#define phantom_time()		20
#define dead_time()		1

#define STATUS_MONSTER_HP_LINE	8

#define STEP_USER		8
#define STEP_MONSTER		8
#define STEP_MAGIC		16

// モンスターの状態
#define MONSTER_ALIVE		0	// 生きている
#define MONSTER_KILLED		1	// 殺された
#define MONSTER_DEAD		3	// 死んだ
#define MONSTER_DISAPPEAR	4	// 消滅した
#define MONSTER_RIP		5	// 永眠中

// Deg 系魔法の状態
#define DEG_PHASE_INACTIVE	0	// 発動していない
#define DEG_PHASE_HIT		1	// ヒット
#define DEG_PHASE_DEAD		2	// 死亡
#define DEG_PHASE_DISAPPEAR	3	// 消滅

// 熟練度が上がる？
#define user_get_skillfull()	(random_integer(4) == 0)

// 熟練度アップ
#define user_skill_up(t, n)			\
  (user.inventory[t][user.equipment[t]].skill =	\
   min(skill_max[t], user.inventory[t][user.equipment[t]].skill + (n)))

// 熟練度の最大値
static const int skill_max[5] = { 255, 255, 200, 200, 255 };

// 誤差: ユーザー: 30/32..38/32、モンスター: 12/16..20/16
#define user_error(p)		(((p) * (random_integer(8) + 30)) / 32)
#define monster_error(p)	(((p) * (random_integer(8) + 12)) / 16)

// 戦場の情報
static member_t member_wall = { MEMBER_UNUSED, 0, 0, 0, 0 };
static member_t member_lock = { MEMBER_UNUSED, 0, 0, 0, 0 };
static room_t *battle_room;
static int battle_map[9][9];
static void construct_battle_map(void);

// メインループ
static void battle_loop(void);

// その他のループ
static member_t *user_attack; // ユーザー「が」攻撃した何か
static void *user_damage; // ユーザー「を」攻撃した何か
static int user_deg_phase; // ユーザーが Deg 系魔法を唱えた？
static void battle_loop_attack(void);
static void battle_loop_magic(void);

// 描画関連
static void update_background(void);

// 移動関連
static int keystate_SHIFT; // Shift キーの状態
static int (*thunk_battle_escape)(int dir);
static int battle_move_user(int dir);
static int battle_control_user_magic(int dir);
static int user_attempt_escape(int dir);
static int user_attempt_attack(member_t *um);
static member_t *battle_can_through(int x1, int y1, int x2, int y2);

// モンスターステータス関連
static SDL_::SubImage *battle_monster_image;
static monster_status_t *monster_status;
static int monster_interval; // モンスター行動間隔

// モンスター個体情報
static monster_t battle_monsters[MAX_MEMBER]; // モンスター個体情報
static int battle_max_monsters; // 合計のモンスター数(固定)
static int battle_num_monsters; // 現在のモンスター数(変動)

// モンスター移動関連その1
static int (*battle_move_monsters)(void);
static int monsters_move_walker(void);
static int monsters_move_rooted(void);
static int monsters_move_vision(void);
static member_t *monster_can_through(int x1, int y1, int dir);
static int monster_can_appear(member_t *mm);
static int monster_capture_user(monster_t *mo);

// モンスター移動関連その2
static int (*monster_change_dir)(const monster_t *mo);
static int monster_sensitive;	// 敏感さ
static int monster_is_caster;	// 魔法を使用する？
static int change_dir_chaser(const monster_t *mo);
static int change_dir_dancer(const monster_t *mo);
static int change_dir_neutral(const monster_t *mo);
static int change_dir_escapee(const monster_t *mo);

// 魔法関連
static magic_t battle_magics[MAX_MEMBER + 1];
static magic_t *battle_user_magic = &battle_magics[MAX_MEMBER];
static int killed_user_magic; // ユーザーの魔法がモンスターを倒した？
static int battle_move_magic(void);
static int magic_attack_monster(member_t *mm, magic_t *ma, int deg);
static void magic_attack_user(magic_t *ma);
static void battle_cast_spell(magic_t *ma, int scroll_id, int INT,
  int x, int y, int dir);

// 攻撃関連
static int decrement_monster_HP(member_t *mm, int how_many, int by_magic);
static int decrement_user_HP(int how_many, int no_echo);
static void battle_monsters_destroyed(void);
static void battle_attack_monster(member_t *mm);
static void battle_attack_user(member_t *mm);

// 宝箱・お宝関連
static void battle_get_goods(member_t *gm);
static void battle_open_box(member_t *um);

// 部屋の四辺のドアの位置
extern const point_t room_door_position[4] = {
  { 4, 8 }, { 0, 4 }, { 8, 4 }, { 4, 0 }
};

extern const int battle_frame_user[10] = { 8, 0, 8, 2, 0, 6, 2, 0, 6, 2 };
  
// これらの定数は field.c に定義されている
extern const point_t move_table[10];
extern const int frame_monster[10];
extern const int frame_magic[MAX_SCROLL_TYPE];

// unknownA テーブル
static struct {
  int		(*move_proc)(void);
  int		(*change_dir)(const monster_t *mo);
  int		sensitive;
  int		is_caster;
} unknownA_table[] = {
  { monsters_move_walker, change_dir_chaser,   8, 0 },	// 0: 追跡型 A
  { monsters_move_walker, change_dir_chaser,   4, 0 },	// 1: 追跡型 B
  { monsters_move_walker, change_dir_chaser,   1, 0 },	// 2: 追跡型 C
  { monsters_move_walker, change_dir_neutral, 16, 0 },	// 3: 中立型 A
  { monsters_move_walker, change_dir_neutral, 16, 0 },	// 4: 中立型 B
  { monsters_move_walker, change_dir_dancer,   1, 0 },	// 5: 舞踏型 A
  { monsters_move_rooted, change_dir_neutral,  8, 0 },	// 6: 固定型
  { monsters_move_vision, change_dir_neutral,  8, 1 },	// 7: ワープ型
  { monsters_move_walker, change_dir_dancer,   1, 1 },	// 8: 舞踏型 B
  { monsters_move_walker, change_dir_chaser,   4, 1 },	// 9: 追跡型 D
  { monsters_move_walker, change_dir_dancer,   1, 1 },	// 10: 舞踏型 C
  { monsters_move_walker, change_dir_escapee,  4, 1 },	// 11: 逃走型 A
  { monsters_move_walker, change_dir_escapee,  4, 0 }	// 12: 逃走型 B
};

int init_battle(room_t *room, const battle_t *suspended,
                int (*thunk_escape)(int dir))
{
  int n;
  
  battle_room = room;
  thunk_battle_escape = thunk_escape;
  
  // 変数の初期化
  user_attack = NULL;
  user_damage = NULL;
  user_deg_phase = DEG_PHASE_INACTIVE;
  killed_user_magic = 0;

  // モンスター情報
  battle_monster_image = frame_monsters[battle_room->monster_id / 4];
  monster_status = &monster_data[battle_room->monster_id];

  // 戦闘時の行動パターン
  n = monster_status->unknownA;
  if (n < 0 || sizeof(unknownA_table)/sizeof(unknownA_table[0]) <= n) {
    n = 0;
  }
  monster_change_dir = unknownA_table[n].change_dir;
  monster_sensitive  = unknownA_table[n].sensitive;
  monster_is_caster  = unknownA_table[n].is_caster;
#if 1
  battle_move_monsters = unknownA_table[n].move_proc;
#else
  // モンスターの移動関数
  if (monster_status->activity & ACTIVITY_TELEPORT) {
    battle_move_monsters = monsters_move_vision;
  }
  else if ((monster_status->activity & ACTIVITY_WALKER) == 0) {
    battle_move_monsters = monsters_move_rooted;
  }
  else {
    battle_move_monsters = monsters_move_walker;
  }
#endif
  
#if 1
  // 魔法を唱えられるようにする
  if (monster_status->unknownA == 6 &&
      monster_status->INT > 0 && battle_room->monster_id % MAX_VARIETY > 1) {
    monster_is_caster = 1;
  }
#endif
  
  // モンスターの行動間隔
  if (monster_status->AGL == 0) {
    monster_interval = 0;
  } else {
    monster_interval = user.status.AGL / monster_status->AGL;
  }

  // モンスター・魔法情報の初期化
  init_battle_monsters(suspended);

  // 戦場の作成
  construct_battle_map();

  // 戦闘中を示すフラグをセット
  user.environment.in_battle = battle_num_monsters > 0;
  
  return CONTEXT_BATTLE;
}

void construct_battle_map(void)
{
  int i, *p;

  // フィールド？
  if (!user.environment.in_tower) {
    // 床
    for (i = 0, p = (int *)battle_map; i < 81; i++, p++)
      *p = i % 2 == 0 ? tile_data.pattern0 : tile_data.pattern1;

    // 外壁
    for (i = 0; i < 9; i++) {
      if (battle_room->barrier[0] == BARRIER_WALL)
        battle_map[8][i] = tile_data.bricks;
      if (battle_room->barrier[1] == BARRIER_WALL)
        battle_map[i][0] = tile_data.bricks;
      if (battle_room->barrier[2] == BARRIER_WALL)
        battle_map[i][8] = tile_data.bricks;
      if (battle_room->barrier[3] == BARRIER_WALL)
        battle_map[0][i] = tile_data.bricks;        
    }
  } else {
    // 床
    for (i = 0, p = (int *)battle_map; i < 81; i++, p++)
      *p = tile_data.floor;

    // 外壁
    for (i = 0; i < 8; i++) {
      battle_map[0    ][i] = tile_data.marble;
      battle_map[i    ][8] = tile_data.marble;
      battle_map[i + 1][0] = tile_data.marble;
      battle_map[8][i + 1] = tile_data.marble;
    }
    // 出入り口
    for (i = 0; i < 4; i++) {
      int tile;
      int x, y;
      switch (battle_room->barrier[i]) {
      case BARRIER_OPEN: tile = tile_data.floor;    break;
      case BARRIER_LOCK: tile = tile_data.locked;   break;
      case BARRIER_EXIT: tile = tile_data.pattern1; break;
      default: continue;
      }
      x = room_door_position[i].x;
      y = room_door_position[i].y;
      battle_map[y][x] = tile;
    }
  }
}

int replace_battle_map(int x, int y, int tile)
{
  if (0 <= x && x < 9 && 0 <= y && y < 9) {
    battle_map[y][x] = tile;
    return 1;
  }
  return 0;
}

void init_battle_monsters(const battle_t *suspended)
{
  int i, n;

  if (!suspended) {
    // モンスターを初期化
    for (i = 0, n = 0; i < MAX_MEMBER; i++) {
      member_t *mm = &battle_room->members[i];
    
      if (member_monster(mm)) {
        monster_t *mo;
        int dir = random_direction();

        if (monster_status->activity & ACTIVITY_VIVID)
          mm->frame = frame_monster[dir] + random_integer(2);
        else
          mm->frame = frame_monster[dir];

        // モンスター識別番号
        mm->value = n;

        // モンスターの個体に関する情報
        mo = &battle_monsters[n];
        mo->member        = mm;
        mo->state         = MONSTER_ALIVE;
        mo->HP            = monster_status->max_HP * 100;
        mo->dir           = dir;
        mo->monster_timer = monster_time() - random_integer(4) + 2;
        mo->phantom_timer = phantom_time() + random_integer(8);
        mo->magic         = &battle_magics[n];

        // 方向の設定
        mo->dir = (*monster_change_dir)(mo);
        n++;
      }
    }
    battle_max_monsters = battle_num_monsters = n;

    // 魔法を初期化
    for (i = 0; i < MAX_MEMBER + 1; i++) {
      battle_magics[i].lifetime = 0;
    }
  } else {
    // 保存されている情報を復元
    battle_max_monsters = suspended->max_monsters;
    battle_num_monsters = 0;
    memcpy(battle_monsters, suspended->monsters, sizeof(battle_monsters));
    memcpy(battle_magics, suspended->magics, sizeof(battle_magics));

    // 生きているモンスターを構成員情報とリンクする
    for (i = 0; i < MAX_MEMBER; i++) {
      member_t *mm = &battle_room->members[i];
      if (member_monster(mm)) {
        battle_monsters[mm->value].member = mm;
        battle_num_monsters++; // 生きているモンスターの数
      }
    }
    // 死んでいるモンスターの member の内容はゴミであることに注意！
    // 魔法情報とリンクする
    for (i = 0; i < battle_max_monsters; i++) {
      battle_monsters[i].magic= &battle_magics[i];
    }
  }
}

void save_battle_monsters(void)
{
  user.battle.max_monsters = battle_max_monsters;
  memcpy(user.battle.monsters, battle_monsters, sizeof(battle_monsters));
  memcpy(user.battle.magics, battle_magics, sizeof(battle_magics));
}

namespace
{
constexpr SoundId kBattleSoundIds[] = {
	SoundId::dead, SoundId::magic, SoundId::failed, SoundId::lost_key,
	SoundId::attack, SoundId::trapped, SoundId::treasure, SoundId::get,
	SoundId::poison,
	SoundId::c_needle, SoundId::c_mittar, SoundId::c_deluge, SoundId::c_fire,
	SoundId::c_thunder, SoundId::c_poison, SoundId::c_corros, SoundId::c_tilte,
	SoundId::c_death,
};
}

void battle_create(void)
{
  auto &res = Resources::instance();
  auto &mixer = Application::instance().getMixer();
  for (SoundId id : kBattleSoundIds) {
    res.loadSound(mixer, id);
  }
}

void battle_destroy(void)
{
  auto &res = Resources::instance();
  for (SoundId id : kBattleSoundIds) {
    res.unloadSound(id);
  }
}

void battle_enter(void)
{
  // 画面の描画
  update_background();
  status_refresh(in_battle());

  if (in_battle()) {
    // 戦闘時のステータス画面
    int i;

    status_draw_text(7, 0, monster_status->name, SDL_::Color::RED);
    
    for (i = 0; i < battle_max_monsters; i++) {
      int row = i + STATUS_MONSTER_HP_LINE;
      if (battle_monsters[i].HP >= 0)
        status_draw_integer(row, 5, battle_monsters[i].HP, SDL_::Color::WHITE);
      else
        status_draw_text(row, 5, "Dead !!", SDL_::Color::RED);
    }
    for (; i < MAX_MEMBER; i++) {
      status_erase_line(i + STATUS_MONSTER_HP_LINE);
    }
    bgm_tempo(1); // テンポを早くする
  }

  // タイマーの設定
  set_timer(BATTLE_INTERVAL, battle_loop);
}

void battle_leave(void)
{
  kill_timer();
}

// メインループ
void battle_loop(void)
{
  static int mirror_wait;
  int (*move_proc)(int dir);
  int interval, update = 0;

  if (doping_AGL()) {
    mirror_wait = (mirror_wait + 1) % 2;
    interval = (BATTLE_INTERVAL / 2);
  } else {
    mirror_wait = 0;
    interval = BATTLE_INTERVAL;
  }
  
  // 時間の経過
  user_time_elapse(interval);
  set_timer(interval, battle_loop);
  
  // 死んだ？
  if (user.status.HP < 0) {
    user_damage = NULL;
    extend_context(init_user_dead(user.x, user.y, update_background));
    return;
  }

  // 前のターンでユーザーは何かを攻撃した？
  if (user_attack != NULL) {
    user_attack = NULL;
    update = 1;
    goto do_monster;
  }
  
  // 前のターンでユーザーは攻撃された？
  if (user_damage != NULL) {
    user_damage = NULL;
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
      begin_pause(SDL_SCANCODE_S, battle_enter);
      return;
    }
    // Ctrl-Q: 保存
    if (isKeyDown(SDL_SCANCODE_Q)) {
      save_battle_monsters();
      // フィールドの場合、戦闘中でなくても戦闘中として保存する
      user.environment.in_battle = 1;
      save_user();
      user.environment.in_battle = battle_num_monsters > 0;
      switch_context(CONTEXT_START_MENU);
      return;
    }
  } else {
    // ENTER: アイテム使用
    if (isReturnDown() &&
        user.equipment[GOODS_MAGIC_ITEM] < MAX_GOODS) {
      extend_context(init_use_item(update_background, battle_room));
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

  // Shift キー状態
  keystate_SHIFT = isShiftDown();

  // CTRL キーが押されている？
  if (isCtrlDown()) {
    move_proc = battle_control_user_magic;
  } else {
    move_proc = battle_move_user;
  }

  // SPACE: 魔法念唱
  if (isKeyDown(SDL_SCANCODE_SPACE)) {
    if (battle_user_magic->lifetime == 0) {
      battle_cast_spell(battle_user_magic,
                        user.equipment[GOODS_SCROLL],
                        user_INT(),
                        user.x, user.y, user.dir);
      update = 1;
    }
  }
  else if (isKeyDown(SDL_SCANCODE_DOWN))  update = move_proc(2);
  else if (isKeyDown(SDL_SCANCODE_LEFT))  update = move_proc(4);
  else if (isKeyDown(SDL_SCANCODE_RIGHT)) update = move_proc(6);
  else if (isKeyDown(SDL_SCANCODE_UP))    update = move_proc(8);

  // 中断？
  if (update < 0) {
    return;
  }

  // 何か攻撃した？
  if (user_attack != NULL) {
    goto done;
  }
  
do_monster:
  if (in_battle() && !mirror_wait && !using_hourglass()) {
    // モンスターの移動
    update |= (*battle_move_monsters)();
    
    // ユーザーを攻撃した？
    if (user_damage != NULL) {
      update |= 1;
      goto do_magic;
    }
  }

do_magic:  
  // ユーザーの直接攻撃に対して同期を取らなければならない
  if (!mirror_wait) {
    update |= battle_move_magic();
  }
  
done:
  // 更新する？
  if (update)
    update_background();

  if (user_attack != NULL) {
    // ダメージ個所を反転
    inverse_image(clip_main, user_attack->x, user_attack->y, mask_damaged);
    set_timer(BATTLE_INTERVAL, battle_loop_attack);
  }
}

void battle_loop_attack(void)
{
  member_t *mm = user_attack;
  int update = 0;

  if (mm->type == MEMBER_BOX || mm->type == MEMBER_GOODS) {
    emit_message("Unlucky!");
    mm->type = MEMBER_UNUSED;
    set_timer_proc(battle_loop); // メインループに戻る
    update = 1;
  }
  // ごくまれに消える瞬間にヒットすることがある
  else if (member_monster(mm)) {
    monster_t *mo = &battle_monsters[mm->value];
    
    switch (mo->state) {
    case MONSTER_ALIVE:
      status_draw_integer(mm->value + STATUS_MONSTER_HP_LINE, 5,
                          mo->HP, SDL_::Color::WHITE);
      set_timer_proc(battle_loop); // メインループに戻る
      return;

    case MONSTER_KILLED:
      mo->monster_timer = dead_time();
      mo->state = MONSTER_DEAD;
      update = 1;
      playSound(SoundId::dead);
      break;

    case MONSTER_DEAD:
      if (--mo->monster_timer < 0) {
        // mo->monster_timer = 0;
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
        mm->value = killed_user_magic || in_battle() ? 0 : 1;
        killed_user_magic = 0;
        
        set_timer_proc(battle_loop); // メインループに戻る
        // update = 1; // たぶん無駄なフレーム
      }
      break;
    }
  }
  // 更新する？
  if (update)
    update_background();
}

void battle_loop_magic(void)
{
  static int timer;
  int update = 0;
  
  switch (user_deg_phase) {
  case DEG_PHASE_HIT:
    {
      int someone_dead = 0;
      int i;
      monster_t *mo = battle_monsters;

      for (i = 0; i < battle_max_monsters; i++, mo++) {
        if (mo->state == MONSTER_KILLED) {
          mo->state = MONSTER_DEAD;
          someone_dead = 1;
        }
        else if (mo->state == MONSTER_ALIVE) {
          status_draw_integer(i + STATUS_MONSTER_HP_LINE, 5,
                              mo->HP, SDL_::Color::WHITE);
        }
      }

      if (!someone_dead) {
        user_deg_phase = DEG_PHASE_INACTIVE;
      } else {
        // 誰か死んだ
        timer = dead_time();
        user_deg_phase = DEG_PHASE_DEAD;
      }
      playSound(SoundId::magic);
      update = 1;
    }
    break;

  case DEG_PHASE_DEAD:
    if (--timer < 0) {
      monster_t *mo = battle_monsters;

      for (; mo < &battle_monsters[battle_max_monsters]; mo++) {
        if (mo->state == MONSTER_DEAD) {
          mo->state = MONSTER_DISAPPEAR;
        }
      }
      timer = dead_time();
      user_deg_phase = DEG_PHASE_DISAPPEAR;
      playSound(SoundId::dead);
      update = 1;
    }
    break;
    
  case DEG_PHASE_DISAPPEAR:
    if (--timer < 0) {
      monster_t *mo = battle_monsters;

      for (; mo < &battle_monsters[battle_max_monsters]; mo++) {
        if (mo->state == MONSTER_DISAPPEAR) {
          mo->state = MONSTER_RIP;
          
          // 宝箱
          mo->member->type = MEMBER_BOX;
          mo->member->frame = 3;
          mo->member->value = 0;
        }
      }
      user_deg_phase = DEG_PHASE_INACTIVE;
      update = 1;
    }
    break;
  }
  
  if (user_deg_phase == DEG_PHASE_INACTIVE) {
    set_timer_proc(battle_loop); // メインループに戻る
  }
  if (update)
    update_background();
}

void update_background(void)
{
  member_t *mm;
  magic_t *ma;
  int i, j;
  
  // 背景
  if (in_darkness()) {
    fill_image(clip_main, 0, 0, clip_main->getWidth(), clip_main->getHeight(), SDL_::Color::BLACK);
  } else {
    int *map = (int *)battle_map;
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
  mm = battle_room->members;
  for (; mm < &battle_room->members[MAX_MEMBER]; mm++) {
    switch (mm->type) {
    case MEMBER_MONSTER:
      {
        monster_t *mo = &battle_monsters[mm->value];
        switch (mo->state) {
        case MONSTER_ALIVE:
        case MONSTER_KILLED:
          draw_sprite(clip_main, mm->x, mm->y, battle_monster_image[mm->frame]);
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
  ma = battle_magics;
  for (; ma < &battle_magics[MAX_MEMBER + 1]; ma++) {
    if (ma->lifetime > 0) {
      draw_sprite(clip_main, ma->x, ma->y, frame_magics[ma->frame]);
    }
  }

  // ダメージ
  if (user_deg_phase == DEG_PHASE_HIT) {
    mm = battle_room->members;

    for (; mm < &battle_room->members[MAX_MEMBER]; mm++) {
      if (mm->type == MEMBER_MONSTER) {
        inverse_image(clip_main, mm->x, mm->y, mask_damaged);
      }
    }
  }
  if (user_damage != NULL) {
    inverse_image(clip_main, user.x, user.y, mask_damaged);
  }
  update_region(rect_main.x, rect_main.y, rect_main.width, rect_main.height);
}

int battle_move_user(int dir)
{
  member_t *something;
  int x, y;
  int update = 0;

  // シフトキーが押されていないならば方向を変える
  if (!keystate_SHIFT && user.dir != dir) {
    user.dir = dir;
    update = 1;
  }

  // その方向へ一歩進む
  x = user.x + move_table[dir].x * STEP_USER;
  y = user.y + move_table[dir].y * STEP_USER;

  // 離脱しようとしている？
  if (x < 0 || x > 360 - 40 || y < 0 || y > 360 - 40) {
    if (update) {
      // フレーム更新
      user.frame = battle_frame_user[user.dir] + (user.frame + 1) % 2;
    }
    if (user_attempt_escape(dir)) {
      return UPDATE_ABORT;
    }
    goto done;
  }

  // 進もうとしている位置の障害物を調べる
  switch (dir) {
  case 2: something = battle_can_through(x, y + 39, x + 39, y + 39); break;
  case 4: something = battle_can_through(x, y, x, y + 39); break;
  case 6: something = battle_can_through(x + 39, y, x + 39, y + 39); break;
  case 8: something = battle_can_through(x, y, x + 39, y); break;
  default: goto do_move;
  }

  if (something == NULL) {
    // 障害物はない
  do_move:
    user.x = x;
    user.y = y;
    update = 1;
    
  } else {
    if (!keystate_SHIFT && user_attempt_attack(something)) {
      update = 1;
      goto done;
    }

    // 壁？
    if (something == &member_wall || something == &member_lock) {
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
    if (in_tower() && something == &member_lock && user_use_key()) {
      int i;
      
      // member_lock には便利な値が書き込まれている
      x = member_lock.x;
      y = member_lock.y;
      battle_map[y][x] = tile_data.floor;

      for (i = 0; i < 4; i++) {
        if (room_door_position[i].x == x &&
            room_door_position[i].y == y) {

          // 鍵を開ける
          battle_room->barrier[i] = BARRIER_OPEN;
          break;
        }
      }
      
      emit_message("Lost key");
      playSound(SoundId::lost_key);
      
      extend_context(init_animation_tile(tile_data.tower_open, 3,
                                         x * 40, y * 40,
                                         update_background));
      update = UPDATE_ABORT;
    }
  }
done:
  if (update) {
    // フレーム更新
    user.frame = battle_frame_user[user.dir] + (user.frame + 1) % 2;
  }
  return update;
}

int battle_control_user_magic(int dir)
{
  battle_user_magic->dir = dir;
  return 0; // 方向を変えるだけ
}

// モンスターの移動。静止
int monsters_move_rooted(void)
{
  monster_t *mo, *mo_end;
  int update = 0;

  for (mo = battle_monsters, mo_end = mo + battle_max_monsters;
       mo < mo_end; mo++) {
    if (mo->state == MONSTER_ALIVE && --mo->monster_timer < 0) {

      mo->monster_timer = monster_time();

      // 隣にユーザーがいる？
      if (monster_capture_user(mo) == 0) {
        battle_attack_user(mo->member);
        goto update_frame;
      }

      // フレーム更新
      if ((monster_status->activity & ACTIVITY_VIVID) != 0) {
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
int monsters_move_vision(void)
{
  monster_t *mo, *mo_end;
  int update = 0;

  for (mo = battle_monsters, mo_end = mo + battle_max_monsters;
       mo < mo_end; mo++) {
    member_t *mm = mo->member;
    int dir;

    // 幻影時間はモンスターの素早さに関わらず時を刻む
    mo->phantom_timer--;
    
    switch (mm->type) {
    case MEMBER_MONSTER:
      {
        // もうすぐ消える？
        if ((monster_status->activity & ACTIVITY_FADEOUT) != 0 &&
            mo->phantom_timer == 4) {
          mm->frame = 1;
          goto update_frame;
        }
          
        // 消える時間だ？
        if (mo->phantom_timer < 0) {
          mo->phantom_timer = phantom_time();
          mm->type = MEMBER_UNSEEN;
          update = 1;
        }
        // 行動する時間だ？
        else if (--mo->monster_timer < 0) {
          mo->monster_timer= monster_time();
          
          dir = monster_capture_user(mo);
          if (dir == 0) {
            // 攻撃
            battle_attack_user(mm);
            update = 1;
          }
          else if (dir > 0) {
            mo->dir = dir;
            goto update_frame;
          }
        }
        // フレーム更新
        if ((monster_status->activity & ACTIVITY_FADEOUT) == 0) {
          mm->frame++;
        update_frame:
          mm->frame = frame_monster[mo->dir] + mm->frame % 2;
          update = 1;
        }
      }
      break;
      
    case MEMBER_UNSEEN:
      if (mo->phantom_timer < 0 && monster_can_appear(mm)) {
        // 現れる場所を確保できた
        mo->phantom_timer = phantom_time();
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
static member_t *battle_hit_test(int x, int y)
{
  if (0 <= x && x < 360 && 0 <= y && y < 360) {
    member_t *mm;
    int i;
    int map;

    for (i = 0, mm = battle_room->members; i < MAX_MEMBER; i++, mm++) {
      // モンスターまたはお宝？
      if (mm->type > 0) {
        if (mm->x <= x && x < mm->x + 40 && mm->y <= y && y < mm->y + 40)
          return mm;
      }
    }
    
    // 地形タイルを調べる
    map = battle_map[y / 40][x / 40];

    // 扉？
    if (map == tile_data.locked) {
      member_lock.x = x / 40;
      member_lock.y = y / 40;
      return &member_lock;
    }

    // 壁？
    if (tile_data.flags[map] & TILE_WALL) {
      member_wall.value = tile_data.flags[map];
      return &member_wall;
    }
    
    return NULL;
  }
  return &member_wall;
}

member_t * monster_can_through(int x1, int y1, int dir)
{
  member_t *um;
  int x2 = x1;
  int y2 = y1;
  switch (dir) {
  case 2: y1 += 39; x2 += 39; y2 = y1; break;
  case 4: y2 += 39; break;
  case 6: x1 += 39; x2 = x1; y2 += 39; break;
  case 8: x2 += 39; break;
  default: return &member_wall;
  }
  // ユーザーと接触する？
  if ((user.x <= x1 && x1 < user.x + 40 && user.y <= y1 && y1 < user.y + 40) ||
      (user.x <= x2 && x2 < user.x + 40 && user.y <= y2 && y2 < user.y + 40)) {
    return &member_wall;
  }
  return (um = battle_hit_test(x1, y1)) != NULL ? um
    : battle_hit_test(x2, y2);
}

member_t *battle_can_through(int x1, int y1, int x2, int y2)
{
  member_t *um1 = battle_hit_test(x1, y1);
  member_t *um2 = battle_hit_test(x2, y2);

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
  return um1 == &member_wall ? um2 : um1;
}

int monster_can_appear(member_t *mm)
{
  // 現れる場所を決める(ユーザーの歩幅にあわせる)
  int x = random_integer(240 / STEP_USER) * STEP_USER + 40;
  int y = random_integer(240 / STEP_USER) * STEP_USER + 40;

  // そこには誰もいない？
  if (battle_hit_test(x,      y     ) == NULL &&
      battle_hit_test(x + 39, y + 39) == NULL &&
      battle_hit_test(x + 39, y     ) == NULL &&
      battle_hit_test(x,      y + 39) == NULL &&
#if 0
      // プレイヤーの軸上に現れない
      (x <= user.x - 40 || user.x + 40 <= x) &&
      (y <= user.y - 40 || user.y + 40 <= y)
#else
      // プレイヤーの軸上に現れる
      !((user.x - 40 < x && x < user.x + 40) &&
        (user.y - 40 < y && y < user.y + 40))
#endif
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
 *  0	ユーザーは隣にいる。mo->dir はユーザーのいる方向に修正される。
 * 2/8	ユーザーは垂直方向に見つかった。
 * 4/6	ユーザーは水平方向に見つかった。*/
int monster_capture_user(monster_t *mo)
{
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
  if (monster_is_caster &&
      monster_status->INT > 0 && mo->magic->lifetime == 0) {
    // ユーザーを捕捉済み
    if (dir > 0 && mo->dir == dir) {
      battle_cast_spell(mo->magic, monster_status->magic,
                        monster_status->INT,
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
int user_attempt_escape(int dir)
{
  int escape;
  
  bgm_tempo(0); // 元のテンポに戻す
  
  escape = thunk_battle_escape && (*thunk_battle_escape)(dir);
  if (!escape && in_battle()) {
    bgm_tempo(1);
  }
  return escape;
}

// 攻撃してみる？
int user_attempt_attack(member_t *um)
{
  switch (um->type) {
  case MEMBER_MONSTER:
    if (!using_candle()) {
      battle_attack_monster(um);
    }
    return 1;
  case MEMBER_BOX:
    battle_open_box(um);
    return 1;
  case MEMBER_GOODS:
    battle_get_goods(um);
    return 1;
  }
  return 0;
}

/* 攻撃の方向を返す。
 * (x1, y1) 攻撃者の位置。(x2, y2) 対象者の位置。d2 対象者の向き */
static int which_direction(int x1, int y1, int x2, int y2, int d2)
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
void battle_attack_monster(member_t *mm)
{
  // 命中？
  int hit = random_integer(user.equipment[GOODS_WEAPON] + 1)
    <= user.status.fighter.rank;

  if (hit || using_hourglass()) {
    int guard;
    int damage;
    
    guard = which_direction(user.x, user.y,
                            mm->x, mm->y,
                            battle_monsters[mm->value].dir);
    
    damage = user_error(user_attack_point())
      - monster_defend_point(monster_status, guard);
    
    if (damage >= 0) {
      // 命中
      decrement_monster_HP(mm, damage, 0);

      // 武器の熟練度を上げる
      if (user_get_skillfull()) {
        user_skill_up(GOODS_WEAPON, 1);
      }
      user_attack = mm;
      playSound(SoundId::attack);
    }
  }
}

// 魔法がモンスターを直撃。0: ミス。1: 当たった
int magic_attack_monster(member_t *mm, magic_t *ma, int deg)
{
  int damage, killed;

  damage = user_error(user_magic_point(monster_status->MGR[ma->scroll_type]));
  if (damage > 0) {
    killed = decrement_monster_HP(mm, damage, 1);

    if (!deg) {
      // 全体魔法ではない
      killed_user_magic = killed; // 魔法でやっつけたことを示すフラグ

      // 魔法の熟練度を上げる
      if (user_get_skillfull()) {
        user_skill_up(GOODS_SCROLL, 1);
      }

      user_attack = mm;
      playSound(SoundId::magic);
    }
    return 1;
  } else {
    if (!deg) {
      playSound(SoundId::failed);
      emit_message("Failed!"); // Deg 系でなければ、一回ごと
    }
    return 0;
  }
}

// モンスター全滅
void battle_monsters_destroyed(void)
{
  user.environment.in_battle = 0;
  status_refresh(0);
  bgm_tempo(0); // 元のテンポに戻す
}

// モンスターのヒットポイントを減少させる
int decrement_monster_HP(member_t *mm, int how_many, int by_magic)
{
  monster_t *mo;

  format_message("HIT-%d", how_many);
  
  mo = &battle_monsters[mm->value];
  mo->HP -= how_many;
  
  // 死んだ？
  if (mo->HP < 0) {
    mo->state = MONSTER_KILLED;

    // カルマキャラクター
    if (monster_status->KRM > 0) {
      user.status.KRM += user.status.fighter.rank
        + user.status.wizard.rank + 1;
    }
    
    if (!by_magic) {
      // 戦士の経験を積む
      user.status.fighter.EXP += monster_status->EXP;
    } else {
      // 魔法使いの経験を積む
      user.status.wizard.EXP += monster_status->EXP;
    }
    format_message("Killed!+%dexp", monster_status->EXP);
    
    // 全滅した？
    if (--battle_num_monsters == 0) {
      battle_monsters_destroyed();
    }
    else
      status_draw_text(mm->value + STATUS_MONSTER_HP_LINE, 5, "Dead !!",
                       SDL_::Color::RED);
    return 1;
  }
  // ステータス画面更新
  status_draw_integer(mm->value + STATUS_MONSTER_HP_LINE, 5,
                      mo->HP, SDL_::Color::RED);
  return 0;
}

// モンスターがユーザーを攻撃
void battle_attack_user(member_t *mm)
{
  int guard;
  int damage;

  // 命中率は100%
  
  guard = which_direction(mm->x, mm->y, user.x, user.y, user.dir);
  
  damage = monster_error(monster_attack_point(monster_status));
  damage -= user_defend_point(guard);
  if (damage >= 0) {
    decrement_user_HP(damage, 0);
    
    // 防具の熟練度を上げる
    // 正面？
    if (guard == GUARD_FRONT && user_get_skillfull()) {
      user_skill_up(GOODS_SHIELD, 1);
    }
    if (user_get_skillfull()) {
      user_skill_up(GOODS_ARMOUR, 1);
    }
    
    if (user_damage == NULL) {
      playSound(SoundId::attack);
    }
    user_damage = mm; // 1 ターンに複数回攻撃されることがある
  }
}

// 魔法がユーザーを直撃
void magic_attack_user(magic_t *ma)
{
  int damage;

  damage = monster_error(monster_magic_point(monster_status, user_MGR()));
  if (damage > 0) {
    decrement_user_HP(damage, 0);

    // 防具の熟練度を上げる
    if (user_get_skillfull()) {
      user_skill_up(GOODS_ARMOUR, 1);
      user_skill_up(GOODS_SHIELD, 1);
    }
    
    if (user_damage == NULL) {
#if 0
      playSound(SoundId::magic);
#else
      playSound(SoundId::trapped);
#endif 
    }
    user_damage = ma;
  }
}

// ユーザーのヒットポイントを減少させる
int decrement_user_HP(int how_many, int no_echo)
{
  user.status.HP -= how_many;
  status_update_HP(SDL_::Color::RED);
  if (!no_echo) {
    format_message("DMG-%d", how_many);
  }
  return 0;
}

// 宝箱を開ける
void battle_open_box(member_t *um)
{
  if (random_integer(50 + user.environment.dungeon_level * 10)
      < user_DEX()) {
    // 開いた？
    if (--um->frame < 0) {
      um->type = MEMBER_GOODS;
    
      if (um->value == 1) {
        // ウエイト: 危険なものが入っているかもしれない
        begin_pause(SDL_SCANCODE_SPACE, battle_enter);
        um->value = monster_status->goods; // 赤箱
      } else {
        um->value = monster_status->goods == GOODS_FOOD
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
void battle_get_goods(member_t *gm)
{
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
    case OTHER_MAGIC_GLOVE: user_skill_up(GOODS_WEAPON, 10); break;
    case OTHER_ROD:         user_skill_up(GOODS_SCROLL, 10); break;
    case OTHER_CRYSTAL:     user.status.MGR += 10; break;
    case OTHER_POTION2:
    its_poison:
      decrement_user_HP(user.status.HP / 2, 1);
      emit_message("It's poison");
      user_damage = gm;
      playSound(SoundId::poison);
      return;
    }
  } else if (goods_type < GOODS_OTHER_ITEM) {
    // 在庫に加算
    user.inventory[goods_type][goods_numb].stock++;
  } else {
    int amount;

    if (gm->value == GOODS_GOLD) {
      if (monster_status->goods == GOODS_GOLD) {
        amount = monster_status->amount;
      } else {
        amount = (user.environment.dungeon_level + 1)
          * random_integer(10) * 2;
      }
      user.status.gold += amount;
      status_update_gold();
      format_message("Get-%dGold", amount);
    }
    else if (gm->value == GOODS_FOOD) {
      amount = monster_status->amount;
      user.status.food += amount;
      if (!in_battle())
        status_update_food();
      format_message("Get-%dFood", amount);
    }
    return;
  }
  format_message("Get %s", goods_data[goods_type][goods_numb].name);
}

void battle_cast_spell(magic_t *ma, int scroll_id, int INT,
                       int x, int y, int dir)
{
  int scroll_type, scroll_attribute;

  if (scroll_id < 0 || MAX_GOODS <= scroll_id || INT == 0)
    return;

  scroll_type = scroll_data()[scroll_id].type;
  scroll_attribute = scroll_data()[scroll_id].attribute;

  ma->lifetime    = INT;
  ma->scroll_type = scroll_type;
  ma->x           = x + 12;
  ma->y           = y + 12;
  ma->dir         = dir;
  ma->frame       = frame_magic[scroll_type];

  // 全体魔法？
  if (scroll_attribute) {
    ma->lifetime = 0;
    
    if (ma == battle_user_magic) {
      // 術者はユーザー
      if (in_battle()) {
        int someone_hit = 0;
        member_t *mm = battle_room->members;
        
        for (; mm < &battle_room->members[MAX_MEMBER]; mm++) {
          if (mm->type == MEMBER_MONSTER) {
            someone_hit |= magic_attack_monster(mm, battle_user_magic, 1);
          }
        }
        if (someone_hit) {
          // 魔法の熟練度を上げる
          if (user_get_skillfull()) {
            user_skill_up(GOODS_SCROLL, 1);
          }
          user_deg_phase = DEG_PHASE_HIT;
          set_timer(BATTLE_INTERVAL, battle_loop_magic); // Deg 系魔法ループ
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
      if (user_damage != NULL)
        return;
      
      magic_attack_user(ma); // 1 ターンに二回以上攻撃しない
    }
  }
  playSound(resolveCastSound(scroll_type));
}

static member_t *move_magic(magic_t *ma)
{
  int x = ma->x;
  int y = ma->y;

  switch (ma->dir) {
  case 2: y += STEP_MAGIC; break;
  case 4: x -= STEP_MAGIC; break;
  case 6: x += STEP_MAGIC; break;
  case 8: y -= STEP_MAGIC; break;
  }

  if (0 <= x && x < 360 - 16 && 0 <= y && y < 360 - 16) {
    ma->x = x;
    ma->y = y;
    ma->frame = frame_magic[ma->scroll_type] + (ma->frame + 1) % 2;
  } else {
    ma->lifetime = 0;
    return NULL;
  }
  return battle_can_through(x, y, x + 15, y + 15);
}

int battle_move_magic(void)
{
  member_t *something;
  magic_t *ma;
  int i, update = 0;

  // モンスターの魔法
  for (i = 0, ma = battle_magics; i < battle_max_monsters; i++, ma++) {
    if (ma->lifetime > 0) {
      ma->lifetime--;

      something = move_magic(ma);
      
      if (ma->x < user.x + 40 && user.x < ma->x + 16 &&
          ma->y < user.y + 40 && user.y < ma->y + 16) {
        // ユーザーに命中
        ma->lifetime = 0;
        magic_attack_user(ma);
      }
      else if (something != battle_monsters[i].member &&
               something != NULL &&
               something != &member_wall &&
               something != &member_lock) {
        // 自分以外の壁や扉でないものに命中
        ma->lifetime = 0;
      }
      update = 1;
    }
  }

  // ユーザーの魔法
  ma = battle_user_magic;
  if (ma->lifetime > 0) {
    ma->lifetime--;
    
    something = move_magic(ma);

    if (something != NULL) {
      if (something->type == MEMBER_MONSTER) {
        // モンスターに命中
        ma->lifetime = 0;
        magic_attack_monster(something, ma, 0);
      }
      else if (something->type == MEMBER_BOX ||
               something->type == MEMBER_GOODS) {
        // 宝箱、お宝に命中
        ma->lifetime = 0;
        user_attack = something;
        // playSound(SoundId::magic);
        playSound(SoundId::attack);
      }
    }
    update = 1;
  }
  return update;
}

int monsters_move_walker(void)
{
  monster_t *mo, *mo_end;
  int update = 0;
  
  for (mo = battle_monsters, mo_end = mo + battle_max_monsters;
       mo < mo_end; mo++) {
    if (mo->state == MONSTER_ALIVE && --mo->monster_timer < 0) {
      int x, y;
      int retry;

      mo->monster_timer = monster_time();
      
      if (monster_capture_user(mo) == 0) {
        // 攻撃
        battle_attack_user(mo->member);
        goto update_frame;
      }

      // 機敏に方向を変える？
      if (random_integer(monster_sensitive) == 0) {
        mo->dir = (*monster_change_dir)(mo);
      }

      for (retry = 0; retry == 0; retry++) {
        x = mo->member->x;
        y = mo->member->y;
      
        // 進む
        switch (mo->dir) {
        case 2: y += STEP_MONSTER; break;
        case 4: x -= STEP_MONSTER; break;
        case 6: x += STEP_MONSTER; break;
        case 8: y -= STEP_MONSTER; break;
        default: goto done_move;
        }

        // 障害物を調べる
        if (monster_can_through(x, y, mo->dir) != NULL) {
#if 1
          mo->dir = change_dir_dancer(mo);
#else
          break;
#endif
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
      if (monster_status->activity & ACTIVITY_VIVID) {
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
int change_dir_chaser(const monster_t *mo)
{
  // ユーザーの姿は見えない？
  if (using_demons_ring()) {
    return change_dir_neutral(mo);
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
int change_dir_escapee(const monster_t *mo)
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
int change_dir_dancer(const monster_t *mo)
{
  static int table28[2] = { 4, 6 };
  static int table46[2] = { 2, 8 };
  switch (mo->dir) {
  case 4:
  case 6: return table46[random_integer(2)];
  case 2:
  case 8: return table28[random_integer(2)];
  default: return random_direction();
  }
}

// 中立型
int change_dir_neutral(const monster_t *mo)
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
