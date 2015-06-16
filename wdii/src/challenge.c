/**************************
 * 
 * File: challenge.c
 *
 * Writen by:  Paulo Estima Mello a.k.a. Lothar
 *
 * Used to make a challenge of player x player.
 * Can fight only 1 x 1.
 *
 * Challenge System Version: 1.1.1
 **************************/

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

extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct zone_data *zone_table;
extern int top_of_zone_table;
extern int top_of_world;
extern sh_int r_mortal_start_room;

void send_challenge_option(struct char_data * ch)
{

	send_to_char("You can use this options to the challenge system...\r\n"
		     "\r\n"
		     "For the challenger:		\r\n"
	             "	challenge 		<player>\r\n"      
	             "	challenge cancel		\r\n"
		     "\r\n"
		     "For who was challenged:		\r\n"
		     "	challenge accept		\r\n"
	             "	challenge decline		\r\n", ch);
		     
}
ACMD(do_challenge)
{
struct char_data *vict;

one_argument(argument, arg);

if(!*arg){
		send_challenge_option(ch);
		return;
	}

else if (!(vict = get_player_vis(ch, arg, FIND_CHAR_WORLD))){
	send_to_char("It is best to challenge someone who is actually here.\r\n", ch);
	return;
}
else if (ch == vict){
    send_to_char("You can not challenge one self.\r\n", ch);
  return;
}
else if (!vict){
	send_to_char("Whom do you wish to challenge?\r\n", ch);
	return;
}
else if (!IS_NPC(vict) && !vict->desc){        /* linkless */
    act("$E's linkless at the moment.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
	return;
}
else if (PLR_FLAGGED(vict, PLR_WRITING)){
	act("$E's writing a message right now; try again later.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
	return;
}
else if (GET_CHALLENGE(ch) == 1){
	sprintf(buf, "You are being challenged by %s. NACCEPT or NDECLINE first!\r\n", GET_NAME(GET_CHALLENGER(ch)));
	send_to_char(buf, ch);
	return;
}
else if (GET_CHALLENGE(ch) == 2){
	sprintf(buf, "You are already challenging &c%s, you must wait.\r\n", GET_NAME(GET_CHALLENGER(ch)));
	send_to_char(buf, ch);
	return;
}
else if (GET_CHALLENGE(ch) == 3){
	sprintf(buf, "You are already dualing %s, you must wait.\r\n", GET_NAME(GET_CHALLENGER(ch)));
	send_to_char(buf, ch);
	return;
}
else if (GET_CHALLENGE(vict) >= 1){
	send_to_char("You can not challenge some one who already is challenged.\r\n",ch);
	return;
}
else {

	GET_CHALLENGE(ch) = 2;
	GET_CHALLENGE(vict) = 1;
	GET_CHALLENGER(ch) = vict;
	GET_CHALLENGER(vict) = ch;

	sprintf(buf, "You challenge %s to a dual.\r\n", GET_NAME(vict));
	send_to_char(buf, ch);
	sprintf(buf, "%s challenges you to a dual, NACCEPT or NDECLINE?\r\n", GET_NAME(ch));
	send_to_char(buf, vict);
}
}
ACMD(do_accept){
	struct char_data *vict;
	room_vnum vto_room = 7505;
	room_rnum to_room;

	if(GET_CHALLENGE(ch) != 1){
		send_to_char("You are not challenged.", ch);
		return;
	}

	vict = GET_CHALLENGER(ch);

	GET_CHALLENGE(ch) = 3;
	GET_CHALLENGE(vict) = 3;

	sprintf(buf, "You accept %s's challenge and are magically transported into the arena.\r\n", GET_NAME(vict));
	send_to_char(buf, ch);

	vto_room = number(7503, 7504);
	to_room = real_room(vto_room);

	act("$n goes to fight THE DUAL.", TRUE, ch, 0, 0, TO_NOTVICT);
	char_from_room(ch);

	char_to_room(ch, to_room);
	look_at_room(ch, 0);

	sprintf(buf, "%s accepts your challenge and you are transported to the arena.\r\n", GET_NAME(ch));
	send_to_char(buf, vict);
	
	vto_room = number(7501, 7502);
	to_room = real_room(vto_room);

    act("$n goes to fight THE DUAL.", TRUE, vict, 0, 0, TO_NOTVICT);
	char_from_room(vict);

	char_to_room(vict, to_room);
	look_at_room(vict, 0);
}

ACMD(do_decline){
	struct char_data *vict;

	if(GET_CHALLENGE(ch) != 1){
		send_to_char("You are not challenged.", ch);
		return;
	}

	vict = GET_CHALLENGER(ch);

	GET_CHALLENGE(ch) = 0;
	GET_CHALLENGE(vict) = 0;

	sprintf(buf, "You decline %s's challenge and hand your title over.\r\n", GET_NAME(vict));
	send_to_char(buf, ch);

	sprintf(buf, "%s declines your challenge and hands over the title.\r\n", GET_NAME(ch));
	send_to_char(buf, vict);
}
