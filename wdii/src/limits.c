/* ************************************************************************
*   File: limits.c                                      Part of CircleMUD *
*  Usage: limits & gain funcs for HMV, exp, hunger/thirst, idle time      *
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
#include "spells.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "dg_scripts.h"
#include "buffer.h"

extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct room_data *world;
extern int max_exp_gain;
extern int max_exp_loss;
extern int idle_rent_time;
extern int idle_max_level;
extern int idle_void;
extern int use_autowiz;
extern int min_wizlist_lev;
extern int free_rent;
extern int trans_aff[][10];

int destransformar(struct char_data *ch, int trans);

int godmana[9] = {50,150,25,100,0,55,50,90,150};
int maxgodmana[9] ={100,200,100,150,0,100,200,150,200 } ;
int godpower[9] = {50,150,25,100,0,55,50,90,150} ;

/* local functions */
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6);
void check_autowiz(struct char_data * ch);


void Crash_rentsave(struct char_data *ch, int cost);
int level_exp(int remort, int level);
char *title_male(int chclass, int level);
char *title_female(int chclass, int level);
void update_char_objects(struct char_data * ch);        /* handler.c */

/* When age < 15 return the value p0 */
/* When age in 15..29 calculate the line between p1 & p2 */
/* When age in 30..44 calculate the line between p2 & p3 */
/* When age in 45..59 calculate the line between p3 & p4 */
/* When age in 60..79 calculate the line between p4 & p5 */
/* When age >= 80 return the value p6 */
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6)
{

  if (age < 15)
    return (p0);                /* < 15   */
  else if (age <= 29)
    return (int) (p1 + (((age - 15) * (p2 - p1)) / 15));        /* 15..29 */
  else if (age <= 44)
    return (int) (p2 + (((age - 30) * (p3 - p2)) / 15));        /* 30..44 */
  else if (age <= 59)
    return (int) (p3 + (((age - 45) * (p4 - p3)) / 15));        /* 45..59 */
  else if (age <= 79)
    return (int) (p4 + (((age - 60) * (p5 - p4)) / 20));        /* 60..79 */
  else
    return (p6);                /* >= 80 */
}


/*
 * The hit_limit, mana_limit, and move_limit functions are gone.  They
 * added an unnecessary level of complexity to the internal structure,
 * weren't particularly useful, and led to some annoying bugs.  From the
 * players' point of view, the only difference the removal of these
 * functions will make is that a character's age will now only affect
 * the HMV gain per tick, and _not_ the HMV maximums.
 */

/* manapoint gain pr. game hour */
int mana_gain(struct char_data * ch)
{
  int gain;

  if (IS_NPC(ch)) {
    /* Neat and fast */
    gain = GET_LEVEL(ch);
  } else {
    gain = (GET_MAX_MANA(ch)*0.035) + (GET_WIS(ch)/2);;

    /* Class calculations */

    /* Skill/Spell calculations */

    /* Position calculations    */
    switch (GET_POS(ch)) {
    case POS_SLEEPING:
      gain += (gain / 2);
      break;
    case POS_RESTING:
      gain += (gain / 3);       /* Divide by 3 */
      break;
    case POS_SITTING:
      gain += (gain / 4);       /* Divide by 4 */
      break;
    }

  if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
      gain /= 4;
  }
  if(gain <= 1)
    gain = 3;

  if (PLR_FLAGGED(ch, PLR_MEDITATE))
      gain *= 2;
      
  if ((IS_SET(ROOM_FLAGS(ch->in_room), ROOM_GOOD_REGEN)) || (ROOM_AFFECTED(ch->in_room, RAFF_FSANCTUARY)))
    gain += (gain * 0.25); 
        
  if (!IS_NPC(ch) && GET_MAX_MANA(ch) == GET_MANA(ch) && GET_SKILL(ch, SKILL_VITALIZE_MANA) > number(1, 101) && !AFF2_FLAGGED(ch, AFF2_TRANSFORM)) {
    improve_skill(ch, SKILL_VITALIZE_MANA);
    gain += (gain * 0.2);
  }  

  if (AFF_FLAGGED(ch, AFF_POISON))
    gain /= 4;

  if(PLR_FLAGGED(ch, PLR_DEAD) || IS_SET(ROOM_FLAGS(ch->in_room), ROOM_NO_REGEN_MANA) || AFF2_FLAGGED(ch, AFF2_TRANSFORM))
    gain = 0;
    
  if(GET_RACE(ch) == RACE_TINKER || (GET_RACE(ch) == RACE_DRACONIAN && number(1,12) == 3))
   gain = gain * 1.5 ;

  return (gain);
}


