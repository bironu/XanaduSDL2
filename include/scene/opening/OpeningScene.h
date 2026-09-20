#if !defined(OPENINGSCENE_H_)
#define OPENINGSCENE_H_

#include "scene/Scene.h"
#include "sdl/SDLImage.h"
#include "geo/FRect.h"
#include "fade.h"

class OpeningScene final : public Scene
{
public:
	OpeningScene();

	void dispatch(const SDL_Event &event) override;
	void onCreate(uint32_t tick) override;
	void onResume(uint32_t tick) override;
	void onSuspend() override;
    void onDestroy(uint32_t tick) override;
    bool onIdle(uint32_t tick) override;

private:
    void onKeyDown(const SDL_KeyboardEvent &key);
    void onWindowExpose(const SDL_WindowEvent &window);
    void onTimer();
    void onDraw();

    // 旧opening_enter/wait_forever/restore_contextに対応
	uint32_t onEnter();
	// フェードが1段階(黒→着色→フル発色)終わるたびに次の段階へ進める
	void advanceFade();
	void waitForever();
	void restoreContext();

	int step_;
	unsigned fadeMask_;
	// フェード自体はタイマーを持たない(XanaduFade参照)。このScene自身の
	// タイマーからstep()を呼び出すことで、フェード中も自分のdispatch()/
	// onIdle()が止まらないようにする。
	XanaduFade fade_;
	// wait_forever()到達後、キー入力を待っている間だけtrueになる
	// (旧実装のthunk_key_event = opening_key_eventに対応)
	bool waitingForKey_;

    std::shared_ptr<SDL_::Image> clip_overall_;
    std::shared_ptr<SDL_::Image> visual_image_;
    static const FRect rect_overall_;

    static const uint32_t SDL_OPENING_TIMER_EVENT;
    // フェードの1ステップと同じ間隔(旧FADE_INTERVAL)
    static constexpr uint32_t kTimerInterval = 50;
};

#endif // OPENINGSCENE_H_
