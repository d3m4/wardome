/* ************************************************************************
*   File: spells.c                                      Part of CircleMUD *
*  Usage: Implementation of "manual spells".  Circle 2.2 spell compat.    *
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
#include "spells.h"
#include "handler.h"
#include "db.h"
#include "constants.h"
#include "screen.h"
#include "buffer.h"
#include "dg_scripts.h"
#include "interpreter.h"

extern sh_int r_mortal_start_room;
extern int top_of_world;
extern char *spells[];
extern struct room_data *world;
extern struct obj_data *object_list;
extern struct char_data *character_list;
extern struct index_data *obj_index;
extern struct descriptor_data *descriptor_list;
extern struct zone_data *zone_table;
void play_sound(struct char_data *ch, char *sound, int type);

extern int mini_mud;
extern int pk_allowed;

extern struct default_mobile_stats *mob_defaults;
extern struct apply_mod_defaults *apmd;

void clearMemory(struct char_data * ch);
void weight_change_object(struct obj_data * obj, int weight);
void add_follower(struct char_data * ch, struct char_data * leader);
int mag_savingthrow(struct char_data * ch, int type);
void name_to_drinkcon(struct obj_data * obj, int type);
void name_from_drinkcon(struct obj_data * obj);
char *howstat(int percent);
void list_obj_to_char(struct obj_data * list, struct char_data * ch, int mode, int show);
void list_char_to_char(struct char_data * list, struct char_data * ch);

ACMD(do_flee);
/*
 * Special spells appear below.
 */

ASPELL(spell_create_water)
{
  int water;

  if (ch == NULL || obj == NULL)
    return;

  if (GET_OBJ_TYPE(obj) == ITEM_DRINKCON) {
    if ((GET_OBJ_VAL(obj, 2) != LIQ_WATER) && (GET_OBJ_VAL(obj, 1) != 0)) {
//      name_from_drinkcon(obj);
      GET_OBJ_VAL(obj, 2) = LIQ_SLIME;
      name_to_drinkcon(obj, LIQ_SLIME);
    } else {
      water = MAX(GET_OBJ_VAL(obj, 0) - GET_OBJ_VAL(obj, 1), 0);
      if (water > 0) {
/*        if (GET_OBJ_VAL(obj, 1) >= 0)
          name_from_drinkcon(obj);*/
        GET_OBJ_VAL(obj, 2) = LIQ_WATER;
        GET_OBJ_VAL(obj, 1) += water;
//        name_to_drinkcon(obj, LIQ_WATER);
        weight_change_object(obj, water);
        act("$p is filled.", FALSE, ch, obj, 0, TO_CHAR);
      }
    }
  }
}

int cant_teleport(int to_room)
{
        if(!ZONE_FLAGGED(world[to_room].zone, ZONE_OPEN) &&
           !ZONE_FLAGGED(world[to_room].zone, ZONE_CLOSED))
           return TRUE;

        if(ZONE_FLAGGED(world[to_room].zone, ZONE_NORECALL) ||
           ZONE_FLAGGED(world[to_room].zone, ZONE_NOSUMMON) ||
           ZONE_FLAGGED(world[to_room].zone, ZONE_CLOSED))
           return TRUE;

        if(ROOM_FLAGGED(to_room, ROOM_PRIVATE | ROOM_DEATH | ROOM_NOMAGIC))
           return TRUE;

   return FALSE;
}


ASPELL(spell_astral_projection)
{
  struct affected_type *aff;
 
  int to_room = 536 ;

  if (ch == NULL || victim == NULL)
    return;

  to_room = victim->in_room ;  
 
 if(cant_teleport(to_room)) {
  send_to_char("You cannot go there. \r\n", ch) ;
  return ;
  }

 if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_ARENA)) {
  send_to_char("You cannot leave the arena!\r\n", ch) ;
  return ;
 }

  if (AFF2_FLAGGED(victim, AFF2_ROOTED)) {  // ips
	for (aff = victim->affected; aff; aff = aff->next) {
		if (aff->bitvector == AFF2_ROOTED) {
			act("The roots around $n's feet disappear.", TRUE, victim, 0, 0, TO_ROOM);
			affect_remove(victim, aff);
		}
	}
  }
 
  act("$n slowly fades out of existence and is gone.",
      FALSE, victim, 0, 0, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, to_room);
  act("$n slowly fades into existence.", FALSE, victim, 0, 0, TO_ROOM);
  look_at_room(ch, 0);

}

ASPELL(spell_recall)
{
  struct affected_type *aff;

  if (victim == NULL || IS_NPC(victim))
    return;

 if (ZONE_FLAGGED(world[victim->in_room].zone, ZONE_NORECALL)) {
   act("A divine force mantains $n in the same place.", TRUE, victim, 0, 0, TO_ROOM);
   act("A divine force mantains you in the same place.", FALSE, ch, 0, 0, TO_CHAR);
 }  
 else { 

  if (AFF2_FLAGGED(victim, AFF2_ROOTED)) {  // ips
	for (aff = victim->affected; aff; aff = aff->next) {
		if (aff->bitvector == AFF2_ROOTED) {
			act("The roots around $n's feet disappear.", TRUE, victim, 0, 0, TO_ROOM);
			affect_remove(victim, aff);
		}
	}
  }

  act("$n disappears.", TRUE, victim, 0, 0, TO_ROOM);
  char_from_room(victim);
  char_to_room(victim, r_mortal_start_room);
  act("$n appears in the middle of the room.", TRUE, victim, 0, 0, TO_ROOM);
  look_at_room(victim, 0);
 } 
}


