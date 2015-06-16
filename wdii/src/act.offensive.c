/* ************************************************************************
*   File: act.offensive.c                               Part of CircleMUD *
*  Usage: player-level commands of an offensive nature                    *
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
#include "buffer.h"
  
/* extern variables */
extern struct con_app_type con_app[];
extern struct room_data *world;
struct index_data *obj_index;   /* index table for object file   */
extern struct descriptor_data *descriptor_list;
extern int pk_allowed;
extern struct obj_data *object_list;
int level_exp(int remort, int level);
extern int can_pk(struct char_data * ch, struct char_data * vt);
int defender(struct char_data *ch, struct char_data *victim);

/* extern functions */
void check_killer(struct char_data * ch, struct char_data * vict);
void raw_kill(struct char_data * ch, struct char_data * killer);
extern const char *dirs[];
int can_pk(struct char_data * ch, struct char_data * vt);
extern int top_of_world;

/* local functions */
ACMD(do_assist);
ACMD(do_hit);
ACMD(do_kill);
ACMD(do_backstab);
ACMD(do_order);
ACMD(do_flee);
ACMD(do_bash);
ACMD(do_rescue);
ACMD(do_kick);
ACMD(do_disarm);
ACMD(do_berzerk);
ACMD(do_headbash);
ACMD(do_stroke);
ACMD(do_escape);
ACMD(do_suicide);
ACMD(do_knockout);
ACMD(do_throw);
ACMD(do_gut);
ACMD(do_drown);
ACMD(do_chop);
ACMD(do_kickflip);
ACMD(do_whirlwind);
ACMD(do_shuriken);
ACMD(do_howl);
//ACMD(do_slang);

ACMD(do_assist)
{
  struct char_data *helpee, *opponent;

  if (FIGHTING(ch)) {
    send_to_char("You're already fighting!  How can you assist someone else?\r\n", ch);
    return;
  }
  one_argument(argument, arg);

  if (!*arg)
    send_to_char("Whom do you wish to assist?\r\n", ch);
  else if (!(helpee = get_char_room_vis(ch, arg)))
    send_to_char(NOPERSON, ch);
  else if (helpee == ch)
    send_to_char("You can't help yourself any more than this!\r\n", ch);
  else {
    /*
     * Hit the same enemy the person you're helping is.
     */
    if (FIGHTING(helpee))
      opponent = FIGHTING(helpee);
    else
      for (opponent = world[ch->in_room].people;
           opponent && (FIGHTING(opponent) != helpee);
           opponent = opponent->next_in_room)
                ;

    if (!opponent)
      act("But nobody is fighting $M!", FALSE, ch, 0, helpee, TO_CHAR);
    else if (!CAN_SEE(ch, opponent))
      act("You can't see who is fighting $M!", FALSE, ch, 0, helpee, TO_CHAR);
    else if (!pk_allowed && !IS_NPC(opponent))  /* prevent accidental pkill */
      act("Use 'murder' if you really want to attack $N.", FALSE,
          ch, 0, opponent, TO_CHAR);
    else {
      send_to_char("&YYou join the fight!\r\n", ch);
      act("$N assists you!", 0, helpee, 0, ch, TO_CHAR);
      act("$n assists $N.", FALSE, ch, 0, helpee, TO_NOTVICT);
      hit(ch, opponent, TYPE_UNDEFINED);
    }
  }
}


ACMD(do_hit)
{
  struct char_data *vict;

  one_argument(argument, arg);

  if (!*arg)
    send_to_char("Hit who?\r\n", ch);
  else if (!(vict = get_char_room_vis(ch, arg)))
    send_to_char("They don't seem to be here.\r\n", ch);
  else if (vict == ch) {
    send_to_char("You hit yourself...OUCH!.\r\n", ch);
    act("$n hits $mself, and says OUCH!", FALSE, ch, 0, vict, TO_ROOM);
  } else if (AFF_FLAGGED(ch, AFF_CHARM) && (ch->master == vict))
    act("$N is just such a good friend, you simply can't hit $M.", FALSE, ch, 0, vict, TO_CHAR);
  else {
   if (!ROOM_FLAGGED(ch->in_room, ROOM_ARENA)){
    if (!IS_NPC(vict) && !IS_NPC(ch) && can_pk(ch, vict)) {
    	 sprintf(buf2, "%s (%d) attacks %s (%d) at %s [%d]",
    	 GET_NAME(ch), GET_LEVEL(ch), GET_NAME(vict), GET_LEVEL(vict),
         world[vict->in_room].name, GET_ROOM_VNUM(vict->in_room));
      mudlog(buf2, BRF, LVL_ELDER, TRUE);
    }
    if (!pk_allowed) {
      if (!IS_NPC(vict) && !IS_NPC(ch)) {
        if (subcmd != SCMD_MURDER) {
          send_to_char("Use 'murder' to hit another player.\r\n", ch);
          return;
        } else {
          check_killer(ch, vict);
        }
      }
      if (AFF_FLAGGED(ch, AFF_CHARM) && !IS_NPC(ch->master) && !IS_NPC(vict))
        return;                 /* you can't order a charmed pet to attack a
                                 * player */
    }
   }
    if ((GET_POS(ch) == POS_STANDING) && (vict != FIGHTING(ch))) {
      hit(ch, vict, TYPE_UNDEFINED);

      WAIT_STATE(ch, PULSE_VIOLENCE + 2);
    } else
      send_to_char("You do the best you can!\r\n", ch);
  }
}



ACMD(do_kill)
{
  struct char_data *vict;

  if ((GET_LEVEL(ch) < LVL_MJGOD) || IS_NPC(ch)) {
    do_hit(ch, argument, cmd, subcmd);
    return;
  }
  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Kill who?\r\n", ch);
  } else {
    if (!(vict = get_char_room_vis(ch, arg)))
      send_to_char("They aren't here.\r\n", ch);
    else if (ch == vict)
      send_to_char("Your mother would be so sad.. :(\r\n", ch);
    else if (GET_LEVEL(ch) <= GET_LEVEL(vict))
      do_hit(ch, argument, cmd, subcmd);
    else {
      act("&RYou chop $M to pieces!  Ah!  The blood!", FALSE, ch, 0, vict, TO_CHAR);
      act("&R$N chops you to pieces!", FALSE, vict, 0, ch, TO_CHAR);
      act("&R$n brutally slays $N!", FALSE, ch, 0, vict, TO_NOTVICT);
      raw_kill(vict, ch);
    }
  }
}



