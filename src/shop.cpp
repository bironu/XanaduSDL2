#include "xanadu.h"
#include "shop.h"
#include "status.h"
#include "field.h" // for field_cave_open()
#include "resources/Resources.h"
#include "resources/ImageId.h"
#include "resources/SoundId.h"
#include "resources/MusicId.h"
#include "app/Application.h"

#include <ctype.h>
#include <unordered_map>
#include <string>

#define STATE_EXIT		-1
#define STATE_TRADE		0
#define STATE_BUY		1
#define STATE_BUY_GOODS		2
#define STATE_SELL		3
#define STATE_SELL_GOODS	4
#define STATE_HOW_MANY		5
#define STATE_HEALERS		6
#define STATE_TEMPLE		7
#define STATE_CASTLE		8
#define STATE_TRAINING		9
// scenario 2
#define STATE_MENU_TRADE	10
#define STATE_MENU_BUY		11
#define STATE_MENU_SELL		12
#define STATE_MENU_BUY_GOODS	13
#define STATE_MENU_SELL_GOODS	14

// ショップデータベース
static struct {
  const char *		picture_file;	// イメージ
  const char *		name;		// 名前
} shop_data1[MAX_SHOP] = {  
  { "weapon.bmp",	"Weapon Shop"	 },
  { "scroll.bmp",	"The Oracle"	 },
  { "armory.bmp",	"Armory"	 },
  { "shield.bmp",	"Shield Shop"	 },
  { "item.bmp",		"Item Shop"	 },
  { "guilds.bmp",	"Guilds"	 },
  { "foods.bmp",	"The Grocery"	 },
  { "inn.bmp",		"Travelers Inn"	 },
  { "healers.bmp",	"The Healers"	 },
  { "temple.bmp",	"Temple"	 },
  { "castle.bmp",	"Enter-Castle"	 },
  { "str.bmp",		"10 STR"	 },	// Barrack
  { "int.bmp",		"10 INT"	 },	// Academy
  { "wis.bmp",		"10 WIS"	 },	// Zen Temple
  { "dex.bmp",		"10 DEX"	 },	// Workplace
  { "agl.bmp",		"5 AGL"		 },	// Gymnasium
  { "chr.bmp",		"10 CHR"	 },	// Salon
  { "mgr.bmp",		"5 MGR"		 }	// A Witch
};

namespace
{
// shop_data1[].picture_file(ファイル名)からImageIdを引く
const std::unordered_map<std::string, ImageId> kShopPictureIds = {
	{"weapon.bmp", ImageId::picture_weapon}, {"scroll.bmp", ImageId::picture_scroll},
	{"armory.bmp", ImageId::picture_armory}, {"shield.bmp", ImageId::picture_shield},
	{"item.bmp", ImageId::picture_item},     {"guilds.bmp", ImageId::picture_guilds},
	{"foods.bmp", ImageId::picture_foods},   {"inn.bmp", ImageId::picture_inn},
	{"healers.bmp", ImageId::picture_healers}, {"temple.bmp", ImageId::picture_temple},
	{"castle.bmp", ImageId::picture_castle}, {"str.bmp", ImageId::picture_str},
	{"int.bmp", ImageId::picture_int},       {"wis.bmp", ImageId::picture_wis},
	{"dex.bmp", ImageId::picture_dex},       {"agl.bmp", ImageId::picture_agl},
	{"chr.bmp", ImageId::picture_chr},       {"mgr.bmp", ImageId::picture_mgr},
};

// ショップBGM。現状の設定では病院/寺院(シナリオ2)以外は全て共通の曲になる
MusicId resolveShopMusic(int scenario, int shopId)
{
	if (scenario != 0) {
		if (shopId == SHOP_HEALERS) return MusicId::xana2_XANA2_HE;
		if (shopId == SHOP_TEMPLE)  return MusicId::xana2_XANA2_TE;
	}
	return MusicId::xana2_XANA2_SH;
}
}

static int shop_id;			// ショップ番号
static int shop_state;			// 状態
static ImageId currentShopImageId = ImageId::picture_shop; // 現在表示中の絵(shop_destroyでunloadする)
static int shop_price;			// 価格
static int shop_goods;			// 品物番号