ASPELL(spell_teleport)
{
  int to_room;
  struct affected_type *aff;

  if (victim == NULL || IS_NPC(victim))
    return;

 if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_ARENA))  {
   send_to_char("You cannot leave the arena!\r\n", ch) ;
   return ;
  }
   
  do {
    to_room = number(2, top_of_world);
  } while (cant_teleport(to_room));


  if (AFF2_FLAGGED(victim, AFF2_ROOTED)) { // ips
	for (aff = victim->affected; aff; aff = aff->next) {
		if (aff->bitvector == AFF2_ROOTED) {
			act("The roots around $n's feet disappear.", TRUE, victim, 0, 0, TO_ROOM);
			affect_remove(victim, aff);
		}
	}
  }
         
  act("$n slowly fades out of existence and is gone.",
      FALSE, victim, 0, 0, TO_ROOM);
  char_from_room(victim);
  char_to_room(victim, to_room);
  act("$n slowly fades into existence.", FALSE, victim, 0, 0, TO_ROOM);
  look_at_room(victim, 0);
}

#define SUMMON_FAIL "You failed.\r\n"

ASPELL(spell_summon)
{
  struct affected_type *aff;

 if (ch == NULL || victim == NULL)
    return;

  if ((GET_LEVEL(victim) > MIN(LVL_IMMORT - 1, level + 3)) || (PLR_FLAGGED(victim, PLR_DEAD))) {
    send_to_char(SUMMON_FAIL, ch);
    return;
  }

  if (MOB_FLAGGED(victim, MOB_AGGRESSIVE)) {
    act("As the words escape your lips and $N travels\r\n"
        "through time and space towards you, you realize that $E is\r\n"
        "aggressive and might harm you, so you wisely send $M back.",
        FALSE, ch, 0, victim, TO_CHAR);
    return;
  }
  
   if ((ZONE_FLAGGED(world[ch->in_room].zone, ZONE_NOSUMMON) ||
        ZONE_FLAGGED(world[victim->in_room].zone, ZONE_NOSUMMON)) &&
       (world[ch->in_room].zone != world[victim->in_room].zone)) {
    sprintf(buf, "%s just tried to summon you to: %s.\r\n"
            "%s failed because you are in another dimension.\r\n",
            GET_NAME(ch), world[ch->in_room].name,
            (ch->player.sex == SEX_MALE) ? "He" : "She");
    send_to_char(buf, victim);

    sprintf(buf, "You failed because %s is in another dimension.\r\n",
            GET_NAME(victim));
    send_to_char(buf, ch);

    sprintf(buf, "%s failed summoning %s to %s.",
            GET_NAME(ch), GET_NAME(victim), world[ch->in_room].name);
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
    return;
  }
  
  if (!IS_NPC(victim) && !PRF_FLAGGED(victim, PRF_SUMMONABLE) &&
      !PLR_FLAGGED(victim, PLR_KILLER) && !PLR_FLAGGED(victim, PLR_THIEF)) {
    sprintf(buf, "%s just tried to summon you to: %s.\r\n"
            "%s failed because you have summon protection on.\r\n"
            "Type NOSUMMON to allow other players to summon you.\r\n",
            GET_NAME(ch), world[ch->in_room].name,
            (ch->player.sex == SEX_MALE) ? "He" : "She");
    send_to_char(buf, victim);

    sprintf(buf, "You failed because %s has summon protection on.\r\n",
            GET_NAME(victim));
    send_to_char(buf, ch);

    sprintf(buf, "%s failed summoning %s to %s.",
            GET_NAME(ch), GET_NAME(victim), world[ch->in_room].name);
    mudlog(buf, BRF, LVL_IMMORT, TRUE);
    return;
  }

  if (MOB_FLAGGED(victim, MOB_NOSUMMON) ||
      (IS_NPC(victim) && mag_savingthrow(victim, SAVING_SPELL))) {
    send_to_char(SUMMON_FAIL, ch);
    return;
  }

  if (AFF2_FLAGGED(victim, AFF2_ROOTED)) {
	for (aff = victim->affected; aff; aff = aff->next) {
		if (aff->bitvector == AFF2_ROOTED) {
			act("The roots around $n's feet disappear.", TRUE, victim, 0, 0, TO_ROOM);
			affect_remove(victim, aff);
		}
	}
  }

  act("$n disappears suddenly.", TRUE, victim, 0, 0, TO_ROOM);

  char_from_room(victim);
  char_to_room(victim, ch->in_room);

  act("$n arrives suddenly.", TRUE, victim, 0, 0, TO_ROOM);
  act("$n has summoned you!", FALSE, ch, 0, victim, TO_VICT);
  look_at_room(victim, 0);
}



ASPELL(spell_locate_object)
{
  struct obj_data *i;
  char name[MAX_INPUT_LENGTH];
  int j;

  /*
   * FIXME: This is broken.  The spell parser routines took the argument
   * the player gave to the spell and located an object with that keyword.
   * Since we're passed the object and not the keyword we can only guess
   * at what the player originally meant to search for. -gg
   */
  strcpy(name, fname(obj->name));
  j = level / 2;

  for (i = object_list; i && (j > 0); i = i->next) {
    if (!isname(name, i->name))
      continue;

    if (i->carried_by)
      sprintf(buf, "%s is being carried by %s.\r\n",
              i->short_description, PERS(i->carried_by, ch));
    else if (i->in_room != NOWHERE)
      sprintf(buf, "%s is in %s.\r\n", i->short_description,
              world[i->in_room].name);
    else if (i->in_obj)
      sprintf(buf, "%s is in %s.\r\n", i->short_description,
              i->in_obj->short_description);
    else if (i->worn_by)
      sprintf(buf, "%s is being worn by %s.\r\n",
              i->short_description, PERS(i->worn_by, ch));
    else
      sprintf(buf, "%s's location is uncertain.\r\n",
              i->short_description);

    CAP(buf);
    send_to_char(buf, ch);
    j--;
  }

  if (j == level / 2)
    send_to_char("You sense nothing.\r\n", ch);
}



