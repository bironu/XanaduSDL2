#include "goods.h"

// scenario 1
const goods_data_t goods_data1[6] = {
  // 武器
  {
    // 名前		種類		属性	   価格	     基本攻撃力
    { "Dagger",		WEAPON_DAGGER,	0,	    300,	      3 },
    { "Short-Sword",	WEAPON_SWORD,	0,	    500,	      5 },
    { "Spear",		WEAPON_SPEAR,	0,	   1000,	     10 },
    { "Hand-Axe",	WEAPON_AXE,	0,	   1500,	     15 },
    { "Long-Sword",	WEAPON_SWORD,	0,	   2500,	     25 },
    { "Battle-Axe",	WEAPON_AXE,	0,	   5000,	     50 },
    { "Broad-Sword",	WEAPON_SWORD,	0,	   7500,	     75 },
    { "Morning-Star",	WEAPON_SPEAR,	0,	  10000,	    100 },
    { "Lance",		WEAPON_SPEAR,	0,	  12500,	    125 },
    { "Halberd",	WEAPON_SPEAR,	0,	  25000,	    250 },
    { "+2 Battle Axe",	WEAPON_AXE,	0,	  50000,	    500 },
    { "Giant-Slayer",	WEAPON_SWORD,	0,	  75000,	    750 },
    { "Luck-Blade",	WEAPON_SWORD,	0,	 100000,	   1000 },
    { "MurasameBlade",	WEAPON_SWORD,	0,	 150000,	   1500 },
    { "Disrupt-Mace",	WEAPON_SPEAR,	0,	 300000,	   3000 },
    { "Vorpal-Weapon",	WEAPON_SWORD,	0,	 500000,	   5000 },
    { "Dragon Slayer",	WEAPON_SWORD,	0,	1000000,	    100 },
    { "",		WEAPON_DAGGER,	0,	      0,              0 }
  },
  // 魔法
  {
    // 名前		種類		全体	   価格	     基本攻撃力
    { "Needle",		SCROLL_NEEDLE,     0,	    100,	      2 },
    { "Deg-Needle",	SCROLL_NEEDLE,	   1,	    500,	      2 },
    { "Mittar",		SCROLL_MITTAR,     0,	   1000,	     10 },
    { "Deluge",		SCROLL_DELUGE,     0,	   2000,	     20 },
    { "Fire",		SCROLL_FIRE,       0,	   2000,	     20 },
    { "Deg-Mittar",	SCROLL_MITTAR,     1,	   5000,	     10 },
    { "Thunder",	SCROLL_THUNDER,    0,	   5000,	     50 },
    { "Poison",		SCROLL_POISON,     0,	  10000,	    100 },
    { "Deg-Deluge",	SCROLL_DELUGE,     1,	  25000,	     20 },
    { "Deg-Fire",	SCROLL_FIRE,       1,	  25000,	     20 },
    { "Corrosion",	SCROLL_CORROSION,  0,	  25000,	    250 },
    { "Deg-Thunder",	SCROLL_THUNDER,    1,	  50000,	     50 },
    { "Tilte",		SCROLL_TILTE,      0,	  50000,	    500 },
    { "Deg-Poison",	SCROLL_POISON,     1,	 125000,	    100 },
    { "Deg-Corrosion",	SCROLL_CORROSION,  1,	 125000,	    250 },
    { "Deg-Tilte",	SCROLL_TILTE,      1,	 250000,	    500 },
    { "Death",		SCROLL_DEATH,      0,	 500000,	   2500 },
    { "",		SCROLL_NEEDLE,	   0,	      0,	      0 }
  },
  // 鎧
  {
    // 名前		種類		属性	   価格	     最大防御力
    { "Cloth",		ARMOUR_LEATHER,	0,	    100,	      1 },
    { "Leather-Armor",	ARMOUR_LEATHER,	0,	    500,	      2 },
    { "Padded-Mail",	ARMOUR_LEATHER,	0,	   1000,	      3 },
    { "Studded-Mail",	ARMOUR_LEATHER,	0,	   2000,	      5 },
    { "Ring-Mail",	ARMOUR_LEATHER,	0,	   3000,	      7 },
    { "Scale-Armor",	ARMOUR_LEATHER,	0,	   5000,	     15 },
    { "Chain-Mail",	ARMOUR_PLATE,	0,	   7500,	     20 },
    { "Sprint-Mail",	ARMOUR_LEATHER,	0,	   8000,	     25 },
    { "Banded-Armor",	ARMOUR_LEATHER,	0,	  10000,	     35 },
    { "Half-Plate",	ARMOUR_PLATE,	0,	  20000,	     50 },
    { "Full-Plate",	ARMOUR_PLATE,	0,	  40000,	    100 },
    { "+2 Leather",	ARMOUR_LEATHER,	0,	  50000,	    200 },
    { "Reflex",		ARMOUR_PLATE,	0,	  75000,	    300 },
    { "+2 Ring-Mail",	ARMOUR_LEATHER,	0,	 100000,	    500 },
    { "+2 Full-Plate",	ARMOUR_PLATE,	0,	 150000,	    750 },
    { "+2 Reflex",	ARMOUR_PLATE,	0,	 200000,	   1000 },
    { "Battle-Suits",	ARMOUR_SUITE,	0,	 500000,	   2000 },
    { "",		ARMOUR_LEATHER,	0,	      0,	      0 }
  },
  // 盾
  {
    // 名前		種類		属性	   価格	     最大防御力
    { "Gloves",		SHIELD_GLOVES,	0,	    100,	      1 },
    { "Small-Shield",	SHIELD_SMALL,	0,	    400,	      2 },
    { "Large-Shield",	SHIELD_LARGE,	0,	   1000,	      5 },
    { "+1SmallShield",	SHIELD_SMALL,	0,	   2500,	     15 },
    { "+1LargeShield",	SHIELD_LARGE,	0,	   5000,	     20 },
    { "+2SmallShield",	SHIELD_SMALL,	0,	   5000,	     25 },
    { "+2LargeShield",	SHIELD_LARGE,	0,	  10000,	     50 },
    { "+3SmallShield",	SHIELD_SMALL,	0,	  10000,	     30 },
    { "+3LargeShield",	SHIELD_LARGE,	0,	  15000,	     75 },
    { "+4SmallShield",	SHIELD_SMALL,	0,	  15000,	     60 },
    { "+4LargeShield",	SHIELD_LARGE,	0,	  25000,	    100 },
    { "+5SmallShield",	SHIELD_SMALL,	0,	  25000,	    125 },
    { "+5LargeShield",	SHIELD_LARGE,	0,	  50000,	    250 },
    { "+6SmallShield",	SHIELD_SMALL,	0,	  50000,	    200 },
    { "+6LargeShield",	SHIELD_LARGE,	0,	 100000,	    600 },
    { "+7SmallShield",	SHIELD_SMALL,	0,	 100000,	    500 },
    { "+7LargeShield",	SHIELD_LARGE,	0,	 250000,	   1250 },
    { "",		SHIELD_GLOVES,	0,	      0,	      0 }
  },
  // 魔法の道具
  {
    // 名前		種類		     属性	   価格    性能
    { "Spectacles",	ITEM_SPECTACLES,	0,	    100,      0 },
    { "Red Potion",	ITEM_RED_POTION,	0,	    100,      0 },
    { "Lamp",		ITEM_LAMP,		0,	    100,      0 },
    { "Black Onyx",	ITEM_BLACK_ONYX,	0,	    100,      0 },
    { "Fire Crystal",	ITEM_FIRE_CRYSTAL,	0,	    100,      0 },
    { "Mattock",	ITEM_MATTOCK,		0,	    100,      0 },
    { "Hourglass",	ITEM_HOURGLASS,		0,	    100,      0 },
    { "Winged-boots",	ITEM_WINGED_BOOTS,	0,	    100,      0 },
    { "Mantle",		ITEM_MANTLE,		0,	    100,      0 },
    { "Demons Ring",	ITEM_DEMONS_RING,	0,	    100,      0 },
    { "Balance",	ITEM_BALANCE,		0,	    100,      0 },
    { "Pendant",	ITEM_PENDANT,		0,	    100,      0 },
    { "Candle",		ITEM_CANDLE,		0,	    100,      0 },
    { "Ruby",		ITEM_RUBY,		0,	    100,      0 },
    { "Brown Potion",	ITEM_BROWN_POTION,	0,	    100,      0 },
    { "Mirror",		ITEM_MIRROR,		0,	    100,      0 },
    { "Bottle",		ITEM_BOTTLE,		0,	    100,      0 },
    { "",		-1,			0,	      0,      0 }
  },
  // その他
  {
    // 名前		種類		     属性	   価格    性能
    { "Crown",		OTHER_CROWN,		0,	      0,      0 },
    { "Key",		OTHER_KEY,		0,	      0,      0 },
    { "Elixir",		OTHER_ELIXIR,		0,	      0,      0 },
    { "Mushroom",	OTHER_MUSHROOM,		0,	      0,      0 },
    { "Potion",		OTHER_POTION,		0,	      0,      0 },
    { "Hammer",		OTHER_HAMMER,		0,	      0,      0 },
    { "Pendant",	OTHER_PENDANT,		0,	      0,      0 },
    { "Holy Bible",	OTHER_HOLY_BIBLE,	0,	      0,      0 },
    { "Boots",		OTHER_BOOTS,		0,	      0,      0 },
    { "Magic Glove",	OTHER_MAGIC_GLOVE,	0,	      0,      0 },
    { "Rod",		OTHER_ROD,		0,	      0,      0 },
    { "Crystal",	OTHER_CRYSTAL,		0,	      0,      0 },
    { "",		-1,			0,	      0,      0 },
    { "",		-1,			0,	      0,      0 },
    { "",		-1,			0,	      0,      0 },
    { "",		-1,			0,	      0,      0 },
    { "",		-1,			0,	      0,      0 },
    { "",		-1,			0,	      0,      0 }
  }
};

