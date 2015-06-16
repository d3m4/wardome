/* ************************************************************************
*   File: act.comm.c                                    Part of CircleMUD *
*  Usage: Player-level communication commands                             *
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
#include "screen.h"
#include "dg_scripts.h"
#include "buffer.h"

/* extern variables */
extern int level_can_shout;
extern int holler_move_cost;
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;



/* local functions */
void perform_tell(struct char_data *ch, struct char_data *vict, char *arg);
int is_tell_ok(struct char_data *ch, struct char_data *vict);
ACMD(do_say);
ACMD(do_gsay);
ACMD(do_tell);
ACMD(do_reply);
ACMD(do_spec_comm);
ACMD(do_write);
ACMD(do_page);
ACMD(do_gen_comm);
ACMD(do_qcomm);
ACMD(do_ctell);
ACMD(do_newbie);
ACMD(do_langchange);
void garble_text(char *string, int percent);

/* Function to include racial speaking 'sludder' to a sentence string */
void add_racial_sentence(struct char_data *ch, char *sent)
{
  char *ptr, *tmp, new[MAX_INPUT_LENGTH], add[16];
  int i = 0, len;

  if(IS_NPC(ch))
    return;

  if (GET_RACE(ch) == RACE_OGRE)
    strcpy(add, " (grunt) ");
  else if (GET_RACE(ch) == RACE_TROLL)
    strcpy(add, " (grunt) ");
  else if (GET_RACE(ch) == RACE_GITH)
    strcpy(add, " (hiss) ");
  else if (GET_RACE(ch) == RACE_LIZARDMAN)
    strcpy(add, " (hiss) ");
  else if (GET_RACE(ch) == RACE_SEA_ELF)
    strcpy(add, " (glup) ");
  else
    return;

  *new = '\0';
  ptr = sent;
  tmp = new;

  while (*ptr != '\0' && i < 240) {
    if (*ptr == ' ' && !number(0, 10)) {
      strcpy(tmp, add);
      len = strlen(add)-1;
      tmp += len;
      i += len;
    } else
      *tmp = *ptr;

    ptr++;
    tmp++;
    i++;
  }
  *tmp = '\0';

  strcpy(sent, new);
}

/*
ACMD(do_say)
{
  skip_spaces(&argument);

  if (!*argument)
    send_to_char("Yes, but WHAT do you want to say?\r\n", ch);
  else {
    sprintf(buf, "$n says, '%s'", argument);
    act(buf, FALSE, ch, 0, 0, TO_ROOM|DG_NO_TRIG);

    if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else {
      sprintf(buf, "You say, '%s'", argument);
      act(buf, FALSE, ch, 0, argument, TO_CHAR);
    }
  }
  speech_mtrigger(ch, argument);
  speech_wtrigger(ch, argument);
}
*/

ACMD(do_say)
{
  char buf2[MAX_STRING_LENGTH];
  skip_spaces(&argument);

  if (!*argument)
  {
    send_to_char("Yes, but WHAT do you want to say?\r\n", ch);
    return;
  }

  if (argument[strlen(argument) - 1] == '?')
  {
    add_racial_sentence(ch, argument);
    sprintf(buf, "You ask, '%s&n'", argument);
    sprintf(buf2, "$U$n asks, '%s&n'", argument);
  }
  else if(argument[strlen(argument) - 1] == '!')
  {
    add_racial_sentence(ch, argument);
    sprintf(buf, "You exclaim, '%s&n'", argument);
    sprintf(buf2, "$U$n exclaims, '%s&n'", argument);
  }
  else if (argument[strlen(argument) - 1] == '.' &&
           argument[strlen(argument) - 2] == '.' &&
           argument[strlen(argument) - 3] == '.')
  {
    add_racial_sentence(ch, argument);
    sprintf(buf, "You mutter, '%s&n'", argument);
    sprintf(buf2, "$U$n mutters, '%s&n'", argument);
  }
  else
  {
    add_racial_sentence(ch, argument);
    sprintf(buf, "You say, '%s&n'", argument);
    sprintf(buf2, "$U$n says, '%s&n'", argument);
  }

  if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOREPEAT))
  send_to_char(OK, ch);
  else
  {
    act(buf, FALSE, ch, 0, 0, TO_CHAR);
  }

  act(buf2, FALSE, ch, 0, argument, TO_ROOM|DG_NO_TRIG);

  /* Remove the next three lines if you don't have dg_scripts. */
  /* trigger check */
  speech_mtrigger(ch, argument);
  speech_wtrigger(ch, argument);
}

