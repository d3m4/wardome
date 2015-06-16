/* ************************************************************************
*   File: condition.c                                   Part of CircleMUD *
*  Usage: Communication, socket handling, main(), central game loop       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "buffer.h"
#include "utils.h"
#include "handler.h"
#include "comm.h"
#include "db.h"

extern struct room_data *world;

int apply_ac(struct char_data * ch, int eq_pos);

char *item_condition(struct obj_data *obj)
{
  int value;
  static char buf[MAX_STRING_LENGTH];

  /* default conditon is 100, and reflects average quality. This can be
     raised, through _special_ and _expensive_ smiths, tailors, etc to a max
     of 125, reflecting exceptional quality and manufacture. Code only
     cares about 1-12, so lets chop the name down a bit...
   */

  value = GET_OBJ_COND(obj);

  if (value == 101)
    sprintf(buf, " &W[indestructible]&n ");
  else if (value >= 90)
    sprintf(buf, " &B[new]&n ");
  else if (value >= 80)
    sprintf(buf, " &b[almost new]&n ");
  else if (value >= 70)
    sprintf(buf, " &G[very good]&n ");
  else if (value >= 60)
    sprintf(buf, " &g[good]&n ");
  else if (value >= 50)
    sprintf(buf, " &C[used]&n ");
  else if (value >= 40)
    sprintf(buf, " &c[fair]&n ");
  else if (value >= 30)
    sprintf(buf, " &M[well worn]&n ");
  else if (value >= 20)
    sprintf(buf, " &m[worn]&n ");
  else if (value >= 10)
    sprintf(buf, " &r[partially worn]&n ");
  else if (value >= 0)
    sprintf(buf, " &R[ruined]&n ");
  else
    sprintf(buf, " ");
    
  return buf;
}

int DamageItem(struct char_data *ch, struct obj_data *o)
{
  int temp,i;

  if (!o)
    return 0;

  if (ROOM_FLAGGED(ch->in_room, ROOM_ARENA | ROOM_PEACEFUL))
    return 0;
      	
  temp = number(1, 2);
  if(GET_OBJ_COND(o) == 101){
	return FALSE;
  } else{
     for (i = 0; i < NUM_WEARS; i++) {
      if (GET_EQ(ch, i))
       GET_AC(ch) += apply_ac(ch, i);
     } 	
     
     GET_OBJ_COND(o) -= temp;
     
     for (i = 0; i < NUM_WEARS; i++) {
      if (GET_EQ(ch, i))
       GET_AC(ch) -= apply_ac(ch, i);
     }
    }  
  save_char(ch, NOWHERE);
  
  sprintf(buf, "&G%s&g was &Gdamaged&g in the combat!&n\r\n", (CAN_SEE_OBJ(ch, o) ? CAP(o->short_description) : "Something"));
  send_to_char(buf, ch);
  
  if (GET_OBJ_COND(o) < 0) {
    GET_OBJ_COND(o) = 0;
    return TRUE;
  }
  return FALSE;
}

int DamageOneItem(struct char_data *ch, struct obj_data *obj)
{

  if (!obj)
    return 0;

  if (DamageItem(ch, obj)) {
    return TRUE;
  }

  return FALSE;
}

void MakeScrap(struct char_data *ch, struct obj_data *obj)
{
  char buf[256];
  struct obj_data *t, *x;
  int pos;


  if (!ch || !obj || (ch->in_room == NOWHERE))
    return;

  act("$p falls to the ground in scraps.", TRUE, ch, obj, 0, TO_CHAR);
  act("$p falls to the ground in scraps.", TRUE, ch, obj, 0, TO_ROOM);

  t = read_object(9, VIRTUAL);
  if (!t)
    return;

  sprintf(buf, "Scraps from %s&n lie in a pile",
	  obj->short_description);
  t->description = str_dup(buf);
  t->short_description = str_dup("a pile of scraps");

  if (obj->carried_by) {
    obj_from_char(obj);
  } else if (obj->worn_by) {
    for (pos = 0; pos < NUM_WEARS; pos++)
      if (ch->equipment[pos] == obj)
	break;
    if (pos >= NUM_WEARS) {
      log("SYSERR: MakeScrap(), can't find worn object in equip");
      exit(1);
    }
    obj = unequip_char(ch, pos);
  }
  obj_to_room(t, ch->in_room);
  while (obj->contains) {
    x = obj->contains;
    obj_from_obj(x);
    obj_to_room(x, ch->in_room);
  }
  extract_obj(obj);
}

void DamageAllStuff(struct char_data *ch, int dam_type, int dam)
{
  int j;
  struct obj_data *obj, *next;

  /* this procedure takes all of the items in equipment and inventory
     and damages the ones that should be damaged */

  /* equipment */

  for (j = 0; j < NUM_WEARS; j++) {
    if (ch->equipment[j]) {
      obj = ch->equipment[j];
      if (DamageOneItem(ch, obj)) {	/* TRUE == destroyed */
	if ((obj = unequip_char(ch, j)) != NULL) {
	  MakeScrap(ch, obj);
	} else {
	  log("SYSERR: DamageAllStuff(): hmm, really wierd!");
	}
      }
    }
  }

  /* inventory */

  for (obj = ch->carrying; obj; obj = next) {
    next = obj->next_content;
    if (DamageOneItem(ch, obj))
      MakeScrap(ch, obj);
  }
}
/*
void DamageStuff(struct char_data *ch)
{
  int j;
  struct obj_data *obj;

  for (j = 0; j < NUM_WEARS; j++) {
    if (ch->equipment[j]) {
      obj = ch->equipment[j];
      if ((number(0, 90) > number(70, 200)) && DamageOneItem(ch, obj)) {
	if ((obj = unequip_char(ch, j)) != NULL) {
	  MakeScrap(ch, obj);
	  save_char(ch, ch->in_room);
	} else {
	  log("SYSERR: DamageStuff(): hmm, really wierd!");
	}
      }
    }
  }
}
*/

void DamageStuff(struct char_data *ch)
{
  int j;
  struct obj_data *obj;

   j = number(0, (NUM_WEARS - 1));
    if (GET_EQ(ch, j)) {
      obj = GET_EQ(ch, j)/*ch->equipment[j]*/;
      if ((number(0, 60) > number(30, 90)) && DamageOneItem(ch, obj)) {
	if ((obj = unequip_char(ch, j)) != NULL) {
	  MakeScrap(ch, obj);
	  save_char(ch, ch->in_room);
	} else {
	  log("SYSERR: DamageStuff(): hmm, really wierd!");
	}
      }
    }
}
