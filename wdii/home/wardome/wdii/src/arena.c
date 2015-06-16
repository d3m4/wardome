/**************************
 *
 * File: Arena.c
 *
 * Writen by:  Kevin Hoogheem aka Goon
 *             Modified by Billy H. Chan (STROM)
 *
 * Implementation of a event driven arena.. were players pay to kill.
 *
 * Using this code without consent by Goon will make your guts spill
 * out or worse.. Maybe I will hire Lauraina Bobbet to come visit you
 *
 */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "screen.h"
#include "spells.h"
#include "handler.h"
#include "interpreter.h"
#include "db.h"
#include "constants.h"
#include "buffer.h"

/*   external vars  */
extern FILE *player_fl;
extern struct room_data *world;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct zone_data *zone_table;
extern int top_of_zone_table;
extern int restrict;
extern int top_of_world;
extern int top_of_mobt;
extern int top_of_objt;
extern int top_of_p_table;
extern sh_int r_mortal_start_room;
struct hall_of_fame_element *fame_list = NULL;
int in_arena = ARENA_OFF;
int start_time;
int game_length;
int lo_lim;
int hi_lim;
int lo_rem;
int hi_rem;
int cost_per_lev;
int time_to_start;
int time_left_in_game;
long arena_pot;
int bet_pot;



ACMD(do_arena)
{ 

if (PLR_FLAGGED(ch, PLR_SECPLAYER)){
		send_to_char("Second players can't enter in arenas.\r\n", ch);
		return;
    	}
 if (IS_NPC(ch)){
   send_to_char("Mobs can't enjoy the &RWar&n.\r\n",ch);
   return;
 }
 if (in_arena == ARENA_OFF)
 {
   send_to_char("The killing fields are closed right now.\r\n", ch);
 } else if (GET_LEVEL(ch) < lo_lim)
 {
   sprintf(buf, "Sorry but you must be at least level &M%d&n to enjoy the &RWar&n.\r\n",lo_lim);
   send_to_char(buf, ch);
 } else if (GET_REMORT(ch) < lo_rem)
 {
   sprintf(buf, "Sorry but you must be at least remort &M%d&n to enjoy the &RWar&n.\n\r", lo_rem);
    send_to_char(buf, ch);
 } else if (PLR_FLAGGED(ch, PLR_DEAD))
 {
    send_to_char("You are dead...\r\n", ch);
 } else if ((GET_LEVEL(ch) > hi_lim) || (GET_REMORT(ch) > hi_rem))
 {
    send_to_char("Sorry the killing fields are not open to you.\r\n",ch);
 } else if (GET_GOLD(ch) < (cost_per_lev * GET_LEVEL(ch)))
 {
    sprintf(buf, "Sorry but you need &Y%d&n coins to enjoy the &RWar&n.\r\n",
           (cost_per_lev * GET_LEVEL(ch)) );
    send_to_char(buf, ch);
 } else if (GET_LEVEL(ch) >= LVL_IMMORT)
 {
     send_to_char("Immortals can't join the &RWar&n.\r\n",ch);
 } else if (in_arena == ARENA_RUNNING)
 {
    send_to_char("It's too late to join the rumble now.  Wait for the next &RWar&n.\r\n", ch);
 } else if (AFF_FLAGGED(ch, AFF_GROUP))
 {
    send_to_char("When you are grouped you can't join the &RWar&n..\r\n", ch);
 } else if(ROOM_FLAGGED(IN_ROOM(ch), ROOM_ARENA))
 {
     send_to_char("You are in the &RWar&n already.\r\n",ch);
 } else
 {
   act("&C$n&n has been whisked away to the killing fields.\r\n", FALSE, ch, 0, 0, TO_ROOM);
   char_from_room(ch);
   char_to_room(ch, real_room(7500));
   act("&C$n&n is droped from the sky.", FALSE, ch, 0, 0, TO_ROOM);
   send_to_char("You have been taken to the killing fields.\r\n",ch);
   look_at_room(ch, 0);
   sprintf(buf, "&C%s&n has entered the &RWar&n.\r\n", GET_NAME(ch));
   sportschan(buf);
   GET_GOLD(ch) -= (cost_per_lev * GET_LEVEL(ch));
   arena_pot += (cost_per_lev * GET_LEVEL(ch));
   sprintf(buf, "You pay &Y%s&n coins to enjoy the &RWar&n.\r\n",
          add_points((cost_per_lev * GET_LEVEL(ch))));
   send_to_char(buf, ch);
  /* ok lets give them there free restore and take away all their */
  /* effects so they have to recast them spells onthemselfs       */
   ch->desc->hp = GET_HIT(ch);
   ch->desc->mana = GET_MANA(ch);
   ch->desc->move = GET_MOVE(ch);
   GET_HIT(ch) = GET_MAX_HIT(ch);
   GET_MANA(ch) = GET_MAX_MANA(ch);
   GET_MOVE(ch) = GET_MAX_MOVE(ch);
   GET_MENTAL(ch) = GET_MAX_MENTAL(ch);
   GET_OXI(ch) = GET_MAX_OXI(ch);
  if (ch->affected)
    while (ch->affected)
      affect_remove(ch, ch->affected);
  }
}


