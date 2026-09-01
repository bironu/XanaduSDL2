#ifndef dungeon_H
#define dungeon_H

#ifdef __GNUC__
#define PACKED __attribute__((packed))
#else
#define PACKED
#endif

#define MAX_DUNGEON_LEVEL	11	/* 迷宮の階層の数 */

#define MAX_TILE		64	/* 地形タイルの数 */

#define MAX_TOMB		16	/* モンスター出現位置の数 */
#define MAX_SHOP		32	/* ショップの数 */

#define MAX_MEMBER		9	/* 構成員の数 */

#define FIELD_SIZE		4000	/* フィールドマップの大きさ */
#define FIELD_WIDTH		80	/* フィールドマップの幅 */
#define FIELD_HEIGHT		50	/* フィールドマップの高さ */

#define TOWER_SIZE		64	/* タワー内部マップの大きさ */

/* 構成員の種類 */
#define MEMBER_UNUSED		0	/* 未使用 */
#define MEMBER_MONSTER		1	/* モンスター */
#define MEMBER_BOX		2	/* 宝箱 */
#define MEMBER_GOODS		3	/* 品物 */
#define MEMBER_UNSEEN		-1	/* 見えないモンスター */

/* 構成員はモンスター？ */
#define member_monster(mm)	((mm)->type == MEMBER_MONSTER || \
                                 (mm)->type == MEMBER_UNSEEN)

/* 部屋の中の構成員の情報を保持する構造体 */
typedef struct {
  short		type;			/* 動的型タグ */
  short		x;			/* 水平位置 */
  short		y;			/* 垂直位置 */
  short		frame;			/* フレーム */
  short		value;			/* 宝箱の色・品物番号 etc */
} PACKED member_t;

#define BARRIER_OPEN		0	/* 開いている */
#define BARRIER_WALL		1	/* 閉じている */
#define BARRIER_LOCK		2	/* 鍵をかけられている */
#define BARRIER_EXIT		3	/* タワーの出口 */

/* タワー内の部屋の情報を保持する構造体 */
typedef struct {
  char		barrier[4];		/* 上下左右のバリア(下左右上) */
  short		monster_id;		/* モンスター番号 */
  short		boss_id;		/* ボス番号(1..n) */
  short		entrance_point;		/* タワー入口のフィールド位置 */
  member_t	members[MAX_MEMBER];	/* 構成員情報 */
} PACKED room_t;

/* フィールドのモンスター(および出現位置)を保持する構造体 */
typedef struct {
  short		point_tomb;		/* 出現位置 */
  short		monster_id;		/* モンスター番号 */
  short		num_members;		/* 構成員の数 */
  short		point;			/* モンスター位置 */
  short		frame;			/* フレーム */
  short		dir;			/* 方向 */
  short		AGL;			/* 素早さ */
  short		activity;		/* 活動形態 */
} PACKED tomb_t;

/* フィールド位置から値にマップする構造体 */
typedef struct {
  short		point;			/* フィールド位置 */
  short		value;			/* マップする値 */
} PACKED field_point_t;

/* 地形タイルを保持する整数型 */
typedef short	map_t;

/* 迷宮の階層情報を保持する構造体 */
typedef struct {
  map_t		field[FIELD_SIZE];	/* フィールドマップ */
  room_t	tower[TOWER_SIZE];	/* タワー内部マップ */
  tomb_t	tombs[MAX_TOMB];	/* モンスター出現位置 */
  field_point_t	shops[MAX_SHOP];	/* ショップ位置 */
} PACKED level_data_t;

/* タイル情報データベースを保持する構造体 */
typedef struct {
  short		flags[MAX_TILE];	/* 地形タイルの種類 */
  map_t		bricks;			/* レンガ */
  map_t		marble;			/* 大理石 */
  map_t		bridge;			/* 橋げた */
  map_t		ladder;			/* はしご */
  map_t		pattern0;		/* 背景１ */
  map_t		pattern1;		/* 背景２ */
  map_t		pattern_warp;		/* 背景(ワープポイント) */
  map_t		cave_next;		/* 洞窟(次の階層に) */
  map_t		cave_back;		/* 洞窟(前の階層に) */
  map_t		cave_closed;		/* 洞窟(封印されている) */
  map_t		floor;			/* フロア */
  map_t		locked;			/* 扉 */
  map_t		field_open[3];		/* 扉を開ける(フィールド) */
  map_t		tower_open[3];		/* 扉を開ける(タワー内部) */
  map_t		digging[3];		/* 掘る */
  /* scenario 1 */
  map_t		last_tower;		/* 最後の砦の入口 */
  /* scenario 2 */
  map_t		slope_left;		/* 斜面(右下がり) */
  map_t		slope_rite;		/* 斜面(左下がり) */
  map_t		stone;			/* 人面石 */
  map_t		icicle_hazard;		/* 逆さツララ */
  map_t		cave_next3;		/* 洞窟(次に三階層) */
  map_t		cave_back3;		/* 洞窟(前に三階層) */
} tile_data_t;

/* 地形タイルの種類 */
#define TILE_WALL		0x1	/* 通過不可能 */
#define TILE_WALL_MARBLE	0x2	/* 外套も無効 */
#define TILE_WALL_DIG		0x4	/* 掘れる */
#define TILE_FOOTHOLD		0x8	/* 足場にできる */

/* 足場にできるかどうかを調べるマスク */
#define FOOTHOLD_MASK		0xf

/* レベルデータ */
extern level_data_t level_data;

/* 地形タイルデータ */
extern tile_data_t tile_data;

/* 読み込み･書き込み */
extern int load_level(int level, const char *dir);
extern int save_level(int level, const char *dir);

/* 初期化 */
extern int init_level(int level, const char *dir);

#endif /* dungeon_H */
