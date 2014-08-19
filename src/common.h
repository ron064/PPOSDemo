#pragma once

typedef struct {
void (*load)();
void (*unload)();
void (*redraw)();
void (*info)(void* disp_info);
} app_t;
