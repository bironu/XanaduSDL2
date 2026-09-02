#include "xanadu.h"
#include "equip.h"
#include "status.h"

#include <ctype.h>

/* 注意: WEAPON..MAGICITEM は GOODS_ 定数に一致すること！ */
#define STATE_WEAPON		GOODS_WEAPON
#define STATE_SCROLL		GOODS_SCROLL
#define STATE_ARMOUR		GOODS_ARMOUR
#define STATE_SHIELD		GOODS_SHIELD
#define STATE_MAGIC_ITEM	GOODS_MAGIC_ITEM
#define STATE_EQUIP		(STATE_MAGIC_ITEM + 1)

#define STATE_EXIT		-1

static int equip_state;
void equip_what(char *s);
void equip_which(char *s);

int init_equip(void)
{
  equip_state = STATE_EQUIP;
  return CONTEXT_EQUIPMENT;
}

void equip_enter(void)
{
  switch (equip_state) {
  case STATE_EQUIP:
    emit_message("w/m/a/s/i?");
    extend_context(init_enter_buffer(CONTEXT_ENTER_CHARACTER, equip_what));
    break;
    
  case STATE_WEAPON:
  case STATE_SCROLL:
  case STATE_ARMOUR:
  case STATE_SHIELD:
  case STATE_MAGIC_ITEM:
    status_user_goods(equip_state);
    emit_message("Select ?");
    extend_context(init_enter_buffer(CONTEXT_ENTER_CHARACTER, equip_which));
    break;
    
  default:
    /* 新しいユーザーイメージをロード */
    load_user_image();
    resume_context();
  }
}

void equip_leave(void)
{
}

void equip_what(char *s)
{
  switch (toupper(*s)) {
  case 'W': equip_state = STATE_WEAPON; break;
  case 'M': equip_state = STATE_SCROLL; break;
  case 'A': equip_state = STATE_ARMOUR; break;
  case 'S': equip_state = STATE_SHIELD; break;
  case 'I': equip_state = STATE_MAGIC_ITEM; break;
  default:
    equip_state = STATE_EXIT;
    emit_message("What ?");
  }
}

void equip_which(char *s)
{
  int n = toupper(*s) - 'A';

  if (0 <= n && n < MAX_GOODS) {
    int goods_type = equip_state;
    
    emit_message(goods_data[goods_type][n].name);
    
    /* 在庫ある？ */
    if (user.inventory[goods_type][n].stock > 0) {
      user.inventory[goods_type][n].stock--;

      /* 初めて装備するものに対して初期熟練度を与える */
      if (user.inventory[goods_type][n].skill == 0) {
        user.inventory[goods_type][n].skill = 30;
      }
      user.inventory[goods_type][user.equipment[goods_type]].stock++;
      
      user.equipment[goods_type] = n;
      equip_state = STATE_EXIT;
      return;
    } else {
      /* 在庫ない */
      emit_message("Not owned !");
    }
  }
  equip_state = STATE_EXIT;
}
