/****************************************************************************/
/**
* Pebble watch face that displays "all the time".
*
* @file   accuinfo.c
*
* @author Bob Hauck <bobh@haucks.org>
*
* This code may be used or modified in any way whatsoever without permission
* from the author.
*
*****************************************************************************/
#include <ctype.h>
#include "pebble.h"
#include "pposdemo.h"
#include "ron64lib.h"


//#define FG_COLOR foreColor
//#define BG_COLOR backColor

/* Possible messages received from the config page
*/
//#define CONFIG_KEY_BACKGROUND   (0)


//Window *window;
TextLayer *day_layer;
TextLayer *date_layer;
TextLayer *time_layer;
TextLayer *secs_layer;
TextLayer *ampm_layer;
TextLayer *year_layer;
//InverterLayer *inverter_layer;
Layer *line_layer;
GFont *font_date;
GFont *font_time;


static char *upcase(char *str)
{
    char *s = str;

    while (*s)
    {
        *s++ = toupper((int)*s);
    }

    return str;
}


static void line_layer_update_callback(Layer *l, GContext *ctx)
{
    (void)l;

    graphics_context_set_stroke_color(ctx, foreColor);
    graphics_draw_line(ctx, GPoint(0, 38), GPoint(144, 38));
    graphics_draw_line(ctx, GPoint(0, 39), GPoint(144, 39));
    graphics_draw_line(ctx, GPoint(0, 108), GPoint(144, 108));
    graphics_draw_line(ctx, GPoint(0, 109), GPoint(144, 109));
}


static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed)
{
    // Need to be static because they're used by the system later.
    static char year_text[] = "0000";
    static char date_text[] = "Xxxxxxxxx 00";
    static char day_text[] = "Xxxxxxxxx";
    static char time_text[] = "00:00";
    static char secs_text[] = "00";
    static char ampm_text[] = "XX";

    //if (units_changed & DAY_UNIT)
    {
        strftime(day_text, sizeof(day_text), "%A", tick_time);
        text_layer_set_text(day_layer, upcase(day_text));

        strftime(date_text, sizeof(date_text), "%B %e", tick_time);
        text_layer_set_text(date_layer, date_text);

        strftime(year_text, sizeof(year_text), "%Y", tick_time);
        text_layer_set_text(year_layer, year_text);
    }

    if (clock_is_24h_style())
    {
        strcpy(ampm_text, "  ");
        strftime(time_text, sizeof(time_text), "%R", tick_time);
    }
    else
    {
        strftime(ampm_text, sizeof(ampm_text), "%p", tick_time);
        strftime(time_text, sizeof(time_text), "%I:%M", tick_time);

        // Kludge to handle lack of non-padded hour format string
        // for twelve hour clock.
        if (!clock_is_24h_style() && (time_text[0] == '0'))
        {
            memmove(time_text, &time_text[1], sizeof(time_text) - 1);
        }
    }

    strftime(secs_text, sizeof(secs_text), "%S", tick_time);
    text_layer_set_text(time_layer, time_text);
    text_layer_set_text(secs_layer, secs_text);
    text_layer_set_text(ampm_layer, ampm_text);
}

