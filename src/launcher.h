// Define the interface between launcher and config

extern void (*load)();
extern void (*unload)();
extern void (*redraw)();
extern app_t current_app;

void config_init(Window *window);
void config_deinit(Window *window);
void window_load(Window *window);
void window_unload(Window *window);
void window_redraw_with_background(GColor backColor);


