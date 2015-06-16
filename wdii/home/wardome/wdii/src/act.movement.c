/* ************************************************************************
*   File: act.movement.c                                Part of CircleMUD *
*  Usage: movement commands, door handling, & sleep/rest/etc state        *
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
#include "house.h"
#include "constants.h"
#include "dg_scripts.h"

/* external vars  */
extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern int top_of_world;
extern sh_int r_mortal_start_room;
extern int no_specials;
extern struct zone_data *zone_table;
void make_breath(struct char_data *ch);
/* external functs */
void add_follower(struct char_data *ch, struct char_data *leader);
int special(struct char_data *ch, int cmd, char *arg);
void death_cry(struct char_data *ch);
int find_eq_pos(struct char_data * ch, struct obj_data * obj, char *arg);
void die_in_dt(struct char_data * ch);
extern const char *from_dir[];
int level_exp(int chclass, int level);
void play_sound(struct char_data *ch, char *sound, int type);

/* local functions */
int has_boat(struct char_data *ch);
int find_door(struct char_data *ch, const char *type, char *dir, const char *cmdname);
int has_key(struct char_data *ch, int key);
void do_doorcmd(struct char_data *ch, struct obj_data *obj, int door, int scmd);
int ok_pick(struct char_data *ch, int keynum, int pickproof, int scmd);
ACMD(do_gen_door);
ACMD(do_enter);
ACMD(do_leave);
ACMD(do_stand);
ACMD(do_sit);
ACMD(do_rest);
ACMD(do_sleep);
ACMD(do_wake);
ACMD(do_follow);

char *leave_enter_string(struct char_data *ch)
{
    if (IS_AFFECTED(ch, AFF_FLY))
	return ("flies");
    else if (!IS_AFFECTED(ch, AFF_FLY) && ((SECT(ch->in_room) == SECT_WATER_NOSWIM) || (SECT(ch->in_room) == SECT_WATER_SWIM)) && IS_AFFECTED(ch, AFF_WATERWALK))
        return ("walks over water");
    else if (!IS_NPC(ch) && !IS_AFFECTED(ch, AFF_FLY) && !IS_AFFECTED(ch, AFF_WATERWALK) && ((SECT(ch->in_room) == SECT_WATER_NOSWIM) || (SECT(ch->in_room) == SECT_WATER_SWIM)) && GET_SKILL(ch, SKILL_SWIM))
        return ("swims");
    else
	return ("walks");

}
/* simple function to determine if char can walk on water */
int has_boat(struct char_data *ch)
{
  struct obj_data *obj;
  int i;
/*
  if (ROOM_IDENTITY(ch->in_room) == DEAD_SEA)
    return 1;
*/
  if (GET_LEVEL(ch) > LVL_IMMORT)
    return (1);

  if (AFF_FLAGGED(ch, AFF_WATERWALK))
    return (1);

  if (AFF_FLAGGED(ch, AFF_FLY))
    return (1);

  if (!IS_NPC(ch) && (GET_LEVEL(ch) < LVL_IMMORT) && (!AFF_FLAGGED(ch, AFF_WATERWALK) && !AFF_FLAGGED(ch, AFF_FLY)) && (GET_SKILL(ch, SKILL_SWIM) > number(1, 101))) {
    improve_skill(ch, SKILL_SWIM);
    return (1);
  }

  /* non-wearable boats in inventory will do it */
  for (obj = ch->carrying; obj; obj = obj->next_content)
    if (GET_OBJ_TYPE(obj) == ITEM_BOAT && (find_eq_pos(ch, obj, NULL) < 0))
      return (1);

  /* and any boat you're wearing will do it too */
  for (i = 0; i < NUM_WEARS; i++)
    if (GET_EQ(ch, i) && GET_OBJ_TYPE(GET_EQ(ch, i)) == ITEM_BOAT)
      return (1);

  return (0);
}

/* do_simple_move assumes
 *    1. That there is no master and no followers.
 *    2. That the direction exists.
 *
 *   Returns :
 *   1 : If succes.
 *   0 : If fail
 */