ASPELL(spell_charm)
{
  struct affected_type af;

  if (victim == NULL || ch == NULL)
    return;
/*  if (!FIGHTING(ch))
send_to_char("You can't charm it now. Fight him before!\r\n", ch);*/
  if (victim == ch)
    send_to_char("You like yourself even better!\r\n", ch);
  else if (!IS_NPC(victim))
    send_to_char("You fail because SUMMON protection is on!\r\n", ch);
  else if (AFF_FLAGGED(victim, AFF_SANCTUARY))
    send_to_char("Your victim is protected by sanctuary!\r\n", ch);
  else if (MOB_FLAGGED(victim, MOB_NOCHARM))
    send_to_char("Your victim resists!\r\n", ch);
  else if (AFF_FLAGGED(ch, AFF_CHARM))
    send_to_char("You can't have any followers of your own!\r\n", ch);
  else if (AFF_FLAGGED(victim, AFF_CHARM) || level < GET_LEVEL(victim) || GET_REMORT(ch) < GET_REMORT(victim))
    send_to_char("You fail.\r\n", ch);
  /* player charming another player - no legal reason for this */
  else if (!pk_allowed && !IS_NPC(victim))
    send_to_char("You fail - shouldn't be doing it anyway.\r\n", ch);
  else if (circle_follow(victim, ch))
    send_to_char("Sorry, following in circles can not be allowed.\r\n", ch);
  else if (!allow_follower(ch, 1))
	send_to_char("You can't have any more followers.\n\r", ch);
  else if (mag_savingthrow(victim, SAVING_PARA))
    send_to_char("Your victim resists!\r\n", ch);
  else if (mag_savingthrow(victim, SAVING_SPELL))
    send_to_char("Your victim resists!\r\n", ch);

  else {
    if (victim->master)
      stop_follower(victim);

    add_follower(victim, ch);

    af.type = SPELL_CHARM;

    if (GET_INT(victim))
      af.duration = 1 + ( 20 * 10 / GET_INT(victim));
    else
      af.duration = 24;

    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_CHARM;
    af.bitvector2 = AFF_CHARM;
    af.bitvector3 = AFF_CHARM;
    affect_to_char(victim, &af);

    act("Isn't $n just such a nice fellow?", FALSE, ch, 0, victim, TO_VICT);
    if (IS_NPC(victim)) {
      REMOVE_BIT(MOB_FLAGS(victim), MOB_AGGRESSIVE);
      REMOVE_BIT(MOB_FLAGS(victim), MOB_SPEC);
    }
  }
}


ASPELL(spell_rigid_thinking)
{
  struct affected_type af;

  if (victim == NULL || ch == NULL)
    return;

  if (victim == ch)
    send_to_char("You like yourself even better!\r\n", ch);
  else if (!IS_NPC(victim))
    send_to_char("You fail because SUMMON protection is on!\r\n", ch);
  else if (AFF_FLAGGED(victim, AFF_SANCTUARY))
    send_to_char("Your victim is protected by sanctuary!\r\n", ch);
  else if (MOB_FLAGGED(victim, MOB_NOCHARM))
    send_to_char("Your victim resists!\r\n", ch);
  else if (AFF_FLAGGED(ch, AFF_CHARM))
    send_to_char("You can't have any followers of your own!\r\n", ch);
  else if (AFF_FLAGGED(victim, AFF_CHARM) || level < GET_LEVEL(victim))
    send_to_char("You fail.\r\n", ch);
  /* player charming another player - no legal reason for this */
  else if (!pk_allowed && !IS_NPC(victim))
    send_to_char("You fail - shouldn't be doing it anyway.\r\n", ch);
  else if (circle_follow(victim, ch))
    send_to_char("Sorry, following in circles can not be allowed.\r\n", ch);
  else if (!allow_follower(ch, 5))
	send_to_char("You can't have any more followers.\n\r", ch);
  else if (mag_savingthrow(victim, SAVING_PARA))
    send_to_char("Your victim resists!\r\n", ch);
  else if(mag_savingthrow(victim, SAVING_PARA))
    send_to_char("Your victim resists!\r\n", ch);

  else {
    if (victim->master)
      stop_follower(victim);

    add_follower(victim, ch);

    af.type = SPELL_CHARM;

    if (GET_INT(victim))
      af.duration = 1+ (20 * 10 / GET_INT(victim));
    else
      af.duration = 24;

    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_CHARM;
    af.bitvector2 = AFF_CHARM;
    af.bitvector3 = AFF_CHARM;
    affect_to_char(victim, &af);

    act("Isn't $n just such a nice fellow?", FALSE, ch, 0, victim, TO_VICT);
    if (IS_NPC(victim)) {
      REMOVE_BIT(MOB_FLAGS(victim), MOB_AGGRESSIVE);
      REMOVE_BIT(MOB_FLAGS(victim), MOB_SPEC);
    }
  }
}


