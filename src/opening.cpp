#include "xanadu.h"
#include "opening.h"
#include "fade.h"

typedef struct {
  char *image;
  int x;
  int y;
  int se;
} visual_t;

static const visual_t visuals1[] = { 
  { "xa1/opening/background.bmp",  0,   0, SE_SOMEWHAT1 },
  { "xa1/opening/battler.bmp",   360, 144, SE_SOMEWHAT1 },
  { "xa1/opening/witch.bmp",     144, 144, SE_SOMEWHAT1 },
  { "xa1/opening/wizard.bmp",    464, 152, SE_SOMEWHAT1 },
  { "xa1/opening/robber.bmp",     16, 168, SE_SOMEWHAT1 },
  { "xa1/opening/swordman.bmp",  232, 144, SE_SOMEWHAT2 },
  { NULL, 0, 0, 0 }
};

static const visual_t visuals2[] = {
  { "xa2/opening/title.bmp",      24,  24, SE_SOMEWHAT1 },
  { "xa2/opening/subtitle.bmp",  112, 248, SE_SOMEWHAT1 },
  { "xa2/opening/hero.bmp",      368,  16, SE_SOMEWHAT1 },
  { NULL, 0, 0, 0 }
};

static int opening_step;
static unsigned fade_mask;

static void wait_forever(void);
static void restore_context(void);

int init_opening(void)
{
  opening_step = 0;
  fade_mask = 0;
  
  /* BGM mute */
  bgm_play("");
  
  /* 効果音 */
  se_load(SE_SOMEWHAT1, se_data.opening0);
  se_load(SE_SOMEWHAT2, se_data.opening1);

  visual_image = nullptr;

  fill_image(clip_overall, 0, 0, clip_overall->getWidth(), clip_overall->getHeight(),
             SDL_::Color::BLACK);
  update(rect_overall);
  update_immediately();

  return CONTEXT_OPENING;
}

void opening_enter(void)
{
  const visual_t *visuals;
  int x;
  int y;

  if (get_keystate(VK_SPACE) || get_keystate(VK_RETURN)) {
    restore_context();
    return;
  }

  visuals = user.environment.scenario == 0 ? visuals1 : visuals2;

  if (visuals[opening_step].image == NULL) {
    wait_forever();
    return;
  }
  x = visuals[opening_step].x;
  y = visuals[opening_step].y;
  switch (fade_mask) {
  case 0x000000:
    {
      char path[BUFSIZ];
      sprintf(path, IMAGE_DIR "/%s", visuals[opening_step].image);
      
      visual_image = load_image(path);
    }
    se_play(visuals[opening_step].se);
    /* thorugh */
    
  case 0x0000FF:
    extend_context(init_fade(clip_overall, x, y, visual_image, fade_mask));
    fade_mask = 0xFF00FF;
    break;
    
  case 0xFF00FF:
    extend_context(init_fade(clip_overall, x, y, visual_image, fade_mask));
    fade_mask = 0xFFFFFF;
    break;
    
  case 0xFFFFFF:
    extend_context(init_fade(clip_overall, x, y, visual_image, fade_mask));
    opening_step++;
    fade_mask = 0x000000;
    break;
    
  default:
    restore_context();
    break;
  }
}

void opening_leave(void)
{
  thunk_key_event = NULL;  
}

void restore_context(void)
{
  load_background(IMAGE_DIR "/user/frame.bmp");
  update(rect_overall);
  resume_context();
}

static void opening_key_event(int c)
{
  restore_context();  
}

void wait_forever(void)
{
  thunk_key_event = opening_key_event;
  bgm_play(bgm_data.theme[user.environment.scenario].opening);
}
