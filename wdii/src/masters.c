/************************************************* 
** File: Masters.c				**
** Usado para os Comandos basicos de Discipulos **
** Implementado por Taerom 		        **
** Inicio do Projeto: 04/07/03                  **
*************************************************/

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "interpreter.h"
#include "db.h"
#include "screen.h"
#include "clan.h"
#include "buffer.h"
#include "masters.h"
#include "spells.h"

/* Tabela de Numeros
-1 =  Sem Master
2 em diante = Id do Master; */

void criacao(struct descriptor_data *d, char *arg)
{
    switch (*arg)
    {
     case 'y':
     case 'Y':
       SEND_TO_Q("Great, And what your Master Name?", d);
       STATE(d) = CON_NEWMASTER;
       break;
     case 'N':
     case 'n':
        GET_MASTER(d->character) = -1;
        SEND_TO_Q("\r\nWhat is your sex (M/F)? ", d);
    	STATE(d) = CON_QSEX;
    	break;
     default:
      SEND_TO_Q("\r\nInvalid choice, Type Yes Or No.", d);
      return;  
     }
}	

void criacao_new(struct descriptor_data *d, char *arg)
{
	int master = 0;
		 	 	 	
	 master = get_id_by_name(arg);
	 if (master < 2)
	 {
	 SEND_TO_Q("\r\nSorry, Nobody by this name", d);
	 SEND_TO_Q("\r\n\r\nPlese a Valid Master Name:", d);
    	 STATE(d) = CON_NEWMASTER;	
    	 }
    	 else
	 {
	 GET_MASTER(d->character) = master;
	 SEND_TO_Q("\r\nWhat is your sex (M/F)? ", d);
    	 STATE(d) = CON_QSEX;	
    	 }
}	

void verifica(struct descriptor_data *d)
{
	 if (GET_LEVEL(d->character) > 100 && (GET_MASTER(d->character) != -1))
	 {
	 SEND_TO_Q("Master System Only at level 100", d);
	 GET_MASTER(d->character) = -1;
	 save_char(d->character, NOWHERE);
	 }
	if ((GET_MASTER(d->character) < 2) && GET_MASTER(d->character) != -1)
	 {
	SEND_TO_Q("Your Master Has been Deleted or we have a Bug Here\r\n", d);
	GET_MASTER(d->character) = -1;
	sprintf(buf, "Sistema de Discipulos (%s) REPROVADO.", GET_NAME(d->character));
        mudlog(buf, NRM, LVL_IMMORT, 0);
	save_char(d->character, NOWHERE);
	 }
	
}	
ACMD(do_master)
{
	struct char_data *victim;
	
	if (GET_MASTER(ch) == -1)
	{
	sprintf(buf, "Sorry you Don't Have a Master\r\n");
	send_to_char(buf, ch);
	}

	else if (!GET_MASTER(ch))
	send_to_char("Bug! Bug! Report to Taerom\r\n", ch);
	 
	else
  	{
    	 CREATE(victim, struct char_data, 1);
    	 clear_char(victim);
    	 CREATE(victim->player_specials, struct player_special_data, 1);

        if (load_char(get_name_by_id(GET_MASTER(ch)), victim) > -1)
         {
 	  {	
	sprintf(buf, "&w          :.~'~.: &WYour Master Information &w :.~'~.:\r\n");
	sprintf(buf + strlen(buf), "&wName: &W%s&w\r\n", GET_NAME(victim));
	sprintf(buf + strlen(buf), "&wLevel: &W%d&w - Race: &W%s&w \r\n", GET_LEVEL(victim), pc_race_types[(int) GET_RACE(victim)]); 
	sprintf(buf + strlen(buf), "&wClass: &W%s&w - Remort: &W%d&w \r\n", pc_class_types[(int) GET_CLASS(victim)], GET_REMORT(victim));	
	sprintf(buf + strlen(buf), "&wYour Benefit to him: &W%d&w Rpoints\r\n", GET_RP(victim));
	send_to_char(buf, ch);
 	  }
	 }
	free_char(victim);
	} 
} 

