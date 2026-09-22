#include "xanadu.h"
#include "field.h"
#include "battle.h"
#include "status.h"
#include "use_item.h"
#include "animation.h"
#include "resources/Resources.h"
#include "resources/SoundId.h"
#include "app/Application.h"

#define STATE_USE		0	// 初期状態
#define STATE_CONTINUE		1	// 継続
#define STATE_WARPED		2	// ワープ完了
#define STATE_EXIT_SUCCESS	-1	// 成功
#define STATE_EXIT_FAILURE	-2	// 失敗

// タイマーインターバル
#define INTERVAL_SPECTACLES	100
#define INTERVAL_BALANCE	100

// ワープフレーム
#define MAX_WARP_FRAME		16

// 画面更新関連
static void (*thunk_update_background)(void);

// アイテム全般
static room_t *use_item_room;
static int use_item_state;
static void restore_context(int comsumed);
static void use_item_no_response(void);
#if 0
static void use_item_wait(void);
static void use_item_flash(void);
#endif

// スペクタクルズ関連
static void use_item_spectacles(void);
static int spectacles_dir;
static int spectacles_x;
static int spectacles_y;
static void loop_spectacles_in_field(void);
static void inspect_monster_status(int monster_id);

// 特定のアイテム関連
static void use_item_healing(int skill);
static void use_item_ignited(void);
static void use_item_mattock(void);
static void use_item_balance(void);
static void loop_balance(void);
static void use_item_pendant(void);
static void use_item_silver_rose(void);
static void use_item_acid(void);
static void use_item_ladder(void);

// 持続する効果があるもの
static void use_item_continuance(int effect_id, int skill);
static void use_item_doping(int effect_id, int skill);
static void use_item_metamorphosis(int effect_id, int skill);

// レベル間ワープ
static animation_frame_t warp_frames[MAX_WARP_FRAME];
static void use_item_warp_level(int up_down);
static void use_item_past_level(void);

extern const point_t room_door_position[4]; // battle.c
extern const point_t move_table[10]; // field.c

// 反応メッセージ
static const char *use_item_response[MAX_ITEM_TYPE] = {
  "Examine Enemy", "Healing", "Ignite Lamp", "Warp-Up", "Warp-Down",
  "Dig ground", "Negate time", "Can fly", "Pass Wall", "Fade you",
  "Open box", "Unlock door", "Transform", "STR-up", "INT-up", "AGL-up",
  "CHR-up",
  // scenario 2
  "Make Wall", "Unlock door", "Melted wall", "Put ladder", "CHR-up"
};

int init_use_item(void (*update_background)(void), room_t *room)
{
  thunk_update_background = update_background;
  use_item_room = room;
  use_item_state = STATE_USE;
  return CONTEXT_USE;
}

namespace
{
constexpr SoundId kUseItemSoundIds[] = { SoundId::invoke, SoundId::treasure, SoundId::lost_key, SoundId::get };
}

void use_item_create(void)
{
  auto &res = Resources::instance();
  auto &mixer = Application::instance().getMixer();
  for (SoundId id : kUseItemSoundIds) {
    res.loadSound(mixer, id);
  }
}

void use_item_destroy(void)
{
  auto &res = Resources::instance();
  for (SoundId id : kUseItemSoundIds) {
    res.unloadSound(id);
  }
}

