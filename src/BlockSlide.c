#include <pebble.h>

#include "BlockSlide.h"
#include "pposdemo.h"

/*
 * 2014.06.18: Round Corners option implementation thanks to Ron64
 */

// Languages
enum {
  LANG_DUTCH =  0,
  LANG_ENGLISH,
  LANG_FRENCH,
  LANG_GERMAN,
  LANG_SPANISH,
  LANG_ITALIAN,
  LANG_MAX
};

enum {
  CONFIG_KEY_DATEORDER = 10,
  CONFIG_KEY_WEEKDAY = 11,
  CONFIG_KEY_LANG = 12,
  CONFIG_KEY_STRIPES = 13,
  CONFIG_KEY_ROUNDCORNERS = 14,
  CONFIG_KEY_FULLDIGITS = 15
};


#define TILEW 22
#define TILEH 13
#define DTILEW 5
#define DTILEH 4
#define HSPACE 8
#define DHSPACE 4
#define VSPACE 8
#define DIGIT_CHANGE_ANIM_DURATION 800
#define STARTDELAY 700
#define BATTERYDELAY 5000
#define SCREENW 144
#define SCREENH 168
#define CX 72
#define CY 84
#define NUMSLOTS 12
#define TILECORNERRADIUS 4
#define DTILECORNERRADIUS 1

char weekDay[LANG_MAX][7][3] = {
  { "ZO", "MA", "DI", "WO", "DO", "VR", "ZA" },  // Dutch
  { "SU", "MO", "TU", "WE", "TH", "FR", "SA" },  // English
  { "DI", "LU", "MA", "ME", "JE", "VE", "SA" },  // French
  { "SO", "MO", "DI", "MI", "DO", "FR", "SA" },  // German
  { "DO", "LU", "MA", "MI", "JU", "VI", "SA" },  // Spanish
  { "DO", "LU", "MA", "ME", "GI", "VE", "SA" }  // Italian
};

uint8_t curLang = LANG_ENGLISH;
uint8_t showWeekday = 0;
uint8_t USDate = 1;
uint8_t stripedDigits = 1;
uint8_t roundCorners = 1;
uint8_t fullDigits = 0;
bool digitShapesChanged = false;


typedef struct {
  Layer *layer;
  int   prevDigit;
  int   curDigit;
  int   tileWidth;
  int   tileHeight;
  uint32_t normTime;
  int   cornerRadius;
} digitSlot;

Window *window;

digitSlot slot[NUMSLOTS]; // 4 big digits for the hour, 6 small for the date
int startDigit[NUMSLOTS] = {
  SPACE_L,
  SPACE_R,
  SPACE_L,
  SPACE_R,
  SPACE_D,
  'B'-'0',
  'L'-'0',
  'K'-'0',
  'S'-'0',
  'L'-'0',
  'D'-'0',
  SPACE_D
};

bool splashEnded = false;
bool animRunning = false;
bool lastBluetoothStatus = true;
static AppTimer *timer;

AnimationImplementation animImpl;
Animation *anim;

char buffer[256] = "";

GRect slotFrame(int i) {
  int x, y;
  int w = slot[i].tileWidth*3;
  int h = slot[i].tileHeight*5;

  if (i<4) {
    // Hour slot -> big digits
    if (i%2) {
      x = CX + HSPACE/2; // i = 1 or 3
    } else {
      x = CX - HSPACE/2 - w; // i = 0 or 2
    }

    if (i<2) {
      y = 1;
    } else {
      y = 1 + h + VSPACE;
    }
  } else {
    // Date slot -> small digits
    //x = CX + (i-7)*(w+DHSPACE) + DHSPACE/2 - ((i<6)?16:0) + ((i>7)?16:0);
    x = 5 + (i-4)*2 + (i-4)*w;
    y = SCREENH - h - VSPACE/2;
  }

  return GRect(x, y, w, h);
}

digitSlot *findSlot(Layer *layer) {
  int i;
  for (i=0; i<NUMSLOTS; i++) {
    if (slot[i].layer == layer) {
      return &slot[i];
    }
  }

  return NULL;
}