// scenario 2
static int shop_count;			// 品物の数
#define MAX_SHOP_DATA2		80	// ショップの最大数
#define MAX_SHOP_TEXT		512	// テキストのサイズ

static int shop_loaded;			// 読み込んだショップの数
static shop_data_t shop_data2[MAX_SHOP_DATA2];

// 武器・魔法・鎧・盾ショップの品揃え
static int shop_num_goods[5] = { 10, 17, 11, 3, 17 };

// ユーザー応答関連
static void shop_trade(char *s);
static void shop_buy(char *s);
static void shop_buy_goods(char *s);
static void shop_sell(char *s);
static void shop_sell_goods(char *s);
static void shop_how_many(char *s);
static void shop_healers(char *s);
static void shop_enter_name(char *s);
static void shop_hit_any_key(char *s);
static void shop_pay_fee(char *s);
static void shop_temple(void);
// scenario 2
static void shop_trade2(char *s);
static void shop_buy2(char *s);
static void shop_buy_goods2(char *s);
static void shop_sell2(char *s);
static void shop_sell_goods2(char *s);

// 値段
static int shop_ask_price(int base_price);

// 品物
static int shop_dealing_goods_type(void);

// 在庫表示関連
static void status_shop_goods(void);

// 訓練場関連
static int training_initiated_all(void);

// ビジュアル表示関連
static void shop_show_visual(ImageId id);

// scenario 2
static int load_shop(void);
static void shop_show_menu(const char *text);

int init_shop(int id)
{
  shop_id = id;

  // 前の文脈(王城の名前入力等)の残り行が持ち越されないよう、
  // メッセージログを消してから店のメッセージを出す
  flush_message();

  // BGM
  playBgm(resolveShopMusic(user.environment.scenario, shop_id));

  // 初期状態を決める
  switch (id) {
  case SHOP_WEAPON:
  case SHOP_SCROLL:
  case SHOP_ARMORY:
  case SHOP_SHIELD:
  case SHOP_SECRET:
    emit_message("Enter-Shop");
    emit_message(shop_data1[shop_id].name);
    shop_state = STATE_TRADE;
    break;

  case SHOP_GUILDS:
    emit_message(shop_data1[shop_id].name);
    // 鍵の値段
    shop_price = (user_higher_rank() + 1) * 100;
    shop_price = shop_ask_price(shop_price);
    format_message("1 key %d gp", shop_price);
    shop_state = STATE_HOW_MANY;
    break;
    
  case SHOP_FOODS:
    emit_message(shop_data1[shop_id].name);
    emit_message("10 foods 1 gp");
    shop_price = 1;
    shop_state = STATE_HOW_MANY;
    break;
    
  case SHOP_INN:
    emit_message(shop_data1[shop_id].name);
    emit_message("10 Htp 1 gp");
    shop_price = 1;
    shop_state = STATE_HOW_MANY;
    break;
    
  case SHOP_HEALERS:
    emit_message(shop_data1[shop_id].name);
    shop_state = STATE_HEALERS;

    // 治療費の計算
    if (user.status.HP < user.status.max_HP) {
      int to_heal;
      if (!in_scenario2()) {
        to_heal = user.status.max_HP;
      } else {
        // scenario 2
        to_heal = user.status.max_HP - user.status.HP;
      }
      // 最低1GPは支払う
      shop_price = max(1, shop_ask_price((8 * to_heal) / 100));
    } else {
      shop_price = 0;
    }
    break;

  case SHOP_TEMPLE:
    emit_message(shop_data1[shop_id].name);
    shop_state = STATE_TEMPLE;
    break;

  case SHOP_CASTLE:
    if (user.status.name[0] != '\0') {
      shop_state = STATE_EXIT;
    } else {
      emit_message(shop_data1[shop_id].name);
      shop_state = STATE_CASTLE;
    }
    break;

  case SHOP_STR:
  case SHOP_INT:
  case SHOP_WIS:
  case SHOP_DEX:
  case SHOP_AGL:
  case SHOP_CHR:
  case SHOP_MGR:
    emit_message("Enter-Training");
    format_message("100gp %s", shop_data1[shop_id].name);
    shop_price = 100;
    shop_state = STATE_TRAINING;
    break;
    
  default:
    // scenario 2
    // まだ読み込んでいなければショップデータを読み込む
    if (shop_loaded == 0) {
      shop_loaded = load_shop();
    }

    shop_id -= SHOP_MGR + 1;

    if (shop_id < shop_loaded) {
      shop_state = STATE_MENU_TRADE;
    } else {
      shop_state = STATE_EXIT;
    }
  }

  // 絵の読み込み
  if (shop_state != STATE_MENU_TRADE &&
      shop_id < sizeof(shop_data1)/sizeof(shop_data1[0])) {
    
    // scenario 2: 寺院
    if (in_scenario2() && shop_id == SHOP_TEMPLE) {
      // レベルアップ処理後に移動
    } else {
      ImageId imageId;

      if (shop_id == SHOP_TEMPLE &&
          user.status.fighter.rank > 12 && user.status.wizard.rank > 12) {
        // ヒント
        imageId = ImageId::picture_slayer;
      } else {
        auto it = kShopPictureIds.find(shop_data1[shop_id].picture_file);
        imageId = it != kShopPictureIds.end() ? it->second : ImageId::picture_shop;
      }

      // ビジュアル表示
      shop_show_visual(imageId);
    }
  }
  // scenario 2
  else if (shop_id < shop_loaded) { 
    // テキストメニュー
    shop_show_menu(shop_data2[shop_id].text);
  }
  return CONTEXT_SHOP;
}

