#include "xanadu.h"
#include "user.h"
#include "menu.h"
#include "field.h"
#include "tower.h"
#include "boss.h"
#include "opening.h"
#include "ending.h"

#ifdef __BORLANDC__
#include <dir.h>
#include <dos.h>
#elif __FreeBSD__
#include <sys/types.h>
#include <dirent.h>
#endif

#include <ctype.h>

#define MENU_GENERIC		0
#define MENU_VERSION		1
#define MENU_LOAD		2
#define MENU_DEBUG		3
#define MENU_BOSS		4
#define MENU_GAME		5	/* 新しいゲーム */

#define reenter()		(menu_enter())

#define MAX_USER_ENTRY		8

/* ディレクトリ中のユーザー情報を保持する構造体 */
typedef struct {
  char		name[16];		/* 名前 */
} user_entry_t;

static user_entry_t user_entries[MAX_USER_ENTRY];
static int menu_state;

/* ロゴ */
static image_t *logo_image;

static void menu_draw_text(int row, int col, const char *, pixel_t);
static void menu_draw_item(int row, int col, int key, const char *, pixel_t);

static void generic_menu_key_event(int c);
static void load_menu_key_event(int c);
static void debug_menu_key_event(int c);
static void boss_menu_key_event(int c);
static void version_info_key_event(int c);

static void init_debug(void);
static int init_load_menu(void);

int load_game(int i);

