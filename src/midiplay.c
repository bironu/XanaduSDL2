#include <stdio.h>
#include <stdlib.h>

#ifdef __GNUC__
#define PACKED __attribute__((packed))
#else
#include <dir.h>
#define PACKED
#endif

#ifdef __WIN32__
#include <windows.h>
#include <mmsystem.h>
#else
typedef struct {
  unsigned	dwDeltaTime;
  unsigned	dwStreamID;
  unsigned	dwEvent;
  unsigned	dwParms[1];
} MIDIEVENT;
#endif

#define midi_error_why(err)

#define MAX_TRACK		32	/* トラック数の上限 */
#define FILE_BUFFER_SIZE	0x10000	/* ファイルバッファサイズ */
#define EVENT_BUFFER_SIZE	0x10000	/* イベントバッファサイズ */
#define STORAGE_SIZE		0x40000 /* メモリ領域のサイズ */
#define MAX_MIDI_SEGMENT	32

/* バイトオーダーの変換 */
#define midi_byte_order16(n)	(((((n) >>  8) & 0xff) <<  0) | \
                                 ((((n) >>  0) & 0xff) <<  8))
#define midi_byte_order32(n)	(((((n) >> 24) & 0xff) <<  0) | \
                                 ((((n) >> 16) & 0xff) <<  8) | \
                                 ((((n) >>  8) & 0xff) << 16) | \
                                 ((((n) >>  0) & 0xff) << 24))

/* ファイルヘッダー情報 */
#pragma option -a1
typedef struct {
  unsigned char		magic[4];	/* 4D 54 68 64 */
  unsigned		header_size;	/* ヘッダーサイズ */
  unsigned short	format;		/* フォーマット */
  unsigned short	num_tracks;	/* トラック数 */
  unsigned short	timebase;	/* タイムベース */
} PACKED midi_file_header_t;

/* トラックヘッダー情報 */
typedef struct {
  unsigned char		magic[4];	/* 4D 54 72 6B */
  unsigned		track_size;	/* トラックサイズ */
} PACKED midi_track_header_t;
#pragma option -a

/* トラック情報 */
typedef struct {
  unsigned		delta_time;	/* デルタ時間 */
  unsigned char *	current;	/* トラックの現在位置 */
  unsigned char *	top;		/* トラックの先頭 */
  unsigned char *	end;		/* トラックの終端 */
  unsigned char		last_stat;	/* 直前のステータスバイト */
} midi_track_t;

/* 小節情報 */
typedef struct midi_measure {
  struct midi_measure *	next;
  int			number;
  short			pitch_bends[16];
  int			size;		/* イベントバッファのサイズ */
  char			events[1];	/* イベントバッファ */
} midi_measure_t;

/* MIDI情報 */
static unsigned		midi_timebase;	/* タイムベース */
static unsigned		midi_tempo;	/* テンポ */
static HMIDISTRM	midi_device;	/* MIDIストリームデバイス */
static int		midi_speed;	/* 速さ: 1)速い 0)普通 -1)遅い */
static int		midi_num_measures;
static midi_measure_t *	midi_measures;	/* 小節リスト */
static midi_measure_t *	midi_current_measure;
static midi_measure_t *	midi_loop_at;	/* ループ再開位置 */
static MIDIHDR		midi_mh[MAX_MIDI_SEGMENT];
static int		midi_mh_used;
static int		midi_random_pitch_bend;
static short		midi_current_pitch_bends[16];

/* オプション */
static int option_debug;

/* イベントバッファ */
static char buffer[EVENT_BUFFER_SIZE];
static char *buffer_top;
#define buffer_left()	(&buffer[EVENT_BUFFER_SIZE] - buffer_top)
#define buffer_init()	(buffer_top = buffer)

/* メモリ領域 */
static char storage[STORAGE_SIZE];
static char *storage_top;
#define storage_left()	(&storage[STORAGE_SIZE] - storage_top)
#define storage_init()	(storage_top = storage)

