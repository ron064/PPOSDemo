#include <pebble.h>
//#include "autoconfig.h"
#include "pposdemo.h" 
#include "hoppicker_gpath.h"

static Layer  *layer;
static InverterLayer* inv_layer;

static GBitmap *bt_disconnected = NULL;

GFont custom_font;
GFont small_font;

/*static GPath *hour_arrow[2];
static const GPathInfo LINE_HAND_POINTS[2] = {
  {4,(GPoint []) {{-3, 0},{-3, -300},{3, -300},{3, 0}}},
  {4,(GPoint []) {{-3, 0},{3, 0},{3, -300},{-3, -300}}}
};
static const GPathInfo ARROW_HAND_POINTS[2] = {
  {4,(GPoint []) {{-6, 0},{-2, -175},{2, -175},{6, 0}}},
  {4,(GPoint []) {{-6, 0},{6, 0},{2, -175},{-2, -175}}}
};*/

static char txt[24][3] = {"0","1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16","17","18","19","20","21","22","23"};

static char date_text[3] = "31";

static AppTimer *timer;

static int anim_minutes = 0;
static bool isAnimating = false;
static int increaseAnimation = 20;

static bool btConnected = false;

static bool getColor()
{
  return (AllSet[SET_HPBW]==1);
}

static bool containsCircle(GPoint center, int radius){
  return center.x - radius > 0 && center.x + radius < 144 && center.y - radius > 0 && center.y + radius < 168;
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(layer);
}

static void animation_timer_callback(void *data) {
  timer = NULL;
  layer_mark_dirty(layer);
    if(isAnimating)
      timer = app_timer_register(70 /* milliseconds */, animation_timer_callback, NULL);
}

static void drawClock(GPoint center, int hour, GContext *ctx){
  GPoint segA;
  GPoint segC;
  
  const int16_t secondHandLengthA = 150;
  const int16_t secondHandLengthC = 180;

  graphics_context_set_fill_color(ctx, getColor() ? GColorBlack : GColorWhite);

  graphics_fill_circle(ctx, center, 180);
    
  graphics_context_set_text_color(ctx, getColor() ? GColorWhite : GColorBlack );
  
  int minhour=(hour+24)-2;
  int maxhour=hour+24+3;
  
  for(int i=minhour*4; i<maxhour*4; i++){
    int32_t angle = TRIG_MAX_ANGLE * (i % (12*4)) / (12*4);
    segA.y = (int16_t)(-cos_lookup(angle) * (int32_t)secondHandLengthA / TRIG_MAX_RATIO) + center.y;
    segA.x = (int16_t)(sin_lookup(angle) * (int32_t)secondHandLengthA / TRIG_MAX_RATIO) + center.x;
    segC.y = (int16_t)(-cos_lookup(angle) * (int32_t)secondHandLengthC / TRIG_MAX_RATIO) + center.y;
    segC.x = (int16_t)(sin_lookup(angle) * (int32_t)secondHandLengthC / TRIG_MAX_RATIO) + center.x;
    
    if(i % 4 == 0) {
      graphics_context_set_fill_color(ctx, getColor() ? GColorBlack : GColorWhite);
      graphics_fill_circle( ctx, segC, 8);
      graphics_context_set_fill_color(ctx, getColor() ? GColorWhite : GColorBlack);
      graphics_fill_circle( ctx, segC, 6);
      
      if(clock_is_24h_style()){
        graphics_draw_text(ctx,
          txt[(i % (24*4))/4],
          custom_font,
          GRect(segA.x-25, segA.y-25, 50, 50),
          GTextOverflowModeWordWrap,
          GTextAlignmentCenter,
          NULL);  
      }
      else {
        graphics_draw_text(ctx,
          (i % (12*4))/4 == 0 ? "12" : txt[(i % (12*4))/4],
          custom_font,
          GRect(segA.x-25, segA.y-25, 50, 50),
          GTextOverflowModeWordWrap,
          GTextAlignmentCenter,
          NULL);
      }
    }
    else {
      graphics_context_set_fill_color(ctx, getColor() ? GColorWhite : GColorBlack);
      graphics_fill_circle( ctx, segC, 3);
    }
  }
}

