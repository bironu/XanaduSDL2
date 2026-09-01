#include <windows.h>
#include "xanadu.h"

#include "rgb.c"

static image_t _clip_overall;
static image_t _clip_main;
static image_t _clip_message;
static image_t _clip_status;
static image_t _clip_shrine;
static image_t _clip_user_guage;
static image_t _clip_boss_guage;
static image_t _clip_endingroll;

image_t *clip_overall = &_clip_overall;
image_t *clip_main    = &_clip_main;
image_t *clip_message = &_clip_message;
image_t *clip_status  = &_clip_status;
image_t *clip_shrine  = &_clip_shrine;
image_t *clip_user_guage = &_clip_user_guage;
image_t *clip_boss_guage = &_clip_boss_guage;
image_t *clip_endingroll = &_clip_endingroll;

const rectangle_t rect_overall    = {   0,   0, 640, 400 };
const rectangle_t rect_main       = {  16,  16, 360, 360 };
const rectangle_t rect_message    = { 392, 304, 240,  80 };
const rectangle_t rect_status     = { 392,  16, 240, 272 };
const rectangle_t rect_shrine     = {  16,  96, 608, 240 };
const rectangle_t rect_user_guage = {  56,  32, 240,  40 };
const rectangle_t rect_boss_guage = { 384,  32, 240,  40 };
const rectangle_t rect_endingroll = {  80,  96, 560, 240 };

image_t frame_user[10];
image_t frame_monsters[N_MONSTERS][4];
image_t frame_magics[N_MAGICS * 2];
image_t frame_tiles[N_TILES];
image_t frame_goods[N_GOODS];
image_t frame_brownbox[4];
image_t frame_whitebox[4];
image_t frame_specials[N_SPECIALS];
image_t mask_damaged;
image_t pattern_guage;
image_t pattern_status;
image_t *visual_image;

static HINSTANCE app_instance;
static HWND main_window;
static HBITMAP offscreen;
static HANDLE thread_handle;
static DWORD thread_id;
static int main_window_W;
static int main_window_H;

static const RECT null_rect = { 640, 400, 0, 0 };
static RECT update_section;
static int screen_updated;
static void (*timer_proc)(void);
static int timer_interval;

static HANDLE midi_server;
static HANDLE wave_server;
static const char *current_bgm_file;
static int current_tempo;

static LRESULT CALLBACK main_wnd_proc(HWND, UINT, WPARAM, LPARAM);
static DWORD WINAPI thread_proc(LPVOID);

static HBITMAP create_dib_section(int width, int height, image_t *img)
{
  BITMAPINFO *bmi;  
  BITMAPINFOHEADER *bi;
  HDC dc;
  HANDLE bmp;
  int i;
  
  img->width  = width;
  img->height = height;
  img->bytes_per_line = (width + 3) & ~3;
  
  bmi = (BITMAPINFO *)malloc(sizeof(BITMAPINFO) + 256 * sizeof(RGBQUAD));
  bi = &bmi->bmiHeader;
  bi->biSize		= sizeof(*bi);
  bi->biWidth		= img->bytes_per_line;
  bi->biHeight		= -img->height;
  bi->biPlanes		= 1;
  bi->biBitCount	= 8;
  bi->biCompression	= BI_RGB;
  bi->biSizeImage	= 0;
  bi->biXPelsPerMeter	= 0;
  bi->biXPelsPerMeter	= 0;
  bi->biClrUsed		= 256;
  bi->biClrImportant	= 0;

  for (i = 0; i < 256; i++) {
    bmi->bmiColors[i].rgbRed   = colors[i].red;
    bmi->bmiColors[i].rgbGreen = colors[i].green;
    bmi->bmiColors[i].rgbBlue  = colors[i].blue;
    bmi->bmiColors[i].rgbReserved = 0;
  }
  
  dc = GetDC(0);
  bmp = CreateDIBSection(dc, bmi, DIB_RGB_COLORS, (void **)&img->data, NULL, 0);
  ReleaseDC(0, dc);
  free(bmi);
  return bmp;
}

