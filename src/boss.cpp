#include "xanadu.h"
#include "boss.h"
#include "battle.h"
#include "animation.h"
#include "resources/Resources.h"
#include "resources/ImageId.h"
#include "resources/SoundId.h"
#include "resources/MusicId.h"
#include "sdl/LegacyPlatform.h"
#include "app/Application.h"
#include <unordered_map>

#define STEP_USER_X		8	// ユーザーの進む速さ(水平方向)
#define STEP_USER_Y		8	// ユーザーの進む速さ(垂直方向)
#define STEP_BOSS_X		4	// ボスの進む速さ(水平方向)
#define STEP_BOSS_Y		8	// ボスの進む速さ(垂直方向)
#define STEP_MAGIC		16	// 魔法の進む速さ

#define BOSS_INTERVAL		80	// インターバル
#define BOSS_LOOP_WAIT		20	// ループウエイト

#define WIDESCREEN_WIDTH	608
#define WIDESCREEN_HEIGHT	240

#define FRAME_BOSS		7	// ボスイメージのフレーム数
#define FRAME_BOSS_DEAD		6	// ボス(やっつけられ図)

// ボスはどちら向き？
#define boss_frame_left(n)	((n) < 3)
#define boss_frame_rite(n)	(!boss_frame_left(n))

// ブレス状態遷移
#define BREATH_NO_BREATH	0	// ブレスはない
#define BREATH_FILLING_UP	1	// 充満している
#define BREATH_WARMING_UP	2	// 準備している
#define BREATH_BREATHING	3	// 吐いている

#define breath_fill_up_interval()	(random_integer(16) + 8)
#define breath_warm_up_interval()	4
#define breath_breathe_interval()	18

// ボスのステータスに関する情報を保持する構造体
typedef struct {
  char		name[16];		// 名前
  int		HP;			// ヒットポイント
  int		STR;			// 攻撃力
  int		DEF;			// 防御力
  int		MGR;			// 魔法防御力
  int		fighter_EXP;		// 武器経験値
  int		wizard_EXP;		// 魔法経験値
} boss_status_t;

// ボスデータを保持する構造体
typedef struct {
  char *	image_filename;		// イメージファイル
  int		can_breathe;		// ブレスを吐く？
  boss_status_t	status;			// ステータス
} boss_data_t;

// 活動中のボスに関する情報を保持する構造体
typedef struct {
  int		x;			// 水平座標
  int		y;			// 垂直座標
  int		frame;			// フレーム
  boss_status_t	status;			// ステータス
} boss_t;

static SDL_::SubImage	frame_boss[FRAME_BOSS];
static SDL_::SubImage	frame_breath[2];	// ブレスイメージ
static boss_t	boss;			// ボス情報
static point_t	*damaged_boss;		// ボスのダメージを受けた場所
static point_t	*damaged_user;		// ユーザーのダメージを受けた場所

static MusicId boss_bgm = MusicId::none;

namespace
{
// 魔法詠唱SE。scroll_typeはSCROLL_NEEDLE(0)..SCROLL_DEATH(8)(goods.h参照)
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

// ボスBGM。シナリオ1は現状ボス専用曲が無く、フィールド/タワーの曲を継続する(MusicId::none)
MusicId resolveBossMusic(int scenario, int bossId)
{
	if (scenario == 0) {
		return MusicId::none;
	}
	static constexpr MusicId kXa2BossMusic[16] = {
		MusicId::xana2dl_xana217, MusicId::xana2dl_xana218, MusicId::xana2dl_xana219,
		MusicId::xana2dl_xana220, MusicId::xana2dl_xana221, MusicId::xana2dl_xana222,
		MusicId::xana2dl_xana223, MusicId::xana2dl_xana224, MusicId::xana2dl_xana225,
		MusicId::xana2dl_xana226, MusicId::xana2dl_xana227, MusicId::xana2dl_xana228,
		MusicId::none, MusicId::none, MusicId::none, MusicId::none,
	};
	if (bossId < 0 || 16 <= bossId) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "resolveBossMusic: boss_id out of range %d\n", bossId);
		return MusicId::none;
	}
	return kXa2BossMusic[bossId];
}

