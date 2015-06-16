#define __TELEPORT_C__

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "interpreter.h"
#include "handler.h"
#include "comm.h"
#include "teleport.h"
#include "buffer.h"

extern struct index_data *obj_index;
extern struct char_data *character_list;
extern struct room_data *world;
extern int top_of_world;

void die_in_dt(struct char_data * ch);

void TeleportPulseStuff()
{
  ACMD(do_look);
  int real_room(int virtual);

  bool teleportP = FALSE;
  struct char_data *next = NULL, *tmp = NULL, *bk = NULL;
  struct room_data *rm = NULL;
  int troom = 0, i = 0;
  struct obj_data *obj_object, *temp_obj;

  for (i = 0; i < top_of_world; i++) {
      rm = &world[i];
      if (!rm->number || rm->tele == NULL) 
	continue;
      if (rm->number == rm->tele->targ)
	continue;
      if (rm->tele->targ == 0)
	continue;
      if (rm->tele->cnt > 0) {		/* decrement counter */
	rm->tele->cnt -= 1;
      } 
      if (rm->tele->cnt == 0) {		/* reset counter if needed */
	rm->tele->cnt = rm->tele->time;
      } else if (rm->tele->cnt > 1) {   /* jump out if not 1 */
	continue;
      }

      if (real_room(rm->tele->targ) == -1) {
          log("invalid tele->targ");
          continue;
      }

      troom = real_room(rm->tele->targ);

      if (!IS_SET(rm->tele->mask, TELE_OBJ) &&
	  !IS_SET(rm->tele->mask, TELE_NOOBJ) &&
	  !IS_SET(rm->tele->mask, TELE_SKIPOBJ)) {
        obj_object = rm->contents;
        while (obj_object) {
          temp_obj = obj_object->next_content;
          obj_from_room(obj_object);
          obj_to_room(obj_object, troom);
          obj_object = temp_obj;
        }
      }
      temp_obj = NULL;
      bk = 0;

      while (rm->people) { 
	/* work through the list of people */
        /* at first, bk = 0, so this checks for null */
	  tmp = rm->people;

	  if (!tmp || tmp == bk) break;

          bk = tmp;
          if (IS_SET(rm->tele->mask, TELE_NOMOB) && IS_MOB(tmp)) {
		teleportP = FALSE;
		/* don't teleport mobs */
	  } else { /* go ahead and check to teleport */
           if (IS_SET(rm->tele->mask, TELE_OBJ)) {
   		teleportP = FALSE;
		temp_obj = tmp->carrying;
  	        while (!teleportP && temp_obj) {
			if (GET_OBJ_VNUM(temp_obj) == rm->tele->obj)
				teleportP= TRUE;
			temp_obj = temp_obj->next;
		}
	   } else if (IS_SET(rm->tele->mask, TELE_NOOBJ)) {
   	        teleportP = TRUE;
	        temp_obj = tmp->carrying;
	        while (teleportP && temp_obj) {
		if (GET_OBJ_VNUM(temp_obj) == rm->tele->obj)
			teleportP = FALSE;
		temp_obj = temp_obj->next;
   	        }
	   } else {
		teleportP = TRUE;
	   }
	  }
          if (teleportP) {
  	    if (!IS_SET(rm->tele->mask, TELE_NOMSG)) {
		act("$n disappears.", FALSE, tmp, 0, 0, TO_ROOM);
		act("You have a very strange feeling.", FALSE, tmp, 0, 0, TO_CHAR);
  	    }
            char_from_room(tmp); /* the list of people in the room has changed */
            char_to_room(tmp, troom);
  	    if (!IS_SET(rm->tele->mask, TELE_NOMSG)) {
		act("$n appears from out of nowhere.", FALSE, tmp, 0, 0, TO_ROOM);
	    }
            if (IS_SET(rm->tele->mask, TELE_LOOK) && !IS_NPC(tmp)) {
              do_look(tmp, "\0",15, 0);
  	       /*look_at_room(ch, 0);*/
            }

            if (IS_SET(world[troom].room_flags, ROOM_DEATH) && 
		(GET_LEVEL(tmp))  < LVL_IMMORT) {
              if (tmp == next)
                next = tmp->next;
              log_death_trap(tmp);
	      /*death_cry(tmp);*/
	      die_in_dt(tmp);
              continue;
            }
             /* if (dest->sector_type == SECT_AIR) {
              * n2 = tmp->next;
              * if (check_falling(tmp)) {
              *   if (tmp == next)
              *     next = n2;
              * }
              * } hmm.... maybe I should add this as welll*/
	  }
	  /* tmp = tmp->next; */
      }
      if (IS_SET(rm->tele->mask, TELE_RANDOM)) {
            rm->tele->time = number(2,30);
      }
   }
}