ASPELL(spell_identify)
{
  int i;
  int found;
  int remort, obj_level;
  double peso;
  
  if (obj) {
    send_to_char("You feel informed:\r\n", ch);
    sprintf(buf, "Object '%s', Item type: ", obj->short_description);
    sprinttype(GET_OBJ_TYPE(obj), item_types, buf2);
    strcat(buf, buf2);
    strcat(buf, "\r\n");
    send_to_char(buf, ch);

    if (obj->obj_flags.bitvector) {
      send_to_char("Item will give you following abilities:  ", ch);
      sprintbit(obj->obj_flags.bitvector, affected_bits, buf);
      strcat(buf, "\r\n");
      send_to_char(buf, ch);
    }
    send_to_char("Item is: ", ch);
    sprintbit(GET_OBJ_EXTRA(obj), extra_bits, buf);
    strcat(buf, "\r\n");
    send_to_char(buf, ch);

    obj_level = (GET_OBJ_LEVEL(obj)-((LVL_IMMORT - 1)*(GET_OBJ_LEVEL(obj)/(LVL_IMMORT - 1))));
    if ((obj_level == 0) && (GET_OBJ_LEVEL(obj) > 0)){
     obj_level = (LVL_IMMORT - 1);	
     remort = (GET_OBJ_LEVEL(obj)/(LVL_IMMORT - 1)) - 1;
    } 
    else{
       remort = (GET_OBJ_LEVEL(obj)/(LVL_IMMORT - 1));
    } 
    peso = GET_OBJ_WEIGHT(obj);
    peso /= 10;
    sprintf(buf, "Weight: %.1fkg, Value: %d, Level: %d, Remorts: %d\r\n",
            peso, GET_OBJ_COST(obj)/*, GET_OBJ_RENT(obj)*/,
            obj_level, remort);
    send_to_char(buf, ch);

    switch (GET_OBJ_TYPE(obj)) {
    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_ERB:  
      sprintf(buf, "This %s casts: ", item_types[(int) GET_OBJ_TYPE(obj)]);

      if (GET_OBJ_VAL(obj, 1) >= 1)
        sprintf(buf + strlen(buf), " %s", spells[GET_OBJ_VAL(obj, 1)]);
      if (GET_OBJ_VAL(obj, 2) >= 1)
        sprintf(buf + strlen(buf), " %s", spells[GET_OBJ_VAL(obj, 2)]);
      if (GET_OBJ_VAL(obj, 3) >= 1)
        sprintf(buf + strlen(buf), " %s", spells[GET_OBJ_VAL(obj, 3)]);
      strcat(buf, "\r\n");
      send_to_char(buf, ch);
      break;
    case ITEM_WAND:
    case ITEM_STAFF:
      sprintf(buf, "This %s casts: ", item_types[(int) GET_OBJ_TYPE(obj)]);
      sprintf(buf + strlen(buf), " %s\r\n", spells[GET_OBJ_VAL(obj, 3)]);
      sprintf(buf + strlen(buf), "It has %d maximum charge%s and %d remaining.\r\n",
              GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 1) == 1 ? "" : "s",
              GET_OBJ_VAL(obj, 2));
      send_to_char(buf, ch);
      break;
    case ITEM_WEAPON:
      sprintf(buf, "Damage Dice is '%dD%d'", GET_OBJ_VAL(obj, 1),
              GET_OBJ_VAL(obj, 2));
      sprintf(buf + strlen(buf), " for an average per-round damage of %.1f.\r\n",
              (((GET_OBJ_VAL(obj, 2) + 1) / 2.0) * GET_OBJ_VAL(obj, 1)));
      send_to_char(buf, ch);
      break;
    case ITEM_ARMOR:
      sprintf(buf, "AC-apply is %d\r\n", GET_OBJ_VAL(obj, 0));
      send_to_char(buf, ch);
      break;
    }
    found = FALSE;
    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
      if ((obj->affected[i].location != APPLY_NONE) &&
          (obj->affected[i].modifier != 0)) {
        if (!found) {
          send_to_char("Can affect you as :\r\n", ch);
          found = TRUE;
        }
        sprinttype(obj->affected[i].location, apply_types, buf2);
        sprintf(buf, "   Affects: %s By %d\r\n", buf2, obj->affected[i].modifier);
        send_to_char(buf, ch);
      }
    }
  } else if (victim) {          /* victim */
    sprintf(buf, "Name: %s\r\n", GET_NAME(victim));
    send_to_char(buf, ch);
    if (!IS_NPC(victim)) {
      sprintf(buf, "%s is %d years, %d months, %d days and %d hours old.\r\n",
              GET_NAME(victim), age(victim)->year, age(victim)->month,
              age(victim)->day, age(victim)->hours);
      send_to_char(buf, ch);
    }
    sprintf(buf, "Height %d cm, Weight %d pounds\r\n",
            GET_HEIGHT(victim), GET_WEIGHT(victim));
    sprintf(buf + strlen(buf), "Level: %d, Remort: %d, Hit: %d, Mana: %d, Move: %d\r\n",
            GET_LEVEL(victim), GET_REMORT(victim), GET_HIT(victim), GET_MANA(victim), GET_MOVE(victim));
    sprintf(buf + strlen(buf), "AC: %d, Hitroll: %d, Damroll: %d\r\n",
            GET_AC(victim), GET_HITROLL(victim), GET_DAMROLL(victim));
    send_to_char(buf, ch);

  }
}

