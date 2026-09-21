#include "scene/ending/EndingScene.h"
#include "xanadu.h"
#include "app/Application.h"
#include "sdl/SDLWindow.h"
#include "sdl/SDLMixMixer.h"
#include "resources/Resources.h"
#include "resources/ImageId.h"
#include "resources/MusicId.h"

#include <SDL3/SDL_events.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>

namespace fs = std::filesystem;

namespace {

// bgm_data(audio/midi/midi.txt)のXA1_MAIN/XA1_ENDING/XA2_MAIN/XA2_ENDINGに対応する
// MusicId。MenuScene/OpeningScene同様、シーン固有の音楽はResources経由の
// 固定IDで読み込む方針へ揃える(ini経由の差し替えには追従しない)。
MusicId mainThemeId()
{
	return in_scenario2() ? MusicId::xana2dl_xana229 : MusicId::xanadu;
}

MusicId endingThemeId()
{
	return in_scenario2() ? MusicId::xana2dl_xana230 : MusicId::xana2_XANA2_SH;
}

} // namespace

const uint32_t EndingScene::SDL_ENDING_TIMER_EVENT = ::SDL_RegisterEvents(1);

const Rect EndingScene::kRectOverall{ 0, 0, 640, 400 };
// 幅は旧レイアウト(560)のまま、画面全体を使うよう中央寄せ・全高にする
// ((640-560)/2 = 40)
const Rect EndingScene::kRectEndingRoll{ 40, 0, 560, 400 };

EndingScene::EndingScene()
	: initialized_(false)
	, waitingForKey_(false)
	, rollActive_(false)
	, kanjiCodeTop_(kanjiCode_.cbegin())
	, msgY_(0)
	, msgRestRows_(0)
{
}

void EndingScene::dispatch(const SDL_Event &event)
{
    switch(event.type) {
    case SDL_EVENT_WINDOW_EXPOSED:
        onWindowExpose(event.window);
        break;

    case SDL_EVENT_KEY_DOWN:
        onKeyDown(event.key);
        break;

    default:
        if (event.type == SDL_ENDING_TIMER_EVENT) {
            onTimer();
        }
        break;
    }
}

void EndingScene::onCreate(uint32_t /*tick*/)
{
	initialized_ = false;
	waitingForKey_ = false;
	rollActive_ = false;
	kanjiCode_.clear();
	kanjiCodeTop_ = kanjiCode_.cbegin();
	msgY_ = 0;
	msgRestRows_ = 0;

	clipOverall_ = std::make_shared<SDL_::Image>(kRectOverall.getWidth(), kRectOverall.getHeight());
	clipEndingRoll_ = std::make_shared<SDL_::Image>(kRectEndingRoll.getWidth(), kRectEndingRoll.getHeight());
	visualImage_.reset();

	auto &app = getApplication();
	auto &mixer = app.getMixer();
	auto &res = getResources();

	// 描画する画像・再生する音声は先にすべてロードしておく
	res.loadImage(ImageId::picture_kanji);
	kanjiBase_ = res.getImage(ImageId::picture_kanji);
	for (int i = 0; i < kKanjiPageRow; i++) {
		for (int j = 0; j < kKanjiPageCol; j++) {
			kanji_[i][j] = SDL_::SubImage{
				kanjiBase_, Rect(j * kKanjiWidth, i * kKanjiHeight, kKanjiWidth, kKanjiHeight)};
		}
	}

	// メッセージ用のオフスクリーンバッファ
	msg_ = std::make_shared<SDL_::Image>(clipEndingRoll_->getWidth(),
	                                      clipEndingRoll_->getHeight() + kKanjiHeight);
	if (kanjiBase_) {
		msg_->setColorKey(kanjiBase_->getColorKey());
	}
	msg_->fillRect(SDL_::Color::BLACK);

	if (in_scenario2()) {
		res.loadImage(ImageId::xa2_ending_background);
	}

	res.loadMusic(mixer, mainThemeId());
	res.loadMusic(mixer, endingThemeId());
}