static HANDLE open_pipe(const char *filename)
{
  STARTUPINFO si;
  PROCESS_INFORMATION process_info;
  int error;
  HANDLE backup_stdin, pipe_in, pipe_out;
  
  backup_stdin = GetStdHandle(STD_INPUT_HANDLE);
  CreatePipe(&pipe_in, &pipe_out, NULL, BUFSIZ);
  SetStdHandle(STD_INPUT_HANDLE, pipe_in);

  memset(&si, 0, sizeof(si));
  si.cb = sizeof(si);
  si.dwFlags = 0;

  error = CreateProcess(filename,
                        NULL,
                        NULL,
                        NULL,
                        TRUE,
                        DETACHED_PROCESS,
                        NULL,
                        NULL,
                        &si,
                        &process_info);

  CloseHandle(process_info.hProcess);
  CloseHandle(process_info.hThread);
  
  if (error == FALSE) {
    MessageBeep(0);
  }
  CloseHandle(pipe_in);
  SetStdHandle(STD_INPUT_HANDLE, backup_stdin);
  return pipe_out;
}

static void write_pipe(HANDLE pipe, const char *fmt, ...)
{
  va_list args;

  if (pipe) {
    DWORD num_writes;
    char buf[BUFSIZ];
    
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);

    WriteFile(pipe, buf, strlen(buf), &num_writes, NULL);
  }
}

static int init(void)
{
  static image_t *magic_base, *goods_base, *pattern_base, *special_base;
  static image_t *damage_mask;
  int i;
  
  for (i = 0; i < 256; i++) {
    colors[i].red   = rgb_R(default_rgb[i]);
    colors[i].green = rgb_G(default_rgb[i]);
    colors[i].blue  = rgb_B(default_rgb[i]);
    colors[i].pixel = i;
  }
  
  if (init_graphics(8)) {
    return 1;
  }
  
  offscreen = create_dib_section(640, 400, clip_overall);
  if (!offscreen) {
    fprintf(stderr, "Can't create offscreen bitmap.\n");
    return 1;
  }
  subsection_image(clip_overall,  16,  16, 360, 360, clip_main);
  subsection_image(clip_overall, 392, 304, 240,  80, clip_message);
  subsection_image(clip_overall, 392,  16, 240, 272, clip_status);
  subsection_image(clip_overall,  16,  96, 608, 240, clip_shrine);
  subsection_image(clip_overall,  80,  96, 560, 240, clip_endingroll);
  
  load_user_image();
  init_level(0, NULL);

  magic_base = load_image(make_path(IMAGE_DIR, "user\\magic.bmp"));
  for (i = 0; i < N_MAGICS * 2; i++) {
    subsection_image(magic_base, i * 16, 0, 16, 16, &frame_magics[i]);
  }

  special_base = load_image(make_path(IMAGE_DIR, "user\\effect.bmp"));
  for (i = 0; i < N_SPECIALS; i++) {
    subsection_image(special_base, i * 40, 0, 40, 40, &frame_specials[i]);
  }
  
  goods_base = load_image(make_path(IMAGE_DIR, "user\\goods.bmp"));
  for (i = 0; i < N_GOODS; i++) {
    subsection_image(goods_base, i * 40, 0, 40, 40, &frame_goods[i]);
  }
  for (i = 0; i < 4; i++) {
    int n;
    n = index_whitebox[i];
    subsection_image(goods_base, n * 40, 0, 40, 40, &frame_whitebox[i]);
    n = index_brownbox[i];
    subsection_image(goods_base, n * 40, 0, 40, 40, &frame_brownbox[i]);
  }
  
  pattern_base = load_image(make_path(IMAGE_DIR, "user\\pattern.bmp"));
  subsection_image(pattern_base,  0, 0, 16, 16, &pattern_status);
  subsection_image(pattern_base, 16, 0, 16, 16, &pattern_guage);

  damage_mask = load_image(make_path(IMAGE_DIR, "user\\damage.bmp"));
  subsection_image(damage_mask,  0,  0, 40, 40, &mask_damaged);
  
  if (init_bgm() == 0) {
    midi_server = open_pipe("midiplay.exe");
    write_pipe(midi_server, "d%s\n", make_path(AUDIO_DIR, "midi"));
  }
  if (init_se() == 0) {
    wave_server = open_pipe("waveplay.exe");
    write_pipe(wave_server, "d%s\n", make_path(AUDIO_DIR, "wave"));
    se_load(SE_USER_HIT,	se_data.user_hit[0]);
    se_load(SE_MAGIC_HIT,	se_data.magic_hit);
    se_load(SE_DAMAGED,		se_data.damaged);
    se_load(SE_MONSTER_DEAD,	se_data.monster_dead);
    se_load(SE_OPEN_LOCKED,	se_data.open_locked);
    se_load(SE_OPEN_BOX,	se_data.open_box);
    se_load(SE_TREASURE,	se_data.treasure);
    se_load(SE_GET,		se_data.get);
    se_load(SE_GET_POISON,	se_data.get_poison);
    se_load(SE_LOST_KEY,	se_data.lost_key);
    se_load(SE_CAST_NEEDLE,	se_data.cast[0]);
    se_load(SE_CAST_MITTAR,	se_data.cast[1]);
    se_load(SE_CAST_DELUGE,	se_data.cast[2]);
    se_load(SE_CAST_FIRE,	se_data.cast[3]);
    se_load(SE_CAST_THUNDER,	se_data.cast[4]);
    se_load(SE_CAST_POISON,	se_data.cast[5]);
    se_load(SE_CAST_CORROSION,	se_data.cast[6]);
    se_load(SE_CAST_TILTE,	se_data.cast[7]);
    se_load(SE_CAST_DEATH,	se_data.cast[8]);
    se_load(SE_USE_ITEM,	se_data.use_item[0]);
  }

  if (load_background(make_path(IMAGE_DIR, "user\\frame.bmp"))) {
    fprintf(stderr, "Can't load background.\n");
  }
  return 0;
}

