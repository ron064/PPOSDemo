#include <pebble.h>
#include "pposdemo.h"

//#define DATEFORMAT_KEY 0

//#define COLOR_FOREGROUND foreColor
//#define COLOR_BACKGROUND backColor

static Layer* background;
static TextLayer* hour_text;
static TextLayer* minute_text;
static TextLayer* day_text;
static TextLayer* date_text;
static TextLayer* month_text;
static TextLayer* year_text;
static TextLayer* ampm_text;

typedef struct {
	int dx;
	int mx;
	int yx;
} date_t;

static date_t dateFormats[3] = {
	{ .mx = 2, .dx = 38, .yx = 74},
	{ .dx = 2, .mx = 38, .yx = 74},
	{ .yx = 2, .mx = 74, .dx = 110},
};

// utility function to strip a leading space or zero from a string.
static char* strip(char* input)
{
	if (strlen(input) > 1 && (input[0] == ' ' || input[0] == '0'))
	{
		return input + 1;
	}
	return input;
}

// callback function for rendering the background layer
static void background_update_callback(Layer *me, GContext* ctx)
{
	graphics_context_set_fill_color(ctx, foreColor);
	graphics_fill_rect(ctx, GRect(2,8,68,68), 4, GCornersAll);
	graphics_fill_rect(ctx, GRect(74,8,68,68), 4, GCornersAll);
	graphics_fill_rect(ctx, GRect(2,92,140,32), 4, GCornersAll);

		graphics_fill_rect(ctx, GRect(dateFormats[AllSet[SET_DATE]].yx,128,68,32), 4, GCornersAll);
		graphics_fill_rect(ctx, GRect(dateFormats[AllSet[SET_DATE]].mx,128,32,32), 4, GCornersAll);
		graphics_fill_rect(ctx, GRect(dateFormats[AllSet[SET_DATE]].dx,128,32,32), 4, GCornersAll);

	graphics_context_set_stroke_color(ctx, backColor);
	graphics_draw_line(ctx, GPoint(2,41), GPoint(142,41));
	graphics_draw_line(ctx, GPoint(2,108), GPoint(142,108));
	graphics_draw_line(ctx, GPoint(2,144), GPoint(142,144));
}

// set update the time and date text layers
static void display_time(struct tm *tick_time)
{
	time_t now;
	if (tick_time == NULL) {
		now = time(NULL);
		tick_time = localtime(&now);
	}

	static char hour[]   = "12";
	static char minute[] = "21";
	static char day[]    = "            ";
	static char date[]   = "  ";
	static char month[]  = "  ";
	static char year[]   = "    ";
	static char ampm[]   = "  ";

	if (clock_is_24h_style())
	{
		strftime(hour, sizeof(hour), "%H", tick_time);
	}
	else
	{
		strftime(hour, sizeof(hour), "%l", tick_time);
	}

	strftime(minute, sizeof(minute), "%M", tick_time);
	strftime(day, sizeof(day), "%A", tick_time);
	strftime(date, sizeof(date), "%e", tick_time);
	strftime(month, sizeof(month), "%m", tick_time);
	strftime(year, sizeof(year), "%Y", tick_time);

	if (!clock_is_24h_style())
	{
		strftime(ampm, sizeof(ampm), "%p", tick_time);
	}

	text_layer_set_text(hour_text, strip(hour));
	text_layer_set_text(minute_text, minute);
	text_layer_set_text(day_text, day);
	text_layer_set_text(date_text, strip(date));
	text_layer_set_text(month_text, strip(month));
	text_layer_set_text(year_text, year);
	text_layer_set_text(ampm_text, ampm);
}

// callback function for minute tick events that update the time and date display
static void handle_tick(struct tm *tick_time, TimeUnits units_changed)
{
	display_time(tick_time);
}

static void update_date_format() {
	GRect rect = layer_get_frame(text_layer_get_layer(date_text));
        rect.origin.x = dateFormats[AllSet[SET_DATE]].dx;
        layer_set_frame(text_layer_get_layer(date_text), rect);
	
	rect = layer_get_frame(text_layer_get_layer(month_text));
        rect.origin.x = dateFormats[AllSet[SET_DATE]].mx;
        layer_set_frame(text_layer_get_layer(month_text), rect);

	rect = layer_get_frame(text_layer_get_layer(year_text));
        rect.origin.x = dateFormats[AllSet[SET_DATE]].yx;
        layer_set_frame(text_layer_get_layer(year_text), rect);
}

