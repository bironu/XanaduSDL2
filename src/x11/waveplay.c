#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <machine/soundcard.h>

#ifdef __GNUC__
#define min(a, b)	((a) < (b) ? (a) : (b))
#define max(a, b)	((a) < (b) ? (b) : (a))
#define PACKED __attribute__((packed))
#else
#define PACKED
#endif

#define MAX_WAVE_DATA		26	/* バッファの最大数 */

#define SIGNATURE_BEGIN		('A')
#define SIGNATURE_END		(SIGNATURE_BEGIN + MAX_WAVE_DATA)

#define MAX_MIXER_PLAYING	8	/* 同時再生数の上限 */
#define MAX_MIXER_BUFFER	1024	/* ミキサー用バッファ */

#define PCM_SILENCE		0x80	/* 静寂 */

#define WAVE_DEVICE		"/dev/dsp"

#ifdef __FreeBSD__
#define WAVE_FORMAT_PCM		1

typedef struct {
  uint16_t		wFormatTag;
  uint16_t		nChannels;
  uint32_t		nSamplesPerSec;
  uint32_t		nAvgBytesPerSec;
  uint16_t		nBlockAlign;
  uint16_t		wBitsPerSample;
  uint16_t		cbSize;
} PACKED WAVEFORMATEX;
#endif

typedef struct {
  unsigned char		magic[4];	/* "fmt " */
  unsigned		chunk_size;	/* ヘッダーを含まない */
  WAVEFORMATEX		wf;
} PACKED format_header_t;

typedef struct {
  unsigned char		magic[4];	/* "data" */
  unsigned		chunk_size;	/* ヘッダーを含まない */
} PACKED data_header_t;

/* WAVE データを格納する構造体 */
typedef struct {
  unsigned char *	top;		/* データの先頭 */
  unsigned char *	end;		/* データの末尾 */
  const char *		name;		/* ファイル名 */
  int			ref_count;	/* 演奏中の参照カウント */
} PACKED wave_data_t;

/* 演奏中の WAVE データを格納する構造体 */
typedef struct {
  unsigned char *	top;		/* 次の再生位置 */
  unsigned char *	end;		/* データの終端 */
  int			data_id;	/* データ番号 */
} PACKED wave_playing_t;

static int		wave_device;
static const char *	wave_file[MAX_WAVE_DATA];
static wave_data_t 	wave_data[MAX_WAVE_DATA];
static wave_playing_t	wave_playing[MAX_MIXER_PLAYING];
static int		wave_num_playing;
static unsigned char	wave_mixer_buffer[MAX_MIXER_BUFFER];

/* サポートする形式 */
static const WAVEFORMATEX wave_format = {
  WAVE_FORMAT_PCM,
  1,			/* モノラル */
  11025,		/* 1 秒当たりのサンプル数 */
  11025,		/* 1 秒当たりのバイト数 */
  1,			/* 1 サンプル何バイト？ */
  8,			/* 1 サンプル何ビット？ */
  sizeof(WAVEFORMATEX)	/* たぶん無視される */
};

static int wave_mixer(void);

