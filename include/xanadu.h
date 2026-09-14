#ifndef xanadu_H
#define xanadu_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __GNUC__
#define stricmp(s1, s2)		strcmp((s1), (s2))
#define strnicmp(s1, s2, n)	strncmp((s1), (s2), (n))
#endif

#include "graphics.h"
#include "audio.h"
#include "keystate.h"

#include "context.h"
#include "dungeon.h"
#include "monster.h"
#include "user.h"
#include "message.h"
#include "pause.h"
#include "numinous.h"

// ディレクトリ
#define IMAGE_DIR		"../bmp"
#define LEVEL_DIR		"../map"
#define AUDIO_DIR		"../audio"
#define USERS_DIR		"../users"

// 擬似乱数
#define random_integer(n)	(rand() % (n))
#define random_direction()	(random_integer(10))

typedef struct {
  short		x;
  short		y;
} point_t;

typedef struct {
  short		x;
  short		y;
  short		width;
  short		height;
} rectangle_t;

#define SQUARE_CHARACTER	40	// キャラクタの幅と高さ
#define SQUARE_TILE		40	// 地形の幅と高さ
#define SQUARE_MAGIC		16	// 魔法の幅と高$5
#define SQUARE_BOSS		120	// ボスの幅と高さ
#define SQUARE_BREATH		80	// ブレスの幅と高さ

#define N_MONSTERS		8	// 1レベルあたりのモンスター(種類)
#define N_MAGICS		9	// 魔法の数
#define N_TILES			64	// 地形タイルの数
#define N_GOODS			64	// 物品の数
#define N_SPECIALS		4	// 特殊イメージ

#define SPECIAL_DEAD		0	// 死亡
#define SPECIAL_DISAPPEAR	1	// 消滅
#define SPECIAL_GRAVE		2	// 墓標
#define SPECIAL_HEAVEN		3	// 昇天

#define MIN_INTERVAL		50	// 最小のインターバル(msec)
#define MAX_INTERVAL		1000	// 最大のインターバル(msec)

// イメージ
extern SDL_::SubImage frame_user[10];
extern SDL_::SubImage frame_monsters[N_MONSTERS][4];
extern std::shared_ptr<SDL_::Image> frame_magics[N_MAGICS * 2];
extern SDL_::SubImage frame_tiles[N_TILES];
extern std::shared_ptr<SDL_::Image> frame_goods[N_GOODS];
extern std::shared_ptr<SDL_::Image> frame_brownbox[4];
extern std::shared_ptr<SDL_::Image> frame_whitebox[4];
extern std::shared_ptr<SDL_::Image> frame_specials[N_SPECIALS];
extern std::shared_ptr<SDL_::Image> mask_damaged;		// ダメージマスク
extern std::shared_ptr<SDL_::Image> pattern_guage;		// ボス戦のHPゲージ背景
extern std::shared_ptr<SDL_::Image> pattern_status;		// ステータス領域背景

// ビジュアル用イメージ
extern std::shared_ptr<SDL_::Image> visual_image;

// クリップ領域
extern std::shared_ptr<SDL_::Image> clip_overall;		// メインウィンドウ全域
extern std::shared_ptr<SDL_::Image> clip_main;		// メインマップ
extern std::shared_ptr<SDL_::Image> clip_message;		// メッセージ
extern std::shared_ptr<SDL_::Image> clip_status;		// ステータス
extern std::shared_ptr<SDL_::Image> clip_shrine;		// ワイドスクリーン(神殿)
extern std::shared_ptr<SDL_::Image> clip_user_guage;	// 生命力ゲージ(ユーザ)
extern std::shared_ptr<SDL_::Image> clip_boss_guage;	// 生命力ゲージ(ボス)
extern std::shared_ptr<SDL_::Image> clip_endingroll;

extern const rectangle_t rect_overall;
extern const rectangle_t rect_main;
extern const rectangle_t rect_message;
extern const rectangle_t rect_status;
extern const rectangle_t rect_shrine;
extern const rectangle_t rect_user_guage;
extern const rectangle_t rect_boss_guage;
extern const rectangle_t rect_endingroll;

extern void update_region(int x, int y, int width, int height);
extern void update_immediately(void);
extern int load_background(const char *filename);

#define update(r) (update_region((r).x, (r).y, (r).width, (r).height))

// アプリケーションの再スタート
extern void restart_application(void);

// タイマー
extern void set_timer(int interval, void (*timer_proc)(void));
extern void kill_timer(void);
extern void set_timer_proc(void (*timer_proc)(void));

extern void beep(void);

// 環境
extern int bgm_enabled(void);
extern int se_enabled(void);

#endif // xanadu_H