namespace
{
constexpr SoundId kShopSoundIds[] = { SoundId::get, SoundId::invoke };
}

void shop_create(void)
{
  auto &res = Resources::instance();
  auto &mixer = Application::instance().getMixer();
  for (SoundId id : kShopSoundIds) {
    res.loadSound(mixer, id);
  }
}

void shop_destroy(void)
{
  auto &res = Resources::instance();
  for (SoundId id : kShopSoundIds) {
    res.unloadSound(id);
  }
  res.unloadImage(currentShopImageId);
}

void shop_enter(void)
{
  switch (shop_state) {
  case STATE_TRADE: // ユーザーが売るのか買うのか尋ねる
    emit_message("Sell or Buy ?");
    extend_context(init_enter_buffer(CONTEXT_ENTER_CHARACTER, shop_trade));
    break;
    
  case STATE_BUY: // ユーザーが買う品物を尋ねる
    status_shop_goods();
    emit_message("Select ?");
    extend_context(init_enter_buffer(CONTEXT_ENTER_CHARACTER, shop_buy));
    break;
    
  case STATE_BUY_GOODS: // 品物を買うかどうか尋ねる
    format_message("%dgp ok?(y/n)", shop_price);
    extend_context(init_enter_buffer(CONTEXT_ENTER_CHARACTER,
                                     shop_buy_goods));
    break;

  case STATE_SELL: // ユーザーが売る品物を尋ねる
    status_user_goods(shop_dealing_goods_type());
    emit_message("Select ?");
    extend_context(init_enter_buffer(CONTEXT_ENTER_CHARACTER, shop_sell));
    break;
    
  case STATE_SELL_GOODS: // ユーザーが品物を売るかどうか尋ねる
    format_message("%d gp ok?(y/n)", shop_price);
    extend_context(init_enter_buffer(CONTEXT_ENTER_CHARACTER,
                                     shop_sell_goods));
    break;
    
  case STATE_HOW_MANY: // いくら？
    status_update_HP(SDL_::Color::WHITE);
    status_update_gold();
    status_update_food();
    emit_message("How many?(1-99)");
    extend_context(init_enter_buffer(CONTEXT_ENTER_NUMBER, shop_how_many));
    break;
    
  case STATE_HEALERS: // 病院
    if (shop_price > 0) {
      format_message("%dgp ok?(y/n)", shop_price);
      extend_context(init_enter_buffer(CONTEXT_ENTER_CHARACTER,
                                       shop_healers));
    } else {
      emit_message("Hit any key");
      extend_context(init_enter_buffer(CONTEXT_ENTER_CHARACTER,
                                       shop_hit_any_key));
    }
    break;

  case STATE_TEMPLE: // 寺院
    shop_temple();
    emit_message("Hit any key");
    extend_context(init_enter_buffer(CONTEXT_ENTER_CHARACTER,
                                     shop_hit_any_key));
    break;

  case STATE_CASTLE: // 王城
    emit_message("Your name ?");
    extend_context(init_enter_buffer(CONTEXT_ENTER_STRING, shop_enter_name));
    break;

  case STATE_TRAINING: // 訓練場
    status_user_status();
    
    // すべて学び終えた？(またはお金がない)
    if (training_initiated_all() || user.status.gold < shop_price) {
      emit_message("Hit any key");
      extend_context(init_enter_buffer(CONTEXT_ENTER_CHARACTER,
                                       shop_hit_any_key));
    } else {
      emit_message("You pay y/n?");
      extend_context(init_enter_buffer(CONTEXT_ENTER_CHARACTER,
                                       shop_pay_fee));
    }
    break;

    // scenario 2
  case STATE_MENU_TRADE:
    // 扱う品物がある？
    if (shop_data2[shop_id].articles[0].price > 0) {
      emit_message("Sell or Buy ?");
      extend_context(init_enter_buffer(CONTEXT_ENTER_CHARACTER, shop_trade2));
    } else {
      emit_message("Hit any key");
      extend_context(init_enter_buffer(CONTEXT_ENTER_CHARACTER,
                                       shop_hit_any_key));
    }
    break;

  case STATE_MENU_BUY:
    emit_message("Select ?");
    extend_context(init_enter_buffer(CONTEXT_ENTER_CHARACTER, shop_buy2));
    break;

  case STATE_MENU_BUY_GOODS:
    format_message("%dgp ok?(y/n)", shop_price);
    extend_context(init_enter_buffer(CONTEXT_ENTER_CHARACTER,
                                     shop_buy_goods2));
    break;
    
  case STATE_MENU_SELL:
    emit_message("Select ?");
    extend_context(init_enter_buffer(CONTEXT_ENTER_CHARACTER, shop_sell2));
    break;
    
  case STATE_MENU_SELL_GOODS:
    format_message("%dgp ok?(y/n)", shop_price);
    extend_context(init_enter_buffer(CONTEXT_ENTER_CHARACTER,
                                     shop_sell_goods2));
    break;
    
  default:
    if (shop_id == SHOP_CASTLE) {
      // ユーザーの衣装を変える
      load_user_image();
    }      
    if (shop_id == SHOP_CASTLE ||
        shop_id == SHOP_TEMPLE) {
      emit_message("Good luck !");
    } else {
      emit_message("Thanks");
    }
    resume_context();
  }
}