static midi_measure_t *midi_serialize(midi_track_t *track, int num_tracks);
static void midi_push_event(midi_track_t *, unsigned delta_time);

int midi_load(const char *filename)
{
  int n, error = 0;
  FILE *fp;
  midi_file_header_t file_info;
  midi_track_header_t track_info;
  midi_track_t track[MAX_TRACK];
  unsigned char track_data[FILE_BUFFER_SIZE], *buf = track_data;

  fp = fopen(filename, "rb");
  if (!fp) {
    perror(filename);
    return 1;
  }
  
  /* ファイルヘッダーの読み込み */
  if (fread(&file_info, sizeof(file_info), 1, fp) != 1 ||
      memcmp(file_info.magic, "MThd", 4) != 0) {
    error = 1;
    goto done;
  }

  /* バイトオーダーの変換 */
  file_info.header_size = midi_byte_order32(file_info.header_size);
  file_info.format      = midi_byte_order16(file_info.format);
  file_info.num_tracks  = midi_byte_order16(file_info.num_tracks);
  file_info.timebase    = midi_byte_order16(file_info.timebase);

  if (file_info.header_size != 6 ||
      file_info.format >= 2 ||
      file_info.num_tracks >= MAX_TRACK) {
    error = 1;
    goto done;
  }

  for (n = 0; n < file_info.num_tracks; n++) {
    int size;
    
    /* トラックヘッダーの読み込み */
    if (fread(&track_info, sizeof(track_info), 1, fp) != 1 ||
        memcmp(track_info.magic, "MTrk", 4) != 0) {
      error = 1;
      goto done;
    }

    track_info.track_size = midi_byte_order32(track_info.track_size);

    /* バッファオーバーラン？ */
    if (track_data + FILE_BUFFER_SIZE - buf < (int)track_info.track_size)
      break;

    /* MIDI イベントの読み込み */
    if (fread(buf, track_info.track_size, 1, fp) != 1) {
      error = 1;
      goto done;
    }

    /* トラックデータへの書き込み */
    track[n].delta_time = 0;
    track[n].current    = buf;
    track[n].top        = buf;
    track[n].end        = buf + track_info.track_size;
    track[n].last_stat  = 0;
    
    buf += track_info.track_size;
  }

  midi_timebase = file_info.timebase;
  midi_tempo = 10000; /* デフォルトテンポ */
  midi_random_pitch_bend = -1 /*0*/; /* 回復する */
  midi_num_measures = 0;
  midi_loop_at = NULL; /* ループ再開位置 */
  
  midi_measures = midi_serialize(track, n);

  if (!midi_loop_at) {
    midi_loop_at =  midi_measures;
  }

  if (option_debug) {
    printf("number of measures: %d\n", midi_num_measures);
    if (midi_loop_at) {
      printf("loop_at: measure #%d\n", midi_loop_at->number);
    }
  }

done:
  fclose(fp);
  return error;
}

/* 数値の読み込み */
static unsigned midi_read_number(unsigned char **p_array)
{
  unsigned char q;
  unsigned n = 0;

  do {
    q = **p_array;
    (*p_array)++;
    n = (n << 7) + (q & 0x7f);
  } while (q & 0x80);

  return n;
}

static MIDIEVENT *midi_alloc_event(int extra_size)
{
  int size = 12 + (extra_size / 4) * 4;
  if (extra_size % 4 != 0) {
    size += 4;
  }
  if (size > buffer_left()) {
    return NULL;
  } else {
    MIDIEVENT *me = (MIDIEVENT *)buffer_top;
    buffer_top += size;
    return me;
  }
}

static void midi_push_nop(unsigned delta_time)
{
  if (delta_time) {
    MIDIEVENT *me = midi_alloc_event(0);
    if (me) {
      me->dwDeltaTime = delta_time;
      me->dwStreamID = 0;
      me->dwEvent = MEVT_F_SHORT | MEVT_NOP;
    }
  }
}

static short buffer_pitch_bends[16];
static int buffer_loop_at;
static int buffer_measure_time;