void list_skills(struct char_data *ch)
{
  char buf[MAX_STRING_LENGTH];
  int i, j, class = (int) GET_CLASS(ch), x = 1;
  int cost;
 
  if (!GET_RP(ch)) 
  {
    send_to_char("You have no practice sessions remaining.\r\n", ch);
    return;
  }
       send_to_char(TROLOLO_MENU, ch);
	for (j = 1; j < LVL_IMMORT; j++) 
 	{
 	 for (i = 1; i < MAX_SKILLS+1; i++)
  	 {
   	  cost = spell_info[i].min_level[class] * 13;
   	  if (spell_info[i].min_level[class] == j)
     	  {
          sprintf(buf, "&Y[&y%4d&Y] &y%-18s&n%s", cost, reprimir(spells[i], 18), (!(x++ % 3) ? "\r\n" : " "));
          send_to_char(buf, ch);
          }
   	 } 
 	}
          sprintf(buf,"\r\n\r\n&yYou have &Y%d &WRole Points&y remaining&n\r\n", GET_RP(ch));
          send_to_char(buf, ch);
}


SPECIAL(master)
{
  int skill_num, percent;
  extern struct spell_info_type spell_info[];
  int cost, class = ((int) GET_CLASS(ch));
 
  if (IS_NPC(ch) || !CMD_IS("practice"))
    return (FALSE);

  skip_spaces(&argument);

  if (!*argument) 
  {
    list_skills(ch);
    return (TRUE);
  }
  
  if (GET_RP(ch) <= 0) 
  {
    send_to_char("You do not seem to be able to practice now.\r\n", ch);
    return (TRUE);
  }

  skill_num = find_skill_num(argument);
  cost = spell_info[skill_num].min_level[class] * 10;
  
  if (skill_num < 1 || (GET_LEVEL(ch) < spell_info[skill_num].min_level[class])) 
  {
    send_to_char("You do not know of that skill/spell.\r\n", ch);
    return (TRUE);
  }
  if (GET_RP(ch) < cost) 
  {
    send_to_char("You don't have Role Points to learn this.\r\n", ch);
    return (TRUE);
  }
  if (GET_SKILL(ch, skill_num) >= LEARNED(ch)) 
  {
    send_to_char( "You are already learned in that area.\r\n", ch);
    return (TRUE);
  }
  send_to_char("The WarMaster show how to do this.\r\n", ch);
  
  GET_RP(ch) -= cost;

  percent = GET_SKILL(ch, skill_num);
  percent += MIN(MAXGAIN(ch), MAX(MINGAIN(ch), int_app[GET_INT(ch)].learn));

  SET_SKILL(ch, skill_num, MIN(LEARNED(ch), percent));

  if (GET_SKILL(ch, skill_num) >= LEARNED(ch))
    send_to_char("You are now learned in that area.\r\n", ch);

  return (TRUE);
}
void ganha_rp(struct char_data *ch)
{ 
	struct char_data *victim = NULL;
        int gain;
        
  	gain = GET_LEVEL(ch) * 100;
  	
  	if (GET_MASTER(ch) < 2)
  	return;
  	else if (GET_LEVEL(ch) > 100)
	return;
	else
  	{
          CREATE(victim, struct char_data, 1);
    	  clear_char(victim);
    	  CREATE(victim->player_specials, struct player_special_data, 1);
          if (load_char(get_name_by_id(GET_MASTER(ch)), victim) > -1)
          {
         	sprintf(buf, "Voce Ganhou %d RolePoints\r\n", gain);
         	send_to_char(buf, victim);	
         	GET_RP(victim) = gain;
         	save_char(victim, NOWHERE);
       
          } 
	}
      free_char(victim);
}

