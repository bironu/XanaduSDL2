#ifndef message_H
#define message_H

#include "graphics.h"

// メッセージ表示関数
extern void display_message(const char *msg, SDL_::Color pixel);
extern void flush_message(void);

// 書式化メッセージ表示関数
extern void format_message(const char *fmt, ...);

#define emit_message(s) display_message(s, SDL_::Color::WHITE)
#define emit_error(s) display_message(s, SDL_::Color::RED)

// キー入力初期化関数
extern int init_enter_buffer(int context_id, void (*consumer)(char *s));

// コンテキスト保護関数
extern void message_enter_enter(void);
extern void message_enter_leave(void);

// キーイベントが発生したときに呼ばれる関数
extern void (*thunk_key_event)(int c);

#endif // message_H