ACMD(do_gsay)
{
  struct char_data *k;
  struct follow_type *f;

  skip_spaces(&argument);

  if (!AFF_FLAGGED(ch, AFF_GROUP)) {
    send_to_char("But you are not the member of a group!\r\n", ch);
    return;
  }
  if (!*argument)
    send_to_char("Yes, but WHAT do you want to group-say?\r\n", ch);
  else {
    if (ch->master)
      k = ch->master;
    else
      k = ch;

    add_racial_sentence(ch, argument);
    sprintf(buf, "&R$U$n &Wtells the group, &w'%s'&n", argument);

    if (AFF_FLAGGED(k, AFF_GROUP) && (k != ch))
      act(buf, FALSE, ch, 0, k, TO_VICT | TO_SLEEP);
    for (f = k->followers; f; f = f->next)
      if (AFF_FLAGGED(f->follower, AFF_GROUP) && (f->follower != ch))
        act(buf, FALSE, ch, 0, f->follower, TO_VICT | TO_SLEEP);

    if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else {
      sprintf(buf, "&WYou tell the group, &w'%s'&n", argument);
      act(buf, FALSE, ch, 0, 0, TO_CHAR | TO_SLEEP);
    }
  }
}


void perform_tell(struct char_data *ch, struct char_data *vict, char *arg)
{
  add_racial_sentence(ch, arg);
  sprintf(buf, "&g$U$n tells you, '%s&y'", arg);
  act(buf, FALSE, ch, 0, vict, TO_VICT | TO_SLEEP);

  if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char(OK, ch);
  else {
    sprintf(buf, "&gYou tell $N, '%s&y'", arg);
    act(buf, FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  }

  if (!IS_NPC(vict) && !IS_NPC(ch))
   if(!PRF2_FLAGGED(ch, PRF2_NOREPEAT))
    GET_LAST_TELL(vict) = GET_IDNUM(ch);
}

int is_tell_ok(struct char_data *ch, struct char_data *vict)
{
  if (ch == vict)
    send_to_char("You try to tell yourself something.\r\n", ch);
  else if (IS_NPC(vict))
    send_to_char("You can't tell mobs.\r\n", ch);
  else if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOTELL))
    send_to_char("You can't tell other people while you have notell on.\r\n", ch);
  else if (!IS_NPC(vict) && PRF_FLAGGED(vict, PRF_AFK))
    send_to_char("You can't tell other people while they are AFK.\r\n", ch);
  else if (ROOM_FLAGGED(ch->in_room, ROOM_SOUNDPROOF))
    send_to_char("The walls seem to absorb your words.\r\n", ch);
  else if (!IS_NPC(vict) && !vict->desc)        /* linkless */
    act("$E's linkless at the moment.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  else if (PLR_FLAGGED(vict, PLR_WRITING))
    act("$E's writing a message right now; try again later.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  else if ((!IS_NPC(vict) && PRF_FLAGGED(vict, PRF_NOTELL)) || ROOM_FLAGGED(vict->in_room, ROOM_SOUNDPROOF))
    act("$E can't hear you.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  else
    return (TRUE);

  return (FALSE);
}

/*
 * Yes, do_tell probably could be combined with whisper and ask, but
 * called frequently, and should IMHO be kept as tight as possible.
 */
ACMD(do_tell)
{
  struct char_data *vict = NULL;

  half_chop(argument, buf, buf2);

  if (!*buf || !*buf2)
    send_to_char("Who do you wish to tell what??\r\n", ch);
  else if (GET_LEVEL(ch) < LVL_IMMORT && !(vict = get_player_vis(ch, buf, 0)))
    send_to_char(NOPERSON, ch);
  else if (GET_LEVEL(ch) >= LVL_IMMORT && !(vict = get_char_vis(ch, buf)))
    send_to_char(NOPERSON, ch);
  else if (is_tell_ok(ch, vict))
    perform_tell(ch, vict, buf2);
}


ACMD(do_reply)
{
  struct char_data *tch = character_list;

  if (IS_NPC(ch))
    return;

  skip_spaces(&argument);

  if (GET_LAST_TELL(ch) == NOBODY)
    send_to_char("You have no-one to reply to!\r\n", ch);
  else if (!*argument)
    send_to_char("What is your reply?\r\n", ch);
  else {
    /*
     * Make sure the person you're replying to is still playing by searching
     * for them.  Note, now last tell is stored as player IDnum instead of
     * a pointer, which is much better because it's safer, plus will still
     * work if someone logs out and back in again.
     */

    /*
     * XXX: A descriptor list based search would be faster although
     *      we could not find link dead people.  Not that they can
     *      hear tells anyway. :) -gg 2/24/98
     */
    while (tch != NULL && (IS_NPC(tch) || GET_IDNUM(tch) != GET_LAST_TELL(ch)))
      tch = tch->next;

    if (tch == NULL)
      send_to_char("They are no longer playing.\r\n", ch);
    else if (is_tell_ok(ch, tch))
      perform_tell(ch, tch, argument);
  }
}


ACMD(do_spec_comm)
{
  struct char_data *vict;
  const char *action_sing, *action_plur, *action_others;

  switch (subcmd) {
  case SCMD_WHISPER:
    action_sing = "whisper to";
    action_plur = "whispers to";
    action_others = "$n whispers something to $N.";
    break;

  case SCMD_ASK:
    action_sing = "ask";
    action_plur = "asks";
    action_others = "$n asks $N a question.";
    break;

  default:
    action_sing = "oops";
    action_plur = "oopses";
    action_others = "$n is tongue-tied trying to speak with $N.";
    break;
  }

  half_chop(argument, buf, buf2);

  if (!*buf || !*buf2) {
    sprintf(buf, "Whom do you want to %s.. and what??\r\n", action_sing);
    send_to_char(buf, ch);
  } else if (!(vict = get_char_room_vis(ch, buf)))
    send_to_char(NOPERSON, ch);
  else if (vict == ch)
    send_to_char("You can't get your mouth close enough to your ear...\r\n", ch);
  else {
    sprintf(buf, "$n %s you, '%s'", action_plur, buf2);
    act(buf, FALSE, ch, 0, vict, TO_VICT);
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else {
      sprintf(buf, "You %s %s, '%s'\r\n", action_sing, GET_NAME(vict), buf2);
      send_to_char(buf, ch);
    }
    act(action_others, FALSE, ch, 0, vict, TO_NOTVICT);
  }
}



#define MAX_NOTE_LENGTH 1000    /* arbitrary */

ACMD(do_write)
{
  struct obj_data *paper, *pen = NULL;
  char *papername, *penname;

  papername = buf1;
  penname = buf2;

  two_arguments(argument, papername, penname);

  if (!ch->desc)
    return;

  if (!*papername) {            /* nothing was delivered */
    send_to_char("Write?  With what?  ON what?  What are you trying to do?!?\r\n", ch);
    return;
  }
  if (*penname) {               /* there were two arguments */
    if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying))) {
      sprintf(buf, "You have no %s.\r\n", papername);
      send_to_char(buf, ch);
      return;
    }
    if (!(pen = get_obj_in_list_vis(ch, penname, ch->carrying))) {
      sprintf(buf, "You have no %s.\r\n", penname);
      send_to_char(buf, ch);
      return;
    }
  } else {              /* there was one arg.. let's see what we can find */
    if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying))) {
      sprintf(buf, "There is no %s in your inventory.\r\n", papername);
      send_to_char(buf, ch);
      return;
    }
    if (GET_OBJ_TYPE(paper) == ITEM_PEN) {      /* oops, a pen.. */
      pen = paper;
      paper = NULL;
    } else if (GET_OBJ_TYPE(paper) != ITEM_NOTE) {
      send_to_char("That thing has nothing to do with writing.\r\n", ch);
      return;
    }
    /* One object was found.. now for the other one. */
    if (!GET_EQ(ch, WEAR_HOLD)) {
      sprintf(buf, "You can't write with %s %s alone.\r\n", AN(papername),
              papername);
      send_to_char(buf, ch);
      return;
    }
    if (!CAN_SEE_OBJ(ch, GET_EQ(ch, WEAR_HOLD))) {
      send_to_char("The stuff in your hand is invisible!  Yeech!!\r\n", ch);
      return;
    }
    if (pen)
      paper = GET_EQ(ch, WEAR_HOLD);
    else
      pen = GET_EQ(ch, WEAR_HOLD);
  }


  /* ok.. now let's see what kind of stuff we've found */
  if (GET_OBJ_TYPE(pen) != ITEM_PEN)
    act("$p is no good for writing with.", FALSE, ch, pen, 0, TO_CHAR);
  else if (GET_OBJ_TYPE(paper) != ITEM_NOTE)
    act("You can't write on $p.", FALSE, ch, paper, 0, TO_CHAR);
  else if (paper->action_description)
    send_to_char("There's something written on it already.\r\n", ch);
  else {
    /* we can write - hooray! */
     /* this is the PERFECT code example of how to set up:
      * a) the text editor with a message already loaed
      * b) the abort buffer if the player aborts the message
      */
     ch->desc->backstr = NULL;
     send_to_char("Write your note.  (/s saves /h for help)\r\n", ch);
     /* ok, here we check for a message ALREADY on the paper */
     if (paper->action_description) {
        /* we str_dup the original text to the descriptors->backstr */
        ch->desc->backstr = str_dup(paper->action_description);
        /* send to the player what was on the paper (cause this is already */
        /* loaded into the editor) */
        send_to_char(paper->action_description, ch);
     }
    act("$n begins to jot down a note.", TRUE, ch, 0, 0, TO_ROOM);
     /* assign the descriptor's->str the value of the pointer to the text */
     /* pointer so that we can reallocate as needed (hopefully that made */
     /* sense :>) */
    ch->desc->str = &paper->action_description;
    ch->desc->max_str = MAX_NOTE_LENGTH;
  }
}