static ATOM register_window_class(HINSTANCE instance)
{
  WNDCLASSEX wc;

  wc.cbSize        = sizeof(wc);
  wc.style         = 0;
  wc.lpfnWndProc   = (WNDPROC)main_wnd_proc;
  wc.cbClsExtra	   = 0;
  wc.cbWndExtra	   = 0;
  wc.hInstance	   = instance;
  wc.hIcon	   = NULL;/*LoadIcon(app_instance, MAKEINTRESOURCE(IDI_ICON1));*/
  wc.hCursor	   = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = NULL;
  wc.lpszMenuName  = NULL;
  wc.lpszClassName = "main-window";
  wc.hIconSm	   = NULL;
  
  return RegisterClassEx(&wc);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmd, int cmd_show)
{
  MSG msg;

  app_instance = instance;

  if (register_window_class(app_instance) == 0) {
    fprintf(stderr, "Can't register a window class.\n");
    return 1;
  }

  main_window_W = rect_overall.width + GetSystemMetrics(SM_CXFRAME) * 2;
  main_window_H = rect_overall.height + GetSystemMetrics(SM_CYFRAME)
    + GetSystemMetrics(SM_CYCAPTION);

  main_window = CreateWindow("main-window", "Xanadu",
                             WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX,
                             CW_USEDEFAULT, CW_USEDEFAULT,
                             main_window_W, main_window_H,
                             NULL,
                             NULL,
                             app_instance, NULL);
  if (!main_window) {
    fprintf(stderr, "Can't create a window.\n");
    return 1;
  }
  
  thread_handle = CreateThread(NULL, 0, thread_proc, NULL,
                               CREATE_SUSPENDED, &thread_id);
  if (!thread_handle) {
    fprintf(stderr, "Can't create a thread.\n");
    return 1;
  }

  if (init()) {
    return 1;
  }

  reset_context();
  
  ShowWindow(main_window, cmd_show);
  UpdateWindow(main_window);
  
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  
  return msg.wParam;
}

LRESULT CALLBACK main_wnd_proc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  if (msg == WM_PAINT) {
    PAINTSTRUCT ps;
    HDC dc, mem_dc;
    int x, y, w, h;

    dc = BeginPaint(wnd, &ps);
    mem_dc = CreateCompatibleDC(dc);
    SelectObject(mem_dc, offscreen);
    x = ps.rcPaint.left;
    y = ps.rcPaint.top;
    w = ps.rcPaint.right - x;
    h = ps.rcPaint.bottom - y;
    BitBlt(dc, x, y, w, h, mem_dc, x, y, SRCCOPY);
    DeleteDC(mem_dc);
    EndPaint(wnd, &ps);
    
    return 0;
  }
  
  switch (msg) {
  case WM_CREATE:
    return 0;
    
  case WM_DESTROY:
    /* コンテキストの退出保護を呼ぶ */
    switch_context(CONTEXT_NULL);
    
    CloseHandle(thread_handle);
    DeleteObject(offscreen);
    PostQuitMessage(0);

    return 0;
    
  case WM_GETMINMAXINFO:
    {
      MINMAXINFO *mmi = (MINMAXINFO *)lparam;
      mmi->ptMaxTrackSize.x = main_window_W;
      mmi->ptMaxTrackSize.y = main_window_H;
      mmi->ptMinTrackSize.x = main_window_W;
      mmi->ptMinTrackSize.y = main_window_H;
    }
    return 0;

  case WM_NCHITTEST:
    {
      int ht = DefWindowProc(wnd, msg, wparam, lparam);
      switch (ht) {
      case HTTOPLEFT:	case HTTOP:	case HTTOPRIGHT:
      case HTLEFT:			case HTRIGHT:
      case HTBOTTOMLEFT:case HTBOTTOM:	case HTBOTTOMRIGHT:        
        ht = HTBORDER;
      }
      return ht;
    }

  case WM_KEYDOWN:
    {
      if (thunk_key_event) {
        int vkey = (int)wparam;
        WORD c;
        char buf[256];

        GetKeyboardState(buf);
        
        if (ToAscii(vkey, 0, buf, &c, 0) == 1) {
          (*thunk_key_event)((int)c);
        } else {
          (*thunk_key_event)('\0');
        }
      }
    }
    return 0;

  case WM_ACTIVATE:
    {
      int active = LOWORD(wparam);

      if ((active & WA_ACTIVE) || (active & WA_CLICKACTIVE)) {
        bgm_restart();
        ResumeThread(thread_handle);
      } else {
        bgm_pause();
        SuspendThread(thread_handle);
      }
    }
    return 0;
  }
  return DefWindowProc(wnd, msg, wparam, lparam);
}