ACMD(do_backstab)
{
  struct char_data *vict;
  int percent, prob, prob2;

  one_argument(argument, buf);

  if (!GET_SKILL(ch, SKILL_BACKSTAB)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  if (!(vict = get_char_room_vis(ch, buf))) {
    send_to_char("Backstab who?\r\n", ch);
    return;
  }

  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  }

  if (vict == ch) {
    send_to_char("How can you sneak up on yourself?\r\n", ch);
    return;
  }
  if (!GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }
  if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_PIERCE - TYPE_HIT) {
    send_to_char("Only piercing weapons can be used for backstabbing.\r\n", ch);
    return;
  }
  if (FIGHTING(vict)) {
    send_to_char("&wYou can't backstab a fighting person -- they're too alert!\r\n", ch);
    return;
  }

  if (MOB_FLAGGED(vict, MOB_AWARE)) {
    act("&rYou notice $N lunging at you!", FALSE, vict, 0, ch, TO_CHAR);
    act("&r$e notices you lunging at $m!", FALSE, vict, 0, ch, TO_VICT);
    act("&r$n notices $N lunging at $m!", FALSE, vict, 0, ch, TO_NOTVICT);
    hit(vict, ch, TYPE_UNDEFINED);
    return;
  }

  percent = number(1, 101);     /* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_BACKSTAB);
  prob2 = GET_SKILL(ch, SKILL_SECOND_STAB);

  if (AWAKE(vict) && (percent > prob))
    damage(ch, vict, 0, SKILL_BACKSTAB);
  else {
	  
	  hit(ch, vict, SKILL_BACKSTAB);
	  improve_skill(ch, SKILL_BACKSTAB);
	  
	  if (AWAKE(vict) && prob2 && percent < (prob2 / 2 + (15 +con_app[GET_DEX(ch)].hitp) / 2 )){
		  hit(ch, vict, SKILL_BACKSTAB);
		  improve_skill(ch, SKILL_SECOND_STAB);
      }
	  
	  WAIT_STATE(ch, PULSE_VIOLENCE * 3);
  }
}



ACMD(do_order)
{
  char name[MAX_INPUT_LENGTH], message[MAX_INPUT_LENGTH];
  bool found = FALSE;
  int org_room;
  struct char_data *vict;
  struct follow_type *k;

  half_chop(argument, name, message);

  if (!*name || !*message)
    send_to_char("Order who to do what?\r\n", ch);
  
  else if (!str_cmp(name, "foll") || (!str_cmp(name, "fol")) || (!str_cmp(name, "follo")) || (!str_cmp(name, "follow")) || (!str_cmp(name, "fo")) || (!str_cmp(name, "f")))
  send_to_char("That person isn't here.\r\n", ch);
  else if (ch == vict)
  send_to_char("That person isn't here.\r\n", ch);
  else if (!(vict = get_char_room_vis(ch, name)))
    send_to_char("That person isn't here.\r\n", ch);
  else if (ch == vict)
    send_to_char("You obviously suffer from skitzofrenia.\r\n", ch);
  else if (!IS_NPC(vict))
  	send_to_char("You can't order players.\r\n", ch);

  else if (CMD_IS("follow"))
        send_to_char("No. You can't.\r\n", ch);
  else {
    if (AFF_FLAGGED(ch, AFF_CHARM)) {
      send_to_char("Your superior would not aprove of you giving orders.\r\n", ch);
      return;
    }
    if (vict) {
      sprintf(buf, "$N orders you to '%s'", message);
      act(buf, FALSE, vict, 0, ch, TO_CHAR);
      act("$n gives $N an order.", FALSE, ch, 0, vict, TO_ROOM);

      if ((vict->master != ch) || !AFF_FLAGGED(vict, AFF_CHARM))
        act("$n has an indifferent look.", FALSE, vict, 0, 0, TO_ROOM);
      else {
        send_to_char(OK, ch);
        command_interpreter(vict, message);
      }
    } else {                    /* This is order "followers" */
      sprintf(buf, "$n issues the order '%s'.", message);
      act(buf, FALSE, ch, 0, vict, TO_ROOM);

      org_room = ch->in_room;

      for (k = ch->followers; k; k = k->next) {
        if (org_room == k->follower->in_room)
          if (AFF_FLAGGED(k->follower, AFF_CHARM)) {
            found = TRUE;
            command_interpreter(k->follower, message);
          }
      }
      if (found)
        send_to_char(OK, ch);
      else
        send_to_char("Nobody here is a loyal subject of yours!\r\n", ch);
    }
  }
}



ACMD(do_flee)
{
  int i, attempt, loss_exp;
  struct char_data *was_fighting;

  if (IS_AFFECTED(ch, AFF_TANGLED)) {
    send_to_char("You would flee, but you're all tied up...\r\n", ch);
    return;
  }

 // if (!FIGHTING(ch)){
 //   send_to_char("You are not fighting!\r\n", ch);
 //   return;
 // }

  if (GET_POS(ch) < POS_FIGHTING) {
    send_to_char("You are in pretty bad shape, unable to flee!\r\n", ch);
    return;
  }

  if(GET_LEVEL(ch) > 1 && GET_LEVEL(ch) < LVL_IMMORT)
	loss_exp = ((level_exp(GET_REMORT(ch), GET_LEVEL(ch)) - level_exp(GET_REMORT(ch), GET_LEVEL(ch) - 1))/100);
  else
	loss_exp = 0;

  for (i = 0; i < 6; i++) {
    attempt = number(0, NUM_OF_DIRS - 1);       /* Select a random direction */
    if (CAN_GO(ch, attempt)) {
      act("$n panics, and attempts to flee!", TRUE, ch, 0, 0, TO_ROOM);
      was_fighting = FIGHTING(ch);
      if (do_simple_move(ch, attempt, TRUE)) {
        send_to_char("You flee head over heels.\r\n", ch);
	if(!ROOM_FLAGGED(ch->in_room, ROOM_ARENA))
		GET_EXP(ch) -= loss_exp;
      } else {
        act("$n tries to flee, but can't!", TRUE, ch, 0, 0, TO_ROOM);
      }
      return;
    }
  }
  send_to_char("&RPANIC!  You couldn't escape!&n\r\n", ch);
}


ACMD(do_bash)
{
  struct char_data *vict;
  int percent, prob;
  sh_int x;

  one_argument(argument, arg);

  x = (!IS_NPC(ch) ? GET_SKILL(ch, SKILL_BASH) : 0);

  if (!x) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }
  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  }
  if (!IS_NPC(ch) && !GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }
  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch))) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Bash who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  percent = number(1, 101);     /* 101% is a complete failure */
  prob = ((x*3.5) + (GET_STR(ch)*5))/4;

  if (prob > 100)
	  prob = 100;

  if (!IS_NPC(vict) && number(1, 110) < ((GET_SKILL(vict, SKILL_SNAKE_FIGHT)/2) + GET_DEX(vict))) {
	  act("You avoid $n's attack by moving like a snake!", FALSE, ch, 0, vict, TO_VICT);
	  act("$N avoids your attack by moving like a snake!", FALSE, ch, 0, vict, TO_CHAR);
	  act("$N avoids $n's attack by moving like a snake!", FALSE, ch, 0, vict, TO_ROOM | TO_NOTVICT);
	  percent = 101;
  }

  if (!IS_NPC(vict) && (defender(vict, ch)))
	  return;

  if (MOB_FLAGGED(vict, MOB_NOBASH) || BASHTIME(vict))
    percent = 101;

  if (percent > prob) {
	  damage(vict, ch, number(1, GET_LEVEL(ch)), SKILL_BASH);
  } else {
    /*
     * If we bash a player and they wimp out, they will move to the previous
     * room before we set them sitting.  If we try to set the victim sitting
     * first to make sure they don't flee, then we can't bash them!  So now
     * we only set them sitting if they didn't flee. -gg 9/21/98
     */
    if (damage(ch, vict, 1, SKILL_BASH) > 0) {	/* -1 = dead, 0 = miss */
      WAIT_STATE(vict, PULSE_VIOLENCE);
      if (IN_ROOM(ch) == IN_ROOM(vict)) {
        GET_POS(vict) = POS_SITTING;
	BASHTIME(vict) = 2;
        improve_skill(ch, SKILL_BASH);
      }
    }
  }
  WAIT_STATE(ch, PULSE_VIOLENCE);
}