ASPELL(spell_oghma)
{
  int i;
  int found;
  int remort, obj_level;
  double peso;
  int at2 = 0 ;
  int at3 = 0 ;
  int at4 = 0 ;
  int dam = 0 ; 
  struct obj_data *wielded = GET_EQ(victim, WEAR_WIELD);


  if (obj) {
    send_to_char("You feel informed:\r\n", ch);
    sprintf(buf, "Object '%s', Item type: ", obj->short_description);
    sprinttype(GET_OBJ_TYPE(obj), item_types, buf2);
    strcat(buf, buf2);
    strcat(buf, "\r\n");
    send_to_char(buf, ch);

    if (obj->obj_flags.bitvector) {
      send_to_char("Item will give you following abilities:  ", ch);
      sprintbit(obj->obj_flags.bitvector, affected_bits, buf);
      strcat(buf, "\r\n");
      send_to_char(buf, ch);
    }
    send_to_char("Item is: ", ch);
    sprintbit(GET_OBJ_EXTRA(obj), extra_bits, buf);
    strcat(buf, "\r\n");
    send_to_char(buf, ch);

    obj_level = (GET_OBJ_LEVEL(obj)-((LVL_IMMORT - 1)*(GET_OBJ_LEVEL(obj)/(LVL_IMMORT - 1))));
    if ((obj_level == 0) && (GET_OBJ_LEVEL(obj) > 0)){
     obj_level = (LVL_IMMORT - 1);	
     remort = (GET_OBJ_LEVEL(obj)/(LVL_IMMORT - 1)) - 1;
    } 
    else{
       remort = (GET_OBJ_LEVEL(obj)/(LVL_IMMORT - 1));
    } 
    peso = GET_OBJ_WEIGHT(obj);
    peso /= 10;
    sprintf(buf, "Weight: %.1fkg, Value: %d, Level: %d, Remorts: %d\r\n",
            peso, GET_OBJ_COST(obj)/*, GET_OBJ_RENT(obj)*/,
            obj_level, remort);
    send_to_char(buf, ch);

    switch (GET_OBJ_TYPE(obj)) {
    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_ERB:
      sprintf(buf, "This %s casts: ", item_types[(int) GET_OBJ_TYPE(obj)]);

      if (GET_OBJ_VAL(obj, 1) >= 1)
        sprintf(buf + strlen(buf), " %s", spells[GET_OBJ_VAL(obj, 1)]);
      if (GET_OBJ_VAL(obj, 2) >= 1)
        sprintf(buf + strlen(buf), " %s", spells[GET_OBJ_VAL(obj, 2)]);
      if (GET_OBJ_VAL(obj, 3) >= 1)
        sprintf(buf + strlen(buf), " %s", spells[GET_OBJ_VAL(obj, 3)]);
      strcat(buf, "\r\n");
      send_to_char(buf, ch);
      break;
    case ITEM_WAND:
    case ITEM_STAFF:
      sprintf(buf, "This %s casts: ", item_types[(int) GET_OBJ_TYPE(obj)]);
      sprintf(buf + strlen(buf), " %s\r\n", spells[GET_OBJ_VAL(obj, 3)]);
      sprintf(buf + strlen(buf), "It has %d maximum charge%s and %d remaining.\r\n",
              GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 1) == 1 ? "" : "s",
              GET_OBJ_VAL(obj, 2));
      send_to_char(buf, ch);
      break;
    case ITEM_WEAPON:
      sprintf(buf, "Damage Dice is '%dD%d'", GET_OBJ_VAL(obj, 1),
              GET_OBJ_VAL(obj, 2));
      sprintf(buf + strlen(buf), " for an average per-round damage of %.1f.\r\n",
              (((GET_OBJ_VAL(obj, 2) + 1) / 2.0) * GET_OBJ_VAL(obj, 1)));
      send_to_char(buf, ch);
      break;
    case ITEM_ARMOR:
      sprintf(buf, "AC-apply is %d\r\n", GET_OBJ_VAL(obj, 0));
      send_to_char(buf, ch);
      break;
    }
    found = FALSE;
    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
      if ((obj->affected[i].location != APPLY_NONE) &&
          (obj->affected[i].modifier != 0)) {
        if (!found) {
          send_to_char("Can affect you as :\r\n", ch);
          found = TRUE;
        }
        sprinttype(obj->affected[i].location, apply_types, buf2);
        sprintf(buf, "   Affects: %s By %d\r\n", buf2, obj->affected[i].modifier);
        send_to_char(buf, ch);
      }
    }
  } else if (victim) {          /* victim */
    sprintf(buf, "Name: %s\r\n", GET_NAME(victim));
    send_to_char(buf, ch);
    if (!IS_NPC(victim)) {
      sprintf(buf, "%s is %d years, %d months, %d days and %d hours old.\r\n",
              GET_NAME(victim), age(victim)->year, age(victim)->month,
              age(victim)->day, age(victim)->hours);
      send_to_char(buf, ch);
    }
    sprintf(buf, "Height %d cm, Weight %d pounds\r\n",
            GET_HEIGHT(victim), GET_WEIGHT(victim));
    sprintf(buf + strlen(buf), "Level: %d, Remort: %d, Hit: %d, Mana: %d, Move: %d\r\n",
            GET_LEVEL(victim), GET_REMORT(victim), GET_HIT(victim), GET_MANA(victim), GET_MOVE(victim));
    sprintf(buf + strlen(buf), "AC: %d, Hitroll: %d, Damroll: %d\r\n",
            GET_AC(victim), GET_HITROLL(victim), GET_DAMROLL(victim));
    send_to_char(buf, ch);

    sprintf(buf, "GOLD: %d, Experience: %d\r\n",
            (GET_GOLD(victim) + GET_BANK_GOLD(victim)), GET_EXP(victim));
    send_to_char(buf, ch);
   
   if(IS_NPC(victim))
   {
    at2 = victim->mob_specials.attack1 ; 
    at3 = victim->mob_specials.attack2 ;
    at4 = victim->mob_specials.attack3 ;
   }
    else
   {
    at2 =  GET_SKILL_LS(victim, SKILL_SECOND_ATTACK);
    at3 =  GET_SKILL_LS(victim, SKILL_THIRD_ATTACK);
    at4 =  GET_SKILL_LS(victim, SKILL_FOURTH_ATTACK);
   } 
  
    sprintf(buf , "ATT2: %d, ATT3: %d, ATT4: %d\r\n",
             at2, at3, at4);
    send_to_char(buf, ch) ; 

    dam = GET_DAMROLL(victim) * 0.8;

    if (wielded && GET_OBJ_TYPE(wielded) == ITEM_WEAPON) {
      dam += ((dice(GET_OBJ_VAL(wielded, 1), GET_OBJ_VAL(wielded, 2))*GET_OBJ_COND(wielded))/100);
    } else {
      if (IS_NPC(victim)) {
        dam += victim->mob_specials.damnodice *
        (victim->mob_specials.damsizedice+1) * 0.5 ; 
      } else {
        dam += number(GET_LEVEL(victim) / 40, GET_LEVEL(victim) / 20);
      }
    }

    if(!IS_NPC(victim)){
       if (!wielded && GET_SKILL(victim, SKILL_HAND_DAMAGE)){
          dam += 1 + (SKILL_HAND_DAMAGE / 50);
       }
    }

    dam = dam * (1+ ((float)GET_STR(ch) /100)) ;  
 
    sprintf(buf , "Dano medio aproximado: %d\r\n",
             dam);
    send_to_char(buf, ch) ;
  }
}



