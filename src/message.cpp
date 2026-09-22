#include "xanadu.h"
#include "message.h"
#include "pause.h"

#include <stdarg.h>
#include <ctype.h>

#ifdef __BORLANDC__
#include <mem.h>
#endif

void (*thunk_key_event)(int c);

// メッセージ画面の現在の行
static int current_line;

void display_message(const char *s, SDL_::Color pixel)
{
  int n_rows;

  // メッセージビューに表示できる行数
  n_rows = rect_message.height / 16;
  while (*s) {
    // 一番下の行を超えている？
    if (current_line >= n_rows) {
      scroll_image(clip_message, -16);
      current_line = n_rows - 1;
    }
    s += draw_text(clip_message, 0, current_line * 16, s, pixel);
    current_line++;
  }
  update(rect_message);
}

void format_message(const char *fmt, ...)
{
  va_list args;
  char buf[256];

  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  va_end(args);
  
  display_message(buf, SDL_::Color::WHITE);
}

void flush_message(void)
{
  fill_image(clip_message, 0, 0, clip_message->getWidth(), clip_message->getHeight(),
             SDL_::Color::BLACK);
  current_line = 0;
  update(rect_message);
}

// メッセージビューを介したキー入力

#define MAX_BUFFER 14

static char enter_buffer[MAX_BUFFER];	// 入力文字列バッファ
static int enter_n_characters;		// 入力文字の数
static void message_key(int vkey);	// キー入力イベント
static void (*thunk_consumer)(char *s);	// 入力文字列を受け取る関数

int init_enter_buffer(int context_id, void (*consumer)(char *s))
{
  int n_rows;
  
  thunk_consumer = consumer;
  n_rows = rect_message.height / 16;  
  enter_n_characters = 0;
  memset(enter_buffer, 0, sizeof(enter_buffer));
    
  if (context_id != CONTEXT_ENTER_CHARACTER) {
    // 一番下の行を超えている？
    if (current_line >= n_rows) {
      scroll_image(clip_message, -16);
      current_line = n_rows - 1;
    }
  }
  return context_id;
}

static void call_consumer(int last_key)
{
  if (current_context_id() != CONTEXT_ENTER_CHARACTER) {
    current_line++;
  }
  // 末尾に終端符を付与してサンクに送る
  enter_buffer[enter_n_characters] = '\0';
  if (thunk_consumer) {
    (*thunk_consumer)(enter_buffer);
  }
#ifdef NO_PAUSE
  resume_context();
#else
  switch_context(init_pause(50, scancodeFromChar(toupper(last_key))));
#endif
}

static void update_enter_buffer(void)
{
  int row = current_line * 16;
  fill_image(clip_message, 0, row, 16, 16, SDL_::Color::BLACK);
  draw_text(clip_message, 0, row, enter_buffer, SDL_::Color::WHITE);
  update(rect_message);
}

void message_enter_enter(void)
{
  thunk_key_event = message_key;
}

void message_enter_leave(void)
{
  thunk_key_event = NULL;
}

void message_key(int c)
{
  switch (current_context_id()) {
  case CONTEXT_ENTER_NUMBER:
    // 数値
    if (c == '\r' || c == '\n') {
      call_consumer(c);
    } else
    if (enter_n_characters < 4 && '0' <= c && c <= '9') {
      enter_buffer[enter_n_characters++] = c;
      update_enter_buffer();
    } else
    if (c == '\b' && enter_n_characters > 0) {
      enter_buffer[--enter_n_characters] = '\0';
      update_enter_buffer();
    }
    break;

  case CONTEXT_ENTER_STRING:
    // 文字列
    if (c == '\r' || c == '\n') {
      call_consumer(c);
    } else
    if (enter_n_characters < MAX_BUFFER - 1 && ' ' <= c && c <= '~') {
      enter_buffer[enter_n_characters++] = c;
      update_enter_buffer();
    } else
    if (c == '\b' && enter_n_characters > 0) {
      enter_buffer[--enter_n_characters] = '\0';
      update_enter_buffer();
    }
    break;
        
  case CONTEXT_ENTER_CHARACTER:
  default:
    enter_buffer[0] = c;
    enter_n_characters = 1;
    call_consumer(c);
  }
}
