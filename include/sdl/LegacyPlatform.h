#if !defined(LEGACY_PLATFORM_H_)
#define LEGACY_PLATFORM_H_

class Resources;
union SDL_Event;
enum class SoundId;
enum class MusicId;

// 旧C実装(main.c / win32, x11の各main.c)が担っていた
//   ・画面バッファ(clip_overall等)の確保
//   ・タイマー/音声/描画のプラットフォームフック(update_region, playSound, draw_text 等)
// のうち、まだSDL3側に本実装がないものについての橋渡し。
//
// 画面バッファの確保とdraw_text(BitmapFont経由)、効果音・BGM(いずれも
// Resources::instance()のSoundId/MusicIdキャッシュ経由でSDL_::Mix_::Mixer/Audioを
// 使う)は実装済み。タイマーと、MIDI固有のテンポ/ピッチ制御(bgm_tempo/bgm_random。
// SDL3_mixerのMIX_Track APIには相当する機能が見当たらない)はまだ未実装。
void initLegacyGraphics(Resources &res);

// 効果音。mute中、または該当idが未ロードなら何もしない
int playSound(SoundId id); // 再生に使ったchannel(Mixer::isChannelPlaying用)を返す。ミュート/未ロード時は-1

// BGM。Resources::instance().playBgm()のApplication::instance()経由ラッパ
void playBgm(MusicId id);

// ミュート切り替え(0:鳴らす 1:鳴らさない)、テンポ/ピッチ制御(未実装の旧フック)
int bgm_mute(void);
void bgm_tempo(int tempo);
void bgm_random(int random_pitch_bend);

// clip_overall/clip_main等を合成し、実際にウィンドウへ描画する。
// Scene::swap()から毎フレーム呼び出される。
void presentLegacyFrame();

#endif // LEGACY_PLATFORM_H_