void check_traps(struct char_data *ch) 
{
	struct obj_data *trap;
	struct affected_type af[2];
	int i;

    af[0].type = 0;
    af[0].bitvector = 0;
    af[0].bitvector2 = 0;
    af[0].bitvector3 = 0;
    af[0].modifier = 0;
    af[0].location = APPLY_NONE;

    af[1].type = 0;
    af[1].bitvector = 0;
    af[1].bitvector2 = 0;
    af[1].bitvector3 = 0;
    af[1].modifier = 0;
    af[1].location = APPLY_NONE;
	
	for (trap = world[ch->in_room].contents; trap; trap = trap->next_content) {
		if ((GET_OBJ_TYPE(trap) == ITEM_TRAP) && (GET_OBJ_VAL(trap, 1) == 1)) {
			if (IS_NPC(ch)) {
				GET_HIT(ch) -= GET_MAX_HIT(ch) * ((float) GET_OBJ_VAL(trap, 0) / 100.0f);
				act("$n steps on a &KSpike Trap&n!", FALSE, ch, 0, 0, TO_ROOM);
				obj_from_room(trap);
				extract_obj(trap);
				return;
			} else if (GET_OBJ_VAL(trap, 3) != GET_IDNUM(ch)) {
				i = (GET_OBJ_LEVEL(trap) - GET_LEVEL(ch));
				if (i < PK_MIN_LEVEL && i > -PK_MIN_LEVEL) {
					GET_HIT(ch) -= GET_MAX_HIT(ch) * ((float) GET_OBJ_VAL(trap, 0) / 100.0f);
					if (GET_HIT(ch) < 5)
						GET_HIT(ch) = 5;
					send_to_char("&RYou step on a &KSpike Trap&R!&n\r\n", ch);
					act("&R$n steps on a &KSpike Trap&R!&n", FALSE, ch, 0, 0, TO_ROOM);
					obj_from_room(trap);
					extract_obj(trap);
					return;
				}
			}
		} else if ((GET_OBJ_TYPE(trap) == ITEM_TRAP) && (GET_OBJ_VAL(trap, 1) == 2)) {
			if (IS_NPC(ch)) {
				GET_MANA(ch) -= GET_MAX_MANA(ch) * ((float) GET_OBJ_VAL(trap, 0) / 100.0f);
				act("$n steps on a &MMana Trap&n!", FALSE, ch, 0, 0, TO_ROOM);
				obj_from_room(trap);
				extract_obj(trap);
				return;
			} else if (GET_OBJ_VAL(trap, 3) != GET_IDNUM(ch)) {
				i = (GET_OBJ_LEVEL(trap) - GET_LEVEL(ch));
				if (i < PK_MIN_LEVEL && i > -PK_MIN_LEVEL) {
					GET_MANA(ch) -= GET_MAX_MANA(ch) * ((float) GET_OBJ_VAL(trap, 0) / 100.0f);
					if (GET_MANA(ch) < 5)
						GET_MANA(ch) = 5;
					send_to_char("&RYou step on a &MMana Trap&R!&n\r\n", ch);
					act("&R$n steps on a &MMana Trap&R!&n", FALSE, ch, 0, 0, TO_ROOM);
					obj_from_room(trap);
					extract_obj(trap);
					return;
				}
			}
		} else if ((GET_OBJ_TYPE(trap) == ITEM_TRAP) && (GET_OBJ_VAL(trap, 1) == 3)) {
			if (IS_NPC(ch)) {
				act("$n steps on a &yFloor Trap&n!", FALSE, ch, 0, 0, TO_ROOM);
				GET_POS(ch) = POS_RESTING;
				obj_from_room(trap);
				extract_obj(trap);
				return;
			} else if (GET_OBJ_VAL(trap, 3) != GET_IDNUM(ch)) {
				i = (GET_OBJ_LEVEL(trap) - GET_LEVEL(ch));
				if (i < PK_MIN_LEVEL && i > -PK_MIN_LEVEL) {
					GET_MOVE(ch) -= GET_MAX_MANA(ch) * ((float) GET_OBJ_VAL(trap, 0) / 60.0f);
					if (GET_MOVE(ch) < 1)
						GET_MOVE(ch) = 1;
					GET_POS(ch) = POS_SITTING;
					send_to_char("&RYou step on a &yFloor Trap&R!&n\r\n", ch);
					act("&R$n steps on a &yFloor Trap&R!&n", FALSE, ch, 0, 0, TO_ROOM);
					obj_from_room(trap);
					extract_obj(trap);
					return;
				}
			}
		} else if ((GET_OBJ_TYPE(trap) == ITEM_TRAP) && (GET_OBJ_VAL(trap, 1) == 4)) {
			if (IS_NPC(ch)) {
				act("$n steps on the &CIpstrap&n!", FALSE, ch, 0, 0, TO_ROOM);
				obj_from_room(trap);
				extract_obj(trap);
				af[0].type = SPELL_CURSE;
				af[0].location = APPLY_HITROLL;
				af[0].duration = GET_OBJ_LEVEL(trap)/2;
				af[0].bitvector = AFF_CURSE;
				af[0].modifier = -10;

				af[1].type = SPELL_CURSE;
				af[1].location = APPLY_DAMROLL;
				af[1].duration = GET_OBJ_LEVEL(trap)/2;
				af[1].bitvector = AFF_CURSE;
				af[1].modifier = -10;

				affect_join(ch, af, TRUE, FALSE, TRUE, FALSE);
				affect_join(ch, af+1, TRUE, FALSE, TRUE, FALSE);
				return;
			} else if (GET_OBJ_VAL(trap, 3) != GET_IDNUM(ch)) {
				i = (GET_OBJ_LEVEL(trap) - GET_LEVEL(ch));
				if (i < PK_MIN_LEVEL && i > -PK_MIN_LEVEL) {
					GET_MOVE(ch) -= GET_MAX_MANA(ch) * ((float) GET_OBJ_VAL(trap, 0) / 60.0f);
					if (GET_MOVE(ch) < 1)
						GET_MOVE(ch) = 1;
					af[0].type = SPELL_CURSE;
					af[0].location = APPLY_STR;
					af[0].duration = number(8, 12);
					af[0].bitvector = AFF_CURSE;
					af[0].modifier = -8;

					af[1].type = SPELL_CURSE;
					af[1].location = APPLY_HIT;
					af[1].duration = number(4, 10);
					af[1].bitvector = AFF_CURSE;
					af[1].modifier = -(GET_HIT(ch)/2);

					affect_join(ch, af, TRUE, FALSE, TRUE, FALSE);
					affect_join(ch, af+1, TRUE, FALSE, TRUE, FALSE);

					send_to_char("&RYou step on the &CIpstrap&R!&n\r\n", ch);
					act("&R$n steps on the &CIpstrap&R!&n", FALSE, ch, 0, 0, TO_ROOM);
					obj_from_room(trap);
					extract_obj(trap);
					return;
				}
			}
		}

	}
}

int do_simple_move(struct char_data *ch, int dir, int need_specials_check)
{
  int was_in, need_movement;

  /*
   * Check for special routines (North is 1 in command list, but 0 here) Note
   * -- only check if following; this avoids 'double spec-proc' bug
   */
  if (need_specials_check && special(ch, dir + 1, "")) /* XXX: Evaluate NULL */
    return (0);

  /* charmed? */
  if (AFF_FLAGGED(ch, AFF_CHARM) && ch->master && ch->in_room == ch->master->in_room) {
	  send_to_char("The thought of leaving your master makes you weep.\r\n", ch);
	  act("$n bursts into tears.", FALSE, ch, 0, 0, TO_ROOM);
	  return (0);
  }

  if (AFF2_FLAGGED(ch, AFF2_ROOTED)) {
	  if (!IS_NPC(ch))
		  send_to_char("&gYou cannot move&G! &gThe &yroots&g are holding your feet&G!&n\r\n", ch);
	  return (0);
  }

  /* if this room or the one we're going to needs a boat, check for one */
  if (((SECT(ch->in_room) == SECT_WATER_NOSWIM) ||
      (SECT(EXIT(ch, dir)->to_room) == SECT_WATER_NOSWIM)) ||
      ((SECT(ch->in_room) == SECT_WATER_SWIM) ||
      (SECT(EXIT(ch, dir)->to_room) == SECT_WATER_SWIM))) {
    if (!has_boat(ch)) {
      send_to_char("You need a boat or need learn how to swim to go there.\r\n", ch);
      return 0;
    }
  }

  if (!IS_NPC(ch) && GET_COND(ch, DRUNK) > 0) {
	  if (number(1, 24) < --GET_COND(ch, DRUNK)) {
	  send_to_char("You feel dizzy! Cannot move correctly..\r\n", ch);
	  switch (number(1, 4)) {
	  case 1:
		  command_interpreter(ch, "n");
	  break;
	  case 2:
		  command_interpreter(ch, "s");
	  break;
	  case 3:
		  command_interpreter(ch, "w");
	  break;
	  case 4:
		  command_interpreter(ch, "e");
	  break;
	  default:
	  break;
	  }
	  return (0);
	  }
  }

  /* move points needed is avg. move loss for src and destination sect type */
  need_movement = (movement_loss[SECT(ch->in_room)] +
                   movement_loss[SECT(EXIT(ch, dir)->to_room)]) / 2;
  
  if ((!IS_NPC(ch)) && (FOREST(ch->in_room)) && (GET_SKILL(ch, SKILL_FOREST_WALK)))  {
	  need_movement = 0; // -ips
  }

  if (GET_MOVE(ch) < need_movement && !IS_NPC(ch) && GET_OXI(ch) > 5) {
    if (need_specials_check && ch->master)
      send_to_char("You are too exhausted to follow.\r\n", ch);
    else
      send_to_char("You are too exhausted.\r\n", ch);

    return (0);
  }

  if (ROOM_FLAGGED(ch->in_room, ROOM_ATRIUM)) {
    if (!House_can_enter(ch, GET_ROOM_VNUM(EXIT(ch, dir)->to_room))) {
      send_to_char("That's private property -- no trespassing!\r\n", ch);
      return (0);
    }
  }
  if (ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_TUNNEL) &&
      num_pc_in_room(&(world[EXIT(ch, dir)->to_room])) > 1) {
    send_to_char("There isn't enough room there for more than one person!\r\n", ch);
    return (0);
  }
  /* Mortals and low level gods cannot enter greater god rooms. */
  if (ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_GODROOM) && GET_LEVEL(ch) < LVL_GRGOD) {
	  send_to_char("You aren't godly enough to use that room!\r\n", ch);
	  return (0);
  }

  if (ROOM_AFFECTED(EXIT(ch, dir)->to_room, RAFF_ICEWALL) && GET_LEVEL(ch) < LVL_ELDER) {
	  send_to_char("&bYou can not enter the room, there is a &Cice wall&b in your way!\r\n", ch);
	  return (0);
  }

  if (AFF_FLAGGED(ch, AFF_HIDE))
	  REMOVE_BIT(AFF_FLAGS(ch), AFF_HIDE);

  if (ROOM_AFFECTED(EXIT(ch, dir)->to_room, RAFF_FIREWALL) && GET_LEVEL(ch) < LVL_ELDER)
	  send_to_char("&rWhen you cross the &Yfire wall&r, your feel your skin &Rburn&r!\r\n", ch);

  /* Now we know we're allow to go into the room. */
  if (GET_LEVEL(ch) < LVL_IMMORT && !IS_NPC(ch) && !AFF_FLAGGED(ch, AFF_FLY))
	  GET_MOVE(ch) -= need_movement;
   
  if (!AFF_FLAGGED(ch, AFF_SNEAK) && !ROOM_AFFECTED(ch->in_room, RAFF_FOG)) {
	  sprintf(buf2, "$n %s to %s.", leave_enter_string(ch), dirs[dir]);
	  act(buf2, TRUE, ch, 0, 0, TO_ROOM);
  }

  check_traps(ch);