void shop_leave(void)
{
}

// ユーザーの所持金を表示
static void emit_user_gold(void)
{
  format_message("%dgp left", user.status.gold);
}

// ショップが取り扱う品物の種類
int shop_dealing_goods_type(void)
{
  return SHOP_WEAPON <= shop_id && shop_id <= SHOP_SECRET
    ? shop_id
    : 0; // え？
}

// 売買
void shop_trade(char *s)
{
  switch (toupper(*s)) {
  case 'B': shop_state = STATE_BUY;  break;
  case 'S': shop_state = STATE_SELL; break;
  default:  shop_state = STATE_EXIT;
  }
}

// 買う品物を選ぶ
void shop_buy(char *s)
{
  int c = toupper(*s);
  
  if (c == '\r') {
    shop_state = STATE_TRADE;
  } else {
    int n = toupper(*s) - 'A';
    int goods_type = shop_dealing_goods_type();

    if (0 <= n && n < shop_num_goods[goods_type]) {
      shop_goods = n + goods_type * GOODS_FACTOR;
      if (goods_type == GOODS_MAGIC_ITEM) {
        shop_price = max(50, 2 * (100 - user_CHR()));
      } else {
        shop_price = shop_ask_price(goods_data[goods_type][n].price);
      }
      shop_state = STATE_BUY_GOODS;

      emit_message(goods_data[goods_type][n].name);
    } else
      emit_message("What ?");
  }
}