ASPELL(spell_enchant_weapon)
{
  int i;

  if (ch == NULL || obj == NULL)
    return;

  if ((GET_OBJ_TYPE(obj) == ITEM_WEAPON) &&
      !OBJ_FLAGGED(obj, ITEM_MAGIC)) {

    for (i = 0; i < MAX_OBJ_AFFECT; i++)
      if (obj->affected[i].location != APPLY_NONE)
        return;

    SET_BIT(GET_OBJ_EXTRA(obj), ITEM_MAGIC);

    obj->affected[0].location = APPLY_HITROLL;
    obj->affected[0].modifier = div(level, 50).quot;

    obj->affected[1].location = APPLY_DAMROLL;
    obj->affected[1].modifier = 1 + div(level, 50).quot;

    if (IS_GOOD(ch)) {
      SET_BIT(GET_OBJ_EXTRA(obj), ITEM_ANTI_EVIL);
      act("$p glows blue.", FALSE, ch, obj, 0, TO_CHAR);
    } else if (IS_EVIL(ch)) {
      SET_BIT(GET_OBJ_EXTRA(obj), ITEM_ANTI_GOOD);
      act("$p glows red.", FALSE, ch, obj, 0, TO_CHAR);
    } else {
      act("$p glows yellow.", FALSE, ch, obj, 0, TO_CHAR);
    }
  }
}


ASPELL(spell_detect_poison)
{
  if (victim) {
    if (victim == ch) {
      if (AFF_FLAGGED(victim, AFF_POISON))
        send_to_char("You can sense poison in your blood.\r\n", ch);
      else
        send_to_char("You feel healthy.\r\n", ch);
    } else {
      if (AFF_FLAGGED(victim, AFF_POISON))
        act("You sense that $E is poisoned.", FALSE, ch, 0, victim, TO_CHAR);
      else
        act("You sense that $E is healthy.", FALSE, ch, 0, victim, TO_CHAR);
    }
  }

  if (obj) {
    switch (GET_OBJ_TYPE(obj)) {
    case ITEM_DRINKCON:
    case ITEM_FOUNTAIN:
    case ITEM_FOOD:
      if (GET_OBJ_VAL(obj, 3))
        act("You sense that $p has been contaminated.",FALSE,ch,obj,0,TO_CHAR);
      else
        act("You sense that $p is safe for consumption.", FALSE, ch, obj, 0,
            TO_CHAR);
      break;
    default:
      send_to_char("You sense that it should not be consumed.\r\n", ch);
    }
  }
}
void farsight(struct char_data * ch, struct char_data * vc)
{
  if (!vc->desc)
    return;

  if (IS_DARK(ch->in_room) && !CAN_SEE_IN_DARK(ch)) {
    send_to_char("It is pitch black...\r\n", vc);
    return;
  } else if (AFF_FLAGGED(ch, AFF_BLIND)) {
    send_to_char("You see nothing but infinite darkness...\r\n", vc);
    return;
  }
	if (ROOM_AFFECTED(ch->in_room, RAFF_FOG)) {
		/* NOTE: you might wish to change so that wizards,
		 * or the use of some 'see through fog' makes you see 
		 * through the fog
		 */
		send_to_char("Your view is obscured by a thick fog.\r\n", vc);
		return;
	}

  send_to_char(CCCYN(ch, C_NRM), vc);
  send_to_char(world[ch->in_room].name, vc);

  send_to_char(CCNRM(ch, C_NRM), vc);
  send_to_char("\r\n", vc);

  send_to_char(world[ch->in_room].description, vc);

  if (ROOM_AFFECTED(ch->in_room, RAFF_FIREWALL))
    send_to_char("&RYou see a large &Yfirewall&R here.&n\r\n", vc);
  if (ROOM_AFFECTED(ch->in_room, RAFF_ICEWALL))
    send_to_char("&CYou see a &Bicewall&C here.&n\r\n", vc);

  /* now list characters & objects */
  send_to_char(CCGRN(ch, C_NRM), vc);
  list_obj_to_char(world[ch->in_room].contents, vc, 0, FALSE);
  send_to_char(CCYEL(ch, C_NRM), vc);
  list_char_to_char(world[ch->in_room].people, vc);
  send_to_char(CCNRM(ch, C_NRM), vc);
}

ASPELL(spell_farsight)
{
  if (ch == NULL || victim == NULL)
    return;

  if ((GET_LEVEL(victim) > MIN(LVL_IMMORT - 1, level + 3)) || (PLR_FLAGGED(victim, PLR_DEAD))) {
    send_to_char(SUMMON_FAIL, ch);
    return;
  }

   if ((ZONE_FLAGGED(world[ch->in_room].zone, ZONE_NOSUMMON) ||
        ZONE_FLAGGED(world[victim->in_room].zone, ZONE_NOSUMMON)) &&
       (world[ch->in_room].zone != world[victim->in_room].zone)) {

    sprintf(buf, "You failed because %s is in another dimension.\r\n",
            GET_NAME(victim));
    send_to_char(buf, ch);

    return;
  }

  send_to_char("&CYou concentrate and see the world with other eyes...&n\r\n\r\n", ch);
  farsight(victim, ch);
}