// ボスイメージ。boss_data[boss_id].image_filenameの末尾の1文字("boss_<c>.bmp")で判別する
ImageId resolveBossImageId(bool scenario2, char suffix)
{
	static const std::unordered_map<char, ImageId> kScenario1 = {
		{'0', ImageId::xa1_boss_0}, {'1', ImageId::xa1_boss_1}, {'2', ImageId::xa1_boss_2},
		{'3', ImageId::xa1_boss_3}, {'4', ImageId::xa1_boss_4},
	};
	static const std::unordered_map<char, ImageId> kScenario2 = {
		{'0', ImageId::xa2_boss_0}, {'1', ImageId::xa2_boss_1}, {'2', ImageId::xa2_boss_2},
		{'3', ImageId::xa2_boss_3}, {'4', ImageId::xa2_boss_4}, {'5', ImageId::xa2_boss_5},
		{'6', ImageId::xa2_boss_6}, {'7', ImageId::xa2_boss_7}, {'8', ImageId::xa2_boss_8},
		{'9', ImageId::xa2_boss_9}, {'a', ImageId::xa2_boss_a}, {'b', ImageId::xa2_boss_b},
	};
	const auto &table = scenario2 ? kScenario2 : kScenario1;
	auto it = table.find(suffix);
	if (it == table.end()) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "resolveBossImageId: unknown boss image suffix: %c\n", suffix);
		return scenario2 ? ImageId::xa2_boss_0 : ImageId::xa1_boss_0;
	}
	return it->second;
}
}

// ** これらの定数は field.c/battle.c で定義されている **
extern const point_t move_table[10];
extern const int battle_frame_user[10];
extern const int frame_magic[MAX_SCROLL_TYPE];

// ボスキャラデータベース
static const boss_data_t boss_data1[] = {
  // イメージ		 名前	         耐久力  攻撃力  防御力 魔法抵抗 経験値
  { "boss_0.bmp", 0, { "Kraken Giant",    50000,   7500,   1500,    2500,   250,   150 } },
  { "boss_1.bmp", 0, { "Grell Giant",    100000,  25000,   2500,   12500,   400,   600 } },
  { "boss_2.bmp", 0, { "Karttikeya",     750000, 100000,  20000,   75000,  2000,  2000 } },
  { "boss_3.bmp", 1, { "Silver Dragon", 1000000, 500000, 200000,  387500,  2550,  2550 } },
  { "boss_0.bmp", 0, { "Big Kraken",      20000,    800,    240,     750,    80,    30 } },
  { "boss_4.bmp", 1, { "King Dragon",   5000000, 700000, 500000, 1000000,     0,     0 } }
};

// scenario 2
static const boss_data_t boss_data2[] = {
  // イメージ		 名前	      耐久力  攻撃力  防御力 魔法抵抗 経験値
  { "boss_0.bmp", 0, { "Marivoux",        60000,   2500,    700,    2000,   800,   300 } },
  { "boss_1.bmp", 0, { "Peluton",        150000,   5000,   1500,    3500,   800,   300 } },
  { "boss_2.bmp", 0, { "Great Kraken",   450000,   7500,   2500,    5250,  4500,  1500 } },
  { "boss_3.bmp", 0, { "Zschokke",       600000,  12500,   5000,    8750,  6000,  2000 } },
  { "boss_4.bmp", 0, { "White Dragon",   800000,  25000,  10000,   17500,  8000,  2500 } },
  { "boss_5.bmp", 0, { "Borges",        1000000,  50000,  20000,   35000, 10000,  3000 } },
  { "boss_6.bmp", 1, { "Red Dragon",    1200000,  75000,  30000,   52500, 11000,  4000 } },
  { "boss_7.bmp", 0, { "Guin",          1400000, 100000,  50000,   70000, 12000,  6000 } },
  { "boss_8.bmp", 1, { "Hydra",         1600000, 150000,  75000,  105000, 14000,  7000 } },
  { "boss_9.bmp", 0, { "Buzzati",       1800000, 300000, 100000,  210000, 15000,  3000 } },
  { "boss_a.bmp", 0, { "Boiardo",       2000000, 500000, 200000,  350000, 20000, 10000 } },
  { "boss_b.bmp", 1, { "King Dragon",   5000000, 700000, 500000, 1000000,     0,     0 } }
};

// 描画関連
static void update_background(void);
static int init_boss_data(int boss_id);

// メインループ
static void boss_loop(void);
static int boss_gravitate(void);

// 最終ボス関連
static int boss_final_battle; // 最終ボス？

// ブレス関連
static int breath_state; // ブレス状態
static int breath_timer; // ブレス時間
static int boss_breathe(void);

// 勝利・敗北ループ
static int boss_loop_counter; // 勝利ループカウンタ
static int boss_user_ascend; // ユーザー昇天
static void boss_win_loop(void);
static void boss_loose_loop(void);