// 品物を買う？
void shop_buy_goods(char *s)
{
  if (*s == ' ' || *s == 'Y' || *s == 'y') {
    if (shop_price <= user.status.gold) {
      int goods_type = shop_goods / GOODS_FACTOR;
      int goods_numb = shop_goods % GOODS_FACTOR;
      
      // 取り引きは成立
      user.inventory[goods_type][goods_numb].stock++;
      user.status.gold -= shop_price;
      emit_message("Thanks");
      emit_user_gold();
    } else {
      emit_message("Not enough!");
    }
  }
  shop_state = STATE_BUY;
}

// 売る品物を選ぶ
void shop_sell(char *s)
{
  int c = toupper(*s);
  
  if (c == '\r') {
    shop_state = STATE_TRADE;
  } else {
    int n = toupper(*s) - 'A';
    int goods_type = shop_dealing_goods_type();
    
    if (0 <= n && n < MAX_GOODS) {
      shop_goods = n + goods_type * GOODS_FACTOR;
      shop_price = goods_data[goods_type][n].price / 2;

      if (user.inventory[goods_type][n].stock > 0) {
        shop_state = STATE_SELL_GOODS;

        emit_message(goods_data[goods_type][n].name);
      }
    }
  }
}

// 品物を売る？
void shop_sell_goods(char *s)
{
  if (*s == ' ' || *s == 'Y' || *s == 'y') {
    int goods_type = shop_goods / GOODS_FACTOR;
    int goods_numb = shop_goods % GOODS_FACTOR;
    
    // 取り引きは成立
    user.inventory[goods_type][goods_numb].stock--;
    user.status.gold += shop_price;
    emit_message("Thanks");
    emit_user_gold();
  }
  shop_state = STATE_SELL;
}

// いくつ買う？
void shop_how_many(char *s)
{
  int n = atoi(s);

  if (n == 0) {
    shop_state = STATE_EXIT;
  } else {
    int price = shop_price * n;
    
    if (price <= user.status.gold) {
      switch (shop_id) {
      case SHOP_GUILDS:
        user.status.KEY += n;
        break;
      case SHOP_FOODS:
        user.status.food += n * 10;
        break;
      case SHOP_INN:
        user.status.HP = min(user.status.max_HP, user.status.HP + n * 10);
        break;
      default:
        shop_state = STATE_EXIT;
        return;
      }
      user.status.gold -= price;
    } else {
      emit_message("Not enough!");
      shop_state = STATE_EXIT;
    }
  }
}

// 治療費を払う？
void shop_healers(char *s)
{
  if (*s == ' ' || *s == 'Y' || *s == 'y') {
    if (shop_price <= user.status.gold) {
      user.status.gold -= shop_price;
      user.status.HP = user.status.max_HP;

      // 効果音
      playSound(SoundId::invoke);
    } else
      emit_message("Not enough!");
  }
  shop_state = STATE_EXIT;
}

void status_shop_goods(void)
{
  int i, goods_type = shop_dealing_goods_type();
  unsigned bits = 0u;

  for (i = 0; i < shop_num_goods[goods_type]; i++)
    bits |= 1u << i;

  status_list_goods(goods_type, bits);
}

int shop_ask_price(int price)
{
  int CHR = user_CHR();

  if (CHR <=  50)
    return price * (100 - CHR) /  50;
  if (CHR <= 100)
    return price * (250 - CHR) / 200;
  else
    return price * (400 - CHR) / 400;
}

// お城
void shop_enter_name(char *s)
{
  if (strlen(s) > 0) {
    strcpy(user.status.name, s);

    user.status.max_HP = 1500;
    user.status.HP     = 1500;
    user.status.KEY    =    5;
    user.status.ELX    =    3;
    user.status.gold   = 3500;
    user.status.food   =  100;

    user.status.fighter.rank = 0;
    user.status.wizard.rank = 0;

    user.equipment[GOODS_WEAPON] = 0;
    user.equipment[GOODS_SCROLL] = 0;
    user.equipment[GOODS_ARMOUR] = 0;
    user.equipment[GOODS_SHIELD] = 0;
    user.equipment[GOODS_MAGIC_ITEM] = 0;

    user.inventory[GOODS_WEAPON][0].skill = 50;
    user.inventory[GOODS_SCROLL][0].skill = 50;
    user.inventory[GOODS_ARMOUR][0].skill = 50;
    user.inventory[GOODS_SHIELD][0].skill = 50;
    
    user.inventory[GOODS_MAGIC_ITEM][0].stock = 10;
    user.inventory[GOODS_MAGIC_ITEM][0].skill = 50;
    user.inventory[GOODS_MAGIC_ITEM][1].stock =  1;

    // 隠し名に一致する？
    match_user_name(user.status.name);
    
    shop_state = STATE_EXIT;
  }
}

