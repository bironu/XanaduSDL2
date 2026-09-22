#if !defined(ENDINGSCENE_H_)
#define ENDINGSCENE_H_

#include "scene/Scene.h"
#include "sdl/SDLImage.h"
#include "geo/Rect.h"
#include "fade.h"
#include <memory>
#include <vector>

class EndingScene final : public Scene
{
public:
	EndingScene();

	void dispatch(const SDL_Event &event) override;
	void onSuspend() override;
	void onCreate(uint32_t tick) override;
	void onDestroy(uint32_t tick) override;
	void onResume(uint32_t tick) override;

private:
    void onKeyDown(const SDL_KeyboardEvent &key);
    void onWindowExpose(const SDL_WindowEvent &window);

    void onEnter();
	void restoreContext();
	void loop();
	void waitForever();
	// フェード(scenario2)/即黒背景(scenario1)のどちらの後でも呼ばれる、
	// クレジットロール開始処理
	void startScroll();

	// フェード演出(XanaduFade)をこのScene自身のタイマーで駆動する
	void startFade(std::shared_ptr<SDL_::Image> dst, int x, int y,
	               std::shared_ptr<SDL_::Image> img, unsigned rgb);
	// フェード中/スクロール中共通のタイマー処理(fade_の状態でどちらかへ振り分ける)
	void onTimer();
	// Application::setTimer()を起動し、以後SDL_ENDING_TIMER_EVENT経由でonTimer()を呼ぶ
	void startTimer(uint32_t interval);

	// clipOverall_(+スクロール中はclipEndingRoll_)をバックバッファへ反映する
	void onDraw();

	bool init();
	int loadKanjiCode(const char *filename);
	std::vector<int>::const_iterator drawKanjiText(std::shared_ptr<SDL_::Image> img, int x, int y,
	                                                std::vector<int>::const_iterator first,
	                                                std::vector<int>::const_iterator last) const;

	static constexpr int kEndingInterval = 80;
	// フェードの1ステップと同じ間隔(旧FADE_INTERVAL)
	static constexpr uint32_t kFadeInterval = 50;
	static const uint32_t SDL_ENDING_TIMER_EVENT;

	XanaduFade fade_;

	static constexpr int kKanjiWidth = 23;    // 漢字フォントの幅
	static constexpr int kKanjiHeight = 23;   // 漢字フォントの高さ
	static constexpr int kKanjiPageCol = 16;  // 漢字フォントの列数
	static constexpr int kKanjiPageRow = 24;  // 漢字フォントの行数

	static const Rect kRectOverall;
	static const Rect kRectEndingRoll;

	bool initialized_;
	bool waitingForKey_;
	bool rollActive_;
	// シナリオ1: クレジットロール終了後、xa1_opening_backgroundへのフェード中か
	bool postRollFadeActive_;

	std::shared_ptr<SDL_::Image> clipOverall_;
	std::shared_ptr<SDL_::Image> clipEndingRoll_;
	std::shared_ptr<SDL_::Image> visualImage_;

	SDL_::SubImage kanji_[kKanjiPageRow][kKanjiPageCol];
	std::shared_ptr<SDL_::Image> kanjiBase_;
	std::shared_ptr<SDL_::Image> msg_;

	std::vector<int> kanjiCode_;
	std::vector<int>::const_iterator kanjiCodeTop_;

	int msgY_;
	int msgRestRows_;
};

#endif // ENDINGSCENE_H_