ACMD(do_rescue)
{
  struct char_data *vict, *tmp_ch ;
  int percent, prob;

  one_argument(argument, arg);

  if (!*arg){
  	send_to_char("Rescue who?\r\n", ch);
  	return;
}

  if (!(vict = get_char_room_vis(ch, arg))) {
    send_to_char("Whom do you want to rescue?\r\n", ch);
    return;
  }

if (IS_NPC(vict)){
  	send_to_char("You can't rescue mobs.\r\n", ch);
  	return;
}

  if (PLR_FLAGGED(ch, PLR_NOPK) && (!(PLR_FLAGGED(vict, PLR_NOPK)))){
  send_to_char("You can only rescue NOPK players.\r\n", ch);
  return;
}
  if (!(PLR_FLAGGED(ch, PLR_NOPK)) && (PLR_FLAGGED(vict, PLR_NOPK))){
  	send_to_char("You can only rescue PK players.\r\n", ch);
  	return;
}

  if ((GET_LEVEL(ch) > GET_LEVEL(vict)) && (GET_LEVEL(ch) - GET_LEVEL(vict)> 10)){
  	send_to_char("You can't rescue someone out of your range.\r\n", ch);
  	return;
}
  if ((GET_LEVEL(vict) > GET_LEVEL(ch)) && ((GET_LEVEL(ch)- GET_LEVEL(vict) * -1) > 10)){
  	send_to_char("You can't rescue someone out of your range.\r\n", ch);
  	return;
}

  if (vict == ch) {
    send_to_char("What about fleeing instead?\r\n", ch);
    return;
  }
  if (FIGHTING(ch) == vict) {
    send_to_char("How can you rescue someone you are trying to kill?\r\n", ch);
    return;
  }
  for (tmp_ch = world[ch->in_room].people; tmp_ch &&
       (FIGHTING(tmp_ch) != vict); tmp_ch = tmp_ch->next_in_room);

  if (!tmp_ch) {
    act("But nobody is fighting $M!", FALSE, ch, 0, vict, TO_CHAR);
    return;
  }
  if (!GET_SKILL(ch, SKILL_RESCUE))
    send_to_char("But you have no idea how!\r\n", ch);
  else {
    percent = number(1, 101);   /* 101% is a complete failure */
    prob = GET_SKILL(ch, SKILL_RESCUE);

    if (percent > prob) {
      send_to_char("You fail the rescue!\r\n", ch);
      return;
    }
    send_to_char("Banzai!  To the rescue...\r\n", ch);
    act("You are rescued by $N, you are confused!", FALSE, vict, 0, ch, TO_CHAR);
    act("$n heroically rescues $N!", FALSE, ch, 0, vict, TO_NOTVICT);
    improve_skill(ch, SKILL_RESCUE);

    if (FIGHTING(vict) == tmp_ch)
      stop_fighting(vict);
    if (FIGHTING(tmp_ch))
      stop_fighting(tmp_ch);
    if (FIGHTING(ch))
      stop_fighting(ch);

    set_fighting(ch, tmp_ch);
    set_fighting(tmp_ch, ch);

    WAIT_STATE(vict, 2 * PULSE_VIOLENCE);
  }

}

ACMD(do_kick)
{
  struct char_data *vict;
  int percent, prob;
  sh_int x;

  x = (!IS_NPC(ch) ? GET_SKILL(ch, SKILL_KICK) : 0);

  if (!x) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }
  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch))) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Kick who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  percent = ((10 - (GET_AC(vict) / 10)) * 2) + number(1, 101);  /* 101% is a complete
                                                                 * failure */
  prob = x;


  if (AWAKE(vict) && percent > prob) {
    damage(ch, vict, 0, SKILL_KICK);
  } else
   { 
    if(GET_SKILL(ch,SKILL_POWER_KICK))
    {
     damage(ch, vict, ((GET_LEVEL(ch) + 10)*2) + (GET_REMORT(ch)*35), SKILL_KICK);
     improve_skill(ch,SKILL_POWER_KICK) ;
    }
    else 
     damage(ch, vict, GET_LEVEL(ch) + 10 + (GET_REMORT(ch)*35), SKILL_KICK);
   }
 
   improve_skill(ch, SKILL_KICK);
 
  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}


ACMD(do_howl)
{
  struct char_data *vict, *next_vict;
  sh_int skill, chance;

  skill = (!IS_NPC(ch) ? GET_SKILL(ch, SKILL_HOWL) : 0);

  if (!skill) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
	  send_to_char("Not here.\r\n", ch);
	  return;
  }

  if ((GET_MOVE(ch) < 30) || (GET_OXI(ch) < 10))  {
	  send_to_char("You are too tired to do this.\r\n", ch);
	  return;
  }
	
  act("&RAaAahHhHh!! &y$n &RHOWLS &yout loud! &RAaAahHhHh!!&n", TRUE, ch, 0, 0, TO_ROOM);
  send_to_char("&RAaAaAaaHHhhHhhHhh!!!&n\r\n", ch);

  for (vict = world[ch->in_room].people; vict; vict = next_vict) {
	next_vict = vict->next_in_room;

	if (vict == ch)
		continue;
	
	if ((GET_REMORT(vict)) && (GET_REMORT(ch) < (GET_REMORT(vict)+1)))
		continue;
	
	// Excluir pets do player e de vítimas fora do pk range -Ips
	if ((AFF_FLAGGED(vict, AFF_CHARM)) && (((vict->master) && (vict->master == ch)) || ((vict->master) && (!IS_NPC(vict->master)) && (!can_pk(ch, vict->master)))))
		continue;

	chance = (GET_LEVEL(ch) - GET_LEVEL(vict) + (GET_CHA(ch)/4));

	if ((IS_NPC(vict) || can_pk(ch, vict)))  {
		if ((GET_SKILL(ch, SKILL_HOWL) > number(1, 130)) && (chance > 0)) {
			do_flee(vict, NULL, 0, 0);
			improve_skill(ch, SKILL_HOWL);
		} else if (IS_NPC(vict) && GET_POS(vict) == POS_STANDING) {
			hit(vict, ch, TYPE_UNDEFINED);
		}
	}

  }

  GET_MOVE(ch) -= (GET_MOVE(ch) / 3);
  GET_OXI(ch) -= number(5, 10);

  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}


ACMD(do_sabre)
{
  struct char_data *vict;
  sh_int maxc, percent, prob, i, level, j = 0;

  prob = (!IS_NPC(ch) ? GET_SKILL(ch, SKILL_SABRE) : 0);

  if (!prob) {
    send_to_char("You'd better leave all the martial arts to samurais.\r\n", ch);
    return;
  }
  
  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  }
  
  if (!IS_NPC(ch) && !GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }

  if (!IS_NPC(ch) && (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_SLASH - TYPE_HIT)) {
    send_to_char("Only slash weapons can be used for the sabre skill.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
	  
	  if (FIGHTING(ch)) {
		  vict = FIGHTING(ch);
	  } else {
		  send_to_char("Sabre who?\r\n", ch);
		  return;
	  }
  }

  if (vict == ch) {
	  send_to_char("Aren't we funny today...\r\n", ch);
	  return;
  }

  level = MAX(30, GET_LEVEL(ch));
  maxc = (level / 23) + 1;

  j = 0;

  for (i = 1; i <= maxc; i++) {
	  percent = number(1, 101);

	  if (percent <= prob) {
		  improve_skill(ch, SKILL_SABRE);

		  if (GET_POS(vict) > POS_DEAD)
			  hit(ch, vict, SKILL_SABRE);
      
		  j++;
	  }
  }

  if (j == 0)
	  send_to_char("You need to practice your sabre.\r\n", ch);
  if (GET_POS(vict) > POS_DEAD)
	  WAIT_STATE(ch, PULSE_VIOLENCE * 3);
  improve_skill(ch, SKILL_SLASH);
}