void menu_enter(void)
{
  pixel_t pixel1 = user.environment.scenario == 0 ? red_pixel : white_pixel;
  pixel_t pixel2 = user.environment.scenario != 0 ? red_pixel : white_pixel;

  bgm_play(bgm_data.start_menu); /* BGM */

  fill_image(clip_main, 0, 0, clip_main->width, clip_main->height, black_pixel);
  
  /* ロゴ */
  if (!logo_image) {
    logo_image = load_image(IMAGE_DIR "/picture/logo.bmp");
  }
  if (logo_image) {
    draw_image(clip_main, 100, 290, logo_image);
  }

  switch (menu_state) {
  case MENU_GAME:
    menu_state = MENU_GENERIC;
    switch_context(init_training_ground(user.environment.scenario));
    return;

  case MENU_DEBUG:
    thunk_key_event = debug_menu_key_event;
    init_debug();
    menu_draw_text( 0, 4, "Debug mode", red_pixel);
    menu_draw_item( 2, 0, '+', "SCENARIO 1", pixel1);
    menu_draw_item( 3, 0, '*', "SCENARIO 2", pixel2);
    {
      pixel_t pixels[11];
      int i;
      for (i = 0; i < 11; i++) { pixels[i] = white_pixel; }
      pixels[user.environment.dungeon_level] = red_pixel;
      if (user.environment.scenario == 0) {
        menu_draw_item( 4, 0, '1', "Level 1",  pixels[0]);
        menu_draw_item( 5, 0, '2', "Level 2",  pixels[1]);
        menu_draw_item( 6, 0, '3', "Level 3",  pixels[2]);
        menu_draw_item( 7, 0, '4', "Level 4",  pixels[3]);
        menu_draw_item( 8, 0, '5', "Level 5",  pixels[4]);
        menu_draw_item( 9, 0, '6', "Level 6",  pixels[5]);
        menu_draw_item(10, 0, '7', "Level 7",  pixels[6]);
        menu_draw_item(11, 0, '8', "Level 8",  pixels[7]);
        menu_draw_item(12, 0, '9', "Level 9",  pixels[8]);
        menu_draw_item(13, 0, 'A', "Level 10", pixels[9]);
        menu_draw_item(14, 0, 'B', "Training Ground", pixels[10]);
      } else {
        menu_draw_item( 4, 0, '1', "Maple Ford",  pixels[0]);
        menu_draw_item( 5, 0, '2', "Filane",      pixels[1]);
        menu_draw_item( 6, 0, '3', "Poigone",     pixels[2]);
        menu_draw_item( 7, 0, '4', "Gandic",      pixels[3]);
        menu_draw_item( 8, 0, '5', "Nuldour",     pixels[4]);
        menu_draw_item( 9, 0, '6', "Alf",         pixels[5]);
        menu_draw_item(10, 0, '7', "Alcanek",     pixels[6]);
        menu_draw_item(11, 0, '8', "Altel",       pixels[7]);
        menu_draw_item(12, 0, '9', "Klepsydar",   pixels[8]);
        menu_draw_item(13, 0, 'A', "Rilvan",      pixels[9]);
        menu_draw_item(14, 0, 'B', "Shhangri-La", pixels[10]);
      }
    }
    menu_draw_item(15, 0, 'F', "Enter field", white_pixel);
    menu_draw_item(16, 0, 'T', "Enter tower", white_pixel);
    menu_draw_item(17, 0, 'R', "Return back", red_pixel);
#if 0
    menu_draw_text(19, 0, "Should reload a level", white_pixel);
    menu_draw_text(20, 0, "just switch SCENARIOs", white_pixel);
#endif
    break;
    
  case MENU_LOAD:
    {
      int i, n;
      thunk_key_event = load_menu_key_event;

      menu_draw_text( 0, 4, "Load game", red_pixel);
      n = init_load_menu();
      for (i = 0; i < n; i++) {
        menu_draw_item( 2 + i, 0, 'A' + i, user_entries[i].name, white_pixel);
      }
      menu_draw_item(2 + i, 0, 'R', "Return back", red_pixel);
    }
    break;

  case MENU_BOSS:
    thunk_key_event = boss_menu_key_event;
    init_debug();
    menu_draw_text( 0, 4, "Boss menu", red_pixel);
    if (user.environment.scenario == 0) {
      menu_draw_item( 2, 0, 'A', "Kraken Giant",  white_pixel);
      menu_draw_item( 3, 0, 'B', "Grell Giant",   white_pixel);
      menu_draw_item( 4, 0, 'C', "Karttikeya",    white_pixel);
      menu_draw_item( 5, 0, 'D', "Silver Dragon", white_pixel);
      menu_draw_item( 6, 0, 'E', "Big Kraken",    white_pixel);
      menu_draw_item( 7, 0, 'F', "King Dragon",   white_pixel);
      menu_draw_item( 8, 0, 'R', "Return back",   red_pixel);
    } else {
      menu_draw_item( 2, 0, 'A', "Marivoux",      white_pixel);
      menu_draw_item( 3, 0, 'B', "Peluton",       white_pixel);
      menu_draw_item( 4, 0, 'C', "Great Kraken",  white_pixel);
      menu_draw_item( 5, 0, 'D', "Zschokke",      white_pixel);
      menu_draw_item( 6, 0, 'E', "White Dragon",  white_pixel);
      menu_draw_item( 7, 0, 'F', "Bogres",        white_pixel);
      menu_draw_item( 8, 0, 'G', "Red Dragon",    white_pixel);
      menu_draw_item( 9, 0, 'H', "Guin",          white_pixel);
      menu_draw_item(10, 0, 'I', "Hydra",         white_pixel);
      menu_draw_item(11, 0, 'J', "Buzzati",       white_pixel);
      menu_draw_item(12, 0, 'K', "Boiardo",       white_pixel);
      menu_draw_item(13, 0, 'L', "King Dragon",   white_pixel);
      menu_draw_item(14, 0, 'R', "Return back",   red_pixel);
    }
#if 0
    menu_draw_text(19, 2, "Which do you fight", white_pixel);
    menu_draw_text(20, 4, "a battle with?", white_pixel);
#endif
    break;

  case MENU_VERSION:
    thunk_key_event = version_info_key_event;
    menu_draw_text( 1,  8, "XANADU", white_pixel);
    menu_draw_text( 3,  1, "REVISION:", red_pixel);
    menu_draw_text( 3, 10, "1.1.4", white_pixel);
    menu_draw_text( 4,  1, "  SYSTEM:", red_pixel);
#ifdef __WIN32__
    menu_draw_text( 4, 10, "Win32", white_pixel);
#else
    menu_draw_text( 4, 10, "X11R6", white_pixel);
#endif
    menu_draw_text( 5,  1, " DISPLAY:", red_pixel);
    {
      char buf[16];
      sprintf(buf, "%dbpp", graphic_methods.bits_per_pixel);
      menu_draw_text(5, 10, buf, white_pixel);
    }
    menu_draw_text( 6,  1, "     BGM:", red_pixel);
    menu_draw_text( 6, 10, bgm_enabled() ? "OK" : "Disable", white_pixel);
    menu_draw_text( 7,  1, "     S.E:", red_pixel);
    menu_draw_text( 7, 10, se_enabled() ? "OK" : "Disable", white_pixel);

    /*menu_draw_text( 9,  0, "-NOTICE-", white_pixel);*/
    menu_draw_text(10,  0, "XANADU WAS ORIGINALLY", white_pixel);
    menu_draw_text(11,  0, "RELEASED IN 1985", white_pixel);
    menu_draw_text(12,  0, "BY FALCOM.", white_pixel);
    
    menu_draw_item(14,  0, 'R', "Return back", red_pixel);
    break;
    
  case MENU_GENERIC:
  default:
    thunk_key_event = generic_menu_key_event;
    menu_draw_text( 0, 4, "Start menu", red_pixel);
    menu_draw_item( 2, 0, 'L', "Load game",  white_pixel);
    menu_draw_item( 3, 0, '1', "SCENARIO 1", pixel1);
    menu_draw_item( 4, 0, '2', "SCENARIO 2", pixel2);
    menu_draw_item( 5, 0, 'N', "New game",   white_pixel);
    menu_draw_item( 6, 0, 'D', "Debug mode", white_pixel);
    menu_draw_item( 7, 0, 'B', "Boss stage", white_pixel);
    menu_draw_item( 8, 0, 'O', "Opening", white_pixel);
    menu_draw_item( 9, 0, 'E', "Ending(LONG)", white_pixel);
    menu_draw_item(10, 0, 'V', "Version info", white_pixel);
    menu_draw_text(12, 0, "Please Num-Lock *OFF*", red_pixel);
  }
  update(rect_main);
}

