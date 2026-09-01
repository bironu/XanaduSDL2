#ifndef goods_H
#define goods_H

#define MAX_GOODS		17	/* 各品物の種類 */

#define GOODS_WEAPON		0	/* 武器 */
#define GOODS_SCROLL		1	/* 魔法 */
#define GOODS_ARMOUR		2	/* 鎧 */
#define GOODS_SHIELD		3	/* 盾 */
#define GOODS_MAGIC_ITEM	4	/* 魔法の道具 */
#define GOODS_OTHER_ITEM	5	/* その他 */

#define MAX_GOODS_TYPE		6

#define GOODS_FACTOR		32

#define goods_type(n)		((n) / GOODS_FACTOR)

/* 一連の品物番号は次の述語(実際には使わない)を満足すること */
#if 0
#define goods_weapon(n)		(goods_type(n) == GOODS_WEAPON)
#define goods_scroll(n)		(goods_type(n) == GOODS_SCROLL)
#define goods_armour(n)		(goods_type(n) == GOODS_ARMOUR)
#define goods_shield(n)		(goods_type(n) == GOODS_SHIELD)
#define goods_magic_item(n)	(goods_type(n) == GOODS_MAGIC_ITEM)
#define goods_other_item(n)	(goods_type(n) == GOODS_OTHER_ITEM)
#endif

/* 固定の品物番号 */
#define GOODS_GOLD		192	/* 金貨 */
#define GOODS_FOOD		193	/* 食料 */

/* 武器の種別 */
#define WEAPON_SWORD		0	/* 長剣 */
#define WEAPON_SPEAR		1	/* 長槍 */
#define WEAPON_AXE		2	/* 戦斧 */
#define WEAPON_DAGGER		3	/* 短剣 */

#define MAX_WEAPON_TYPE		4

/* 魔法の種別 */
#define SCROLL_NEEDLE		0	/* 針金 */
#define SCROLL_MITTAR		1	/* 爆弾 */
#define SCROLL_DELUGE		2	/* 洪水 */
#define SCROLL_FIRE		3	/* 火炎 */
#define SCROLL_THUNDER		4	/* 雷撃 */
#define SCROLL_POISON		5	/* 毒液 */
#define SCROLL_CORROSION	6	/* 腐食 */
#define SCROLL_TILTE		7	/* 分解 */
#define SCROLL_DEATH		8	/* デス */

#define MAX_SCROLL_TYPE		9

/* 鎧の種類 */
#define ARMOUR_LEATHER		0	/* 皮革 */
#define ARMOUR_PLATE		1	/* 鉄甲 */
#define ARMOUR_SUITE		2	/* 一式 */

#define MAX_ARMOUR_TYPE		3

/* 盾の種類 */
#define SHIELD_SMALL		0	/* 小型 */
#define SHIELD_LARGE		1	/* 大型 */
#define SHIELD_GLOVES		2	/* 小手 */

#define MAX_SHIELD_TYPE		3

/* 魔法の道具の種類 */
#define ITEM_SPECTACLES		0	/* 眼鏡 */
#define ITEM_RED_POTION		1	/* 赤薬 */
#define ITEM_LAMP		2	/* ランプ */
#define ITEM_BLACK_ONYX		3	/* 黒曜石 */
#define ITEM_FIRE_CRYSTAL	4	/* 赤水晶 */
#define ITEM_MATTOCK		5	/* つるはし */
#define ITEM_HOURGLASS		6	/* 砂時計 */
#define ITEM_WINGED_BOOTS	7	/* 羽飾り付きブーツ */
#define ITEM_MANTLE		8	/* 外套 */
#define ITEM_DEMONS_RING	9	/* 精霊の指輪 */
#define ITEM_BALANCE		10	/* 天秤 */
#define ITEM_PENDANT		11	/* 首飾り */
#define ITEM_CANDLE		12	/* 蝋燭 */
#define ITEM_RUBY		13	/* ルビー */
#define ITEM_BROWN_POTION	14	/* 茶色薬 */
#define ITEM_MIRROR		15	/* 鏡 */
#define ITEM_BOTTLE		16	/* 壷 */
/* scenario 2 */
#define ITEM_SILVER_ROSE	17	/* 薔薇細工 */
#define ITEM_KEY		18	/* 鍵 */
#define ITEM_ACID		19	/* 酸 */
#define ITEM_LADDER		20	/* 脚立 */
#define ITEM_CROSS		21	/* 十字架 */

#define MAX_ITEM_TYPE		22

/* その他の道具の種類 */
#define OTHER_CROWN		0	/* 王冠 */
#define OTHER_KEY		1	/* 鍵 */
#define OTHER_ELIXIR		2	/* 霊薬 */
#define OTHER_MUSHROOM		3	/* マッシュルーム */
#define OTHER_POTION		4	/* 毒薬 */
#define OTHER_HAMMER		5	/* 金槌 */
#define OTHER_PENDANT		6	/* 首飾り */
#define OTHER_HOLY_BIBLE	7	/* 聖書 */
#define OTHER_BOOTS		8	/* ブーツ */
#define OTHER_MAGIC_GLOVE	9	/* 小手 */
#define OTHER_ROD		10	/* ロッド */
#define OTHER_CRYSTAL		11	/* 水晶 */
/* scenario 2 */
#define OTHER_POTION2		12	/* 毒薬(パチ物) */

/* 品物の情報を保持する構造体 */
typedef struct {
  char		name[16];		/* 名前 */
  short		type;			/* 種類 */
  short		attribute;		/* 属性 */
  int		price;			/* 価格 */
  int		performance;		/* 性能 */
} goods_t;

/* 番兵 */
#define GOODS_NULL_WEAPON	MAX_GOODS
#define GOODS_NULL_SCROLL	MAX_GOODS
#define GOODS_NULL_ARMOUR	MAX_GOODS
#define GOODS_NULL_SHIELD	MAX_GOODS
#define GOODS_NULL_MAGIC_ITEM	MAX_GOODS

/* 便利なマクロ */
#define weapon_data()		(goods_data[GOODS_WEAPON])
#define scroll_data()		(goods_data[GOODS_SCROLL])
#define armour_data()		(goods_data[GOODS_ARMOUR])
#define shield_data()		(goods_data[GOODS_SHIELD])

/* 品物データを保持する配列 */
typedef goods_t goods_data_t[MAX_GOODS + 1];

/* 品物データベース */
extern const goods_data_t *goods_data;

/* フレーム情報 */
extern const short *index_goods;	/* 品物のフレーム番号 */
extern const short index_brownbox[4];	/* 赤箱のフレーム番号 */
extern const short index_whitebox[4];	/* 白箱のフレーム番号 */

/* 初期化 */
extern int init_goods(int scenario);

#endif /* goods_H */