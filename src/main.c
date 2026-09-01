#include <windows.h>
#include "xanadu.h"

#include "xanadu.rc"

#include "rgb.c"
#include "init.c"

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

static HBITMAP create_dib_section(int width, int height, int bits_per_pixel,
                                  image_t **img_return)
{
  BITMAPINFO *bmi;  
  BITMAPINFOHEADER *bi;
  HDC dc;
  HANDLE bmp;
  image_t *img;
  int i, n_colors;

  img = create_image(width, height);
  if (!img) {
    fprintf(stderr, "Can't create an image.\n");
    return NULL;
  }

  n_colors = bits_per_pixel == 8 ? 256 : 0;
  
  bmi = (BITMAPINFO *)malloc(sizeof(BITMAPINFO) + n_colors * sizeof(RGBQUAD));
  bi = &bmi->bmiHeader;
  bi->biSize		= sizeof(*bi);
  bi->biWidth		= img->bytes_per_line / (bits_per_pixel / 8);
  bi->biHeight		= -img->height;
  bi->biPlanes		= 1;
  bi->biBitCount	= bits_per_pixel;
  bi->biCompression	= BI_RGB;
  bi->biSizeImage	= 0;
  bi->biXPelsPerMeter	= 0;
  bi->biXPelsPerMeter	= 0;
  bi->biClrUsed		= 0;
  bi->biClrImportant	= 0;

  if (n_colors > 0) {
    for (i = 0; i < 256; i++) {
      bmi->bmiColors[i].rgbRed   = colors[i].red;
      bmi->bmiColors[i].rgbGreen = colors[i].green;
      bmi->bmiColors[i].rgbBlue  = colors[i].blue;
      bmi->bmiColors[i].rgbReserved = 0;
    }
  }
  
  dc = GetDC(main_window);
  bmp = CreateDIBSection(dc, bmi, DIB_RGB_COLORS, (void **)&img->data, NULL, 0);
  ReleaseDC(main_window, dc);
  free(bmi);

  *img_return = img;
  return bmp;
}

static HANDLE open_pipe(const char *filename)
{
  STARTUPINFO si;
  SECURITY_ATTRIBUTES sa;  
  PROCESS_INFORMATION process_info;
  HANDLE process;
  HANDLE pipe_in, pipe_out, pipe_out_dup;
  
  sa.nLength = sizeof(sa);
  sa.lpSecurityDescriptor = NULL;
  sa.bInheritHandle = TRUE;
  CreatePipe(&pipe_in, &pipe_out, &sa, BUFSIZ);

  process = GetCurrentProcess();
  DuplicateHandle(process, pipe_out, process, &pipe_out_dup,
                  0, NULL, DUPLICATE_SAME_ACCESS);
  CloseHandle(pipe_out);
  
  memset(&si, 0, sizeof(si));
  si.cb = sizeof(si);
  si.dwFlags = STARTF_USESTDHANDLES;
  si.hStdInput = pipe_in;
  si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
  si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

  if (!CreateProcess(filename,
                     NULL,
                     NULL,
                     NULL,
                     TRUE,
                     DETACHED_PROCESS,
                     NULL,
                     NULL,
                     &si,
                     &process_info)) {
    fprintf(stderr, "exec failed: %s\n", filename);
  }

  CloseHandle(process_info.hProcess);
  CloseHandle(process_info.hThread);
  
  CloseHandle(pipe_in);
  return pipe_out_dup;
}

static void write_pipe(HANDLE pipe, const char *fmt, ...)
{
  va_list args;

  if (pipe) {
    DWORD num_writes;
    int n, status;
    char buf[BUFSIZ];
    
    va_start(args, fmt);
    n = vsprintf(buf, fmt, args);
    va_end(args);

    status = WriteFile(pipe, buf, n, &num_writes, NULL);
    if (!status) {
      fprintf(stderr, "failure %p: %s (%d)", pipe, buf, n);
      switch (GetLastError()) {
      case ERROR_BROKEN_PIPE:
        fprintf(stderr, "Broken pipe\n");
        break;

      case ERROR_INVALID_USER_BUFFER:
        fprintf(stderr, "Invalid User buffer\n");
        break;

      case ERROR_NOT_ENOUGH_MEMORY:
        fprintf(stderr, "Not enough memory\n");
        break;

      default:
        fprintf(stderr, "Error code: %d\n", GetLastError());
        break;
      }
      fflush(stderr);
    }
  }
}

static int start(void)
{
  int bits_per_pixel;
  int i;
  HDC dc;
  
  midi_server = open_pipe("midiplay.exe");
  write_pipe(midi_server, "d%s/midi\n", AUDIO_DIR);
  
  wave_server = open_pipe("waveplay.exe");
  write_pipe(wave_server, "d%s/wave\n", AUDIO_DIR);
  
  dc = GetDC(main_window);
  bits_per_pixel = GetDeviceCaps(dc, BITSPIXEL);

  switch (bits_per_pixel) {
  case 16:
    for (i = 0; i < 256; i++) {
      colors[i].red   = rgb_R(default_rgb[i]);
      colors[i].green = rgb_G(default_rgb[i]);
      colors[i].blue  = rgb_B(default_rgb[i]);
      colors[i].pixel = rgb2pixel16(default_rgb[i]);
    }
    break;
  case 24:
  case 32:
    for (i = 0; i < 256; i++) {
      colors[i].red   = rgb_R(default_rgb[i]);
      colors[i].green = rgb_G(default_rgb[i]);
      colors[i].blue  = rgb_B(default_rgb[i]);
      colors[i].pixel = default_rgb[i];
    }
    break;
  default:
    bits_per_pixel = 8;
    for (i = 0; i < 256; i++) {
      colors[i].red   = rgb_R(default_rgb[i]);
      colors[i].green = rgb_G(default_rgb[i]);
      colors[i].blue  = rgb_B(default_rgb[i]);
      colors[i].pixel = i;
    }
  }
  ReleaseDC(main_window, dc);

  if (init_graphics(bits_per_pixel)) {
    fprintf(stderr, "init_graphics() failed.\n");
    return 1;
  }
  
  offscreen = create_dib_section(640, 400, bits_per_pixel, &clip_overall);
  if (!offscreen) {
    fprintf(stderr, "Can't create offscreen bitmap.\n");
    return 1;
  }
  
  return init();
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
  wc.hIcon	   = LoadIcon(app_instance, MAKEINTRESOURCE(IDI_ICON1));
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

  if (start()) {
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
    CloseHandle(midi_server);
    CloseHandle(wave_server);
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
      interval = min(timer_interval, time_spent - interval);
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

void bgm_random(int random_pitch_bend)
{
  if (random_pitch_bend) {
    write_pipe(midi_server, "-\n");
  } else {
    write_pipe(midi_server, "+\n");
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
  write_pipe(wave_server, "@%c%s\n", 'A' + id, filename ? filename : "");
}

void beep(void)
{
  MessageBeep(-1);
}

int bgm_enabled(void)
{
  return midi_server != NULL;
}

int se_enabled(void)
{
  return wave_server != NULL;
}