void weather_chang(int modifier)
{
  int diff, change;
	struct char_data *ch;
  diff = modifier;

  weather_info.change += (dice(1, 4) * diff + dice(2, 6) - dice(2, 6));

  weather_info.change = MIN(weather_info.change, 12);
  weather_info.change = MAX(weather_info.change, -12);

  weather_info.pressure += weather_info.change;

  weather_info.pressure = MIN(weather_info.pressure, 1040);
  weather_info.pressure = MAX(weather_info.pressure, 960);

  change = 0;

  switch (weather_info.sky) {
  case SKY_CLOUDLESS:
    if (weather_info.pressure < 990)
      change = 1;
    else if (weather_info.pressure < 1010)
      if (dice(1, 4) == 1)
        change = 1;
    break;
  case SKY_CLOUDY:
    if (weather_info.pressure < 970)
      change = 2;
    else if (weather_info.pressure < 990) {
      if (dice(1, 4) == 1)
        change = 2;
      else
        change = 0;
    } else if (weather_info.pressure > 1030)
      if (dice(1, 4) == 1)
        change = 3;

    break;
  case SKY_RAINING:
    if (weather_info.pressure < 970) {
      if (dice(1, 4) == 1)
        change = 4;
      else
        change = 0;
    } else if (weather_info.pressure > 1030)
      change = 5;
    else if (weather_info.pressure > 1010)
      if (dice(1, 4) == 1)
        change = 5;

    break;
  case SKY_LIGHTNING:
    if (weather_info.pressure > 1010)
      change = 6;
    else if (weather_info.pressure > 990)
      if (dice(1, 4) == 1)
        change = 6;

    break;
  default:
    change = 0;
    weather_info.sky = SKY_CLOUDLESS;
    break;
  }

  switch (change) {
  case 0:
    break;
  case 1:
    send_to_outdoor("&WThe sky starts to get cloudy.&n\r\n");
    weather_info.sky = SKY_CLOUDY;
    break;
  case 2:
    send_to_outdoor("&cIt starts to rain.&n\r\n");
    send_to_outdoor("!!SOUND(thunder7.wav)&n\r\n");
    //play_sound(ch, "Thunder7.wav", SND_CHAR);
    weather_info.sky = SKY_RAINING;
    break;
  case 3:
    send_to_outdoor("&wThe clouds disappear.&n\r\n");
    weather_info.sky = SKY_CLOUDLESS;
    break;
  case 4:
    send_to_outdoor("&CLightning starts to show in the sky.&n\r\n");
    weather_info.sky = SKY_LIGHTNING;
    break;
  case 5:
    send_to_outdoor("&cThe rain stops.&n\r\n");
    weather_info.sky = SKY_CLOUDY;
    break;
  case 6:
    send_to_outdoor("&CThe lightning stops.&n\r\n");
   send_to_outdoor("!!SOUND(rain.wav)&n\r\n");
   // play_sound(ch, "Rain.wav", SND_CHAR);
    weather_info.sky = SKY_RAINING;
    break;
  default:
    break;
  }
}

ASPELL(spell_control_weather)
{
  int modifier;

  if (ch == NULL)
    return;
     
  send_to_char("You draw upon the strength of your loyalty to Ipslore and Strahd, the Elder Gods!\r\n", ch);
  send_to_room("The heavens stir as the forces of nature are changed!\r\n", ch->in_room);

  /* weather_info.pressure limits are now 940 - 1040 and
     weather_info.change is limited to +/- 12 anyhow */
  modifier = (int) div(GET_LEVEL(ch), 100).quot;

  /* the weather becomes better if the caster is inside and worse if outside */
  if (OUTSIDE(ch)) {
    modifier *= -1;
    send_to_room("A cold arctic wind blows from the north.\r\n", ch->in_room);
  } else {
    send_to_room("A warm wind blows from the south.\r\n", ch->in_room);
  }

  weather_chang(modifier);
}

ASPELL(spell_death_scythe)
{
	if (GET_EQ(ch, WEAR_WIELD)) {
		send_to_char("You cannot cast this spell while wielding something.\r\n", ch);
		return;
	}

	  obj = read_object(99, VIRTUAL); // !rent
	  GET_OBJ_WEIGHT(obj) = 1 ;
	  GET_OBJ_VAL(obj, 2) = (GET_LEVEL(ch) + (GET_REMORT(ch) * 7)); 
	  GET_OBJ_VAL(obj, 1) = 2;
	  REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_EVIL);
	  GET_OBJ_LEVEL(obj) = GET_LEVEL(ch); 
	  equip_char(ch, obj, WEAR_WIELD);
	  send_to_char("Death sends its scythe to help you.\r\n", ch);
}

#define MANA_COST (GET_MANA(ch));
ASPELL(spell_simulacrum)
{
 struct char_data *mob;

 if (!allow_follower(ch, 3))
 {
  send_to_char("You can't have any more followers.\n\r", ch);
  return;
 }

 if (number(0, 101) < 26)
 {
  send_to_char("You failed.\r\n", ch);
  return;
 }

 mob = read_mobile(29, VIRTUAL);
 GET_LEVEL(mob) = GET_LEVEL(ch);
 GET_MAX_HIT(mob) = (GET_MAX_HIT(ch)/2);
 GET_HITROLL(mob) = (GET_HITROLL(ch)/2);
 GET_DAMROLL(mob) = (GET_DAMROLL(ch)/2);
 GET_AC(mob) = GET_AC(ch);
 GET_CLASS(mob) = -1;
 mob->mob_specials.damsizedice = GET_LEVEL(ch);
 mob->mob_specials.damnodice = 1;
 GET_SEX(mob) = GET_SEX(ch);
 mob->player.name = str_dup(GET_NAME(ch));
 mob->player.short_descr = str_dup(GET_NAME(ch));
 char_to_room(mob, ch->in_room);
 SET_BIT(AFF_FLAGS(mob), AFF_CHARM);
 send_to_char("You make a copy of yourself.\r\n",ch);
 add_follower(mob, ch);
}

