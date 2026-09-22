#if !defined(GAME_SCENE_H_)
#define GAME_SCENE_H_

#include "scene/Scene.h"
#include <functional>

// 旧C実装(src/context.cpp)の「コンテキスト」に対応するSceneの共通基底クラス。
//
//   旧context_tのenter_guard  -> GameScene派生クラスのonEnter (onCreate/onResumeから呼ばれる)
//   旧context_tのleave_guard  -> GameScene派生クラスのonLeave (onDestroy/onSuspendから呼ばれる)
//   switch_context/extend_context/resume_context (src/context.cpp) が
//   Application::registerNextScene 経由でこのクラスの生成・破棄・一時停止・再開を駆動する。
class GameScene : public Scene
{
public:
	GameScene(int contextId);
	virtual ~GameScene() = default;

	// src/context.h の CONTEXT_* 定数
	int contextId() const { return contextId_; }

	void onCreate(uint32_t tick) override;
	void onResume(uint32_t tick) override;
	void onSuspend() override;
	void onDestroy(uint32_t tick) override;
    bool onIdle(uint32_t tick) override;

	void dispatch(const SDL_Event &event) override;

	void setGameTimer(int intervalMs, std::function<void()> onTimer);
	void killGameTimer();

	// isDone()がtrueになるまで、dispatch()の通常イベント処理とゲーム進行を
	// 止める(旧pause.h XanaduPause)。killGameTimer()を内部で呼ぶので、
	// 呼び出し元の周期処理タイマーは止まる。再開が必要ならonCompleteで
	// 行うこと。maxWaitMs>0ならisDone()が満たされなくてもタイムアウトする
	// (0=無制限)
	void wait(std::function<bool()> isDone, std::function<void()> onComplete = nullptr, uint32_t maxWaitMs = 0);
	// waitの特化版: clearkeyが離されるか最大500ms経過するまで待つ。
	// キーボードの論理状態(押しっぱなし判定)が更新されるまでのdebounce用途
	void pauseFor(int clearkey, std::function<void()> onComplete = nullptr);
	bool isWaiting() const { return waiting_; }

protected:
	virtual void onEnter() = 0;
	virtual void onLeave() = 0;

private:
	void updateWait(uint32_t tick);

	std::function<void()> onTimer_;
	std::function<bool()> waitIsDone_;
	std::function<void()> waitOnComplete_;
	bool waiting_ = false;
	const int contextId_;
};

#endif // GAME_SCENE_H_