/* バッファ管理最悪 */
static midi_measure_t *flush_buffer(void)
{
  int size = buffer_top - buffer;

  /* バッファクリア */
  buffer_top = buffer;
  
  if (size == 0 || size > storage_left()) {
    return NULL;
  } else {
    midi_measure_t *mm = (midi_measure_t *)storage_top;
    storage_top += (sizeof(midi_measure_t) + size - 1);
    mm->next = NULL;
    mm->number = ++midi_num_measures;

    memcpy(mm->pitch_bends, buffer_pitch_bends, sizeof(buffer_pitch_bends));
    memcpy(buffer_pitch_bends, midi_current_pitch_bends,
           sizeof(buffer_pitch_bends));
    
    mm->size = size;
    memcpy(mm->events, buffer, size);
    return mm;
  }
}

/* 複数のトラックデータを小節ごとのストリームに凝縮する */
midi_measure_t *midi_serialize(midi_track_t *track, int num_tracks)
{
  int i;
  unsigned track_time[MAX_TRACK];
  unsigned now = 0;
  unsigned measure_time = 0;
  midi_measure_t *mm, **mp = &mm;

  /* メモリ領域とバッファの初期化 */
  storage_init();
  buffer_init();

  buffer_loop_at = 0;
  buffer_measure_time = 4 * midi_timebase;

  memset(buffer_pitch_bends, 0, sizeof(buffer_pitch_bends));
  memset(midi_current_pitch_bends, 0, sizeof(midi_current_pitch_bends));

  /* 最初のイベントのデルタ時間を読み込む */
  for (i = 0; i < num_tracks; i++) {
    track_time[i] = track[i].delta_time = midi_read_number(&track[i].current);
  }

  for (;;) {
    unsigned min_time = ~0;
    unsigned delta_time;
    
    /* もっとも早い次のトラックを調べる */
    for (i = 0; i < num_tracks; i++) {
      if (track_time[i] < min_time) {
        min_time = track_time[i];
      }
    }
    if (min_time == ~0)
      break;

    delta_time = min_time - now;
    
    /* 直前の小節は終了した？ */
    if (min_time >= measure_time + buffer_measure_time) {
      measure_time += buffer_measure_time;

      /* 次の小節までの遅延を追加する */
      if (delta_time >= measure_time - now) {
        delta_time -= measure_time - now;
        midi_push_nop(measure_time - now);
      }

      *mp = flush_buffer();
      if (*mp) {
        if (buffer_loop_at) {
          buffer_loop_at = 0;
          midi_loop_at = *mp;
        }
        mp = &(*mp)->next;
      }
    }
    now = min_time;

    for (i = 0; i < num_tracks; i++) {
      if (track_time[i] == min_time) {
        /* メッセージをプッシュ */
        midi_push_event(&track[i], delta_time);
        delta_time = 0;

        /* そのトラックは終了した？ */
        if (!track[i].current) {
          track_time[i] = ~0; /* 番兵 */
        } else {
          track[i].delta_time = midi_read_number(&track[i].current);
          track_time[i] += track[i].delta_time;
        }
      }
    }
  }

  /* 最後のバッファをフラッシュ */
  *mp = flush_buffer();
  if (mm) {
    memcpy(mm->pitch_bends, midi_current_pitch_bends, sizeof(mm->pitch_bends));
  }

  /* リストの先頭を返す */
  return mm;
}

