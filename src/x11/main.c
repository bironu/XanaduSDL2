#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/extensions/XShm.h>

#include <unistd.h>
#include <signal.h>
#include <stdarg.h>
#include <ctype.h>

#include "xanadu.h"

#include "../rgb.c"
#include "../init.c"

int keystate_vector[256];

static Display *display;
static Window main_window;
static GC gc;
static Colormap colormap;
static XImage *offscreen;
static int Completion;

static Atom WM_PROTOCOLS;
static Atom WM_DELETE_WINDOW;

static const rectangle_t null_rect = { 640, 400, 0, 0 };
static rectangle_t *update_section;
static int waiting_completion;

static void (*timer_proc)(void);
static long timer_interval = MIN_INTERVAL;

#if 0
static const char *current_bgm_file;
static int current_tempo;
#endif

static int midi_server = -1;
static pid_t midi_server_pid;

static int wave_server = -1;
static pid_t wave_server_pid;

static void change_keystate(KeySym keysym, unsigned modifiers, int state);
static long process_event(XEvent *e);

typedef struct xshminfo {
  XShmSegmentInfo shminfo;
  XImage *ximage;
  struct xshminfo *next;
} xshminfo_t;

static xshminfo_t *cleanup_list;

static void cleanup_images(void)
{
  xshminfo_t *shm;

  for (shm = cleanup_list; shm; shm = shm->next) {
    XShmDetach(display, &shm->shminfo);
    XDestroyImage(shm->ximage);
    shmdt(shm->shminfo.shmaddr);
    shmctl(shm->shminfo.shmid, IPC_RMID, 0);
  }
}

static XImage *create_xshm_image(int width, int height)
{
  xshminfo_t *shm;

  shm = (xshminfo_t *)malloc(sizeof(xshminfo_t));
  if (!shm) {
    fprintf(stderr, "Memory exhausted.\n");
    exit(EXIT_FAILURE);
  }
  shm->ximage = XShmCreateImage(display,
				DefaultVisual(display, 0),
				DefaultDepth(display, 0),
				ZPixmap,
				NULL,
				&shm->shminfo, width, height);
  if (!shm->ximage) {
    fprintf(stderr, "Can't create an image.\n");
    exit(EXIT_FAILURE);
  }
  shm->shminfo.shmid = shmget(IPC_PRIVATE,
			      shm->ximage->bytes_per_line * shm->ximage->height,
			      IPC_CREAT | 0777);
  if (shm->shminfo.shmid < 0) {
    perror("shmget()");
    exit(EXIT_FAILURE);
  }
  shm->shminfo.shmaddr = shm->ximage->data = shmat(shm->shminfo.shmid, 0, 0);
  shm->shminfo.readOnly = False;
  shm->next = cleanup_list;
  cleanup_list = shm;

  if (!XShmAttach(display, &shm->shminfo)) {
    fprintf(stderr, "XShmAttach failed.\n");
    exit(EXIT_FAILURE);
  }
  return shm->ximage;
}

static pixel_t get_pixel(unsigned R, unsigned G, unsigned B)
{
  XColor xc;
  xc.red = R << 8;
  xc.green = G << 8;
  xc.blue = B << 8;
  XAllocColor(display, colormap, &xc);
  return xc.pixel;
}

static int open_pipe(char *path, pid_t *pid_return)
{
  char *argv[2];
  int fds[2];
  pid_t pid;

  if (pipe(fds) == -1) {
    perror("pipe()");
    return -1;
  }

  if ((pid = fork()) == 0) {
    close(fds[1]); /* 書き込み用端子を閉じる */
    dup2(fds[0], 0); /* 読み込み用端子を標準入力にリダイレクト */
    argv[0] = path;
    argv[1] = NULL;
    if (execvp(path, argv) == -1) {
      perror("exec()");
      _exit(1);
    }
  } else {
    *pid_return = pid;
#if 0
    close(fds[0]); /* 読み込み用端子を閉じる */
#endif
    return fds[1]; /* 書き込み用端子を返す */
  }
  return -1;
}

static void write_pipe(int fd, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);

  if (fd >= 0) {
    char buf[BUFSIZ];
    int n;
    n = vsprintf(buf, fmt, args);
    write(fd, buf, n);
  }
  va_end(args);
}