ACMD(do_chaos)
{
  char cost[MAX_INPUT_LENGTH], lolimit[MAX_INPUT_LENGTH];
  char hilimit[MAX_INPUT_LENGTH], start_delay[MAX_INPUT_LENGTH];
  char length[MAX_INPUT_LENGTH], lorem[MAX_INPUT_LENGTH];
  char hirem[MAX_INPUT_LENGTH];

/*Usage: chaos lo hi Rem Lo, Rem High start_delay cost/lev length*/

  if (in_arena != ARENA_OFF) {
	send_to_char("There is a &Rwar&n running already.\r\n", ch);
	return;
  }
half_chop(argument, lolimit, buf);
  lo_lim = atoi(lolimit);

  half_chop(buf, hilimit, buf);
  hi_lim = atoi(hilimit);

  half_chop(buf, start_delay, buf);
  start_time = atoi(start_delay);

  half_chop(buf, cost, buf);
  cost_per_lev = atoi(cost);

  half_chop(buf,length, buf);
  game_length = atoi(length);

  half_chop(buf, lorem, buf);
  lo_rem = atoi(lorem);

  half_chop(buf, hirem, buf);
  hi_rem = atoi(hirem);

  if (hi_lim > LVL_IMPL )
  {
    send_to_char("Please choose a <max level> under the Imps level.\r\n", ch);
    return;
  }

  if (lolimit < 0)
   silent_end();


  if(!*lolimit || !*hilimit || !*start_delay || !*cost || !*length || !*lorem || !*hirem)
  {
    send_to_char("Usage: chaos <low level> <max level> start_delay cost/lev length <min remort> <max remort>\r\n", ch);
    return;
  }
  if (!*lorem || !*hirem)
  {
  lo_rem = 0;
  hi_rem = 12;
  }

  if ((lo_lim > hi_lim) || (lo_rem > hi_rem))
  {
    send_to_char("Sorry, low limit must be lower than hi limit.\n\r", ch);
    return;
  }

  if ((hi_lim < 0) || (cost_per_lev < 0) || (game_length < 0) || (lo_rem < 0) || (hi_rem < 0))
  {
    send_to_char("I like positive numbers thank you.\n\r", ch);
    return;
  }

  if (start_time <= 0)
  {
    send_to_char("Lets at least give them a chance to enter!\r\n", ch);
    return;
  }

  if ((GET_LEVEL(ch) < LVL_SUBIMPL) && (cost_per_lev < MIN_ARENA_COST))
  {
    send_to_char("The implementors have put a minimum of 1000 entry fee.\r\n",ch);
    return;
  }

  in_arena = ARENA_START;
  time_to_start = start_time;
  time_left_in_game =0;
  arena_pot =0;
  start_arena();

}

