#include "xanadu.h"
#include <string.h>
#include <ctype.h>

#ifdef __BORLANDC__
#include <dir.h>
#endif

#ifndef __BORLANDC__
#include <sys/types.h>
#include <sys/stat.h>

#define mkdir(dir) (mkdir((dir), 0700))
#endif

static int save_user_data(void);

int make_user_dir(void)
{
  char path[BUFSIZ];
  int i, error = 0;
            
  /* 名前がない？ */
  if (user.status.name[0] == '\0')
    return 1;

  /* ディレクトリの作成 */
  sprintf(path, USERS_DIR "/%s", user.status.name);
  mkdir(USERS_DIR);
  mkdir(path);
  
  /* ユーザーディレクトリ */
  free((void *)user_path);
  user_path = strdup(path);

  /* 階層データファイルをコピー */
  for (i = 0; i < MAX_DUNGEON_LEVEL - 1; i++) {
    error |= load_level(i, NULL);
    error |= save_level(i, user_path);
  }
  /* scenario 2 */
  if (in_scenario2()) {
    error |= load_level(MAX_DUNGEON_LEVEL - 1, NULL);
    error |= save_level(MAX_DUNGEON_LEVEL - 1, user_path);
  }

  error |= save_user_data(); /* ユーザー情報の保存 */
  return error;
}

int save_user_data(void)
{
  char path[BUFSIZ];
  FILE *fp;
  int error;
  
  if (user_path == NULL)
    return 1;

  sprintf(path, "%s/user.dat", user_path);
  fp = fopen(path, "wb");
  if (fp == NULL) {
    perror(path);
    return 1;
  }
  
  /* ユーザー情報の保存 */
  error = fwrite(&user, sizeof(user), 1, fp) != 1;
  fclose(fp);

  return error;
}

int load_user(void)
{
  char path[BUFSIZ];
  FILE *fp;
  int error;

  if (user_path == NULL)
    return 1;

  sprintf(path, "%s/user.dat", user_path);
  fp = fopen(path, "rb");
  if (fp == NULL) {
    perror(path);
    return 1;
  }
  
  /* ユーザー情報の読み込み */
  error = fread(&user, sizeof(user), 1, fp) != 1;
  fclose(fp);

  return error;
}

int save_user(void)
{
  if (save_user_data() == 0)
    return save_level(user.environment.dungeon_level, user_path);
  else
    return 1;
}

/* 隠し名キャラクター */

static int find_name(FILE *fp, const char *name)
{
  char buffer[BUFSIZ], *p, *tag;

  while (fgets(buffer, sizeof(buffer), fp)) {
    for (p = buffer; isspace(*p); p++)
      ;
    /* 空行？ */
    if (*p == '#' || *p == '\0')
      continue;

    tag = strtok(p, ":");

    if (stricmp(tag, "NAME") == 0) {
      p = (strtok(NULL, "\""), strtok(NULL, "\""));

      /* 一致する？ */
      if (p != NULL && strcmp(p, name) == 0)
        return 1;
    }

    /* 次の空行まで読み飛ばす */
    while (fgets(buffer, sizeof(buffer), fp)) {
      for (p = buffer; isspace(*p); p++)
        ;
      /* 空行？ */
      if (*p == '#' || *p == '\0')
        break;
    }
  }
  return 0; /* 見つからない */
}

static void parse_tags(FILE *fp)
{
  static const struct {
    char *	tag;
    int *	location;
  } tags_status[] = {
    { "HP",	&user.status.max_HP	},
    { "GOLD",	&user.status.gold	},
    { "FOOD",	&user.status.food	},
    { "STR",	&user.status.STR	},
    { "INT",	&user.status.INT	},
    { "WIS",	&user.status.WIS	},
    { "DEX",	&user.status.DEX	},
    { "AGL",	&user.status.AGL	},
    { "CHR",	&user.status.CHR	},
    { "MGR",	&user.status.MGR	},
    { "KRM",	&user.status.KRM	},
    { "ELX",	&user.status.ELX	},
    { "KEY",	&user.status.KEY	},
    { "CRN",	&user.status.CRN	}
  };
  static const char *tags_equip[] = {
    "Weapon", "Scroll", "Armor", "Shield", "Item"
  };
  char buffer[BUFSIZ], *p, *tag;
  int i, j;
  
  while (fgets(buffer, sizeof(buffer), fp)) {
    for (p = buffer; isspace(*p); p++)
      ;
    /* 空行？ */
    if (*p == '#' || *p == '\0')
      break;

    tag = strtok(p, ":");
    if (tag == NULL)
      break;

    for (i = 0; i < sizeof(tags_status)/sizeof(tags_status[0]); i++) {
      if (stricmp(tag, tags_status[i].tag) == 0) {
        p = strtok(NULL, "");
        *(tags_status[i].location) = atoi(p);
        goto done;
      }
    }

    for (i = 0; i < sizeof(tags_equip)/sizeof(tags_equip[0]); i++) {
      if (stricmp(tag, tags_equip[i]) == 0) {
        p = (strtok(NULL, "\""), strtok(NULL, "\""));
        if (p != NULL) {
          for (j = 0; j < MAX_GOODS; j++)
            if (strcmp(p, goods_data[i][j].name) == 0)
              user.equipment[i] = j;
        }
        goto done;
      }
    }

    if (stricmp(tag, "INVENTORY") == 0) {
      for (;;) {
        char *q;

        p = (strtok(NULL, "\""), strtok(NULL, "\""));
        q = strtok(NULL, ",");
        if (p == NULL || q == NULL || *q++ != '*')
          break;

        for (i = 0; i < 5; i++)
          for (j = 0; j < MAX_GOODS; j++)
            if (strcmp(p, goods_data[i][j].name) == 0)
              user.inventory[i][j].stock = atoi(q);
      }
    }
  done:
    ;
  }
}

void match_user_name(const char *name)
{
  FILE *fp;

  fp = fopen(USERS_DIR "/thanks.txt", "rt");
  if (fp == NULL)
    return;

  if (find_name(fp, name)) {
    parse_tags(fp);
  }
  fclose(fp);
}