static int wave_load(const char *filename, wave_data_t *wave_data)
{
  FILE *fp;
  unsigned char magic[4], *buf;
  unsigned size;
  format_header_t fmt_;
  data_header_t data;
  int error = 0;

  fp = fopen(filename, "rb");
  if (fp == NULL) {
    perror(filename);
    return 1;
  }

  /* RIFF */
  if (fread(magic, sizeof(magic), 1, fp) != 1 ||
      memcmp(magic, "RIFF", 4) != 0 ||
      fread(&size, sizeof(size), 1, fp) != 1) {
    error = 1;
    goto done;
  }

  /* WAVE */
  if (fread(magic, sizeof(magic), 1, fp) != 1 ||
      memcmp(magic, "WAVE", 4) != 0) {
    error = 1;
    goto done;
  }

  /* fmt  */
  if (fread(&fmt_, sizeof(fmt_) - sizeof(uint16_t), 1, fp) != 1 ||
      memcmp(fmt_.magic, "fmt ", 4) != 0) {
    error = 1;
    goto done;
  }

  /* 8ビット、モノラル、11000KHz？ */
  if (memcmp(&fmt_.wf, &wave_format, sizeof(WAVEFORMATEX) - sizeof(uint16_t)) != 0) {
    error = 1;
    goto done;
  }
  
  /* data */
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

static void wave_error_why(int error)
{
  printf("Why?\n");
}

/* 現在演奏中のデータをミキシングして結果のバイト数を返す */
static int wave_mixer(void)
{
  unsigned char *src_top, *src_end;
  unsigned char *dst_top, *dst_end;
  int i, max_n;
  
  /* 一番長いデータはどれ？ */
  for (i = 0, max_n = 0; i < wave_num_playing; i++) {
    max_n = max(max_n, wave_playing[i].end - wave_playing[i].top);
  }
  if (max_n == 0)
    return 0;
  else
    max_n = min(max_n, MAX_MIXER_BUFFER);

  /* 最初のデータ */
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
 
  /* 以降のデータ */
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

  /* 演奏を終了したデータをバッファから除去する */
  for (i = wave_num_playing - 1; i >= 0; i--) {
    /* 終端に到達した？ */
    if (wave_playing[i].top >= wave_playing[i].end) {
      /* 参照カウントを減じる */
      wave_data[wave_playing[i].data_id].ref_count--;
      
      wave_num_playing--;
      wave_playing[i] = wave_playing[wave_num_playing];
    }
  }
  return max_n;
}

static void wave_play(int data_id)
{
  /* データが存在し、まだ空きがある？ */
  if (wave_data[data_id].top && wave_num_playing < MAX_MIXER_PLAYING - 1) {
    int n;
  
    n = wave_num_playing;
    
    wave_data[data_id].ref_count++;
    
    wave_playing[n].data_id = data_id;
    wave_playing[n].top = wave_data[data_id].top;
    wave_playing[n].end = wave_data[data_id].end;
    
    wave_num_playing++;
  }
#if 0
  printf("n-playing: %d (data_id=%c, data=%p)\n", wave_num_playing,
	 data_id + 'A', wave_data[data_id].top);
#endif
}

/* 指定されたデータの再生を止める */
static void wave_stop_playing(int data_id)
{
  int i;
  
  for (i = wave_num_playing - 1; i >= 0; i--) {
    if (wave_playing[i].data_id == data_id) {
      /* 参照カウントを減じる */
      wave_data[wave_playing[i].data_id].ref_count--;
      
      wave_num_playing--;
      wave_playing[i] = wave_playing[wave_num_playing];
    }
  }
}

static void wave_register(int data_id, const char *filename)
{
  wave_data_t *wd = &wave_data[data_id];
	
  /* 同じファイル？ */
  if (wd->name != NULL && strcmp(wd->name, filename) == 0) {
    return;
  }
  
  /* 再生中ならば現在再生中のデータを除去する */
  if (wd->ref_count > 0) {
    wave_stop_playing(data_id);
  }

  if (wave_load(filename, wd) == 0) {
    free((void *)wd->name);
    wd->name = strdup(filename);
#if 0
    printf("%s: loaded (id=%c, data=%p)\n",
	   filename, data_id + 'A', wave_data[data_id]);
#endif
  } else {
    /* 読み込みに失敗しても以前のデータはそのまま残っているが、
     * 使えないようにする */
    fprintf(stderr, "Can't load %s\n", filename);
    free(wd->top);
    wd->top = NULL;
    wd->end = NULL;
    free((void *)wd->name);
    wd->name = NULL;
  }
}

static char *chop(char *s)
{
  char *orig = s;
  for (; *s && *s != '\n'; s++);
  *s = '\0';
  return orig;
}

int main(void)
{
  char buf[BUFSIZ];
  long interval, wave_interval;
  int error;
  int n_channels, sample_rate, bits_per_sample;
  
  wave_device = open(WAVE_DEVICE, O_WRONLY);
  if (wave_device == -1) {
    perror("open()");
    exit(EXIT_FAILURE);
  }

  n_channels = wave_format.nChannels;
  error = ioctl(wave_device, SOUND_PCM_WRITE_CHANNELS, &n_channels);
  if (error == -1 || n_channels != wave_format.nChannels) {
    fprintf(stderr, "%d channels not supported.\n", wave_format.nChannels);
    exit(EXIT_FAILURE);
  }

  sample_rate = wave_format.nSamplesPerSec;
  error = ioctl(wave_device, SOUND_PCM_WRITE_RATE, &sample_rate);
  if (error == -1) {
    perror("ioctl()");
    exit(EXIT_FAILURE);
  }
  
  bits_per_sample = wave_format.wBitsPerSample;
  error = ioctl(wave_device, SOUND_PCM_WRITE_BITS, &bits_per_sample);
  if (error == -1 || bits_per_sample != wave_format.wBitsPerSample) {
    fprintf(stderr, "%d bits not supported.\n", wave_format.wBitsPerSample);
    exit(EXIT_FAILURE);
  }

  wave_interval = (MAX_MIXER_BUFFER * 1000000.0 / wave_format.nAvgBytesPerSec) * 0.8;
  interval = wave_interval;

  for (;;) {
    struct timeval ago, now, time_out;
    fd_set fds;
    int c, n, n_bytes;

    gettimeofday(&ago, NULL);
    
    FD_ZERO(&fds);

    FD_SET(0, &fds);

    time_out.tv_sec = 0;
    time_out.tv_usec = interval;

    n = select(1, &fds, NULL, NULL, &time_out);
    if (n < 0) {
      if (errno != EINTR) {
	perror("select()");
	exit(EXIT_FAILURE);
      }
    } else if (n > 0) {
      if (!fgets(buf, sizeof(buf), stdin)) {
	break;
      }

      c = toupper(buf[0]);
      switch (c) {
      case '@':
	c = toupper(buf[1]);
	if (SIGNATURE_BEGIN <= c && c < SIGNATURE_END) {
	  char *filename = chop(&buf[2]);
	  wave_register(c - SIGNATURE_BEGIN, filename);
	}
	break;
	
      case '!':
	c = toupper(buf[1]);
	if (SIGNATURE_BEGIN <= c && c < SIGNATURE_END) {
	  wave_play(c - SIGNATURE_BEGIN);
	  goto play_wave;
	}
	break;
	
      case 'D':
	if (chdir(chop(&buf[1])) == -1) {
	  perror("chdir()");
	}
	break;
      }

      gettimeofday(&now, NULL);
      if (now.tv_usec < ago.tv_usec) {
	interval = now.tv_usec - ago.tv_usec + 1000000;
      } else {
	interval = now.tv_usec - ago.tv_usec;
      }
      interval = min(interval, wave_interval);
    } else {

    play_wave:
      n_bytes = wave_mixer();
      if (n_bytes > 0) {
	ioctl(wave_device, SOUND_PCM_SYNC);
	write(wave_device, wave_mixer_buffer, n_bytes);
      }
      interval = wave_interval;
    }
  }
  close(wave_device);
  return 0;
}