void updateSlot(Layer *layer, GContext *ctx) {
  int t, tx1, tx2, ty1, ty2, ox, oy;
  int cornerRadius = 0;
  uint8_t curCorner, prevCorner;
  GCornerMask cornerMask;
  GRect bounds;
  digitSlot *slot;
  static unsigned int animMiddle = ANIMATION_NORMALIZED_MAX / 2;

  slot = findSlot(layer);
  graphics_context_set_fill_color(ctx, backColor);
  bounds = layer_get_bounds(slot->layer);
  graphics_fill_rect(ctx, GRect(0, 0, bounds.size.w, bounds.size.h), 0, GCornerNone);

  curCorner=slot->curDigit;
/*
 if (curCorner > NO_ROUND_CORNER) {
    curCorner = NO_ROUND_CORNER; //prevent error on mising corner data
  }
*/
  prevCorner=slot->prevDigit;
/*
 if (prevCorner > NO_ROUND_CORNER) {
    prevCorner = NO_ROUND_CORNER; //prevent error on mising corner data
  }
*/
  for (t=0; t<13; t++) {
    cornerMask = GCornerNone;
    cornerRadius = 0;
    if (digits[slot->curDigit][t][0] != digits[slot->prevDigit][t][0]
        || digits[slot->curDigit][t][1] != digits[slot->prevDigit][t][1]) {
      if (slot->normTime == ANIMATION_NORMALIZED_MAX) {
        ox = digits[slot->curDigit][t][0]*slot->tileWidth;
        oy = digits[slot->curDigit][t][1]*slot->tileHeight;
      } else {
        tx1 = digits[slot->prevDigit][t][0]*slot->tileWidth;
        tx2 = digits[slot->curDigit][t][0]*slot->tileWidth;
        ty1 = digits[slot->prevDigit][t][1]*slot->tileHeight;
        ty2 = digits[slot->curDigit][t][1]*slot->tileHeight;

        ox = slot->normTime * (tx2-tx1) / ANIMATION_NORMALIZED_MAX + tx1;
        oy = slot->normTime * (ty2-ty1) / ANIMATION_NORMALIZED_MAX + ty1;
      }
    } else {
      ox = digits[slot->curDigit][t][0]*slot->tileWidth;
      oy = digits[slot->curDigit][t][1]*slot->tileHeight;
    }

    if (roundCorners) {
      if (digitCorners[curCorner][t] != digitCorners[prevCorner][t]) {
        // ToDo: fix calculation. Corner become smaller till half, and bigger afterward;
        if (slot->normTime > ANIMATION_NORMALIZED_MAX) {
          cornerRadius = 0;
          cornerMask = digitCorners[prevCorner][t]; //point to corner of prv digit
        } else {
          cornerRadius = 2*slot->cornerRadius * slot->normTime / ANIMATION_NORMALIZED_MAX - slot->cornerRadius;
          if (cornerRadius < 0) {
            cornerRadius = -cornerRadius;
          }
          if (slot->normTime < animMiddle) {
            cornerMask = digitCorners[prevCorner][t];
          } else {
            cornerMask = digitCorners[curCorner][t];
          }
        }
      } else {
        cornerRadius = slot->cornerRadius;
				cornerMask = digitCorners[curCorner][t];
      }
    }

    graphics_context_set_fill_color(ctx, foreColor);
    graphics_fill_rect(ctx, GRect(ox, oy, slot->tileWidth, slot->tileHeight-stripedDigits), cornerRadius, cornerMask);
  }
}

void initSlot(int i, Layer *parent) {
  slot[i].normTime = ANIMATION_NORMALIZED_MAX;
  slot[i].prevDigit = 0;
  slot[i].curDigit = startDigit[i];
  if (i<4) {
    // Hour slots -> big digits
    slot[i].tileWidth = TILEW;
    slot[i].tileHeight = TILEH;
    slot[i].cornerRadius = TILECORNERRADIUS;
  } else {
    // Date slots -> small digits
    slot[i].tileWidth = DTILEW;
    slot[i].tileHeight = DTILEH;
    slot[i].cornerRadius = DTILECORNERRADIUS;
  }
  slot[i].layer = layer_create(slotFrame(i));
  layer_set_update_proc(slot[i].layer, updateSlot);
  layer_add_child(parent, slot[i].layer);
}

void deinitSlot(int i) {
  layer_destroy(slot[i].layer);
}

void redrawAllSlots() {
  int i;

  for (i=0; i<NUMSLOTS; i++) {
    layer_mark_dirty(slot[i].layer);
  }
}