void use_item_enter(void)
{
  if (use_item_state == STATE_USE || use_item_state == STATE_CONTINUE) {
    
    int item_type = goods_data[GOODS_MAGIC_ITEM]
                              [user.equipment[GOODS_MAGIC_ITEM]].type;
    int skill = user.inventory[GOODS_MAGIC_ITEM]
                              [user.equipment[GOODS_MAGIC_ITEM]].skill;
    
    if (0 <= item_type && item_type < MAX_ITEM_TYPE) {
      // 反応メッセージを表示
      emit_message(use_item_response[item_type]);
    }
    
    playSound(SoundId::invoke); // se
    
    switch (item_type) {
    case ITEM_SPECTACLES:   use_item_spectacles(); break;
    case ITEM_RED_POTION:   use_item_healing(skill); break;
    case ITEM_LAMP:         use_item_ignited(); break;
    case ITEM_BLACK_ONYX:   use_item_warp_level(+1); break;
    case ITEM_FIRE_CRYSTAL: use_item_warp_level(-1); break;
    case ITEM_MATTOCK:      use_item_mattock(); break;
    case ITEM_HOURGLASS:
      use_item_continuance(EFFECT_HOURGLASS, skill);
      break;
    case ITEM_WINGED_BOOTS:
      if (use_item_room == NULL) {
        use_item_continuance(EFFECT_WINGED_BOOTS, skill);
      } else {
        use_item_no_response();
        restore_context(0);
      }
      break;
    case ITEM_MANTLE:
      use_item_continuance(EFFECT_MANTLE, skill);
      break;
    case ITEM_DEMONS_RING:
      use_item_metamorphosis(EFFECT_DEMONS_RING, skill);
      break;
    case ITEM_BALANCE:      use_item_balance(); break;
    case ITEM_PENDANT:      use_item_pendant(); break;
    case ITEM_CANDLE:
      use_item_metamorphosis(EFFECT_CANDLE, skill);
      break;
    case ITEM_RUBY:
      use_item_doping(EFFECT_RUBY, skill);
      break;
    case ITEM_BROWN_POTION:
      use_item_doping(EFFECT_BROWN_POTION, skill);
      break;
    case ITEM_MIRROR:       use_item_doping(EFFECT_MIRROR, skill); break;
    case ITEM_BOTTLE:       use_item_doping(EFFECT_BOTTLE, skill); break;
      // scenario 2
    case ITEM_SILVER_ROSE:  use_item_silver_rose(); break;
    case ITEM_KEY:          use_item_pendant(); break;
    case ITEM_ACID:         use_item_acid(); break;
    case ITEM_LADDER:       use_item_ladder(); break;
    case ITEM_CROSS:        use_item_doping(EFFECT_BOTTLE, skill); break;
    default:
      resume_context();
    }
  } else if (use_item_state == STATE_WARPED) {
    // ワープ先レベルでの処理
    use_item_past_level();
  } else {
    restore_context(use_item_state == STATE_EXIT_SUCCESS);
  }
}

void use_item_leave(void)
{
  kill_timer();
}

// 以前のコンテキストを復元する
void restore_context(int consumed)
{
  if (consumed) {
    int magic_item = user.equipment[GOODS_MAGIC_ITEM];
    int *stock = &user.inventory[GOODS_MAGIC_ITEM][magic_item].stock;
    int *skill = &user.inventory[GOODS_MAGIC_ITEM][magic_item].skill;

    // 熟練度アップ
    *skill = min(*skill + 10, 255);

    // 消費・充填
    if (*stock > 0) {
      (*stock)--;
    } else {
      // 充填できない
      user.equipment[GOODS_MAGIC_ITEM] = GOODS_NULL_MAGIC_ITEM;
      status_update_equipment();
    }
  }
  user_hidden = 0; // ユーザーを見えるようにする

#ifdef NO_PAUSE
  resume_context();
#else
  switch_context(init_pause(50, SDL_SCANCODE_RETURN));
#endif
}

void use_item_no_response(void)
{
  emit_message("No response");
}

// スペクタクルズ
void use_item_spectacles(void)
{
  if (use_item_room != NULL) {
    int i, consumed = 0;
    
    // 部屋の中にモンスターがいるかどうか？
    for (i = 0; i < MAX_MEMBER; i++) {
      if (member_monster(&use_item_room->members[i])) {
        consumed = 1;
        break;
      }
    }
    if (consumed)
      inspect_monster_status(use_item_room->monster_id);
    else
      use_item_no_response();
    
    restore_context(consumed);
  } else {
    switch (user.dir) {
    case 1:  spectacles_dir = 4;
    case 3:  spectacles_dir = 6;
    case 7:  spectacles_dir = 4;
    case 9:  spectacles_dir = 6;
    default: spectacles_dir = user.dir;
    }
    spectacles_x = user.x;
    spectacles_y = user.y;
    set_timer(INTERVAL_SPECTACLES, loop_spectacles_in_field);
  }
}

