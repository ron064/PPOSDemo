#include "pebble.h"
#include "common.h"
#include "launcher.h"
#include "pposdemo.h"
#include "config.h"
#include "ron64lib.h"
#include "overlord.h"


uint8_t APP_COUNT =1; //temporal value

int whichApp = 0;

static bool inverted = false;
static AppTimer *savetimer = NULL;
static int8_t save_face=0;
static AppTimer *nametimer = NULL;
static AppTimer *NextFace =NULL;
//static AppTimer * FirstTap;
int8_t AllSet[SETSIZE]; //settings for all. use 0 as default value

int resourceOffset = 0; //Starting point for resources, updated before calling compiled display
int setOffset = 32; //Starting point within AllSet array, updated before calling compiled display

InverterLayer *inverter_layer;

Layer *rootLayer;
Layer *mainLayer;
Layer *serviceLayer;
static TextLayer *name_layer;
//																			static TextLayer *dev_layer;
static Layer *BGLayer;
	
//Layer *rootLayer;
GColor backColor = GColorBlack;						
GColor foreColor = GColorWhite;

app_t *app;

enum {START_NAME, END_NAME};
display_info disp_info;
char app_name[40] = "unknown error";

uint16_t Delay[]={1500,2000,3000,5000,7000,10000,15000,20000,30000,60000};

void handle_next_face_timeout(void* data) {
	//NextFace=NULL;
	int8_t deley_index=AllSet[SET_TIME];
	if (deley_index==0)
		deley_index=4; //if no value is set, fix to 7sec
	NextFace= app_timer_register(Delay[deley_index], handle_next_face_timeout, NULL);
	config_swap(true);
}

void update_mode()
{
	// Curently there is only one mode: Demo
	uint8_t mode= AllSet[SET_MODE]; 
	//if ((mode>=5)&&(mode<=7)) //start demo mode
	if (mode!= 1)
	NextFace= app_timer_register(Delay[AllSet[SET_TIME]], handle_next_face_timeout, NULL);
}	
int current_opr; //ToDo: remove this global later
void display_name(int operation);

void handle_name_display(void* data) {
	nametimer= NULL;
	int * opr = (int *) data;
	if (opr[0]==START_NAME)
	{
		//current_app.info(&disp_info);
		if (strlen(disp_info.dev_name)>0)
			snprintf(app_name, sizeof(app_name),"%s\nby\n%s",disp_info.app_name,disp_info.dev_name);
		else
			snprintf(app_name, sizeof(app_name),"\n%s",disp_info.app_name);

		text_layer_set_text(name_layer, app_name);
		layer_set_hidden(BGLayer, false);
		display_name(END_NAME);
	}	
	else
	if (opr[0]==END_NAME)
	{
	  layer_set_hidden(BGLayer, true);
	}	
	
	//APP_LOG(APP_LOG_LEVEL_INFO, "Data recived:%d",opr[0]);
	
}
	
void display_name(int operation)
{
	//APP_LOG(APP_LOG_LEVEL_INFO, "starting/ending name %d",operation);
	current_opr= operation;
	if (operation == START_NAME)
	{
		if (nametimer!=NULL)
			app_timer_cancel(nametimer);
		nametimer = app_timer_register(500, handle_name_display, &current_opr);
	}
	else
	if (operation == END_NAME)
	{
		if (nametimer!=NULL)
			app_timer_cancel(nametimer);
		nametimer = app_timer_register(1500, handle_name_display, &current_opr);
	}
	// ToDo: set random X location of display window
}

	
void configRedraw() {
	window_redraw_with_background(backColor);
}

void do_invert() {
	inverted = !inverted;
	backColor = inverted ? GColorWhite : GColorBlack;
	foreColor = inverted ? GColorBlack : GColorWhite;
	configRedraw();
}

void handle_save_face(void* data) {
	savetimer=NULL;
	if ((AllSet[SET_MODE]<5)||(AllSet[SET_MODE]>7)) //Prevent saving in demo mode
		persist_write_int(KEY_WHICH_APP, save_face);
}
void save_last(int8_t face) // save last face with delay to reduce persist writes and avoid saveing temporal display
{
	save_face=face; 
/*  disabled in this appliation;
	if (savetimer==NULL)
		savetimer= app_timer_register(2000, handle_save_face, NULL);
	else
		app_timer_reschedule(savetimer, 2000);*/
}
void swap_face(int8_t face)
{
	APP_LOG(APP_LOG_LEVEL_INFO, "face: %d",face);
	if (face==-1) 
		face =whichApp;
	
	if ((face>=0)&&(face<APP_COUNT))
	{
		window_unload(NULL);
		whichApp=face;
		int ovl_rslt= overlay_load(whichApp);	
		if (ovl_rslt<0)
		{
			APP_LOG(APP_LOG_LEVEL_ERROR,"fail to load app:%d, error:%d", whichApp, ovl_rslt);
			whichApp=0; // load some watch-face that is not overlay
		}
		APP_LOG(APP_LOG_LEVEL_INFO, "app:%d",whichApp);
		
		current_app = app[whichApp];
		resourceOffset = resource_offset[whichApp];
		current_app.info(&disp_info);
		if ((disp_info.def_cnt>0) && (AllSet[SET_USEOPT]==0))
			for (int8_t i=0; i< disp_info.def_cnt; i++)
			{
				AllSet[disp_info.def_set[i][0]]=disp_info.def_set[i][1];
			}
		window_load(NULL);
		//if (whichApp!=1) //don't save hop_picker for now
			save_last(whichApp);
		//if (Delay[ AllSet[SET_TIME] ] >=3000)
		display_name(START_NAME); //Shows App Name box
		
	}
	//APP_LOG(APP_LOG_LEVEL_INFO, "loaded");
	
}


