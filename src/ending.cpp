#include "xanadu.h"
#include "ending.h"
#include "fade.h"

#define ENDING_INTERVAL		80

#define KANJI_WIDTH		23	/* 漢字フォントの幅 */
#define KANJI_HEIGHT		23	/* 漢字フォントの高さ */

#define KANJI_PAGE_COL		16	/* 漢字フォントの列数 */
#define KANJI_PAGE_ROW		24	/* 漢字フォントの行数 */

static std::shared_ptr<SDL_::Image> kanji[KANJI_PAGE_ROW][KANJI_PAGE_COL];
static std::shared_ptr<SDL_::Image> kanji_base;
static std::shared_ptr<SDL_::Image> msg;
static int msg_y;
static int msg_rest_rows;
static int *kanji_code, *kanji_code_top, *kanji_code_end;

static void restore_context(void);
static void ending_loop(void);
static void wait_forever(void);

static int load_kanji_code(const char *filename);
static int draw_kanji_text(std::shared_ptr<SDL_::Image> img, int x, int y, int *code);

int init_ending(void)
{
  /* 初期化はinit()で行う */
  return CONTEXT_ENDING;
}

static int init(void)
{
  char path[BUFSIZ];
  int i, j;

  msg_y = 0;
  msg_rest_rows = 0;

  sprintf(path, IMAGE_DIR "/%s", !in_scenario2() ?
          "xa1/ending/message.txt" : "xa2/ending/message.txt");

  /* エンディングメッセージの読み込み */
  if (load_kanji_code(path)) {
    return 1;
  }

  /* 漢字フォントの読み込み */
  kanji_base = load_image(IMAGE_DIR "/picture/kanji.bmp");
  if (!kanji_base) {
    return 1;
  }
  for (i = 0; i < KANJI_PAGE_ROW; i++) {
    for (j = 0; j < KANJI_PAGE_COL; j++) {
      kanji[i][j] = std::make_shared<SDL_::Image>(
          kanji_base, Rect(j * KANJI_WIDTH, i * KANJI_HEIGHT, KANJI_WIDTH, KANJI_HEIGHT));
    }
  }

  /* メッセージ用のオフスクリーンバッファ */
  msg = create_image(clip_endingroll->getWidth(),
                     clip_endingroll->getHeight() + KANJI_HEIGHT);
  if (!msg) {
    return 1;
  }
  msg->setColorKey(kanji_base->getColorKey());
  fill_image(msg, 0, 0, msg->getWidth(), msg->getHeight(), SDL_::Color::BLACK);

  return 0;
}

static void restore_context(void)
{
  kanji_base = nullptr;
  msg = nullptr;

  free(kanji_code);
  kanji_code = NULL;

  visual_image = nullptr;

  /* バックグラウンドイメージを復元 */
  load_background(IMAGE_DIR "/user/frame.bmp");
  update(rect_overall);

  resume_context();
}

void ending_enter(void)
{
  if (!kanji_base) {
    if (init()) {
      beep();
      restore_context();
    }

    if (!in_scenario2()) {
      /* スクリーンのコピーを作成 */
      visual_image = create_image(clip_overall->getWidth(), clip_overall->getHeight());
      if (!visual_image) {
        restore_context();
      }
      draw_image(visual_image, 0, 0, clip_overall);
      fill_image(clip_overall, 0, 0, clip_overall->getWidth(), clip_overall->getHeight(),
                 SDL_::Color::BLACK);

      /* セピア色? */
      extend_context(init_fade(clip_overall, 0, 0, visual_image, 0xff7f00));
    } else {
      visual_image = load_image(IMAGE_DIR "/xa2/ending/background.bmp");
      if (!visual_image) {
        restore_context();
      }
      extend_context(init_fade(clip_overall, 0, 0, visual_image, 0xffffff));
    }
  } else {
    /* もう一度スクリーンをコピー */
    visual_image = create_image(clip_overall->getWidth(), clip_overall->getHeight());
    if (!visual_image) {
      restore_context();
    }
    draw_image(visual_image, 0, 0, clip_overall);

    bgm_play(bgm_data.theme[user.environment.scenario].main);
    set_timer(ENDING_INTERVAL, ending_loop);
  }
}

void ending_leave(void)
{
  thunk_key_event = NULL;
  kill_timer();
}

void ending_loop(void)
{
  if (get_keystate(VK_CONTROL) && get_keystate('Q')) {
    restore_context();
    return;
  }

  msg_y += 1;

  /* 新しい行が完全に現れた? */
  if (msg_y > KANJI_HEIGHT) {
    scroll_image(msg, -msg_y);
    msg_y = 0;
    if (kanji_code_top < kanji_code_end) {
      int n;

      /* 最下行に描画 */
      n = draw_kanji_text(msg, 0, msg->getHeight() - KANJI_HEIGHT, kanji_code_top);
      kanji_code_top += n;
    } else {
      msg_rest_rows++;

      /* 最後の行がスクリーンから見えなくなった? */
      if (msg_rest_rows > msg->getHeight() / KANJI_HEIGHT) {
        wait_forever();
      }
    }
  }

  auto clip_visual = std::make_shared<SDL_::Image>(
      visual_image, Rect(rect_endingroll.x, rect_endingroll.y,
                          rect_endingroll.width, rect_endingroll.height));

  draw_image(clip_endingroll, 0, 0, clip_visual);
  draw_sprite(clip_endingroll, 0, -msg_y, msg);

  update(rect_endingroll);
}

static void ending_key_event(int c)
{
  restore_context();
}

void wait_forever(void)
{
  kill_timer();
  thunk_key_event = ending_key_event;
  bgm_play(bgm_data.theme[user.environment.scenario].ending);
}

#define MESSAGE_BUFFER_SIZE	8024

int load_kanji_code(const char *filename)
{
  FILE *fp;
  int i;

  kanji_code = (int *)malloc(MESSAGE_BUFFER_SIZE * sizeof(int));
  if (!kanji_code) {
    return 1;
  }
  kanji_code_top = kanji_code;
  kanji_code_end = kanji_code;

  fp = fopen(filename, "rt");
  if (!fp) {
    perror(filename);
    return 1;
  }

  for (i = 0; i < MESSAGE_BUFFER_SIZE - 1; i++, kanji_code_end++) {
    if (fscanf(fp, "%d ", kanji_code_end) != 1)
      break;
  }
  *kanji_code_end = -1; /* 終わりを示す */

  fclose(fp);
  return 0;
}

int draw_kanji_text(std::shared_ptr<SDL_::Image> img, int x, int y, int *code)
{
  int row, col;
  int n = 0;
  for (; *code >= 0; code++, n++) {
    row = *code / KANJI_PAGE_COL;
    col = *code % KANJI_PAGE_COL;
    if (row < KANJI_PAGE_ROW) {
      draw_image(img, x, y, kanji[row][col]);
    }
    x += KANJI_WIDTH;
  }
  return n + 1;
}