/* MIDIメッセージ読み込みをイベントに変換してバッファに送る */
void midi_push_event(midi_track_t *t, unsigned delta_time)
{
  unsigned char stat;
  
  /* ランニングモード？ */
  stat = *t->current < 0x80 ? t->last_stat : *t->current++;
  t->last_stat = stat;
  
  switch (stat & 0xf0) {
  case 0x80:
  case 0x90:
  case 0xa0:
  case 0xb0:
  case 0xe0:
    {
      unsigned char data1 = *t->current++;
      unsigned char data2 = *t->current++;
      unsigned msg = stat | (data1 << 8) | (data2 << 16);
      MIDIEVENT *me = midi_alloc_event(0);

      if (me != NULL) {
        me->dwDeltaTime = delta_time;
        me->dwStreamID  = 0;
        me->dwEvent = MEVT_F_SHORT | msg;
      }
      if ((stat & 0xf0) == 0xe0) {
        int channel = stat & 0x0f;
        midi_current_pitch_bends[channel] = (data1 | (data2 << 7)) - 0x2000;
      }
    }
    break;

  case 0xc0:
  case 0xd0:
    {
      unsigned char data1 = *t->current++;
      unsigned msg = stat | (data1 << 8);
      MIDIEVENT *me = midi_alloc_event(0);

      if (me != NULL) {
        me->dwDeltaTime = delta_time;
        me->dwStreamID  = 0;
        me->dwEvent = MEVT_F_SHORT | msg;
      }
    }
    break;
    
  case 0xf0:
    {
      unsigned size, n;
      MIDIEVENT *me;
      unsigned char *buf;

      /* システムエクスクルーシブ？ */
      if (stat == 0xf0) {
        size = midi_read_number(&t->current) + 1;
        
        me = midi_alloc_event(size);

        if (me != NULL) {
          buf = (unsigned char*)me->dwParms;
          *buf++ = 0xf0;
          for (n = 1; n < size; n++) {
            *buf++ = *t->current++;
          }
          /* 4 バイト境界にパディング */
          switch (size % 4) {
          case 1: *buf++ = 0;
          case 2: *buf++ = 0;
          case 3: *buf   = 0;
          }
          me->dwDeltaTime = delta_time;
          me->dwStreamID  = 0;
          me->dwEvent = MEVT_F_LONG | (size & 0xffffff);
        }
      }
      /* メタイベント？ */
      else if (stat == 0xff) {
        unsigned char code;
        
        code = *t->current++;
        size = midi_read_number(&t->current);

        switch (code) {
        case 0x01:
        case 0x07:
          if (option_debug) {
            printf("measure #%d has text: ", midi_num_measures + 1);
            fwrite(t->current, 1, size, stdout);
            printf("\n");            
          }
          if (strnicmp(t->current, "XA_MIDI_CTL.LOOP_AT", size) == 0 ||
              strnicmp(t->current, "loop", size) == 0) {
            buffer_loop_at = 1;
          }
          goto skip;
          
        case 0x2f:
          t->current = NULL; /* 終了 */
          return;

        case 0x51:
          {
            unsigned tempo = 0;
            
            for (n = 0; n < size; n++) {
              tempo += (tempo << 8) + *t->current++;
            }
            /* 同じテンポの連続する指定は無視する */
            if (midi_tempo != tempo) {
              midi_tempo = tempo;
              
              me = midi_alloc_event(0);
              if (me != NULL) {
                me->dwDeltaTime = delta_time;
                me->dwStreamID  = 0;
                me->dwEvent = (MEVT_NOP << 24) | MEVT_F_CALLBACK |
                  (tempo & 0x00FFFFFF);
              }
            }
          }
          break;

        case 0x58:
          if (size == 4) {
            int nn = *t->current++; /* 拍子記号の分子 */
            int dd = *t->current++; /* 拍子記号の分母 = 2^dd */
            int cc = *t->current++; /* MIDIクロック/メトロノームカウント */
            int bb = *t->current++; /* 32分音符/4分音符中 */

            buffer_measure_time = 4 * midi_timebase * nn / (1 << dd);

            if (option_debug) {
              printf("measure #%d: beat = %d/%d, %d, %d, time=%u\n",
                     midi_num_measures + 1,
                     nn, 1 << dd, cc, bb,
                     buffer_measure_time);
            }
          } else {
            goto skip;
          }
          break;

        case 0x7f:
          if (option_debug) {
            printf("measure #%d: Unknown sequencer event, set the loop position here.\n", midi_num_measures + 1);
          }
          buffer_loop_at = 1;
          /* through */
          
        default:
        skip:
          t->current += size; /* スキップ */
          break;
        }
      }
    }
    break;
  }
}