// 移動関連
static int user_jump;
static int boss_move_user(int dir);
static int boss_move_boss(void);
static int boss_hit_test(int user_x, int user_y, int boss_x, int boss_y);

// 攻撃関連
static void boss_attack_boss(void);
static void boss_attack_user(void);

// 魔法関連
static magic_t user_magic;
static int magic_attacked; // 魔法がボスを攻撃
static void boss_cast_spell(int scroll_id, int INT, int x, int y, int dir);
static int boss_move_magic(void);
static void magic_attack_boss(void);
static void magic_damage_effect(void);
// static void boss_move_user_magic(int dir);

// 回復関連
static void boss_healing(void);

// ステータス
static void update_user_HP(SDL_::Color);
static void update_boss_HP(SDL_::Color);

// 以前のコンテキストを復元
static int old_user_dir;
static int old_user_x;
static int old_user_y;
static int old_user_frame;
static int old_user_STR;
static void restore_context(int won);

int init_boss(int boss_id)
{
  // ユーザー情報をバックアップ
  old_user_dir = user.dir;
  old_user_x = user.x;
  old_user_y = user.y;
  old_user_frame = user.frame;
  old_user_STR = user.status.STR;

  // ボスデータの初期化
  if (init_boss_data(boss_id) != 0) {
    // 致命的エラー
    emit_error("Can't load boss stage!");
    return 0;
  }
  
  if (boss_final_battle) {
    // 最終ボスなのに最終兵器ではなかったりカルマがある？
    if (user.equipment[GOODS_WEAPON] != MAX_GOODS - 1 ||
        user.status.KRM != 0) {
      user.status.STR = 0;
    }
    // scenario 2: 特殊なアイテムを所持していない？
    if (in_scenario2() && user.equipment[GOODS_MAGIC_ITEM] != MAX_GOODS - 1) {
      user.status.STR = 0;
      boss.status.HP = 0;
    }
  }

  // 背景を読み込む
  load_background(ImageId::user_boss_st);
  
  // ボスの位置を画面の右端にセット
  boss.x = rect_shrine.width  - SQUARE_BOSS - 40 * 2;
  boss.y = rect_shrine.height - SQUARE_BOSS;
  boss.frame = 0;
  
  // ユーザーを画面の左端にセット
  user.x = 0;
  user.y = rect_shrine.height - 40;
  user.dir = 2;
  user.frame = battle_frame_user[user.dir];

  // ユーザーの魔法をリセット
  user_magic.lifetime = -1;
  
  // 変数の初期化
  damaged_boss = NULL;
  damaged_user = NULL;
  magic_attacked = 0;
  user_jump  = 0;
  boss_loop_counter = BOSS_LOOP_WAIT;
  boss_user_ascend = WIDESCREEN_HEIGHT - 40;

  // 名前
  draw_text(clip_user_guage, 0, 0, user.status.name, SDL_::Color::RED);
  draw_text(clip_boss_guage, 0, 0, boss.status.name, SDL_::Color::RED);

  update(rect_overall);
  return CONTEXT_BOSS;
}

int init_boss_data(int boss_id)
{
  static bool hasCurrentBoss = false;
  static ImageId currentBossId;
  const boss_data_t *boss_data;
  int n_bosses, i;
  bool scenario2 = in_scenario2();

  if (!scenario2) {
    boss_data = boss_data1;
    n_bosses = sizeof(boss_data1)/sizeof(boss_data1[0]);
  } else {
    boss_data = boss_data2;
    n_bosses = sizeof(boss_data2)/sizeof(boss_data2[0]);
  }

  if (boss_id < 0 || n_bosses <= boss_id) {
    return 1;
  }
  // BGM の設定
  boss_bgm = resolveBossMusic(user.environment.scenario, boss_id);

  // データベースの最後のエントリ？
  boss_final_battle = boss_id == n_bosses - 1;

  // ボスデータをセット
  boss.status = boss_data[boss_id].status;

  // ボスイメージ: image_filenameは常に"boss_<c>.bmp"の形をしている
  {
    const char *fname = boss_data[boss_id].image_filename;
    char suffix = fname[strlen(fname) - 5]; // "boss_X.bmp" -> X
    ImageId newBossId = resolveBossImageId(scenario2, suffix);
    if (hasCurrentBoss && currentBossId != newBossId) {
      Resources::instance().unloadImage(currentBossId);
    }
    Resources::instance().loadImage(newBossId);
    currentBossId = newBossId;
    hasCurrentBoss = true;
    auto boss_base = Resources::instance().getImage(newBossId);
    for (i = 0; i < FRAME_BOSS; i++) {
      frame_boss[i] = SDL_::SubImage{boss_base, Rect(i * 120, 0, 120, 120)};
    }
  }

  // ブレス状態
  breath_state = BREATH_NO_BREATH;

  // ブレスを吐く？
  if (boss_data[boss_id].can_breathe) {
    Resources::instance().loadImage(ImageId::user_breath);
    auto breath_base = Resources::instance().getImage(ImageId::user_breath);
    if (breath_base) {
      breath_state = BREATH_FILLING_UP;
      breath_timer = breath_fill_up_interval();
    }
    for (i = 0; i < 2; i++) {
      frame_breath[i] = SDL_::SubImage{breath_base, Rect(i * 80, 0, 80, 80)};
    }
  }

  // 神殿イメージの読み込み
  {
    ImageId shrineId = scenario2 ? ImageId::xa2_shrine : ImageId::xa1_shrine;
    Resources::instance().loadImage(shrineId);
    visual_image = Resources::instance().getImage(shrineId);
  }

  return 0;
}