/*  if (EXIT_FLAGGED(EXIT(ch, dir), EX_CLOSED) && AFF2_FLAGGED(ch, AFF2_PASSDOOR))
 sprintf(buf2, "$n %s straight through the closed door.", leave_enter_string(ch));
    act(buf2, TRUE, ch, 0, 0, TO_ROOM);   */

  /* see if an entry trigger disallows the move */
  if (!entry_mtrigger(ch))
	  return 0;
  if (!enter_wtrigger(&world[EXIT(ch, dir)->to_room], ch, dir))
	  return 0;

  was_in = ch->in_room;
  char_from_room(ch);
  char_to_room(ch, world[was_in].dir_option[dir]->to_room);

  if (ROOM_AFFECTED(ch->in_room, RAFF_FIREWALL) && GET_LEVEL(ch) < LVL_ELDER) {
	  send_to_char("&rWhen you enter the room, your feel your skin &Rburn&r!\r\n", ch);

	  // Dano modificado, Ips
	  if (GET_HIT(ch) > (GET_HIT(ch)*0.1f))
		  GET_HIT(ch) -= (GET_HIT(ch)*0.1f);
	  else
		  GET_HIT(ch) = -1;
  }

  if (!AFF_FLAGGED(ch, AFF_SNEAK) && !ROOM_AFFECTED(ch->in_room, RAFF_FOG)) {
    sprintf(buf2, "$n comes from %s.", from_dir[dir]);
    act(buf2, TRUE, ch, 0, 0, TO_ROOM);
  }

  check_traps(ch);

  if (ch->desc != NULL)
    look_at_room(ch, 0);

  if (ROOM_FLAGGED(ch->in_room, ROOM_DEATH) && GET_LEVEL(ch) < LVL_IMMORT) {
    log_death_trap(ch);
    die_in_dt(ch);
    return (0);
  }

  entry_memory_mtrigger(ch);
  if (!greet_mtrigger(ch, dir)) {
    char_from_room(ch);
    char_to_room(ch, was_in);
    look_at_room(ch, 0);
  } else
   greet_memory_mtrigger(ch);

  return (1);
}


int perform_move(struct char_data *ch, int dir, int need_specials_check)
{
  int was_in;
  struct follow_type *k, *next;

  if (ch == NULL || dir < 0 || dir >= NUM_OF_DIRS || FIGHTING(ch))
	  return (0);
  else if (!EXIT(ch, dir) || EXIT(ch, dir)->to_room == NOWHERE)
	  send_to_char("Alas, you cannot go that way...\r\n", ch);
  else if (EXIT_FLAGGED(EXIT(ch, dir), EX_CLOSED) /*&& (EXIT_FLAGGED(EXIT(ch, dir), EX_HIDDEN)  */&& !(!IS_NPC(ch) &&
      (PRF_FLAGGED(ch, PRF_CROSSDOORS) || AFF2_FLAGGED(ch, AFF2_PASSDOOR)))) {
    if (EXIT(ch, dir)->keyword) {
      sprintf(buf2, "The %s seems to be closed.\r\n", fname(EXIT(ch, dir)->keyword));
      send_to_char(buf2, ch);
    }
    else
      send_to_char("It seems to be closed.\r\n", ch);
  }
  else {
    if (!ch->followers)
      return (do_simple_move(ch, dir, need_specials_check));

    was_in = ch->in_room;
    if (!do_simple_move(ch, dir, need_specials_check))
      return (0);

    for (k = ch->followers; k; k = next) {
      next = k->next;
      if ((k->follower->in_room == was_in) &&
          (GET_POS(k->follower) >= POS_STANDING)) {
        act("You follow $N.\r\n", FALSE, k->follower, 0, ch, TO_CHAR);
        perform_move(k->follower, dir, 1);
      }
    }
    return (1);
  }
  return (0);
}


ACMD(do_move)
{
  /*
   * This is basically a mapping of cmd numbers to perform_move indices.
   * It cannot be done in perform_move because perform_move is called
   * by other functions which do not require the remapping.
   */
  perform_move(ch, subcmd - 1, 0);
}

int find_door(struct char_data *ch, const char *type, char *dir, const char *cmdname)
{
  int door;

  if (*dir) {                   /* a direction was specified */
	  
	  if ((door = search_block(dir, dirs, FALSE)) == -1) {        /* Partial Match */
		  send_to_char("That's not a direction.\r\n", ch);
		  return (-1);
	  }

	  if (EXIT(ch, door) /*&& IS_SET(EXIT(ch, door)->exit_info, EX_HIDDEN)*/) {       /* Braces added according to indent. -gg */
      if (EXIT(ch, door)->keyword) {
        if (isname(type, EXIT(ch, door)->keyword))
          return (door);
        else {
          sprintf(buf2, "I see no %s there.\r\n", type);
          send_to_char(buf2, ch);
          return (-1);
        }
      } else
        return (door);
    } else {
      sprintf(buf2, "I really don't see how you can %s anything there.\r\n", cmdname);
      send_to_char(buf2, ch);
      return (-1);
    }
  } else {                      /* try to locate the keyword */
    if (!*type) {
      sprintf(buf2, "What is it you want to %s?\r\n", cmdname);
      send_to_char(buf2, ch);
      return (-1);
    }
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (EXIT(ch, door))
        if (EXIT(ch, door)->keyword)
          if (isname(type, EXIT(ch, door)->keyword))
            return (door);

    sprintf(buf2, "There doesn't seem to be %s %s here.\r\n", AN(type), type);
    send_to_char(buf2, ch);
    return (-1);
  }
}


