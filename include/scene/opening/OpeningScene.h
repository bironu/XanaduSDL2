#if !defined(OPENINGSCENE_H_)
#define OPENINGSCENE_H_

#include "scene/Scene.h"

class OpeningScene final : public Scene
{
public:
	OpeningScene();

	void dispatch(const SDL_Event &event) override;
	void onSuspend() override;
	void onCreate(uint32_t tick) override;
	void onResume(uint32_t tick) override;

private:
    void onKeyDown(const SDL_KeyboardEvent &key);
    void onWindowExpose(const SDL_WindowEvent &window);

    // 旧opening_enter/wait_forever/restore_contextに対応
	void onEnter();
	void waitForever();
	void restoreContext();

	int step_;
	unsigned fadeMask_;
	// wait_forever()到達後、キー入力を待っている間だけtrueになる
	// (旧実装のthunk_key_event = opening_key_eventに対応)
	bool waitingForKey_;
};

#endif // OPENINGSCENE_H_