void shop_hit_any_key(char *s)
{
  shop_state = STATE_EXIT;
}

int training_initiated_all(void)
{
  switch (shop_id) {
  case SHOP_STR: return user.status.STR >= 100;
  case SHOP_INT: return user.status.INT >= 100;
  case SHOP_WIS: return user.status.WIS >= 100;
  case SHOP_DEX: return user.status.DEX >= 100;
  case SHOP_AGL: return user.status.AGL >= 100;
  case SHOP_CHR: return user.status.CHR >= 100;
  case SHOP_MGR: return user.status.MGR >= 100;
  }
  return 1;
}

void shop_pay_fee(char *s)
{
  if ((*s == ' ' || *s == 'Y' || *s == 'y') &&
      user.status.gold >= shop_price) {
    user.status.gold -= shop_price;
    
    switch (shop_id) {
    case SHOP_STR: user.status.STR += 10; break;
    case SHOP_INT: user.status.INT += 10; break;
    case SHOP_WIS: user.status.WIS += 10; break;
    case SHOP_DEX: user.status.DEX += 10; break;
    case SHOP_AGL: user.status.AGL +=  5; break;
    case SHOP_CHR: user.status.CHR += 10; break;
    case SHOP_MGR: user.status.MGR +=  5; break;
    }
  } else
    shop_state = STATE_EXIT;
}

static int increase_max_HP(void)
{
  int dungeon_level;
  int rank;

  dungeon_level = user.environment.dungeon_level;
  
  // 現在のランクの高い方
  rank = user_higher_rank();
  return (abs(rank - dungeon_level) + 1) * 2500;
}

void shop_temple(void)
{
  if (user.status.KRM > 0) {
    emit_message("Hmm..."); // 何て言うの？
    goto done;
  }
  
  // 戦士の経験
  if (user.status.fighter.rank < MAX_RANK - 1) {
    int rank = user.status.fighter.rank;
    int EXP  = user.status.fighter.EXP;

    if (fighter_rank[rank + 1].require_EXP <= EXP) {
      emit_message("Go up a Level!");
      emit_message("Fighter Level");
      emit_message(fighter_rank[rank + 1].name);
      
      // レベルアップ
      user.status.max_HP += increase_max_HP();
      user.status.STR += 5;
      user.status.DEX += 5;
      user.status.AGL += 5;
      user.status.fighter.rank += 1; // max_HP の増加後にやること

      playSound(SoundId::get);
      status_update_rank();

      // 封印されていた洞窟が開く(ことがある)
      field_cave_open();
      goto done;
    }
  }
  
  // 魔法使いの経験
  if (user.status.wizard.rank < MAX_RANK - 1) {
    int rank = user.status.wizard.rank;
    int EXP  = user.status.wizard.EXP;
    
    if (wizard_rank[rank + 1].require_EXP <= EXP) {
      emit_message("Go up a Level");
      emit_message("Wizard Level");
      emit_message(wizard_rank[rank + 1].name);
      
      // レベルアップ
      user.status.max_HP += increase_max_HP();
      user.status.INT += 5;
      user.status.WIS += 5;
      user.status.MGR += 5;
      user.status.wizard.rank += 1; // max_HP の増加後にやること

      playSound(SoundId::get);
      status_update_rank();

      // 封印されていた洞窟が開く(ことがある)
      field_cave_open();
      goto done;
    }
  }

done:
  if (in_scenario2()) {
    char temple_text[BUFSIZ];
    const char *temple_fmt =
      "       Temple\n\n\n\n"
      " Fighter Level %03d\n\n"
      "  %s\n\n"
      "  Next Exp. %07d\n\n"
      " Wizard Level  %03d\n\n"
      "  %s\n\n"
      "  Next Exp. %07d\n\n"
      "\n"
      "  Good luck !!";
        
    sprintf(temple_text, temple_fmt,
            user.status.fighter.rank,
            fighter_rank[user.status.fighter.rank].name,
            fighter_rank[user.status.fighter.rank + 1].require_EXP,
            user.status.wizard.rank,
            wizard_rank[user.status.wizard.rank].name,
            wizard_rank[user.status.wizard.rank + 1].require_EXP);

    shop_show_menu(temple_text);
  }
}