static void signal_child(int signo)
{
  pid_t pid = waitpid(-1, NULL, WNOHANG);
  if (pid > 0) {
    if (pid == wave_server_pid) {
      wave_server = -1;
      wave_server_pid = 0;
      printf("wave-server closed. wave_server: %d\n", wave_server);
    }
    if (pid == midi_server_pid) {
      midi_server = -1;
      midi_server_pid = 0;
    }
  }
}

static int start(void)
{
  int i;
  static image_t _overall;

#if 0
  if (init_bgm() == 0) {
    midi_server = open_pipe("./midiplay", &midi_server_pid);
    write_pipe(midi_server, "d%s/midi\n", AUDIO_DIR);
  }
#endif

#if 0
  if (init_se() == 0) {
    wave_server = open_pipe("./waveplay", &wave_server_pid);
    if (wave_server >= 0) {
      write_pipe(wave_server, "d%s/wave\n", AUDIO_DIR);
      se_load(SE_USER_HIT,		se_data.user_hit[0]);
      se_load(SE_MAGIC_HIT,		se_data.magic_hit);
      se_load(SE_DAMAGED,		se_data.damaged);
      se_load(SE_MONSTER_DEAD,		se_data.monster_dead);
      se_load(SE_OPEN_LOCKED,		se_data.open_locked);
      se_load(SE_OPEN_BOX,		se_data.open_box);
      se_load(SE_TREASURE,		se_data.treasure);
      se_load(SE_GET,			se_data.get);
      se_load(SE_GET_POISON,		se_data.get_poison);
      se_load(SE_LOST_KEY,		se_data.lost_key);
      se_load(SE_CAST_NEEDLE,		se_data.cast[0]);
      se_load(SE_CAST_MITTAR,		se_data.cast[1]);
      se_load(SE_CAST_DELUGE,		se_data.cast[2]);
      se_load(SE_CAST_FIRE,		se_data.cast[3]);
      se_load(SE_CAST_THUNDER,		se_data.cast[4]);
      se_load(SE_CAST_POISON,		se_data.cast[5]);
      se_load(SE_CAST_CORROSION,	se_data.cast[6]);
      se_load(SE_CAST_TILTE,		se_data.cast[7]);
      se_load(SE_CAST_DEATH,		se_data.cast[8]);
      se_load(SE_USE_ITEM,		se_data.use_item[0]);
    }
  }
#endif

  if (!XShmQueryExtension(display)) {
    fprintf(stderr, "MIT-SHM extension not supported.\n");
    return 1;
  }
  atexit(cleanup_images);
  
  for (i = 0; i < 256; i++) {
    colors[i].red   = rgb_R(default_rgb[i]);
    colors[i].green = rgb_G(default_rgb[i]);
    colors[i].blue  = rgb_B(default_rgb[i]);
    colors[i].pixel = get_pixel(colors[i].red, colors[i].green, colors[i].blue);
  }
  
  offscreen = create_xshm_image(640, 400);
  if (!offscreen) {
    fprintf(stderr, "Can't create offscreen bitmap.\n");
    return 1;
  }
  _overall.width = offscreen->width;
  _overall.height = offscreen->height;
  _overall.bytes_per_line = offscreen->bytes_per_line;
  _overall.data = offscreen->data;
  
  clip_overall = &_overall;
  
  if (init_graphics(offscreen->bits_per_pixel)) {
    fprintf(stderr, "init_graphics() failed.\n");
    return 1;
  }

  return init();
}

