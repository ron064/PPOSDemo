#pragma once

#include <pebble.h>

enum {LINE_HAND,ARROW_HAND};
extern GPath *hour_arrow[2];
void set_hour_arrow (int type);