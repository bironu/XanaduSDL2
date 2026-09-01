#include "xanadu.h"
#include "numinous.h"

/* 道具の効果 */
int item_performance(int WIS, int skill)
{
  return WIS * skill / 100;
}

/* ユーザー攻撃力 */
int user_attack_point(void)
{
  int weapon_id = user.equipment[GOODS_WEAPON];
  int STR = user_STR();
  STR *= user.inventory[GOODS_WEAPON][weapon_id].skill;
  STR = min(25500, STR) / 100;
  return weapon_performance(STR, weapon_id);
}

/* ユーザー魔法攻撃力 */
int user_magic_point(int MGR)
{
  int scroll_id = user.equipment[GOODS_SCROLL];
  int INT = user_INT();;
  INT *= user.inventory[GOODS_SCROLL][scroll_id].skill;
  INT = min(25500, INT) / 100;
  INT = INT * (100 - MGR) / 100;
  return scroll_performance(INT, scroll_id);
}

/* ユーザー防御力 */
int user_defend_point(int guard)
{
  int armour_id = user.equipment[GOODS_ARMOUR];
  int DEF = user.inventory[GOODS_ARMOUR][armour_id].skill;
  int point = armour_performance(DEF, armour_id);

#if 0
  /* 方向による修正 */
  switch (guard) {
  case GUARD_FRONT:
    {
      /* 盾の防御力を加算 */
      int shield_id = user.equipment[GOODS_SHIELD];
      DEF = user.inventory[GOODS_SHIELD][shield_id].skill;
      point *= 1.2;
      point += shield_performance(DEF, shield_id);
      return point;
    }
  case GUARD_BACK:
    return point / 2;
    
  case GUARD_SIDE: /* return DEF * 1.0; */
  default:
    return point;
  }
#else
  /* 方向による修正 */
  switch (guard) {
  case GUARD_FRONT:
    {
      /* 盾の防御力を加算 */
      int shield_id = user.equipment[GOODS_SHIELD];
      DEF = user.inventory[GOODS_SHIELD][shield_id].skill;
      point += shield_performance(DEF, shield_id);
      return point;
    }
  case GUARD_BACK:
    return (point / 2) * 0.8;
    
  case GUARD_SIDE: /* return DEF * 1.0; */
  default:
    return point;
  }
#endif
}

/* モンスター攻撃力 */
int monster_attack_point(const monster_status_t *m)
{
  return weapon_performance(m->STR, m->attack_level);
}

/* モンスター魔法攻撃力 */
int monster_magic_point(const monster_status_t *m, int MGR)
{
  int INT = m->INT;
  
  if (MGR == 100)
    return 0;

  /* 修正: MGR が100を超えている場合、MGR=0 に等しい */
  if (MGR < 100) {
    INT = INT * (100 - MGR) / 100;
  }
  return scroll_performance(INT, m->magic);
}

/* モンスター防御力 */
int monster_defend_point(const monster_status_t *m, int guard)
{
  return armour_performance(m->DEF[guard], m->defend_level);
}
