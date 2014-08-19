// Overlord is the Overlay loader for pebble watch
// By Ron Ravid and Gregoire Sage
// Dual Licence: GPL is valid for free application with open source, contact us for other cases.

#include "pebble.h"
#include "overlord.h"

#include "pebble_process_info.h"
extern const PebbleProcessInfo __pbl_app_info;


uint8_t * OVL_P= NULL;  //pointer to start of overlay area

int overlay_init()
{
	APP_LOG(APP_LOG_LEVEL_INFO, "App: %s Version %d.%d size:%d", __pbl_app_info.name, __pbl_app_info.process_version.major, __pbl_app_info.process_version.minor, __pbl_app_info.load_size);
	//ToDo: 
	//1. (Optionel), get address from linker data
	OVL_P = (void *) & __pbl_app_info;
	OVL_P+=0x84;
	return 0; // don't think there will ever be error here
}


#ifndef OVERLAY_OFF
extern unsigned long _ovly_table[][2];
#endif


// Number of overlays present 
#define NUM_OVERLAYS 10

// Record for current overlay 
int current_overlay = 0;

// struct to hold addresses and lengths
typedef struct overlay_region_t_struct
{
	void* load_ro_base;
	void* load_rw_base;
	void* exec_zi_base;
	unsigned int ro_length;
	unsigned int zi_length;
} overlay_region_t;
//ToDo: use this to allow Debug information.

#ifndef OVERLAY_OFF
// Array describing the overlays 
extern const overlay_region_t overlay_regions[NUM_OVERLAYS];
#endif

int overlay_load(int app_num)
{
 	if (OVL_P== NULL)
		return -3; //overlay_init was not called
	
	int size=-2;
	//const overlay_region_t * selected_region;
	// ToDo: change check to OVL_debug[app_num]>NUM_OVERLAYS
        // (after finishing to add overlays)
        /*if (app_num>NUM_OVERLAYS)
	{
		APP_LOG(APP_LOG_LEVEL_ERROR, "app_error:%d",app_num);
		return -1;
	}*/
#ifndef OVERLAY_OFF
	if (OVL_RESOURCE_IDS[app_num]>-1){
		size= resource_load_byte_range(resource_get_handle(RESOURCE_ID_OVL_FULL), _ovly_table[OVL_RESOURCE_IDS[app_num]][0], OVL_P, _ovly_table[OVL_RESOURCE_IDS[app_num]][1]);
	}else
		size= 1;
	
	if ((size>1)&&(OVL_debug[app_num]>=0))
	{
		current_overlay=OVL_debug[app_num]; //just in case we can get the debug data to work.
		//APP_LOG(APP_LOG_LEVEL_INFO, "current %d ro:%d zi:%d",current_overlay,
		//	overlay_regions[current_overlay].ro_length, overlay_regions[current_overlay].zi_length);
	}		
#else
	return app_num; //incase of overlay disabled all pass
#endif
	APP_LOG(APP_LOG_LEVEL_INFO, "app:%d size:%d",app_num, size);
	if (size<0)
		return size;

	
	// this function will try to load the app into the reserved area.
	// on sucsess it will return app number
	// on fail it will return negative error code
	// This function should also save the active app for the debugger
	
	// ToDo: handle DebugData

	// set selected region 
	//selected_region = &overlay_regions[app_num-1];	
	
	return app_num;
}