// scenario 2
static int lookup_goods(const char *name);

int load_shop(void)
{
  const char *filename = "../map/xa2/shop.txt";
  FILE *fp;
  int i, n;
  char text[MAX_SHOP_TEXT], buf[256];
  
  fp = fopen(filename, "rt");
  if (fp == NULL) {
    perror(filename);
    return 0;
  }
  
  for (n = 0; n < MAX_SHOP_DATA2; n++) {
    char *top = text, *end = text + sizeof(text) - 1;
    char *p;
    
    for (;;) {
      if (!fgets(buf, sizeof(buf), fp)) {
        goto done;
      }
      // コメントを読み捨てる
      if (buf[0] != '#') {
        break;
      }
    }

    // テキスト
    for (;;) {
      if (buf[0] == '$')
        break;

      for (p = buf; *p != '\0'; p++) {
        if (top < end)
          *top++ = *p;
      }
      if (!fgets(buf, sizeof(buf), fp))
        goto done;
    }
    *top++ = '\0';
    
    shop_data2[n].text = (char *)malloc(top - text);
    strcpy(shop_data2[n].text, text);

    // 商品
    i = 0;
    for (;;) {
      if (!fgets(buf, sizeof(buf), fp))
        goto done;

      if (buf[0] == '\0' || buf[0] == '\n')
        break;

      if (i < MAX_ARTICLE) {
        char *price, *count, *goods;

        price = strtok(buf,  " ");
        count = strtok(NULL, " ");
        goods = strtok(NULL, "\"");

        // 正しく切り分けられた？
        if (price != NULL && count != NULL && goods != NULL) {
          int goods_id = lookup_goods(goods);
          if (goods_id >= 0) {
            shop_data2[n].articles[i].goods = goods_id;
            shop_data2[n].articles[i].count = atoi(count);
            shop_data2[n].articles[i].price = atoi(price) / 100;
          } else {
            fprintf(stderr, "No-GOODS: %s\n", goods);
          }
	}
        i++;
      }
    }
  }
done:
  fclose(fp);
  return n;
}

int lookup_goods(const char *name)
{
  int goods_type, n;

  // 食料？
  if (strcmp(name, "Food") == 0)
    return GOODS_FOOD;

  for (goods_type = 0; goods_type < MAX_GOODS_TYPE; goods_type++) {
    for (n = 0; n < MAX_GOODS; n++) 
      if (strcmp(name, goods_data[goods_type][n].name) == 0)
        return goods_type * GOODS_FACTOR + n;
  }
  return -1;
}

// 売買
void shop_trade2(char *s)
{
  switch (toupper(*s)) {
  case 'B': shop_state = STATE_MENU_BUY;  break;
  case 'S': shop_state = STATE_MENU_SELL; break;
  default:  shop_state = STATE_EXIT;
  }
}

void shop_buy2(char *s)
{
  int c = toupper(*s);
  
  if (c == '\r') {
    shop_state = STATE_MENU_TRADE;
  } else {
    shop_data_t *shop = &shop_data2[shop_id];
    int n = toupper(*s) - 'A';

    if (0 <= n && n < MAX_ARTICLE && shop->articles[n].price > 0) {
      shop_goods = shop->articles[n].goods;
      if (shop_goods == GOODS_FOOD)
        shop_count = shop->articles[n].count * 100;
      else
        shop_count = shop->articles[n].count;
      shop_price = shop_ask_price(shop->articles[n].price * 100);
      shop_state = STATE_MENU_BUY_GOODS;
    } else
      emit_message("What ?");
  }
}

