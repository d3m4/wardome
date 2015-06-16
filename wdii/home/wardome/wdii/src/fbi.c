/***************************************************************************
* File: fbi.c                                       a part of DungeonMUD   *
*                                                                          *
* Description: Is a code of a FBI service... it will be used to turn       *
* players agents of your mud.                                              *
*                                                                          *
* By Archangel(Paulo Mello) Implementor of DungeonMUD                      *
****************************************************************************/

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "interpreter.h"
#include "db.h"
#include "screen.h"
#include "fbi.h"
#include "buffer.h"

void send_fbi_option(struct char_data * ch)
{

	send_to_char("You can use this options to the FBI system:\r\n"
	             "	Fbi enjoy	<player>\r\n"
	             "	Fbi promote	<player>\r\n"
	             "	Fbi demote	<player>\r\n"
	             "	Fbi dismiss	<player>\r\n", ch);

}

void fbi_enjoy(struct char_data * ch, char *arg)
{
	struct char_data *vict;

	if(!*arg){
		send_to_char("You need a player to enjoy the fbi...\r\n", ch);
		send_fbi_option(ch);
		return;
	}

	if(!(vict = get_char_vis(ch, arg)) || IS_NPC(vict)) {
		send_to_char("Who you want enjoy FBI??\r\n",ch);
		send_fbi_option(ch);
		return;
	}

	if(GET_FBI_POS(vict) > 0){
		send_to_char("He is already a FBI agent...\r\n", ch);
		return;
	}


	sprintf(buf, "%s enjoy you to the secret service of Dungeon MUD.\r\n", GET_NAME(ch));
	send_to_char(buf, vict);
	sprintf(buf, "You enjoy %s to the secret service of Dungeon MUD.\r\n", GET_NAME(vict));
	send_to_char(buf, ch);
	sprintf(buf, "(FBI) %s enjoys %s to the FBI service.", GET_NAME(ch), GET_NAME(vict));
	mudlog(buf, BRF, LVL_SUPGOD, TRUE);
	GET_FBI_POS(vict) = FBI_AGENT;

}

void fbi_promote(struct char_data *ch, char *arg)
{

	struct char_data *vict;

	if(!*arg){
		send_to_char("You need a player to promote...\r\n", ch);
		send_fbi_option(ch);
		return;
	}

	if(!(vict = get_char_vis(ch, arg)) || IS_NPC(vict)) {
		send_to_char("Who you want promote in FBI??\r\n",ch);
		send_fbi_option(ch);
		return;
	}

	if(GET_FBI_POS(vict) > 0 && GET_FBI_POS(vict) < 3){
		sprintf(buf, "%s promotes you in the secret service of Dungeon MUD.\r\n", GET_NAME(ch));
		send_to_char(buf, vict);
		sprintf(buf, "You promotes %s in the secret service of Dungeon MUD.\r\n", GET_NAME(vict));
		send_to_char(buf, ch);
		sprintf(buf, "(FBI) %s promotes %s in the FBI service.", GET_NAME(ch), GET_NAME(vict));
		mudlog(buf, BRF, LVL_SUPGOD, TRUE);
		GET_FBI_POS(vict) += 1;
	} else
		send_to_char("You can't promote...\r\n", ch);
		return;
}

void fbi_demote(struct char_data *ch, char *arg)
{

	struct char_data *vict;

	if(!*arg){
		send_to_char("You need a player to demote...\r\n", ch);
		send_fbi_option(ch);
		return;
	}

	if(!(vict = get_char_vis(ch, arg)) || GET_FBI_POS(vict) < 1 || IS_NPC(vict)) {
		send_to_char("Who you want demote in FBI??\r\n",ch);
		send_fbi_option(ch);
		return;
	}

	if(GET_FBI_POS(vict) > 1 && GET_FBI_POS(vict) <= 3){
		sprintf(buf, "%s demotes you in the secret service of Dungeon MUD.\r\n", GET_NAME(ch));
		send_to_char(buf, vict);
		sprintf(buf, "You demote %s in the secret service of Dungeon MUD.\r\n", GET_NAME(vict));
		send_to_char(buf, ch);
		sprintf(buf, "(FBI) %s demotes %s in the FBI service.", GET_NAME(ch), GET_NAME(vict));
		mudlog(buf, BRF, LVL_SUPGOD, TRUE);
		GET_FBI_POS(vict) -= 1;
	} else
		send_to_char("You can't demote...\r\n", ch);
		return;
}

void fbi_dismiss(struct char_data * ch, char *arg)
{
	struct char_data *vict;

	if(!*arg){
		send_to_char("You need a player to dismiss...\r\n", ch);
		send_fbi_option(ch);
		return;
	}

	if(!(vict = get_char_vis(ch, arg)) || IS_NPC(vict)) {
		send_to_char("Who you want dismiss FBI??\r\n", ch);
		send_fbi_option(ch);
		return;
	}

	if(GET_FBI_POS(vict) < 1){
		send_to_char("He is'n a FBI agent...\r\n", ch);
		return;
	}

	sprintf(buf, "%s dismiss you of the secret service of Dungeon MUD...\r\nYou don't have acess to the secret base of FBI service.\r\n", GET_NAME(ch));
	send_to_char(buf, vict);
	sprintf(buf, "You dismiss %s of the secret service of Dungeon MUD.\r\n", GET_NAME(vict));
	send_to_char(buf, ch);
	sprintf(buf, "(FBI) %s dismiss %s of the FBI service.", GET_NAME(ch), GET_NAME(vict));
	mudlog(buf, BRF, LVL_SUPGOD, TRUE);
	GET_FBI_POS(vict) = 0;

}

ACMD(do_fbi)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];

	argument = two_arguments(argument, arg1, arg2);

	if(!*arg1){
		send_fbi_option(ch);
		return;
	}

	if (!str_cmp("enjoy", arg1)){
		fbi_enjoy(ch, arg2);
		return;
	}
	if (!str_cmp("promote", arg1)){
		fbi_promote(ch, arg2);
		return;
	}
	if (!str_cmp("demote", arg1)){
		fbi_demote(ch, arg2);
		return;
	}
	if (!str_cmp("dismiss", arg1)){
		fbi_dismiss(ch, arg2);
		return;
	}
}
