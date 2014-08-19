#pragma once
// Each added screen must include pposdemo.h
// This file is part of Overley_demo / Point_of_sale demo By Ron Ravid & Grégoire Sage

enum {SET_INV, SET_DATE, SET_LANG, SET_BAT, SET_COMM, SET_BUZZ_DIS, SET_BUZZ_CON, SET_USEOPT, //General settings
      SET_BIT_RL, SET_BLOCK_LINE, SET_HPDATE, SET_HPBW, SET_HPINV, SET_HPHAND, SET_RES2, SET_RES3, //Specific settings
      SET_MODE, SET_FACE, SET_FXP, SET_FXN, SET_FYP, SET_FYN, SET_FZP, SET_FZN, // selecting Display
      SET_TIME, SET_BLK_DAY, SET_BLK_RND, SET_FULL_DIG, SET_028, SET_029, SET_030, SET_031,  // More faces.
	  SET_032, SET_033, SET_034, SET_035, SET_036, SET_037, SET_038, SET_039,
	  SET_040, SET_041, SET_043, SET_044, SET_045, SET_046, SET_047};
#define SETSIZE 64
extern int8_t AllSet[SETSIZE]; //settings for all. use 0 as default value
extern GColor backColor;
extern GColor foreColor;
extern Layer *rootLayer;
extern int resourceOffset; //Starting point for resources, updated before calling compiled display
extern int setOffset; //Starting point within AllSet array, updated before calling compiled display
void configRedraw();

typedef struct  {
  char app_name[17];		//
  char dev_name[17];
  int8_t  def_cnt; // number of settings;
  int8_t  def_opt; // count optional settings;
  int8_t def_set[8][3]; //up to 8 setting parameters, each contain [n][0]key position, [n][1]setting value and [n][2]optional value
  int8_t  bt_pos;  // prefered location for BT comm icon 0- disabled 1=left, 2=Right.
  int8_t  bat_pos; // prefered location for battery icon 0- disabled 1=left, 2=Right.
  int8_t  reserved[19];
// 80 bytes
} __attribute__((__packed__)) display_info;
