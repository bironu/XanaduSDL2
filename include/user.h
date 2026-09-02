#ifndef user_H
#define user_H

#include "xanadu.h"
#include "goods.h"
#include "battle.h"

/* ユーザーの経験に関する情報を保持する構造体 */
typedef struct {
  int		EXP;			/* 経験値 */
  int		rank;			/* ランク */
} experience_t;

/* ユーザーの在庫に関する情報を保持する配列 */
typedef struct {
  int		stock;			/* 在庫量 */
  int		skill;			/* 熟練度 */
} inventory_t[5][GOODS_FACTOR];

/* ユーザーのステータス情報を保持する構造体 */
typedef struct {
  char		name[16];		/* 名前 */
  int		HP;			/* 耐久力 */
  int		max_HP;			/* 最大生命力 */
  int		STR;			/* 腕力 */
  int		INT;			/* 知識 */
  int		WIS;			/* 賢さ */
  int		AGL;			/* 敏捷 */
  int		DEX;			/* 器用 */
  int		CHR;			/* 魅力 */
  int		MGR;			/* 魔法抵抗力 */
  int		KEY;			/* 鍵 */
  int		KRM;			/* カルマ */
  int		ELX;			/* 霊薬 */
  int		CRN;			/* 王冠 */
  int		gold;			/* 金貨 */
  int		food;			/* 食料 */
  experience_t	fighter, wizard;	/* 戦士・魔法使いの経験 */
} user_status_t;

#define MAX_EFFECT		9	/* 道具の時限効果の種類 */

#define EFFECT_DEMONS_RING	0	/* 指輪 */
#define EFFECT_CANDLE		1	/* 蝋燭 */
#define EFFECT_HOURGLASS	2	/* 砂時計 */
#define EFFECT_MANTLE		3	/* 外套 */
#define EFFECT_WINGED_BOOTS	4	/* 羽飾り付きブーツ */
#define EFFECT_RUBY		5	/* ルビー */
#define EFFECT_BROWN_POTION	6	/* 茶色薬 */
#define EFFECT_MIRROR		7	/* 鏡 */
#define EFFECT_BOTTLE		8	/* 壷 */
/* scenario 2 */
#define EFFECT_CROSS		8	/* 十字架 */

/* ユーザーの設定を保持する構造体 */
typedef struct {
  short		mute;			/* 消音 */
} config_t;

/* ユーザーの環境を保持する構造体 */
typedef struct {
  short		scenario;		/* シナリオ */
  short		dungeon_level;		/* 迷宮の階層 */
  short		in_training_ground;	/* 訓練場 */
  short		in_tower;		/* タワー内部？ */
  short		in_battle;		/* 戦闘中？ */
  short		lighting;		/* ランプ */
  short		effect[MAX_EFFECT];	/* 道具の時限効果 */
  short		field_encountered;	/* フィールド用遭遇モンスター */
  room_t	field_room;		/* フィールド用の戦場 */
} environment_t;

/* ユーザー情報を保持する構造体 */
typedef struct {
  short		point;			/* マップ位置(共用) */
  short		x;			/* 水平位置 */
  short		y;			/* 垂直位置 */
  short		frame;			/* フレーム */
  short		dir;			/* 方向 */
  user_status_t status;			/* ステータス */
  short		equipment[5];		/* 装備 */
  inventory_t	inventory;		/* 在庫 */
  environment_t	environment;		/* 環境 */
  battle_t	battle;			/* 戦闘時の情報 */
  config_t	config;			/* 設定 */
} user_t;

/* 便利なマクロ１ */
#define in_training_ground()	(user.environment.in_training_ground)
#define in_tower()		(user.environment.in_tower)
#define in_battle()		(user.environment.in_battle)
#define in_darkness()		(in_tower() && user.environment.lighting == 0)
#define in_scenario2()		(user.environment.scenario != 0)

/* 便利なマクロ２ */
#define using_demons_ring()	(user.environment.effect[EFFECT_DEMONS_RING])
#define using_candle()		(user.environment.effect[EFFECT_CANDLE])
#define using_hourglass()	(user.environment.effect[EFFECT_HOURGLASS])
#define using_mantle()		(user.environment.effect[EFFECT_MANTLE])
#define using_winged_boots()	(user.environment.effect[EFFECT_WINGED_BOOTS])
#define doping_STR()		(user.environment.effect[EFFECT_RUBY])
#define doping_INT()		(user.environment.effect[EFFECT_BROWN_POTION])
#define doping_AGL()		(user.environment.effect[EFFECT_MIRROR])
#define doping_CHR()		(user.environment.effect[EFFECT_BOTTLE])

#define user_STR()	(doping_STR() ? user.status.STR * 2 : user.status.STR)
#define user_INT()	(doping_INT() ? user.status.INT * 2 : user.status.INT)
#define user_WIS()	(user.status.WIS)
#define user_DEX()	(user.status.DEX)
#define user_AGL()	(doping_STR() ? user.status.AGL * 2 : user.status.AGL)
#define user_CHR()	(doping_CHR() ? user.status.CHR * 2 : user.status.CHR)
#define user_MGR()	(user.status.MGR)
#define user_KRM()	(user.status.KRM)

/* 高い方のランクレベル */
#define user_higher_rank() (max(user.status.fighter.rank, \
                                user.status.wizard.rank))

#define MAX_RANK		17	/* ランクの種類 */
#define NULL_RANK		MAX_RANK

/* ランクデータを保持する構造体 */
typedef struct {
  char *	name;			/* 称号 */
  int		require_EXP;		/* 必要経験値 */
} rank_data_t;

/* ランクデータベース */
extern const rank_data_t fighter_rank[MAX_RANK + 1];
extern const rank_data_t wizard_rank[MAX_RANK + 1];

/* ユーザー情報 */
extern user_t user;

/* ユーザー表示フラグ */
extern int user_hidden;

/* ユーザーディレクトリ */
extern const char *user_path;

/* ユーザーイメージの読み込み */
extern int load_user_image(void);
extern int load_user_unarmed(void);

/* 時間の経過 */
extern void user_time_elapse(int interval);

/* 鍵 */
extern int user_use_key(void);

/* user_io.c */
extern int save_user(void);
extern int load_user(void);
extern int make_user_dir(void);
extern void match_user_name(const char *name);

#endif /* user_H */