#if 1
static void CALLBACK midi_proc(HMIDIOUT, UINT, DWORD, DWORD, DWORD);

static void midi_set_tempo(unsigned tempo)
{
  if (midi_device) {
    MIDIPROPTEMPO mt;
    
    mt.cbStruct = sizeof(mt);
    mt.dwTempo = tempo;
    midiStreamProperty(midi_device, (void *)&mt, MIDIPROP_TEMPO | MIDIPROP_SET);
  }
}

static void midi_set_timebase(unsigned timebase)
{
  if (midi_device) {
    MIDIPROPTIMEDIV mt;
  
    mt.cbStruct  = sizeof(mt);
    mt.dwTimeDiv = timebase;
    midiStreamProperty(midi_device, (void *)&mt, MIDIPROP_TIMEDIV | MIDIPROP_SET);
  }
}

static int midi_prepare_current_measure(MIDIHDR *mh)
{
  if (midi_device && midi_current_measure) {
    midiOutUnprepareHeader((HMIDIOUT)midi_device, mh, sizeof(*mh));
    mh->lpData = midi_current_measure->events;
    mh->dwFlags = 0;
    mh->dwBufferLength = mh->dwBytesRecorded = midi_current_measure->size;
    return midiOutPrepareHeader((HMIDIOUT)midi_device, mh, sizeof(*mh));
  } else {
    printf("No measures.\n");
    return MMSYSERR_INVALPARAM;
  }
}

static int midi_play(const char *filename)
{
  char buf[256];
  int i, error;
  unsigned device_id;

  if (midi_load(filename))
    return 1;
  
  device_id = 0;
  error = midiStreamOpen(&midi_device, &device_id,
                         1,
                         (DWORD)midi_proc,
                         0,
                         CALLBACK_FUNCTION);
  if (error) {
    midi_error_why(error);
    return 1;
  }
  
  midi_set_tempo(midi_tempo);
  midi_set_timebase(midi_timebase);
  
  midi_speed = 0; /* 通常の速さ */
  midi_current_measure = midi_measures;

  for (i = 0; i < 16; i++) {
    midi_current_pitch_bends[i] = 0;
  }

  /* MIDIHDR配列全体をゼロクリア */
  memset(midi_mh, 0, sizeof(midi_mh));
  midi_mh_used = 0;

  error = midi_prepare_current_measure(&midi_mh[midi_mh_used]);
  if (error) {
    midi_error_why(error);
    return 1;
  }
  
  error = midiStreamOut(midi_device, &midi_mh[0], sizeof(midi_mh[0]));
  if (error) {
    midi_error_why(error);
    return 1;
  }
  
  error = midiStreamRestart(midi_device);
  if (error) {
    midi_error_why(error);
    return 1;
  }

  return 0;
}

static int midi_close(void)
{
  if (midi_device) {
    HMIDISTRM device = midi_device;
    int i;
    
    /* コールバック関数が処理されないようにデバイスハンドルを無効化する */
    midi_device = 0;
    
    midiStreamStop(device);
    for (i = 0; i < sizeof(midi_mh)/sizeof(midi_mh[0]); i++) {
      midiOutUnprepareHeader((HMIDIOUT)device, &midi_mh[i], sizeof(midi_mh[i]));
    }
    midiStreamClose(device);
  }
  return 0;
}