void menu_leave(void)
{
  thunk_key_event = NULL;
}

void menu_draw_text(int row, int col, const char *s, pixel_t pixel)
{
  draw_text(clip_main, col * 16, row * 16, s, pixel);
}

void menu_draw_item(int row, int col, int key, const char *s, pixel_t pixel)
{
  char buf[3] = "*:";
  buf[0] = key;
  draw_text(clip_main, col * 16, row * 16, buf, red_pixel);
  col += 2;
  draw_text(clip_main, col * 16, row * 16, s, pixel);
}

void generic_menu_key_event(int c)
{
  c = toupper(c);
  switch (c) {
  case 'L': menu_state = MENU_LOAD; reenter(); break;
  case '1': user.environment.scenario = 0; reenter(); break;
  case '2': user.environment.scenario = 1; reenter(); break;
  case 'N':
    menu_state = MENU_GAME;
    extend_context(init_opening());
    return;
  case 'D': menu_state = MENU_DEBUG; reenter(); break;
  case 'B': menu_state = MENU_BOSS; reenter(); break;
  case 'O': extend_context(init_opening()); break;
  case 'E': extend_context(init_ending());  break;
  case 'V': menu_state = MENU_VERSION; reenter(); break;
  default: return;
  }
  update(rect_main);
}

void debug_menu_key_event(int c)
{
  c = toupper(c);
  switch (c) {  
  case '+': case ';': user.environment.scenario = 0; break;
  case '*': case ':': user.environment.scenario = 1; break;
  case '1': user.environment.dungeon_level =  0; break;
  case '2': user.environment.dungeon_level =  1; break;
  case '3': user.environment.dungeon_level =  2; break;
  case '4': user.environment.dungeon_level =  3; break;
  case '5': user.environment.dungeon_level =  4; break;
  case '6': user.environment.dungeon_level =  5; break;
  case '7': user.environment.dungeon_level =  6; break;
  case '8': user.environment.dungeon_level =  7; break;
  case '9': user.environment.dungeon_level =  8; break;
  case 'A': user.environment.dungeon_level =  9; break;
  case 'B': user.environment.dungeon_level = 10; break;
  case 'F':
    init_level(user.environment.dungeon_level, NULL);
    switch_context(CONTEXT_FIELD);
    return;
  case 'T':
    init_level(user.environment.dungeon_level, NULL);
    user.x = 0;
    user.y = 4 * 40;
    switch_context(CONTEXT_TOWER);
    return;
  case 'O':
    init_level(-1, NULL);
    user.point = field_offset_XY(4, 3);
    switch_context(CONTEXT_FIELD);
    return;
  case 'R':
    menu_state = MENU_GENERIC;
    break;
  }
  reenter();
}

void load_menu_key_event(int c)
{
  c = toupper(c);
  switch (c) {
  case 'A': case 'B': case 'C': case 'D':
  case 'E': case 'F': case 'G': case 'H':
    if (load_game(c - 'A') == 0) {
      /* ゲームを再開する */
      load_user_image();
      init_level(user.environment.dungeon_level, user_path);
      if (in_tower())
        switch_context(init_tower());
      else
        switch_context(init_field());
    }
    break;
  case 'R': menu_state = MENU_GENERIC; reenter(); break;
  }    
}