ACMD(do_page)
{
  struct descriptor_data *d;
  struct char_data *vict;

  half_chop(argument, arg, buf2);
  
  if (IS_NPC(ch))
    send_to_char("Monsters can't page.. go away.\r\n", ch);
  else if (!*arg)
    send_to_char("Whom do you wish to page?\r\n", ch);
  else {
    if (!GET_INVIS_LEV(ch))
    sprintf(buf, "\007\007*%s* %s\r\n", GET_NAME(ch), buf2);
    if (GET_INVIS_LEV(ch))
    sprintf(buf, "\007\007*Someone* %s\r\n", buf2);
    if (!str_cmp(arg, "all")) {
      if (GET_LEVEL(ch) > LVL_GOD) {
        for (d = descriptor_list; d; d = d->next)
          if (STATE(d) == CON_PLAYING && d->character)
            act(buf, FALSE, ch, 0, d->character, TO_VICT);
      } else
        send_to_char("You will never be godly enough to do that!\r\n", ch);
      return;
    }
    
    if ((vict = get_char_vis(ch, arg)) != NULL) {
      act(buf, FALSE, ch, 0, vict, TO_VICT);
      if (PRF_FLAGGED(ch, PRF_NOREPEAT))
        send_to_char(OK, ch);
      else
        act(buf, FALSE, ch, 0, vict, TO_CHAR);
    } else
      send_to_char("There is no such person in the game!\r\n", ch);
  }
}


