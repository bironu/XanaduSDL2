#include "field.h"
#include "battle.h"
#include "boss.h"
#include "shop.h"
#include "cave.h"
#include "equip.h"
#include "use_item.h"
#include "inventory.h"
#include "status.h"
#include "user_dead.h"
#include "animation.h"
#include "pause.h"
#include "resources/Resources.h"
#include "resources/ImageId.h"
#include "resources/SoundId.h"
#include "resources/MusicId.h"
#include "app/Application.h"

namespace
{
constexpr SoundId kFieldSoundIds[] = { SoundId::lost_key, SoundId::trapped, SoundId::encount };
}

MusicId resolveFieldMusic(int scenario, int dungeonLevel)
{
	if (scenario == 0) {
		return MusicId::xanadu; // XA1_DEFAULT_FIELDが全レベルに適用される(個別上書きなし)
	}
	static constexpr MusicId kXa2Field[MAX_DUNGEON_LEVEL] = {
		MusicId::xana2_XANA2_01, MusicId::xana2_XANA2_02, MusicId::xana2_XANA2_03,
		MusicId::xana2_XANA2_04, MusicId::xana2_XANA2_05, MusicId::xana2_XANA2_06,
		MusicId::xana2_XANA2_07, MusicId::xana2_XANA2_08, MusicId::xana2_XANA2_09,
		MusicId::xana2_XANA2_10, MusicId::xana2_XANA2_11,
	};
	if (dungeonLevel < 0 || MAX_DUNGEON_LEVEL <= dungeonLevel) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "resolveFieldMusic: dungeon_level out of range %d\n", dungeonLevel);
		return MusicId::none;
	}
	return kXa2Field[dungeonLevel];
}

// タワーは現状の設定ではフィールドと同じ曲になる(未設定時にフィールドへフォールバックするため)
MusicId resolveTowerMusic(int scenario, int dungeonLevel)
{
	return resolveFieldMusic(scenario, dungeonLevel);
}

#define GRAVITY_RATE		0	// 重力発生タイミング
#define GRAVITY_WAIT		1	// 入力用重力ウエイト

#define MAX_ACCELERATION	4	// 最大加速度

#define UPDATE_ABORT		-1	// すぐさまループを抜ける

#define FIELD_INTERVAL		120	// フィールドタイマー
#define TRAP_RECOVERY_INTERVAL	(FIELD_INTERVAL / 2)

// フレーム
static const int field_frame_user[10] = { 8, 0, 4, 2, 0, 4, 2, 0, 4, 2 };
extern const int frame_monster[10] = { 0, 0, 2, 2, 0, 2, 2, 0, 0, 2 };
extern const int frame_magic[MAX_SCROLL_TYPE] = { 0, 2, 4, 6, 8, 10, 12, 12, 14 };

// 移動表
extern const point_t move_table[10] = {
  {  0,  0 },
  { -1,  1 }, {  0,  1 }, {  1,  1 },
  { -1,  0 }, {  0,  0 }, {  1,  0 },
  { -1, -1 }, {  0, -1 }, {  1, -1 }
};

// ループ
static void field_loop(void);
static void field_trap_loop(void);

// 描画関連
static void update_background(void);

// 重力関連
static int gravity_clock; // 重力カウンタ
static int field_gravitate_user(void);
static int field_gravitate_monsters(void);

// 逆さツララ
static int field_user_trapped; // ダメージフラグ
static void user_fall_hazard(void);

// 移動関連
static int monster_clock; // モンスター移動カウンタ
static int field_accel; // 加速
static int field_jump; // 跳躍力
static int field_warp_count; // ワープカウンタ
static int field_move_user(int dir);
static int field_move_monsters(void);
static int open_tombs(void);
static int field_warp_next(int point);

// 戦闘関連
static tomb_t *monster_encountered;
static void field_begin_battle(void);
static int field_battle_escape(int dir);

// 進入関連
static void field_enter_where(void);

// マップ関連
static tomb_t *monster_map[FIELD_SIZE];
static int in_user_sight(int point);
static int point_can_move(int point, int dir);
static int point_can_through(int point);
static int point_can_escape(int point);
static int point_no_foothold(int point);
static tomb_t *point_monster(int point);

#define TRAINING_GROUND_LEVEL 10

