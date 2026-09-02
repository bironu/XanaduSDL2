#ifndef monster_H
#define monster_H

#ifdef __GNUC__
#define PACKED __attribute__((packed))
#else
#define PACKD
#endif

#define MAX_MONSTER		8	// モンスター数
#define MAX_VARIETY		4	// バラエティ数

// モンスターの行動形態フラグ
#define ACTIVITY_CREEPER	0x01	// 左右に這い回る
#define ACTIVITY_LADDER		0x08	// 垂直に移動する
#define ACTIVITY_VIVID		0x04	// 毎フレーム更新
#define ACTIVITY_FLIGHT		0x02	// 重量の影響を受けない
#define ACTIVITY_TELEPORT	0x10	// 一定時間でテレポート
#define ACTIVITY_FADEOUT	0x20	// 溶暗(フェードアウト)

#define ACTIVITY_WALKER		(ACTIVITY_CREEPER | ACTIVITY_LADDER)

// モンスターのステータス情報を保持する構造体
typedef struct {
  char		name[16];		// 名前
  char		group_min;		// 最小構成員数
  char		group_max;		// 最大構成員数
  short		max_HP;			// 耐久力(100分の1)
  short		STR;			// 強さ
  short		INT;			// 賢さ
  short		AGL;			// 素早さ
  short		DEF[3];			// 防御力
  short		MGR[9];			// 魔法抵抗力
  short		EXP;			// 経験値
  short		KRM;			// カルマ
  short		attack_level;		// 攻撃レベル
  short		defend_level;		// 防御レベル
  short		magic;			// 魔法
  short		goods;			// 所持品
  short		amount;			// 金貨・食料の量
  short		unknownA;		// 何？
  short		activity;		// 活動形態
} PACKED monster_status_t;

// モンスターデータ
extern monster_status_t monster_data[MAX_MONSTER * MAX_VARIETY];

#endif // monster_H
