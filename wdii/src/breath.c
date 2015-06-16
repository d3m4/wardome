/***************************************************************************
* File: breath.c                                    a part of WarDomeMUD   *
*                                                                          *
* Description: This is a simple code to the breath system.		   *
*                                                                          *
* By Archangel(Paulo Mello) Implementor of WarDomeMUD                      *
****************************************************************************/

#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "buffer.h"

extern struct room_data *world;
extern struct char_data *character_list;
void raw_kill(struct char_data * ch, struct char_data * killer);
int has_boat(struct char_data *ch);
int breath_gain(struct char_data * ch);

void loss_breath(struct char_data * ch, int breath)
{

	if (AFF2_FLAGGED(ch, AFF2_IRON_BODY))
		return;

	if (ROOM_AFFECTED(ch->in_room, RAFF_LIQUID_AIR)) {
		GET_OXI(ch) -= number(2, 10);
	}

	if(GET_LEVEL(ch) < LVL_IMMORT && breath > 0)
	{
		if(GET_OXI(ch) >= 1)
		{
			GET_OXI(ch) -= breath;
			send_to_char("Your breath becomes deeper and slower...\r\n", ch);
		} else {
			if(GET_HIT(ch) > 0)
			{
            	GET_HIT(ch) -= (GET_MAX_HIT(ch)*0.15);
            	send_to_char("&RYou need some oxygen, your life is almost extinguished!&n\r\n", ch);
			} else {
	           	GET_HIT(ch) = 0;
	           	send_to_char("&RYour breath becomes so slow that you die because of it.&n\r\n", ch);
            	raw_kill(ch, NULL);
			}
		}
	}
}

int breath[][12] = {
//				VA DR DW EL OG OR TR GI GN LI SE GO
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/*mountain*/			{2, 3, 4, 3, 4, 4, 5, 2, 3, 2, 5, 0},
/*water_swim*/			{1, 1, 1, 1, 2, 1, 2, 1, 2, 1, 0, 1},
/*waternswim*/			{6, 6, 6, 6, 7, 6, 7, 6, 7, 6, 0, 6},
/*underwater*/			{3, 4, 2, 3, 5, 3, 4, 2, 4, 2, 0, 1},
/*flying*/			{0, 4, 3, 3, 2, 4, 3, 2, 4, 3, 5, 0}
};

void make_breath(struct char_data *ch)
{
	loss_breath(ch, breath[SECT(ch->in_room)][(GET_RACE(ch) > RACE_GORAK ? (GET_RACE(ch)-RACE_DUNEDAIN) : GET_RACE(ch))] );
}

void update_breath(void)
{
  struct char_data *i, *next_char;

  for (i = character_list; i; i = next_char) {
    
	  next_char = i->next;

	  if (!i)
		return;

	if (!IS_NPC(i))
	{
		if (!has_boat)
			make_breath(i);
	 	else {
			if (GET_POS(i) >= POS_STUNNED) {
				if (!AFF2_FLAGGED(i, AFF2_ASPHYXIATE))
					GET_OXI(i) = MIN(GET_OXI(i) + breath_gain(i), GET_MAX_OXI(i));
			}
		}
	}
  }
}