// フィールドにおけるスペクタクルズビーム
void loop_spectacles_in_field(void)
{
  int dir = spectacles_dir;

  if (isKeyDown(SDL_SCANCODE_DOWN))  dir = 2;
  if (isKeyDown(SDL_SCANCODE_LEFT))  dir = 4;
  if (isKeyDown(SDL_SCANCODE_RIGHT)) dir = 6;
  if (isKeyDown(SDL_SCANCODE_UP))    dir = 8;

  switch (spectacles_dir) {
  case 2:
  case 8:
    if (dir == 2 || dir == 8 || spectacles_y % 40 == 0)
      spectacles_dir = dir;
    break;
  case 4:
  case 6:
    if (dir == 4 || dir == 6 || spectacles_x % 40  == 0)
      spectacles_dir = dir;
    break;
  default:
    spectacles_dir = dir;
  }
  spectacles_x += move_table[spectacles_dir].x * 20;
  spectacles_y += move_table[spectacles_dir].y * 20;

  if (spectacles_x <= -40  || 360 <= spectacles_x ||
      spectacles_y <= -40 || 360 <= spectacles_y) {
    // アウトオブバウンズ
    use_item_no_response();
    restore_context(0);
    return;
  }

  // マス目にきっちり合ったときに、その場所にいるモンスターを調べる
  if (spectacles_x % 40  == 0 &&
      spectacles_y % 40 == 0) {
    int i, point;
    tomb_t *tm;

    point = user.point
      + field_offset_XY(spectacles_x / 40  - 4,
                        spectacles_y / 40 - 4);
    
    for (i = 0, tm = &level_data.tombs[i]; i < MAX_TOMB; i++, tm++)
      if (point == tm->point && tm->num_members > 0) {
        inspect_monster_status(tm->monster_id);
        restore_context(1);
        return;
      }
  }
  (*thunk_update_background)();
  inverse_image(clip_main, spectacles_x, spectacles_y, mask_damaged);
}

// モンスターのステータスを表示する
void inspect_monster_status(int monster_id)
{
  monster_status_t *mo;
  
  if (monster_id < 0 || MAX_MONSTER * MAX_VARIETY <= monster_id) {
    // おそらくバグ
    use_item_no_response();
    return;
  }
  
  mo = &monster_data[monster_id];
  
  emit_message(mo->name);
  format_message("HP-%d", mo->max_HP * 100);
  if (mo->INT == 0) {
    format_message("STR-%d", monster_attack_point(mo));
  } else {
    emit_message(goods_data[GOODS_SCROLL][mo->magic].name);
  }
  format_message("DEF-%d", monster_defend_point(mo, GUARD_FRONT));
  switch (mo->goods) {
  case GOODS_GOLD: emit_message("Golds"); break;
  case GOODS_FOOD: emit_message("Foods"); break;
  default:
    {
      int goods_type = mo->goods / GOODS_FACTOR;
      int goods_numb = mo->goods % GOODS_FACTOR;
      emit_message(goods_data[goods_type][goods_numb].name);
    }
  }
}

// 回復薬
void use_item_healing(int skill)
{
  int point = (user_WIS() * skill / 10000.0) * user.status.max_HP;
  user.status.HP = min(user.status.HP + point, user.status.max_HP);
  status_update_HP(SDL_::Color::WHITE);
  restore_context(1);
}

// ランプ
void use_item_ignited(void)
{
  user.environment.lighting++;
  restore_context(1);
}

// マトック
void use_item_mattock(void)
{
  if (use_item_room == NULL) {
    int point = user.point + FIELD_WIDTH;
  
    switch (user.dir) {
    case 1: case 4: case 7: point--; break;
    case 3: case 6: case 9: point++; break;
    }

    if (0 <= point && point < FIELD_SIZE &&
        (tile_data.flags[level_data.field[point]] & TILE_WALL_DIG) != 0) {
      // 掘る
      int top = user.point + user_sight_XY();
      int x = (point - top) % FIELD_WIDTH * 40;
      int y = (point - top) / FIELD_WIDTH * 40;
      
      level_data.field[point] = tile_data.pattern1;
      
      use_item_state = STATE_EXIT_SUCCESS; // 成功裏に抜ける
      extend_context(init_animation_tile(tile_data.digging, 3, x, y,
                                         thunk_update_background));
      return;
    }
  }
  use_item_no_response();
  restore_context(0);
}

// バランス
void use_item_balance(void)
{
  if (use_item_room != NULL) {
    member_t *mm;
    int i;
    
    // 宝箱があるかどうか？
    for (i = 0, mm = use_item_room->members; i < MAX_MEMBER; i++, mm++) {
      if (mm->type == MEMBER_BOX) {
        set_timer(INTERVAL_BALANCE, loop_balance);
        return;
      }
    }
  }
  use_item_no_response();
  restore_context(0);
}