int main(int argc, char *argv[])
{
  char buf[BUFSIZ];

  if (argc == 2 && stricmp(argv[1], "-debug") == 0) {
    option_debug = 1;
  }

  while (fgets(buf, sizeof(buf), stdin)) {
    char *p;

    /* 改行文字を除去 */
    for (p = buf; *p != '\0' && *p != '\n'; p++);
    *p = '\0';

    switch (toupper(buf[0])) {
    case '!':
      midi_close();
      break;
      
    case '@':
      midi_close();
      midi_play(&buf[1]);
      break;

    case 'D':
      if (chdir(&buf[1])) {
        perror(&buf[1]);
      }
      break;
      
    case 'P':
      if (midi_device)
        midiStreamPause(midi_device);
      break;
      
    case 'R':
      if (midi_device)
        midiStreamRestart(midi_device);
      break;
      
    case 'Q':
      midi_speed = 1;
      midi_set_tempo((unsigned)(midi_tempo / 1.5));
      break;
      
    case 'S':
      midi_speed = -1;
      midi_set_tempo((unsigned)(midi_tempo * 1.5));
      break;

    case 'T':
      midi_speed = 0;
      midi_set_tempo(midi_tempo);
      break;

    case '-':
      midi_random_pitch_bend = 1;
      break;

    case '+':
      if (midi_random_pitch_bend > 0) {
        midi_random_pitch_bend = -1;
      }
      break;
    }
  }

  midi_close();
  return 0;
}

#define random_integer(n) (rand() % (n))

/* ランダムなピッチベンドイベントからなる小節を生成する */
midi_measure_t *random_pitch_bend(midi_measure_t *next)
{
  static char buf[sizeof(midi_measure_t) + 4 * sizeof(MIDIEVENT)];
  midi_measure_t *mm;
  MIDIEVENT *me;
  int value, channel;
  unsigned char stat, data1, data2;
  
  mm = (midi_measure_t *)buf;
  mm->next = next;
  mm->number = -1;
  mm->size = 12;

  channel = 0;
  stat = 0xe0 | (channel & 0xf);

  value = random_integer(0x4000);
  data1 = value & 0x7f;
  data2 = (value >> 7) & 0x7f;
  midi_current_pitch_bends[0] = value - 0x2000;

  me = (MIDIEVENT *)mm->events;
  me->dwDeltaTime = 0;
  me->dwStreamID = 0;
  me->dwEvent = MEVT_F_SHORT | stat | (data1 << 8) | (data2 << 16);

  return mm;
}

static midi_measure_t *restore_pitch_bend(midi_measure_t *next)
{
  static char buf[sizeof(midi_measure_t) + 16 * sizeof(MIDIEVENT)];
  midi_measure_t *mm;
  int i;
  unsigned char *p;

  if (!next) {
    return NULL;
  }
  mm = (midi_measure_t *)buf;
  mm->next = next;
  mm->number = -1;
  mm->size = 12 * 16;
  for (i = 0, p = mm->events; i < 16; i++, p += 12) {
    unsigned stat = 0xe0 | i;
    int value = next->pitch_bends[i] + 0x2000;
    unsigned data1 = value & 0x7f;
    unsigned data2 = (value >> 7) & 0x7f;
    MIDIEVENT *me = (MIDIEVENT *)p;
    me->dwDeltaTime = 0;
    me->dwStreamID = 0;
    me->dwEvent = MEVT_F_SHORT | stat | (data1 << 8) | (data2 << 16);

    midi_current_pitch_bends[i] = value;
  }
  return mm;
}

