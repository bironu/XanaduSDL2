#include "pause.h"
#include "keyboard.h"
#include "xanadu.h"

#include <SDL3/SDL_timer.h>

namespace {

constexpr uint32_t kMaxWaitMs = 500; // 旧WAIT_INTERVAL(begin_pauseのタイムアウト)

bool active_ = false;
std::function<bool()> isDone_;
std::function<void()> onComplete_;

} // namespace

void begin_wait(std::function<bool()> isDone, std::function<void()> onComplete, uint32_t maxWaitMs)
{
  kill_timer(); // 呼び出し元の周期処理を止める。再開はonCompleteの責任

  if (maxWaitMs > 0) {
    const uint32_t deadline = SDL_GetTicks() + maxWaitMs;
    auto inner = isDone;
    isDone = [inner, deadline]() { return inner() || SDL_GetTicks() >= deadline; };
  }

  active_ = true;
  isDone_ = std::move(isDone);
  onComplete_ = std::move(onComplete);
}

void begin_pause(int clearkey, std::function<void()> onComplete)
{
  const SDL_Scancode key = static_cast<SDL_Scancode>(clearkey);
  begin_wait([key]() { return !isKeyDown(key); }, std::move(onComplete), kMaxWaitMs);
}

bool pause_active()
{
  return active_;
}

void pause_update(uint32_t /*nowTick*/)
{
  if (!active_ || !isDone_ || !isDone_()) {
    return;
  }
  active_ = false;
  isDone_ = nullptr;
  auto onComplete = std::move(onComplete_);
  onComplete_ = nullptr;
  if (onComplete) {
    onComplete();
  }
}
