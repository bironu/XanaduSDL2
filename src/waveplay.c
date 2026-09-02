#include <windows.h>
#include <mmsystem.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef __GNUC__
#define PACKED __attribute__((packed))
#else
#include <dir.h>
#define PACKED
#endif

#define MAX_WAVE_DATA		26	// バッファの最大数

#define SIGNATURE_BEGIN		('A')
#define SIGNATURE_END		(SIGNATURE_BEGIN + MAX_WAVE_DATA)

#define MAX_MIXER_PLAYING	8	// 同時再生数の上限
#define MAX_MIXER_BUFFER	1024	// ミキサー用バッファ

#define PCM_SILENCE		0x80	// 静寂

#pragma option -a1
typedef struct _FORMATHEADER {
  unsigned char		magic[4];	// "fmt "
  unsigned		chunk_size;	// ヘッダーを含まない
  WAVEFORMATEX		wf;
} PACKED FORMATHEADER;

typedef struct _DATAHEADER {
  unsigned char		magic[4];	// "data"
  unsigned		chunk_size;	// ヘッダーを含まない
} PACKED DATAHEADER;
#pragma option -a

// WAVE データを格納する構造体
typedef struct _WAVEDATA {
  unsigned char *	top;		// データの先頭
  unsigned char *	end;		// データの末尾
  char *		filename;	// ファイル名
  int			refcount;	// 演奏中の参照カウント
} WAVEDATA;

// 演奏中の WAVE データを格納する構造体
typedef struct _WAVEPLAYING {
  unsigned char *	top;		// 次の再生位置
  unsigned char *	end;		// データの終端
  int			data_id;	// データ番号
} WAVEPLAYING;

static HWAVEOUT		wave_device;	// WAVE デバイス
static char *		wave_file[MAX_WAVE_DATA];
static WAVEDATA 	wave_data[MAX_WAVE_DATA];
static WAVEPLAYING	wave_playing[MAX_MIXER_PLAYING];
static int		wave_num_playing;
static unsigned char	wave_mixer_buffer[MAX_MIXER_BUFFER];

// WAVEHDR
static WAVEHDR wave_wh = {
  wave_mixer_buffer,	// 再生用バッファ
  sizeof(wave_mixer_buffer),
  sizeof(wave_mixer_buffer),
  0,			// ユーザーデータ
  0,			// フラグ
  0,			// ループ回数
  NULL,			// 予約されている; ゼロでなければならない
  0			// 予約さ$l$F$$$k; ゼロでなければならない
};

// サポートする形式
static const WAVEFORMATEX wave_format = {
  WAVE_FORMAT_PCM,
  1,			// モノラル
  11025,		// 1 秒当たりのサンプル数
  11025,		// 1 秒当たりのバイト数
  1,			// 1 サンプル何バイト？
  8,			// 1 サンプル何ビット？
  sizeof(WAVEFORMATEX)	// たぶん無視される
};

static int wave_mixer(void);

static int wave_load(const char *filename, WAVEDATA *wave_data)
{
  FILE *fp;
  unsigned char magic[4], *buf;
  unsigned size;
  FORMATHEADER fmt_;
  DATAHEADER data;
  int error = 0;

  fp = fopen(filename, "rb");
  if (fp == NULL) {
    perror(filename);
    return 1;
  }

  // RIFF
  if (fread(magic, sizeof(magic), 1, fp) != 1 ||
      memcmp(magic, "RIFF", 4) != 0 ||
      fread(&size, sizeof(size), 1, fp) != 1) {
    error = 1;
    goto done;
  }

  // WAVE
  if (fread(magic, sizeof(magic), 1, fp) != 1 ||
      memcmp(magic, "WAVE", 4) != 0) {
    error = 1;
    goto done;
  }

  // fmt
  if (fread(&fmt_, sizeof(fmt_)-sizeof(WORD), 1, fp) != 1 ||
      memcmp(fmt_.magic, "fmt ", 4) != 0) {
    error = 1;
    goto done;
  }

  // 8ビット、モノラル、11000KHz？
  if (memcmp(&fmt_.wf, &wave_format, sizeof(WAVEFORMATEX)-sizeof(WORD)) != 0) {
    error = 1;
    goto done;
  }
  
  // data
  if (fread(&data, sizeof(data), 1, fp) != 1 ||
      memcmp(data.magic, "data", 4) != 0) {
    error = 1;
    goto done;
  }

  buf = (unsigned char *)malloc(data.chunk_size);
  if (fread(buf, data.chunk_size, 1, fp) != 1) {
    free(buf);
    error = 1;
    goto done;
  }

  free(wave_data->top);

  wave_data->top = buf;
  wave_data->end = buf + data.chunk_size;
  
done:
  fclose(fp);
  return error;
}

void CALLBACK wave_proc(HWAVE wave_device, UINT msg,
                        DWORD client_data,
                        DWORD param1,
                        DWORD param2)
{
  if (msg == MM_WOM_DONE) {
    int n = wave_mixer();
    if (n > 0) {
      wave_wh.dwBufferLength = n;
      wave_wh.dwBytesRecorded = n;
      waveOutWrite(wave_device, &wave_wh, sizeof(wave_wh));
    }
  }
}

#if 0
typedef struct { 
    LPSTR  lpData;                   // address of the waveform buffer 
    DWORD  dwBufferLength;           // length, in bytes, of the buffer 
    DWORD  dwBytesRecorded;          // see below 
    DWORD  dwUser;                   // 32 bits of user data 
    DWORD  dwFlags;                  // see below 
    DWORD  dwLoops;                  // see below 
    struct wavehdr_tag far * lpNext; // reserved; must be zero 
    DWORD  reserved;                 // reserved; must be zero 
} WAVEHDR;
#endif

