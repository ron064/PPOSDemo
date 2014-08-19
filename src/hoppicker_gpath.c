#include <pebble.h>
#include "hoppicker_gpath.h"
	
GPath *hour_arrow[2];


const GPathInfo LINE_HAND_POINTS[2] = {
	{4,(GPoint []) {{-3, 0},{-3, -300},{3, -300},{3, 0}}},
	{4,(GPoint []) {{-3, 0},{3, 0},{3, -300},{-3, -300}}}
};
const GPathInfo ARROW_HAND_POINTS[2] = {
	{4,(GPoint []) {{-6, 0},{-2, -175},{2, -175},{6, 0}}},
	{4,(GPoint []) {{-6, 0},{6, 0},{2, -175},{-2, -175}}}
};

void set_hour_arrow (int type)
{
	for(int i=0; i<2; i++)
		if (type==LINE_HAND)
			hour_arrow[i] = gpath_create(&LINE_HAND_POINTS[i]);
		else
			hour_arrow[i] = gpath_create(&ARROW_HAND_POINTS[i]);
}