ACMD(do_combo)
{
  struct char_data *vict;
  int maxc, percent, prob, i, level, j = 0;
  sh_int x;

  x = (!IS_NPC(ch) ? GET_SKILL(ch, SKILL_COMBO) : 0);

  if (!x) {
    send_to_char("You'd better leave all the martial arts to fighters.\r\n", ch);
    return;
  }
  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  }
  one_argument(argument, arg);
  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Combo who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  level = MAX(20, GET_LEVEL(ch));
  maxc = level / 27 + 1;
  j = 0;
  prob = x;
  for (i = 1; i <= maxc; i++) {
    percent = number(1, 101);
    if (percent <= prob) {
      improve_skill(ch, SKILL_COMBO);
      if (GET_POS(vict) > POS_DEAD)
        hit(ch, vict, SKILL_COMBO);
      j++;
    }
  }
  if (j == 0)
   send_to_char("You need to practice your combo!\r\n", ch);

  if (GET_POS(vict) > POS_DEAD)
    WAIT_STATE(ch, PULSE_VIOLENCE * 4);
}

ACMD(do_suicide)
{
  struct char_data *vict;
  vict = FIGHTING(ch);
 int limit;

 limit = (GET_MAX_HIT(ch)*0.1);
 
 //if (check_wanted(ch)){
// send_to_char("Wanted players can't suicide. Run Run Run! \r\n", ch);
// return;
//}
 if (ROOM_FLAGGED(ch->in_room, ROOM_ARENA)){
 	send_to_char("You can't suicide in arenas.\r\n", ch);
 	return;
}
 
  if (GET_POS(ch) != POS_FIGHTING){
  send_to_char("You can just suicide in the fight.\r\n", ch);
  return;
	}
	if (GET_HIT(ch) > limit){
		send_to_char("Suicide? why? you are to healthly!\r\n", ch);
		return;
	}
  if(IS_NPC(ch))
  {
   send_to_char("Mobs cannot do this.\r\n", ch) ;
   return ;
  }
  if (!GET_SKILL(ch, SKILL_SUICIDE)) {
    send_to_char("You need to know the skill to use it.\r\n", ch);
    return;
  }

  if (GET_LEVEL(ch) >= LVL_IMMORT) {
    send_to_char("Why would you kill yourself?\r\n", ch);
    return;
  }

 // if (vict == NULL ||(ch->in_room != vict->in_room) )
 //    return ;      // para evitar bugs

  send_to_char("You stab yourself in the heart and die, but you take your corpse with you.", ch);
  act("$n stabs himself on the heart to the death and takes his corpse with him.", TRUE, ch, 0, FALSE, TO_ROOM);
  act("$n suffers the wraith of the dead.", TRUE, vict, 0, FALSE, TO_ROOM);
  sprintf(buf, "(SUICIDE) %s suicides at %s.", GET_NAME(ch), world[ch->in_room].name);
    mudlog(buf, BRF, LVL_ELDER, TRUE);
  die_in_dt(ch);
  improve_skill(ch, SKILL_SUICIDE);
  
  	
}

ACMD(do_circle)
{
  struct char_data *vict;
  int percent, prob;

  one_argument(argument, buf);

  if (!GET_SKILL(ch, SKILL_CIRCLE_ARROUND)) {
    send_to_char("You do not know how.\r\n", ch);
    return;
  }

  if (!(vict = get_char_room_vis(ch, buf))) {
	if (FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch))) {
		vict = FIGHTING(ch);
	} else {
		send_to_char("Circle around who?\r\n", ch);
		return;
    	}
  }

  if (vict == ch) {
    send_to_char("How can you sneak up on yourself?\r\n", ch);
    return;
  }

  if (!GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }
  if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_PIERCE - TYPE_HIT) {
    send_to_char("Only piercing weapons can be used for backstabbing.\r\n", ch);
    return;
  }

  if (MOB_FLAGGED(vict, MOB_AWARE)) {
    act("&rYou notice $N lunging at you!", FALSE, vict, 0, ch, TO_CHAR);
    act("&r$e notices you lunging at $m!", FALSE, vict, 0, ch, TO_VICT);
    act("&r$n notices $N lunging at $m!", FALSE, vict, 0, ch, TO_NOTVICT);
    hit(vict, ch, TYPE_UNDEFINED);
    return;
  }

  percent = number(1, 101);     /* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_CIRCLE_ARROUND);

  if (AWAKE(vict) && (percent > prob))
    damage(ch, vict, 0, SKILL_BACKSTAB);
  else {
    hit(ch, vict, SKILL_BACKSTAB);
    improve_skill(ch, SKILL_CIRCLE_ARROUND);
    WAIT_STATE(ch, PULSE_VIOLENCE * 4);
  }
}

ACMD(do_disarm)
{
  struct char_data *victim;
  byte percent, prob;

  victim = FIGHTING(ch);
  percent = (!IS_NPC(ch) ? GET_SKILL(ch, SKILL_DISARM) : 0);

  if (!victim) {
    send_to_char("You are not fighting anyone.\n\r", ch);
    return;
  }

  if (!percent) {
    send_to_char("You don't know how!\n\r", ch);
    return;
  }

  if (MOB_FLAGGED(victim, MOB_NODISARM))
  {
	  send_to_char("This mob can not be disarmed.\r\n", ch);
	  return;
  }

  if (!GET_EQ(victim, WEAR_WIELD) || (GET_EQ(victim, WEAR_WIELD) &&
      !CAN_SEE_OBJ(ch, GET_EQ(victim, WEAR_WIELD)))) {
    act("$N isn't wielding anything.", FALSE, ch, 0, victim, TO_CHAR);
    return;
  }

  prob = number(1,101);
  if ((prob < percent) && (GET_LEVEL(victim) <= GET_LEVEL(ch))) {
    act("$n makes a quick manouver, disarming you.",
	TRUE, ch, 0, victim, TO_VICT);
    act("$n easily disarms $N, sending $p flying.",
	TRUE, ch, GET_EQ(victim, WEAR_WIELD), victim, TO_NOTVICT);
    act("You send $S weapon flying.", TRUE, ch, 0, victim, TO_CHAR);
    obj_to_room(unequip_char(victim, WEAR_WIELD), victim->in_room);
    improve_skill(ch, SKILL_DISARM);
  } else
    send_to_char("You try to disarm your opponent but fail.\n\r", ch);

  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}

ACMD(do_berzerk)
{
  sh_int x;

  x = (!IS_NPC(ch) ? GET_SKILL(ch, SKILL_BERZERK) : 0);

  if (!x) {
    send_to_char("You don't know how!\n\r", ch);
    return;
  }

  if (FIGHTING(ch) &&
      number(1, 101) < x) {
    send_to_char("You work up a frenzy and will fight to death!\n\r", ch);
    act("$n works up a frenzy and charges you!", TRUE, ch, 0, FIGHTING(ch), TO_VICT);
    act("$n works up a frenzy and charges $N.", TRUE, ch, 0, FIGHTING(ch), TO_ROOM);
    SET_BIT(AFF_FLAGS(ch), AFF_BERZERK);
    improve_skill(ch, SKILL_BERZERK);
  } else {
    send_to_char("You can't find the energy.\n\r", ch);
    act("$n hops around like a frenzied pig...", TRUE, ch, 0, 0, TO_ROOM);
  }

  WAIT_STATE(ch, PULSE_VIOLENCE);
}

ACMD(do_headbash)
{
  struct char_data *vict;
  int percent, prob;
  sh_int x;

  one_argument(argument, arg);

  x = (!IS_NPC(ch) ? GET_SKILL(ch, SKILL_HEADBASH) : 0);

  if (!x) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }
  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  }
  if (!IS_NPC(ch) && !GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }
  if (!FIGHTING(ch)) {
    send_to_char("You can't do this if you aren't fighting...\r\n", ch);
    return;
  }
  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch))) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Headbash who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  percent = number(1, 140);     /* 101% is a complete failure */
  prob = x/2;

  if (!IS_NPC(vict) && number(1, 140) < ((GET_SKILL(vict, SKILL_SNAKE_FIGHT)/2) + GET_DEX(vict))) {
	  act("You avoid $n's attack by moving like a snake!", FALSE, ch, 0, vict, TO_VICT);
	  act("$N avoids your attack by moving like a snake!", FALSE, ch, 0, vict, TO_CHAR);
	  act("$N avoids $n's attack by moving like a snake!", FALSE, ch, 0, vict, TO_ROOM | TO_NOTVICT);
	  percent = 101;

  }

  if (!IS_NPC(vict) && (defender(vict, ch)))
	  return;

  if (MOB_FLAGGED(vict, MOB_NOBASH) || BASHTIME(vict))
    percent = 101;

  if (percent > prob ) {
    send_to_char("You miss your power hit.\r\n", ch);
    act("$n miss the power hit.", TRUE, ch, 0, 0, TO_ROOM);
    WAIT_STATE(ch, PULSE_VIOLENCE);
  } else {
    WAIT_STATE(vict, PULSE_VIOLENCE);
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    if (IN_ROOM(ch) == IN_ROOM(vict)){
       act("You hit $S head forcing a fainting.", TRUE, ch, 0, FIGHTING(ch), TO_CHAR);
       act("$n hits your head and you fall down fainted! ZzZzzZZz.", TRUE, ch, 0, FIGHTING(ch), TO_VICT);
       act("$n hits $N's head forcing a fainting.", TRUE, ch, 0, FIGHTING(ch), TO_NOTVICT);
       GET_POS(vict) = POS_SLEEPING;
	BASHTIME(vict) = 3;
       improve_skill(ch, SKILL_HEADBASH);
    }
  }
}

