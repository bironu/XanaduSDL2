#include "pause.h"
#include "keyboard.h"
#include "xanadu.h"

#include <SDL3/SDL_timer.h>

namespace {

constexpr uint32_t kMaxWaitMs = 500; // 旧WAIT_INTERVAL

bool active_ = false;
uint32_t startTick_ = 0;
SDL_Scancode clearKey_ = SDL_SCANCODE_UNKNOWN;
std::function<void()> onComplete_;

} // namespace

void begin_pause(int clearkey, std::function<void()> onComplete)
{
  kill_timer(); // 呼び出し元の周期処理を止める。再開はonCompleteの責任
  active_ = true;
  startTick_ = SDL_GetTicks();
  clearKey_ = static_cast<SDL_Scancode>(clearkey);
  onComplete_ = std::move(onComplete);
}

bool pause_active()
{
  return active_;
}

void pause_update(uint32_t nowTick)
{
  if (!active_) {
    return;
  }
  if (!isKeyDown(clearKey_) || nowTick - startTick_ >= kMaxWaitMs) {
    active_ = false;
    auto onComplete = std::move(onComplete_);
    onComplete_ = nullptr;
    if (onComplete) {
      onComplete();
    }
  }
}