void restore_context(int won)
{
  // 最終ボスに勝ったとき以外は表示しない
  if (!boss_final_battle || !won) {
    // バックグラウンドイメージを復元
    load_background(ImageId::user_frame);
    update(rect_shrine);
  }
  
  if (won) {
    // ユーザー情報を復元する
    user.dir = old_user_dir;
    user.x = old_user_x;
    user.y = old_user_y;
    user.frame = old_user_frame;
    user.status.STR = old_user_STR;

    // 経験値を加算
    user.status.fighter.EXP += boss.status.fighter_EXP;
    user.status.wizard.EXP += boss.status.wizard_EXP;

    // 最終ボス？
    if (boss_final_battle) {
      // エンディング
      switch_context(CONTEXT_ENDING);
    } else {
      // コンテキストの復元
      resume_context();
    }
  } else {
    // リスタート
    reset_context();
  }
}

namespace
{
constexpr SoundId kBossSoundIds[] = {
	SoundId::boss_hit, SoundId::dead, SoundId::invoke,
	SoundId::c_needle, SoundId::c_mittar, SoundId::c_deluge, SoundId::c_fire,
	SoundId::c_thunder, SoundId::c_poison, SoundId::c_corros, SoundId::c_tilte,
	SoundId::c_death,
};
}

void boss_enter(void)
{
  auto &res = Resources::instance();
  auto &mixer = Application::instance().getMixer();
  for (SoundId id : kBossSoundIds) {
    res.loadSound(mixer, id);
  }

  // 背景を描画
  update_background();

  // ヒットポイント
  update_user_HP(SDL_::Color::WHITE);
  update_boss_HP(SDL_::Color::WHITE);

  // BGM: シナリオ1では専用曲が無くフィールド/タワーの曲を継続する
  if (boss_bgm != MusicId::none) {
    playBgm(boss_bgm);
  }

  set_timer(BOSS_INTERVAL, boss_loop);
}

void boss_leave(void)
{
  auto &res = Resources::instance();
  for (SoundId id : kBossSoundIds) {
    res.unloadSound(id);
  }
  kill_timer();
}

void update_background(void)
{
  if (in_darkness() || !visual_image) {
    fill_image(clip_shrine, 0, 0, clip_shrine->getWidth(), clip_shrine->getHeight(), SDL_::Color::BLACK);
  } else {
    draw_image(clip_shrine, 0, 0, visual_image);
  }
  
  // ユーザーの描画
  if (user.status.HP >= 0) {
    draw_sprite(clip_shrine, user.x, user.y, frame_user[user.frame]);
  } else {
    draw_sprite(clip_shrine, user.x, user.y, frame_specials[SPECIAL_GRAVE]);
  }

  if (user.status.HP >= 0) {
    // ブレス中？
    if (breath_state == BREATH_BREATHING && breath_timer % 4 < 2) {
      int y = boss.y + SQUARE_BOSS - SQUARE_BREATH;
      int x = boss.x;
      int frame;
      if (boss_frame_left(boss.frame)) {
        x -= SQUARE_BREATH;
        frame = 0;
      } else {
        x += SQUARE_BOSS;
        frame = 1;
      }
      draw_sprite(clip_shrine, x, y, frame_breath[frame]);
    }
  }
  
  // ボスの描画
  draw_sprite(clip_shrine, boss.x, boss.y, frame_boss[boss.frame]);
  
  // 魔法の描画
  if (user_magic.lifetime >= 0) {
    draw_sprite(clip_shrine, user_magic.x, user_magic.y,
                frame_magics[user_magic.frame]);
  }
  
  // ユーザーのダメージの個所を反転
  if (damaged_user) {
    point_t *p = damaged_user;
    inverse_image(clip_shrine, p->x, p->y, mask_damaged);
  }

  // ボスのダメージの個所を反転
  if (damaged_boss) {
    point_t *p = damaged_boss;
    inverse_image(clip_shrine, p->x, p->y, mask_damaged);
  }

  // 魔法による効果
  if (magic_attacked) {
    magic_damage_effect();
  }
  
  update(rect_shrine);
}