int load_background(const char *filename)
{
  image_t *img;
  char path[BUFSIZ];

  sprintf(path, IMAGE_DIR "\\%s", filename);
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
  RECT r = update_section;
  
  r.left   = min(r.left,   x);
  r.top    = min(r.top,    y);
  r.right  = max(r.right,  x + width);
  r.bottom = max(r.bottom, y + height);

  update_section = r;
  screen_updated = 1;
}

void update_immediately(void)
{
  if (screen_updated) {
    InvalidateRect(main_window, &update_section, FALSE);
    SendMessage(main_window, WM_PAINT, 0, 0);
    screen_updated = 0;
    update_section = null_rect;
  }
}

DWORD WINAPI thread_proc(LPVOID null)
{
  int time_spent = 0;
  for (;;) {
    int interval = timer_interval;

    if (time_spent <= interval) {
      interval -= time_spent;
    } else {
      interval = min(timer_interval * 2, time_spent - interval);
    }

    SleepEx(interval, FALSE);
    time_spent = GetTickCount();
    
    if (timer_proc) {
      (*timer_proc)();
    }
    if (screen_updated) {
      InvalidateRect(main_window, &update_section, FALSE);
      SendMessage(main_window, WM_PAINT, 0, 0);
      screen_updated = 0;
      update_section = null_rect;
    }
    time_spent = GetTickCount() - time_spent;
  }
}

void set_timer(int interval, void (*proc)(void))
{
  timer_interval = max(interval, MIN_INTERVAL);
  timer_proc = proc;
  ResumeThread(thread_handle);
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
  if (!filename) {
    return;
  }
  if (!current_bgm_file || strcmp(filename, current_bgm_file) != 0) {
    current_bgm_file = filename;
    if (*filename == '\0') {
      bgm_stop(); /* See audio/midi.txt */
    } else {
      if (!user.config.mute) {
        write_pipe(midi_server, "@%s\n", filename);
        current_tempo = 0;
      }
    }
  }
}

void bgm_stop(void)
{
  write_pipe(midi_server, "!\n");
}

void bgm_pause(void)
{
  write_pipe(midi_server, "P\n");
}

void bgm_restart(void)
{
  write_pipe(midi_server, "R\n");
}

/* Change tempo; 1:Quickly 0:Normal -1:Slowly */
void bgm_tempo(int tempo)
{
  if (tempo != current_tempo) {
    if (tempo == 0) {
      write_pipe(midi_server, "T\n");
    } else if (tempo > 0) {
      write_pipe(midi_server, "Q\n");
    } else {
      write_pipe(midi_server, "S\n");
    }
    current_tempo = tempo;
  }
}

int bgm_mute(void)
{
  user.config.mute = !user.config.mute;
  if (user.config.mute) {
    bgm_stop();
  } else {
    if (current_bgm_file && *current_bgm_file) {
      write_pipe(midi_server, "@%s\n", current_bgm_file);
    }
  }
  return user.config.mute;
}

void se_play(int id)
{
  if (!user.config.mute) {
    DWORD num_writes;
    char buf[] = "!*\n";

    buf[1] = 'A' + id;
    WriteFile(wave_server, buf, 3, &num_writes, NULL);
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
  MessageBeep(-1);
}