static void drawDate(GPoint center, int angle, int date, GContext *ctx){
  GPoint segA;

  int16_t posFromCenter = 100;
  do{
    segA.y = (int16_t)(-cos_lookup(angle) * (int32_t)posFromCenter / TRIG_MAX_RATIO) + center.y;
    segA.x = (int16_t)(sin_lookup(angle) * (int32_t)posFromCenter / TRIG_MAX_RATIO) + center.x;
    posFromCenter--;
  }
  while(containsCircle(segA, 14 + 1));

  snprintf(date_text, sizeof(date_text), "%d", date); 

  graphics_context_set_fill_color(ctx, getColor() ? GColorWhite : GColorBlack);
  graphics_fill_circle(ctx, GPoint(segA.x, segA.y), 14);
  graphics_context_set_fill_color(ctx, getColor() ? GColorBlack : GColorWhite);
  graphics_fill_circle(ctx, GPoint(segA.x, segA.y), 12);
  graphics_context_set_text_color(ctx, getColor() ? GColorWhite : GColorBlack );
  graphics_draw_text(ctx,
          date_text,
          small_font,
          GRect(segA.x-12, segA.y-11, 25, 25),
          GTextOverflowModeWordWrap,
          GTextAlignmentCenter,
          NULL);
}

static void drawHand(GPoint secondHand, int angle, GContext *ctx){
  graphics_context_set_stroke_color(ctx, getColor() ? GColorBlack : GColorWhite);
  graphics_context_set_fill_color(ctx, getColor() ? GColorWhite : GColorBlack);
  
  for(int i=0; i<2; i++){
    gpath_move_to(hour_arrow[i], secondHand);
    gpath_rotate_to(hour_arrow[i], angle);
    gpath_draw_filled(ctx, hour_arrow[i]);
    gpath_draw_outline(ctx, hour_arrow[i]);
  }
}

static void layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  const GPoint center = grect_center_point(&bounds);
  const int16_t secondHandLength = 150;

  GPoint secondHand;

  int hours = 0;
  int minutes = 0;

  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  if(isAnimating){
    hours = anim_minutes / 60;
    minutes = anim_minutes - hours * 60;
    if((increaseAnimation > 0 && anim_minutes >= t->tm_hour * 60 + t->tm_min) || (increaseAnimation < 0 && anim_minutes <= t->tm_hour * 60 + t->tm_min)){
      hours = t->tm_hour;
      minutes = t->tm_min;
      isAnimating = false;
    }
    anim_minutes+=increaseAnimation;
  }
  else {
    hours = t->tm_hour;
    minutes = t->tm_min;
  }


  int32_t second_angle = TRIG_MAX_ANGLE * ((hours % 12) * 60 + minutes) / (12 * 60);

  secondHand.y = (int16_t)(cos_lookup(second_angle) * (int32_t)secondHandLength / TRIG_MAX_RATIO) + center.y;
  secondHand.x = (int16_t)(-sin_lookup(second_angle) * (int32_t)secondHandLength / TRIG_MAX_RATIO) + center.x;

  drawClock(secondHand, hours, ctx);

  if(AllSet[SET_COMM] && !btConnected)
  {
     if(hours % 12 < 3)
         graphics_draw_bitmap_in_rect(ctx, bt_disconnected, GRect(144-32,0,32,32));
       else if(hours % 12 < 6)
         graphics_draw_bitmap_in_rect(ctx, bt_disconnected, GRect(144-32,160-32,32,32));
       else if(hours % 12 < 9)
         graphics_draw_bitmap_in_rect(ctx, bt_disconnected, GRect(0,160-32,32,32));
       else 
         graphics_draw_bitmap_in_rect(ctx, bt_disconnected, GRect(0,0,32,32));
  }
  
  drawHand(secondHand, second_angle, ctx);

  if(AllSet[SET_HPDATE]){
    drawDate(secondHand, second_angle, t->tm_mday, ctx);
  }
}

static void window_load(Window *window) {
  GRect bounds = layer_get_bounds(rootLayer);
  
  layer = layer_create(bounds);
  layer_set_update_proc(layer, layer_update_proc);
  layer_add_child(rootLayer, layer);

  inv_layer = inverter_layer_create (bounds);
  layer_set_hidden(inverter_layer_get_layer(inv_layer), AllSet[SET_HPINV]);
  layer_add_child(rootLayer, inverter_layer_get_layer(inv_layer));

}