/* Hitpoint gain pr. game hour */
int hit_gain(struct char_data * ch)
{
  int gain;

  if (IS_NPC(ch)) {
    /* Neat and fast */
    gain = (GET_LEVEL(ch)*2) + (GET_CON(ch)/2);
    
  } else {
    gain = (GET_MAX_HIT(ch)*0.04) + (GET_CON(ch)/2);

    /* Class/Level calculations */

    /* Skill/Spell calculations */

    /* Position calculations    */

    switch (GET_POS(ch)) {
    case POS_SLEEPING:
      gain += (gain / 2);       /* Divide by 2 */
      break;
    case POS_RESTING:
      gain += (gain / 4);       /* Divide by 4 */
      break;
    case POS_SITTING:
      gain += (gain / 8);       /* Divide by 8 */
      break;
    }

    if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
      gain /= 4;
 
  }
  if(gain <= 1)
    gain = 3;
    
  if (AFF_FLAGGED(ch, AFF_REGEN))
    gain *= 1.5;
    
  if ((IS_SET(ROOM_FLAGS(ch->in_room), ROOM_GOOD_REGEN)) || (ROOM_AFFECTED(ch->in_room, RAFF_FSANCTUARY)))
    gain += (gain * 0.75); 

  if (!IS_NPC(ch) && GET_MAX_HIT(ch) == GET_HIT(ch) && GET_SKILL(ch, SKILL_VITALIZE_HEALTH) > number(1, 101) && !AFF2_FLAGGED(ch, AFF2_TRANSFORM)) {
    improve_skill(ch, SKILL_VITALIZE_HEALTH);
    gain += (gain * 0.3);
  }

  if (AFF_FLAGGED(ch, AFF_POISON))
    gain /= 4;

  if(PLR_FLAGGED(ch, PLR_DEAD) || IS_SET(ROOM_FLAGS(ch->in_room), ROOM_NO_REGEN_HIT) ||
  	(GET_POS(ch) == POS_FIGHTING) || AFF2_FLAGGED(ch, AFF2_TRANSFORM))
    gain = 0;

  if(GET_RACE(ch) == RACE_CAVE_TROLL || (GET_RACE(ch) == RACE_DRACONIAN && number(1,12) == 
3))  gain = gain * 1.5 ;
          
  return (gain);
}



/* move gain pr. game hour */
int move_gain(struct char_data * ch)
{
  int gain;

  if (IS_NPC(ch)) {
    /* Neat and fast */
    gain = (GET_LEVEL(ch)*2)+(GET_DEX(ch)/2);
  } else {
    gain = (GET_MAX_MOVE(ch)*0.035) +(GET_DEX(ch)/2);

    /* Position calculations    */
    switch (GET_POS(ch)) {
    case POS_SLEEPING:
      gain += (gain / 2);       /* Divide by 2 */
      break;
    case POS_RESTING:
      gain += (gain / 4);       /* Divide by 4 */
      break;
    case POS_SITTING:
      gain += (gain / 8);       /* Divide by 8 */
      break;
    }

    if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
      gain /= 4;
  }
  if(gain <= 1)
    gain = 3;
    
  if (AFF_FLAGGED(ch, AFF_POISON))
    gain /= 4;

  if(PLR_FLAGGED(ch, PLR_DEAD) || AFF2_FLAGGED(ch, AFF2_TRANSFORM))
    gain = 0;

  if(GET_RACE(ch) == RACE_BUGBEAR || (GET_RACE(ch) == RACE_DRACONIAN && number(1,12) == 3))
   gain = gain * 1.5 ;

  if(AFF2_FLAGGED(ch, AFF2_TRANSFORM))
    if(GET_MOVE(ch) < 1)
	destransformar(ch, GET_TRANS(ch));
    
  return (gain);
}
#define COST			8