// メインループ
void boss_loop(void)
{
  int update = 0;

  // ボスを攻撃した？
  if (damaged_boss) {
    damaged_boss = NULL;
    update |= 1;
    update_boss_HP(SDL_::Color::WHITE);
  }
  else if (magic_attacked) {
    magic_attacked = 0;
    update |= 1;
    update_boss_HP(SDL_::Color::WHITE);
  }
  else {
    int user_update = 0;
    int key2 = isKeyDown(SDL_SCANCODE_DOWN);
    int key4 = isKeyDown(SDL_SCANCODE_LEFT);
    int key6 = isKeyDown(SDL_SCANCODE_RIGHT);
    int key8 = isKeyDown(SDL_SCANCODE_UP);
    int key7 = (key8 && key4) || isKeyDown(SDL_SCANCODE_HOME);
    int key9 = (key8 && key6) || isKeyDown(SDL_SCANCODE_PAGEUP);

    if (isKeyDown(SDL_SCANCODE_SPACE)) {
      // 跳躍力を無効に
      user_jump = -1;

      if (user_magic.lifetime < 0) {
        boss_cast_spell(user.equipment[GOODS_SCROLL],
                        user_INT(), user.x, user.y, user.dir);
        update = 1;
      }
    }
    else if (isReturnDown()) {
      user_jump = -1;
      boss_healing();
    }
    else if (isCtrlDown()) {
      if (isKeyDown(SDL_SCANCODE_Q)) {
        user.status.HP = -1;
        set_timer_proc(boss_loose_loop);
        return;
      }
      else if (key4) user_magic.dir = 4;
      else if (key6) user_magic.dir = 6;
      else if (key2) user_magic.dir = 2;
      else if (key8) user_magic.dir = 8;
    }
    else if (key9) user_update = boss_move_user(9);
    else if (key7) user_update = boss_move_user(7);
    else if (key2) user_update = boss_move_user(2);
    else if (key4) user_update = boss_move_user(4);
    else if (key6) user_update = boss_move_user(6);
    else if (key8) user_update = boss_move_user(8);
    else if (isKeyDown(SDL_SCANCODE_END)) user_update = boss_move_user(1);
    else if (isKeyDown(SDL_SCANCODE_PAGEDOWN)) user_update = boss_move_user(3);
    else {
      user_jump = -1;
    }
    update |= user_update;
    
    if (damaged_boss)
      goto done;
  }

  // 前のターンでユーザーを攻撃した？
  if (damaged_user != NULL) {
    damaged_user = NULL;
    update |= 1;
    update_user_HP(SDL_::Color::WHITE);
  }
  // ブレス準備中でない？
  else if (breath_state != BREATH_WARMING_UP) {
    update |= boss_move_boss();
    if (damaged_user != NULL) {
      goto done;
    }
  }

  update |= boss_move_magic();
  
done:  
  // 重力
  update |= boss_gravitate();

  // ブレス
  update |= boss_breathe();

  // 更新する？
  if (update) {
    update_background();
  }

  if (user.status.HP <= 0) {
    // ユーザー死亡
    set_timer_proc(boss_loose_loop);
  }
  else if (boss.status.HP < 0) {
    // ボス死亡
    set_timer_proc(boss_win_loop);
  }
}

int boss_gravitate(void)
{
  int n = 0;
  int y;
  
  if (user.y < WIDESCREEN_HEIGHT - 40 && user_jump < 0) {

    y = user.y + STEP_USER_Y;

    // 当たってる？
    if (boss_hit_test(user.x, y, boss.x, boss.y)) {
      user_jump = 0;
    } else {
      user.y = y;
    }
    n++;
  }
#if 0 // 重力に逆らう
  if (boss.y < WIDESCREEN_HEIGHT - 120) {
    y = boss.y + STEP_BOSS_Y;
      
    // 当たってる？
    if (boss_hit_test(user.x, user.y, boss.x, y)) {
    } else
      boss.y = y;
    n++;
  }
#endif
  return n;
}