void start_arena()
{

  if(time_to_start == 0){
    show_jack_pot();
    in_arena = ARENA_RUNNING;    /* start the blood shed */
    time_left_in_game = game_length;
    start_game();
  } else {
   if(time_to_start >1){
     sprintf(buf1, "The &RWAR&n will begin in %d hours open to levels: &M%d - &M%d&n and remorts: &M%d - &M%d&n.\r\n",
         time_to_start, lo_lim, hi_lim, lo_rem, hi_rem);
     sportschan(buf1);
     sprintf(buf1, "&Y%s&n coins per level to enter.\r\n",
         add_points(cost_per_lev));
     sportschan(buf1);
     sprintf(buf1, "Type &R'war'&n to enter.\r\n");
     sportschan(buf1);
   }else{
     sprintf(buf1, "The next &RWAR&n will begin in &G1&n hour open to levels: &M%d - &M%d&n and remorts: &M%d - &M%d&n.\r\n",
         lo_lim, hi_lim, lo_rem, hi_rem);
    sportschan(buf1);
    sprintf(buf1, "&Y%s&n coins per level to enter.\r\n",
	    add_points(cost_per_lev));
    sportschan(buf1);
    sprintf(buf1, "Type &R'war'&n to enter.\r\n");
    sportschan(buf1);
   }
   time_to_start--;
  }
}

void show_jack_pot()
{
  sprintf(buf1, "\007\007The &RWar&n begin NOW!!!!!!!!\r\n");
  sportschan(buf1);
  sprintf(buf1, "The &yjackpot&n for this &RWAR&n is &Y%s&n coins.\r\n", add_points(arena_pot));
  sportschan(buf1);

}

void start_game()
{
  struct char_data *i;
  struct descriptor_data *d;

  for (d = descriptor_list; d; d = d->next) {
   if (!d->connected && d->character) {
    i = d->character;
    if (i->in_room == real_room(7500)) {
       send_to_char("\r\nThe floor falls out from bellow, droping you in the middle of the &RWAR&n.\r\n", i);
         char_from_room(i);
         char_to_room(i, real_room(number(7501, 7517)));
         look_at_room(i, 0);
        }
   }
  }
  do_game();
}

void do_game()
{

  if(num_in_arena() == 1){
   in_arena = ARENA_OFF;
   find_game_winner();
  }else if(time_left_in_game == 0){
   in_arena = ARENA_OFF;
   do_end_game();
  }else if (num_in_arena() == 0){
   in_arena = ARENA_OFF;
   silent_end();
  } else if ((time_left_in_game % 5) || time_left_in_game <= 4) {
   sprintf(buf, "With &G%d&n hours left in the game there are &B%d&n players left.\r\n",
            time_left_in_game, num_in_arena());
   sportschan(buf);
  } else if(time_left_in_game == 1){
   sprintf(buf, "With &G1&n hour left in the game there are &B%d&n players left.\r\n",
            num_in_arena());
   sportschan(buf);
  }
  time_left_in_game--;
}

void find_game_winner()
{
  struct char_data *i;
  struct descriptor_data *d;
  struct hall_of_fame_element *fame_node;

  for (d = descriptor_list; d; d = d->next)
    if (!d->connected && d->character)
 {
   i = d->character;
        if (ROOM_FLAGGED(IN_ROOM(i), ROOM_ARENA) && (!IS_NPC(i)) && (i->in_room != NOWHERE) && GET_LEVEL(i) < LVL_ELDER)
  {
      GET_HIT(i) = i->desc->hp;
      GET_MANA(i) = i->desc->mana;
      GET_MOVE(i) = i->desc->move;
      GET_OXI(i) = GET_MAX_OXI(i);
      GET_MENTAL(i) = GET_MAX_MENTAL(i);

      if (i->affected)
          while (i->affected) affect_remove(i, i->affected);
      char_from_room(i);
      char_to_room(i, r_mortal_start_room);
      look_at_room(i, 0);
      act("$n falls from the sky.\r\n", FALSE, i, 0, 0, TO_ROOM);

      if(time_left_in_game == 1)
     {
         sprintf(buf, "After &G1&n hour of battle &C%s&n is declared the winner.\r\n",GET_NAME(i));
         sportschan(buf);

     }
     else
     {
        sprintf(buf, "After &G%d&n hours of battle &C%s&n is declared the winner.\r\n", game_length - time_left_in_game, GET_NAME(i));
        sportschan(buf);
     }

     GET_GOLD(i) += arena_pot;
     GET_TITLE(i) = "The Last Arena Winner";

     sprintf(buf, "You have been awarded &Y%s&n coins for being the best gladiator.\r\n", add_points(arena_pot));
     send_to_char(buf, i);

     sprintf(buf2, "%s has been awarded &Y%s&n coins for being the best gladiator.\r\n", GET_NAME(i), add_points(arena_pot));
     sportschan(buf2);
    if ((game_length - time_left_in_game) > 1)
        {
          CREATE(fame_node, struct hall_of_fame_element, 1);
          strncpy(fame_node->name, GET_NAME(i), MAX_NAME_LENGTH);
          fame_node->name[MAX_NAME_LENGTH] = '\0';
          fame_node->date = time(0);
          fame_node->award = (arena_pot/2);
          fame_node->next = fame_list;
          fame_list = fame_node;
          write_fame_list();
          find_bet_winners(i);
        }
  }
 }
}

