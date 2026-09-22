#include "xanadu.h"
#include "dungeon.h"
#include "resources/Resources.h"
#include "resources/ImageId.h"

// 地形タイルデータベース
#define NO_TILE MAX_TILE

// scenario 1
static const tile_data_t scenario1_db = {
  { TILE_WALL | TILE_WALL_DIG,
    TILE_WALL | TILE_WALL_MARBLE,
    TILE_WALL,
    TILE_FOOTHOLD,
    TILE_FOOTHOLD,
    0, 0, 0, 0, 0, 0, 0, 0,
    TILE_WALL,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  0,			// レンガ
  1,			// 大理石
  3,			// 橋げた
  4,			// はしご
  7,			// 背景１
  8,			// 背景２
  6,			// 背景(ワープ)
  9,			// 次の階層に通じる洞窟
  11,			// 前の階層に通じる洞窟
  15,			// 封印されている洞窟
  14,			// フロア
  13,			// 扉
  { 13, 18, 19 },	// 扉を開ける(フィールド)
  { 13, 18, 19 },	// 扉を開ける(タワー内部)
  { 48, 49, 50 },	// 掘る
  12,			// 最後の砦の入口
  NO_TILE,		// 斜面(左下がり)
  NO_TILE,		// 斜面(右下がり)
  NO_TILE,		// 人面石
  NO_TILE,		// 氷柱(トゲトゲ)
  NO_TILE,		// 三階層次に通じる洞窟
  NO_TILE		// 三階層前に通じる洞窟
};

// scenario 2
static const tile_data_t scenario2_db = {
  { TILE_WALL,
    TILE_WALL | TILE_WALL_MARBLE,
    TILE_WALL | TILE_WALL_MARBLE,
    TILE_WALL,
    TILE_WALL,
    TILE_WALL | TILE_WALL_DIG,
    TILE_WALL,
    TILE_WALL,
    TILE_WALL,
    TILE_WALL,
    TILE_WALL,
    TILE_WALL,
    TILE_FOOTHOLD,
    TILE_FOOTHOLD,
    0, 0, 0,
    TILE_WALL,
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  0,			// レンガ
  1,			// 大理石
  12,			// 橋げた
  13,			// はしご
  22,			// 背景１
  23,			// 背景２
  20,			// 背景(ワープ)
  26,			// 次の階層に通じる洞窟
  27,			// 前の階層に通じる洞窟
  14,			// 封印されている洞窟
  21,			// フロア
  17,			// 扉
  { 17, 18, 19 },	// 扉を開ける(フィールド)
  { 17, 18, 19 },	// 扉を開ける(タワー内部)
  { 50, 51, 52 },	// 掘る
  NO_TILE,		// 最後の砦の入口
  10,			// 斜面(左下がり)
  11,			// 斜面(右下がり)
  5,			// 人面石
  9,			// 氷柱(トゲトゲ)
  25,			// 三階層次に通じる洞窟
  24			// 三階層前に通じる洞窟
};

// training-ground
static const tile_data_t training_db = {
  { TILE_WALL,
    TILE_WALL | TILE_WALL_MARBLE,
    TILE_WALL,
    TILE_FOOTHOLD,
    TILE_FOOTHOLD,
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  0,			// レンガ
  1,			// 大理石
  3,			// 橋げた
  4,			// はしご
  7,			// 背景１
  8,			// 背景２
  6,			// 背景(ワープ)
  5,			// 次の階層に通じる洞窟
  NO_TILE,
  NO_TILE,
  NO_TILE,
  NO_TILE,
  { NO_TILE, NO_TILE, NO_TILE },
  { NO_TILE, NO_TILE, NO_TILE },
  { NO_TILE, NO_TILE, NO_TILE },
  NO_TILE,
  NO_TILE,
  NO_TILE,
  NO_TILE,
  NO_TILE,
  NO_TILE,
  NO_TILE
};

// 階層データを保持する構造体
typedef struct {
  char *		mon_filename;	// モンスター記述ファイル
  char *		mon_image;	// モンスターイメージ
  char *		map_image;	// 地形タイルイメージ
  const tile_data_t *	tile_database;	// 地形タイルデータベース
} dungeon_level_data_t;

// 階層データベース

// scenario 1
static const dungeon_level_data_t dungeon1[MAX_DUNGEON_LEVEL] = {
  { "monst_0.mon", "monst_0.bmp", "xa1/field.bmp", &scenario1_db },
  { "monst_1.mon", "monst_1.bmp", "xa1/field.bmp", &scenario1_db },
  { "monst_2.mon", "monst_2.bmp", "xa1/field.bmp", &scenario1_db },
  { "monst_3.mon", "monst_3.bmp", "xa1/field.bmp", &scenario1_db },
  { "monst_4.mon", "monst_4.bmp", "xa1/field.bmp", &scenario1_db },
  { "monst_5.mon", "monst_5.bmp", "xa1/field.bmp", &scenario1_db },
  { "monst_6.mon", "monst_6.bmp", "xa1/field.bmp", &scenario1_db },
  { "monst_7.mon", "monst_7.bmp", "xa1/field.bmp", &scenario1_db },
  { "monst_8.mon", "monst_8.bmp", "xa1/field.bmp", &scenario1_db },
  { "monst_9.mon", "monst_9.bmp", "xa1/field.bmp", &scenario1_db },
  { "monst_a.mon", "monst_a.bmp", "xa1/train.bmp", &training_db }
};

// scenario 2
static const dungeon_level_data_t dungeon2[MAX_DUNGEON_LEVEL] = {
  { "monst_0.mon", "monst_0.bmp", "xa2/field.bmp", &scenario2_db },
  { "monst_1.mon", "monst_1.bmp", "xa2/field.bmp", &scenario2_db },
  { "monst_2.mon", "monst_2.bmp", "xa2/field.bmp", &scenario2_db },
  { "monst_3.mon", "monst_3.bmp", "xa2/field.bmp", &scenario2_db },
  { "monst_4.mon", "monst_4.bmp", "xa2/field.bmp", &scenario2_db },
  { "monst_5.mon", "monst_5.bmp", "xa2/field.bmp", &scenario2_db },
  { "monst_6.mon", "monst_6.bmp", "xa2/field.bmp", &scenario2_db },
  { "monst_7.mon", "monst_7.bmp", "xa2/field.bmp", &scenario2_db },
  { "monst_8.mon", "monst_8.bmp", "xa2/field.bmp", &scenario2_db },
  { "monst_9.mon", "monst_9.bmp", "xa2/field.bmp", &scenario2_db },
  { "monst_a.mon", "monst_a.bmp", "xa2/field.bmp", &scenario2_db },
};

// レベルデータ
level_data_t level_data;

// 地形タイルデータ
tile_data_t tile_data;
static int load_tile_image(ImageId id);

// モンスターデータ
monster_status_t monster_data[MAX_MONSTER * MAX_VARIETY];
static int load_monster_image(ImageId id);

namespace
{
const ImageId kMonsterImageIds1[MAX_DUNGEON_LEVEL] = {
	ImageId::xa1_monst_0, ImageId::xa1_monst_1, ImageId::xa1_monst_2,
	ImageId::xa1_monst_3, ImageId::xa1_monst_4, ImageId::xa1_monst_5,
	ImageId::xa1_monst_6, ImageId::xa1_monst_7, ImageId::xa1_monst_8,
	ImageId::xa1_monst_9, ImageId::xa1_monst_a,
};
const ImageId kMonsterImageIds2[MAX_DUNGEON_LEVEL] = {
	ImageId::xa2_monst_0, ImageId::xa2_monst_1, ImageId::xa2_monst_2,
	ImageId::xa2_monst_3, ImageId::xa2_monst_4, ImageId::xa2_monst_5,
	ImageId::xa2_monst_6, ImageId::xa2_monst_7, ImageId::xa2_monst_8,
	ImageId::xa2_monst_9, ImageId::xa2_monst_a,
};

ImageId monsterImageId(bool scenario2, int level)
{
	if (level < 0 || MAX_DUNGEON_LEVEL <= level) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "monsterImageId: dungeon level out of range %d\n", level);
		return ImageId::xa1_monst_0;
	}
	return (scenario2 ? kMonsterImageIds2 : kMonsterImageIds1)[level];
}

ImageId tileImageId(bool scenario2, const dungeon_level_data_t *entry)
{
	if (scenario2) {
		return ImageId::xa2_field;
	}
	return entry->tile_database == &training_db ? ImageId::xa1_train : ImageId::xa1_field;
}
}

static int load_outoflevel(void)
{
  int i, j;
  int error;
  FILE *fp;

  tile_data = scenario1_db;
  load_tile_image(ImageId::xa1_field);

  load_monster_image(ImageId::xa2_outoflevel);
  for (i = 0; i < N_MONSTERS; i++) {
    for (j = 0; j < 4; j++) {
      frame_monsters[i][j].sheet->setColorKey(0xFF000000);
    }
  }

  fp = fopen(LEVEL_DIR "/xa2/outoflevel.map", "rb");
  if (!fp) {
    perror("outoflevel.mon");
    return 1;
  }
  error = fread(&level_data, sizeof(level_data), 1, fp) != 1;
  fclose(fp);
  if (error) {
    fprintf(stderr, "Can't read level-data.\n");
    return 1;
  }

  // モンスター情報の読み込み
  fp = fopen(LEVEL_DIR "/xa2/outoflevel.mon", "rb");
  if (!fp) {
    perror("outoflevel.mon");
    return 1;
  }
  error = fread(monster_data, sizeof(monster_data), 1, fp) != 1;
  fclose(fp);
  if (error) {
    fprintf(stderr, "Can't read monster-data.\n");
    return 1;
  }
  return 0;
}

int load_level(int level, const char *dir)
{
  char path[BUFSIZ];
  FILE *fp;
  int error;
  const char *subdir;
  const dungeon_level_data_t *dungeons;

  if (level < 0 || MAX_DUNGEON_LEVEL <= level) {
    return load_outoflevel();
  }
  
  if (in_training_ground() || !in_scenario2()) {
    dungeons = dungeon1;
    subdir = "xa1";
  } else {
    dungeons = dungeon2;
    subdir = "xa2";
  }

  // 地形タイルデータベースの更新
  tile_data = *(dungeons[level].tile_database);

  load_tile_image(tileImageId(dungeons == dungeon2, &dungeons[level]));

  // レベル情報の読み込み
  if (dir) {
    sprintf(path, "%s/level_%x.map", dir, level);
  } else {
    // デフォルトディレクトリ
    sprintf(path, "%s/%s/level_%x.map", LEVEL_DIR, subdir, level);
  }
  
  if ((fp = fopen(path, "rb")) == NULL) {
    perror(path);
    return 1;
  }
  error = fread(&level_data, sizeof(level_data), 1, fp) != 1;
  fclose(fp);

  if (error) {
    fprintf(stderr, "Can't load level: %s\n", path);
    return 1;
  }

  // モンスターイメージの読み込み
  if (load_monster_image(monsterImageId(dungeons == dungeon2, level))) {
    return 1;
  }
  // モンスター情報の読み込み
  sprintf(path, "%s/%s/%s", LEVEL_DIR, subdir, dungeons[level].mon_filename);
  if ((fp = fopen(path, "rb")) == NULL) {
    perror(path);
    return 1;
  }
  error = fread(monster_data, sizeof(monster_data), 1, fp) != 1;
  fclose(fp);
  
  if (error) {
    fprintf(stderr, "Can't load monsters: %s\n", path);
    return 1;
  }
  return 0;
}

int load_tile_image(ImageId id)
{
  static bool hasCurrent = false;
  static ImageId currentId;
  if (!hasCurrent || currentId != id) {
    int i;

    if (hasCurrent) {
      Resources::instance().unloadImage(currentId);
    }
    Resources::instance().loadImage(id);
    currentId = id;
    hasCurrent = true;
    auto tile_base = Resources::instance().getImage(id);
    for (i = 0; i < N_TILES; i++) {
      frame_tiles[i] = SDL_::SubImage{tile_base, Rect(i * 40, 0, 40, 40)};
    }
  }
  return 0;
}

int load_monster_image(ImageId id)
{
  static bool hasCurrent = false;
  static ImageId currentId;
  int i, j;

  if (hasCurrent && currentId != id) {
    Resources::instance().unloadImage(currentId);
  }
  Resources::instance().loadImage(id);
  currentId = id;
  hasCurrent = true;
  auto monster_base = Resources::instance().getImage(id);
  for (i = 0; i < N_MONSTERS; i++) {
    for (j = 0; j < 4; j++) {
      frame_monsters[i][j] = SDL_::SubImage{monster_base, Rect(j * 40, i * 40, 40, 40)};
    }
  }
  return 0;
}

int save_level(int level, const char *dir)
{
  char path[BUFSIZ];
  FILE *fp;
  int error;
  
  if (dir == NULL || level < 0 || MAX_DUNGEON_LEVEL <= level) {
    return 1;
  }
  
  sprintf(path, "%s/level_%x.map", dir, level);
  if ((fp = fopen(path, "wb")) == NULL) {
    perror(path);
    return 1;
  }
  
  error = fwrite(&level_data, sizeof(level_data), 1, fp) != 1;
  fclose(fp);

  return error;
}
