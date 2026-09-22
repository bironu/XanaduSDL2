#ifndef pause_H
#define pause_H

#include <cstdint>
#include <functional>

/* GameScene::wait/pauseFor(include/scene/GameScene.h)へのブリッジ。
 * 状態は全てGameScene側のメンバとして持ち、ここではグローバル変数を
 * 一切持たない。battle.cpp等、まだGameScene派生クラスのメンバ関数に
 * なっていないレガシーC関数群から現在のSceneへ委譲するためだけに存在する */

void begin_wait(std::function<bool()> isDone, std::function<void()> onComplete = nullptr, uint32_t maxWaitMs = 0);
void begin_pause(int clearkey, std::function<void()> onComplete = nullptr);

#endif // pause_H