void loop_balance(void)
{
  member_t *mm;
  int i, something_opening = 0;

  for (i = 0, mm = use_item_room->members; i < MAX_MEMBER; i++, mm++) {
    if (mm->type == MEMBER_BOX) {
      if (--mm->frame >= 0) {
        if (!something_opening) {
          // 宝箱を開ける音はget.wavを使う
          playSound(SoundId::get);
        }
      } else {
        int goods = monster_data[use_item_room->monster_id].goods;
        
        if (!something_opening) {
          playSound(SoundId::treasure);
        }
        mm->type = MEMBER_GOODS;
        if (mm->value == 1) {
          // 赤箱
          mm->value = goods;
        } else {
          // 白箱
          mm->value = goods == GOODS_FOOD ? GOODS_FOOD : GOODS_GOLD;
        }
        mm->frame = index_goods[mm->value];
      }
      something_opening = 1;
    }
  }
  
  (*thunk_update_background)();
  
  if (!something_opening) {
    // すべて開いた
    restore_context(1);
  } 
}

// ペンダント
void use_item_pendant(void)
{
  if (use_item_room == NULL) {
    int point[2];
    int i, map;

    // ユーザーの前後
    if (user.dir == 0 || user.dir == 2 ||
        user.dir == 5 || user.dir == 8) {
      point[0] = user.point - FIELD_WIDTH;
      point[1] = user.point + FIELD_WIDTH;
    } else {
      point[0] = user.point - 1;
      point[1] = user.point + 1;
    }

    for (i = 0; i < 2; i++) {
      int top = user.point + user_sight_XY();
      int x = (point[i] - top) % FIELD_WIDTH * 40;
      int y = (point[i] - top) / FIELD_WIDTH * 40;

      map = point_map(point[i]);

      if (map == tile_data.locked) {
        // 扉を開ける
        level_data.field[point[i]] = point[i] != 0
          ? tile_data.pattern0
          : tile_data.pattern1;
        
        use_item_state = STATE_CONTINUE;
        playSound(SoundId::lost_key);
        extend_context(init_animation_tile(tile_data.field_open, 3, x, y,
                                           thunk_update_background));
        return;
      }
    }

    // ユーザーの場所: シナリオ1
    if (point_map(user.point) == tile_data.cave_closed &&
        !in_scenario2()) {
      
      // 洞窟の扉を開ける
      level_data.field[user.point] = tile_data.cave_next;
      
      use_item_state = STATE_CONTINUE;
      playSound(SoundId::lost_key);
    }
  } else {
    // たぶんタワー内部
    int i;

    for (i = 0; i < 4; i++) {
      if (use_item_room->barrier[i] == BARRIER_LOCK) {
        int x, y;
          
        // 扉を開ける
        use_item_room->barrier[i] = BARRIER_OPEN;
        
        // 戦闘マップの更新
        replace_battle_map(room_door_position[i].x,
                           room_door_position[i].y, tile_data.floor);

        use_item_state = STATE_CONTINUE;
        playSound(SoundId::lost_key);
        x = room_door_position[i].x * 40;
        y = room_door_position[i].y * 40;
        extend_context(init_animation_tile(tile_data.tower_open, 3, x, y,
                                           thunk_update_background));
        return;
      }
    }
  }
  // 一回以上ここを通った？
  if (use_item_state != STATE_CONTINUE) {
    use_item_no_response();
    restore_context(0);
  } else
    restore_context(1);
}

// 持続時間
static int use_item_time_period(int skill)
{
  return user_WIS() * skill / 100;
}

// 砂時計・羽飾りつきブーツ・外套
void use_item_continuance(int effect_id, int skill)
{
  if (user.environment.effect[effect_id] < 255) {
    int second = use_item_time_period(skill);
    second = min(255, second + user.environment.effect[effect_id]);
    user.environment.effect[effect_id] = second;
    format_message("%d time", second);
    restore_context(1);
  } else {
    use_item_no_response();
    restore_context(0);
  }
}

// ルビー・茶色薬・かがみ・つぼ
void use_item_doping(int effect_id, int skill)
{
  if (user.environment.effect[effect_id] < 255) {
    int second = use_item_time_period(skill);
    second = min(255, second + user.environment.effect[effect_id]);
    user.environment.effect[effect_id] = second;
    format_message("%d time", second);
    restore_context(1);
  } else {
    use_item_no_response();
    restore_context(0);
  }
}

// 変態
void use_item_metamorphosis(int effect_id, int skill)
{
  int second = use_item_time_period(skill);
  user.environment.effect[effect_id] += second;
  format_message("%d time", second);
  load_user_image();
  restore_context(1);
}

