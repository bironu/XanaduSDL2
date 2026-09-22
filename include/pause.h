#ifndef pause_H
#define pause_H

#include <cstdint>
#include <functional>

/* clearkeyが離されるか、開始から最大500ms経過するまで待つ軽量ヘルパー。
 * キーボードの論理状態(押しっぱなし判定)が更新されるまでの
 * debounce用途。以前はPauseScene(Sceneスタックへの一時的な積み直し)+
 * 専用タイマースレッドで実現していたが、GameScene::onIdle()の
 * ポーリングだけで十分なためXanaduPauseに一本化した */

// 呼び出し元の周期処理タイマー(battle_loop等)はbegin_pause()が内部で
// kill_timer()して止める。再開が必要ならonCompleteで行うこと
// (例: begin_pause(SDL_SCANCODE_S, battle_enter))
void begin_pause(int clearkey, std::function<void()> onComplete = nullptr);

// GameScene::onIdle/dispatchから呼ばれる。呼び出し元が直接使うことはない
bool pause_active();
void pause_update(uint32_t nowTick);

#endif // pause_H