void silent_end()
{
  struct descriptor_data *d;
  struct char_data *i;

     in_arena = ARENA_OFF;
     start_time = 0;
     game_length = 0;
     time_to_start = 0;
     time_left_in_game = 0;
     arena_pot = 0;

  sprintf(buf, "It looks like no one was brave enough to enter in the &RWar&n.\r\n\r\n");
  sportschan(buf);

        for (d = descriptor_list; d; d = d->next)
      {
           if (!d->connected && d->character)
          {
            i = d->character;
             if (i->in_room == real_room(7500))
            {

             send_to_char("\r\nThe floor falls out from bellow, droping you in The Wardome Preparation Room.\r\n", i);
             char_from_room(i);
             char_to_room(i, real_room(500));
             look_at_room(i, 0);
            }
          }
      }
}

void do_end_game()
{
  struct char_data *i;
  struct descriptor_data *d;

    for (d = descriptor_list; d; d = d->next)
      if (!d->connected && d->character)
   {
      i = d->character;
         if (ROOM_FLAGGED(IN_ROOM(i), ROOM_ARENA) && (i->in_room != NOWHERE) && (!IS_NPC(i)))
      {
           GET_HIT(i) = GET_MAX_HIT(i);
           GET_MANA(i) = GET_MAX_MANA(i);
           GET_MOVE(i) = GET_MAX_MOVE(i);
           GET_OXI(i) = GET_MAX_OXI(i);
           GET_MENTAL(i) = GET_MAX_MENTAL(i);

         if (i->affected)
           while (i->affected) affect_remove(i, i->affected);
        char_from_room(i);
        char_to_room(i, r_mortal_start_room);
        look_at_room(i, 0);
        act("$n falls from the sky.\r\n", FALSE, i, 0, 0, TO_ROOM);
      }
   }
      sprintf(buf, "After &G%d&n hours of battle the Match is a draw.\r\n",game_length);
      sportschan(buf);
      time_left_in_game = 0;
}

int num_in_arena()
{
  struct char_data *i;
  struct descriptor_data *d;
  int num = 0;

  for (d = descriptor_list; d; d = d->next){
     if (d->character)
   {
       i = d->character;
           if ((ROOM_FLAGGED(IN_ROOM(i), ROOM_ARENA)) && (i->in_room != NOWHERE))
        {
           if(GET_LEVEL(i) < LVL_ELDER)
             num++;
        }
   }
}
 return num;
}