int init_training_ground(int scenario)
{
  memset(&user, 0, sizeof(user));

  user.environment.scenario = scenario;
  user.environment.dungeon_level = TRAINING_GROUND_LEVEL;
  user.environment.in_training_ground = 1;
  
  user.point = field_offset_XY(5, 10);
  user.frame = battle_frame_user[2];

  user.status.fighter.rank = NULL_RANK;
  user.status.wizard.rank = NULL_RANK;
  
  user.equipment[GOODS_WEAPON] = GOODS_NULL_WEAPON;
  user.equipment[GOODS_SCROLL] = GOODS_NULL_SCROLL;
  user.equipment[GOODS_ARMOUR] = GOODS_NULL_ARMOUR;
  user.equipment[GOODS_SHIELD] = GOODS_NULL_SHIELD;
  user.equipment[GOODS_MAGIC_ITEM] = GOODS_NULL_MAGIC_ITEM;
  
  load_user_unarmed();
  init_level(TRAINING_GROUND_LEVEL, LEVEL_DIR "/xa1");

  // ワープカウンタの初期化
  field_warp_count = 0;

  // BGM
  playBgm(resolveFieldMusic(0, TRAINING_GROUND_LEVEL));

  return CONTEXT_FIELD;
}

int init_level(int level, const char *dir)
{
  int i;
  
  if (load_level(level, dir)) {
    return 1;
  }
  
  // モンスターマップの初期化
  for (i = 0; i < FIELD_SIZE; i++) {
    monster_map[i] = NULL;
  }
  for (i = 0; i < MAX_TOMB; i++) {
    tomb_t *tm = &level_data.tombs[i];
    if (tm->num_members > 0) 
      monster_map[tm->point] = tm;
  }
  
  // デバッグモード
  if (dir == NULL) {
    open_tombs();
  }
  
  field_cave_open();
  
  // 品物データベースを初期化
  init_goods(user.environment.scenario);
  
  return 0;
}

int init_field(void)
{
  // BGM
  if (in_training_ground()) {
    playBgm(resolveFieldMusic(0, TRAINING_GROUND_LEVEL));
  } else {
    playBgm(resolveFieldMusic(user.environment.scenario, user.environment.dungeon_level));
    bgm_tempo(0); // テンポ
  }

  // 戦闘中だった？
  if (in_battle())
    return init_battle(&user.environment.field_room,
                       &user.battle,
                       field_battle_escape);
  else
    return CONTEXT_FIELD;
}

void field_enter(void)
{
  auto &res = Resources::instance();
  auto &mixer = Application::instance().getMixer();
  for (SoundId id : kFieldSoundIds) {
    res.loadSound(mixer, id);
  }

  // 変数の初期化
  gravity_clock = GRAVITY_RATE;
  monster_encountered = NULL;
  field_accel = 0;
  field_jump = 0;
  field_user_trapped = 0;
  
  user.environment.in_tower  = 0;
  user.environment.in_battle = 0;

  if (in_training_ground()) {
    user.x = 160;
    user.y = 280;
  } else {
    user.x = 160;
    user.y = 160;
  }

  // 背景フレーム(枠)。ボス撃破後はboss.cppのrestore_context()が再描画するが、
  // フィールドへの最初の入場時にも描いておく必要がある
  load_background(ImageId::user_frame);

  // BGM
  if (in_training_ground()) {
    playBgm(resolveFieldMusic(0, TRAINING_GROUND_LEVEL));
  } else {
    playBgm(resolveFieldMusic(user.environment.scenario, user.environment.dungeon_level));
    bgm_tempo(0); // テンポ
  }

  // おっと？
  if ((monster_encountered = point_monster(user.point)) != NULL &&
      !in_training_ground()) {
    field_begin_battle();
  } else {
    // 画面の更新
    update_background();
    status_refresh(0);

    // メインループの開始
    set_timer(FIELD_INTERVAL, field_loop);
  }
}

void field_leave(void)
{
  auto &res = Resources::instance();
  for (SoundId id : kFieldSoundIds) {
    res.unloadSound(id);
  }
  Resources::instance().unloadImage(ImageId::user_frame);
  kill_timer();
}

