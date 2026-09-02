#ifndef audio_H
#define audio_H

/* 定義済みの効果音番号 */
#define SE_USER_HIT		0	/* ユーザーヒット */
#define SE_MAGIC_HIT		1	/* 魔法ヒット */
#define SE_MAGIC_FAILED		2	/* 魔法失敗 */
#define SE_DAMAGED		3	/* ダメージ */
#define SE_TRAPPED		4	/* おっと！ */
#define SE_MONSTER_DEAD		5	/* モンスター死亡 */

#define SE_OPEN_LOCKED		6	/* 扉を開ける */
#define SE_OPEN_BOX		7	/* 宝箱を開ける */
#define SE_TREASURE		8	/* お宝出現 */
#define SE_GET			9	/* お宝ゲット */
#define SE_GET_POISON		10	/* うげ毒だ！ */
#define SE_LOST_KEY		11	/* ロストキー */

#define SE_CAST_NEEDLE		12	/* 魔法(針金) */
#define SE_CAST_MITTAR		13	/* 魔法(爆弾) */
#define SE_CAST_DELUGE		14	/* 魔法(洪水) */
#define SE_CAST_FIRE		15	/* 魔法(火炎) */
#define SE_CAST_THUNDER		16	/* 魔法(雷撃) */
#define SE_CAST_POISON		17	/* 魔法(毒液) */
#define SE_CAST_CORROSION	18	/* 魔法(腐食) */
#define SE_CAST_TILTE		19	/* 魔法(分解) */
#define SE_CAST_DEATH		20	/* 魔法(デス) */

#define SE_USE_ITEM		21	/* 道具 */

#define SE_SOMEWHAT1		22	/* 何か１ */
#define SE_SOMEWHAT2		23	/* 何か２ */
#define SE_SOMEWHAT3		24	/* 何か３ */
#define SE_SOMEWHAT4		25	/* 何か４ */

/* 効果音ファイルを保持する構造体 */
typedef struct {
  char *		user_hit[4];	/* ユーザーヒット */
  char *		magic_hit;	/* 魔法ヒット */
  char *		magic_failed;	/* 魔法失敗 */
  char *		damaged;	/* ダメージ */
  char *		trapped;	/* おっと！ */
  char *		user_dead;	/* ユーザー死亡 */
  char *		boss_dead;	/* ボス死亡 */
  char *		monster_dead;	/* モンスター死亡 */
  char *		open_box;	/* 宝箱を開ける */
  char *		treasure;	/* お宝出現 */
  char *		get;		/* お宝ゲット */
  char *		get_poison;	/* うげ毒だ！ */
  char *		use_elixer;	/* 霊薬を飲む */
  char *		lost_key;	/* ロストキー */
  char *		cast[9];	/* 魔法 */
  char *		item[22];	/* 道具 */
  char *		boss_hit;	/* ボスヒット */
  char *		boss_breath;	/* ボスブレス */
  char *		opening0;	/* オープニング0 */
  char *		opening1;	/* オープニング1 */
  char *		opening2;	/* オープニング2 */
} se_data_t;

#define BGM_EXTRA_XA2_HEALERS	0	/* シナリオ2: 病院 */
#define BGM_EXTRA_XA2_TEMPLE	1	/* シナリオ2: 寺院 */

/* テーマ曲 */
typedef struct {
  char *		opening;	/* オープニング */
  char *		ending;		/* エンディング */
  char *		main;		/* メインテーマ */
} theme_song_t;

/* 迷宮内のBGMファイルを保持する構造体 */
typedef struct {
  char *		field[11];	/* フィールド */
  char *		tower[11];	/* タワー内部 */
  char *		boss[16];	/* ボス */
} dungeon_bgm_t;

typedef struct {
  char *		start_menu;	/* スタートメニュー */
  char *		default_shop;	/* ショップ全般 */
  char *		shop[10];	/* ショップ */
  dungeon_bgm_t		dungeon[2];	/* 迷宮 */
  theme_song_t		theme[2];	/* テーマ */
  char *		extra[8];	/* その他 */
} bgm_data_t;

/* 効果音および BGM データベース */
extern se_data_t se_data;
extern bgm_data_t bgm_data;

extern void bgm_play(const char *filename);
extern void bgm_stop(void);
extern void bgm_pause(void);
extern void bgm_restart(void);
extern void bgm_tempo(int tempo);
extern void bgm_random(int random_pitch_bend);
extern int bgm_mute(void); /* 0:SOUND ON 1:SOUND OFF */

extern void se_play(int id);
extern void se_load(int id, const char *filename);

/* 初期化関数 */
extern int init_se(void);
extern int init_bgm(void);

#endif /* audio_H */
