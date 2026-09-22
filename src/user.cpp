#include "user.h"
#include "status.h"
#include "resources/Resources.h"
#include "resources/ImageId.h"

// 戦士の称号
const rank_data_t fighter_rank[MAX_RANK + 1] = {
  // 称号		経験
  { "Novice Fighter",	      0 },	
  { "Aspirant",		   3000 },
  { "Battler",		  10000 },
  { "Fighter",		  20000 },
  { "Adept",		  30000 },
  { "Chevalier",	  50000 },
  { "Veteran",		  75000 },
  { "Warrior",		 100000 },
  { "Swordsman",	 150000 },
  { "Hero",		 200000 },
  { "Swashbuckler",	 250000 },
  { "Myrmidon",		 400000 },
  { "Champion",		 600000 },
  { "Super-Hero",	 800000 },
  { "Paladin",		1000000 },
  { "Lord",		1250000 },
  { "Master-Lord",	1500000 },
  { "",			      0 } // 番兵
};

// 魔法使いの称号
const rank_data_t wizard_rank[MAX_RANK + 1] = {
  // 称号		経験
  { "Novice Wizard",	      0 },
  { "Initiate",		   2000 },
  { "Trickster",	   5000 },
  { "Prestidigitatr",	  11000 },
  { "Evoker",		  17000 },
  { "Conjurer",		  30000 },
  { "Theurgist",	  59000 },
  { "Thaumaturgist",	  75000 },
  { "Magician",		 107000 },
  { "Phantasmist",	 139000 },
  { "Enchanter",	 203000 },
  { "Warlock",		 267000 },
  { "Sorcerer",		 395000 },
  { "Necromancer",	 683000 },
  { "Illusionist",	 811000 },
  { "Wizard-Lv.15",	1067000 },
  { "Master-Wizard",	1230000 },
  { "",			      0 } // 番兵
};

std::string user_path;
user_t user;
int user_hidden;

static int current_page = -1;

static int load_page0(int row);

namespace
{
ImageId userPageImageId(int page)
{
	switch (page) {
	case 1: return ImageId::user_user1;
	case 2: return ImageId::user_user2;
	case 3: return ImageId::user_user3;
	default:
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "userPageImageId: unknown page %d\n", page);
		return ImageId::user_user1;
	}
}
}

// 現在のユーザーの装備にふさわしいイメージをロードする
int load_user_image(void)
{
  int weapon_type;
  int shield_type;
  int armour_type;
  int page, row, i;

  weapon_type = weapon_data()[user.equipment[GOODS_WEAPON]].type;
  shield_type = shield_data()[user.equipment[GOODS_SHIELD]].type;
  armour_type = armour_data()[user.equipment[GOODS_ARMOUR]].type;

  if (using_demons_ring()) {
    // 姿は見えない
    return load_page0(0);
  }
  if (using_candle()) {
    // モンスターに変身
    return load_page0(1);
  }

  page = armour_type + 1;

  // 現在読み込んでいるページ？
  if (current_page != page) {
    ImageId newId = userPageImageId(page);
    if (current_page >= 0) {
      Resources::instance().unloadImage(userPageImageId(current_page));
    }
    Resources::instance().loadImage(newId);
    current_page = page;
  }
  auto user_base = Resources::instance().getImage(userPageImageId(current_page));

  // 何行め？
  row = (weapon_type + shield_type * MAX_WEAPON_TYPE) * 40;
  for (i = 0; i < 10; i++) {
    frame_user[i] = SDL_::SubImage{user_base, Rect(i * 40, row, 40, 40)};
  }
  return 0;
}

// ユーザーを丸腰状態にする
int load_user_unarmed(void)
{
  return load_page0(2);
}

int load_page0(int row)
{
  int i;

  Resources::instance().loadImage(ImageId::user_user0);
  auto user_base_0 = Resources::instance().getImage(ImageId::user_user0);
  for (i = 0; i < 10; i++) {
    frame_user[i] = SDL_::SubImage{user_base_0, Rect(i * 40, row * 40, 40, 40)};
  }
  return 0;
}

static int elapse_time(short *p)
{
  if (*p > 0)
    return --(*p) == 0;
  else
    return 0;
}

static int user_time, user_lunch_time;

// 時間の経過
void user_time_elapse(int interval)
{
  // シナリオ 1 では戦闘中に時間は経過しない
  if (!in_scenario2() && in_battle())
    return;
  
  user_time += interval;

  // 1 秒経過？
  if (user_time >= 1000) {
    int i, time_up, decrement_food;

    user_time = 0;

    // 変身の効果が切れた？
    time_up = elapse_time(&user.environment.effect[0]) |
              elapse_time(&user.environment.effect[1]);
    if (time_up) {
      load_user_image();
    }
    for (i = 2; i < MAX_EFFECT; i++) {
      elapse_time(&user.environment.effect[i]);
    }

    // 食料の消費    
    if (!in_battle()) {
      user_lunch_time++;
      if ((user.environment.in_tower && user_lunch_time % 4 == 0) ||
          user_lunch_time % 8 == 0) {
        
        user_lunch_time = 0;
        
        decrement_food =(user.status.max_HP + 500) / 1000;
        
        if (user.status.food < 0) {
          user.status.HP -= (user.status.max_HP + 50) / 100;
          bgm_random(1); // BGM
        } else {
          user.status.HP += decrement_food;
          user.status.HP = min(user.status.max_HP, user.status.HP);
          user.status.food -= decrement_food;
          bgm_random(0); // BGM          
        }
      }
      
      // ステータスの表示を更新
      status_update_HP(SDL_::Color::WHITE);
      status_update_food();
    }
  }
}

// 鍵
int user_use_key(void)
{
  if (in_scenario2()) {
    // シナリオ2では鍵はペンダントの位置にあるべき
    if (user.inventory[GOODS_MAGIC_ITEM][ITEM_PENDANT].stock > 0) {
      user.inventory[GOODS_MAGIC_ITEM][ITEM_PENDANT].stock--;
      return 1;
    }
  } else {
    if (user.status.KEY > 0) {
      user.status.KEY--;
      return 1;
    }
  }
  return 0;
}