/*
void update_configuration(void)
{
    bool inv = 0;    // default to not inverted 

    if (persist_exists(CONFIG_KEY_BACKGROUND))
    {
        inv = persist_read_bool(CONFIG_KEY_BACKGROUND);
    }

    layer_set_hidden(inverter_layer_get_layer(inverter_layer), !inv);
}


void in_received_handler(DictionaryIterator *received, void *context)
{
    Tuple *background_tuple = dict_find(received, CONFIG_KEY_BACKGROUND);

    if (background_tuple)
    {
        app_log(APP_LOG_LEVEL_DEBUG,
                __FILE__,
                __LINE__,
                "bg=%s",
                background_tuple->value->cstring);

        if (strcmp(background_tuple->value->cstring, "black") == 0)
        {
            persist_write_bool(CONFIG_KEY_BACKGROUND, false);
        }
        else
        {
            persist_write_bool(CONFIG_KEY_BACKGROUND, true);
        }
    }

    update_configuration();
}


void in_dropped_handler(AppMessageResult reason, void *ctx)
{
    app_log(APP_LOG_LEVEL_WARNING,
            __FILE__,
            __LINE__,
            "Message dropped, reason code %d",
            reason);
}
*/
void redraw_accu() {
    time_t t = time(NULL);
    struct tm *tick_time = localtime(&t);
    handle_second_tick(tick_time, SECOND_UNIT);
}
void load_accu(void)
{
    ResHandle res_d;
    ResHandle res_t;
    //Layer *window_layer;
    //time_t t = time(NULL);
    //struct tm *tick_time = localtime(&t);
    TimeUnits units_changed = SECOND_UNIT | MINUTE_UNIT | HOUR_UNIT | DAY_UNIT;
    int move = clock_is_24h_style() ? 15 : 0;

    //app_message_register_inbox_received(in_received_handler);
    //app_message_register_inbox_dropped(in_dropped_handler);
    //app_message_open(64, 64);

    //window = window_create();
    //window_layer = rootLayer;//window_get_root_layer(window);
    //window_set_background_color(window, backColor);

    res_d = resource_get_handle(RESOURCE_ID_FONT_ROBOTO_CONDENSED_20);
    res_t = resource_get_handle(RESOURCE_ID_FONT_ROBOTO_BOLD_SUBSET_41);
    font_date = fonts_load_custom_font(res_d);
    font_time = fonts_load_custom_font(res_t);

    day_layer = text_layer_create(GRect(0, 2, 144, 36));
	//APP_LOG(APP_LOG_LEVEL_INFO, "day %p",(void*) day_layer);

	//set_layer_attr(day_layer, GTextAlignmentCenter, FONT_KEY_GOTHIC_18, window_layer);
    text_layer_set_font(day_layer, font_date);
    text_layer_set_text_color(day_layer, foreColor);
    text_layer_set_background_color(day_layer, GColorClear);
    text_layer_set_text_alignment(day_layer, GTextAlignmentCenter);
    layer_add_child(rootLayer, text_layer_get_layer(day_layer));

    time_layer = text_layer_create(GRect(2, 48, 112, 60));
	//set_layer_attr(time_layer, GTextAlignmentRight, FONT_KEY_GOTHIC_28, window_layer);
    text_layer_set_font(time_layer, font_time);
    text_layer_set_text_color(time_layer, foreColor);
    text_layer_set_background_color(time_layer, GColorClear);
    text_layer_set_text_alignment(time_layer, GTextAlignmentRight);
    layer_add_child(rootLayer, text_layer_get_layer(time_layer));

    secs_layer = text_layer_create(GRect(116, 46+move, 144-116, 28));
	//set_layer_attr(secs_layer, GTextAlignmentCenter, FONT_KEY_GOTHIC_18, window_layer);
    text_layer_set_font(secs_layer, font_date);
    text_layer_set_text_color(secs_layer, foreColor);
    text_layer_set_background_color(secs_layer, GColorClear);
    text_layer_set_text_alignment(secs_layer, GTextAlignmentCenter);
    layer_add_child(rootLayer, text_layer_get_layer(secs_layer));

    ampm_layer = text_layer_create(GRect(116, 74, 144-116, 28));
	//set_layer_attr(ampm_layer, GTextAlignmentCenter, FONT_KEY_GOTHIC_18, window_layer);
    text_layer_set_font(ampm_layer, font_date);
    text_layer_set_text_color(ampm_layer, foreColor);
    text_layer_set_background_color(ampm_layer, GColorClear);
    text_layer_set_text_alignment(ampm_layer, GTextAlignmentCenter);
    layer_add_child(rootLayer, text_layer_get_layer(ampm_layer));

    date_layer = text_layer_create(GRect(1, 117, 144-1, 28));
	//set_layer_attr(date_layer, GTextAlignmentCenter, FONT_KEY_GOTHIC_18, window_layer);
    text_layer_set_font(date_layer, font_date);
    text_layer_set_text_color(date_layer, foreColor);
    text_layer_set_background_color(date_layer, GColorClear);
    text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
    layer_add_child(rootLayer, text_layer_get_layer(date_layer));

    year_layer = text_layer_create(GRect(0, 141, 144, 28));
	//set_layer_attr(year_layer, GTextAlignmentCenter, FONT_KEY_GOTHIC_18, window_layer);
    text_layer_set_font(year_layer, font_date);
    text_layer_set_text_color(year_layer, foreColor);
    text_layer_set_background_color(year_layer, GColorClear);
    text_layer_set_text_alignment(year_layer, GTextAlignmentCenter);
    layer_add_child(rootLayer, text_layer_get_layer(year_layer));

    line_layer = layer_create(GRect(0, 0, 144, 168));
    layer_set_update_proc(line_layer, line_layer_update_callback);
    layer_add_child(rootLayer, line_layer);

    //inverter_layer = inverter_layer_create(GRect(0, 0, 144, 168));
    //layer_add_child(window_layer, inverter_layer_get_layer(inverter_layer));
    //update_configuration();

    //handle_second_tick(tick_time, units_changed);
	//redraw_accu();
	configRedraw();
    //window_stack_push(window, true /* Animated */);
    tick_timer_service_subscribe(units_changed, handle_second_tick);
}


void unload_accu(void)
{
    tick_timer_service_unsubscribe();
    app_message_deregister_callbacks();

    text_layer_destroy(day_layer);
    text_layer_destroy(time_layer);
    text_layer_destroy(secs_layer);
    text_layer_destroy(ampm_layer);
    text_layer_destroy(date_layer);
    text_layer_destroy(year_layer);
    layer_destroy(line_layer);
    //inverter_layer_destroy(inverter_layer);

    //window_destroy(window);

    fonts_unload_custom_font(font_date);
    fonts_unload_custom_font(font_time);
}

void info_accu(void* disp_info)
{
	display_info * info = (display_info *) disp_info;
	snprintf(info->app_name, sizeof(info->app_name),"accuinfo");
	snprintf(info->dev_name, sizeof(info->dev_name),"Bob Hauck");
	info->def_cnt= 0; //no setting items
}
