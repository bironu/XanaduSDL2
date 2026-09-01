#include "context.h"
#include "menu.h"
#include "field.h"
#include "tower.h"
#include "battle.h"
#include "boss.h"
#include "shop.h"
#include "cave.h"
#include "use_item.h"
#include "equip.h"
#include "inventory.h"
#include "user_dead.h"
#include "animation.h"
#include "pause.h"
#include "fade.h"
#include "opening.h"
#include "ending.h"

/* コンテキストの情報を保持する構造体 */
typedef struct {
  int		context_id;		/* コンテキストID */
  void		(*enter_guard)(void);	/* 進入保護 */
  void		(*leave_guard)(void);	/* 退出保護 */
} context_t;

context_t current;
context_t contexts[MAX_CONTEXT] = {
  { CONTEXT_NULL,		NULL,		NULL		 },
  { CONTEXT_START_MENU,		menu_enter,	menu_leave	 },
  { CONTEXT_FIELD,		field_enter,	field_leave	 },
  { CONTEXT_TOWER,		tower_enter,	tower_leave	 },
  { CONTEXT_BATTLE,		battle_enter,	battle_leave	 },
  { CONTEXT_BOSS,		boss_enter,	boss_leave	 },
  { CONTEXT_SHOP,		shop_enter,	shop_leave	 },
  { CONTEXT_CAVE,		cave_enter,	cave_leave	 },
  { CONTEXT_USE,		use_item_enter,	use_item_leave	 },
  { CONTEXT_EQUIPMENT,		equip_enter,	equip_leave	 },
  { CONTEXT_INVENTORY,		inventory_enter, inventory_leave },
  { CONTEXT_ANIMATION,		animation_enter, animation_leave },
  { CONTEXT_USER_DEAD,		user_dead_enter, user_dead_leave },
  { CONTEXT_ENTER_CHARACTER,	message_enter_enter, message_enter_leave },
  { CONTEXT_ENTER_NUMBER,	message_enter_enter, message_enter_leave },
  { CONTEXT_ENTER_STRING,	message_enter_enter, message_enter_leave },
  { CONTEXT_PAUSE,		pause_enter,	pause_leave	 },
  { CONTEXT_FADE,		fade_enter,	fade_leave	 },
  { CONTEXT_OPENING,		opening_enter,	opening_leave	 },
  { CONTEXT_ENDING,		ending_enter,	ending_leave	 }
};

#define STACK_SIZE 8

static int stack_top;
static context_t suspended[STACK_SIZE];

void switch_context(int context_id)
{
  /* 現在のコンテキストの退出保護を呼び出す */
  if (current.leave_guard)
    (*(current.leave_guard))();

  /* コンテキストの変更 */
  if (context_id == CONTEXT_RESUME)
    current = suspended[--stack_top];
  else
    current = contexts[context_id];

  /* 新しいコンテキストの進入保護を呼び出す */
  if (current.enter_guard)
    (*(current.enter_guard))();
}

void extend_context(int context_id)
{
  suspended[stack_top++] = current;
  switch_context(context_id);
}

void resume_context(void)
{
  switch_context(CONTEXT_RESUME);
}

/* 初期状態に戻す */
void reset_context(void)
{
  user_hidden = 0;
  stack_top = 0;  
  switch_context(CONTEXT_START_MENU);
}

int current_context_id(void)
{
  return current.context_id;
}