// 品物を買う？
void shop_buy_goods2(char *s)
{
  if (*s == ' ' || *s == 'Y' || *s == 'y') {
    if (user.status.gold < shop_price) {
      emit_message("Not enough!");
    } else {
      int goods_type = shop_goods / GOODS_FACTOR;
      int goods_numb = shop_goods % GOODS_FACTOR;
      
      // 取り引きは成立      
      user.status.gold -= shop_price;
      
      // 食料？
      if (shop_goods == GOODS_FOOD) {
        user.status.food += shop_count;
        status_update_food();
      }
      // 在庫？
      else if (goods_type < GOODS_OTHER_ITEM) {
        user.inventory[goods_type][goods_numb].stock += shop_count;
      }
      // その他の道具
      else if (goods_data[goods_type][goods_numb].type == OTHER_ELIXIR) {
        user.status.ELX += shop_count;
      } else {
        emit_message("How do you carry it?");
      }
      emit_message("Thanks");
      status_update_gold();
    }
  }
  shop_state = STATE_MENU_BUY;
}

void shop_sell2(char *s)
{
  int c = toupper(*s);
  
  if (c == '\r') {
    shop_state = STATE_MENU_TRADE;
  } else {
    shop_data_t *shop = &shop_data2[shop_id];
    int n = toupper(*s) - 'A';

    if (0 <= n && n < MAX_ARTICLE && shop->articles[n].price > 0) {
      shop_goods = shop->articles[n].goods;
      if (shop_goods == GOODS_FOOD)
        shop_count = shop->articles[n].count * 100;
      else
        shop_count = shop->articles[n].count;
      shop_price = shop->articles[n].price * 100 / 2;
      shop_state = STATE_MENU_SELL_GOODS;
    } else
      emit_message("What ?");
  }
}

// 品物を売る？
void shop_sell_goods2(char *s)
{
  if (*s == ' ' || *s == 'Y' || *s == 'y') {
    int goods_type = shop_goods / GOODS_FACTOR;
    int goods_numb = shop_goods % GOODS_FACTOR;

    // 食料？
    if (shop_goods == GOODS_FOOD) {
      if (user.status.food < shop_count) {
      not_enough:
        emit_message("Not enough!");
        goto done;
      }
      user.status.food -= shop_count;
      status_update_food();
    }
    // 在庫？
    else if (goods_type < GOODS_OTHER_ITEM) {
      if (user.inventory[goods_type][goods_numb].stock < shop_count) {
        goto not_enough;
      }
      user.inventory[goods_type][goods_numb].stock -= shop_count;
    }
    // その他の道具
    else if (goods_data[goods_type][goods_numb].type == OTHER_ELIXIR) {
      if (user.status.ELX < shop_count) {
        goto not_enough;
      }
      user.status.ELX -= shop_count;
    } else {
      emit_message("Can't deal it!");
      goto done;
    }
      
    // 取り引きは成立
    user.status.gold += shop_price;
    status_update_gold();
  }
done:
  shop_state = STATE_MENU_SELL;
}

void shop_show_visual(ImageId id)
{
  Resources::instance().loadImage(id);
  visual_image = Resources::instance().getImage(id);
  currentShopImageId = id;
  if (visual_image) {
    draw_image(clip_main, 0, 0, visual_image);
    update_region(rect_main.x, rect_main.y, rect_main.width, rect_main.height);
  }
}

// scenario 2: テキスト形式のメニューを表示する
void shop_show_menu(const char *text)
{
  char buf[32];
  int row;
  const char *p = text;

  Resources::instance().loadImage(ImageId::picture_shop);
  visual_image = Resources::instance().getImage(ImageId::picture_shop);
  currentShopImageId = ImageId::picture_shop;
  if (visual_image) {
    draw_image(clip_main, 0, 0, visual_image);
  } else {
    fill_image(clip_main, 0, 0, clip_main->getWidth(), clip_main->getHeight(), SDL_::Color::BLACK);
  }

  for (row = 3; row < 22; row++) {
    char *top = buf;
    char *end = buf + sizeof(buf) - 1;

    while (*p != '\0' && *p != '\n' && top < end) {
      *top++ = *p++;
    }
    *top = '\0';
    draw_text(clip_main, 16 + 16 / 2, row * 16, buf, SDL_::Color::WHITE);
    if (*p++ == '\0')
      break;
  }
  update_region(rect_main.x, rect_main.y, rect_main.width, rect_main.height);
}