void animateDigits(struct Animation *anim, const uint32_t normTime) {
  int i;

  for (i=0; i<NUMSLOTS; i++) {
    if (slot[i].curDigit != slot[i].prevDigit) {
      slot[i].normTime = normTime;
      layer_mark_dirty(slot[i].layer);
    }
  }

  if (normTime == ANIMATION_NORMALIZED_MAX) {
    animRunning = false;
  }
}

void handle_tick(struct tm *now, TimeUnits units_changed) {
  int h, m;
  int D, M;
  int i;
  int wd = 0;
  int Y = 0;

  //APP_LOG(APP_LOG_LEVEL_DEBUG, "Entering handle_tick");

  if (splashEnded && !animation_is_scheduled(anim)) {
    h = now->tm_hour;
    m = now->tm_min;
    D = now->tm_mday;
    M = now->tm_mon+1;

    if (showWeekday) {
      wd = now->tm_wday;
    } else {
      Y = now->tm_year%100;
    }

    if (!clock_is_24h_style()) {
      h = h%12;
      if (h == 0) {
        h = 12;
      }
    }

    for (i=0; i<NUMSLOTS; i++) {
      slot[i].prevDigit = slot[i].curDigit;
    }

    // Hour slots
    slot[0].curDigit = h/10;
    slot[1].curDigit = h%10;
    slot[2].curDigit = m/10;
    slot[3].curDigit = m%10;
    slot[6].curDigit = SPACE_D;
    slot[9].curDigit = SPACE_D;


    // Date slots
    if (showWeekday && USDate) {
      slot[4].curDigit = weekDay[curLang][wd][0] - '0';
      slot[5].curDigit = weekDay[curLang][wd][1] - '0';
      slot[7].curDigit = M/10;
      slot[8].curDigit = M%10;
      slot[10].curDigit = D/10;
      slot[11].curDigit = D%10;
    } else if (showWeekday && !USDate) {
      slot[4].curDigit = weekDay[curLang][wd][0] - '0';
      slot[5].curDigit = weekDay[curLang][wd][1] - '0';
      slot[7].curDigit = D/10;
      slot[8].curDigit = D%10;
      slot[10].curDigit = M/10;
      slot[11].curDigit = M%10;
    } else if (!showWeekday && USDate) {
      slot[4].curDigit = M/10;
      slot[5].curDigit = M%10;
      slot[7].curDigit = D/10;
      slot[8].curDigit = D%10;
      slot[10].curDigit = Y/10;
      slot[11].curDigit = Y%10;
    } else {
      slot[4].curDigit = D/10;
      slot[5].curDigit = D%10;
      slot[7].curDigit = M/10;
      slot[8].curDigit = M%10;
      slot[10].curDigit = Y/10;
      slot[11].curDigit = Y%10;
    }

    animation_schedule(anim);
  }
}

void do_update() {
  time_t curTime;
  struct tm *now;

  splashEnded = true;
  curTime = time(NULL);
  now = localtime(&curTime);
  handle_tick(now, 0);
}

void handle_timer(void *data) {
  timer=NULL;
  do_update();
}

void handle_tap(AccelAxisType axis, int32_t direction) {
  static BatteryChargeState chargeState;
  int i, s;

  if (splashEnded && !animRunning) {
    if (animation_is_scheduled(anim)) {
      animation_unschedule(anim);
    }

    animRunning = true;

    chargeState = battery_state_service_peek();
    s = chargeState.charge_percent;

    for (i=0; i<NUMSLOTS; i++) {
      slot[i].prevDigit = slot[i].curDigit;
    }

    slot[4].curDigit = 'B' - '0';
    slot[5].curDigit = 'A' - '0';
    slot[6].curDigit = 'T' - '0';
    slot[7].curDigit = SPACE_D;
    slot[8].curDigit = (s==100)?1:SPACE_D;
    slot[9].curDigit = (s<100)?s/10:0;
    slot[10].curDigit = (s<100)?s/100:0;
    slot[11].curDigit = PERCENT;

    animation_schedule(anim);
    if (timer==NULL)
      timer= app_timer_register(BATTERYDELAY, handle_timer, NULL);
  }
}