ACMD(do_stroke)
{
  struct char_data *vict;
  int percent, prob;
  sh_int x;

  one_argument(argument, arg);

  x = (!IS_NPC(ch) ? GET_SKILL(ch, SKILL_STROKE) : 0);

  if (!x) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }
  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL)) {
    send_to_char("This room just has such a peaceful, easy feeling...\r\n", ch);
    return;
  }
  if (!IS_NPC(ch) && !GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }
  if (!FIGHTING(ch)) {
    send_to_char("You can't do this if you aren't fighting...\r\n", ch);
    return;
  }
  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch))) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Stroke who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  percent = number(1, 100);     /* 101% is a complete failure */
  prob = (x/4);

  if (!IS_NPC(vict) && number(1, 160) < ((GET_SKILL(vict, SKILL_SNAKE_FIGHT)/2) + GET_DEX(vict))) {
	  act("You avoid $n's attack by moving like a snake!", FALSE, ch, 0, vict, TO_VICT);
	  act("$N avoids your attack by moving like a snake!", FALSE, ch, 0, vict, TO_CHAR);
	  act("$N avoids $n's attack by moving like a snake!", FALSE, ch, 0, vict, TO_ROOM | TO_NOTVICT);
	  percent = 101;
  }

  if (!IS_NPC(vict) && (defender(vict, ch)))
	  return;


  if (MOB_FLAGGED(vict, MOB_NOBASH) || BASHTIME(vict))
    percent = 101;

  if (percent > prob) {
    send_to_char("You miss your crafty hit.\r\n", ch);
    act("$n miss the crafty hit.", TRUE, ch, 0, 0, TO_ROOM);
    WAIT_STATE(ch, PULSE_VIOLENCE);
  } else {
    WAIT_STATE(vict, PULSE_VIOLENCE);
    WAIT_STATE(ch, PULSE_VIOLENCE);	
    if (IN_ROOM(ch) == IN_ROOM(vict)){
       act("You hit $S strongly forcing a gidiness.", TRUE, ch, 0, FIGHTING(ch), TO_CHAR);
       act("$n hits you strongly, you are stunned! ZzZzzZZz.", TRUE, ch, 0, FIGHTING(ch), TO_VICT);
       sprintf(buf1, "$n hits $N's strongly and now %s is stunned.", HSSH(FIGHTING(ch)));
       act(buf1, TRUE, ch, 0, FIGHTING(ch), TO_NOTVICT);
       GET_POS(vict) = POS_MORTALLYW;
	BASHTIME(vict) = 2;
       improve_skill(ch, SKILL_STROKE);
    }
  }
}

ACMD(do_escape)
{
 int prob, percent, loss_exp;
 int retreat_type;

 one_argument(argument, arg);

 if (!FIGHTING(ch)){
   send_to_char("You are not fighting!\r\n", ch);
   return;
 }

 if (!*arg){
   send_to_char("Escape to?\r\n", ch);
   return;
 }

 if (!GET_SKILL(ch, SKILL_ESCAPE)){
   send_to_char("You do not know how.\r\n", ch);
   return;
 }

 retreat_type = search_block(argument + 1, dirs, FALSE);

 if (retreat_type < 0 || !EXIT(ch, retreat_type) ||
   EXIT(ch, retreat_type)->to_room == NOWHERE)
   {
   send_to_char("Escape to?\r\n", ch);
   return;
   }

 percent = GET_SKILL(ch, SKILL_ESCAPE);
 prob = number(0, 101);

 if(GET_LEVEL(ch) < LVL_IMMORT)
	loss_exp = ((level_exp(GET_CLASS(ch), GET_LEVEL(ch)) - level_exp(GET_CLASS(ch), GET_LEVEL(ch) - 1))/110);
 else
	loss_exp = 0;

 if (prob < percent){
    if (CAN_GO(ch, retreat_type) && !IS_SET(ROOM_FLAGS(EXIT(ch,retreat_type)->to_room), ROOM_DEATH)) {
       act("$n skillfully escape from combat.", TRUE, ch, 0, 0, TO_ROOM);
       send_to_char("You skillfully escape from combat.\r\n\r\n", ch);
       WAIT_STATE(ch, PULSE_VIOLENCE);
       improve_skill(ch, SKILL_ESCAPE);
       do_simple_move(ch, retreat_type, TRUE);
       if(!ROOM_FLAGGED(ch->in_room, ROOM_ARENA))
	  GET_EXP(ch) -= loss_exp;
      if (FIGHTING(ch)){
       stop_fighting(FIGHTING(ch));
       stop_fighting(ch);
      }
     } else {
      act("$n tries to escape from combat but has no where to go!", TRUE, ch, 0, 0, TO_ROOM);
      send_to_char("You cannot escape in that direction!\r\n", ch);
      return;
     }
 } else {
    send_to_char("You fail your attempt to escape!\r\n", ch);
    WAIT_STATE(ch, PULSE_VIOLENCE);
    return;
  }
}