ACMD(do_awho)
{
  struct descriptor_data *d;
  struct char_data *tch;
  double x = 100;
  int num =0, i = 0;
  *buf2 = '\0';

  if (in_arena == ARENA_OFF) {
    send_to_char("&WThere is no &RWAR&W going on right now&n.\r\n", ch);
	return;
  }

  sprintf(buf,"\r\n&CWarDome &RWAR&n\r\n");
  sprintf(buf + strlen(buf),"  &GLevels &W[&c%d&W] &Gto &W[&c%d&W]&n\r\n", lo_lim, hi_lim);
  sprintf(buf + strlen(buf),"     &GRemort &W[&c%d&W] &Gto &W[&c%d&W]&n\r\n", lo_rem, hi_rem);
  sprintf(buf + strlen(buf),"         Jackpot [&Y%s&n]\r\n\r\n", add_points(arena_pot));
  sprintf(buf + strlen(buf),"&WGladiators playing the &RWAR&W:\r\n");

  for (d = descriptor_list; d; d = d->next)
   if (!d->connected && d->character){
     tch = d->character;
 	 if (ROOM_FLAGGED(IN_ROOM(tch), ROOM_ARENA) &&
	     (tch->in_room != NOWHERE) && GET_LEVEL(tch)<LVL_ELDER){
		 if(!ROOM_FLAGGED(IN_ROOM(ch), ROOM_ARENA))
		 	x = ((GET_HIT(tch)*100)/GET_MAX_HIT(tch));
         sprintf(buf + strlen(buf), "&c[&R%3d&c] &C%-20.20s &R%3.2f%%&n%s ", GET_LEVEL(tch),
         GET_NAME(tch), x, (!(++num % 2) ? "\r\n" : ""));
	i++;
       }
   }
   if(i == 0)
       sprintf(buf, "&WNo one was brave to enjoy the &RWAR&W.&n");

  strcat(buf, "\r\n");
  send_to_char(buf, ch);
}
ACMD(do_bet)
{
  int newbet;
  struct char_data *bet_on;
//  struct char_data *winner;
//  struct char_data *i;


  two_arguments(argument, arg, buf1);

  if (IS_NPC(ch)) {
    send_to_char("Mobs cant bet on the war.\r\n",ch);
    return;
  }

  if(!*arg) {
    if(start_time) {
      send_to_char("Usage: bet <player> <amount>\r\n",ch);
      return;
    } else if(!start_time){
      send_to_char("&WSorry no &Rwar&W is in going on.&n\r\n", ch);
      return;
    } else if(in_arena == ARENA_RUNNING) {
      send_to_char("&WSorry &Rwar&W has already started, no more bets.&n\r\n", ch);
      return;
    }
  }

  if (!start_time)
    send_to_char("&WSorry the &Rwar &Wis not happening, wait until it opens up to bet.&n\r\n",ch);
  else if(num_in_arena() < 2)
    send_to_char("&WNeed more than one &ygladiator&W to bet.\r\n", ch);
  else if (in_arena == ARENA_RUNNING)
    send_to_char("&WSorry &Rwar&W is happening, no more bets.&n\r\n", ch);
  else if (!(bet_on = get_char_vis(ch, arg)))
    send_to_char(NOPERSON, ch);
  else if (bet_on == ch)
    send_to_char("&WThat doesn't make much sense, does it?&n\r\n", ch);
  else if (!(ROOM_FLAGGED(IN_ROOM(bet_on), ROOM_ARENA)))
    send_to_char("&WSorry that person is not fighting on the &Rwar.&n\r\n", ch);
  else if(GET_REMORT(ch) == 0 && GET_LEVEL(ch) < 2)
    send_to_char("&WYou don't have level to perform this movement.&n\r\n", ch);
  else {

    if(GET_BET_AMT(ch) > 0) {
      send_to_char("&WYou have already made a bet this time!&n\r\n", ch);
      return;
    }

    GET_BETTED_ON(ch) = bet_on;

    if (!is_number(buf1)) {
      send_to_char("That should be a number idiot!\r\n", ch);
      return;
    }

    newbet = atoi(buf1);

    if (newbet <= 0 ) {
      send_to_char("Bet some gold why dont you!\r\n", ch);
      return;
    }

    if (newbet > GET_GOLD(ch) ) {
      send_to_char ("You don't have that much money!\n\r",ch);
      return;
    }

    if ( newbet > 25000 ) {
      send_to_char("Sorry the house will not accept that much.\r\n", ch);
      return;
    }

    GET_BET_AMT(ch) = newbet;

    GET_GOLD(ch) -= newbet; /* substract the gold - important :) */
    arena_pot += (newbet / 2);
    bet_pot += (newbet * 2);
    sprintf(buf, "&c[&R-&rWAR&R-&c] &b%s &nhas placed &y%s &ncoins on &b%s&G!&n\r\n", GET_NAME(ch),
	    add_points(newbet), GET_NAME(bet_on));
    sportschan(buf);
  }
}