void CALLBACK
midi_proc(HMIDIOUT __device, UINT msg, DWORD client_data,
          DWORD param1, DWORD param2)
{
  if (midi_device == 0) {
    return;
  }
  switch (msg) {
  case MOM_DONE:
    {
      int error, i;

      /* 次の小節を演奏する */
      midi_current_measure = midi_current_measure->next;
      if (!midi_current_measure) {
        midi_current_measure = midi_loop_at;
      }
      /* へろへろ？ */
      if (midi_random_pitch_bend > 0) {
        static unsigned counter;
        if (counter++ % 2 == 0) {
          midi_current_measure = random_pitch_bend(midi_current_measure);
        }
      } else if (midi_random_pitch_bend < 0) {
        /* 回復 */
        midi_random_pitch_bend = 0;        
        midi_current_measure = restore_pitch_bend(midi_current_measure);
      }
      
      midi_mh_used = (midi_mh_used + 1) % MAX_MIDI_SEGMENT;
      error = midi_prepare_current_measure(&midi_mh[midi_mh_used]);
      if (error) {
        midi_error_why(error);
      } else {
        midiStreamOut(midi_device, &midi_mh[midi_mh_used],
                      sizeof(midi_mh[midi_mh_used]));
      }
    }
    break;
    
  case MOM_POSITIONCB:
    {
      /* コールバックイベントを発生させたイベントの位置を特定 */
      MIDIHDR   *mh = (MIDIHDR *)param1;
      MIDIEVENT *me = (MIDIEVENT *)(mh->lpData + mh->dwOffset);

      /* それはテンポの指定に決まっている */
      midi_tempo = me->dwEvent & 0x00FFFFFF;

             if (midi_speed < 0) {
        midi_set_tempo((unsigned)(midi_tempo * 1.5));
      } else if (midi_speed > 0) {
        midi_set_tempo((unsigned)(midi_tempo / 1.5));
      } else {
        midi_set_tempo(midi_tempo);
      }
    }
    break;
  }
}
#else
/* サイズを返す */
int print_midi_event(MIDIEVENT *me, unsigned now)
{
  int i;

  if (me->dwEvent & MEVT_F_CALLBACK) {
    printf("%10u: Callback event should be tempo=%d\n",
           now, me->dwEvent & 0xffffff);
    return 12;
  }
  if (me->dwEvent & MEVT_F_LONG) {
    int size = me->dwEvent & 0xffffff;
    unsigned char *p = (unsigned char *)me->dwParms;
    int i;
    printf("%10u: Long event ", now);
    for (i = 0; i < size; i++, p++) {
      printf(" %02x", *p);
    }
    printf("\n");
    switch (size % 4) {
    case 1: p++;
    case 2: p++;
    case 3: p++;
    }
    return 12 + p - (unsigned char *)me->dwParms;
  } else {
    unsigned stat = me->dwEvent & 0xff;
    unsigned data1 = (me->dwEvent >>  8) & 0xff;
    unsigned data2 = (me->dwEvent >> 16) & 0xff;
    printf("%10u: %02x %02x %02x", now, stat, data1, data2);
    if (stat & 0x80) {
      if ((stat & 0xf0) == 0xe0) {
        printf(" (pitch-bend %2d:(%04x=%+4d))",
               (stat & 0x0f) + 1,
               data1 | (data2 << 7),               
               (int)(data1 | (data2 << 7)) - 0x2000);
      }
    } else {
      printf(" (maybe dummy)");
    }
    printf("\n");
    return 12;
  }
}

int main(int argc, char *argv[])
{
  midi_measure_t *mm;
  int num_measures;
  unsigned track_time;

  if (argc != 2) {
    fprintf(stderr, "mididump midi-file\n");
    exit(EXIT_FAILURE);
  }

  option_debug = 1;
  
  if (midi_load(argv[1])) {
    fprintf(stderr, "%s: Cound not load.\n", argv[1]);
    exit(EXIT_FAILURE);
  }

  num_measures = 1;
  track_time = 0;
  
  for (mm = midi_measures; mm; mm = mm->next) {
    int i;
    MIDIEVENT *top, *end;
    top = (MIDIEVENT *)mm->events;
    end = (MIDIEVENT *)(mm->events + mm->size);

    printf("----------------measure #%d (size=%u, at=%p)\n",
	   num_measures, mm->size, mm);

    for (i = 0; i < 16; i++) {
      switch (i) {
      case 4: case 8: case 12:
        printf("\n");
      case 0:
        printf("pitch-bends:");
      }
      printf(" %2d:(%+4d)", i + 1, mm->pitch_bends[i]);
    }
    printf("\n");
    
    while (top < end) {
      int size;

      track_time += top->dwDeltaTime;
      size = print_midi_event(top, top->dwDeltaTime);
      top = (MIDIEVENT *)((unsigned char *)top + size);
    }

    printf("(ELAPSED=%u)\n", track_time);
    num_measures++;
  }
  return 0;
}
#endif /* MIDIDUMP */