/**********************************************************************
 * generalized communication func, originally by Fred C. Merkel (Torg) *
  *********************************************************************/

ACMD(do_gen_comm)
{
  struct descriptor_data *i;
  char color_on[24];
  byte gmote = FALSE;

  /* Array of flags which must _not_ be set in order for comm to be heard */
  int channels[][2] = {
    { PRF2_NOHOLLER, 2},
    { PRF_DEAF, 1},
    { PRF_NOGOSS, 1},
    { PRF_NOAUCT, 1},
    { PRF_NOGRATZ, 1},
    { 0, 1},
    { PRF_NOCHAT, 1},
    { PRF2_NONEWBIE, 2},  
    { 0, 0 },
  };

  /*
   * com_msgs: [0] Message if you can't perform the action because of noshout
   *           [1] name of the action
   *           [2] message if you're not on the channel
   *           [3] a color string.
   */
  const char *com_msgs[][4] = {
    {"You cannot ooc!!\r\n",
      "OOC",
      "",
    KBGRN},

    {"You cannot shout!!\r\n",
      "shout",
      "Turn off your noshout flag first!\r\n",
    KYEL},

    {"You cannot gossip!!\r\n",
      "gossip",
      "You aren't even on the channel!\r\n",
    KCYN},

    {"You cannot auction!!\r\n",
      "auction",
      "You aren't even on the channel!\r\n",
    KMAG},

    {"You cannot congratulate!\r\n",
      "congrat",
      "You aren't even on the channel!\r\n",
    KGRN},

    {"You cannot gemote!!\r\n",
     "gemote",
     "",
    KYEL},

    { "You cannot chat!\n\r",
       "chat",
       "You aren't even on the chat channel!\n\r",
    KBWHT},

    { "You cannot newbie chat!\n\r",
      "newbie chat",
      "You aren't even on the newbie chat channel!\n\r",
      KMAG},
  };

  ACMD(do_gmote);

  /* to keep pets, etc from being ordered to shout */
  if (!ch->desc)
    return;

  if (PLR_FLAGGED(ch, PLR_NOSHOUT)) {
    send_to_char(com_msgs[subcmd][0], ch);
    return;
  }
  if (ROOM_FLAGGED(ch->in_room, ROOM_SOUNDPROOF)) {
    send_to_char("The walls seem to absorb your words.\r\n", ch);
    return;
  }
  /* level_can_shout defined in config.c */
  if (GET_LEVEL(ch) < level_can_shout) {
    sprintf(buf1, "You must be at least level %d before you can %s.\r\n",
            level_can_shout, com_msgs[subcmd][1]);
    send_to_char(buf1, ch);
    return;
  }
  /* make sure the char is on the channel */
  if (!IS_NPC(ch) && (channels[subcmd][1] == 1 ? PRF_FLAGGED(ch, channels[subcmd][0]) : PRF2_FLAGGED(ch, channels[subcmd][0]))) {
    send_to_char(com_msgs[subcmd][2], ch);
    return;
  }

  /* skip leading spaces */
  skip_spaces(&argument);
  
    
  if(subcmd == SCMD_GMOTE || (subcmd == SCMD_GOSSIP && *argument == '@')) {
    subcmd = SCMD_GOSSIP;
    gmote = TRUE;
  }

  /* make sure that there is something there to say! */
  if (!*argument) {
    sprintf(buf1, "Yes, %s, fine, %s we must, but WHAT???\r\n",
            com_msgs[subcmd][1], com_msgs[subcmd][1]);
    send_to_char(buf1, ch);
    return;
  }

  if (gmote) {
    if (*argument == '@')
      do_gmote(ch, argument + 1, 0, 1);
    else
      do_gmote(ch, argument, 0, 1);
    return;
  }

  /* verify if you *can* do it */
  if (subcmd == SCMD_HOLLER) {
    if (GET_MOVE(ch) < holler_move_cost) {
      send_to_char("You're too exhausted to holler.\r\n", ch);
      return;
    } else
      GET_MOVE(ch) -= holler_move_cost;
  }

  /* set up the color on code */
  strcpy(color_on, com_msgs[subcmd][3]);

  add_racial_sentence(ch, argument);

  /* first, set up strings to be given to the communicator */
  if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char(OK, ch);
  else {
    if (COLOR_LEV(ch) >= C_CMP)
      sprintf(buf1, "%sYou %s, '%s%s'%s", color_on, com_msgs[subcmd][1],
              argument, color_on, KNRM);
    else
      sprintf(buf1, "You %s, '%s'", com_msgs[subcmd][1], argument);
    act(buf1, FALSE, ch, 0, 0, TO_CHAR | TO_SLEEP);
  }


  sprintf(buf, "$n %ss, '%s%s'", com_msgs[subcmd][1], argument, color_on);
	
if (subcmd != SCMD_HOLLER) {  

	 if (strstr(argument, "merda") || strstr(argument, "m3rda") || strstr(argument, "merd@") || strstr(argument, "m3rd@")
	 || (strstr(argument, " porra ") || strstr(argument, "porr@") || !strcmp(argument, "porra"))
	 || (strstr(argument, "caralho") || (strstr(argument, "c@ralho")))
	 || (strstr(argument, "buceta") || strstr(argument, "bucet@"))
	 || (strstr(argument, " puto ") || strstr(argument, " put@") || strstr(argument, " puta ") )
	 || (strstr(argument, "m.e.r.d.a"))
	 || (strstr(argument, "i3")) || (strstr(argument, "hollow")) || (strstr(argument, "elite")) 
	 || (strstr(argument, "vitalia"))){
	send_to_char("\r\n&WVoce precisa de mais educacao! Agora voce esta' mudo.\r\n", ch);
	PLR_TOG_CHK(ch, PLR_NOSHOUT);
	log("\r\n&g(GC) %s foi mal educado e levou mute.&n",
		GET_NAME(ch));

}
}



  /* now send all the strings out */
  for (i = descriptor_list; i; i = i->next) {
    if (STATE(i) == CON_PLAYING && i != ch->desc && i->character &&
        !IS_NPC(i->character) && !(channels[subcmd][1] == 1 ? PRF_FLAGGED(i->character, channels[subcmd][0]) : PRF2_FLAGGED(i->character, channels[subcmd][0])) &&
        !PLR_FLAGGED(i->character, PLR_WRITING) &&
        !ROOM_FLAGGED(i->character->in_room, ROOM_SOUNDPROOF)) {

      if (subcmd == SCMD_SHOUT &&
          ((world[ch->in_room].zone != world[i->character->in_room].zone) ||
           GET_POS(i->character) < POS_RESTING))
        continue;

      if (COLOR_LEV(i->character) >= C_NRM)
        send_to_char(color_on, i->character);
      act(buf, FALSE, ch, 0, i->character, TO_VICT | TO_SLEEP);
      if (COLOR_LEV(i->character) >= C_NRM)
        send_to_char(KNRM, i->character);
    }
  }
}


