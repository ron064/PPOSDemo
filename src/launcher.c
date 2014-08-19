#include "pebble.h"
#include "common.h"
#include "launcher.h"
#include "pposdemo.h"
#include "ppos_msg.h"
	
static Window *window;

app_t current_app;

// This initializes the layers upon window load
void window_load(Window *window) {
	current_app.load();	
}

// Deinitialize resources on window unload that were initialized on window load
void window_unload(Window *window) {
	current_app.unload();
	layer_remove_child_layers(rootLayer);
	if (window != NULL) {
		config_deinit(window);
	if (window != NULL)
		window_destroy(window);
	}
}

void window_redraw_with_background(GColor backColor) {
	window_set_background_color(window, backColor);
    current_app.redraw();
}

static void handle_init() {
	window = window_create();

  	window_set_fullscreen(window, true);
	config_init(window);

	// Setup the window handlers
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
  	});

	window_stack_push(window, true /* Animated */);
	ppos_msg_init();
}

static void handle_deinit() {
	ppos_msg_deinit();
}

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}