static void wave_error_why(int error)
{
  printf("Why?\n");
}

// 現在演奏中のデータをミキシングして結果のバイト数を返す
static int wave_mixer(void)
{
  unsigned char *src_top, *src_end;
  unsigned char *dst_top, *dst_end;
  int i, max_n;
  
  // 一番長いデータはどれ？
  for (i = 0, max_n = 0; i < wave_num_playing; i++) {
    max_n = max(max_n,
                wave_playing[i].end - wave_playing[i].top);
  }
  if (max_n == 0)
    return 0;
  else
    max_n = min(max_n, MAX_MIXER_BUFFER);

  // 最初のデータ
  src_top = wave_playing[0].top;
  src_end = min(wave_playing[0].end, src_top + MAX_MIXER_BUFFER);
  dst_top = wave_mixer_buffer;
  dst_end = wave_mixer_buffer + MAX_MIXER_BUFFER;

  while (src_top < src_end) {
    *dst_top++ = *src_top++;
  }
  wave_playing[0].top = src_top;
  
  while (dst_top < dst_end) {
    *dst_top++ = PCM_SILENCE;
  }
 
  // 以降のデータ
  for (i = 1; i < wave_num_playing; i++) {
    src_top = wave_playing[i].top;
    src_end = min(wave_playing[i].end, src_top + MAX_MIXER_BUFFER);
    dst_top = wave_mixer_buffer;

    while (src_top < src_end) {
      int p;
      p = *dst_top + *src_top++ - PCM_SILENCE;
      p = min(max(0, p), 255);
      *dst_top++ = p;
    }
    wave_playing[i].top = src_top;
  }

  // 演奏を終了したデータをバッファから除去する
  for (i = wave_num_playing - 1; i >= 0; i--) {
    // 終端に到達した？
    if (wave_playing[i].top >= wave_playing[i].end) {
      // 参照カウントを減じる
      wave_data[wave_playing[i].data_id].refcount--;
      
      wave_num_playing--;
      wave_playing[i] = wave_playing[wave_num_playing];
    }
  }
  return max_n;
}

static void wave_play(int data_id)
{
  // まだ空きがある？
  if (wave_num_playing < MAX_MIXER_PLAYING) {
    int n = wave_num_playing;

    // データが存在する？
    if (wave_data[data_id].top == NULL)
      return;
    
    wave_data[data_id].refcount++;
    
    wave_playing[n].data_id = data_id;
    wave_playing[n].top = wave_data[data_id].top;
    wave_playing[n].end = wave_data[data_id].end;

    // 再生を開始する？
    if (wave_num_playing++ == 0) {
      n = wave_mixer();

      wave_wh.dwBufferLength  = n;
      wave_wh.dwBytesRecorded = n;
      
      waveOutWrite(wave_device, &wave_wh, sizeof(wave_wh));
    }
  }
}

// 指定されたデータの再生を止める
void wave_stop_playing(int data_id)
{
  int i;
  
  for (i = wave_num_playing - 1; i >= 0; i--) {
    if (wave_playing[i].data_id == data_id) {
      // 参照カウントを減じる
      wave_data[wave_playing[i].data_id].refcount--;
      
      wave_num_playing--;
      wave_playing[i] = wave_playing[wave_num_playing];
    }
  }
}

int main(void)
{
  char buf[BUFSIZ];
  int error;

  error = waveOutOpen(&wave_device, WAVE_MAPPER, &wave_format,
                      (DWORD)wave_proc, 0, CALLBACK_FUNCTION);
  if (error) {
    wave_error_why(error);
    return 0;
  }

  error = waveOutPrepareHeader(wave_device, &wave_wh, sizeof(wave_wh));
  if (error) {
    wave_error_why(error);
    return 0;
  }
  
  while (fgets(buf, sizeof(buf), stdin)) {

    switch (buf[0]) {
    case '!':
      {      
        int c = toupper(buf[1]);

        if (SIGNATURE_BEGIN <= c && c < SIGNATURE_END) {
          wave_play(c - SIGNATURE_BEGIN);
        }
      }
      break;

    case 'd':
    case 'D':
      {
        char *p, *path = &buf[1];
        
        // 改行コードを除去する
        for (p = path; *p != '\0' && *p != '\n'; p++);
        *p = '\0';

        chdir(path);
      }
      break;

    case '@':
      {
        int c = toupper(buf[1]);

        if (SIGNATURE_BEGIN <= c && c < SIGNATURE_END) {
          char *p, *filename = &buf[2];
          int data_id = c - SIGNATURE_BEGIN;
          WAVEDATA *wd = &wave_data[data_id];
        
          // 改行コードを除去する
          for (p = filename; *p != '\0' && *p != '\n'; p++);
          *p = '\0';
          
          if (filename[0] == '\0')
            goto junk;
          
          // 同じファイル？
          if (wd->filename != NULL && strcmp(wd->filename, filename) == 0)
            continue;
          
          // 再生中？
          if (wd->refcount > 0) {
            // 現在再生中のデータを除去する
            wave_stop_playing(data_id);
          }
          
          if (wave_load(filename, wd) == 0) {
            free(wd->filename);
            wd->filename = strdup(filename);
          } else {
            /* 読み込みに失敗しても以前のデータはそのまま残っているが、
             * 使えないようにする */
          junk:
            free(wd->top);
            wd->top = NULL;
            wd->end = NULL;
            free(wd->filename);
            wd->filename = NULL;
          }
        }
      }
      break;
    }
  }
  waveOutReset(wave_device);
  waveOutUnprepareHeader(wave_device, &wave_wh, sizeof(wave_wh));
  waveOutClose(wave_device);
  return 0;
}
