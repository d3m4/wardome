/* ************************************************************************
*   File: mob_int.c                                     Part of CircleMUD *
*  Usage: Combat system                                                   *
*									  *
*  Acoes de mobs para serem altamente eficazes				  *
*									  *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "handler.h"
#include "interpreter.h"
#include "spells.h"

extern struct room_data *world;
struct spell_info_type spell_info[TOP_SPELL_DEFINE + 1];

ACMD(do_bash);    
ACMD(do_kick);  
ACMD(do_stroke);  
ACMD(do_combo);   
ACMD(do_sabre);  
ACMD(do_circle);  
ACMD(do_disarm); 
ACMD(do_berzerk); 
ACMD(do_headbash);

struct mob_fight {
   int skill;
   void (*fazer_skill)
           (struct char_data *ch, char * argument, int cmd, int subcmd);
};

const struct mob_fight ataque[] = {
   { SPELL_BLINDNESS		,	0},
   { SPELL_BURNING_HANDS	,	0},
   { SPELL_CALL_LIGHTNING	,	0},
   { SPELL_CHILL_TOUCH   	,	0},
   { SPELL_COLOR_SPRAY   	,	0},
   { SPELL_CURSE         	,	0},
   { SPELL_ENERGY_DRAIN  	,	0},
   { SPELL_FIREBALL      	,	0},
   { SPELL_HARM          	,	0},
   { SPELL_LIGHTNING_BOLT	,	0},
   { SPELL_MAGIC_MISSILE 	,	0},
   { SPELL_POISON        	,	0},
   { SPELL_SHOCKING_GRASP	,	0},
   { SPELL_DISPEL_GOOD   	,	0},
   { SPELL_DAMNED_CURSE  	,	0},
   { SPELL_ENTANGLE      	,	0},
   { SKILL_BASH           	,	do_bash},
   { SKILL_KICK              	,	do_kick},
   { SKILL_STROKE            	,	do_stroke},
   { SKILL_COMBO             	,	do_combo},
   { SKILL_SABRE             	,	do_sabre},
   { SKILL_DISARM            	,	do_disarm},
   { SKILL_BERZERK           	,	do_berzerk},
   { SKILL_HEADBASH          	,	do_headbash},

};

#define MAX_MOB_AB			24

int ver_melhor(struct char_data *ch, int ataque)
{
  bool x = FALSE;
  int class;
  int levelch;
  int minlevel;
  
  class = GET_CLASS(ch);
  levelch = GET_LEVEL(ch);
  minlevel = spell_info[ataque].min_level[class];
  if (minlevel < 202)
      if (minlevel < levelch)
	         x = TRUE;

  return (x);
}

void mob_ia(struct char_data *ch)
{               
  int i;
  sh_int z;
  sh_int possiveis[MAX_MOB_AB];
  int num_ab;
 
  update_pos(ch);
   
  if(!IS_NPC(ch))
    return;

  if(GET_POS(ch) < POS_FIGHTING || GET_CLASS(ch) < 0)
    return;

  /* evitar loops infinitos */
 /* for (i = 1; i < MAX_SKILLS+1; i++)
    for(z = 0; z <= MAX_MOB_AB; z++) {
      if (i == ataque[z].skill)
	 b = TRUE;
    }*/
  
  num_ab = 0;
  for(z = 0; z <= MAX_MOB_AB - 1; z++) 
      if ( ver_melhor(ch, ataque[z].skill)){
	 possiveis[num_ab] = z;
	 num_ab++;
      }
 
  if(num_ab == 0)
    return;
  
 /* do {
  	x = number(0, MAX_MOB_AB);
  } while (!ver_melhor(ch, ataque[x].skill));*/
  
  i = number(0, num_ab - 1);
    
  if(ataque[possiveis[i]].skill > MAX_SPELLS)
    ((*ataque[possiveis[i]].fazer_skill) (ch, GET_NAME(FIGHTING(ch)), 0, 0));
  else
    call_magic(ch, FIGHTING(ch), 0, ataque[possiveis[i]].skill, GET_LEVEL(ch), CAST_SPELL);
}

/*
 mudar calculo
*/
int alvo_better(struct char_data *ch)
{
 int x;
 x =  GET_DAMROLL(ch);
 if(GET_EQ(ch, WEAR_WIELD)) 
    x += dice(GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 1), GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 2));

 return (MAX(x,1));
}

int contar_briga(struct char_data *ch)
{
  struct char_data *tch;
  int x = 0;

   for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room)
	if(FIGHTING(tch) == ch)
		x++;

   return (x);
}

struct char_data *melhor_alvo(struct char_data *ch)
{
   struct char_data *tch;
   struct char_data *vict = NULL;
   int menor = 0 ;
   int maior;
   int cont = 1 ;
      

   for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room)
  {
        if(FIGHTING(tch) == ch) 
       {
          if(!IS_NPC(tch)) cont = 2 ; 
          maior = alvo_better(tch) * cont;
          if(maior > menor) 
          {
            menor = maior;
            vict = tch;
          }
         cont = 1 ;
       }
   }
   return (vict);
}