static void window_appear(Window *window){
  isAnimating = true;
  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  int targetMinutes = 0;
  increaseAnimation = 6;
  if(clock_is_24h_style()){
    targetMinutes = t->tm_hour * 60 + t->tm_min;
    if(targetMinutes < 6*60){
      anim_minutes = 0;
    }
    else if(targetMinutes < 12*60){
      anim_minutes = 12 * 60;
      increaseAnimation = -increaseAnimation;
    }
    else if(targetMinutes < 18*60){
      anim_minutes = 12 * 60;
    }
    else {
      anim_minutes = 24 * 60;
      increaseAnimation = -increaseAnimation;
    }
  }
  else {
    anim_minutes = 0;
    targetMinutes = (t->tm_hour % 12) * 60 + t->tm_min;
    if(targetMinutes > 6*60)
      increaseAnimation = -increaseAnimation;

  }

  timer = app_timer_register(500 /* milliseconds */, animation_timer_callback, NULL);
}

static void window_unload(Window *window) {
  layer_destroy(layer);
  inverter_layer_destroy(inv_layer);
  
}

/*
static void in_received_handler(DictionaryIterator *iter, void *context) {
  // call autoconf_in_received_handler
  autoconfig_in_received_handler(iter, context);

  for(int i=0; i<2; i++){
    gpath_destroy(hour_arrow[i]);
  }
  if(getHand() == HAND_LINE){
    for(int i=0; i<2; i++){
      hour_arrow[i] = gpath_create(&LINE_HAND_POINTS[i]);
    }
  }
  else {
    for(int i=0; i<2; i++){
      hour_arrow[i] = gpath_create(&ARROW_HAND_POINTS[i]);
    }
  }

  layer_set_hidden(inv_layer_get_layer(inv), !getInverted());
  layer_mark_dirty(layer);
}
*/
static void bluetooth_connection_handler(bool connected){
  btConnected = connected;
  if(AllSet[SET_BUZZ_DIS]>0){
    vibes_cancel();
    if(connected){
      vibes_long_pulse();
    }
    else {
      vibes_double_pulse();
    }
  }
  layer_mark_dirty(layer);
}

void redraw_pick() {
}

void load_pick(void) {
  //autoconfig_init();
  //APP_LOG(APP_LOG_LEVEL_INFO, "window %p",(void*) window);

  //ToDo: Add fonts
  custom_font = fonts_get_system_font(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS);
    //fonts_load_custom_font(resource_get_handle(RESOURCE_ID_COMFORTAA_40));
  small_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
    //fonts_load_custom_font(resource_get_handle(RESOURCE_ID_COMFORTAA_18));

  bt_disconnected = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_DISCONNECTED);

  if(AllSet[SET_HPHAND] == 1) //HAND_LINE){ 
    set_hour_arrow (LINE_HAND);
  else
    set_hour_arrow (ARROW_HAND);    
    /*for(int i=0; i<2; i++){
      hour_arrow[i] = gpath_create(&LINE_HAND_POINTS[i]);
        //static_gpath_create(0,i);
    }
  }
  else {
    for(int i=0; i<2; i++){
      hour_arrow[i] = gpath_create(&ARROW_HAND_POINTS[i]);
        //static_gpath_create(1,i);
    }
  }*/

    window_load(NULL);
    window_appear(NULL);

  //app_message_register_inbox_received(in_received_handler);

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);

  btConnected = bluetooth_connection_service_peek();
  bluetooth_connection_service_subscribe(bluetooth_connection_handler);


}

void unload_pick(void) {
  //autoconfig_deinit();
  if (timer!=NULL)
  {
    app_timer_cancel(timer);
    timer=NULL;
  }
  //fonts_unload_custom_font(custom_font);
  //fonts_unload_custom_font(small_font);
  gbitmap_destroy(bt_disconnected);
  for(int i=0; i<2; i++){
    gpath_destroy(hour_arrow[i]);
  }
  window_unload(NULL);
  bluetooth_connection_service_unsubscribe();
}

void info_pick(void* disp_info)
{
	char Defaults[6][3]={{SET_HPBW,0,1}, {SET_COMM,0,0}, {SET_HPDATE,0,0},  //ToDo: Fix Default
                          {SET_HPINV,0,1}, {SET_BUZZ_DIS,0,0}, {SET_HPHAND,0,1}};
	display_info * info = (display_info *) disp_info;
	snprintf(info->app_name, sizeof(info->app_name),"Hop Picker");
	snprintf(info->dev_name, sizeof(info->dev_name),"Gregoire Sage");
	memcpy(info->def_set, &Defaults, 6*3 );
	info->def_cnt= 6; //six setting items
	info->def_opt= 6; //all six have alternative value
}
