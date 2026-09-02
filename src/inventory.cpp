#include "xanadu.h"
#include "inventory.h"

static void inventory_hit_any_key(char *s);
static void inventory_list_goods(void);

#define STATE_EXIT		-1	// すぐさま抜ける

// 表示状態: GOODS_WEAPON..GOODS_MAGICITEM
static int inventory_state;

int init_inventory(void)
{
  inventory_state = GOODS_WEAPON;
  
  // scenario 2
  visual_image = load_image(IMAGE_DIR "/picture/shop.bmp");
  
  emit_message("Hit any key");
  return CONTEXT_INVENTORY;
}

void inventory_enter(void)
{
  switch (inventory_state) {
  case GOODS_WEAPON:
  case GOODS_SCROLL:
  case GOODS_ARMOUR:
  case GOODS_SHIELD:
  case GOODS_MAGIC_ITEM:
    // 品物のリストを表示
    inventory_list_goods();
    extend_context(init_enter_buffer(CONTEXT_ENTER_CHARACTER,
                                     inventory_hit_any_key));
    break;
    
  default:
    resume_context();
  }
}

void inventory_leave(void)
{
}

void inventory_hit_any_key(char *s)
{
  if (*s == 'q' || *s == 'Q')
    inventory_state = STATE_EXIT;
  else
    inventory_state++;
}

void inventory_list_goods(void)
{
  int goods_type = inventory_state;
  int i;
  char buf[256];
  static const char *title[] = {
    "Weapon", "Scroll", "Armor", "Shield", "Item"
  };

  if (goods_type < GOODS_WEAPON || goods_type > GOODS_MAGIC_ITEM)
    return;

  if (visual_image) {
    draw_image(clip_main, 0, 0, visual_image);
  } else {
    fill_image(clip_main, 0, 0, clip_main->getWidth(), clip_main->getHeight(), SDL_::Color::BLACK);
  }

  draw_text(clip_main,
            (360 - strlen(title[goods_type]) * 16) / 2,
            16,
            title[goods_type], SDL_::Color::WHITE);

  for (i = 0; i < MAX_GOODS; i++) {
    sprintf(buf, "%-13s%3d %3d", goods_data[goods_type][i].name,
            user.inventory[goods_type][i].stock,
            user.inventory[goods_type][i].skill);
    
    draw_text(clip_main, 16 + 4, 16 * (i + 3), buf, SDL_::Color::WHITE);
  }
#if 0
  draw_text(&offscreen, &clip_mapview, 5 * 16, 20 * 16,
            "Hit any key", SDL_::Color::WHITE);
#endif
  update(rect_main);
}