ACMD(do_qcomm)

{

  struct descriptor_data *i;

 // if (IS_NPC(ch)) /*adicionado por ultimo pra tentar resolver bug*/
 //   return;

  if (PLR_FLAGGED(ch, PLR_SECPLAYER)){
  	send_to_char("Second players can't do part of a quest!\r\n", ch);
    	return;
	}
  if(PLR_FLAGGED(ch, PLR_NOSHOUT)) {
    send_to_char("You can't do this while you can't talk!\r\n", ch);
    return;
}
  if (!PRF_FLAGGED(ch, PRF_QUEST)) {
    send_to_char("You aren't even part of the quest!\r\n", ch);
    return;
  }
  skip_spaces(&argument);

  if (!*argument) {
    sprintf(buf, "%s?  Yes, fine, %s we must, but WHAT??\r\n", CMD_NAME,
            CMD_NAME);
    CAP(buf);
    send_to_char(buf, ch);
  } else {
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else {
      if (subcmd == SCMD_QSAY)
        sprintf(buf, "&WYou quest-say, &w'%s'&n", argument);
      else
        strcpy(buf, argument);
      act(buf, FALSE, ch, 0, argument, TO_CHAR);
    }

    if (subcmd == SCMD_QSAY)
      sprintf(buf, "$n quest-says, '%s'", argument);
    else
      strcpy(buf, argument);

    for (i = descriptor_list; i; i = i->next)
            if (STATE(i) == CON_PLAYING && i != ch->desc &&
          PRF_FLAGGED(i->character, PRF_QUEST))
        act(buf, 0, ch, 0, i->character, TO_VICT | TO_SLEEP);
  }
}

void garble_text(char *string, int percent)
{
  char letters[] = "aeiousthpwxyz";
  int i;

  for (i = 0; i < strlen(string); ++i)
    if (isalpha(string[i]) && number(0, 1) && number(0, 100) > percent)
      string[i] = letters[number(0, 12)];
}

ACMD(do_langchange)
{
	char l[128];
	one_argument(argument, l);

	if (!is_abbrev(l, "port")) {
		if (GET_LANGUAGE(ch) == FALSE) {
			send_to_char("Sua lingua ja' esta portugues.\r\n", ch);
			return;
		}
		GET_LANGUAGE(ch) = FALSE;
	} else if (!is_abbrev(l, "ing")) {
		if (GET_LANGUAGE(ch) == TRUE) {
			send_to_char("Sua lingua ja' esta ingles.\r\n", ch);
			return;
		}
		GET_LANGUAGE(ch) = TRUE;
	} else
		send_to_char("Sintaxe: language <ingles/portugues>\r\n", ch);
}







