/* ************************************************************************
*   File:winddragon.c                                  Part of CircleMUD  *
*  Usage: implementation of rooms to dragons                              *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "dg_scripts.h"
#include "winddragon.h"
#include "buffer.h"

extern struct descriptor_data *descriptor_list;

const sh_int to_fly_rooms [NUM_ROOMS + 1] =               
   /* air room #1     air room #2     air room #3       final room */
   { 11004,              11005,          11006,              11002};
   


void dragon_upd()
{
  int k, conectado;
  struct char_data *mob;
  struct descriptor_data *d, *next_d;
  
   for(k=0;k <= NUM_DRAGONS;k++)
    if (vet_dragons[k] == 1){
     conectado=0;
     for (d = descriptor_list; d; d = next_d) {
      	next_d = d->next;
      	if ((buf_dragon_indo[k].player->desc) &&
      	   (d->descriptor == buf_dragon_indo[k].player->desc->descriptor))
      	 conectado = 1;
      }	  
      if (conectado){       
       act("The dragon fly in the clouds carring you.", FALSE, buf_dragon_indo[k].player, 0, 0, TO_CHAR);
       act("The dragon fly in the clouds carring $n.", TRUE, buf_dragon_indo[k].player, 0, 0, TO_NOTVICT);
       char_from_room(buf_dragon_indo[k].player);
       buf_dragon_indo[k].room++;
       char_to_room(buf_dragon_indo[k].player, real_room(to_fly_rooms[buf_dragon_indo[k].room]));
       look_at_room(buf_dragon_indo[k].player, 0);
       if (buf_dragon_indo[k].room == 3){
        act("The dragon down and leave you safe.", TRUE, buf_dragon_indo[k].player, 0, 0, TO_CHAR);
        act("A dragon comes from the sky and leave $n safe.", TRUE, buf_dragon_indo[k].player, 0, 0, TO_NOTVICT);	
        send_to_room("The dragon fly to the sky and vanish in the clouds.\n\r", real_room(to_fly_rooms[NUM_ROOMS]));        
        vet_dragons[k] = 0;
        mob = read_mobile(real_mobile(DRAGON_VNUM), REAL);
        char_to_room(mob, real_room(DRAGONROOM_INI));
        send_to_room("A Dragon comes from sky.\n\r", real_room(DRAGONROOM_INI));
       }
       else{
        act("$n comes from clouds mounting a winged dragon.", TRUE, buf_dragon_indo[k].player, 0, 0, TO_NOTVICT);	
       }           
     }//desc
     else{
     /* conectado=0;
      for (d = descriptor_list; d; d = next_d) {
      	next_d = d->next;
      	if ((buf_dragon_indo[k].player->desc) &&
      	   (d->descriptor == buf_dragon_indo[k].player->desc->descriptor))
      	 conectado = 1;
      }
      */	  
      if (!buf_dragon_indo[k].player->desc){
       char_from_room(buf_dragon_indo[k].player);
       char_to_room(buf_dragon_indo[k].player, real_room(to_fly_rooms[NUM_ROOMS]));
       act("A dragon comes from the sky and leave $n safe.", TRUE, buf_dragon_indo[k].player, 0, 0, TO_NOTVICT);	
       send_to_room("The dragon fly to the sky and vanish in the clouds.\n\r", real_room(to_fly_rooms[NUM_ROOMS]));        
      }
      vet_dragons[k] = 0;      
      mob = read_mobile(real_mobile(DRAGON_VNUM), REAL);
      char_to_room(mob, real_room(DRAGONROOM_INI));
      send_to_room("A Dragon comes from sky.\n\r", real_room(DRAGONROOM_INI));
     }          
   }//==1   
}
