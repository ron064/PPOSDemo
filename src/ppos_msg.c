#include "pebble.h"
#include "ppos_msg.h"
#include "pposdemo.h"
#include "config.h"

uint8_t get_offset(uint8_t key)
{
	uint8_t offset=0;
	if ((key>=48)&&(key<=58))
		offset=48;
	if ((key>='a')&&(key<='z'))
		offset='a'-10;
 	//APP_LOG(APP_LOG_LEVEL_INFO,"Key:%c offset:%d",key, offset);
	return offset;
}
void in_received_handler(DictionaryIterator *received, void *context)
{
	//APP_LOG(APP_LOG_LEVEL_INFO,"recived msg. info:%d",(int)received->cursor->key);
	int8_t CellCount=0;
	Tuple *tuple = dict_read_first(received);
	while (tuple) {
  		//APP_LOG(APP_LOG_LEVEL_INFO,"Key:%d Data:%d",(int)tuple->key, tuple->value->uint8 );
/*		if ((tuple->key>=1024)&&(tuple->key<=1071))
		{
			AllSet[tuple->key & 0x3f]= tuple->value->uint8 - get_offset(tuple->value->uint8);
  		    //APP_LOG(APP_LOG_LEVEL_INFO,"Set cell:%d to:%d",(int)(tuple->key &0x3f), (int)(tuple->value->uint8-offset) );
			CellCount++;
		}*/
		if (tuple->key==2048)
		{
			int len=strlen(tuple->value->cstring);
			if (len >SETSIZE) len=SETSIZE;
			for (int i=0;i<len;i++)
			{
				AllSet[i]= tuple->value->cstring[i] -get_offset(tuple->value->uint8);
			}
			CellCount+= len;
		}
		tuple = dict_read_next(received);
	}
	int saverslt= save_settings();
	APP_LOG(APP_LOG_LEVEL_INFO,"Updated %d config values. Saved Config result:%d",CellCount, saverslt );
	swap_face(-1); //refresh current display
}

void in_dropped_handler(AppMessageResult reason, void *ctx)
{
    APP_LOG(APP_LOG_LEVEL_WARNING, "Message dropped, reason code %d", reason);
}


void ppos_msg_init()
{
	//loadconfig(); not needed, called from launcher
	app_message_register_inbox_received(in_received_handler);
 	app_message_register_inbox_dropped(in_dropped_handler);
	app_message_open(200,100);	
	
}
void ppos_msg_deinit()
{
	app_message_deregister_callbacks();
}
