#ifndef numinous_H
#define numinous_H

#include "user.h"
#include "monster.h"

/* 防御面 */
#define GUARD_FRONT		0	/* 前面 */
#define GUARD_BACK		1	/* 背面 */
#define GUARD_SIDE		2	/* 側面 */

/* 武器・魔法・防具の性能 */
#define weapon_performance(STR, weapon_id) \
  ((STR) * weapon_data()[weapon_id].performance)
#define scroll_performance(INT, scroll_id) \
  ((INT) * scroll_data()[scroll_id].performance)
#define armour_performance(DEF, armour_id) \
  ((DEF) * armour_data()[armour_id].performance)
#define shield_performance(DEF, shield_id) \
  ((DEF) * shield_data()[shield_id].performance)

extern int user_attack_point(void);
extern int user_magic_point(int MGR);
extern int user_defend_point(int guard);

extern int monster_attack_point(const monster_status_t *m);
extern int monster_magic_point(const monster_status_t *m, int MGR);
extern int monster_defend_point(const monster_status_t *m, int guard);

/* 道具の効力 */
extern int item_performance(int WIS, int skill);

#endif /* numinous_H */