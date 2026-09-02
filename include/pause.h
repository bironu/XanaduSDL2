#ifndef pause_H
#define pause_H

/* このモジュールはキーボードの論理状態が更新されるまで
 * しばらく(最大で 1 秒間)待つときに使われる */

/* コンテキスト保護関数 */
extern void pause_enter(void);
extern void pause_leave(void);

/* 初期化関数 */
extern int init_pause(int interval, int clearkey);

#endif /* pause_H */