int has_key(struct char_data *ch, int key)
{
  struct obj_data *o;

  for (o = ch->carrying; o; o = o->next_content)
    if (GET_OBJ_VNUM(o) == key)
      return (1);

  if (GET_EQ(ch, WEAR_HOLD))
    if (GET_OBJ_VNUM(GET_EQ(ch, WEAR_HOLD)) == key)
      return (1);

  return (0);
}



#define NEED_OPEN	(1 << 0)
#define NEED_CLOSED	(1 << 1)
#define NEED_UNLOCKED	(1 << 2)
#define NEED_LOCKED	(1 << 3)

const char *cmd_door[] =
{
  "open",
  "close",
  "unlock",
  "lock",
  "pick"
};

const int flags_door[] =
{
  NEED_CLOSED | NEED_UNLOCKED,
  NEED_OPEN,
  NEED_CLOSED | NEED_LOCKED,
  NEED_CLOSED | NEED_UNLOCKED,
  NEED_CLOSED | NEED_LOCKED
};


#define EXITN(room, door)               (world[room].dir_option[door])
#define OPEN_DOOR(room, obj, door)      ((obj) ?\
                (TOGGLE_BIT(GET_OBJ_VAL(obj, 1), CONT_CLOSED)) :\
                (TOGGLE_BIT(EXITN(room, door)->exit_info, EX_CLOSED)))
#define LOCK_DOOR(room, obj, door)      ((obj) ?\
                (TOGGLE_BIT(GET_OBJ_VAL(obj, 1), CONT_LOCKED)) :\
                (TOGGLE_BIT(EXITN(room, door)->exit_info, EX_LOCKED)))

void do_doorcmd(struct char_data *ch, struct obj_data *obj, int door, int scmd)
{
  int other_room = 0;
  struct room_direction_data *back = 0;

  sprintf(buf, "$n %ss ", cmd_door[scmd]);
  if (!obj && ((other_room = EXIT(ch, door)->to_room) != NOWHERE))
    if ((back = world[other_room].dir_option[rev_dir[door]]) != NULL)
      if (back->to_room != ch->in_room)
        back = 0;

  switch (scmd) {
  case SCMD_OPEN:
  case SCMD_CLOSE:
    OPEN_DOOR(ch->in_room, obj, door);
    if (back)
      OPEN_DOOR(other_room, obj, rev_dir[door]);
    play_sound(ch, "door.wav", SND_CHAR);
    send_to_char(OK, ch);
    break;
  case SCMD_UNLOCK:
  case SCMD_LOCK:
    LOCK_DOOR(ch->in_room, obj, door);
    if (back)
      LOCK_DOOR(other_room, obj, rev_dir[door]);
    send_to_char("*Click*\r\n", ch);
        play_sound(ch, "unlock.wav", SND_ROOM);

    break;
  case SCMD_PICK:
    LOCK_DOOR(ch->in_room, obj, door);
    if (back)
      LOCK_DOOR(other_room, obj, rev_dir[door]);
    send_to_char("The lock quickly yields to your skills.\r\n", ch);
    strcpy(buf, "$n skillfully picks the lock on ");
        play_sound(ch, "picklock.wav", SND_ROOM);
    improve_skill(ch, SKILL_PICK_LOCK);
    break;
  }

  /* Notify the room */
  sprintf(buf + strlen(buf), "%s%s.", ((obj) ? "" : "the "), (obj) ? "$p" : (EXIT(ch, door)->keyword ? "$F" : "door"));
  if (!(obj) || (obj->in_room != NOWHERE))
    act(buf, FALSE, ch, obj, obj ? 0 : EXIT(ch, door)->keyword, TO_ROOM);

  /* Notify the other room */
  if ((scmd == SCMD_OPEN || scmd == SCMD_CLOSE) && back) {
    sprintf(buf, "The %s is %s%s from the other side.",
         (back->keyword ? fname(back->keyword) : "door"), cmd_door[scmd],
            (scmd == SCMD_CLOSE) ? "d" : "ed");
    if (world[EXIT(ch, door)->to_room].people) {
      act(buf, FALSE, world[EXIT(ch, door)->to_room].people, 0, 0, TO_ROOM);
      act(buf, FALSE, world[EXIT(ch, door)->to_room].people, 0, 0, TO_CHAR);
    }
  }
}


int ok_pick(struct char_data *ch, int keynum, int pickproof, int scmd)
{
  int percent;

  percent = number(1, 101);

  if (scmd == SCMD_PICK) {
    if (keynum < 0)
      send_to_char("Odd - you can't seem to find a keyhole.\r\n", ch);
    else if (pickproof)
      send_to_char("It resists your attempts to pick it.\r\n", ch);
    else if (percent > GET_SKILL(ch, SKILL_PICK_LOCK))
      send_to_char("You failed to pick the lock.\r\n", ch);
    else
      return (1);
    return (0);
  }
  return (1);
}


#define DOOR_IS_OPENABLE(ch, obj, door) ((obj) ? \
                        ((GET_OBJ_TYPE(obj) == ITEM_CONTAINER) && \
                        OBJVAL_FLAGGED(obj, CONT_CLOSEABLE)) :\
                        (EXIT_FLAGGED(EXIT(ch, door), EX_ISDOOR)))
#define DOOR_IS_OPEN(ch, obj, door)     ((obj) ? \
                        (!OBJVAL_FLAGGED(obj, CONT_CLOSED)) :\
                        (!EXIT_FLAGGED(EXIT(ch, door), EX_CLOSED)))
#define DOOR_IS_UNLOCKED(ch, obj, door) ((obj) ? \
                        (!OBJVAL_FLAGGED(obj, CONT_LOCKED)) :\
                        (!EXIT_FLAGGED(EXIT(ch, door), EX_LOCKED)))
#define DOOR_IS_PICKPROOF(ch, obj, door) ((obj) ? \
                        (OBJVAL_FLAGGED(obj, CONT_PICKPROOF)) : \
                        (EXIT_FLAGGED(EXIT(ch, door), EX_PICKPROOF)))

#define DOOR_IS_CLOSED(ch, obj, door)   (!(DOOR_IS_OPEN(ch, obj, door)))
#define DOOR_IS_LOCKED(ch, obj, door)   (!(DOOR_IS_UNLOCKED(ch, obj, door)))
#define DOOR_KEY(ch, obj, door)         ((obj) ? (GET_OBJ_VAL(obj, 2)) : \
                                        (EXIT(ch, door)->key))