int mental_gain(struct char_data * ch)
{
  int gain = 0;
  int custo[] = {-50,-5,-10,-20,-30,-40,-50} ; // custo por tick
                     // adicionado por Luigi para corrigir o bug dos transforms
                   
  if(!IS_NPC(ch)) {
    gain = (ch->real_abils.wis/6)+1;

    if (!PLR_FLAGGED(ch, PLR_MEDITATE))
        gain = 0;

    if(AFF2_FLAGGED(ch, AFF2_TRANSFORM)) {
      //  gain = trans_aff[GET_TRANS(ch)][COST]; 
        gain = custo[GET_TRANS(ch)] ; // substitui a linha anterior
        if(GET_MENTAL(ch)+gain < 1) {
	    destransformar(ch, GET_TRANS(ch));
	    gain = 0;
        }
    }
  }

  return (gain);
}

void mental_update(void)
{
  struct char_data *i, *next_char;

  for (i = character_list; i; i = next_char) {
    next_char = i->next;
    if(!i)  return;	
    if (GET_POS(i) >= POS_STUNNED)
      GET_MENTAL(i) = MIN(GET_MENTAL(i) + mental_gain(i), GET_MAX_MENTAL(i));
    if (GET_POS(i) < POS_INCAP)
      update_pos(i);  
  }
}

int breath_gain(struct char_data * ch)
{
  int gain;

  if (IS_NPC(ch)) {
    gain = 0;
  } else {
    gain = GET_CON(ch) / 2;
  }
  
    /* Position calculations    */
    switch (GET_POS(ch)) {
    case POS_SLEEPING:
      gain += (gain / 2);       /* Divide by 2 */
      break;
    case POS_RESTING:
      gain += (gain / 4);       /* Divide by 4 */
      break;
    case POS_SITTING:
      gain += (gain / 8);       /* Divide by 8 */
      break;
    case POS_FIGHTING:
      gain -= (gain / 2);       /* Divide by 2 */
      break;
    }

  if (AFF_FLAGGED(ch, AFF_POISON))
    gain /= 4;

  if(PLR_FLAGGED(ch, PLR_DEAD))
    gain = 0;
  
  return (gain);
}

void set_title(struct char_data *ch, char *title)
{
  if (title == NULL) {
    if (GET_SEX(ch) == SEX_FEMALE)
      title = title_female(GET_CLASS(ch), GET_LEVEL(ch));
    else
      title = title_male(GET_CLASS(ch), GET_LEVEL(ch));
  }

  if (strlen(title) > MAX_TITLE_LENGTH)
    title[MAX_TITLE_LENGTH] = '\0';

  if (GET_TITLE(ch) != NULL)
    free(GET_TITLE(ch));

  GET_TITLE(ch) = str_dup(title);
}

void set_prename(struct char_data * ch, char *title)
{
  if (title == NULL)
    title = NULL;

//  if (strlen(title) > MAX_TITLE_LENGTH)
//    title[MAX_TITLE_LENGTH] = '\0';

  if (GET_PRENAME(ch) != NULL)
    free(GET_PRENAME(ch));

  if(title != NULL)
    GET_PRENAME(ch) = str_dup(title);
  else
    GET_PRENAME(ch) = title;
}

void check_autowiz(struct char_data * ch)
{
#ifndef CIRCLE_UNIX
  return;
#else
  char buf[100];

  if (use_autowiz && GET_LEVEL(ch) >= LVL_IMMORT) {
    sprintf(buf, "nice ../bin/autowiz %d %s %d %s %d &", min_wizlist_lev,
            WIZLIST_FILE, LVL_IMMORT, IMMLIST_FILE, (int) getpid());
    mudlog("Initiating autowiz.", CMP, LVL_IMMORT, FALSE);
    system(buf);
  }
#endif /* CIRCLE_UNIX */
}

