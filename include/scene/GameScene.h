#if !defined(GAME_SCENE_H_)
#define GAME_SCENE_H_

#include "scene/Scene.h"

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

	// キー入力等のOSイベント処理。挙動は全コンテキスト共通のため、この基底クラスで実装する。
	void dispatch(const SDL_Event &event) override;

protected:
	// 旧*_enter関数、旧*_leave関数に対応
	virtual void onEnter() = 0;
	virtual void onLeave() = 0;

private:
	const int contextId_;
};

#endif // GAME_SCENE_H_