void handle_bluetooth(bool connected) {
  int i;

  if (lastBluetoothStatus == connected) {
    return;
  } else {
    lastBluetoothStatus = connected;

    if (splashEnded && !animRunning) {
      if (animation_is_scheduled(anim)) {
        animation_unschedule(anim);
      }

      animRunning = true;

      for (i=0; i<NUMSLOTS; i++) {
        slot[i].prevDigit = slot[i].curDigit;
      }

      slot[0].curDigit = 'B' - '0';
      slot[1].curDigit = 'T' - '0';

      if (connected) {
        slot[2].curDigit = 'O' - '0';
        slot[3].curDigit = 'K' - '0';

        slot[4].curDigit  = SPACE_D;
        slot[5].curDigit  = SPACE_D;
        slot[6].curDigit  = SPACE_D;
        slot[7].curDigit  = SPACE_D;
        slot[8].curDigit  = SPACE_D;
        slot[9].curDigit  = SPACE_D;
        slot[10].curDigit = SPACE_D;
        slot[11].curDigit = SPACE_D;

        vibes_double_pulse();
      } else {
        slot[2].curDigit = SPACE_L;
        slot[3].curDigit = SPACE_R;

        slot[4].curDigit  = SPACE_D;
        slot[5].curDigit  = 'F' - '0';
        slot[6].curDigit  = 'A' - '0';
        slot[7].curDigit  = 'I' - '0';
        slot[8].curDigit  = 'L' - '0';
        slot[9].curDigit  = 'E' - '0';
        slot[10].curDigit = 'D' - '0';
        slot[11].curDigit = SPACE_D;

        vibes_long_pulse();
      }

      animation_schedule(anim);
      if (timer==NULL)
        timer=app_timer_register(BATTERYDELAY, handle_timer, NULL);
    }
  }
}

void applyConfig() {
  if (splashEnded) {
    do_update();
  }
}

/*bool checkAndSaveInt(int *var, int val, int key) {
  int ret;

  if (*var != val) {
    *var = val;
    ret = persist_write_int(key, val);
    if (ret < 0) {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "checkAndSaveInt() : persist_write_int(%d, %d) returned %d",
              val, key, ret);
    }
    return true;
  } else {
    return false;
  }
}*/
static inline int8_t getDigitTile(int8_t *digit, int x, int y) {
  if ( (x < 0) || (x > 2) || (y < 0) || (y > 4)) {
    return 0;
  } else {
    return digit[x+3*y];
  }
}

uint8_t calcTileCorners(int8_t *digit, int x, int y) {
  int i, j, ox, oy;
  uint8_t mask = GCornerNone;
  int8_t surroundingTile[3][3];

  for (j=0, oy=y-1; j<3; j++, oy++) {
    for (i=0, ox=x-1; i<3; i++, ox++) {
      surroundingTile[i][j] = getDigitTile(digit, ox, oy);
    }
  }

  // Top Left Corner
  if ( (surroundingTile[0][0] == 0) && (surroundingTile[1][0] == 0) && (surroundingTile[0][1] == 0) ) {
    mask |= GCornerTopLeft;
  }

  // Top Right Corner
  if ( (surroundingTile[1][0] == 0) && (surroundingTile[2][0] == 0) && (surroundingTile[2][1] == 0) ) {
    mask |= GCornerTopRight;
  }

  // Bottom Left Corner
  if ( (surroundingTile[0][1] == 0) && (surroundingTile[0][2] == 0) && (surroundingTile[1][2] == 0) ) {
    mask |= GCornerBottomLeft;
  }

  // Bottom Right Corner
  if ( (surroundingTile[1][2] == 0) && (surroundingTile[2][1] == 0) && (surroundingTile[2][2] == 0) ) {
    mask |= GCornerBottomRight;
  }

  return mask;
}

void calcDigitCorners(int i) {
  int t, x, y;
  int8_t digit[15];
  uint8_t cornerMask;

  // Clear digit matrix
  for (t=0; t<15; t++) {
    digit[t] = 0;
  }

  // Compute digit matrix
  for (t=0; t<13; t++) {
    x = digits[i][t][0];
    y = digits[i][t][1];

    if ( (x >= 0) && (x <= 2) && (y >= 0) && (y <= 4)) {
      digit[x + 3*y] = 1;
    }
  }

  // Compute round corners from matrix
  for (y=0; y<5; y++) {
    for (x=0; x<3; x++) {
      cornerMask = GCornerNone;
      if (digit[x+3*y]) {
        cornerMask = calcTileCorners(digit, x, y);
      }
      // Fill the corners array
      for (t=0; t<13; t++) {
        if ( (digits[i][t][0] == x) && (digits[i][t][1] == y) ) {
          digitCorners[i][t] = cornerMask;
        }
      }
    }
  }
}

