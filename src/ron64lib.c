#include "pebble.h"
#include "ron64lib.h"
#include "pposdemo.h"
	
#define iter1(N) try = root + (1 << (N)); if (n >= try << (N)) { n -= try << (N); root |= 2 << (N); }
uint32_t Wilco_sqrt (uint32_t n)
{
	uint32_t root = 0, try;
	iter1 (15); iter1 (14); iter1 (13); iter1 (12); iter1 (11);
	iter1 (10); iter1 ( 9); iter1 ( 8); iter1 ( 7); iter1 ( 6);
	iter1 ( 5); iter1 ( 4); iter1 ( 3); iter1 ( 2); iter1 ( 1); iter1 ( 0);
	return root >> 1;
}

void set_layer_attr(TextLayer *textlayer, GTextAlignment Alignment,GFont font, Layer *ParentLayer)
{
  text_layer_set_text_alignment(textlayer, Alignment);
  text_layer_set_text_color(textlayer, foreColor);
  text_layer_set_background_color(textlayer, GColorClear);
  text_layer_set_font(textlayer, fonts_get_system_font(font));
  if (ParentLayer != NULL)
	layer_add_child(ParentLayer, text_layer_get_layer(textlayer));
}