void config_swap(bool up) {
	if (up) {
		if (++whichApp >= APP_COUNT) {
			whichApp = 0;
		}
	} else if (--whichApp < 0) {
		whichApp += APP_COUNT;
	}
	swap_face(whichApp);
}


void BG_layer_update_callback(Layer *layer, GContext* ctx) {
    graphics_context_set_fill_color(ctx, foreColor);
    graphics_fill_rect(ctx, layer_get_bounds(layer), 4, GCornersAll);
    graphics_context_set_fill_color(ctx, backColor);
    graphics_fill_rect(ctx, GRect(4,5,122,58) , 4, GCornersAll);
	
}

int load_settings()
{
	return 	persist_read_data(KEY_ALL_SET, &AllSet, sizeof(AllSet));
}
int save_settings()
{
	return persist_write_data(KEY_ALL_SET, &AllSet, sizeof(AllSet)); // safer to write on exit.
}

void config_deinit(Window *window) {
	// Entire app is quitting. Deallocate everything that config allocated.
	if (savetimer!=NULL) //clear save timer (will not save if quitting shortly after changing)
	{
		app_timer_cancel(savetimer);
		savetimer=NULL;
	}
	if (NextFace !=NULL)
	{
		app_timer_cancel(NextFace);
		NextFace=NULL;
	}
	if (nametimer !=NULL)
	{
		app_timer_cancel(nametimer);
		nametimer=NULL;
	}

	text_layer_destroy(name_layer);
	layer_destroy(BGLayer);
	layer_destroy(serviceLayer);
	
	//custom_deinit(window);
	if (rootLayer!=NULL)
	{
		layer_destroy(rootLayer);
		rootLayer=NULL;
	}
}

void config_init(Window *window) {
	APP_COUNT = sizeof(appConfig) / sizeof(app_t); //one less variable to update
	load_settings();
	
	mainLayer = window_get_root_layer(window);
	rootLayer = layer_create(layer_get_frame(mainLayer) );
	layer_add_child(mainLayer, rootLayer);
	//window_set_click_config_provider(window, click_config_provider);

	serviceLayer = layer_create(GRect(0, 0, 144, 168)); //layer on top of root layer
    layer_add_child(mainLayer, serviceLayer);
	
    BGLayer = layer_create(GRect(7, 2, 130, 68));
	layer_set_update_proc(BGLayer, BG_layer_update_callback);
	layer_add_child(serviceLayer, BGLayer);
	
	name_layer = text_layer_create(GRect(5, 2, 120, 64));
	layer_add_child(serviceLayer, text_layer_get_layer(name_layer));
	text_layer_set_text(name_layer, "Pebble PoS Demo\n by\n Ron64 & Pedrolane");
	
	//inverter_layer = inverter_layer_create(GRect(0, 0, 144, 168));
    //layer_add_child(serviceLayer, inverter_layer_get_layer(inverter_layer));
	//layer_set_hidden(inverter_layer_get_layer(inverter_layer), inverted);
	
	inverted = AllSet[SET_INV];
	backColor = inverted ? GColorWhite : GColorBlack;
	foreColor = inverted ? GColorBlack : GColorWhite;
	
	//whichApp = persist_read_int(KEY_WHICH_APP);
	whichApp =AllSet[SET_FACE];
	if (whichApp < 0 || whichApp >= APP_COUNT) {
		whichApp = 0;
	}

	overlay_init(); // run this in init to ensure correct operation of overlay. 

	int ovl_rslt= overlay_load(whichApp); 
	if (ovl_rslt<0)
	{
		APP_LOG(APP_LOG_LEVEL_ERROR, "fail to load app:%d, error:%d",whichApp,ovl_rslt);
		whichApp=0; // load some watch-face that is not overlay
	}

	//custom_init();
	current_app = app[whichApp];
	
	set_layer_attr(name_layer, GTextAlignmentCenter, FONT_KEY_GOTHIC_18, BGLayer);

	update_mode(); //curently start the display cycle
	
}
