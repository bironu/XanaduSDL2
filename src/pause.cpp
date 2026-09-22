#include "xanadu.h"
#include "pause.h"

#define WAIT_INTERVAL		500	// 待ち時間

static int pause_interval;
static int pause_clearkey;
static int pause_time;
static void pause_loop(void);

int init_pause(int interval, int clearkey)
{
  pause_interval = interval;
  pause_clearkey = clearkey;
  pause_time = 0;
  return CONTEXT_PAUSE;
}

void pause_enter(void)
{
  set_timer(pause_interval, pause_loop);
}

void pause_leave(void)
{
  kill_timer();
}

void pause_loop(void)
{
  if (!isKeyDown(static_cast<SDL_Scancode>(pause_clearkey)) ||
      (pause_time += pause_interval) >= WAIT_INTERVAL)
    resume_context();
}