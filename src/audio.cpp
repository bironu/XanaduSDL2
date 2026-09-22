#include "xanadu.h"
#include <ctype.h>

#define number_of(m)	(sizeof(m)/sizeof(m[0]))

se_data_t se_data;
bgm_data_t bgm_data;

// デフォルトの BGM
static char *default_field_bgm[2];
static char *default_tower_bgm[2];
static char *default_boss_bgm[2];

static void set_bgm_data(const char *symbol, const char *filename);
static void set_se_data(const char *symbol, const char *filename);

// 行を分割(0: 解析できた、1: 解析できなかった)
static int parse_line(char *p, char **symbol, char **filename)
{
  // コメント行？
  if (*p == '#')
    return 1;

  // 空白を読み飛ばす
  while (isspace(*p)) p++;

  // シンボルを区切る
  *symbol = p;
  while (isdigit(*p) || isalpha(*p) || *p == '_') p++;

  // コロンでない？
  if (*p != ':') {
    return 1;
  } else {
    *p++ = '\0';
  }
  while (isspace(*p)) p++;

  // ファイル名を区切る
  *filename = p;
  while (*p != '\0' && !isspace(*p)) p++;
  *p = '\0';

  return 0;
}

int init_bgm(void)
{
  FILE *fp;
  int i, scenario;
  char buf[BUFSIZ];

  fp = fopen(AUDIO_DIR "/midi/midi.txt", "rt");
  if (fp == NULL)
    return 1;

  while (fgets(buf, sizeof(buf), fp) != NULL) {
    char *symbol, *filename;

    // 行を分解
    if (parse_line(buf, &symbol, &filename) == 0) {
      set_bgm_data(symbol, filename);
    }
  }
  fclose(fp);

  // このフェイズが終了した後は、文字列を破棄するのは危険
  for (scenario = 0; scenario < 2; scenario++) {
    dungeon_bgm_t *dungeon_bgm = &bgm_data.dungeon[scenario];

    for (i = 0; i < MAX_DUNGEON_LEVEL; i++) {
      // デフォルトの指定を反映する
      if (dungeon_bgm->field[i] == NULL) {
        dungeon_bgm->field[i] = default_field_bgm[scenario];
      }
      if (dungeon_bgm->tower[i] == NULL) {
        dungeon_bgm->tower[i] = default_tower_bgm[scenario];
      }
      // 未指定のタワーの音楽をそのフィールド(もしあれば)と同じにする
      if (dungeon_bgm->field[i] != NULL &&
          dungeon_bgm->tower[i] == NULL) {
        dungeon_bgm->tower[i] = dungeon_bgm->field[i];
      }
    }

    // ボスステージ
    for (i = 0; i < 16; i++) {
      if (dungeon_bgm->boss[i] == NULL) {
        dungeon_bgm->boss[i] = default_boss_bgm[scenario];
      }
    }
  }
  // ショップ
  for (i = 0; i < 32; i++) {
    if (bgm_data.shop[i] == NULL) {
      bgm_data.shop[i] = bgm_data.default_shop;
    }
  }
  return 0;
}

int init_se(void)
{
  FILE *fp;
  char buf[BUFSIZ];

  fp = fopen(AUDIO_DIR "/wave/wave.txt", "rt");
  if (fp == NULL) {
    perror(AUDIO_DIR "/wave/wave.txt");
    return 1;
  }

  while (fgets(buf, sizeof(buf), fp) != NULL) {
    char *symbol, *filename;

    // 行を分解
    if (parse_line(buf, &symbol, &filename) == 0) {
      set_se_data(symbol, filename);
    }
  }
  
  fclose(fp);
  return 0;
}

