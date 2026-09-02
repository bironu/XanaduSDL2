#if !defined(LEGACY_PLATFORM_H_)
#define LEGACY_PLATFORM_H_

class Resources;

// 旧C実装(main.c / win32, x11の各main.c)が担っていた
//   ・画面バッファ(clip_overall等)の確保
//   ・タイマー/音声/描画のプラットフォームフック(update_region, bgm_play, se_play, draw_text 等)
// のうち、まだSDL2側に本実装がないものについての橋渡し。
//
// 画面バッファの確保とdraw_text(BitmapFont経由)、効果音(se_play/se_load)、
// BGM(bgm_play等、MIDI)は、いずれもSDL_::Mix_::Audio/Chunk/Music経由で
// 実装済み。タイマーと、MIDI固有のテンポ/ピッチ制御(bgm_tempo/bgm_random。
// SDL_mixerのMix_Music APIには相当する機能がない)はまだ未実装。
void initLegacyGraphics(Resources &res);

// 音声サブシステムの初期化(サウンドフォントの設定、wave.txt/midi.txtの読み込み)。
// initLegacyGraphicsとは別に、Application(Mix_::Audio)の生成後に呼び出すこと。
void initLegacySound(Resources &res);

// clip_overall/clip_main等を合成し、実際にウィンドウへ描画する。
// Scene::swap()から毎フレーム呼び出される。
void presentLegacyFrame();

#endif // LEGACY_PLATFORM_H_