ACMD(do_knockout)
{
  struct char_data *vict;
  int percent, prob;

  one_argument(argument, buf);

  if (!GET_SKILL(ch, SKILL_KNOCKOUT)) {
    send_to_char("You don't know how!\r\n", ch);
    return;
  }

  if (!(vict = get_char_room_vis(ch, buf))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Knockout who?\r\n", ch);
      return;
    }
  }

  if (vict == ch) {
    send_to_char("That would be funny to see.\r\n", ch);
    return;
  }
  if (!GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }
  if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_BLUDGEON - TYPE_HIT) {
    send_to_char("Only bludgeoning weapons can be used for knockingout.\r\n", ch);
    return;
  }
  if (FIGHTING(vict)) {
    send_to_char("You can't knock out a fighting person -- they're too alert!\r\n", ch);
    return;
  }

  if (MOB_FLAGGED(vict, MOB_AWARE)) {
    act("You notice $N raising $s weapon at you!", FALSE, vict, 0, ch, TO_CHAR);
    act("$e notices you raising your weapon!", FALSE, vict, 0, ch, TO_VICT);
    act("$n notices $N raising $s weapon at $m!", FALSE, vict, 0, ch, TO_NOTVICT);
    hit(vict, ch, TYPE_UNDEFINED);
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    return;
  }

  percent = number(1, 101);	/* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_KNOCKOUT);
  percent += MIN(72, (GET_CON(vict) * 3));
  percent += (GET_LEVEL(vict) - GET_LEVEL(ch));

  if (AWAKE(vict) && (percent < prob)) {
    act("You are knocked out when $N hits you upside your head.", FALSE, vict, 0, ch, TO_CHAR);
    act("$n sees stars, and slumps over, knocked out.", FALSE, vict, 0, ch, TO_VICT);
    act("$n sees stars, and slumps over, knocked out, after $N brains $m.", FALSE, vict, 0, ch, TO_NOTVICT);
    

    if(!IS_NPC(ch) && !IS_NPC(vict)){
      if(!can_pk(ch, vict)){
       send_to_char("&WYou can not &Rkill &Wyour opponent right now.&n\r\n", ch) ;
       return;
      }
    }

   if (GET_POS(vict) > POS_SLEEPING) {
      act("You feel very sleepy...  Zzzz......", FALSE, vict, 0, 0,TO_CHAR);
      act("$n goes to sleep.", TRUE, vict, 0, 0, TO_ROOM);
      GET_POS(vict) = POS_SLEEPING;
    }

    WAIT_STATE(vict, PULSE_VIOLENCE * 2);
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    improve_skill(ch, SKILL_KNOCKOUT);

  }  else {
    act("You notice $N raising $s weapon at you!", FALSE, vict, 0, ch, TO_CHAR);
    act("$e notices you raising your weapon!", FALSE, vict, 0, ch, TO_VICT);
    act("$n notices $N raising $s weapon at $m!", FALSE, vict, 0, ch, TO_NOTVICT);
    hit(ch, vict, SKILL_KNOCKOUT);
    WAIT_STATE(ch, PULSE_VIOLENCE);
  }
}

ACMD(do_throw)
{

  struct char_data *vict;
  struct obj_data *obj;
  int percent, prob;
  int damage_val;
  two_arguments(argument, buf, buf2);


  if (!GET_SKILL(ch, SKILL_THROW)) {
    send_to_char("You don't know how!\r\n", ch);
    return;
  }

  if (!(vict = get_char_room_vis(ch, buf2))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Throw what at who?\r\n", ch);
      return;
    }
  }

 if (!(obj = get_obj_in_list_vis(ch, buf, ch->carrying))) {
    send_to_char("Throw what at who?\r\n", ch);
    return;
  }

  if (vict == ch) {
    send_to_char("That would be funny to see.\r\n", ch);
    return;
  }

  percent = number(1, 101);	/* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_THROW);

  damage_val = GET_STR(ch) / 2 + GET_OBJ_WEIGHT(obj)  + GET_LEVEL(ch);

  if (percent > prob) {
    /* miss like a mother fucker. */
      damage(ch, vict, 0, SKILL_THROW);
        /* victim */
      act("$N throws $p at you and misses by a long shot.", FALSE, vict, obj, ch, TO_CHAR);
        /* ch */
      act("You throw $p at $n but, miss by a long shot.", FALSE, vict, obj, ch, TO_VICT);
        /* everyone else */
      act("$N throws $p at $n but, misses by a long shot.", FALSE, vict, obj, ch, TO_NOTVICT);
      return;
  }

  else {
      if (GET_OBJ_TYPE(obj) == ITEM_SCROLL || (GET_OBJ_TYPE(obj) == ITEM_NOTE)) {
        /* victim */
        act("$N hits you upside the head with $p and exclaims, Bad Doggie!", FALSE, vict, obj, ch, TO_CHAR);
        /* ch */
        act("You hit $n in the head with $p and exclaim, Bad Doggie!", FALSE, vict, obj, ch, TO_VICT);
        /* everyone else */
        act("$N hits $n in the head with $p and exclaims, Bad Doggie!", FALSE, vict, obj, ch, TO_NOTVICT);
        extract_obj(obj);

      }

      else if (GET_OBJ_TYPE(obj) == ITEM_WEAPON) {
       /* victim */
        act("$N throws $p at you and cuts your chest.", FALSE, vict, obj, ch, TO_CHAR);
        /* ch */
        act("You throw $p at $n and cut $m chest.", FALSE, vict, obj, ch, TO_VICT);
        /* everyone else */
        act("$N throws $p at $n and cuts $m chest.", FALSE, vict, obj, ch, TO_NOTVICT);
        extract_obj(obj);
      }

      else if (GET_OBJ_TYPE(obj) == ITEM_POTION) {

         /* victim */
        act("$N throws $p at you and it goes right down your throat!", FALSE, vict, obj, ch, TO_CHAR);
        /* ch */
        act("You throw $p at $n and it goes right down $m throat!", FALSE, vict, obj, ch, TO_VICT);
        /* everyone else */
        act("$N throws $p at $n and it goes right down $m throat!", FALSE, vict, obj, ch, TO_NOTVICT);
	extract_obj(obj);

//      if (prob > number(1, 100)) {  -> dando crash, -ips

	if (prob == -100) {

           mag_objectmagic(vict, obj, buf);

	} else {

        /* victim */
        act("You gag and spit out $p.", FALSE, vict, obj, ch, TO_CHAR);
        /* ch */
        act("$n gags and spits out $p.", FALSE, vict, obj, ch, TO_VICT);
        /* everyone else */
        act("$n gags and spits out $p.", FALSE, vict, obj, ch, TO_NOTVICT);
	
	}
      }

      else {
        act("$N throws $p and hits you square in the chest.", FALSE, vict, obj, ch, TO_CHAR);
        /* ch */
        act("You throw $p at $n and hit $m in the chest.", FALSE, vict, obj, ch, TO_VICT);
        /* everyone else */
        act("$N throws $p at $n and hits $m in the chest.", FALSE, vict, obj, ch, TO_NOTVICT);
       extract_obj(obj);
      }

  }

  damage(ch, vict, damage_val, SKILL_THROW);
  WAIT_STATE(ch, PULSE_VIOLENCE * 3);
  improve_skill(ch, SKILL_THROW);
  /* all done */

}