void swapDigitShapes() {
  int i;

  for (i = 0; i < 13; i++) {
    // Swap digit 2
    digits[2][i][0] = digit2[fullDigits][i][0];
    digits[2][i][1] = digit2[fullDigits][i][1];
    // Swap digit 4
    digits[4][i][0] = digit4[fullDigits][i][0];
    digits[4][i][1] = digit4[fullDigits][i][1];
    // Swap digit 5
    digits[5][i][0] = digit5[fullDigits][i][0];
    digits[5][i][1] = digit5[fullDigits][i][1];
  }

  calcDigitCorners(2);
  calcDigitCorners(4);
  calcDigitCorners(5);
}
/*
void in_dropped_handler(AppMessageResult reason, void *context) {
}

void in_received_handler(DictionaryIterator *received, void *context) {
  bool somethingChanged = false;
  bool digitShapesHaveToBeSwapped = false;

  Tuple *dateorder = dict_find(received, CONFIG_KEY_DATEORDER);
  Tuple *weekday = dict_find(received, CONFIG_KEY_WEEKDAY);
  Tuple *lang = dict_find(received, CONFIG_KEY_LANG);
  Tuple *stripes = dict_find(received, CONFIG_KEY_STRIPES);
  Tuple *corners = dict_find(received, CONFIG_KEY_ROUNDCORNERS);
  Tuple *digits = dict_find(received, CONFIG_KEY_FULLDIGITS);

  if (dateorder && weekday && lang && stripes && corners && digits) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Received config (dateorder=%d, weekday=%d, lang=%d, stripes=%d, corners=%d, digits=%d)",
            (int)dateorder->value->int32, (int)weekday->value->int32, (int)lang->value->int32,
            (int)stripes->value->int32, (int)corners->value->int32, (int)digits->value->int32);

    somethingChanged |= checkAndSaveInt(&USDate, dateorder->value->int32, CONFIG_KEY_DATEORDER);
    somethingChanged |= checkAndSaveInt(&showWeekday, weekday->value->int32, CONFIG_KEY_WEEKDAY);
    somethingChanged |= checkAndSaveInt(&curLang, lang->value->int32, CONFIG_KEY_LANG);

    digitShapesChanged = false;
    digitShapesChanged |= checkAndSaveInt(&stripedDigits, stripes->value->int32, CONFIG_KEY_STRIPES);
    digitShapesChanged |= checkAndSaveInt(&roundCorners, corners->value->int32, CONFIG_KEY_ROUNDCORNERS);

    digitShapesHaveToBeSwapped = checkAndSaveInt(&fullDigits, digits->value->int32, CONFIG_KEY_FULLDIGITS);
    digitShapesChanged |= digitShapesHaveToBeSwapped;

    if (digitShapesHaveToBeSwapped) {
      swapDigitShapes();
    }

    if (somethingChanged) {
      applyConfig();
    } else if (digitShapesChanged) {
      digitShapesChanged = false;
      redrawAllSlots();
    }
  }
}

void readConfig() {
  if (persist_exists(CONFIG_KEY_DATEORDER)) {
    USDate = persist_read_int(CONFIG_KEY_DATEORDER);
  } else {
    USDate = 1;
    persist_write_int(CONFIG_KEY_DATEORDER, USDate);
  }

  if (persist_exists(CONFIG_KEY_WEEKDAY)) {
    showWeekday = persist_read_int(CONFIG_KEY_WEEKDAY);
  } else {
    showWeekday = 0;
    persist_write_int(CONFIG_KEY_WEEKDAY, showWeekday);
  }

  if (persist_exists(CONFIG_KEY_LANG)) {
    curLang = persist_read_int(CONFIG_KEY_LANG);
  } else {
    curLang = LANG_ENGLISH;
    persist_write_int(CONFIG_KEY_LANG, curLang);
  }

  if (persist_exists(CONFIG_KEY_STRIPES)) {
    stripedDigits = persist_read_int(CONFIG_KEY_STRIPES);
  } else {
    stripedDigits = 1;
    persist_write_int(CONFIG_KEY_STRIPES, stripedDigits);
  }

  if (persist_exists(CONFIG_KEY_ROUNDCORNERS)) {
    roundCorners = persist_read_int(CONFIG_KEY_ROUNDCORNERS);
  } else {
    roundCorners = 1;
    persist_write_int(CONFIG_KEY_ROUNDCORNERS, roundCorners);
  }

  if (persist_exists(CONFIG_KEY_FULLDIGITS)) {
    fullDigits = persist_read_int(CONFIG_KEY_FULLDIGITS);
  } else {
    fullDigits = 0;
    persist_write_int(CONFIG_KEY_FULLDIGITS, fullDigits);
  }

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Stored config :");
  APP_LOG(APP_LOG_LEVEL_DEBUG, "  dateorder=%d, weekday=%d, lang=%d, stripedDigits=%d, roundCorners=%d, fullDigits=%d",
          USDate, showWeekday, curLang, stripedDigits, roundCorners, fullDigits);
}

static void app_message_init(void) {
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_open(128, 128);
}*/

