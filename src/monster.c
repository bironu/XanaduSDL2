#include "monster.h"

#define number_of(m)		(sizeof(m)/sizeof(m[0]))

// 攻撃レベルごとの倍率
static int strength_table[16] = {
  3,
  5,
  10,
  15,
  25,
  50,
  75,
  100,
  125,
  250,
  500,
  750,
  1000,
  1500,
  3000,
  5000
};

// 防御レベルごとの倍率
static int defence_table[17] = {
  1,
  2,
  3,
  5,
  7,
  15,
  20,
  25,
  35,
  50,
  100,
  200,
  300,
  500,
  750,
  1000,
  2000
};

// 魔法ごとの倍率
static int magic_table[17] = {
  2,
  2,
  10,
  20,
  20,
  10,
  50,
  100,
  20,
  20,
  250,
  50,
  500,
  100,
  250,
  500,
  5000
};

// 攻撃レベルから攻撃力を返す
int get_monster_strength(int STR, int attack_level)
{
  if (0 <= attack_level && attack_level < number_of(strength_table))
    return STR * strength_table[attack_level];
  else
    return 0;
}

// 防御レベルから防御力を返す
int get_monster_defence(int DEF, int defend_level)
{
  if (0 <= defend_level && defend_level < number_of(defence_table))
    return DEF * defence_table[defend_level];
  else
    return 0;
}

// 魔法から攻撃力を返す
int get_monster_magic(int INT, int scroll_type)
{
  if (0 <= scroll_type && scroll_type < number_of(magic_table))
    return INT * magic_table[scroll_type];
  else
    return 0;
}  
