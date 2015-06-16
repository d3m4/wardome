#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "buffer.h"
#include "interpreter.h"
#include "constants.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "buffer.h"
#include "handler.h"



extern struct char_data *character_list;
extern struct room_data *world;


int i;
char path;
struct char_data *ch; 
	
	
	
	if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PATH))//flagada com path
		{
		i = world[ch->in_room].zone
		
		if (i = 120)
		char rome[]= { "2sd10e2s10w" };
		rome = path;
		
sprintf(buf, "You found the path to %s: %s\r\n", zone_table[i].name, path); 
	send_to_char(buf, ch);
	
}