void gain_exp(struct char_data * ch, int gain)
{
  int is_altered = FALSE;
  int num_levels = 0;
  char buf[128];

  if (!IS_NPC(ch) && ((GET_LEVEL(ch) < 1 || GET_LEVEL(ch) >= LVL_IMMORT)))
    return;

  if (IS_NPC(ch)) {
    GET_EXP(ch) += gain;
    return;
  }

  if (gain > 0) {
    gain = MIN(max_exp_gain, gain);	/* put a cap on the max gain per kill */
    GET_EXP(ch) += gain;
    GET_EXP_SUM(ch) += gain;
    while (GET_LEVEL(ch) < LVL_IMMORT &&
	GET_EXP(ch) >= level_exp(GET_REMORT(ch), GET_LEVEL(ch) + 1)) {
      GET_LEVEL(ch) += 1;
      num_levels++;
      advance_level(ch);
      is_altered = TRUE;
    }

    if (is_altered) {
      sprintf(buf, "%s advanced %d level%s to level %d.",
		GET_NAME(ch), num_levels, num_levels == 1 ? "" : "s",
		GET_LEVEL(ch));
      mudlog(buf, BRF, 201, TRUE);
      if (num_levels == 1){
        GET_HIT(ch) = GET_MAX_HIT(ch);
	  GET_MANA(ch) = GET_MAX_MANA(ch);
	  GET_MOVE(ch) = GET_MAX_MOVE(ch);
        send_to_char("\007\007&WYou rise a level!&n\r\n", ch);
      }
      else {
	sprintf(buf, "\007\007&wYou rise &m%d &wlevels!&n\r\n", num_levels);
	send_to_char(buf, ch);
      }
//      set_title(ch, NULL);
      check_autowiz(ch);
    }
  } else if (gain < 0) {
    gain = MAX(-max_exp_loss, gain);	/* Cap max exp lost per death */
    GET_EXP(ch) += gain;
    GET_EXP_SUM(ch) += gain;
    if (GET_EXP(ch) < 0)
      GET_EXP(ch) = 0;
  }
}


void gain_exp_regardless(struct char_data * ch, int gain)
{
  GET_EXP(ch) += gain;

  if (!IS_NPC(ch)) {
    while(gain--) {
      GET_LEVEL(ch) += 1;
      advance_level(ch);
    }
//      set_title(ch, NULL);
      check_autowiz(ch);
  }
}

void gain_condition(struct char_data * ch, int condition, int value)
{
  bool intoxicated;

  if (IS_NPC(ch) || GET_COND(ch, condition) == -1)      /* No change */
    return;

  intoxicated = (GET_COND(ch, DRUNK) > 0);

  GET_COND(ch, condition) += value;

  GET_COND(ch, condition) = MAX(0, GET_COND(ch, condition));
  GET_COND(ch, condition) = MIN(24, GET_COND(ch, condition));

  if (GET_COND(ch, condition) || PLR_FLAGGED(ch, PLR_WRITING))
    return;

  switch (condition) {
  case FULL:
    send_to_char("&wYou are hungry.&n\r\n", ch);
    return;
  case THIRST:
    send_to_char("&wYou are thirsty.&n\r\n", ch);
    return;
  case DRUNK:
    if (intoxicated)
      send_to_char("You are now sober.\r\n", ch);
    return;
  default:
    break;
  }

}

void check_idling(struct char_data * ch)
{
  if (++(ch->char_specials.timer) > idle_void) {
    if (GET_WAS_IN(ch) == NOWHERE && ch->in_room != NOWHERE) {
      GET_WAS_IN(ch) = ch->in_room;
      if (FIGHTING(ch)) {
        stop_fighting(FIGHTING(ch));
        stop_fighting(ch);
      }
      act("$n disappears into the void.", TRUE, ch, 0, 0, TO_ROOM);
      send_to_char("You have been idle, and are pulled into a void.\r\n", ch);
      save_char(ch, NOWHERE);
      Crash_crashsave(ch);
      char_from_room(ch);
      char_to_room(ch, 1);
    } else if (ch->char_specials.timer > idle_rent_time) {
      if (ch->in_room != NOWHERE)
        char_from_room(ch);
      char_to_room(ch, 3);
      if (ch->desc) {
        STATE(ch->desc) = CON_DISCONNECT;
        /*
         * For the 'if (d->character)' test in close_socket().
         * -gg 3/1/98 (Happy anniversary.)
         */
        ch->desc->character = NULL;
        ch->desc = NULL;
      }
      if (free_rent)
        Crash_rentsave(ch, 0);

      sprintf(buf, "%s force-rented and extracted (idle).", GET_NAME(ch));
      mudlog(buf, CMP, LVL_GOD, TRUE);
      extract_char(ch);
    }
  }
}