ASPELL(spell_peace)
{
   struct char_data *temp;


   act("$n tries to stop the fight.", TRUE, ch, 0, 0, TO_ROOM);
   act("You try to stop the fight.", FALSE, ch, 0, 0, TO_CHAR);

   if (IS_EVIL(ch)) {
        send_to_char("You are too much evil.\r\n", ch) ; 
	return;
	}
   
   if(IS_NEUTRAL(ch) && number(0,1)){
      send_to_char("You fail.\r\n", ch) ;
      return ;
   }
  
   for (temp = world[ch->in_room].people; temp; temp = temp->next_in_room)
       if (FIGHTING(temp)) {
          if(IS_NPC(temp))if(MOB_FLAGGED(temp, MOB_NOCHARM)) continue ;
          if(GET_LEVEL(ch) < GET_LEVEL(temp)) continue ;        
	  stop_fighting(temp);
          if (ch != temp) {
                act("$n stops fighting.", TRUE, temp, 0, 0, TO_ROOM);
                act("You stop fighting.", TRUE, temp, 0, 0, TO_CHAR);
                }
          }
   return;
}

ASPELL(spell_mana_sword)
{
  struct obj_data *tobj;
  
  if (ch == NULL || obj == NULL)
	  return;

  if ((GET_OBJ_TYPE(obj) == ITEM_STAFF) && OBJ_FLAGGED(obj, ITEM_MAGIC)) {
      extract_obj(obj);

      if (!(tobj = read_object(11, VIRTUAL))) {
        send_to_char("I seem to have goofed.\r\n", ch);
        return;
      }
      
      SET_BIT(GET_OBJ_EXTRA(tobj), ITEM_MAGIC);

      sprintf(buf, "mana sword %s", GET_NAME(ch));
      tobj->name = str_dup(buf);
      sprintf(buf, "The mana sword of %s", GET_NAME(ch));
      tobj->short_description = str_dup(buf);
      sprintf(buf, "A sword made with %s's magic powers is lying here.", GET_NAME(ch));
      tobj->description = str_dup(buf);
      tobj->affected[0].location = APPLY_HITROLL;
      tobj->affected[0].modifier = 2 + (level >= 18);
      tobj->affected[1].location = APPLY_DAMROLL;
      tobj->affected[1].modifier = 2 + (level >= 20);
      GET_OBJ_LEVEL(tobj) = GET_LEVEL(ch);
      GET_OBJ_VAL(tobj, 1) = 1;
      GET_OBJ_VAL(tobj, 2) = GET_LEVEL(ch)* 1.4;

      if (IS_GOOD(ch)) {
        SET_BIT(GET_OBJ_EXTRA(tobj), ITEM_ANTI_EVIL);
        act("$p glows blue.", FALSE, ch, tobj, 0, TO_CHAR);
      } else if (IS_EVIL(ch)) {
        SET_BIT(GET_OBJ_EXTRA(tobj), ITEM_ANTI_GOOD);
        act("$p glows red.", FALSE, ch, tobj, 0, TO_CHAR);
      } else {
        act("$p glows yellow.", FALSE, ch, tobj, 0, TO_CHAR);
      }

      obj_to_char(tobj, ch);
      act("$n creates $p.", FALSE, ch, tobj, 0, TO_ROOM);
      act("You create $p.", FALSE, ch, tobj, 0, TO_CHAR);
      load_otrigger(tobj);
  } else
    send_to_char("You cannot turn this equipment into a mana sword.\r\n", ch);
}



ASPELL(spell_call_night)
{

	if (weather_info.sunlight == SUN_DARK) {
		send_to_char("You cannot cast this spell at night.\r\n", ch);
		return;
	}
	weather_info.sunlight = SUN_DARK;
	send_to_outdoor("&bA &Kdark&b mantle covers the &nsky&b.&n\r\n");
}

ASPELL(spell_call_daylight)
{

	if (weather_info.sunlight == SUN_LIGHT) {
		send_to_char("You cannot cast this spell during the day.\r\n", ch);
		return;
	}
	weather_info.sunlight = SUN_LIGHT;
	send_to_outdoor("&YA shinning &nlight &Ycovers the &Csky&Y.&n\r\n");
}

ASPELL(spell_storm_sky)
{

	if (weather_info.sky == SKY_LIGHTNING) {
		send_to_char("You cannot cast this spell during the storm.\r\n", ch);
		return;
	}
	weather_info.sky = SKY_LIGHTNING;
	send_to_outdoor("&CA big &Bstorm &Cappears upon the skies.&n\r\n");
}



ASPELL(spell_fear)
{
  struct char_data *target = (struct char_data *) victim;
  struct char_data *next_target;
  int rooms_to_flee = 0;

  ACMD(do_flee);

  if (ch == NULL) 
        return;

  send_to_char("You radiate an aura of fear into the room!\r\n", ch);
  act("$n is surrounded by an aura of fear!", TRUE, ch, 0, 0, TO_ROOM);

  for (target = world[ch->in_room].people; target; target = next_target) {
        next_target = target->next_in_room;

        if (target == NULL)

       return;

if (GET_INT(ch) < GET_DEX(target))
	return;
	
	if (GET_LEVEL(ch) < GET_LEVEL(target))
	return;

if (target == ch)

       continue;


if (GET_LEVEL(target) >= LVL_IMMORT)

       continue;




if (mag_savingthrow(target, 1)) {

       sprintf(buf, "%s is unaffected by the fear!\r\n", GET_NAME(target));

       act(buf, TRUE, ch, 0, 0, TO_ROOM);

       send_to_char("Your victim is not afraid of the likes of you!\r\n", ch);

       if (IS_NPC(target))


 hit(target, ch, TYPE_UNDEFINED);

       }
               else {


 for(rooms_to_flee = level / 60; rooms_to_flee > 0; rooms_to_flee--) {


    send_to_char("You flee in terror!\r\n", target);


    do_flee(target, "", 0, 0);


 }

       }

}
}
