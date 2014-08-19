#pragma once

#include "pposdemo.h"
//#include "overlord.h" 
#include "common.h"

extern void do_invert();
extern void swap_face(int8_t face);
void config_swap(bool up);
int load_settings();
int save_settings();

#define ENTRYPOINTS(NAME) void redraw_ ## NAME (); void load_ ## NAME (); \
                          void unload_ ## NAME (); void info_ ## NAME (void* disp_info);

	ENTRYPOINTS(fuzzy)
	ENTRYPOINTS(pick)
	ENTRYPOINTS(accu)
	ENTRYPOINTS(retro)
	ENTRYPOINTS(Perspective)
	ENTRYPOINTS(bit)
	ENTRYPOINTS(ill)
	ENTRYPOINTS(toe)
	ENTRYPOINTS(block)
	ENTRYPOINTS(simplicity)
	ENTRYPOINTS(one)
// former config7.h
extern app_t *app;
extern int whichApp;

enum {KEY_WHICH_APP, KEY_INVERTED, KEY_ALL_SET}; 

//#define APP_COUNT 10


#define ENTRYPOINTROW(NAME) { .load = load_ ## NAME,     .unload = unload_ ## NAME, \
                              .redraw = redraw_ ## NAME, .info = info_ ## NAME }

//static app_t appConfig[APP_COUNT] = {
static app_t appConfig[] = {
	ENTRYPOINTROW(fuzzy),
	ENTRYPOINTROW(pick),
	ENTRYPOINTROW(accu),
	ENTRYPOINTROW(retro),
	ENTRYPOINTROW(Perspective),
	ENTRYPOINTROW(bit),
	ENTRYPOINTROW(ill),
	ENTRYPOINTROW(toe),
	ENTRYPOINTROW(block),
	ENTRYPOINTROW(simplicity),
	ENTRYPOINTROW(one),
};

static const int8_t resource_offset[] = { //resource_offset for windows from library
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		RESOURCE_ID_PONE_IMAGE_MENU_ICON  - 1,
	};