#define DOOR_LOCK(ch, obj, door)        ((obj) ? (GET_OBJ_VAL(obj, 1)) : \
                                        (EXIT(ch, door)->exit_info))

ACMD(do_gen_door)
{
  int door = -1, keynum;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  struct obj_data *obj = NULL;
  struct char_data *victim = NULL;

  skip_spaces(&argument);
  if (!*argument) {
    sprintf(buf, "%s what?\r\n", cmd_door[subcmd]);
    send_to_char(CAP(buf), ch);
    return;
  }
  two_arguments(argument, type, dir);
  if (!generic_find(type, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj))
    door = find_door(ch, type, dir, cmd_door[subcmd]);

  if ((obj) || (door >= 0)) {
    keynum = DOOR_KEY(ch, obj, door);
    if (!(DOOR_IS_OPENABLE(ch, obj, door)))
      act("You can't $F that!", FALSE, ch, 0, cmd_door[subcmd], TO_CHAR);
    else if (!DOOR_IS_OPEN(ch, obj, door) &&
             IS_SET(flags_door[subcmd], NEED_OPEN))
      send_to_char("But it's already closed!\r\n", ch);
    else if (!DOOR_IS_CLOSED(ch, obj, door) &&
             IS_SET(flags_door[subcmd], NEED_CLOSED))
      send_to_char("But it's currently open!\r\n", ch);
    else if (!(DOOR_IS_LOCKED(ch, obj, door)) &&
             IS_SET(flags_door[subcmd], NEED_LOCKED))
      send_to_char("Oh.. it wasn't locked, after all..\r\n", ch);
    else if (!(DOOR_IS_UNLOCKED(ch, obj, door)) &&
             IS_SET(flags_door[subcmd], NEED_UNLOCKED))
      send_to_char("It seems to be locked.\r\n", ch);
    else if (!has_key(ch, keynum) && (GET_LEVEL(ch) < LVL_GOD) &&
             ((subcmd == SCMD_LOCK) || (subcmd == SCMD_UNLOCK)))
      send_to_char("You don't seem to have the proper key.\r\n", ch);
    else if (ok_pick(ch, keynum, DOOR_IS_PICKPROOF(ch, obj, door), subcmd))
      do_doorcmd(ch, obj, door, subcmd);
  }
  return;
}



ACMD(do_enter)
{
  int door;

  one_argument(argument, buf);

  if (*buf) {                   /* an argument was supplied, search for door
                                 * keyword */
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (EXIT(ch, door))
        if (EXIT(ch, door)->keyword)
          if (!str_cmp(EXIT(ch, door)->keyword, buf)) {
            perform_move(ch, door, 1);
            return;
          }
    sprintf(buf2, "There is no %s here.\r\n", buf);
    send_to_char(buf2, ch);
  } else if (ROOM_FLAGGED(ch->in_room, ROOM_INDOORS))
    send_to_char("You are already indoors.\r\n", ch);
  else {
    /* try to locate an entrance */
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (EXIT(ch, door))
        if (EXIT(ch, door)->to_room != NOWHERE)
          if (!EXIT_FLAGGED(EXIT(ch, door), EX_CLOSED) &&
              ROOM_FLAGGED(EXIT(ch, door)->to_room, ROOM_INDOORS)) {
            perform_move(ch, door, 1);
            return;
          }
    send_to_char("You can't seem to find anything to enter.\r\n", ch);
  }
}


ACMD(do_leave)
{
  int door;

  if (!ROOM_FLAGGED(ch->in_room, ROOM_INDOORS))
    send_to_char("You are outside.. where do you want to go?\r\n", ch);
  else {
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (EXIT(ch, door))
        if (EXIT(ch, door)->to_room != NOWHERE)
          if (!EXIT_FLAGGED(EXIT(ch, door), EX_CLOSED) &&
            !ROOM_FLAGGED(EXIT(ch, door)->to_room, ROOM_INDOORS)) {
            perform_move(ch, door, 1);
            return;
          }
    send_to_char("I see no obvious exits to the outside.\r\n", ch);
  }
}


