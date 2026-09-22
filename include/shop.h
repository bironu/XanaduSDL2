#ifndef shop_H
#define shop_H

// scenario 1
#define SHOP_WEAPON		0	// 武器
#define SHOP_SCROLL		1	// 魔法
#define SHOP_ARMORY		2	// 鎧
#define SHOP_SHIELD		3	// 盾
#define SHOP_SECRET		4	// 道具
#define SHOP_GUILDS		5	// 鍵
#define SHOP_FOODS		6	// 食料
#define SHOP_INN		7	// 宿屋
#define SHOP_HEALERS		8	// 病院
#define SHOP_TEMPLE		9	// 寺院

// trainig-ground
#define SHOP_CASTLE		10	// 王城
#define SHOP_STR		11	// 強さ
#define SHOP_INT		12	// 知識
#define SHOP_WIS		13	// 賢さ
#define SHOP_DEX		14	// 器用さ
#define SHOP_AGL		15	// 速さ
#define SHOP_CHR		16	// 魅力
#define SHOP_MGR		17	// 魔法抵抗力

// scenario 2
#define MAX_ARTICLE		16	// 取り扱い商品の数

// ショップの取り扱い商品に関する情報を保持する構造体
typedef struct {
  short		price;			// 価格(100分の1)
  short		count;			// 個数(100分の1)
  short		goods;			// 品物
} article_t;

// ショップに関する情報を保持する構造体
typedef struct {
  char *	text;			// テキスト
  article_t	articles[MAX_ARTICLE];	// 商品
} shop_data_t;

// コンテキスト保護関数
extern void shop_create(void);
extern void shop_destroy(void);
extern void shop_enter(void);
extern void shop_leave(void);

// 初期化関数
extern int init_shop(int shop_id);

#endif // shop_H