// レベル間ワープ
void use_item_warp_level(int up_down)
{
  if (use_item_room != NULL) {
    use_item_no_response();
    restore_context(0);
  } else {
    int max_dungeon_level;
    int i;
    int to_level = user.environment.dungeon_level + up_down;

    for (i = 0; i < MAX_WARP_FRAME; i++) {
      warp_frames[i].image = i % 2 == 0 ? SDL_::SubImage() : frame_user[user.frame];
      warp_frames[i].x = user.x;
      warp_frames[i].y = user.y;
    }

    // ユーザーデータの保存
    save_user();

    // scenario 2
    max_dungeon_level = 10;
    
    if(0 <= to_level && to_level < max_dungeon_level) {
      user.environment.dungeon_level = to_level;
      use_item_state = STATE_WARPED;
    } else
      use_item_state = STATE_EXIT_FAILURE;
    
    user_hidden = 1; // ユーザーを見えなくする
    extend_context(init_animation(clip_main, warp_frames, MAX_WARP_FRAME,
                                  thunk_update_background));
  }
}

void use_item_past_level(void)
{
  init_level(user.environment.dungeon_level, user_path.empty() ? nullptr : user_path.c_str());
  use_item_state = STATE_EXIT_SUCCESS;

  format_message("Level %d", user.environment.dungeon_level + 1);
  
  // ワープ完了
  extend_context(init_animation(clip_main, warp_frames, MAX_WARP_FRAME,
                                thunk_update_background));
}

void use_item_silver_rose(void)
{
  int dx = move_table[user.dir].x;
  int dy = move_table[user.dir].y;
  short making[3];

  // 「掘る」の逆順
  making[0] = tile_data.digging[2];
  making[1] = tile_data.digging[1];
  making[2] = tile_data.digging[0];
  
  // フィールド？
  if (use_item_room == NULL) {
    int point = user.point + field_offset_XY(dx, dy);

    // ユーザーの向いている方向に人面石を作る
    if (0 <= point && point < FIELD_SIZE) {
      level_data.field[point] = tile_data.stone;

      use_item_state = STATE_EXIT_SUCCESS; // 成功裏に抜ける
      extend_context(init_animation_tile(making, 3,
                                         user.x + dx * 40,
                                         user.y + dy * 40,
                                         thunk_update_background));
      return;
    }
  } else {
    // 部屋の中
    int x = dx + (user.x / 40);
    int y = dy + (user.y / 40);

    // 戦場の更新
    if (replace_battle_map(x, y, tile_data.stone)) {
      // 成功
      int i;
      for (i = 0; i < 4; i++) {
        if (room_door_position[i].x == x &&
            room_door_position[i].y == y) {
          use_item_room->barrier[i] = BARRIER_OPEN;
          break;
        }
      }
      use_item_state = STATE_EXIT_SUCCESS; // 成功裏に抜ける
      extend_context(init_animation_tile(making, 3, x * 40, y * 40,
                                         thunk_update_background));
      return;
    }
  }
  use_item_no_response();
  restore_context(0);
}

void use_item_acid(void)
{
  // タワー内部？
  if (in_tower() && use_item_room != NULL) {
    int i, done;
    
    // scenario 2: 最終レベルでは使えない
    if (in_scenario2() && user.environment.dungeon_level == 10) {
      goto failure;
    }

    for (i = 0, done = 0; i < 4; i++) {
      // 壁に穴を開ける
      if (use_item_room->barrier[i] == BARRIER_WALL) {
        use_item_room->barrier[i] = BARRIER_OPEN;
        // 戦場マップの更新
        replace_battle_map(room_door_position[i].x,
                           room_door_position[i].y, tile_data.floor);
        done = 1;
      }
    }
    if (done) {
      restore_context(1);
      return;
    }
  }
failure:
  use_item_no_response();
  restore_context(0);
}

void use_item_ladder(void)
{
  // フィールド？
  if (use_item_room == NULL) {
    int dx = move_table[user.dir].x;
    int dy = move_table[user.dir].y;
    int point = user.point + field_offset_XY(dx, dy);

    // ユーザーの向いている方向にはしごを作る
    if (0 <= point && point < FIELD_SIZE &&
        (level_data.field[point] == tile_data.pattern0 ||
         level_data.field[point] == tile_data.pattern1)) {
      level_data.field[point] = tile_data.ladder;
      restore_context(1);
      return;
    }
  }
  use_item_no_response();
  restore_context(0);
}