int boss_move_user(int dir)
{
  int dx, dy;
  int x, y;

  if (!isShiftDown()) {
    user.dir = dir; // 方向を変える
  }
  dx = move_table[dir].x;
  dy = move_table[dir].y;

  // 宙に浮いている？
  if (user.y + 40 <  WIDESCREEN_HEIGHT &&
      !boss_hit_test(user.x, user.y + STEP_USER_Y, boss.x, boss.y)) {
    if (user_jump < 0) {
      dy = 0;
    } else {
      if (dy >= 0) {
        user_jump--;
      }
    }
  } else {
    user_jump = 0;
  }
  
  x = user.x + dx * STEP_USER_X;
  y = user.y + dy * STEP_USER_Y;

  // アウトオブバウンズ？
  if (x < 0 || WIDESCREEN_WIDTH  - 40  < x ||
      y < 0 || WIDESCREEN_HEIGHT - 40 < y) {
    user_jump = -1;
    goto done_move;
  }

  // 当たってる？
  if (boss_hit_test(x, y, boss.x, boss.y)) {
    boss_attack_boss();
    user_jump = -1;
    goto done_move;
  }
  
  // 下方向の移動をクリア
  dy = min(dy, 0);
  
  // 移動
  user.x = x;
  user.y = user.y + dy * STEP_USER_Y;

done_move:
  user.frame = battle_frame_user[user.dir] + (user.frame + 1) % 2;
  return 1;
}

int boss_move_boss(void)
{
  int x, y;
  int dx = 0;
  int dy = 0;
  
  if (boss.y + 120 < user.y + 40) dy++;
  else if (boss.y >= user.y + 40) dy--;

#if 0
  // ブレスを吐くならば、頭上のユーザーを嫌う
  if (breath_state != BREATH_NO_BREATH && dy < 0) {
    dy = 0; // 飛ばない
    if (user.x < (WIDESCREEN_WIDTH + 40) / 2)
      dx++;
    else
      dx--;
    // 最終ボス？
    if (boss_final_battle) {
      dx *= 4;
    }
  }
  else if (boss.x + 120 <= user.x) dx++;
  else if (boss.x >= user.x + 40) dx--;
#else
  if (boss.x + 120 <= user.x) dx++; else if (boss.x >= user.x + 40) dx--;
#endif
  
  x = boss.x + dx * STEP_BOSS_X;
  y = boss.y + dy * STEP_BOSS_Y;

  // 当たってる？
  if (boss_hit_test(user.x, user.y, x, y)) {
#if 0
    // ブレスを吐いていない？
    if (breath_state != BREATH_WARMING_UP ||
        breath_state != BREATH_BREATHING) {
      boss_attack_user();
    }
#else
    boss_attack_user();
#endif
    if (boss.y + 120 < WIDESCREEN_HEIGHT &&
        boss.y + 120 > user.y) {
      // 重力
      boss.y += STEP_BOSS_Y;
    }
    goto done_move;
  }
  
  // アウトオブバウンズ？
  boss.x = max(0, min(WIDESCREEN_WIDTH  - 120,  x));
  boss.y = max(0, min(WIDESCREEN_HEIGHT - 120, y));

done_move:
  boss.frame = (boss.x < user.x ? 3 : 0) + (boss.frame + 1) % 3;
  return 1;
}

int boss_hit_test(int user_x, int user_y, int boss_x, int boss_y)
{
  return boss_x < user_x + 40  &&
         user_x < boss_x + 120  &&
         boss_y < user_y + 40 &&
         user_y < boss_y + 120;
}

// ユーザーがボスを攻撃
void boss_attack_boss(void)
{
  int damage = user_attack_point();
  
  // 最終ボス？
  if (boss_final_battle) {
    damage *= 32;
  }

  damage -= boss.status.DEF;
  if (damage > 0) {
    boss.status.HP -= damage;
    
    // ボスは死んでない？
    if (boss.status.HP >= 0) {
      static point_t damaged;
      int x = user.x + move_table[user.dir].x * 40;
      int y = user.y + move_table[user.dir].y * 40;
      
      x = min(max(x, boss.x), boss.x + 120  - 40);
      y = min(max(y, boss.y), boss.y + 120 - 40);
      damaged.x = x;
      damaged.y = y;
      damaged_boss = &damaged;
    }
    update_boss_HP(SDL_::Color::RED);
    playSound(SoundId::boss_hit); // SE
  }
}