const short index_goods1[256] = {
  // 武器
  10, 10, 12, 11, 10, 11, 10, 12, 12, 12, 11, 10, 10, 10, 12, 10, 10,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  // 魔法
  13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  // 鎧
  14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  // 盾
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  // 魔法の道具
  28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  // その他の道具
  16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   8,	// 金貨
   9,	// 食料
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

// scenario 2
const goods_data_t goods_data2[6] = {
  // 武器
  {
    // 名前		種類		属性	   価格	     基本攻撃力
    { "Dagger",		WEAPON_DAGGER,	0,	    300,	      3 },
    { "Short-Sword",	WEAPON_SWORD,	0,	    500,	      5 },
    { "Spear",		WEAPON_SPEAR,	0,	   1000,	     10 },
    { "Hand-Axe",	WEAPON_AXE,	0,	   1500,	     15 },
    { "Long-Sword",	WEAPON_SWORD,	0,	   2500,	     25 },
    { "Battle-Axe",	WEAPON_AXE,	0,	   5000,	     50 },
    { "Broad-Sword",	WEAPON_SWORD,	0,	   7500,	     75 },
    { "Heavy-Spear",	WEAPON_SPEAR,	0,	  10000,	    100 },
    { "Lance",		WEAPON_SPEAR,	0,	  12500,	    125 },
    { "Halberd",	WEAPON_SPEAR,	0,	  25000,	    250 },
    { "+2 Battle Axe",	WEAPON_AXE,	0,	  50000,	    500 },
    { "Giant-Slayer",	WEAPON_SWORD,	0,	  75000,	    750 },
    { "Luck-Blade",	WEAPON_SWORD,	0,	 100000,	   1000 },
    { "MurasameBlade",	WEAPON_SWORD,	0,	 150000,	   1500 },
    { "Disrupt-Mace",	WEAPON_SPEAR,	0,	 300000,	   3000 },
    { "Vorpal-Weapon",	WEAPON_SWORD,	0,	 500000,	   5000 },
    { "Dragon Slayer",	WEAPON_SWORD,	0,	1000000,	    100 },
    { "",		WEAPON_DAGGER,	0,	      0,              0 }
  },
  // 魔法
  {
    // 名前		種類		全体	   価格	     基本攻撃力
    { "Needle",		SCROLL_NEEDLE,     0,	    100,	      2 },
    { "Deg-Needle",	SCROLL_NEEDLE,	   1,	    500,	      2 },
    { "Mittar",		SCROLL_MITTAR,     0,	   1000,	     10 },
    { "Deluge",		SCROLL_DELUGE,     0,	   2000,	     20 },
    { "Fire",		SCROLL_FIRE,       0,	   2000,	     20 },
    { "Deg-Mittar",	SCROLL_MITTAR,     1,	   5000,	     10 },
    { "Thunder",	SCROLL_THUNDER,    0,	   5000,	     50 },
    { "Poison",		SCROLL_POISON,     0,	  10000,	    100 },
    { "Deg-Deluge",	SCROLL_DELUGE,     1,	  25000,	     20 },
    { "Deg-Fire",	SCROLL_FIRE,       1,	  25000,	     20 },
    { "Corrosion",	SCROLL_CORROSION,  0,	  25000,	    250 },
    { "Deg-Thunder",	SCROLL_THUNDER,    1,	  50000,	     50 },
    { "Tilte",		SCROLL_TILTE,      0,	  50000,	    500 },
    { "Deg-Poison",	SCROLL_POISON,     1,	 125000,	    100 },
    { "Deg-Corrosion",	SCROLL_CORROSION,  1,	 125000,	    250 },
    { "Deg-Tilte",	SCROLL_TILTE,      1,	 250000,	    500 },
    { "Death",		SCROLL_DEATH,      0,	 500000,	   2500 },
    { "",		SCROLL_NEEDLE,	   0,	      0,	      0 }
  },
  // 鎧
  {
    // 名前		種類		属性	   価格	     最大防御力
    { "Cloth",		ARMOUR_LEATHER,	0,	    100,	      1 },
    { "Leather-Armor",	ARMOUR_LEATHER,	0,	    500,	      2 },
    { "Padded-Mail",	ARMOUR_LEATHER,	0,	   1000,	      3 },
    { "Studded-Mail",	ARMOUR_LEATHER,	0,	   2000,	      5 },
    { "Ring-Mail",	ARMOUR_LEATHER,	0,	   3000,	      7 },
    { "Scale-Armor",	ARMOUR_LEATHER,	0,	   5000,	     15 },
    { "Chain-Mail",	ARMOUR_PLATE,	0,	   7500,	     20 },
    { "Sprint-Mail",	ARMOUR_LEATHER,	0,	   8000,	     25 },
    { "Banded-Armor",	ARMOUR_LEATHER,	0,	  10000,	     35 },
    { "Half-Plate",	ARMOUR_PLATE,	0,	  20000,	     50 },
    { "Full-Plate",	ARMOUR_PLATE,	0,	  40000,	    100 },
    { "+2 Leather",	ARMOUR_LEATHER,	0,	  50000,	    200 },
    { "Reflex",		ARMOUR_PLATE,	0,	  75000,	    300 },
    { "+2 Ring-Mail",	ARMOUR_LEATHER,	0,	 100000,	    500 },
    { "+2 Full-Plate",	ARMOUR_PLATE,	0,	 150000,	    750 },
    { "+2 Reflex",	ARMOUR_PLATE,	0,	 200000,	   1000 },
    { "Battle-Suits",	ARMOUR_SUITE,	0,	 500000,	   2000 },
    { "",		ARMOUR_LEATHER,	0,	      0,	      0 }
  },
  // 盾
  {
    // 名前		種類		属性	   価格	     最大防御力
    { "Gloves",		SHIELD_GLOVES,	0,	    100,	      1 },
    { "Small-Shield",	SHIELD_SMALL,	0,	    400,	      2 },
    { "Large-Shield",	SHIELD_LARGE,	0,	   1000,	      5 },
    { "+1SmallShield",	SHIELD_SMALL,	0,	   2500,	     15 },
    { "+1LargeShield",	SHIELD_LARGE,	0,	   5000,	     20 },
    { "+2SmallShield",	SHIELD_SMALL,	0,	   5000,	     25 },
    { "+2LargeShield",	SHIELD_LARGE,	0,	  10000,	     50 },
    { "+3SmallShield",	SHIELD_SMALL,	0,	  10000,	     30 },
    { "+3LargeShield",	SHIELD_LARGE,	0,	  15000,	     75 },
    { "+4SmallShield",	SHIELD_SMALL,	0,	  15000,	     60 },
    { "+4LargeShield",	SHIELD_LARGE,	0,	  25000,	    100 },
    { "+5SmallShield",	SHIELD_SMALL,	0,	  25000,	    125 },
    { "+5LargeShield",	SHIELD_LARGE,	0,	  50000,	    250 },
    { "+6SmallShield",	SHIELD_SMALL,	0,	  50000,	    200 },
    { "+6LargeShield",	SHIELD_LARGE,	0,	 100000,	    600 },
    { "+7SmallShield",	SHIELD_SMALL,	0,	 100000,	    500 },
    { "+7LargeShield",	SHIELD_LARGE,	0,	 250000,	   1250 },
    { "",		SHIELD_GLOVES,	0,	      0,	      0 }
  },
  // 魔法の道具
  {
    // 名前		種類		     属性	   価格    性能
    { "Spectacles",	ITEM_SPECTACLES,	0,	    100,      0 },
    { "Red Potion",	ITEM_RED_POTION,	0,	    100,      0 },
    { "Lamp",		ITEM_LAMP,		0,	    100,      0 },
    { "Black Onyx",	ITEM_BLACK_ONYX,	0,	    100,      0 },
    { "Fire Crystal",	ITEM_FIRE_CRYSTAL,	0,	    100,      0 },
    { "Mattock",	ITEM_MATTOCK,		0,	    100,      0 },
    { "Hourglass",	ITEM_HOURGLASS,		0,	    100,      0 },
    { "Winged-boots",	ITEM_WINGED_BOOTS,	0,	    100,      0 },
    { "Mantle",		ITEM_MANTLE,		0,	    100,      0 },
    { "Demons Ring",	ITEM_DEMONS_RING,	0,	    100,      0 },
    { "Silver Rose",	ITEM_SILVER_ROSE,	0,	    100,      0 },
    { "Key",		ITEM_KEY,		0,	    100,      0 },
    { "Candle",		ITEM_CANDLE,		0,	    100,      0 },
    { "Acid",		ITEM_ACID,		0,	    100,      0 },
    { "Ladder",		ITEM_LADDER,		0,	    100,      0 },
    { "Mirror",		ITEM_MIRROR,		0,	    100,      0 },
    { "Cross",		ITEM_CROSS,		0,	    100,      0 },
    { "",		-1,			0,	      0,      0 }
  },
  // その他
  {
    // 名前		種類		     属性	   価格    性能
    { "Crown",		OTHER_CROWN,		0,	      0,      0 },
    { "Potion",		OTHER_POTION2,		0,	      0,      0 },
    { "Elixir",		OTHER_ELIXIR,		0,	      0,      0 },
    { "Mushroom",	OTHER_MUSHROOM,		0,	      0,      0 },
    { "Potion",		OTHER_POTION,		0,	      0,      0 },
    { "Hammer",		OTHER_HAMMER,		0,	      0,      0 },
    { "Pendant",	OTHER_PENDANT,		0,	      0,      0 },
    { "Holy Bible",	OTHER_HOLY_BIBLE,	0,	      0,      0 },
    { "Boots",		OTHER_BOOTS,		0,	      0,      0 },
    { "Magic Glove",	OTHER_MAGIC_GLOVE,	0,	      0,      0 },
    { "Rod",		OTHER_ROD,		0,	      0,      0 },
    { "Crystal",	OTHER_CRYSTAL,		0,	      0,      0 },
    { "",		-1,			0,	      0,      0 },
    { "",		-1,			0,	      0,      0 },
    { "",		-1,			0,	      0,      0 },
    { "",		-1,			0,	      0,      0 },
    { "",		-1,			0,	      0,      0 },
    { "",		-1,			0,	      0,      0 }
  }
};

const short index_goods2[256] = {
  // 武器
  10, 10, 12, 11, 10, 11, 10, 12, 12, 12, 11, 10, 10, 10, 12, 10, 10,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  // 魔K!
  13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  // 鎧
  14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  // 盾
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  // 魔法の道具
  28, 46, 30, 31, 32, 33, 34, 35, 36, 37, 47, 17, 40, 48, 49, 43, 50,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  // その他の道具
  16, 45, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   8,	// 金貨
   9,	// 食料
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

const short index_whitebox[4] = { 0, 1, 2, 3 };
const short index_brownbox[4] = { 4, 5, 6, 7 };

// 品物データベース
const goods_data_t *goods_data = goods_data1;
const short *index_goods = index_goods1;

int init_goods(int scenario)
{
  if (scenario == 0) {
    goods_data = goods_data1;
    index_goods = index_goods1;
  } else {
    goods_data = goods_data2;
    index_goods = index_goods2;
  }
  return 0;
}