// Overlord is the Overlay loader for pebble watch
// Copyright (C) 2014  Ron Ravid and Gregoire Sage
// See README.md for license details

#pragma once
// #define OVERLAY_OFF //Disable overlay compilation for CloudPebble testing(also disable few displays to reduce toward < 24K)

#ifndef OVERLAY_OFF
#include "overlays.h"
#endif

int overlay_init();
int overlay_load(int app_num);

static const int16_t OVL_RESOURCE_IDS[] = {
#ifndef OVERLAY_OFF
		FUZZY_TIME_OVL,//RESOURCE_ID_OVL_FUZZY, //better to have one without overlay
		HOP_PICKER_OVL, //note: the problematic data of GPath moved to non_overlay file
		ACCUINFO_OVL, 
		RETROCLOCK_OVL,
		PERSPECTIVE_OVL,
		JUST_A_BIT_OVL,
		ILLUDERE_OVL,
		TIC_TOCK_TOE_OVL,
		BLOCKSLIDE_OVL,
		SIMPLICITY_OVL,
		PEBBLE_ONE_OVL,
#else
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
#endif
};
static const int8_t OVL_debug[] = { //debug information for active overlay part
		-1,0,1,2,3,4,5,6,7,3,3,3
	};