int main(int argc, char *argv[])
{
  XGCValues gc_values;
  XSizeHints hint;
  XEvent e;
  struct timeval ago, now;

  signal(SIGCHLD, signal_child);

  display = XOpenDisplay(NULL);
  if (!display) {
    fprintf(stderr, "Can't open display.\n");
    exit(EXIT_FAILURE);
  }

  colormap = DefaultColormap(display, 0);

  main_window = XCreateSimpleWindow(display, DefaultRootWindow(display),
				    0, 0, 640, 400, 1,
				    WhitePixel(display, 0),
				    BlackPixel(display, 0));
  XSelectInput(display, main_window,
	       ExposureMask | KeyPressMask | KeyReleaseMask);

  gc = XCreateGC(display, main_window, 0, &gc_values);

  WM_PROTOCOLS = XInternAtom(display, "WM_PROTOCOLS", False);
  WM_DELETE_WINDOW = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, main_window, &WM_DELETE_WINDOW, 1);

  hint.width = hint.min_width = hint.max_width = 640;
  hint.height = hint.min_height = hint.max_height = 400;
  hint.flags = PSize | PMinSize | PMaxSize;
  XSetWMNormalHints(display, main_window, &hint);

  XStoreName(display, main_window, "Xanadu");

  if (start()) {
    return 1;
  }
  reset_context();

  Completion = XShmGetEventBase(display) + ShmCompletion;

  XMapWindow(display, main_window);

  gettimeofday(&ago, NULL);
  for (;;) {
    long interval;

    /* イベント？ */
    while (XPending(display)) {
      XNextEvent(display, &e);
      process_event(&e);
    }
    usleep(1024);

    gettimeofday(&now, NULL);

    if (now.tv_usec < ago.tv_usec) {
      interval = now.tv_usec - ago.tv_usec + 1000000;
    } else {
      interval = now.tv_usec - ago.tv_usec;
    }
    if (interval / 912 >= timer_interval) {
#if 0
      printf("interval=%dusec(%dmsec), timer_interval=%dmsc\n",
	     interval, interval / 1024, timer_interval);
#endif
      if (timer_proc) {
	(*timer_proc)();
      }
      gettimeofday(&ago, NULL);
    }

    /* スクリーン更新 */
    update_immediately();
  }
}

long process_event(XEvent *e)
{
  struct timeval ago, now;
  KeySym keysym;

  gettimeofday(&ago, NULL);

  if (e->type == Completion) {
    waiting_completion = 0;
  } else {
    switch (e->type) {
    case Expose:
      update_region(e->xexpose.x, e->xexpose.y, e->xexpose.width, e->xexpose.height);
      break;
      
    case KeyPress:
      keysym = XKeycodeToKeysym(display, e->xkey.keycode, 0);
      change_keystate(keysym, e->xkey.state, 1);
      if (thunk_key_event) {
	if (0x20 <= keysym && keysym <= 0x7f) {
	  int c = (int)keysym;
	  if (e->xkey.state & ShiftMask) {
	    c = toupper(c);
	  }
	  (*thunk_key_event)(c);
	} else if (keysym == XK_Return || keysym == XK_KP_Enter) {
	  (*thunk_key_event)('\r');
	} else if (keysym == XK_BackSpace) {
	  (*thunk_key_event)(0x08);
	} else {
	  (*thunk_key_event)(0x00);
	}
      }
      break;
      
    case KeyRelease:
      keysym = XKeycodeToKeysym(display, e->xkey.keycode, 0);
      change_keystate(keysym, e->xkey.state, 0);
      break;
   
    case ClientMessage:
      if (e->xclient.message_type == WM_PROTOCOLS &&
	  e->xclient.data.l[0] == WM_DELETE_WINDOW) {
	exit(EXIT_SUCCESS);
      }
      break;
    }
  }

  gettimeofday(&now, NULL);

  /* 秒は無視してかまわない */
  if (now.tv_usec < ago.tv_usec) {
    return now.tv_usec - ago.tv_usec + 1000000;
  } else {
    return now.tv_usec - ago.tv_usec;
  }
}