void redraw_retro() {
	text_layer_set_text_color(hour_text, backColor);
	text_layer_set_text_color(minute_text, backColor);
	text_layer_set_text_color(day_text, backColor);
	text_layer_set_text_color(ampm_text, backColor);
	text_layer_set_text_color(date_text, backColor);
	text_layer_set_text_color(month_text, backColor);
	text_layer_set_text_color(year_text, backColor);
	update_date_format();
	display_time(NULL);
}

// utility function for initializing a text layer
static void init_text(TextLayer* textlayer, GFont font)
{
	text_layer_set_text_alignment(textlayer, GTextAlignmentCenter);
	text_layer_set_text_color(textlayer, backColor);
	text_layer_set_background_color(textlayer, GColorClear);
	text_layer_set_font(textlayer, font);
}

static GFont rb46;
static GFont rb22;
static GFont rc10;

// callback function for the app initialization
void load_retro()
{
	background = layer_create(layer_get_frame(rootLayer));
	layer_set_update_proc(background, &background_update_callback);
	layer_add_child(rootLayer, background);

	rb46 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_SUBSET_46));
	rb22 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_22));
	rc10 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_SUBSET_10));
	
	hour_text = text_layer_create(GRect(2, 8 + 4, 68, 68 - 4));
	init_text(hour_text, rb46);

	minute_text = text_layer_create(GRect(74, 8 + 4, 68, 68 - 4));
	init_text(minute_text, rb46);

	day_text = text_layer_create(GRect(2, 92 + 2, 140, 32 - 2));
	init_text(day_text, rb22);

	ampm_text = text_layer_create(GRect(4, 48, 16, 12));
	init_text(ampm_text, rc10);

		date_text = text_layer_create(GRect(dateFormats[AllSet[SET_DATE]].dx, 128 + 2, 32, 32 - 2));
		init_text(date_text, rb22);

		month_text = text_layer_create(GRect(dateFormats[AllSet[SET_DATE]].mx, 128 + 2, 32, 32 - 2));
		init_text(month_text, rb22);

		year_text = text_layer_create(GRect(dateFormats[AllSet[SET_DATE]].yx, 128 + 2, 68, 32 - 2));
		init_text(year_text, rb22);

	layer_add_child(rootLayer, text_layer_get_layer(hour_text));
	layer_add_child(rootLayer, text_layer_get_layer(minute_text));
	layer_add_child(rootLayer, text_layer_get_layer(day_text));
	layer_add_child(rootLayer, text_layer_get_layer(date_text));
	layer_add_child(rootLayer, text_layer_get_layer(month_text));
	layer_add_child(rootLayer, text_layer_get_layer(year_text));
	layer_add_child(rootLayer, text_layer_get_layer(ampm_text));

	configRedraw();
	display_time(NULL);

	tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
}

void unload_retro()
{
	tick_timer_service_unsubscribe();

	text_layer_destroy(hour_text);
	text_layer_destroy(minute_text);
	text_layer_destroy(day_text);
	text_layer_destroy(ampm_text);
	text_layer_destroy(date_text);
	text_layer_destroy(month_text);
	text_layer_destroy(year_text);

	// TODO: unload custom fonts

	layer_destroy(background);
	fonts_unload_custom_font(rb46);
	fonts_unload_custom_font(rb22);
	fonts_unload_custom_font(rc10);
}

void info_retro(void* disp_info)
{
	char Defaults[1][3]={{SET_DATE,0,1}};
	display_info * info = (display_info *) disp_info;
	snprintf(info->app_name, sizeof(info->app_name),"Retro Clock");
	snprintf(info->dev_name, sizeof(info->dev_name),"lingen.me");
	memcpy(info->def_set, &Defaults, 1*3 );
	info->def_cnt= 1; //one setting items
	info->def_opt= 1; //one alternative value
}