void gods_update(void)
{
  struct char_data *i, *next_char;
  int x;
  for (i = character_list; i; i = next_char) {
    next_char = i->next;
    
    if (!i) return;

    x = GET_RELIGION(i) ;
 
    if((x>0) && (x != 5) && ((GET_LEVEL(i) > 100) || (GET_REMORT(i) > 0)) 
    && (GET_LEVEL(i) < 201))
     godmana[x-1] = MIN(godmana[x-1]+1, maxgodmana[x-1]) ;
  }
}




void hmm_update(void)
{
  struct char_data *i, *next_char;

  for (i = character_list; i; i = next_char) {
    next_char = i->next;

    if (!i) return;

    if (GET_POS(i) >= POS_STUNNED) {
      GET_MOVE(i) = MIN(GET_MOVE(i) + move_gain(i), GET_MAX_MOVE(i));
      GET_HIT(i) = MIN(GET_HIT(i) + hit_gain(i), GET_MAX_HIT(i));
      GET_MANA(i) = MIN(GET_MANA(i) + mana_gain(i), GET_MAX_MANA(i));
    }
    if (GET_POS(i) < POS_INCAP)
      update_pos(i);  
  }
}

/* Update PCs, NPCs, and objects */
void point_update(void)
{
  struct char_data *i, *next_char;
  struct obj_data *j, *next_thing, *jj, *next_thing2;
  char crashbuf[256];

  /* characters */
  for (i = character_list; i; i = next_char) {
    next_char = i->next;

    if (!i) return;

    gain_condition(i, FULL, -1);
    gain_condition(i, DRUNK, -1);
    gain_condition(i, THIRST, -1);
        
    if (GET_POS(i) >= POS_STUNNED) {
      if (AFF_FLAGGED(i, AFF_POISON))
        if (damage(i, i, 2, SPELL_POISON) == -1)
          continue;     /* Oops, they died. -gg 6/24/98 */
      if (GET_POS(i) <= POS_STUNNED)
        update_pos(i);
    } else if (GET_POS(i) == POS_INCAP) {
      if (damage(i, i, 1, TYPE_SUFFERING) == -1)
        continue;
    } else if (GET_POS(i) == POS_MORTALLYW) {
      if (damage(i, i, 2, TYPE_SUFFERING) == -1)
        continue;
    }
    if (!IS_NPC(i)) {
      update_char_objects(i);
      if (GET_LEVEL(i) < idle_max_level)
        check_idling(i);
    }
  }

  /* objects */
  for (j = object_list; j; j = next_thing) {
	  next_thing = j->next;       /* Next in object list */

	  if (!j)
		  return;

	  /* If this is a corpse */
	  if (IS_CORPSE(j)) {
		  /* timer count down */
		  if (GET_OBJ_TIMER(j) > 0)
			  GET_OBJ_TIMER(j)--;

		  if (!GET_OBJ_TIMER(j)) {

        if (j->carried_by)
          act("$p decays in your hands.", FALSE, j->carried_by, j, 0, TO_CHAR);
        else if ((j->in_room != NOWHERE) && (world[j->in_room].people)) {
          act("A quivering horde of maggots consumes $p.",
              TRUE, world[j->in_room].people, j, 0, TO_ROOM);
          act("A quivering horde of maggots consumes $p.",
              TRUE, world[j->in_room].people, j, 0, TO_CHAR);
        }
        for (jj = j->contains; jj; jj = next_thing2) {
			next_thing2 = jj->next_content;       /* Next in inventory */
			obj_from_obj(jj);
			
			if (!jj || !j)
				break;

			if (j->in_obj)
				obj_to_obj(jj, j->in_obj);
			else if (j->carried_by)
				obj_to_room(jj, j->carried_by->in_room);
			else if (j->in_room != NOWHERE)
				obj_to_room(jj, j->in_room);
			else { // Aqui o bixo cai..
				sprintf(crashbuf, "Objeto-Crash: %s", j->short_description);
				mudlog(crashbuf, CMP, LVL_IMMORT, FALSE);
				core_dump();
			}
        }
        extract_obj(j);
      }
    }
    /* If the timer is set, count it down and at 0, try the trigger */
    /* note to .rej hand-patchers: make this last in your point-update() */
    else if (GET_OBJ_TIMER(j)>0) {
      GET_OBJ_TIMER(j)--; 
      if (!GET_OBJ_TIMER(j))
        timer_otrigger(j);
    }
  }
}