void EndingScene::onDestroy(uint32_t /*tick*/)
{
	auto &app = getApplication();
	auto &mixer = app.getMixer();
	auto &res = getResources();
	app.killTimer();

	mixer.stopMusic();

	res.unloadImage(ImageId::picture_kanji);
	if (in_scenario2()) {
		res.unloadImage(ImageId::xa2_ending_background);
	}
	res.unloadMusic(mainThemeId());
	res.unloadMusic(endingThemeId());

	kanjiBase_.reset();
	msg_.reset();
	clipOverall_.reset();
	clipEndingRoll_.reset();
	visualImage_.reset();
}

void EndingScene::onResume(uint32_t /*tick*/)
{
	onEnter();
}

void EndingScene::onSuspend()
{
	getApplication().killTimer();
}

void EndingScene::onKeyDown(const SDL_KeyboardEvent &key)
{
	if (key.repeat != 0) {
		return;
	}
	if (waitingForKey_) {
		// スクロール終了後は何を押してもメニューへ戻る
		restoreContext();
		return;
	}
	else if (
        (key.mod & SDL_KMOD_CTRL) != 0
        && key.key == SDLK_Q)
    {
		restoreContext();
    }
}

void EndingScene::onWindowExpose(const SDL_WindowEvent &window)
{
    onDraw();
}

void EndingScene::onEnter()
{
	auto &res = getResources();

	if (!initialized_) {
		if (!init()) {
			beep();
			restoreContext();
			return;
		}
		initialized_ = true;

		if (!in_scenario2()) {
			// 呼び出し元(MenuScene等)も独立した画像バッファへ移行済みで、
			// エンディング突入直前の画面を引き継ぐ手段がない。無理に前画面を
			// 取り込もうとせず、単純に黒背景としてそのままスクロールへ進む
			clipOverall_->fillRect(SDL_::Color::BLACK);
			startScroll();
		} else {
			visualImage_ = res.getImage(ImageId::xa2_ending_background);
			if (!visualImage_) {
				restoreContext();
				return;
			}
			startFade(clipOverall_, 0, 0, visualImage_, 0xffffff);
		}
	} else {
		startScroll();
	}
}

void EndingScene::startScroll()
{
	auto &mixer = getApplication().getMixer();
	auto &res = getResources();

	// もう一度スクリーンをコピー(クレジットロール背景として使う)
	visualImage_ = std::make_shared<SDL_::Image>(clipOverall_->getWidth(), clipOverall_->getHeight());
	visualImage_->blit(clipOverall_, 0, 0);

	mixer.playMusic(*res.getMusic(mainThemeId()), -1);
	rollActive_ = true;
	startTimer(kEndingInterval);
}

void EndingScene::startFade(std::shared_ptr<SDL_::Image> dst, int x, int y,
                             std::shared_ptr<SDL_::Image> img, unsigned rgb)
{
	fade_.start(dst, x, y, img, rgb);
	startTimer(kFadeInterval);
}

void EndingScene::startTimer(uint32_t interval)
{
	getApplication().setTimer(interval, [](Uint32 interval) -> Uint32 {
		SDL_Event event{};
		event.type = SDL_ENDING_TIMER_EVENT;
		::SDL_PushEvent(&event);
		return interval;
	});
}

void EndingScene::onTimer()
{
	// フェード中/スクロール中は互いに排他(どちらか一方だけがタイマーを
	// 使っている)ので、fade_の状態だけで振り分けられる
	if (!fade_.isDone()) {
		fade_.step();
		onDraw();
		if (fade_.isDone()) {
			getApplication().killTimer();
			onEnter(); // 旧resume_context()->onResume()->onEnter()の再入に相当
		}
		return;
	}

	loop();
}