void change_keystate(KeySym keysym, unsigned modifiers, int state)
{
  if (modifiers & ShiftMask) {
    keystate_vector[VK_SHIFT] = state;
  }
  if (modifiers & ControlMask) {
    keystate_vector[VK_CONTROL] = state;
  }
  switch (keysym) {
  case XK_Return:    keystate_vector[VK_RETURN] = state; return;
  case XK_Escape:    keystate_vector[VK_ESCAPE] = state; return;

  case XK_KP_Home:   keystate_vector[VK_HOME]   = state; return;
  case XK_KP_Left:   keystate_vector[VK_LEFT]   = state; return;
  case XK_KP_Up:     keystate_vector[VK_UP]     = state; return;
  case XK_KP_Right:  keystate_vector[VK_RIGHT]  = state; return;
  case XK_KP_Down:   keystate_vector[VK_DOWN]   = state; return;
  case XK_KP_Prior:  keystate_vector[VK_PRIOR]  = state; return;
  case XK_KP_Next:   keystate_vector[VK_NEXT]   = state; return;
  case XK_KP_End:    keystate_vector[VK_END]    = state; return;
  case XK_KP_Enter:  keystate_vector[VK_RETURN] = state; return;

  case XK_H: case XK_h: keystate_vector[VK_LEFT]  = state; return;
  case XK_J: case XK_j: keystate_vector[VK_DOWN]  = state; return;
  case XK_K: case XK_k: keystate_vector[VK_UP]    = state; return;
  case XK_L: case XK_l: keystate_vector[VK_RIGHT] = state; return;
  case XK_Y: case XK_y: keystate_vector[VK_HOME]  = state; return;
  case XK_U: case XK_u: keystate_vector[VK_PRIOR] = state; return;
  case XK_B: case XK_b: keystate_vector[VK_END]   = state; return;
  case XK_N: case XK_n: keystate_vector[VK_NEXT]  = state; return;
  }
  if (keysym == XK_space) {
    keystate_vector[VK_SPACE] = state;
  }
  else if (XK_A <= keysym && keysym <= XK_Z) {
    keystate_vector[keysym] = state;
    keystate_vector[keysym - 'A' + 'a'] = state;
  }
  else if (XK_a <= keysym && keysym <= XK_z) {
    keystate_vector[keysym] = state;
    keystate_vector[keysym - 'a' + 'A'] = state;
  }
}

int load_background(const char *filename)
{
  image_t *img;
  char path[BUFSIZ];

  sprintf(path, IMAGE_DIR "/%s", filename);
  img = load_image(filename);
  if (!img) {
    return 1;
  } else {
    draw_image(clip_overall, 0, 0, img);
    update(rect_overall);
    free(img);
    return 0;
  }
}

void update_region(int x, int y, int width, int height)
{
  static rectangle_t r;

  if (!update_section) {
    r.x = x;
    r.y = y;
    r.width = width;
    r.height = height;
    update_section = &r;
  } else {
    int left = min(r.x, x);
    int top = min(r.y, y);
    int right = max(r.x + r.width, x + width);
    int bottom = max(r.y + r.height, y + height);
    r.x = left;
    r.y = top;
    r.width = right - left;
    r.height = bottom - top;
  }
}

void update_immediately(void)
{
#if 0
  if (update_section && !waiting_completion) {
#else
  if (update_section) {
#endif
    XShmPutImage(display, main_window, gc, offscreen,
		 update_section->x,
		 update_section->y,
		 update_section->x,
		 update_section->y,
		 update_section->width,
		 update_section->height, True);
    update_section = NULL;
    waiting_completion = 1;
  }
}

void set_timer(int interval, void (*proc)(void))
{
  timer_interval = max(interval, MIN_INTERVAL);
  timer_proc = proc;
}

void kill_timer(void)
{
  timer_proc = NULL;
  timer_interval = MIN_INTERVAL;
}

void set_timer_proc(void (*proc)(void))
{
  timer_proc = proc;
}

void bgm_play(const char *filename)
{
}

void bgm_stop(void)
{
}

void bgm_pause(void)
{
}

void bgm_restart(void)
{
}

/* Change tempo; 1:Quickly 0:Normal -1:Slowly */
void bgm_tempo(int tempo)
{
}

int bgm_mute(void)
{
  user.config.mute = !user.config.mute;
  return user.config.mute;
}

void bgm_random(int random_pitch_bend)
{
  if (random_pitch_bend) {
    write_pipe(midi_server, "-\n");
  } else {
    write_pipe(midi_server, "+\n");
  }
}

void se_play(int id)
{
  if (!user.config.mute && wave_server >= 0) {
    char buf[4];
    buf[0] = '!';
    buf[1] = 'A' + id;
    buf[2] = '\n';
    buf[3] = '\0';
    write(wave_server, buf, 3);
  }
}

void se_load(int id, const char *filename)
{
  if (filename) {
    write_pipe(wave_server, "@%c%s\n", 'A' + id, filename);
  }
}

void beep(void)
{
  XBell(display, 50);
}

int bgm_enabled(void)
{
  return midi_server != NULL;
}

int se_enabled(void)
{
  return wave_server != NULL;
}