// BGM データを登録
void set_bgm_data(const char *symbol, const char *filename)
{
  // シンボル照合表
  static struct {
    const char *	symbol;
    char **		value;
  } symbol_table[] = {
    { "TRAINING_GROUND",	&bgm_data.dungeon[0].field[10] },
    { "START_MENU",		&bgm_data.start_menu	},
    { "DEFAULT_SHOP",		&bgm_data.default_shop	},
    { "WEAPON",			&bgm_data.shop[0]	},
    { "SCROLL",			&bgm_data.shop[1]	},
    { "ARMORY",			&bgm_data.shop[2]	},
    { "SHIELD",			&bgm_data.shop[3]	},
    { "ITEM",			&bgm_data.shop[4]	},
    { "GUILDS",			&bgm_data.shop[5]	},
    { "FOODS",			&bgm_data.shop[6]	},
    { "INN",			&bgm_data.shop[7]	},
    { "HEALERS",		&bgm_data.shop[8]	},
    { "TEMPLE",			&bgm_data.shop[9]	},
    { "XA2_HEALERS",		&bgm_data.extra[BGM_EXTRA_XA2_HEALERS] },
    { "XA2_TEMPLE",		&bgm_data.extra[BGM_EXTRA_XA2_TEMPLE] }
  };
  dungeon_bgm_t *dungeon_bgm;
  theme_song_t *theme_song;
  char **default_field;
  char **default_tower;
  char **default_boss;
  int i, scenario = 0;

  // シンボル表を照会する
  for (i = 0; i < number_of(symbol_table); i++) {
    if (stricmp(symbol, symbol_table[i].symbol) == 0) {
      free(*symbol_table[i].value);
      *symbol_table[i].value = strdup(filename);
    }
  }

  if (strnicmp(symbol, "XA1_", 4) == 0) {
    symbol += 4;
  } else
  if (strnicmp(symbol, "XA2_", 4) == 0) {
    symbol += 4;
    scenario = 1;
  }

  dungeon_bgm = &bgm_data.dungeon[scenario];
  theme_song = &bgm_data.theme[scenario];
  default_field = &default_field_bgm[scenario];
  default_tower = &default_tower_bgm[scenario];
  default_boss  = &default_boss_bgm[scenario];

  if (stricmp(symbol, "MAIN") == 0) {
    free(theme_song->main);
    theme_song->main = strdup(filename);
  }
  else if (stricmp(symbol, "OPENING") == 0) {
    free(theme_song->opening);
    theme_song->opening = strdup(filename);
  }
  else if (stricmp(symbol, "ENDING") == 0) {
    free(theme_song->ending);
    theme_song->ending = strdup(filename);
  }
  else if (stricmp(symbol, "DEFAULT_BOSS") == 0) {
    free(*default_boss);
    *default_boss = strdup(filename);
  }
  else if (stricmp(symbol, "DEFAULT_FIELD") == 0) {
    free(*default_field);
    *default_field = strdup(filename);
  }
  else if (stricmp(symbol, "DEFAULT_TOWER") == 0) {
    free(*default_tower);
    *default_tower = strdup(filename);
  }
  // フィールド？
  else if (strnicmp(symbol, "FIELD", 5) == 0) {
    int n = atoi(symbol + 5);
    if (1 <= n && n <= MAX_DUNGEON_LEVEL) {
      n--;
      free(dungeon_bgm->field[n]);
      dungeon_bgm->field[n] = strdup(filename);
    }
  }
  // タワー？
  else if (strnicmp(symbol, "TOWER", 5) == 0) {
    int n = atoi(symbol + 5);
    if (1 <= n && n <= MAX_DUNGEON_LEVEL) {
      n--;
      free(dungeon_bgm->tower[n]);
      dungeon_bgm->tower[n] = strdup(filename);
    }
  }
  // ボス？
  else if (strnicmp(symbol, "BOSS", 4) == 0) {
    int n = atoi(symbol + 4);
    if (1 <= n && n <= 16) {
      n--;
      free(dungeon_bgm->boss[n]);
      dungeon_bgm->boss[n] = strdup(filename);
    }
  }
}