void EndingScene::onDraw()
{
	auto &app = getApplication();
	auto mainWindow = app.getMainWindow();
	if (!mainWindow || !clipOverall_) {
		return;
	}
	if (rollActive_) {
		clipOverall_->blit(clipEndingRoll_, kRectEndingRoll.getXPos(), kRectEndingRoll.getYPos());
	}
	auto backBuffer = mainWindow->getBackBuffer();
	backBuffer->blitScaled(clipOverall_, nullptr, nullptr, SDL_SCALEMODE_PIXELART);
	mainWindow->swap();
}

void EndingScene::restoreContext()
{
	// フェード中/スクロール中に中断(Ctrl+Q)された場合に備え、念のためタイマーを止めておく
	getApplication().killTimer();

	kanjiCode_.clear();
	kanjiCodeTop_ = kanjiCode_.cbegin();

	visualImage_.reset();
	rollActive_ = false;

	resume_context();
}

bool EndingScene::init()
{
    fs::path path = IMAGE_DIR;
    path.append(!in_scenario2()?"xa1":"xa2");
    path.append("ending");
    path.append("message.txt");

	if (loadKanjiCode(path.c_str())) {
		return false;
	}

	return true;
}

void EndingScene::loop()
{
	msgY_ += 1;

	// 新しい行が完全に現れた?
	// scroll_image()はSDL_::Imageに同等のメソッドが無い低レベルなピクセル
	// シフト処理のため、graphics.hの関数のまま使う
	if (msgY_ > kKanjiHeight) {
		scroll_image(msg_, -msgY_);
		msgY_ = 0;
		if (kanjiCodeTop_ != kanjiCode_.cend()) {
			// 最下行に描画
			kanjiCodeTop_ = drawKanjiText(msg_, 0, msg_->getHeight() - kKanjiHeight, kanjiCodeTop_, kanjiCode_.cend());
		} else {
			msgRestRows_++;

			// 最後の行がスクリーンから見えなくなった?
			if (msgRestRows_ > msg_->getHeight() / kKanjiHeight) {
				waitForever();
			}
		}
	}

	clipEndingRoll_->blit(visualImage_, kRectEndingRoll, 0, 0);
	clipEndingRoll_->blit(msg_, 0, -msgY_);

	onDraw();
}

void EndingScene::waitForever()
{
	waitingForKey_ = true;
    auto &app = getApplication();
	app.killTimer();
	auto &mixer = app.getMixer();
	mixer.playMusic(*getResources().getMusic(endingThemeId()), -1);
}

int EndingScene::loadKanjiCode(const char *filename)
{
	std::ifstream ifs(filename);
	if (!ifs) {
		std::cerr << filename << ": " << std::strerror(errno) << '\n';
		return 1;
	}

	// message.txtは各行が-1で終端された漢字コード列。改行は無視して
	// フラットに読み込み、行区切りの-1もそのままバッファに残す
	// (drawKanjiText側で1行分ずつ読み飛ばす)
	kanjiCode_.assign(std::istream_iterator<int>(ifs), std::istream_iterator<int>());
	kanjiCodeTop_ = kanjiCode_.cbegin();

	return 0;
}

std::vector<int>::const_iterator EndingScene::drawKanjiText(
    std::shared_ptr<SDL_::Image> img, int x, int y,
    std::vector<int>::const_iterator first,
    std::vector<int>::const_iterator last
) const
{
	for (; first != last && *first >= 0; ++first) {
		const int row = *first / kKanjiPageCol;
		const int col = *first % kKanjiPageCol;
		if (row < kKanjiPageRow) {
			// kanjiBase_(kanji_[][].sheet)のcolorkeyを一時的に無効化して不透明合成
			// する(旧tmpl_draw相当)。Image::blit()はcolorkeyを尊重してしまうため
			// 単純に置き換えられず、ここは意図的にgraphics.hの関数のまま残す
			draw_image(img, x, y, kanji_[row][col]);
		}
		x += kKanjiWidth;
	}
	if (first != last) {
		++first; // 行区切りの-1を読み飛ばす
	}
	return first;
}