ACMD(do_gut)
{
  struct char_data *vict;
  int percent, prob, healthpercent;
  struct obj_data *piece;

  one_argument(argument, arg);

     if (!GET_SKILL(ch, SKILL_GUT))
   {
     send_to_char("You don't know how!\r\n", ch);
     return;
   }

     if (!(vict = get_char_room_vis(ch, arg)))
   {
       if (FIGHTING(ch))
      {
        vict = FIGHTING(ch);
      }
        else
      {
        send_to_char("Gut who?\r\n", ch);
       return;
      }
   }

     if (vict == ch)
   {
     send_to_char("Aren't we funny today...\r\n", ch);
     return;
   }
     if (!GET_EQ(ch, WEAR_WIELD))
   {
     send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
     return;
   }
     if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_SLASH - TYPE_HIT)
   {
    send_to_char("Only slashing weapons can be used for gutting.\r\n", ch);
    return;
   }
     percent = number(1, 101);	/* 101% is a complete failure */
     prob = ((GET_SKILL(ch, SKILL_GUT)/2)+(15 + con_app[GET_DEX(ch)].hitp)*2);

     if (GET_MAX_HIT(vict) > 0)
     healthpercent = (100 * GET_HIT(vict)) / GET_MAX_HIT(vict);

     else
     healthpercent = -1;

     if (healthpercent >= 5)
   {
     send_to_char("They are not hurt enough for you to attempt that.\r\n", ch);
     hit(vict, ch, TYPE_UNDEFINED);
     WAIT_STATE(ch, PULSE_VIOLENCE * 2);
     return;
   }

     if (percent > prob)
   {
    sprintf(buf, "Even in %s's bad state, they manage to avoid your wild slash.\r\n", GET_NAME(vict));
    send_to_char(buf, ch);
    send_to_char("You avoid a wild slash at your midsection.\r\n", ch);
   }
    else
   {

    /* EWWWW */
    GET_HIT(vict) = -10;

    act("You gut $N!", FALSE, ch, 0, vict, TO_CHAR);
    act("$N guts you!", FALSE, vict, 0, ch, TO_CHAR);
    act("$n brutally guts $N!", FALSE, ch, 0, vict, TO_NOTVICT);

    act("$N looks down in horror as their intestines spill out!", FALSE, ch, 0, vict, TO_ROOM);
    act("$N looks down in horror as their intestines spill out!", FALSE, ch, 0, vict, TO_CHAR);
    act("$N looks down in horror as their intestines spill out!", FALSE, vict, 0, ch, TO_CHAR);
    hit(vict, ch, TYPE_UNDEFINED);

    piece = create_obj();

    piece->item_number = NOTHING;
    piece->in_room = NOWHERE;
    piece->name = strdup("intestine");

    piece->short_description = strdup("An icky pile of intestines");
    piece->description = strdup("An icky pile of intestines is here, colon and all.");

    SET_BIT(GET_OBJ_WEAR(piece), ITEM_WEAR_TAKE);
    GET_OBJ_TYPE(piece) = ITEM_FOOD;
    GET_OBJ_VAL(piece, 0) = 1;
    GET_OBJ_VAL(piece, 3) = 1;   /* watch what you eat. */
    SET_BIT(GET_OBJ_EXTRA(piece), ITEM_NODONATE);
    GET_OBJ_WEIGHT(piece) = 23;
    GET_OBJ_RENT(piece) = 100000;
    obj_to_room(piece, ch->in_room);

    WAIT_STATE(vict, PULSE_VIOLENCE * 2);
    improve_skill(ch, SKILL_GUT);
    improve_skill(ch, SKILL_SLASH);
    update_pos(vict);
  }

}
ACMD(do_drown)
{

  struct char_data *vict;
  int flail, prob;

  if (!GET_SKILL(ch, SKILL_DROWN)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  if(IS_NPC(ch))
  {
   send_to_char("Mobs cannot do this.\r\n", ch) ;
   return ;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Drown who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }

  if ((SECT(ch->in_room) != SECT_WATER_NOSWIM) &&
      (SECT(ch->in_room) != SECT_WATER_SWIM)) {
    send_to_char("Drowning someone on dry land is pretty damn hard.\r\n", ch);
    return;
  }
   if (GET_RACE(vict) == RACE_SEA_ELF) {
   send_to_char(" Drowning a Sea Elf? I Don't Think so!\r\n", ch);
   return;
  }
  if (IS_NPC(vict)) {
    flail = number(1, 130) + GET_STR(vict) + (15 + con_app[GET_DEX(ch)].hitp);
  }
  else {
    flail = GET_SKILL(vict, SKILL_SWIM) + GET_STR(vict) + (15 + 
con_app[GET_DEX(ch)].hitp);
  }
  prob = GET_SKILL(ch, SKILL_DROWN) + GET_STR(ch) + (15 + con_app[GET_DEX(ch)].hitp);

  if (flail > prob) {
    damage(ch, vict, 0, SKILL_CHOP);
  } else {
    damage(ch, vict, (GET_SKILL(ch, SKILL_DROWN)) + (GET_LEVEL(ch)) + (GET_REMORT(ch)*10), SKILL_CHOP);
    GET_OXI(vict) -= number(8, 12) + (GET_STR(ch)/2) - (GET_CON(vict)/2);
  }

  WAIT_STATE(ch, PULSE_VIOLENCE);
  improve_skill(ch, SKILL_DROWN);
}


ACMD(do_chop)
{
  struct char_data *vict;
  int percent, prob;

	if (IS_NPC(ch)) {return;}

 	if (!GET_SKILL(ch, SKILL_CHOP))
 	{
		send_to_char("You have no idea how.\r\n", ch);
		return;
 	}

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) 
  {
    if (FIGHTING(ch)) 
    {
      vict = FIGHTING(ch);
    } else 
    {
      send_to_char("Chop who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) 
  {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }

  percent = number(1, 101);
  prob = (GET_SKILL(ch, SKILL_CHOP)/2)+(15 + con_app[GET_DEX(ch)].hitp)*3;
  
  if (percent > prob) 
  {
    damage(vict, ch, (GET_LEVEL(ch)*1.5 + GET_REMORT(ch)*20), SKILL_CHOP);
  } else
    damage(ch, vict, (GET_LEVEL(ch)*3 + GET_REMORT(ch)*25), SKILL_CHOP);

  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
  improve_skill(ch, SKILL_CHOP);
}

ACMD(do_kickflip)
{
  struct char_data *vict;
  int percent, prob;

  one_argument(argument, arg);

  if (GET_POS(ch) < POS_FIGHTING) {
	send_to_char("You can't!\r\n", ch);
	return;
  }

  if (!GET_SKILL(ch, SKILL_KICKFLIP)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Kickflip who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }

  if(!can_pk(ch, vict) && !IS_NPC(vict))
   return ;

  percent = number(1, 105);	/* 109% is a complete failure */
  prob = (GET_SKILL(ch, SKILL_KICKFLIP) / 2) + (15 + con_app[GET_DEX(ch)].hitp)*2;


  if (!IS_NPC(vict) && number(1, 100) < ((GET_SKILL(vict, SKILL_SNAKE_FIGHT)/2) + GET_DEX(vict))) {
	  act("You avoid $n's attack by moving like a snake!", FALSE, ch, 0, vict, TO_VICT);
	  act("$N avoids your attack by moving like a snake!", FALSE, ch, 0, vict, TO_CHAR);
	  act("$N avoids $n's attack by moving like a snake!", FALSE, ch, 0, vict, TO_ROOM | TO_NOTVICT);
	  percent = 101;
  }

  if (!IS_NPC(vict) && (defender(vict, ch)))
	  return;

  if (MOB_FLAGGED(vict, MOB_NOBASH) || BASHTIME(vict))
    percent = 101;

  if (percent > prob) {
    GET_POS(ch) = POS_SITTING;
    damage(vict, ch, GET_LEVEL(vict)*2, SKILL_KICKFLIP);
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
  } else {
      WAIT_STATE(ch, PULSE_VIOLENCE * 2);
      if (IN_ROOM(ch) == IN_ROOM(vict)) {
        GET_POS(vict) = POS_SITTING;
        damage(ch, vict, GET_LEVEL(ch), SKILL_KICKFLIP);
        WAIT_STATE(vict, PULSE_VIOLENCE * 2);
	BASHTIME(vict) = 3;
        improve_skill(ch, SKILL_KICKFLIP);
      }
  }
}


ACMD(do_whirlwind)
{
  struct char_data *tch, *next_tch;
  int percent, prob, x = 0;

  /* If player is a mob or skill isn't learned, we can't do this. */
  if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_WHIRLWIND)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  /* Neither can we do this in a peaceful room */
  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL)) {
    send_to_char("No, not in here...\r\n", ch);
    return;
  }

  /* And finally, the show costs 30 moves, so the player must be able to pay */
  if (GET_MOVE(ch) < 100) {
    send_to_char("You don't have the energy to do that just now!\r\n", ch);
    return;
  }

  /* Now we just need to calculate the chance for sucess before we begin. */
  percent = number(1, 111);	            /* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_WHIRLWIND);

  if (percent > prob) {
    send_to_char("You fail to complete your whirlwind!\r\n", ch);
	WAIT_STATE(ch, PULSE_VIOLENCE);
    return;
  } else

  /* Find first target, hit it, then move on to next one */
  for (tch = world[ch->in_room].people; tch; tch = next_tch) {
    next_tch = tch->next_in_room;

  /*  We'll leave out immortals, players (for !pk muds) and pets from the
   *  hit list
   */
    if (tch == ch)
      continue;
    if (!IS_NPC(tch) && GET_LEVEL(tch) >= LVL_IMMORT)
      continue;
    if (!pk_allowed && !IS_NPC(ch) && !IS_NPC(tch))
      continue;
    if (!IS_NPC(ch) && IS_NPC(tch) && AFF_FLAGGED(tch, AFF_CHARM))
      continue;

    /* GET_LEVEL(ch) is the damage value, change if you like */
    if(damage(ch, tch, GET_LEVEL(ch), SKILL_WHIRLWIND)) {
      x = TRUE;
      GET_MOVE(ch) -= 10;
    }
   }

   if (x) {
     improve_skill(ch, SKILL_WHIRLWIND);
     WAIT_STATE(ch, PULSE_VIOLENCE);
   } else
     send_to_char("You can not whirlwind without targets.\r\n", ch);
 }