void init_debug(void)
{
  int i;
  
  free((void *)user_path);
  user_path = NULL;

  user.environment.in_battle = 0;

  strcpy(user.status.name, "Nobody");
  user.status.max_HP = 6000000;
  user.status.HP     = 6000000;
  user.status.gold   = 6000000;
  user.status.food   = 1000000;
    
  for (i = 0; i < MAX_GOODS; i++) {
    user.inventory[GOODS_WEAPON][i].stock = 1;
    user.inventory[GOODS_WEAPON][i].skill = 255;
    user.inventory[GOODS_SCROLL][i].stock = 1;
    user.inventory[GOODS_SCROLL][i].skill = 255;
    user.inventory[GOODS_ARMOUR][i].stock = 1;
    user.inventory[GOODS_ARMOUR][i].skill = 200;
    user.inventory[GOODS_SHIELD][i].stock = 1;
    user.inventory[GOODS_SHIELD][i].skill = 200;
    user.inventory[GOODS_MAGIC_ITEM][i].stock = 255;
    user.inventory[GOODS_MAGIC_ITEM][i].skill = 0;
  }
  user.inventory[GOODS_MAGIC_ITEM][1].skill = 255;
  user.inventory[GOODS_MAGIC_ITEM]
    [user.equipment[GOODS_MAGIC_ITEM]].skill = 30;
  
  user.status.STR = 100;
  user.status.INT = 100;
  user.status.WIS =  50;
  user.status.DEX = 100;
  user.status.AGL = 100;
  user.status.CHR = 100;
  user.status.MGR =  95;
  user.status.KEY = 200;
  user.status.ELX = 100;
  user.status.CRN =   4;
  user.status.KRM =   0;

  user.equipment[GOODS_WEAPON] = 15;
  user.equipment[GOODS_SCROLL] = 15;
  user.equipment[GOODS_ARMOUR] = 16;
  user.equipment[GOODS_SHIELD] = 16;

  user.status.fighter.rank = 15;
  user.status.wizard .rank = 15;

  user.environment.lighting = 255;
  user.environment.in_training_ground = 0;

  user.point = 0;
  user.x = 0;
  user.y = 0;

  match_user_name("Debugger");
}

int init_load_menu(void)
{
  int i, n = 0;

#ifdef __BORLANDC__
  int done;
  struct ffblk ffblk;

  done = findfirst(USERS_DIR "/*", &ffblk, FA_DIREC);
  while (!done) {
    if (ffblk.ff_attrib == FA_DIREC &&
        strcmp(ffblk.ff_name, "." ) != 0 &&
        strcmp(ffblk.ff_name, "..") != 0 &&
        strlen(ffblk.ff_name) < sizeof(user_entries[n].name) - 1) {
      strcpy(user_entries[n].name, ffblk.ff_name);
      n++;
    }
    done = findnext(&ffblk);
  }
#elif __FreeBSD__
  DIR *dir;
  struct dirent *dirent;

  dir = opendir(USERS_DIR);
  if (!dir) {
    perror(USERS_DIR);
    return 0;
  }
  while ((dirent = readdir(dir)) != NULL) {
    if (dirent->d_type == DT_DIR &&
        strcmp(dirent->d_name, "." ) != 0 &&
        strcmp(dirent->d_name, "..") != 0 &&
        strlen(dirent->d_name) < sizeof(user_entries[n].name) - 1) {
      strcpy(user_entries[n].name, dirent->d_name);
      n++;
    }
  }
#endif
  for (i = n; i < MAX_USER_ENTRY; i++) {
    memset(&user_entries[i], 0, sizeof(user_entries[i]));
  }
  return n;
}

int load_game(int i)
{
  if (0 <= i && i < MAX_USER_ENTRY && strlen(user_entries[i].name) > 0) {
    char path[BUFSIZ];

    sprintf(path, "%s/%s", USERS_DIR, user_entries[i].name);
    free((void *)user_path);
    user_path = strdup(path);

    if (load_user()) {
      /* 失敗 */
      emit_error("Can't load user.dat!");
      free((void *)user_path);
      user_path = NULL;
      goto failure;
    } else
      return 0; /* 成功 */
  }
failure:
  return 1;
}

void boss_menu_key_event(int c)
{
  int boss_id;
  switch (toupper(c)) {
  case 'A': boss_id =  0; break;
  case 'B': boss_id =  1; break;
  case 'C': boss_id =  2; break;
  case 'D': boss_id =  3; break;
  case 'E': boss_id =  4; break;
  case 'F': boss_id =  5; break;
  case 'G': boss_id =  6; break;
  case 'H': boss_id =  7; break;
  case 'I': boss_id =  8; break;
  case 'J': boss_id =  9; break;
  case 'K': boss_id = 10; break;
  case 'L': boss_id = 11; break;
  case 'R': menu_state = MENU_GENERIC; reenter(); /* return */
  default: return;
  }
  extend_context(init_boss(boss_id));
}

void version_info_key_event(int c)
{
  switch (toupper(c)) {
  case 'R': menu_state = MENU_GENERIC; reenter(); return;
  }
}