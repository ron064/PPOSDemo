#include "pebble.h"
#include "pposdemo.h"
#include "num2words.h"

static struct CommonWordsData {
  TextLayer *label;
  char buffer[BUFFER_SIZE];
} s_data;

void redraw_fuzzy() {
  text_layer_set_background_color(s_data.label, backColor);
  text_layer_set_text_color(s_data.label, foreColor);
}

static void update_time(struct tm* t) {
  fuzzy_time_to_words(t->tm_hour, t->tm_min, s_data.buffer, BUFFER_SIZE);
  text_layer_set_text(s_data.label, s_data.buffer);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  update_time(tick_time);
}

void load_fuzzy() {
  GFont font = fonts_get_system_font(FONT_KEY_DROID_SERIF_28_BOLD);

  GRect frame = layer_get_frame(rootLayer);

  s_data.label = text_layer_create(GRect(0, 40, frame.size.w, frame.size.h - 40));
  configRedraw();
  text_layer_set_font(s_data.label, font);
  layer_add_child(rootLayer, text_layer_get_layer(s_data.label));

  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  update_time(t);

  tick_timer_service_subscribe(MINUTE_UNIT, &handle_minute_tick);
}

void unload_fuzzy() {
  text_layer_destroy(s_data.label);
}

void info_fuzzy(void* disp_info)
{
	display_info * info = (display_info *) disp_info;
	snprintf(info->app_name, sizeof(info->app_name),"Fuzzy Time");
	snprintf(info->dev_name, sizeof(info->dev_name),"Pebble");	
	info->def_cnt= 0; //no setting items
}