// ボスがユーザーを攻撃
void boss_attack_user(void)
{
  int damage = boss.status.STR - user_defend_point(GUARD_FRONT);

  if (damage <= 0)
    damage = 5;
      
  user.status.HP -= damage;
  
  // ユーザーは死んでない？
  if (user.status.HP > 0) {
    static point_t damaged;
    
    damaged.x = user.x;
    damaged.y = user.y;
    damaged_user = &damaged;
  }
  update_user_HP(SDL_::Color::RED);
  playSound(SoundId::boss_hit); // SE
}

static void boss_update_integer(std::shared_ptr<SDL_::Image> img, int pts, SDL_::Color pixel)
{
  char buf[16];

  // 背景を消す(pattern_guageは旧実装のタイル地紋で未移植のため、単色で塗りつぶす)
  fill_image(img, 0, 16, 128, 16, SDL_::Color::BLACK);

  if (pts < 0) {
    pts = 0;
    pixel = SDL_::Color::RED;
  }
  sprintf(buf, "%07d", pts);
  draw_text(img, 0, 16, buf, pixel);
}

void update_user_HP(SDL_::Color pixel)
{
  boss_update_integer(clip_user_guage, user.status.HP, pixel);
  update(rect_user_guage);
}

void update_boss_HP(SDL_::Color pixel)
{
  boss_update_integer(clip_boss_guage, boss.status.HP, pixel);
  update(rect_boss_guage);
}

void boss_cast_spell(int scroll_id, int INT, int x, int y, int dir)
{
  int scroll_type, scroll_attribute;
  if (scroll_id < 0 || MAX_GOODS <= scroll_id || INT == 0)
    return;

  scroll_type = scroll_data()[scroll_id].type;
  scroll_attribute = scroll_data()[scroll_id].attribute;

  playSound(resolveCastSound(scroll_type));

  // 全体魔法？
  if (scroll_attribute) {
    magic_attack_boss();
  } else {
    user_magic.lifetime    = INT;
    user_magic.scroll_type = scroll_type;
    user_magic.x           = x + 12;
    user_magic.y           = y + 12;
    user_magic.dir         = dir;
    user_magic.frame       = frame_magic[scroll_type];
  }    
}

int boss_move_magic(void)
{
  magic_t *ma;
  int x, y;
  int n = 0;

  ma = &user_magic;

  if (ma->lifetime >= 0) {
    ma->lifetime--;
    
    x = ma->x;
    y = ma->y;
    
    n++;

    switch (ma->dir) {
    case 2:                 y += STEP_MAGIC; break;
    case 1: case 4: case 7: x -= STEP_MAGIC; break;
    case 3: case 6: case 9: x += STEP_MAGIC; break;
    case 8:                 y -= STEP_MAGIC; break;
    }

    if (0 <= x && x < rect_shrine.width  - 16 &&
        0 <= y && y < rect_shrine.height - 16) {
      // 当たった？
      if (boss.x < x + 16 && x < boss.x + 120  &&
          boss.y < y + 16 && y < boss.y + 120) {
        ma->lifetime = -1;
        magic_attack_boss();
      } else {
        ma->x = x;
        ma->y = y;
        ma->frame = frame_magic[ma->scroll_type] + (ma->frame + 1) % 2;
      }
    } else {
      // アウトオブバウンズ
      ma->lifetime = -1;
    }
  }
  return n;
}

// 魔法がボスを攻撃
void magic_attack_boss(void)
{
  int damage = user_magic_point(0) - boss.status.MGR;
  
  if (damage > 0) {
    boss.status.HP -= damage;
    // 死んだ？
    if (boss.status.HP < 0) {
      set_timer_proc(boss_win_loop);
    } else {
      magic_attacked = 1;
    }
    update_boss_HP(SDL_::Color::RED);
    playSound(SoundId::boss_hit); // se
  }
}

// 魔法ダメージ
void magic_damage_effect(void)
{
  int i, x, y;
  for (i = 0; i < 8; i++) {
    for (y = 40; y < 120; y += 40) {
      for (x = 40; x < 120; x += 40) {
        inverse_image(clip_shrine,
                      boss.x + x + random_integer(40) - 40,
                      boss.y + y + random_integer(40) - 40,
                      mask_damaged);
      }
    }
  }
}

