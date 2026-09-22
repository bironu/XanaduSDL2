#ifndef pause_H
#define pause_H

#include <cstdint>
#include <functional>

/* isDone()がtrueになるまで、GameScene::onIdle/dispatchが通常のゲーム
 * 進行・入力処理を止める軽量ヘルパー。以前はPauseScene(Sceneスタックへの
 * 一時的な積み直し)+専用タイマースレッドで実現していたが、
 * GameScene::onIdle()のポーリングだけで十分なためXanaduPauseに一本化した */

// 呼び出し元の周期処理タイマー(battle_loop等)はbegin_wait()が内部で
// kill_timer()して止める。再開が必要ならonCompleteで行うこと。
// maxWaitMs>0なら、isDone()が満たされなくても経過時にタイムアウトする
// (0=無制限、isDone()が満たされるまで待つ)
void begin_wait(std::function<bool()> isDone, std::function<void()> onComplete = nullptr, uint32_t maxWaitMs = 0);

// begin_waitの特化版: clearkeyが離されるか最大500ms経過するまで待つ。
// キーボードの論理状態(押しっぱなし判定)が更新されるまでのdebounce用途
// (例: begin_pause(SDL_SCANCODE_S, battle_enter))
void begin_pause(int clearkey, std::function<void()> onComplete = nullptr);

// GameScene::onIdle/dispatchから呼ばれる。呼び出し元が直接使うことはない
bool pause_active();
void pause_update(uint32_t nowTick);

#endif // pause_H