ACMD(do_stand)
{
  switch (GET_POS(ch)) {
  case POS_STANDING:
    send_to_char("You are already standing.\r\n", ch);
    break;
  case POS_SITTING:
    send_to_char("You stand up.\r\n", ch);
    act("$n clambers to $s feet.", TRUE, ch, 0, 0, TO_ROOM);
    /* Will be sitting after a successful bash and may still be fighting. */
    GET_POS(ch) = FIGHTING(ch) ? POS_FIGHTING : POS_STANDING;
    break;
  case POS_RESTING:
    send_to_char("You stop resting, and stand up.\r\n", ch);
    act("$n stops resting, and clambers on $s feet.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_STANDING;
    break;
  case POS_SLEEPING:
    send_to_char("You have to wake up first!\r\n", ch);
    break;
  case POS_FIGHTING:
    send_to_char("Do you not consider fighting as standing?\r\n", ch);
    break;
  default:
    send_to_char("You stop floating around, and put your feet on the ground.\r\n", ch);
    act("$n stops floating around, and puts $s feet on the ground.",
        TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_STANDING;
    break;
  }
}


ACMD(do_sit)
{
  switch (GET_POS(ch)) {
  case POS_STANDING:
    send_to_char("You sit down.\r\n", ch);
    act("$n sits down.", FALSE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SITTING;
    break;
  case POS_SITTING:
    send_to_char("You're sitting already.\r\n", ch);
    break;
  case POS_RESTING:
    send_to_char("You stop resting, and sit up.\r\n", ch);
    act("$n stops resting.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SITTING;
    break;
  case POS_SLEEPING:
    send_to_char("You have to wake up first.\r\n", ch);
    break;
  case POS_FIGHTING:
    send_to_char("Sit down while fighting? Are you MAD?\r\n", ch);
    break;
  default:
    send_to_char("You stop floating around, and sit down.\r\n", ch);
    act("$n stops floating around, and sits down.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SITTING;
    break;
  }
}


ACMD(do_rest)
{
  switch (GET_POS(ch)) {
  case POS_STANDING:
    send_to_char("You sit down and rest your tired bones.\r\n", ch);
    act("$n sits down and rests.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_RESTING;
    break;
  case POS_SITTING:
    send_to_char("You rest your tired bones.\r\n", ch);
    act("$n rests.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_RESTING;
    break;
  case POS_RESTING:
    send_to_char("You are already resting.\r\n", ch);
    break;
  case POS_SLEEPING:
    send_to_char("You have to wake up first.\r\n", ch);
    break;
  case POS_FIGHTING:
    send_to_char("Rest while fighting?  Are you MAD?\r\n", ch);
    break;
  default:
    send_to_char("You stop floating around, and stop to rest your tired bones.\r\n", ch);
    act("$n stops floating around, and rests.", FALSE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SITTING;
    break;
  }
}


ACMD(do_sleep)
{
  switch (GET_POS(ch)) {
  case POS_STANDING:
  case POS_SITTING:
  case POS_RESTING:
    send_to_char("You go to sleep.\r\n", ch);
        play_sound(ch, "sleep.wav", SND_CHAR);
    act("$n lies down and falls asleep.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SLEEPING;
    break;
  case POS_SLEEPING:
    send_to_char("You are already sound asleep.\r\n", ch);
    break;
  case POS_FIGHTING:
    send_to_char("Sleep while fighting?  Are you MAD?\r\n", ch);
    break;
  default:
    send_to_char("You stop floating around, and lie down to sleep.\r\n", ch);
    act("$n stops floating around, and lie down to sleep.",
        TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SLEEPING;
    break;
  }
}


ACMD(do_wake)
{
  struct char_data *vict;
  int self = 0;

  one_argument(argument, arg);
  if (*arg) {
    if (GET_POS(ch) == POS_SLEEPING)
      send_to_char("Maybe you should wake yourself up first.\r\n", ch);
    else if ((vict = get_char_room_vis(ch, arg)) == NULL)
      send_to_char(NOPERSON, ch);
    else if (vict == ch)
      self = 1;
    else if (GET_POS(vict) > POS_SLEEPING)
      act("$E is already awake.", FALSE, ch, 0, vict, TO_CHAR);
    else if (AFF_FLAGGED(vict, AFF_SLEEP))
      act("You can't wake $M up!", FALSE, ch, 0, vict, TO_CHAR);
    else if (GET_POS(vict) < POS_SLEEPING)
      act("$E's in pretty bad shape!", FALSE, ch, 0, vict, TO_CHAR);
    else {
      act("You wake $M up.", FALSE, ch, 0, vict, TO_CHAR);
      act("You are awakened by $n.", FALSE, ch, 0, vict, TO_VICT | TO_SLEEP);
      GET_POS(vict) = POS_SITTING;
    }
    if (!self)
      return;
  }
  if (AFF_FLAGGED(ch, AFF_SLEEP))
    send_to_char("You can't wake up!\r\n", ch);
  else if (GET_POS(ch) > POS_SLEEPING)
    send_to_char("You are already awake...\r\n", ch);
  else {
    send_to_char("You awaken, and stand up.\r\n", ch);
    act("$n awakens.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_STANDING;
  }

}

ACMD(do_follow)
{
  struct char_data *leader;
  
  
  one_argument(argument, buf);

  if (*buf) {
    if (!(leader = get_char_room_vis(ch, buf))) {
      send_to_char(NOPERSON, ch);
      return;
    }
  } else {
    send_to_char("Whom do you wish to follow?\r\n", ch);
    return;
  }

  if (ch->master == leader) {
    act("You are already following $M.", FALSE, ch, 0, leader, TO_CHAR);
    return;
  }
  if (AFF_FLAGGED(ch, AFF_CHARM) && (ch->master)) {
    act("But you only feel like following $N!", FALSE, ch, 0, ch->master, TO_CHAR);
  } else {                      /* Not Charmed follow person */
    if (leader == ch) {
      if (!ch->master) {
        send_to_char("You are already following yourself.\r\n", ch);
        return;
      }
      stop_follower(ch);
    } else {
      if (circle_follow(ch, leader)) {
        send_to_char("Sorry, but following in loops is not allowed.\r\n", ch);
        return;
      }
      if (ch->master)
        stop_follower(ch);
      REMOVE_BIT(AFF_FLAGS(ch), AFF_GROUP);
      add_follower(ch, leader);
    }
  }
}

ACMD(do_recall)
{

  int exp;

  if (IS_NPC(ch)) {
    send_to_char("Hehehe... Too fun...\r\n", ch);
    return;
  }
  if (ROOM_FLAGGED(ch->in_room, ROOM_ARENA)) {
    send_to_char("Hehehe... Too fun... Looser!\r\n", ch);
    return;
  }
  if (ROOM_FLAGGED(ch->in_room, ROOM_LEARN)) {
    send_to_char("Must be level 2 to get out here!\r\n", ch);
    return;
  }
  if (PLR_FLAGGED(ch, PLR_DEAD)) {
    send_to_char("Are you alive?\r\n", ch);
    return;
  }

  exp = level_exp(GET_REMORT(ch), GET_LEVEL(ch))/2;
  if(GET_LEVEL(ch) > LVL_NEWBIE && GET_LEVEL(ch) < LVL_IMMORT) {
    if(GET_EXP(ch) > exp) {
       sprintf(buf, "&WYou loss &R%s&W experience.&n\r\n", add_points(exp));
       send_to_char(buf, ch);
       GET_EXP(ch) -= exp;
       GET_EXP(ch) = MAX(0,GET_EXP(ch));
    } else {
       send_to_char("You can not recall.\r\n", ch);
       return;
    }
  }

  send_to_char("You pray to the gods for your safety.\r\n"
               "You start fading and appear in The Wardome Preparation Room.\r\n"
               "\r\n", ch);
  act("$n mumbles a pray and disappears.", TRUE, ch, 0, 0, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, r_mortal_start_room);
  act("The gods drop $n in the middle of the room.", TRUE, ch, 0, 0, TO_ROOM);
  look_at_room(ch, 0);

  if (GET_LEVEL(ch) > LVL_NEWBIE_WARN && GET_LEVEL(ch) < LVL_NEWBIE)
    send_to_char("\r\nBeware, you are getting more experienced, some day you'll not get free recall.\r\n", ch);
}

#define MAX_PATHS       100
#define MAX_PATH_LENGTH 25
int do_run(struct char_data *ch, char argument[MAX_PATH_LENGTH])
{
  int times = 0, i, j, pos = 0, total = 0, step = 0, map = 0;
  char path[MAX_PATH_LENGTH+1];
  char comando[20];
  char paths[MAX_PATHS];
  char rome[]= "2sd10e2s10w" ;
  char newbie[]= "3es" ;
  char chess[]= "7end" ;
  char desire[]= "8wd" ;
  char drow[]= "2sd4endndd" ;
  char arachnos[]= "14wswwwunn" ;
  char ultima[]= "11wnnw";
  char zombie[]= "3e5n5n2e4n3euen";
  char elemental[]= "3e6n8undn";
  char elvenwoods[]= "14ws3wue";
  char orcenclave[]= "14w2sesws2w;swn3wnw3n2e3n3w";
  char olimpo[]= "s3d2sese2s7e";
  char bull[]= "4w2sws2w2n";
  char undead[]= "4e2se2s2e";
  char ghenna[]= "8w2sesesesu;wdswnne3n";
  char astral[]= "n2es;enter portal";
  char abyssal[]= "7w2nwnw2n;open stone;d;s";
  char armageddon[]= "3e5n5n2e4n3euen;suen";
  char abysmal[]= "2sd6wsw2s2wsw2d";
  char atlantis[]= "2sd6wsw2s2wsw3d;2sendsedenwdsend";
  char graveyard[]= "4e2sesseee3s";
  char gray[]= "2sd10e2s5w;open gate west;4w3n;open gate north;n";
  char dragon[]= "3e6n8undn;nnenenenenennenen;u;mount dragon";
  char france[]= "7w2nwnw2ne";
  char hell []= "6un;open gate;6n;wuwwu;open stee;n;open door west;w;n;n;e;open door;n;n;open mist;n;n;open hole;d";
  char tower[]= "3e10n10n4e10nwnen";
  char illiun[]= "2sd3w3wsw2s;2wse3n4neu";
  char mahnthor[]= "5e3s";
  char middlearth[]= "8w2seseses3u";
  char newthalos[]= "2sd6e5n;10e";
  char newcamelot[]= "14ws2w;2sws3wnw;3n2e3n;3w2n3w";
  char ogre[]= "5e3ses4es4e";
  char oldcamelot[]= "14w3ne3n";
  char oldthalos[]= "2sd10e4sw";
  char kerjim[]= "7w2nwnw2n;op stone;d";
  char minos[]= "2sd6wswn2ws";
  char shaolin[]= "2sd6wswnwssswwnu";
  char straight[]= "2sd6e19n";
  char sundhaven[]= "4w2sw2su";
  char shalizaard[]= "7eneennnnwwn";
  char thieves[]= "14wnnu";
  char theshire[]= "8e";
  char harpys[]= "8wsse5us";
  char wasteland[]= "14ws3wu;open tree;u";
  char welmar[]= "7en;open gate";
  char weriths[]= "2sd10e2s5w;open gate;wwws;wss;uuuuuuuu;open door up;u";
  char asgard[]= "2sd10e2s5w;op gate;3wswss8u8unesswwnwnw";
  char ratos[]= "s3ess;open grat; d";
  char midnir[]= "4ws";
  char ofcol[]= "2sd6e5n10e;open gra;3eu11n;open gate;5ne3n4e3ne";
  char dwarf[]= "3ed";
  char chapel[]= "3ed3nw;open doo;w2nw2n3e;open door; 2sd";
  char safari[]= "12wsd";
  char rands[]= "3e5n5n2e4n;3euede";
  char threeofsword[]= "3e2ne";
  char littlehaven[]= "2sd6wsw2s2ws";
  char redferne[]= "8w2seu";
  char piramide[]= "2sd11enen";
  char juargan[]= "3e3n2e;5u2edenes";
  char vampire[]= "3e5n5n2e4n;3euee";
  char ankou[]= "3e5n5n2e4n3eueu";
  char valleyofelfs[]= "2sd6e5n10e;open gra;3eu11n;open gate;7nwn";
  char mazeofdts[]= "2sd10ess;8wswsse";
  char newts[]= "6w;open rock;s";
  char samurai[]= "2sd10e2s;8wswss7us";
  char kerofk[]= "7ene";
  char cloudy[]= "3e6n8undn;nnenenenenennenen;u;mount dragon;w";
  char draconia[]= "3e6n8undn;nnenenenenennenen;u;mount dragon;7u";
  char anthill[]= "14wswwwuwn";
  char termite[]= "14wswwwuw;open do;s";

  
  skip_spaces(&argument);

  strcpy(path, "");
  strcpy(comando, "");

  for (pos=0; argument[pos] != '\0'; pos++) {
    switch (argument[pos]) {
      case 'n': if (argument[pos] == 'n') step = 0;
      case 'e': if (argument[pos] == 'e') step = 1;
      case 's': if (argument[pos] == 's') step = 2;
      case 'w': if (argument[pos] == 'w') step = 3;
      case 'u': if (argument[pos] == 'u') step = 4;
      case 'd': if (argument[pos] == 'd') step = 5;
        if (times == 0)
         times = 1;
        for (i=0; i!=times; i++) {
          if ((total++) == MAX_PATH_LENGTH) {
            sprintf(buf, "Path too long. Limit your pathes to %d steps.\r\n", MAX_PATH_LENGTH);
            send_to_char(buf, ch);
            return (TRUE);
          }
          sprintf(path + strlen(path), "%d", step);
        }
        times = 0;
        break;
      case '1': times = times * 10 + 1; break;
      case '2': times = times * 10 + 2; break;
      case '3': times = times * 10 + 3; break;
      case '4': times = times * 10 + 4; break;
      case '5': times = times * 10 + 5; break;
      case '6': times = times * 10 + 6; break;
      case '7': times = times * 10 + 7; break;
      case '8': times = times * 10 + 8; break;
      case '9': times = times * 10 + 9; break;
      case '0': times = times * 10;     break;
      default:
//        sprintf(buf, "Unrecognized direction in path (%d:'%c').\r\n", pos+1, argument[pos]);
//        send_to_char(buf, ch);
        return (FALSE);
        break;
    }
  }
	//CODIGO PRA MOSTRAR O PATH DE UMA ZONA QUANDO ACHADA
	//By ZHANTAR - Wardome Mud 2004
	if (!PRF2_FLAGGED(ch, PRF2_NOPATHS) && (!IS_NPC(ch)))
	{
				
	if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PATH)) //flagada com path
		{
		
    		
		j = world[ch->in_room].zone;
		
		if (zone_table[world[ch->in_room].zone].number == 120){
		strcpy(paths, rome);
		}
		if (zone_table[world[ch->in_room].zone].number == 186){
		strcpy(paths, newbie);
		}
		if (zone_table[world[ch->in_room].zone].number == 63){
		strcpy(paths, arachnos);
		}
		if (zone_table[world[ch->in_room].zone].number == 51){
		strcpy(paths, drow);
		}
		if (zone_table[world[ch->in_room].zone].number == 20){
		strcpy(paths, desire);
		}	
		if (zone_table[world[ch->in_room].zone].number == 36){
		strcpy(paths, chess);
		}
		if (zone_table[world[ch->in_room].zone].number == 141){
		  strcpy(paths, ultima);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 66){
		  strcpy(paths, zombie);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 92){
		  strcpy(paths, elemental);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 190){
		  strcpy(paths, elvenwoods);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 62){
		  strcpy(paths, orcenclave);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 97){
		  strcpy(paths, olimpo);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 32){
		  strcpy(paths, bull);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 30){
		  strcpy(paths, undead);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 99){
		  strcpy(paths, ghenna);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 122){
		  strcpy(paths, asgard);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 77){
		  strcpy(paths, astral);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 215){
		  strcpy(paths, abyssal);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 245){
		  strcpy(paths, armageddon);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 284){
		  strcpy(paths, abysmal);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 285){
		  strcpy(paths, atlantis);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 37){
		  strcpy(paths, graveyard);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 21){
		  strcpy(paths, gray);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 110){
		  strcpy(paths, dragon);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 82){
		  strcpy(paths, france);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 104){
		  strcpy(paths, hell);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 25){
		  strcpy(paths, tower);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 295){
		  strcpy(paths, illiun);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 23){
		  strcpy(paths, mahnthor);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 27){
		  strcpy(paths, middlearth);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 54){
		  strcpy(paths, newthalos);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 173){
		  strcpy(paths, newcamelot);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 52){
		  strcpy(paths, oldthalos);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 220){
		  strcpy(paths, kerjim);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 9){
		  strcpy(paths, minos);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 201){
		  strcpy(paths, shaolin);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 15){
		  strcpy(paths, straight);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 67){
		  strcpy(paths, sundhaven);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 98){
		  strcpy(paths, shalizaard);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 11){
		  strcpy(paths, theshire);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 91){
		  strcpy(paths, harpys);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 90){
		  strcpy(paths, wasteland);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 150){
		  strcpy(paths, welmar);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 200){
		  strcpy(paths, weriths);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 23){
		  strcpy(paths, ogre);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 251){
		  strcpy(paths, oldcamelot);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 130){
		  strcpy(paths, thieves);
		  }
		  if (zone_table[world[ch->in_room].zone].number == 205){
		strcpy(paths, termite);
		}
		if (zone_table[world[ch->in_room].zone].number == 204){
		strcpy(paths, anthill);
		}
		if (zone_table[world[ch->in_room].zone].number == 22){
		strcpy(paths, draconia);
		}
		if (zone_table[world[ch->in_room].zone].number == 132){
		strcpy(paths, cloudy);
		}
		if (zone_table[world[ch->in_room].zone].number == 7){
		strcpy(paths, kerofk);
		}
		if (zone_table[world[ch->in_room].zone].number == 130){
		strcpy(paths, samurai);
		}
		if (zone_table[world[ch->in_room].zone].number == 29){
		strcpy(paths, newts);
		}
		if (zone_table[world[ch->in_room].zone].number == 95){
		strcpy(paths, mazeofdts);
		}
		if (zone_table[world[ch->in_room].zone].number == 78){
		strcpy(paths, valleyofelfs);
		}
		if (zone_table[world[ch->in_room].zone].number == 1){
		strcpy(paths, ankou);
		}
		if (zone_table[world[ch->in_room].zone].number == 100){
		strcpy(paths, vampire);
		}
		if (zone_table[world[ch->in_room].zone].number == 47){
		strcpy(paths, juargan);
		}	
		if (zone_table[world[ch->in_room].zone].number == 53){
		strcpy(paths, piramide);
		}
		if (zone_table[world[ch->in_room].zone].number == 79){
		strcpy(paths, redferne);
		}
		if (zone_table[world[ch->in_room].zone].number == 18){
		strcpy(paths, littlehaven);
		}
		if (zone_table[world[ch->in_room].zone].number == 33){
		strcpy(paths, threeofsword);
		}
		if (zone_table[world[ch->in_room].zone].number == 64){
		strcpy(paths, rands);
		}
		if (zone_table[world[ch->in_room].zone].number == 2){
		strcpy(paths, safari);
		}
		if (zone_table[world[ch->in_room].zone].number == 34){
		strcpy(paths, chapel);
		}
		if (zone_table[world[ch->in_room].zone].number == 65){
		strcpy(paths, dwarf);
		}
		if (zone_table[world[ch->in_room].zone].number == 176){
		strcpy(paths, ofcol);
		}
		if (zone_table[world[ch->in_room].zone].number == 35){
		strcpy(paths, midnir);
		}
		if (zone_table[world[ch->in_room].zone].number == 38){
		strcpy(paths, ratos);
		}
		  
sprintf(buf, "\r\n&YYou found the path to &y%s: &C%s&n\r\n", zone_table[j].name, paths); 
	send_to_char(buf, ch);
send_to_char("&YFrom the new market square.&n\r\n\n", ch);

}
}

  if (GET_POS(ch) == POS_FIGHTING) {
	send_to_char("No way, stay right here and fight for your glory!\r\n", ch);
	return (TRUE);
  }

  if (GET_POS(ch) != POS_STANDING) {
	send_to_char("Nah... You feel too relaxed to do that..\r\n", ch);
	return (TRUE);
  }

  for (pos=0; path[pos] != '\0'; pos++) {
    sprintf(buf, "%c", path[pos]);
    if (path[pos] == '0') strcpy(comando,"north\0");
    if (path[pos] == '1') strcpy(comando,"east\0");
    if (path[pos] == '2') strcpy(comando,"south\0");
    if (path[pos] == '3') strcpy(comando,"west\0");
    if (path[pos] == '4') strcpy(comando,"up\0");
    if (path[pos] == '5') strcpy(comando,"down\0");

    if (no_specials || !special(ch, find_command(comando), comando)) {
	  if (total > 1 && PRF2_FLAGGED(ch, PRF2_MAP)) {
		REMOVE_BIT(PRF2_FLAGS(ch), PRF2_MAP);
	  	map = 1;
	  }
     perform_move(ch, atoi(buf), 0);
    } else
     return (TRUE);

    if (path[pos+1] != '\0')
     send_to_char("\r\n", ch);
  }
  if(map)
     SET_BIT(PRF2_FLAGS(ch), PRF2_MAP);

  return (TRUE);
}
/*
#define MAX_PATH_LENGTH 25
ACMD(do_run) {
  int times = 0, i, pos = 0, total = 0, step = 0;
  char path[MAX_PATH_LENGTH+1];

  skip_spaces(&argument);

  if (!*argument) {
    send_to_char("Run to where?\r\n", ch);
    return;
  }

  strcpy(path, "");

  for (pos=0; argument[pos] != '\0'; pos++) {
    switch (LOWER(argument[pos])) {
      case 'n': if (argument[pos] == 'n') step = 0;
      case 'e': if (argument[pos] == 'e') step = 1;
      case 's': if (argument[pos] == 's') step = 2;
      case 'w': if (argument[pos] == 'w') step = 3;
      case 'u': if (argument[pos] == 'u') step = 4;
      case 'd': if (argument[pos] == 'd') step = 5;
        if (times == 0)
         times = 1;
        for (i=0; i!=times; i++) {
          if ((total++) == MAX_PATH_LENGTH) {
            sprintf(buf, "Path too long. Limit your pathes to %d steps.\r\n", MAX_PATH_LENGTH);
            send_to_char(buf, ch);
            return;
          }
          sprintf(path + strlen(path), "%d", step);
        }
        times = 0;
        break;
      case '1': times = times * 10 + 1; break;
      case '2': times = times * 10 + 2; break;
      case '3': times = times * 10 + 3; break;
      case '4': times = times * 10 + 4; break;
      case '5': times = times * 10 + 5; break;
      case '6': times = times * 10 + 6; break;
      case '7': times = times * 10 + 7; break;
      case '8': times = times * 10 + 8; break;
      case '9': times = times * 10 + 9; break;
      case '0': times = times * 10;     break;
      default:
        sprintf(buf, "Unrecognized direction in path (%d:'%c').\r\n", pos+1, argument[pos]);
        send_to_char(buf, ch);
        return;
        break;
    }
  }

  for (pos=0; path[pos] != '\0'; pos++) {
    sprintf(buf, "%c", path[pos]);
    perform_move(ch, atoi(buf), 0);
    if (path[pos+1] != '\0')
     send_to_char("\r\n", ch);
  }
}
*/