ACMD(do_shuriken)
{
  struct char_data *tch, *next_tch;
  struct obj_data *obj;
  int percent, prob, x = 0;
  int dam = 1;
  byte sh_type = 0;

  if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_SHURIKEN)) {
    	send_to_char("You have no idea how.\r\n", ch);
    	return;
  }

  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL)) {
    	send_to_char("Not here.\r\n", ch);
    	return;
  }

  if (GET_MOVE(ch) < 80) {
    send_to_char("You don't have the energy to do that just now!\r\n", ch);
    return;
  }

  for (obj = ch->carrying; (obj) && (sh_type == 0); obj = obj->next_content) {
	if (GET_OBJ_VNUM(obj) == 51) {
		sh_type = 1;
		obj_from_char(obj);
		extract_obj(obj);
	} else if (GET_OBJ_VNUM(obj) == 52) {
		sh_type = 2;
		obj_from_char(obj);
		extract_obj(obj);
	}
  }

  if (!sh_type) {
	send_to_char("You don't have any shurikens to throw.\r\n", ch);
	return;
  }

  percent = number(1, 101); // 101 = falha
  prob = GET_SKILL(ch, SKILL_SHURIKEN);

  if (percent > prob) {
    	send_to_char("You throw your shurikens away, missing the targets!\r\n", ch);
	WAIT_STATE(ch, PULSE_VIOLENCE*3);
	improve_skill(ch, SKILL_SHURIKEN);
    	return;
  }
 
  for (tch = world[ch->in_room].people; tch; tch = next_tch) {
	next_tch = tch->next_in_room;
	
	if (tch == ch)
		continue;
	if (!IS_NPC(tch) && GET_LEVEL(tch) >= LVL_IMMORT)
		continue;
	if (!pk_allowed && !IS_NPC(ch) && !IS_NPC(tch))
		continue;
	if (!IS_NPC(ch) && IS_NPC(tch) && AFF_FLAGGED(tch, AFF_CHARM))
		continue;
	if (!IS_NPC(tch) && !can_pk(ch, tch))
		continue;
 
	// Um alvo
	x += 1;
	GET_MOVE(ch) -= x*5;
			
	if ((IS_NPC(tch)) && (!FIGHTING(tch)))
		hit(tch, ch, TYPE_UNDEFINED);

	if (GET_DEX(ch) > number(0, (GET_AC(tch)/14))) {
		if (sh_type == 1) {
			dam = GET_HIT(tch) * ((float) 9 / 100.0f);
			act("&GYour shurikens strike &C$N&G!&n", FALSE, ch, 0, tch, TO_CHAR);
			sprintf(buf, "&G...&R%d&C points of hit damage done&C!&n\r\n", dam);
			send_to_char(buf, ch);

			GET_HIT(tch) -= dam;
			if (GET_HIT(tch) < 1)
				GET_HIT(tch) = 1;

		} else if (sh_type == 2) {
			dam = GET_MANA(tch) * ((float) 15 / 100.0f);
			act("&GYour shurikens strike &C$N&G!&n", FALSE, ch, 0, tch, TO_CHAR);
			sprintf(buf, "&G...&M%d&C points of mana damage done&C!&n\r\n", dam);
			send_to_char(buf, ch);
			
			GET_MANA(tch) -= dam;
			if (GET_MANA(tch) < 1)
				GET_MANA(tch) = 1;
		}

		act("&C$n's&R shurikens strikes you!&n", FALSE, ch, 0, tch, TO_VICT);	
		act("$n shurikens strikes $N!", FALSE, ch, 0, tch, TO_NOTVICT | TO_ROOM);
	} else {
		act("Your shurikens miss $N!", FALSE, ch, 0, tch, TO_CHAR);	
		act("$n's shurikens miss you!", FALSE, ch, 0, tch, TO_VICT);	
		act("$n shurikens miss $N!", FALSE, ch, 0, tch, TO_NOTVICT | TO_ROOM);
	}

	if (GET_MOVE(ch) <= 10)
	break;
   }


   if (x) {
	improve_skill(ch, SKILL_SHURIKEN);
	WAIT_STATE(ch, PULSE_VIOLENCE * 3);
   } else {
	send_to_char("You throw your shurikens.. but there was no one to hit.\r\n", ch);
   }
}


/*
ACMD(do_slang)
{
  struct char_data *vict;
  int percent, prob;
  sh_int x;

  x = (!IS_NPC(ch) ? GET_SKILL(ch, SKILL_SLANG) : 0);

  if (!x) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }
  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch))) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Slang who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Oh yes, veeery smart.\r\n", ch);
    return;
  }
  percent = ((15 - (GET_AC(vict) / 10)) * 2) + number(1, 101);  // 101% is a complete
                                                                 * failure
  prob = x;

  if (AWAKE(vict) && percent > prob) {
    damage(ch, vict, 0, SKILL_SLANG);
  } else
    damage(ch, vict, GET_LEVEL(ch) / 2, SKILL_SLANG);
    improve_skill(ch, SKILL_SLANG);

  WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}


*/