ACMD(do_ahall)
{
  char site[MAX_INPUT_LENGTH], format[MAX_INPUT_LENGTH], *timestr;
  char format2[MAX_INPUT_LENGTH];
  struct hall_of_fame_element *fame_node;

  *buf = '\0';
  *buf2 = '\0';
  *buf3 = '\0';

    if (!fame_list) {
      send_to_char("&wThere are no &BGladiators &won &RHALL OF FAME&w.\n\r", ch);
      return;
    }

  sprintf(buf2,       "&y'''''''''''''''''''''''''''''''''''''''''\n\r");
  sprintf(buf2,     "%s&y'      &BW&bar&BD&bome &RArena &YHall &yof &YFame&y       '&n\n\r",buf2);
  sprintf(buf2,     "%s&y'''''''''''''''''''''''''''''''''''''''''\n\r\n\r",buf2);

  strcpy(format, "&G%-10.10s&n  &Y%-20.16s&n  &R%-40s&n\n\r");
    sprintf(buf, format,
	    "&gDate&n",
	    "&yAward Amt&n",
	    "&rName&n"
	  );
  strcat(buf2, buf);
  sprintf(buf, format,
	    "-=-=-=-=-=-=-=--=-=-=-=-=-=-=--=-",
	    "-=-=-=-=-=-=-=--=-=-=-=-=-=-=--=-",
	    "-=-=-=-=-=-=-=");
  strcat(buf2, buf);
  strcpy(format2, "&G%-10.10s&n  &Y%16.16s&n  &R%s&n\n\r");

  for (fame_node = fame_list; fame_node; fame_node = fame_node->next) {
    if (fame_node->date) {
	  timestr = asctime(localtime(&(fame_node->date)));
	*(timestr + 10) = 0;
	strcpy(site, timestr);
      }
      else
	strcpy(site, "Unknown");
      add_points((fame_node->award) * 2 );
      sprintf(buf, format2, site, buf3, fame_node->name);
      strcat(buf2, buf);
    }

  page_string(ch->desc, buf2, 1);
  return;
}



void load_hall_of_fame(void)
{
  FILE *fl;
  int date, award;
  char name[MAX_NAME_LENGTH + 1];
  struct hall_of_fame_element *next_node;

  fame_list = 0;

  if (!(fl = fopen(HALL_FAME_FILE, "r"))) {
    perror("Unable to open hall of fame file");
    return;
  }
  while (fscanf(fl, "%s %d %d", name, &date, &award) == 3) {
    CREATE(next_node, struct hall_of_fame_element, 1);
    strncpy(next_node->name, name, MAX_NAME_LENGTH);
    next_node->name[MAX_NAME_LENGTH] = '\0';
    next_node->date = date;
    next_node->award = award;
    next_node->next = fame_list;
    fame_list = next_node;
  }

  fclose(fl);
}


void write_fame_list(void)
{
  FILE *fl;

  if (!(fl = fopen(HALL_FAME_FILE, "w"))) {
    log("Error writing _hall_of_fame_list");
    return;
  }
  write_one_fame_node(fl, fame_list);/* recursively write from end to start */
  fclose(fl);

  return;
}

void write_one_fame_node(FILE * fp, struct hall_of_fame_element * node)
{
  if (node) {
    write_one_fame_node(fp, node->next);
    fprintf(fp, "%s %ld %ld\n", node->name,
	    (long) node->date, node->award);
  }
}
void find_bet_winners(struct char_data *winner)
{
  struct char_data *i;
  struct descriptor_data *d;

  *buf1 = '\0';

  for (d = descriptor_list; d; d = d->next)
    if (!d->connected) {
      i = d->character;
   if ((!IS_NPC(i)) && (i->in_room != NOWHERE) &&
       (GET_BETTED_ON(i) == winner) && GET_BET_AMT(i) > 0){
     sprintf(buf1, "\r\nYou have won &Y%d&n coins on your bet.\r\n",
	     GET_BET_AMT(i)*2);
     send_to_char(buf1, i);
     GET_GOLD(i) += GET_BET_AMT(i)*2;
     GET_BETTED_ON(i) = NULL;
     GET_BET_AMT(i) = 0;
   }
     GET_BETTED_ON(i) = NULL;
     GET_BET_AMT(i) = 0;
  }
}