void initDigitCorners() {
  int i;
  for (i=0; i<NUM_DIGITS; i++) {
    calcDigitCorners(i);
  }
}

void redraw_block() {
	redrawAllSlots();
}

void load_block() {
  //Layer *rootLayer;
  int i;
  digits= malloc(NUM_DIGITS*13*2);
  digitCorners = malloc(NUM_DIGITS*13);

  resource_load(resource_get_handle(RESOURCE_ID_BIN_BLOCK_DIG), (uint8_t *)digits, NUM_DIGITS*13*2);

  //window = window_create();
  //window_set_background_color(window, backColor);
  //window_stack_push(window, true);

  //readConfig();
  if (AllSet[SET_LANG]<LANG_MAX)
    curLang = AllSet[SET_LANG];
  if (AllSet[SET_BLK_DAY]<=1)
    showWeekday = AllSet[SET_BLK_DAY];
  if (AllSet[SET_DATE]<=1)
    USDate = AllSet[SET_DATE];
  if (AllSet[SET_BLOCK_LINE]<=1)
    stripedDigits = AllSet[SET_BLOCK_LINE];
  if (AllSet[SET_BLK_RND]<=1)
  roundCorners = AllSet[SET_BLK_RND];
  if (AllSet[SET_FULL_DIG]<=1)
    fullDigits = AllSet[SET_FULL_DIG];
	
  swapDigitShapes();
  //app_message_init();

  //rootLayer = window_get_root_layer(window);//

  for (i=0; i<NUMSLOTS; i++) {
    initSlot(i, rootLayer);
  }

  initDigitCorners();

  animImpl.setup = NULL;
  animImpl.update = animateDigits;
  animImpl.teardown = NULL;

  anim = animation_create();
  animation_set_delay(anim, 0);
  animation_set_duration(anim, DIGIT_CHANGE_ANIM_DURATION);
  animation_set_implementation(anim, &animImpl);

  timer = app_timer_register(STARTDELAY, handle_timer, NULL);

  tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);

  accel_tap_service_subscribe(handle_tap);

  lastBluetoothStatus = bluetooth_connection_service_peek();
  bluetooth_connection_service_subscribe(handle_bluetooth);
}

void unload_block() {
  int i;
  if (timer != NULL) {
    app_timer_cancel(timer);
    timer=NULL;
  }

  bluetooth_connection_service_unsubscribe();
  accel_tap_service_unsubscribe();
  tick_timer_service_unsubscribe();
  animation_destroy(anim);
  free(digits);
  free(digitCorners);
  for (i=0; i<NUMSLOTS; i++) {
    deinitSlot(i);
  }
  
  //window_destroy(window);
}

//removed main

void info_block(void* disp_info)
{
	char Defaults[6][3]={{SET_DATE,1,0}, {SET_LANG,LANG_ENGLISH,LANG_ENGLISH}, {SET_BLOCK_LINE,1,0}, 
                          {SET_BLK_DAY,0,1}, {SET_BLK_RND,1,0}, {SET_FULL_DIG,0,1}};
		
	display_info * info = (display_info *) disp_info;
	snprintf(info->app_name, sizeof(info->app_name),"Block Slide");
	snprintf(info->dev_name, sizeof(info->dev_name),"Jnm");	
	memcpy(info->def_set, &Defaults, 6*3 );
	info->def_cnt= 6; //six setting items
	info->def_opt= 6; //all six have alternative value
}