// 回復
void boss_healing(void)
{
  int magic_item, item_type;
  int skill, point;
  
  magic_item = user.equipment[GOODS_MAGIC_ITEM];
  item_type = goods_data[GOODS_MAGIC_ITEM][magic_item].type;
  
  if (item_type == ITEM_RED_POTION &&
      user.inventory[GOODS_MAGIC_ITEM][magic_item].stock > 0) {
    user.inventory[GOODS_MAGIC_ITEM][magic_item].stock--;
    skill = user.inventory[GOODS_MAGIC_ITEM][magic_item].skill;

    // see use_item.c
    point = (user_WIS() * skill / 10000.0) * user.status.max_HP;
    user.status.HP = min(user.status.HP + point, user.status.max_HP);
    
    update_user_HP(SDL_::Color::WHITE);
    playSound(SoundId::invoke);
  }
}

// 勝利ループ
void boss_win_loop(void)
{
  // ダメージ個所を消す
  damaged_boss = damaged_user = NULL;
  
  breath_state = BREATH_NO_BREATH;

  // ボスが着地するのを待つ
  if (boss.y < WIDESCREEN_HEIGHT - 120) {
    boss.y += STEP_BOSS_Y;
    update_background();
  } else {
    boss_loop_counter--;
    
    // まだやっつけられ図ではない？
    if (boss.frame != FRAME_BOSS_DEAD) {
      if (boss_loop_counter < 0) {
        // やっつけられ図をしばらく停止する
        boss_loop_counter = BOSS_LOOP_WAIT;
        boss.frame = FRAME_BOSS_DEAD;

        playSound(SoundId::dead); // se
        update_background();
      } else {
        update_background();
        magic_damage_effect();
        playSound(SoundId::boss_hit); // se
      }
    } else if (boss_loop_counter < 0) {
      // 踏み潰した？
      if (boss_hit_test(user.x, user.y, boss.x, boss.y)) {
        set_timer_proc(boss_loose_loop);
      } else {
        // 帰還する
        restore_context(1);
      }
    }
  }
}

void boss_loose_loop(void)
{
  // ダメージ個所を消す
  if (damaged_boss || damaged_user) {
    damaged_boss = damaged_user = NULL;
    update_background();
  }

  if (user.status.HP >= 0) {
    // ユーザーのヒットポイントが0になるまで待つ
    if (user.status.HP < 1000) {
      user.status.HP = -1;
    } else {
      user.status.HP /= 2;
    }
    update_user_HP(SDL_::Color::RED);
  }
  
  if (user.y < rect_shrine.height - 40) {
    // ユーザーが落下するまで待つ
    user.y += STEP_USER_Y;
    update_background();
  } else if (user.status.HP < 0) {
    if ((boss_user_ascend -= STEP_USER_Y) > 0) {
      // 昇天中
      update_background();
      draw_sprite(clip_shrine, user.x, boss_user_ascend, frame_specials[SPECIAL_HEAVEN]);
      draw_text(clip_shrine, 180, 120, "You are Dead !!", SDL_::Color::RED);
    } else {
      restore_context(0);
    }
  }
}

int boss_breathe(void)
{
  switch (breath_state) {
  case BREATH_FILLING_UP:
    // 接地している？
    if (boss.y + 120 == rect_shrine.height && --breath_timer < 0) {
      breath_timer = breath_warm_up_interval();
      breath_state = BREATH_WARMING_UP;
    }
    break;
    
  case BREATH_WARMING_UP:
    if (--breath_timer < 0) {
      breath_timer = breath_breathe_interval();
      breath_state = BREATH_BREATHING;
      // ボスのブレス音(breath.wav)は現状アセットが存在せず無音のまま(既知の欠落)
    }
    break;
    
  case BREATH_BREATHING:
    // ブレスの有効期間？: update_background() も参照
    if (breath_timer % 4 < 2) {
      // 矩形がボスの大きさになるように少しずらす...
      int y = boss.y + 120 - SQUARE_BREATH;
      int x = boss.x + (boss_frame_left(boss.frame)
                        ? -SQUARE_BREATH
                        : 120 - (120 - SQUARE_BREATH));
      // ユーザーに命中？
      if (boss_hit_test(user.x, user.y, x, y)) {
        set_timer_proc(boss_loose_loop);
        playSound(SoundId::boss_hit); // SE
        return 0;
      }
    }
    // または離陸している？
    if (boss.y + 120 < rect_shrine.height || --breath_timer < 0) {
      breath_timer = breath_fill_up_interval();
      breath_state = BREATH_FILLING_UP;
    }
    break;
  }
  return 0;
}