// メインループ
void field_loop(void)
{
  int key1, key2, key3, key4, key6, key7, key8, key9;
  int update = 0;

  // 時間の経過
  if (!in_training_ground()) {
    user_time_elapse(FIELD_INTERVAL);
  }

  // 死んだ？
  if (user.status.HP < 0) {
    extend_context(init_user_dead(user.x, user.y, update_background));
    return;
  }

  if (isCtrlDown()) {
    // Ctrl+Q: 保存
    if (isKeyDown(SDL_SCANCODE_Q)) {
      save_user();
      switch_context(CONTEXT_START_MENU);
      return;
    }
    // Ctrl+S: サウンド
    if (isKeyDown(SDL_SCANCODE_S)) {
      if (bgm_mute()) {
        emit_message("Sound Off");
      } else {
        emit_message("Sound On");
      }
      extend_context(init_pause(100, SDL_SCANCODE_S));
      return;
    }
  } else {
    // SPACE: 建物・洞窟に入る
    if (isKeyDown(SDL_SCANCODE_SPACE)) {
      field_enter_where();
      return;
    }
    // ENTER: アイテム使用
    if (isReturnDown() && !in_training_ground() &&
        user.equipment[GOODS_MAGIC_ITEM] < MAX_GOODS) {
      extend_context(init_use_item(update_background, NULL));
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
    if (isKeyDown(SDL_SCANCODE_E) && !in_training_ground()) {
      extend_context(init_equip());
      return;
    }
  }

  // 移動
  key2 = isKeyDown(SDL_SCANCODE_DOWN);
  key4 = isKeyDown(SDL_SCANCODE_LEFT);
  key6 = isKeyDown(SDL_SCANCODE_RIGHT);
  key8 = isKeyDown(SDL_SCANCODE_UP);

  if (key2 && key4) {
    key2 = key4 = 0; key1 = 1;
  } else {
    key1 = isKeyDown(SDL_SCANCODE_END);
  }
  if (key2 && key6) {
    key2 = key6 = 0; key3 = 1;
  } else {
    key3 = isKeyDown(SDL_SCANCODE_PAGEDOWN);
  }
  if (key8 && key4) {
    key8 = key4 = 0; key7 = 1;
  } else {
    key7 = isKeyDown(SDL_SCANCODE_HOME);
  }
  if (key8 && key6) {
    key8 = key6 = 0; key9 = 1;
  } else {
    key9 = isKeyDown(SDL_SCANCODE_PAGEUP);
  }

       if (key1) update = field_move_user(1);
  else if (key3) update = field_move_user(3);
  else if (key4) update = field_move_user(4);
  else if (key6) update = field_move_user(6);
  else if (key2) update = field_move_user(2);
  else if (key8) update = field_move_user(8);
  else if (user.dir == 7) {
         if (key9) update = field_move_user(9);
    else if (key7) update = field_move_user(7);
    else field_accel = field_jump = 0; // 加速度・跳躍力をクリア
  } else {
         if (key7) update = field_move_user(7);
    else if (key9) update = field_move_user(9);
    else field_accel = field_jump = 0; // 加速度・跳躍力をクリア
  }
    
  // ループを抜ける？
  if (update < 0)
    return;

  // ユーザーはモンスターに遭遇した？
  if (monster_encountered != NULL && !in_training_ground())
    goto do_battle;

  // モンスターを出撃させる
  open_tombs();

  if (!using_hourglass()) {
    // モンスターの移動
    update += field_move_monsters();
  
    // モンスターはユーザーに接触した？
    if (monster_encountered != NULL && !in_training_ground()) {
    do_battle:
      field_begin_battle();
      return;
    }
  }

  // 重力を発生させる
  if (--gravity_clock < 0) {
    gravity_clock = GRAVITY_RATE;
    update += field_gravitate_user();
  }

  // 更新する？
  if (update)
    update_background();

  if (field_user_trapped) {
    set_timer(TRAP_RECOVERY_INTERVAL, field_trap_loop);
    field_trap_loop();
  }
}

void field_trap_loop(void)
{
  // ダメージ個所を反転: ここは必ず二回通る
  inverse_image(clip_main, user.x, user.y, mask_damaged);
  update_region(rect_main.x + user.x, rect_main.y + user.y, 40, 40);

  if (field_user_trapped) {
    field_user_trapped = 0;
  } else {
    status_update_HP(SDL_::Color::WHITE);
    set_timer(FIELD_INTERVAL, field_loop); // 復帰
  }
}

void update_background(void)
{
  map_t *map = level_data.field + user.point + user_sight_XY();
  map_t *top = level_data.field;
  map_t *end = level_data.field + FIELD_SIZE;
  tomb_t **mo_top = monster_map;
  tomb_t **mo_end = monster_map + FIELD_SIZE;
  int i, j;

  for (i = 0; i < 360; i += 40) {
    for (j = 0; j < 360; j += 40) {
      // フィールドの範囲外ならレンガで埋め尽くす
      int index = (top <= map && map < end) ? *map : tile_data.bricks;
      draw_image(clip_main, j, i, frame_tiles[index]);
      map++;
    }
    map += FIELD_WIDTH - 9;
  }

  // ユーザーの描画
  if (!user_hidden) {
    draw_sprite(clip_main, user.x, user.y, frame_user[user.frame]);
  }

  mo_top += user.point + user_sight_XY();
  
  // モンスターの描画
  for (i = 0; i < 360; i += 40) {
    for (j = 0; j < 360; j += 40) {
      if (monster_map <= mo_top && mo_top < mo_end && *mo_top != NULL) {
        tomb_t *mo = *mo_top;
        draw_sprite(clip_main, j, i, frame_monsters[mo->monster_id / 4][mo->frame]);
      }
      mo_top++;
    }
    mo_top += FIELD_WIDTH - 9;
  }
  update(rect_main);
}

// ユーザーの移動
int field_move_user(int dir)
{
  int pos, map;
  int dx, dy;
  int same_dir, no_foothold = 0;

  dx = move_table[dir].x;
  dy = move_table[dir].y;
  same_dir = dir == user.dir;

  if (!same_dir) {
    field_accel = 0;
    user.dir = dir;
  }
  
  // 加速度アップ！
  field_accel = min(MAX_ACCELERATION, field_accel + 1);
  
  if (!using_winged_boots()) {
    // 足場がない？
    if ((map = point_no_foothold(user.point)) != 0) {
      no_foothold = 1;
    
      // scenario 2
      if (map == tile_data.slope_left ||
          map == tile_data.slope_rite) {
        // 制御できない
        return 0; // goto done_move;
      }
    
      // 跳躍力を消費できる？
      switch (field_jump) {
      case 1:
      case 2:
      case 3:
        if (same_dir) {
          dy = 1;
        }
      case 4:
        break;
      default:
        dy = 1;
      }
      field_jump = 0;
    } else {
      // 足場を利用して跳躍力をためる
      field_jump = field_accel;
    }
  }

retry:
  // 移動先の位置
  pos = user.point + field_offset_XY(dx, dy);

  // アウトオブバウンズ？
  if (pos < 0 || FIELD_SIZE <= pos) {
    goto done_move;
  }

  // モンスターがいる？
  if ((monster_encountered = point_monster(pos)) != NULL) {
    if (dx == 0 || dy == 0)
      return 0; // 遭遇
#if 0
    if (no_foothold && dy == 1 && (dir == 4 || dir == 6)) {
      user.point += FIELD_WIDTH;
      return 0;
    }
#endif
    monster_encountered = NULL;
    goto done_move;
  }

  // 現在の地形が指定した方向への移動を許す？
  if (!using_mantle() && !point_can_move(user.point, dir)) {
    goto done_move;
  }

  // 移動先の地形が侵入を許さない？
  if (!point_can_through(pos)) {
    map = point_map(pos);

    // 外套が無効？
    if (!using_mantle() || (tile_data.flags[map] & TILE_WALL_MARBLE) != 0) {
      
      // 扉？
      if (map == tile_data.locked && user_use_key()) {
      
        level_data.field[pos] = pos % 2 != 0
          ? tile_data.pattern0
          : tile_data.pattern1;
      
        playSound(SoundId::lost_key); // SE
        emit_message("Lost key");
      
        // 扉を開ける
        extend_context(init_animation_tile(tile_data.field_open, 3,
                                           user.x + dx * 40,
                                           user.y + dy * 40,
                                           update_background));
        return -1;
      }

      if (no_foothold && dy < 0) {
#if 1
        // スムーズな方法
        dy = 1;
        goto retry;
#else
        // 自由落下を待つ
        gravity_clock = GRAVITY_WAIT;
#endif
      }
      field_jump = 0;
      goto done_move;
    }
  }
  
  // 重力発生タイミングの遅延
  gravity_clock = GRAVITY_WAIT;
  
  // ワープポイント？
  if (point_map(pos) == tile_data.pattern_warp) {
    user.point = field_warp_next(pos);
  } else {
    user.point = pos;
  }

done_move:
  // scenario 2
  if (!using_winged_boots()) {
    map = point_map(user.point + FIELD_WIDTH);
    if (map == tile_data.icicle_hazard &&
        point_map(user.point) != tile_data.ladder) {
      // 逆さツララ
      user_fall_hazard();
    }
  }
  
  user.frame = field_frame_user[user.dir] + (user.frame + 1) % 2;
  return 1;
}

int field_gravitate_user(void)
{
  int pos = user.point + FIELD_WIDTH;
  int map = point_map(pos);
  int update = 0;

  // ユーザーは宙に浮いている？
  if (using_winged_boots())
    return 0;
  
  // 足場がない？
  if (point_no_foothold(user.point)) {

    // ワープポイント？
    if (map == tile_data.pattern_warp) {
      pos = field_warp_next(pos);
    }
    // 斜面(左下がり)？    
    else if (map == tile_data.slope_left) {
      pos -= 1;
      if (!point_can_through(pos))
        return 0;
      
      // 左下向き
      user.frame = field_frame_user[1] + (user.frame + 1) % 2;
      field_accel = field_jump = MAX_ACCELERATION; // 加速
    }
    // 斜面(右下がり)？
    else if (map == tile_data.slope_rite) {
      pos += 1;
      if (!point_can_through(pos))
        return 0;

      // 右下向き
      user.frame = field_frame_user[3] + (user.frame + 1) % 2;
      field_accel = field_jump = MAX_ACCELERATION; // 加速
    }

    // 落下
    user.point = pos;
    update = 1;
  
    // 着地した？
    if (!point_no_foothold(user.point)) {
      field_jump = field_accel;
    }
    
    map = point_map(pos + FIELD_WIDTH);
  }
  
  if (point_map(user.point) != tile_data.ladder &&
      map == tile_data.icicle_hazard) {
    // 逆さツララ
    user_fall_hazard();
  }
  return update;
}

// 次のワープ位置を返す
int field_warp_next(int point)
{
  int p;

  if (in_training_ground()) {
    // 訓練場で 10 回ワープした？
    if (field_warp_count < 10)
      field_warp_count++;
    else
      return point;
  }

  for (p = point + 1; p < FIELD_SIZE; p++) {
    if (level_data.field[p] == tile_data.pattern_warp)
      return p;
  }
  for (p = 0; p < point; p++) {
    if (level_data.field[p] == tile_data.pattern_warp)
      return p;
  }
  return point;
}

void user_fall_hazard(void)
{
  int i, damage;

  // ダメージの計算
  damage = 1;
  for (i = 0; i < MAX_GOODS; i++) {
    damage += user.inventory[GOODS_WEAPON][i].stock;
    damage += user.inventory[GOODS_SCROLL][i].stock;
    damage += user.inventory[GOODS_ARMOUR][i].stock;
    damage += user.inventory[GOODS_SHIELD][i].stock;
    damage += user.inventory[GOODS_MAGIC_ITEM][i].stock;
  }
  damage *= 100;
  user.status.HP -= damage;
    
  format_message("DMG-%d", damage);
  status_update_HP(SDL_::Color::RED);
  
  field_user_trapped = 1;
  playSound(SoundId::trapped);
}

int open_tombs(void)
{
  tomb_t *top, *end;
  int n = 0;

  for (top = level_data.tombs, end = top + MAX_TOMB; top < end; top++) { 
    // 退治されてしまったが、基地にはまだ次の世代がいる？
    if (top->num_members == 0 && top->monster_id >= 0) {

      // そこは他のモンスターがおらず、ユーザーに見えない？
      if (monster_map[top->point_tomb] == NULL &&
          !in_user_sight(top->point_tomb)) {
        
        monster_status_t *mo = &monster_data[top->monster_id];
        int group_min = mo->group_min;
        int group_rnd = mo->group_max - mo->group_min;
        int num_members;

        // 新しいグループを組織する
        num_members = max(1,
                          group_rnd > 0
                          ? group_min + random_integer(group_rnd) : group_min);
        
        // 新しいモンスターを出現させる
        top->point       = top->point_tomb;
        top->num_members = num_members;
        top->frame       = 0;
        top->dir         = 0;
        top->AGL         = mo->AGL;
        top->activity    = mo->activity;

        monster_map[top->point] = top;
        n++;
      }
    }
  }
  return n;
}

int field_move_monsters(void)
{
  tomb_t *top, *end;
  int n = 0;

  // 移動クロックの更新
  monster_clock = (monster_clock + 1) % 2;

  for (top = level_data.tombs, end = top + MAX_TOMB; top < end; top++) {

    // まだ退治されていない構成員がいる？
    if (top->num_members > 0) {
      int dx, dy;
      int old;
      int pos;
      
      // ユーザーより遅いと移動速度は半分
      if (top->AGL < user_AGL() && monster_clock == 0)
        continue;
        
      // 自分の場所をいったんクリア
      monster_map[top->point] = NULL;

      // 位置
      old = top->point;
      pos = top->point;

      dx = move_table[top->dir].x;
      dy = move_table[top->dir].y;

      // モンスターは動かない？
      if ((top->activity & ACTIVITY_WALKER) == 0)
        goto done_move;

      // 重力の影響を受けない？
      if ((top->activity & ACTIVITY_FLIGHT) != 0) {
        if (dx == 0 && dy == 0)
          goto no_move; // 無意味なホバーリングは避けたい
        
        // とりあえずこの方向に移動してみる
        pos = top->point + field_offset_XY(dx, dy);
        if (!point_can_through(pos))
          goto no_move;
        
      } else {
        // 空を飛べるもの以外は足場なしでは移動できない
        if (point_no_foothold(pos))
          goto done_move;

        switch (top->dir) {
        case 0:
        case 5: goto no_move;

          // 下および斜め下
        case 2:
          if ((top->activity & ACTIVITY_LADDER) == 0) {
            top->dir = 6;
            break;
          }
        case 1:
        case 3:
          pos = top->point + field_offset_XY(dx,  1);
          // 足場がない場所には降りない
          if (!point_can_through(pos) || point_no_foothold(pos))
            top->dir += 3;
          break;

          // 上および斜め上
        case 8:
          if ((top->activity & ACTIVITY_LADDER) == 0) {
            top->dir = 4;
            break;
          }
        case 7:
        case 9:
          pos = top->point + field_offset_XY(dx, -1);
          // 足場があるなら上に行ける
          if (!point_can_through(pos) || point_no_foothold(pos))
            top->dir -= 3;
          break;
        }
        pos = top->point + field_offset_XY(dx, move_table[top->dir].y);
        if (!point_can_through(pos) || point_no_foothold(pos))
          goto no_move;
      }
      // 移動
      top->point = pos;
      goto done_move;

    no_move:
      // しかしそれはユーザー？(上下左右の移動のみ)
      if (top->dir % 2 == 0 && pos == user.point) {
        monster_encountered = top;
      }
      // この位置で足踏み
      pos = old;

      // 方向転換に 1 ターン消費する
      top->dir = random_direction();

    done_move:
      if ((top->activity & ACTIVITY_VIVID) != 0) {
        top->frame++; // フレーム更新
      }
      top->frame = frame_monster[top->dir] + top->frame % 2;

      // ユーザーが見ていた？
      if (in_user_sight(old) || in_user_sight(pos))
        n++;

      // 新しい場所に自分自身を置く
      monster_map[pos] = top;
    }
  }

  // ユーザーに見られたモンスターの数を返す
  return n + field_gravitate_monsters();
}

int field_gravitate_monsters(void)
{
  tomb_t *top, *end;
  int n = 0;
  
  for (top = level_data.tombs, end = top + MAX_TOMB; top < end; top++) {
    // 重力の影響を受ける？
    if (top->num_members > 0 && (top->activity & ACTIVITY_FLIGHT) == 0) {
      int old = top->point;
      int pos = top->point;
      int map = point_no_foothold(top->point);

      // 足場がない
      if (map != 0) {
        // 斜面(左下がり)
        if (map == tile_data.slope_left) {
          pos += field_offset_XY(-1, 1);
          if (!point_can_through(pos))
            continue;
        }
        // 斜面(右下がり)
        else if (map == tile_data.slope_rite) {
          pos += field_offset_XY(+1, 1);
          if (!point_can_through(pos))
            continue;
        }
        // 自由落下
        else {
          pos += field_offset_XY( 0, 1);
        }
        monster_map[old] = NULL;
        monster_map[pos] = top;
        top->point = pos;

        // ユーザーからそれが見える？
        if (in_user_sight(old) || in_user_sight(top->point))
          n++;
      }
    }
  }
  return n;
}

// 戦闘を開始する
void field_begin_battle()
{
  // モンスターの戦闘時フォーメーション
  static const point_t monster_formation[MAX_MEMBER] = {
    {  0,  0 }, { -1,  0 }, {  1,  0 },
    {  0, -1 }, {  0,  1 }, { -1,  1 },
    {  1, -1 }, { -1, -1 }, {  1,  1 }
  };
  int i, diff;
  room_t *room = &user.environment.field_room;

  // 遭遇音
  playSound(SoundId::encount);
  tomb_t *ma = monster_encountered;

  // 遭遇したモンスターの出現位置番号を控えておく
  user.environment.field_encountered = ma - level_data.tombs;

  // 遭遇したモンスターの情報
  room->monster_id = ma->monster_id;

  // 各モンスターの初期位置を決める
  for (i = 0; i < ma->num_members; i++) {
    room->members[i].type = MEMBER_MONSTER;
    room->members[i].x = (monster_formation[i].x + 4) * 40;
    room->members[i].y = (monster_formation[i].y + 4) * 40;
  }

  // 残りのエントリは使わない
  for (; i < MAX_MEMBER; i++)
    room->members[i].type = MEMBER_UNUSED;
  
  // 戦場の情報
  room->barrier[0] = !point_can_escape(ma->point + FIELD_WIDTH);
  room->barrier[1] = !point_can_escape(ma->point - 1);
  room->barrier[2] = !point_can_escape(ma->point + 1);
  room->barrier[3] = !point_can_escape(ma->point - FIELD_WIDTH);
  
  // ユーザーの初期位置を推測する
  diff = user.point - ma->point;
#if 0
       if (diff >   1) { user.x = 160; user.y = 320; }
  else if (diff ==  1) { user.x = 320; user.y = 160; }
  else if (diff <  -1) { user.x = 160; user.y =   0; }
  else if (diff == -1) { user.x =   0; user.y = 160; }
  else                 { user.x = 160; user.y =   0; } // 上
#else
       if (diff >   1) { user.x = 160; user.y = 320; user.dir = 8; }
  else if (diff ==  1) { user.x = 320; user.y = 160; user.dir = 4; }
  else if (diff <  -1) { user.x = 160; user.y =   0; user.dir = 2; }
  else if (diff == -1) { user.x =   0; user.y = 160; user.dir = 6; }
  else                 { user.x = 160; user.y =   0; user.dir = 2; } // 上
  
  {
    user.frame = battle_frame_user[user.dir];
  }
#endif
  
  switch_context(init_battle(room, NULL, field_battle_escape));
}

int field_battle_escape(int dir)
{
  int i, p;
  tomb_t *ma;

  if (user.environment.field_encountered >= 0 &&
      user.environment.field_encountered < MAX_TOMB) {
    ma = &level_data.tombs[user.environment.field_encountered];
  } else {
    // 致命的エラー: 脱出できない
    emit_error("Can't leave there!");
    return 0;
  }
  
  // ユーザーの新しい位置を決める
  p = ma->point;
  switch (dir) {
  case 2: p += FIELD_WIDTH; break;
  case 4: p -= 1; break;
  case 6: p += 1; break;
  case 8: p -= FIELD_WIDTH; break;
  }

  // フィールドの上下の境界を越えない？
  if (0 <= p && p < FIELD_SIZE) {
    int num_members = 0;
  
    // 生き残ったモンスターの数を調べ、結果を反映する
    for (i = 0; i < MAX_MEMBER; i++) {
      if (member_monster(&user.environment.field_room.members[i]))
        num_members++;
    }
    ma->num_members = num_members;
    
    // 退治された？
    if (num_members <= 0) {
      ma->monster_id++; // 次の世代
      if (ma->monster_id % MAX_VARIETY == 0) {
        ma->monster_id = -1; // 打ち止め
      }
      monster_map[ma->point] = NULL;
    }

    // 新しい場所にはモンスターがいる？
    if ((monster_encountered = point_monster(p)) != NULL) {
      user.point = ma->point;
      field_begin_battle();
    } else {
      user.point = p;
      switch_context(CONTEXT_FIELD);
    }
    return 1;
  }
  return 0; // 脱出できない！
}

// 指定された位置はユーザーから見える？
int in_user_sight(int p)
{
  int top = user.point + user_sight_XY();
  if (top <= p) {
    int x = (p - top) % FIELD_WIDTH;
    int y = (p - top) / FIELD_WIDTH;
    return x < 9 && y < 9;
  }
  return 0;
}

// 指定された位置のマップの地形を返す
int point_map(int p)
{
  if (0 <= p && p < FIELD_SIZE)
    return level_data.field[p];
  else
    return tile_data.bricks;
}

// 指定された位置「から」、指定された方向「に」移動可能？
int point_can_move(int p, int d)
{
  return point_map(p) != tile_data.bridge || move_table[d].y >= 0;
}

// 指定された位置「に」通過可能？
int point_can_through(int p)
{
  if (0 <= p && p < FIELD_SIZE) {
    int map = level_data.field[p];

    // ユーザーまたはモンスターを考慮に入れる
    return (tile_data.flags[map] & TILE_WALL) == 0 &&
      p != user.point && monster_map[p] == NULL;
  }
  return 0;
}

// 指定された位置「に」、指定された方向「で」脱出可能？
int point_can_escape(int p)
{
  if (0 <= p && p < FIELD_SIZE) {
    int map = level_data.field[p];
    return (tile_data.flags[map] & TILE_WALL) == 0;
  }
  return 0;
}

// 指定された位置は足場がない？
// -1: 足場がない、[斜面の地形]: 足場がない
int point_no_foothold(int pos)
{
  if (0 <= pos && pos < FIELD_SIZE - FIELD_WIDTH) {
    int map;
    
    // その位置の地形は「はしご」？
    if (level_data.field[pos] == tile_data.ladder)
      return 0;

    // 直下の位置
    pos += FIELD_WIDTH;
    map = level_data.field[pos];

    // 斜面？
    if (map == tile_data.slope_left || map == tile_data.slope_rite)
      return map;

    // そこには足場となる地形がなく、ユーザー・モンスターがいない？
    if ((tile_data.flags[map] & FOOTHOLD_MASK) == 0u &&
        pos != user.point &&
        monster_map[pos] == NULL)
      return -1;
  }
  // 最下層およびフィールドの範囲外は調べるまでもない
  return 0;
}

// 指定された位置にモンスターがいる？
tomb_t *point_monster(int p)
{
  return 0 <= p && p < FIELD_SIZE ? monster_map[p] : NULL;
}

void field_enter_where(void)
{
  int i;
  int map = point_map(user.point);

  if (map == tile_data.cave_closed) {
    // scenario 2: 最下層に通じる扉？
    if (in_scenario2()) {
      if (user.equipment[GOODS_WEAPON    ] == MAX_GOODS - 1 &&
          user.equipment[GOODS_SCROLL    ] == MAX_GOODS - 1 &&
          user.equipment[GOODS_ARMOUR    ] == MAX_GOODS - 1 &&
          user.equipment[GOODS_SHIELD    ] == MAX_GOODS - 1 &&
          user.equipment[GOODS_MAGIC_ITEM] == MAX_GOODS - 1 &&
          user.status.CRN == 4 &&
          user.status.KRM == 0) {
        extend_context(init_cave(10));
        return;
      }
    }
    emit_message("Enter-Closed");
    return;
  }

  // 次のレベルへ行く？
  if (map == tile_data.cave_next) {
    int to_level = user.environment.dungeon_level + 1;
    
    if (0 <= to_level && to_level < MAX_DUNGEON_LEVEL) {
      // このときユーザーデータの保存
      extend_context(init_cave(to_level));
      return;
    }
    // 訓練場から抜ける？
    else if (in_training_ground()) {
      if (strcmp(user.status.name, "") == 0) {
        // 王様に会っていない
        user.point = field_offset_XY(5, 10);
        user.frame = 0;
      } else {
        user.environment.in_training_ground = 0;
        user.point = field_offset_XY(4,  4);
        user.frame = battle_frame_user[2];
        // scenario 2
        if (in_scenario2()) {
          user.status.ELX = 0;
        }

        extend_context(init_cave(0));
        
        make_user_dir(); // ユーザーディレクトリを作成する
        init_level(0, user_path.empty() ? nullptr : user_path.c_str()); // 読み直し
      }
      return;
    }
  }
  // 前のレベルへ行く？
  if (map == tile_data.cave_back) {
    int to_level = user.environment.dungeon_level - 1;
    if (0 <= to_level && to_level < MAX_DUNGEON_LEVEL) {
      extend_context(init_cave(to_level));
      return;
    }
  }
  // 三階層次のレベルに行く？
  if (map == tile_data.cave_next3) {
    int to_level = user.environment.dungeon_level + 3;
    if (0 <= to_level && to_level < MAX_DUNGEON_LEVEL) {
      extend_context(init_cave(to_level));
      return;
    }
  }
  // 三階層前のレベルに行く？
  if (map == tile_data.cave_back3) {
    int to_level = user.environment.dungeon_level - 3;
    if (0 <= to_level && to_level < MAX_DUNGEON_LEVEL) {
      extend_context(init_cave(to_level));
      return;
    }
  }
  
  // ショップ
  for (i = 0; i < MAX_SHOP; i++) {
    if (level_data.shops[i].point == user.point) {
      extend_context(init_shop(level_data.shops[i].value));
      return;
    }
  }

  // シナリオ1: 最後の砦
  if (map == tile_data.last_tower && !in_scenario2()) {
    if (user.status.CRN != 4 ||
        user.status.KRM != 0) {
      emit_message("Enter-Closed"); // 本当は何て言うの？
      return;
    }
  }

  // タワー
  for (i = 0; i < TOWER_SIZE; i++) {
    if (level_data.tower[i].entrance_point == user.point) {
      int boss_id = level_data.tower[i].boss_id;

      // ボス？
      if (boss_id >= 0) {
        save_user(); // タワーに入る前の状態を保存する
      }

      user.point = i;
      user.x = 160;
      user.y = 320;
      user.dir = 8;
      user.frame = battle_frame_user[8];

      switch_context(CONTEXT_TOWER);
      extend_context(init_pause(50, SDL_SCANCODE_SPACE));
      return;
    }
  }

  emit_message("Enter-Where?");
}

void field_cave_open(void)
{
  // scenario 1
  if (!in_scenario2()) {
    // 必要ランク
    const int require_rank[10] = { 1, 3, 4, 5, 7, 8, 10, 11, 13 };
    int level = user.environment.dungeon_level;
    
    // 規定のランクをクリアした？
    if (0 <= level && level < 10 &&
        require_rank[level] <= user_higher_rank()) {
      int i;
      // 封印されていた洞窟を開く
      for (i = 0; i < FIELD_SIZE; i++) {
        if (level_data.field[i] == tile_data.cave_closed) {
          level_data.field[i] = tile_data.cave_next;
        }
      }
    }
  }
}