// SE データを登録
void set_se_data(const char *symbol, const char *filename)
{
  // シンボル照合表
  static struct {
    const char *	symbol;
    char **		value;
  } symbol_table[] = {
    { "USER_HIT_DAGGER",		&se_data.user_hit[0]	},
    { "USER_HIT_SWORD",			&se_data.user_hit[1]	},
    { "USER_HIT_SPEAR",			&se_data.user_hit[2]	},
    { "USER_HIT_AXE",			&se_data.user_hit[3]	},
    { "MAGIC_HIT",			&se_data.magic_hit	},
    { "MAGIC_FAILED",			&se_data.magic_failed	},
    { "USER_DAMAGED",			&se_data.damaged	},
    { "USER_TRAPPED",			&se_data.trapped	},
    { "USER_DEAD",			&se_data.user_dead	},
    { "BOSS_DEAD",			&se_data.boss_dead	},
    { "MONSTER_DEAD",			&se_data.monster_dead	},
    { "OPEN_BOX",			&se_data.open_box	},
    { "TREASURE",			&se_data.treasure	},
    { "GET",				&se_data.get		},
    { "GET_POISON",			&se_data.get_poison	},
    { "USE_ELIXER",			&se_data.use_elixer	},
    { "LOST_KEY",			&se_data.lost_key	},
    { "CAST_NEEDLE",			&se_data.cast[0]	},
    { "CAST_MITTAR",			&se_data.cast[1]	},
    { "CAST_DELUGE",			&se_data.cast[2]	},
    { "CAST_FIRE",			&se_data.cast[3]	},
    { "CAST_THUNDER",			&se_data.cast[4]	},
    { "CAST_POISON",			&se_data.cast[5]	},
    { "CAST_CORROSION",			&se_data.cast[6]	},
    { "CAST_TILTE",			&se_data.cast[7]	},
    { "CAST_DEATH",			&se_data.cast[8]	},
    { "USE_SPECTACLES",			&se_data.item[0]	},
    { "USE_RED_POSION",			&se_data.item[1]	},
    { "USE_LAMP",			&se_data.item[2]	},
    { "USE_BLACK_ONYX",			&se_data.item[3]	},
    { "USE_FIRE_CRYSTAL",		&se_data.item[4]	},
    { "USE_MATTOCK",			&se_data.item[5]	},
    { "USE_HOURGLASS",			&se_data.item[6]	},
    { "USE_WINGED_BOOTS",		&se_data.item[7]	},
    { "USE_MANTLE",			&se_data.item[8]	},
    { "USE_DEMONS_RING",		&se_data.item[9]	},
    { "USE_BALANCE",			&se_data.item[10]	},
    { "USE_PENDANT",			&se_data.item[11]	},
    { "USE_CANDLE",			&se_data.item[12]	},
    { "USE_RUBY",			&se_data.item[13]	},
    { "USE_BROWN_POTION",		&se_data.item[14]	},
    { "USE_MIRROR",			&se_data.item[15]	},
    { "USE_BOTTLE",			&se_data.item[16]	},
    { "USE_SILVER_ROSE",		&se_data.item[17]	},
    { "USE_KEY",			&se_data.item[18]	},
    { "USE_ACID",			&se_data.item[19]	},
    { "USE_LADDER",			&se_data.item[20]	},
    { "USE_CROSS",			&se_data.item[21]	},
    { "BOSS_HIT",			&se_data.boss_hit	},
    { "BOSS_BREATH",			&se_data.boss_breath	},
    { "ENCOUNT",			&se_data.encount	},
    { "OPENING0",			&se_data.opening0	},
    { "OPENING1",			&se_data.opening1	},
    { "OPENING2",			&se_data.opening2	}
  };
  int i;

  if (!filename || filename[0] == '\0') {
    // 空文字列では上書きしない
  }
  else if (stricmp(symbol, "DEFAULT_CAST") == 0) {
    for (i = 0; i < number_of(se_data.cast); i++) {
      if (se_data.cast[i] == NULL) {
        se_data.cast[i] = strdup(filename);
      }
    }
  }
  else if (stricmp(symbol, "DEFAULT_USE") == 0) {
    for (i = 0; i < number_of(se_data.item); i++) {
      if (se_data.item[i] == NULL) {
        se_data.item[i] = strdup(filename);
      }
    }
  }
  else {
    for (i = 0; i < number_of(symbol_table); i++) {
      if (stricmp(symbol, symbol_table[i].symbol) == 0) {
        free(*symbol_table[i].value);
        *symbol_table[i].value = strdup(filename);
      }
    }
  }
#if 0
  fprintf(stderr, "%s: %s\n", symbol, filename);
#endif
}
