/* ************************************************************************
*   File: spec_procs.c                                  Part of CircleMUD *
*  Usage: implementation of special procedures for mobiles/objects/rooms  *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"
 

#include "structs.h"
#include "buffer.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "dg_scripts.h"
#include "winddragon.h"
#include "clan.h"


/*   external vars  */
extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern struct spell_info_type spell_info[];
extern struct int_app_type int_app[];
extern int top_of_world;
extern struct zone_data *zone_table;
extern char *spells[];
byte number_of_small_houses;
byte number_of_medium_houses;
byte number_of_large_houses;
extern struct con_app_type con_app[];


/* extern functions */
void add_follower(struct char_data * ch, struct char_data * leader);
char *add_points(int value);
void save_char(struct char_data * ch, sh_int load_room);
int defender(struct char_data *ch, struct char_data *victim);
ACMD(do_drop);
ACMD(do_gen_door);
ACMD(do_say);
ACMD(do_flee);
void obj_to_char(struct obj_data * object, struct char_data * ch);
void die_in_dt(struct char_data * ch);
int conta_cores(const char *texto);
void play_sound(struct char_data *ch, char *sound, int type);
extern sh_int r_mortal_start_room;

/* local functions */
int aceita_ra(int num1, int num2) ;
void sort_spells(void);
const char *how_good(int percent);
void list_skills(struct char_data * ch);
SPECIAL(dump);
SPECIAL(mayor);
SPECIAL(forger);
void npc_steal(struct char_data * ch, struct char_data * victim);
SPECIAL(snake);
SPECIAL(thief);
SPECIAL(magic_user);
SPECIAL(puff);
SPECIAL(fido);
SPECIAL(janitor);
SPECIAL(cityguard);
SPECIAL(gym);
SPECIAL(temple);
SPECIAL(archer);
SPECIAL(pet_shops);
SPECIAL(marbles);
SPECIAL(bank);
SPECIAL(priest_healer);
SPECIAL(priest);
SPECIAL(blacksmith);
SPECIAL(icewizard);
SPECIAL(preparation_room);
SPECIAL(sund_earl);
SPECIAL(hangman);
SPECIAL(blinder);
SPECIAL(silktrader);
SPECIAL(butcher);
SPECIAL(idiot);
SPECIAL(athos);
SPECIAL(stu);
SPECIAL(winddragon);
SPECIAL(windroom);
SPECIAL(wardome);
SPECIAL(questshop);
SPECIAL(black_dragon);
SPECIAL(black_monster);
SPECIAL(clan_guard);
SPECIAL(house_shop);
SPECIAL(reborn);
SPECIAL(alinhamento);
SPECIAL(random_room);
SPECIAL(espada_fica_azul);
SPECIAL(religiao) ;
SPECIAL(oracle);
SPECIAL(destructor) ; 
//SPECIAL(cassino);
SPECIAL(soundroom);
SPECIAL(soundroom2);
SPECIAL(anelrecall);
SPECIAL(ninja);

/* ********************************************************************
*  Special procedures for mobiles                                     *
******************************************************************** */

int spell_sort_info[MAX_SKILLS+1];

void sort_spells(void)
{
  int a, b, tmp;

  /* initialize array */
  for (a = 1; a < MAX_SKILLS; a++)
    spell_sort_info[a] = a;

  /* Sort.  'a' starts at 1, not 0, to remove 'RESERVED' */
  for (a = 1; a < MAX_SKILLS - 1; a++)
    for (b = a + 1; b < MAX_SKILLS; b++)
      if (strcmp(spells[spell_sort_info[a]], spells[spell_sort_info[b]]) > 0) {
        tmp = spell_sort_info[a];
        spell_sort_info[a] = spell_sort_info[b];
        spell_sort_info[b] = tmp;
      }
}
/*
void actualize_splskl(struct char_data *ch)
{
  int i, x;
  char skillbuf[MAX_STRING_LENGTH];

 for (i = 1; i < MAX_SPELLS+1; i++) {
  if (GET_LEVEL(ch) >= spell_info[i].min_level[(int) GET_CLASS(ch)] && GET_SKILL_LS(ch, i) < 20){
      SET_SKILL(ch, i, 20);
      sprintf(skillbuf, "&GYou learn the &gnew&G spell &Y%s&G.&n\r\n", spells[i]);
      send_to_char(skillbuf, ch);
  }
 }
 for (x = MAX_SPELLS + 1; x < MAX_SKILLS + 1; x++){
  if (GET_LEVEL(ch) >= spell_info[x].min_level[(int) GET_CLASS(ch)] && GET_SKILL_LS(ch, x) < 20){
      SET_SKILL(ch, x, 20);
      sprintf(skillbuf, "&GYou learn the &gnew&G skill &Y%s&G.&n\r\n", spells[x]);
      send_to_char(skillbuf, ch);
  }
 }
}
*/

int check_remort_sk(struct char_data *ch, int x)
{
	int y, remort;

	if(GET_REMORT(ch) > 0)
		for(remort = 1; remort <= GET_REMORT(ch); remort++)
			for(y = 0; y < MAX_RSKILLS; y++)
				if(GET_RSKILL(ch, remort, y) == x && GET_LEVEL(ch) >= spell_info[x].min_level[(int) GET_CLASS_REMORT(ch, remort)])
					return 1;

	return 0;
}
void actualize_splskl(struct char_data *ch)
{
	int x;
	char skillbuf[MAX_STRING_LENGTH];

	for(x = 1; x <= MAX_SKILLS; x++)
	{
		if(!GET_SKILL(ch, x))
		{
				if(GET_LEVEL(ch) >= spell_info[x].min_level[(int) GET_CLASS(ch)] || check_remort_sk(ch, x))
				{
					SET_SKILL(ch, x, 20);
//					sprintf(skillbuf, "&GYou learn the &gnew&G %s &Y%s&G.&n\r\n", (x >= MAX_SPELLS ? "skill" : "spell"), spells[x]);
//					send_to_char(skillbuf, ch);
				}
		}
	}
}

SPECIAL(dump)
{
  struct obj_data *k;
  int value = 0;

  for (k = world[ch->in_room].contents; k; k = world[ch->in_room].contents) {
    act("$p vanishes in a puff of smoke!", FALSE, 0, k, 0, TO_ROOM);
    extract_obj(k);
  }

  if (!CMD_IS("drop"))
    return 0;

  do_drop(ch, argument, cmd, 0);

  for (k = world[ch->in_room].contents; k; k = world[ch->in_room].contents) {
    act("$p vanishes in a puff of smoke!", FALSE, 0, k, 0, TO_ROOM);
    value += MAX(1, MIN(50, GET_OBJ_COST(k) / 10));
    extract_obj(k);
  }

  if (value) {
    act("You are awarded for outstanding performance.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n has been awarded for being a good citizen.", TRUE, ch, 0, 0, TO_ROOM);

    if (GET_LEVEL(ch) < 3)
      gain_exp(ch, value);
    else
      GET_GOLD(ch) += value;
      GET_GOLD_SUM(ch) += value;
  }
  return 1;
}

struct pont_player {
   struct char_data *player;
   struct obj_data *obj;
  };

static int fila_fim = 0;
static int fila_inicio = 0;

SPECIAL(forger)
{
  long preco;
  int cont;
  struct obj_data *obj;
  struct char_data *self;
  int j = 0, number,pos = 0;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp = tmpname;
  static struct pont_player buf_player[100];

  self = (struct char_data *) me;

  if (CMD_IS("value"))
  {
    one_argument(argument, arg);
  if (!*arg)
    {
     send_to_char("Value what?\r\n", ch);
     return(1);
    }

     strcpy(tmp, arg);
     if (!(number = get_number(&tmp)))
     number = 1;
       cont = 0;
        obj = NULL;
     
     for (j = 0; j < NUM_WEARS; j++)
        if (GET_EQ(ch, j) && CAN_SEE_OBJ(ch, GET_EQ(ch, j)) && isname(tmp, GET_EQ(ch, j)->name))
         {
             cont = cont + 1;
           if (cont == number)
           {
              obj = GET_EQ(ch, j);
              pos = j;
           }
         }

    if (obj == NULL)
     {
       sprintf(buf, "You don't seem to be using any %s.\r\n", tmp);
       send_to_char(buf, ch);
       return (1);
     }

    if (((GET_OBJ_TYPE(obj)) == ITEM_WAND) || ((GET_OBJ_TYPE(obj)) == ITEM_STAFF) ||
        ((GET_OBJ_TYPE(obj)) == ITEM_TREASURE) || ((GET_OBJ_TYPE(obj)) == ITEM_POTION) ||
        ((GET_OBJ_TYPE(obj)) == ITEM_OTHER) || ((GET_OBJ_TYPE(obj)) == ITEM_TRASH) ||
        ((GET_OBJ_TYPE(obj)) == ITEM_CONTAINER) || ((GET_OBJ_TYPE(obj)) == ITEM_NOTE) ||
        ((GET_OBJ_TYPE(obj)) == ITEM_DRINKCON) || ((GET_OBJ_TYPE(obj)) == ITEM_KEY) ||
        ((GET_OBJ_TYPE(obj)) == ITEM_FOOD) || ((GET_OBJ_TYPE(obj)) == ITEM_PEN) ||
        ((GET_OBJ_TYPE(obj)) == ITEM_BOAT) || ((GET_OBJ_TYPE(obj)) == ITEM_SCROLL) || (obj->obj_flags.cond == 101))
    {
       sprintf(buf, "&c$n tells you, 'I don't repair this item type.&c'" );
       act(buf, FALSE, self, 0, ch, TO_VICT);
       return (1);
    }

    preco = (obj->obj_flags.cost*0.8)/((obj->obj_flags.cond/25)+1);
    preco = MIN(preco, 1000) ; // trava by Luigi
    sprintf(buf, "The value of the repair is %ld gold coins.\r\n", preco);
    send_to_char(buf, ch);
    return (1);
  }


  if ((CMD_IS("repair")) || (CMD_IS("forge")))
   {
	play_sound(ch, "blacksmith.wav", SND_ROOM);
        one_argument(argument, arg);

     if (!*arg) 
     {
      if ((CMD_IS("repair")))
      {
       send_to_char("Blacksmith commands:\n\r", ch);
       send_to_char("\n\r", ch);
       send_to_char("      command                          price/action              \n\r", ch);
       send_to_char("=====================  ==========================================\n\r", ch);
       send_to_char(" repair                 this help                                \n\r", ch);
       send_to_char(" repair <object name>   depends of the equipament condition      \n\r", ch);
       send_to_char(" forge <object name>    10,000,000/make the object indestructible\n\r", ch);
       send_to_char(" value <object name>    evaluates the price of the repair        \n\r", ch);
       send_to_char("\n\r", ch);
       send_to_char("Example: repair 2.ring\n\r", ch);
       send_to_char("\n\r", ch);
       send_to_char("&ROBS:&w The equipment might beeing used by the player.\n\r", ch);
       send_to_char("     In case the owner of the equipment is not in this room in the \n\r", ch);
       send_to_char("     finish of the repair, the equipment will be confiscated as rent payment.\n\r", ch);
      }
      else
       send_to_char("Forge what?\r\n", ch);
      return (1);
     }
     strcpy(tmp, arg);
     if (!(number = get_number(&tmp)))
       number = 1;
         cont = 0;
          obj = NULL;
     for (j = 0; j < NUM_WEARS; j++)
           if (GET_EQ(ch, j) && CAN_SEE_OBJ(ch, GET_EQ(ch, j)) && isname(tmp, GET_EQ(ch, j)->name)) 
       {
             cont = cont + 1;
           if (cont == number)
           {
              obj = GET_EQ(ch, j);
              pos = j;
           }
        }

    if (obj == NULL)
     {
       sprintf(buf, "You don't seem to be using any %s.\r\n", tmp);
       send_to_char(buf, ch);
       return (1);
     }

     if (((GET_OBJ_TYPE(obj)) == ITEM_WAND) || ((GET_OBJ_TYPE(obj)) == ITEM_STAFF) ||
        ((GET_OBJ_TYPE(obj)) == ITEM_TREASURE) || ((GET_OBJ_TYPE(obj)) == ITEM_POTION) ||
        ((GET_OBJ_TYPE(obj)) == ITEM_OTHER) || ((GET_OBJ_TYPE(obj)) == ITEM_TRASH) ||
        ((GET_OBJ_TYPE(obj)) == ITEM_CONTAINER) || ((GET_OBJ_TYPE(obj)) == ITEM_NOTE) ||
        ((GET_OBJ_TYPE(obj)) == ITEM_DRINKCON) || ((GET_OBJ_TYPE(obj)) == ITEM_KEY) ||
        ((GET_OBJ_TYPE(obj)) == ITEM_FOOD) || ((GET_OBJ_TYPE(obj)) == ITEM_PEN) ||
        ((GET_OBJ_TYPE(obj)) == ITEM_BOAT) || ((GET_OBJ_TYPE(obj)) == ITEM_SCROLL) || (obj->obj_flags.cond > 100))

     {
       sprintf(buf, "&c$n tells you, 'I don't repair this item type.&c'" );
       act(buf, FALSE, self, 0, ch, TO_VICT);
       return (1);
     }

	 if (GET_OBJ_VNUM(obj) == 99) {
		 send_to_char("&nWow! I won't touch The Scythe of Death!\r\n", ch);
		 return (1);
	 }

     if (IS_OBJ_STAT(obj, ITEM_NODROP)){
       sprintf(buf, "I can't remove %s from you, it must be CURSED!\r\n", GET_EQ(ch, pos)->name);
       send_to_char(buf, ch);
       return (1);
     }

     if ((CMD_IS("repair")))
      preco = MIN(1000, (obj->obj_flags.cost*0.8)/((obj->obj_flags.cond/25)+1));
     else
      preco = 1000000;

     if ((GET_GOLD(ch) < preco)) 
     {
  	send_to_char("You don't have enough gold!\r\n", ch);
  	return (1);
     }

     GET_GOLD(ch) -= preco;
     sprintf(buf, "That'll be %ld gold coins.\r\n", preco);
     send_to_char(buf, ch);
     if ((CMD_IS("repair")))
      obj->obj_flags.cond = 100;
     else
      obj->obj_flags.cond = 101;
      buf_player[fila_fim].player = ch;
      buf_player[fila_fim].obj = obj;
      fila_fim++;
      
     if (fila_fim == 101)
      fila_fim = 0;
     unequip_char(ch, pos);
     //obj_from_char(obj);
     obj_to_char(obj, self);
     act("$N get $p from you.", FALSE, ch, obj, self, TO_CHAR);
     act("You get $p from $n.", FALSE, ch, obj, self, TO_VICT);
     act("$N get $p from $n.", TRUE, ch, obj,self, TO_NOTVICT);
     save_char(ch, NOWHERE);
     save_char(self, NOWHERE);
     do_say(self, "Wait a minute here.", 0, 0);
     if ((CMD_IS("repair")))
      act("$n start his work.", FALSE, self, 0, 0, TO_ROOM);
     else
      act("$n get a holy metal and start his work.", FALSE, self, 0, 0, TO_ROOM);
     return (1);
  }

  if (!cmd) {
   sprintf(buf,"inicio: %d.\r\n",fila_inicio);
   send_to_char(buf, ch);
   sprintf(buf,"fim: %d.\r\n",fila_fim);
   send_to_char(buf, ch);

    if (fila_inicio != fila_fim){
      obj_from_char(buf_player[fila_inicio].obj);
      if (buf_player[fila_inicio].player->in_room == self->in_room){
        obj_to_char(buf_player[fila_inicio].obj, buf_player[fila_inicio].player);

        act("$N give $p to you.", FALSE, buf_player[fila_inicio].player, buf_player[fila_inicio].obj, self, TO_CHAR);
        act("You give $p to $n.", FALSE, buf_player[fila_inicio].player, buf_player[fila_inicio].obj, self, TO_VICT);
        act("$N give $p to $n.", TRUE, buf_player[fila_inicio].player, buf_player[fila_inicio].obj, self, TO_NOTVICT);
        save_char(buf_player[fila_inicio].player, NOWHERE);
        save_char(self, NOWHERE);
      }
      else{
        save_char(self, NOWHERE);
        act("You forgot to get your $p with $N. It will be donated now.", FALSE, buf_player[fila_inicio].player, buf_player[fila_inicio].obj, self, TO_CHAR);
        act("You kept $p in the closet to pay your rent service..", FALSE, buf_player[fila_inicio].player, buf_player[fila_inicio].obj, self, TO_VICT);
        act("$N kept $p in the closet to pay your rent service.", FALSE, self, buf_player[fila_inicio].obj, self, TO_ROOM);
        //extract_obj(buf_player[fila_inicio].obj);
         obj_to_room(buf_player[fila_inicio].obj, real_room(502));
      }
      if (fila_inicio == 100){
          fila_inicio = 0;
      }
      else {
          fila_inicio++;
      }
    }
    return (1);
  }
  return (0);
}


SPECIAL(winddragon)
{
  int k, found;
  struct char_data *targ;
  struct char_data *dragao;

  if (CMD_IS("mount")) {

     skip_spaces(&argument);


     if (!AWAKE(ch)) {
       sprintf(buf, "Stand first to mount any.\r\n");
       send_to_char(buf, ch);
       return (1);
     }

     if (!*argument) {
       sprintf(buf, "Mount what?.\r\n");
       send_to_char(buf, ch);
       return (1);
     }

    if (is_abbrev(argument,"dragon")){

      if IS_NPC(ch) {
      	return 0;
      }

      k = real_room(DRAGONROOM_INI);
      dragao = NULL;
      for(targ = world[k].people; targ; targ = targ->next_in_room)
       if (IS_NPC(targ) && (GET_MOB_VNUM(targ) == DRAGON_VNUM) && (dragao == NULL)) {
       	dragao = targ;
       }
      if (dragao == NULL) {
       sprintf(buf, "I don't see any dragon here.\r\n");
       send_to_char(buf, ch);
       return (1);
      }

      if (IS_AFFECTED(ch, AFF_FLY)){
       sprintf(buf, "You don't mount while flying.\r\n");
       send_to_char(buf, ch);
       return (1);
      }

      k=0;
      found=0;
      while ((!found) && (k<=NUM_DRAGONS)){
      	if (vet_dragons[k] == 0){
      	  vet_dragons[k] = 1;
          buf_dragon_indo[k].player = ch;
          buf_dragon_indo[k].room = 0;
          found = 1;
        }
        k++;
      }
      act("You mount $n and starts fly.", FALSE, dragao, 0, ch, TO_VICT);
      act("$N mount $n and starts fly to the sky.", TRUE, dragao, 0, ch, TO_NOTVICT);

      extract_char(dragao);

      char_from_room(ch);
      char_to_room(ch, real_room(to_fly_rooms[0]));
      look_at_room(ch, 0);
      act("$n flies from down mounting a winged dragon.", TRUE, buf_dragon_indo[k-1].player, 0, 0, TO_NOTVICT);

      return 1;
    }
    else{
     sprintf(buf, "You don't mount this.\r\n");
     send_to_char(buf, ch);
     return (1);
    }

  }

  return 0;

}

SPECIAL(windroom)
{
  int k;
  struct char_data *mob;

   if (IS_NPC(ch) || (GET_LEVEL(ch) >= LVL_IMMORT))  {
      	return 0;
      }

  if (CMD_IS("unmount")) {

     skip_spaces(&argument);

  if (!is_abbrev(argument,"dragon")){
       sprintf(buf, "Unmount what?.\r\n");
       send_to_char(buf, ch);
       return (1);
     }

     act("$n unmount the dragon and falls to death.", TRUE, ch, 0, 0, TO_NOTVICT);
     act("The dragon go away.", TRUE, ch, 0, 0, TO_NOTVICT);
     for(k=0;k <= NUM_DRAGONS;k++)
      if (vet_dragons[k] == 1)
       if (ch->desc->descriptor == buf_dragon_indo[k].player->desc->descriptor)
        vet_dragons[k] = 0;

     mob = read_mobile(real_mobile(DRAGON_VNUM), REAL);
     char_to_room(mob, real_room(DRAGONROOM_INI));
     send_to_room("A Dragon comes from sky.\n\r", real_room(DRAGONROOM_INI));
     char_from_room(ch);
     char_to_room(ch, real_room(DRAGONROOM_FALL));
     look_at_room(ch, 0);
     log_death_trap(ch);
     die_in_dt(ch);
     return (1);
  }

  if (CMD_IS("mount")) {
     sprintf(buf, "You already mount a dragon.\r\n");
     send_to_char(buf, ch);
     return (1);
  }

  if (CMD_IS("quit")) {
     sprintf(buf, "You don't quit while flying mount a dragon.\r\n");
     send_to_char(buf, ch);
     return (1);
  }

  if (CMD_IS("recall")) {
     sprintf(buf, "You don't recall while flying mount a dragon.\r\n");
     send_to_char(buf, ch);
     return (1);
  }

  if (CMD_IS("cast")) {
     sprintf(buf, "You are not concentrated enough to cast spells.\r\n");
     send_to_char(buf, ch);
     return (1);
  }

  if (CMD_IS("recite")) {
     sprintf(buf, "You are not concentrated enough to recite.\r\n");
     send_to_char(buf, ch);
     return (1);
  }

  if (CMD_IS("use")) {
     sprintf(buf, "You are not concentrated enough to use any.\r\n");
     send_to_char(buf, ch);
     return (1);
  }

  if (CMD_IS("quaff")) {
     sprintf(buf, "You are not concentrated enough to quaff any.\r\n");
     send_to_char(buf, ch);
     return (1);
  }

  if (CMD_IS("kill")) {
     sprintf(buf, "Kill anyone flying in a dragon??Isn't a good idea.\r\n");
     send_to_char(buf, ch);
     return (1);
  }

  if (CMD_IS("hit")) {
     sprintf(buf, "Hit anyone flying in a dragon??Isn't a good idea.\r\n");
     send_to_char(buf, ch);
     return (1);
  }

  if (CMD_IS("kick")) {
     sprintf(buf, "Kick anyone flying in a dragon??Are you crazy?.\r\n");
     send_to_char(buf, ch);
     return (1);
  }

  if (CMD_IS("backstab")) {
     sprintf(buf, "Backstab anyone flying in a dragon??Isn't a good idea.\r\n");
     send_to_char(buf, ch);
     return (1);
  }

  if (CMD_IS("bash")) {
     sprintf(buf, "Bash anyone flying in a dragon??Are you crazy?\r\n");
     send_to_char(buf, ch);
     return (1);
  }

  if (CMD_IS("murder")) {
     sprintf(buf, "Murder anyone flying in a dragon??Isn't a good idea.\r\n");
     send_to_char(buf, ch);
     return (1);
  }

  if (CMD_IS("steal")) {
     sprintf(buf, "Steal anyone flying in a dragon??Isn't a good idea.\r\n");
     send_to_char(buf, ch);
     return (1);
  }

  if (CMD_IS("rest")) {
     sprintf(buf, "Can you fly by yourself? wait here...\r\n");
     send_to_char(buf, ch);
     return (1);
  }

  if (CMD_IS("sleep")) {
     sprintf(buf, "Can you fly by yourself? wait here...\r\n");
     send_to_char(buf, ch);
     return (1);
  }

  if (CMD_IS("sit")) {
     sprintf(buf, "Can you fly by yourself? wait here...\r\n");
     send_to_char(buf, ch);
     return (1);
  }

  if (CMD_IS("stand")) {
     sprintf(buf, "Can you fly by yourself? wait here...\r\n");
     send_to_char(buf, ch);
     return (1);
  }

  if (CMD_IS("meditate")) {
     sprintf(buf, "Can you fly by yourself? wait here...\r\n");
     send_to_char(buf, ch);
     return (1);
  }

  if (CMD_IS("follow")) {
     sprintf(buf, "Say to the dragon follow.\r\n");
     send_to_char(buf, ch);
     return (1);
  }

  if (CMD_IS("hide")) {
     sprintf(buf, "Hide where?Below of the dragon wings?\r\n");
     send_to_char(buf, ch);
     return (1);
  }

  if (CMD_IS("gas")) {
     sprintf(buf, "Hide where?Below of the dragon wings?\r\n");
     send_to_char(buf, ch);
     return (1);
  }

  if (CMD_IS("take")) {
     sprintf(buf, "Take the clouds?\r\n");
     send_to_char(buf, ch);
     return (1);
  }

  if (CMD_IS("drop")) {
     sprintf(buf, "Drop any in the clouds?\r\n");
     send_to_char(buf, ch);
     return (1);
  }

  if (CMD_IS("war")) {
     sprintf(buf, "Will it be that the dragon wants war too?\r\n");
     send_to_char(buf, ch);
     return (1);
  }

  if (CMD_IS("harakiri")) {
     sprintf(buf, "Well, do you think the suicide is the best choice??\r\n");
     send_to_char(buf, ch);
     return (1);
  }

  return (0);
}


SPECIAL(mayor)
{
  const char open_path[] =
        "W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";
  const char close_path[] =
        "W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

  static const char *path;
  static int index;
  static bool move = FALSE;

  if (!move) {
    if (time_info.hours == 6) {
      move = TRUE;
      path = open_path;
      index = 0;
    } else if (time_info.hours == 20) {
      move = TRUE;
      path = close_path;
      index = 0;
    }
  }
  if (cmd || !move || (GET_POS(ch) < POS_SLEEPING) ||
      (GET_POS(ch) == POS_FIGHTING))
    return FALSE;

  switch (path[index]) {
  case '0':
  case '1':
  case '2':
  case '3':
    perform_move(ch, path[index] - '0', 1);
    break;

  case 'W':
    GET_POS(ch) = POS_STANDING;
    act("$n awakens and groans loudly.", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'S':
    GET_POS(ch) = POS_SLEEPING;
    act("$n lies down and instantly falls asleep.", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'a':
    act("$n says 'Hello Honey!'", FALSE, ch, 0, 0, TO_ROOM);
    act("$n smirks.", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'b':
    act("$n says 'What a view!  I must get something done about that dump!'",
        FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'c':
    act("$n says 'Vandals!  Youngsters nowadays have no respect for anything!'",
        FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'd':
    act("$n says 'Good day, citizens!'", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'e':
    act("$n says 'I hereby declare the bazaar open!'", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'E':
    act("$n says 'I hereby declare Wardome city closed!'", FALSE, ch, 0, 0, TO_ROOM);
    break;

  case 'O':
   // do_gen_door(ch, "gate", 0, SCMD_UNLOCK);
    do_gen_door(ch, "gate", 0, SCMD_OPEN);
    break;

  case 'C':
    do_gen_door(ch, "gate", 0, SCMD_CLOSE);
   // do_gen_door(ch, "gate", 0, SCMD_LOCK);
    break;

  case '.':
    move = FALSE;
    break;

  }

  index++;
  return FALSE;
}


/* ********************************************************************
*  General special procedures for mobiles                             *
******************************************************************** */


void npc_steal(struct char_data * ch, struct char_data * victim)
{
  int gold;

  if (IS_NPC(victim))
    return;
  if (GET_LEVEL(victim) >= LVL_IMMORT)
    return;

  if (AWAKE(victim) && (number(0, (GET_LEVEL(ch)/2)) == 0)) {
    act("You discover that $n has $s hands in your wallet.", FALSE, ch, 0, victim, TO_VICT);
    act("$n tries to steal gold from $N.", TRUE, ch, 0, victim, TO_NOTVICT);
  } else {
    /* Steal some gold coins */
    gold = (int) ((GET_GOLD(victim) * number(10, 40)) / 100);
    if (gold > 0) {
      GET_GOLD(ch) += gold;
      GET_GOLD(victim) -= gold;
    }
  }
}

SPECIAL(destructor)
{
 if (cmd)
   return FALSE ;

  static int tempo = 0 ;

 if (tempo > 50)
 {
  extract_char(ch);
  tempo = 0 ;
 }
 else
  tempo++ ;
  
 return TRUE ; 

}



SPECIAL(snake)
{
  if (cmd)
    return FALSE;

  if (GET_POS(ch) != POS_FIGHTING)
    return FALSE;

  if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room) &&
      (number(0, (((LVL_IMPL + 10) - GET_LEVEL(ch)) /3)) == 0)) {
    act("$n bites $N!", 1, ch, 0, FIGHTING(ch), TO_NOTVICT);
    act("$n bites you!", 1, ch, 0, FIGHTING(ch), TO_VICT);
    call_magic(ch, FIGHTING(ch), 0, SPELL_POISON, GET_LEVEL(ch), CAST_SPELL);
    return TRUE;
  }
  return FALSE;
}


SPECIAL(thief)
{
  struct char_data *cons;

  if (cmd)
    return FALSE;

  if (GET_POS(ch) != POS_STANDING)
    return FALSE;

  for (cons = world[ch->in_room].people; cons; cons = cons->next_in_room)
    if (!IS_NPC(cons) && (GET_LEVEL(cons) < LVL_IMMORT) && (!number(0, 3))) {
      npc_steal(ch, cons);
      return TRUE;
    }
  return FALSE;
}


SPECIAL(magic_user)
{
  struct char_data *vict;
  int a, b;

  if (cmd || GET_POS(ch) != POS_FIGHTING || GET_LEVEL(ch) < 10)
	  return FALSE;

  /* pseudo-randomly choose someone in the room who is fighting me */
  for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
	  if (FIGHTING(vict) == ch && !number(0, 4))
		  break;

  /* if I didn't pick any of those, then just slam the guy I'm fighting */
  if (vict == NULL && IN_ROOM(FIGHTING(ch)) == IN_ROOM(ch))
	  vict = FIGHTING(ch);

  /* Hm...didn't pick anyone...I'll wait a round. */
  if (vict == NULL)
	  return TRUE;

  if (number(0, 4))
	  return TRUE;

  if ((number(0, GET_REMORT(ch)) > 5) && (GET_LEVEL(ch) > 130)) {
	  cast_spell(ch, vict, NULL, SPELL_ULTIMA);
	  return TRUE;
  } else if ((number(0, GET_REMORT(ch)) > 3) && (GET_LEVEL(ch) > 80)) {
	  cast_spell(ch, vict, NULL, SPELL_SUMMON_BAHAMUT);
	  return TRUE;
  }

  b = (int) (GET_LEVEL(ch) / 8);
  a = (int) (GET_LEVEL(ch) / 11);

  b += GET_REMORT(ch);
  a += GET_REMORT(ch);
  
  b = ((b > 22) ? 22 : b);
  a = ((a > 21) ? 21 : a);

  switch (number(a, b)) {
  case 1:
	  cast_spell(ch, vict, NULL, SPELL_METEOR_SHOWER);
  break;
  case 2:
	  cast_spell(ch, vict, NULL, SPELL_ACIDARROW);
  break;
  case 3:
	  cast_spell(ch, vict, NULL, SPELL_BLADEBARRIER);
  break;
  case 4:
	  cast_spell(ch, vict, NULL, SPELL_FLAMEARROW);
  break;
  case 5:
	  cast_spell(ch, vict, NULL, SPELL_GREASE);
  break;
  case 6:
	  cast_spell(ch, vict, NULL, SPELL_MAGIC_MISSILE);
  break;
  case 7:
	  cast_spell(ch, vict, NULL, SPELL_CROMATIC_ORB);
  break;
  case 8:
	  cast_spell(ch, vict, NULL, SPELL_CHILL_TOUCH);
  break;
  case 9:
	  cast_spell(ch, vict, NULL, SPELL_AREA_LIGHTNING);
  break;
  case 10:
	  cast_spell(ch, vict, NULL, SPELL_BURNING_HANDS);
  break;
  case 11:
	  cast_spell(ch, vict, NULL, SPELL_ELETRICSTORM);
  break;
  case 12:
	  cast_spell(ch, vict, NULL, SPELL_SHOCKING_GRASP);
  break;
  case 13:
	  cast_spell(ch, vict, NULL, SPELL_ICESTORM);
  break;
  case 14:
	  cast_spell(ch, vict, NULL, SPELL_COLOR_SPRAY);
  break;
  case 15:
	  cast_spell(ch, vict, NULL, SPELL_FIRESTORM);
  break;
  case 16:
	  cast_spell(ch, vict, NULL, SPELL_LIGHTNING_BOLT);
  break;
  case 17:
	  cast_spell(ch, vict, NULL, SPELL_MANA_BURN);
  break;
  case 18:
	  cast_spell(ch, vict, NULL, SPELL_METEORSTORM);
  break;
  case 19:
	  cast_spell(ch, vict, NULL, SPELL_GLACIAL_CONE);
  break;
  case 20:
	  cast_spell(ch, vict, NULL, SPELL_PRISMATIC_SPHERE);
  break;
  case 21:
	  cast_spell(ch, vict, NULL, SPELL_FIREBALL); 
  break;
  case 22:
	  cast_spell(ch, vict, NULL, SPELL_DELAYED_FIREBALL);
  break;
  default:
    cast_spell(ch, vict, NULL, SPELL_METEOR_SHOWER);
  break;
  }

  return TRUE;
}

void mob_kickflip(struct char_data *mob, struct char_data *vict)
{
  int percent, prob;

  if (GET_POS(mob) < POS_FIGHTING)
	return;
 
  percent = number(1, 105);	/* 109% is a complete failure */
  prob = (GET_LEVEL(mob)) + (15 + con_app[GET_DEX(mob)].hitp)*2;


  if (!IS_NPC(vict) && number(1, 100) < ((GET_SKILL(vict, SKILL_SNAKE_FIGHT)/2) + GET_DEX(vict))) {
	  act("You avoid $n's attack by moving like a snake!", FALSE, mob, 0, vict, TO_VICT);
	  act("$N avoids your attack by moving like a snake!", FALSE, mob, 0, vict, TO_CHAR);
	  act("$N avoids $n's attack by moving like a snake!", FALSE, mob, 0, vict, TO_ROOM | TO_NOTVICT);
	  percent = 101;
  }

  if (!IS_NPC(vict) && (defender(vict, mob)))
	  return;

  if (MOB_FLAGGED(vict, MOB_NOBASH))
    percent = 1000;

  if (percent > prob) {
    GET_POS(mob) = POS_SITTING;
    damage(vict, mob, GET_LEVEL(vict)*2, SKILL_KICKFLIP);
  } else {
        if (IN_ROOM(mob) == IN_ROOM(vict)) {
        GET_POS(vict) = POS_SITTING;
        damage(mob, vict, GET_LEVEL(mob), SKILL_KICKFLIP);
        WAIT_STATE(vict, PULSE_VIOLENCE * 2);
      }
  }
}

void mob_kick(struct char_data *mob, struct char_data *vict)
{
  int percent, prob;
  sh_int x=GET_LEVEL(mob);

  percent = ((10 - (GET_AC(vict) / 10)) * 2) + number(1, 101);  /* 101% is a complete
                                                                 * failure */
  prob = x + 60;

  if (AWAKE(vict) && percent > prob) {
    damage(mob, vict, 0, SKILL_KICK);
  } else
   { 
    if(GET_REMORT(mob))
    {
     damage(mob, vict, ((GET_LEVEL(mob) + 10)*2) + (GET_REMORT(mob)*35), SKILL_KICK);
    }
    else 
     damage(mob, vict, GET_LEVEL(mob) + 10 + (GET_REMORT(mob)*35), SKILL_KICK);
   }
}

void mob_howl(struct char_data *mob, struct char_data *vict)
{
  sh_int chance;

  act("&RAaAahHhHh!! &y$n &RHOWLS &yout loud! &RAaAahHhHh!!&n", TRUE, mob, 0, 0, TO_ROOM);
	
  if ((GET_REMORT(vict)) && (GET_REMORT(mob) < (GET_REMORT(vict)+1)))
  	return;
	
  chance = GET_LEVEL(mob) - GET_LEVEL(vict) + 25;

  if ((chance > 0) && (number(1,100) < 80))
	do_flee(vict, NULL, 0, 0);
}

void mob_headbash(struct char_data *mob, struct char_data *vict)
{
  int percent, prob;

  percent = number(1, 140);     /* 101% is a complete failure */
  prob = GET_LEVEL(mob)/2;

  if (!IS_NPC(vict) && number(1, 140) < ((GET_SKILL(vict, SKILL_SNAKE_FIGHT)/2) + GET_DEX(vict))) {
	  act("You avoid $n's attack by moving like a snake!", FALSE, mob, 0, vict, TO_VICT);
	  act("$N avoids your attack by moving like a snake!", FALSE, mob, 0, vict, TO_CHAR);
	  act("$N avoids $n's attack by moving like a snake!", FALSE, mob, 0, vict, TO_ROOM | TO_NOTVICT);
	  percent = 101;

  }

  if (!IS_NPC(vict) && (defender(vict, mob)))
	  return;

  if (MOB_FLAGGED(vict, MOB_NOBASH))
    percent = 101;

  if (percent > prob ) {
     act("$n miss the power hit.", TRUE, mob, 0, 0, TO_ROOM);
   } else {
    WAIT_STATE(vict, PULSE_VIOLENCE);
    if (IN_ROOM(mob) == IN_ROOM(vict)) {
       act("You hit $S head forcing a fainting.", TRUE, mob, 0, FIGHTING(mob), TO_CHAR);
       act("$n hits your head and you fall down fainted! ZzZzzZZz.", TRUE, mob, 0, FIGHTING(mob), TO_VICT);
       act("$n hits $N's head forcing a fainting.", TRUE, mob, 0, FIGHTING(mob), TO_NOTVICT);
       GET_POS(vict) = POS_SLEEPING;
    }
  }
}

SPECIAL(ninja)
{
  struct char_data *vict;

  if (cmd || GET_POS(ch) != POS_FIGHTING || GET_LEVEL(ch) < 10)
	  return FALSE;

  /* pseudo-randomly choose someone in the room who is fighting me */
  for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
	  if (FIGHTING(vict) == ch && !number(0, 4))
		  break;

  /* if I didn't pick any of those, then just slam the guy I'm fighting */
  if (vict == NULL && IN_ROOM(FIGHTING(ch)) == IN_ROOM(ch))
	  vict = FIGHTING(ch);

  /* Hm...didn't pick anyone...I'll wait a round. */
  if (vict == NULL)
	  return TRUE;

  if (number(0, 2))
	  return TRUE;

  if (GET_LEVEL(ch) > 44)
	mob_kickflip(ch, vict);
  else
	mob_kick(ch, vict);

  return TRUE;
}

SPECIAL(barbarian)
{
  struct char_data *vict;

  if (cmd || GET_POS(ch) != POS_FIGHTING || GET_LEVEL(ch) < 10)
	  return FALSE;

  /* pseudo-randomly choose someone in the room who is fighting me */
  for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
	  if (FIGHTING(vict) == ch && !number(0, 4))
		  break;

  /* if I didn't pick any of those, then just slam the guy I'm fighting */
  if (vict == NULL && IN_ROOM(FIGHTING(ch)) == IN_ROOM(ch))
	  vict = FIGHTING(ch);

  /* Hm...didn't pick anyone...I'll wait a round. */
  if (vict == NULL)
	  return TRUE;

  if (number(0, 2))
	  return TRUE;

  if ((!IS_AFFECTED(ch, AFF_BERZERK)) && (GET_LEVEL(ch) > 169)) {
	act("$n works up a frenzy and charges you!", TRUE, ch, 0, FIGHTING(ch), TO_VICT);
    	act("$n works up a frenzy and charges $N.", TRUE, ch, 0, FIGHTING(ch), TO_ROOM);
    	SET_BIT(AFF_FLAGS(ch), AFF_BERZERK);
	return TRUE;
  }

  if (GET_LEVEL(ch) < 135)
	if (!number(0,3))
		mob_howl(ch, vict);
	else
		mob_kick(ch, vict);
  else
	mob_headbash(ch,vict);
  
  return TRUE;
}


/* ********************************************************************
*  Special procedures for mobiles                                      *
******************************************************************** */

SPECIAL(puff)
{
  if (cmd)
    return (0);

  switch (number(0, 60)) {
  case 0:
    do_say(ch, "My god!  It's full of stars!", 0, 0);
    return (1);
  case 1:
    do_say(ch, "How'd all those fish get up here?", 0, 0);
    return (1);
  case 2:
    do_say(ch, "I'm a very female dragon.", 0, 0);
    return (1);
  case 3:
    do_say(ch, "I've got a peaceful, easy feeling.", 0, 0);
    return (1);
  case 4:
    do_say(ch, "I don't wanna play another MUD. Wardome is the best!", 0, 0);
    return (1);
  default:
    return (0);
  }
}



SPECIAL(fido)
{

  struct obj_data *i, *temp, *next_obj;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  for (i = world[ch->in_room].contents; i; i = i->next_content) {
    if (IS_CORPSE(i)) {
      act("$n savagely devours a corpse.", FALSE, ch, 0, 0, TO_ROOM);
      for (temp = i->contains; temp; temp = next_obj) {
        next_obj = temp->next_content;
        obj_from_obj(temp);
        obj_to_room(temp, ch->in_room);
      }
      extract_obj(i);
      return (TRUE);
    }
  }
  return (FALSE);
}



SPECIAL(janitor)
{
  struct obj_data *i;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  for (i = world[ch->in_room].contents; i; i = i->next_content) {
    if (!CAN_WEAR(i, ITEM_WEAR_TAKE))
      continue;
    if (GET_OBJ_TYPE(i) != ITEM_DRINKCON && GET_OBJ_COST(i) >= 15)
      continue;
    act("$n picks up some trash.", FALSE, ch, 0, 0, TO_ROOM);
    obj_from_room(i);
    obj_to_char(i, ch);
    return TRUE;
  }

  return FALSE;
}


SPECIAL(cityguard)
{
  struct char_data *tch, *evil;
  int max_evil;

  if (cmd || !AWAKE(ch) || FIGHTING(ch))
    return FALSE;

  max_evil = 1000;
  evil = 0;

  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
    if (!IS_NPC(tch) && CAN_SEE(ch, tch) && PLR_FLAGGED(tch, PLR_KILLER)) {
      act("$n screams 'HEY!!!  You're one of those PLAYER KILLERS!!!!!!'", FALSE, ch, 0, 0, TO_ROOM);
      hit(ch, tch, TYPE_UNDEFINED);
      return (TRUE);
    }
  }

  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
    if (!IS_NPC(tch) && CAN_SEE(ch, tch) && PLR_FLAGGED(tch, PLR_THIEF)){
      act("$n screams 'HEY!!!  You're one of those PLAYER THIEVES!!!!!!'", FALSE, ch, 0, 0, TO_ROOM);
      hit(ch, tch, TYPE_UNDEFINED);
      return (TRUE);
    }
  }

  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
    if (CAN_SEE(ch, tch) && FIGHTING(tch)) {
      if ((GET_ALIGNMENT(tch) < max_evil) &&
          (IS_NPC(tch) || IS_NPC(FIGHTING(tch)))) {
        max_evil = GET_ALIGNMENT(tch);
        evil = tch;
      }
    }
  }

  if (evil && (GET_ALIGNMENT(FIGHTING(evil)) >= 0)) {
    act("$n screams 'PROTECT THE INNOCENT!  BANZAI!  CHARGE!  ARARARAGGGHH!'", FALSE, ch, 0, 0, TO_ROOM);
    hit(ch, evil, TYPE_UNDEFINED);
    return (TRUE);
  }
  return (FALSE);
}


#define MSG_GYM \
"THE PIT BULL GYM\r\n" \
"\r\n" \
"Here you can train your phisical abilities.\r\n" \
"Type META <field> to train a specific item.\r\n" \
"\r\n" \
"You can train the following:\r\n" \
"  Field          Cost\r\n" \
"  -----------    ------------------------\r\n" \
"  &Cconstitution&n    &c%2d&n points and &y%5s gold\r\n" \
"  &Chealth  &n        &c%2d&n points and &y%5s gold\r\n" \
"  &Cmove &n           &c%2d&n points and &y%5s gold\r\n" \
"  &Cstrength &n      &c%2d&n points and &y%5s&n gold\r\n" \
"  &Cdexterity&n      &c%2d&n points and &y%5s gold\r\n" \
"\r\n" \
"  &Ccheck &n         &c%2d&n points and &y%5s&n gold\r\n" \
"\r\n" \
"TIP: You can train mana, inteligence and wisdom on the Cryogenic center." \
"\r\n"

SPECIAL(gym)
{
               /*   Hp   Mv   Str   Dex   Not Con*/
  int costs[] = {   3,   2,   100,   100,   0, 100 };
  int money[] = {1000, 500, 2500, 2500, 300, 1000 };
  const int gains[] = {  15,   7,    1,    1,   0, 1 };
  int mode;

  if (IS_NPC(ch) || !CMD_IS("meta"))
    return 0;

  if ((GET_LEVEL(ch) + (200*GET_REMORT(ch))) < 50)
  {
   money[0] = 0 ;
   money[1] = 0 ;
   money[4] = 0 ;
 }

  if(ch->real_abils.str < 18)   costs[2] = 50 ;
  if(ch->real_abils.dex < 18) costs[3] = 50 ;
  if(ch->real_abils.con < 18) costs[5] = 50 ;

  if(ch->real_abils.str > 50)   costs[2] = 200 ;
  if(ch->real_abils.dex > 50) costs[3] = 200 ;
  if(ch->real_abils.con > 50) costs[5] = 200 ;

  skip_spaces(&argument);

  if (!*argument) { /* no argument, just display the options. */
    sprintf(buf, MSG_GYM, costs[5], add_points(money[5]), costs[0], add_points(money[0]), 
     costs[1], add_points(money[1]), costs[2], add_points(money[2]), costs[3], 
     add_points(money[3]),costs[4], add_points(money[4]));
    send_to_char(buf, ch);
    return 1;
  }

  if (!strcmp(argument, "health") || !strcmp(argument, "hp"))
    mode = 0;
  else if (!strcmp(argument, "move") || !strcmp(argument, "mv"))
    mode = 1;
  else if (!strcmp(argument, "strength") || !strcmp(argument, "str"))
    mode = 2;
  else if (!strcmp(argument, "dexterity") || !strcmp(argument, "dex"))
    mode = 3;
  else if (!strcmp(argument, "check") || !strcmp(argument, "che"))
    mode = 4;
  else if (!strcmp(argument, "constitution") || !strcmp(argument, "con"))
    mode = 5;
  else if (!strcmp(argument, "mana") || !strcmp(argument, "mn") ||
           !strcmp(argument, "wisdom") || !strcmp(argument, "wis") ||
           !strcmp(argument, "inteligence") || !strcmp(argument, "int")) {
    send_to_char("Find a Temple to train mana, inteligence or wisdom.\r\n", ch);
    return 1;
  }
  else {
    send_to_char("Invalid training type.\r\n", ch);
    return 1;
  }

  if (GET_POINTS(ch) < costs[mode]) {
    send_to_char("You don't have enough points!\r\n", ch);
    return 1;
  }
  if (GET_GOLD(ch) < money[mode]) {
    send_to_char("You don't have enough money!\r\n", ch);
    return 1;
  }

  switch (mode) {
    case 0: /* health */
      GET_MAX_HIT(ch) += 15 + con_app[ch->real_abils.con].hitp;
      GET_HIT(ch) += 15 + con_app[ch->real_abils.con].hitp;
      save_char(ch, NOWHERE);
      send_to_char("Your vitality increases!\r\n", ch);
      break;
    case 1: /* move */
      GET_MAX_MOVE(ch) += (15 + con_app[ch->real_abils.dex].hitp) ;
      GET_MOVE(ch) += (15 + con_app[ch->real_abils.dex].hitp) ;
      save_char(ch, NOWHERE);
      send_to_char("Your movement points increases!\r\n", ch);
      break;
    case 2: /* str */
      if (ch->real_abils.str  >= 100) {
          send_to_char("You are already strong enough.\r\n", ch);
          return 1;
        }
      ch->real_abils.str += gains[mode];
      save_char(ch, NOWHERE);
      send_to_char("You feel stronger a lot!\r\n", ch);
      break;
    case 3: /* dex */
      if (ch->real_abils.dex >= 100) {
        send_to_char("You don't need to train your dexterity.\r\n", ch);
        return 1;
      }
      ch->real_abils.dex += gains[mode];
      save_char(ch, NOWHERE);
      send_to_char("You feel more skillful.\r\n", ch);
      break;
    case 4: /* not */
      send_to_char("You MAX stats are:\r\n", ch);
      sprintf(buf, " &CHp&c[&r%s&c] &CMv&c[&r%d&c]&n\r\n", add_points(GET_MAX_HIT(ch)), (GET_MAX_MOVE(ch)));
      sprintf(buf + strlen(buf), " &CStr&c[&r%d&c] &CDex&c[&r%d&c] &CCon&c[&r%d&c]&n\r\n", (GET_STR(ch)), (GET_DEX(ch)), (GET_CON(ch)));
      send_to_char(buf, ch);
      save_char(ch, NOWHERE);
      break;
    case 5: /* con */
      if (ch->real_abils.con >= 100) {
        send_to_char("You don't need to train your con.\r\n", ch);
        return 1;
      }
      ch->real_abils.con += gains[mode];
      save_char(ch, NOWHERE);
      send_to_char("You feel more robust.\r\n", ch);
      break;
  }
  GET_POINTS(ch) -= costs[mode];
  GET_POINTS_GS(ch) += costs[mode];
  GET_GOLD(ch) -= money[mode];

  act("$n trains in the Gym.", TRUE, ch, 0, FALSE, TO_ROOM);
  return 1;
}

#define MSG_TEMPLE \
"THE CRYOGENIC CENTER\r\n" \
"\r\n" \
"Here you can train your mental abilities.\r\n" \
"Type META <field> to train a specific item.\r\n" \
"\r\n" \
"You can train the following:\r\n" \
"  Field          Cost\r\n" \
"  -----------    ------------------------\r\n" \
"  &Ccharisma&n       &c%2d&n points and &y%5s&n gold\r\n" \
"  &Cmana&n           &c%2d&n points and &y%5s&n gold\r\n" \
"  &Cinteligence&n    &c%2d&n points and &y%5s &ngold\r\n" \
"  &Cwisdom&n         &c%2d&n points and &y%5s&n gold\r\n" \
"\r\n" \
"  &Ccheck&n          &c%2d &npoints and &y%5s&n gold\r\n" \
"\r\n" \
"TIP: You can train health, movement, strength and dexterity on the Pit Bull Gym."

SPECIAL(temple)
{
                /*  Mn   Int   Wis  Not   Cha*/
  int costs[] = {   2,   100,   100,   0, 100 };
  int money[] = {1200, 2500, 2500, 300, 1000 };
  const int gains[] = {  15,    1,    1,   0, 1 };
  int mode;

  if (IS_NPC(ch) || !CMD_IS("meta"))
    return 0;

  if ((GET_LEVEL(ch) + (200*GET_REMORT(ch))) < 50)
  {
   money[0] = 0 ;
   money[1] = 0 ;
   money[4] = 0 ;
 }

  if(ch->real_abils.intel < 18) costs[1] = 50 ;
  if(ch->real_abils.wis < 18)   costs[2] = 50 ;
  if(ch->real_abils.cha < 18) costs[4] = 50 ;

  if(ch->real_abils.intel > 50) costs[1] = 200 ;
  if(ch->real_abils.wis > 50)   costs[2] = 200 ;
  if(ch->real_abils.cha > 50) costs[4] = 200 ;


  skip_spaces(&argument);

  if (!*argument) { /* no argument, just display the options. */
    sprintf(buf, MSG_TEMPLE, costs[4], add_points(money[4]),costs[0], 
      add_points(money[0]), costs[1], add_points(money[1]), costs[2], add_points(money[2]),
      costs[3], add_points(money[3]));
    send_to_char(buf, ch);
    return 1;
  }

  if (!strcmp(argument, "mana") || !strcmp(argument, "mn"))
    mode = 0;
  else if (!strcmp(argument, "inteligence") || !strcmp(argument, "int"))
    mode = 1;
  else if (!strcmp(argument, "wisdom") || !strcmp(argument, "wis"))
    mode = 2;
  else if (!strcmp(argument, "check") || !strcmp(argument, "che"))
    mode = 3;
  else if (!strcmp(argument, "cha") || !strcmp(argument, "charisma"))
    mode = 4;
  else if (!strcmp(argument, "health") || !strcmp(argument, "hp") ||
           !strcmp(argument, "movement") || !strcmp(argument, "mv") ||
           !strcmp(argument, "strength") || !strcmp(argument, "str") ||
           !strcmp(argument, "dexterity") || !strcmp(argument, "dex")) {
    send_to_char("Find a Gym to train health, movements, strength or dexterity.\r\n", ch);
    return 1;
  }
  else {
    send_to_char("Invalid training type.\r\n", ch);
    return 1;
  }

  if (GET_POINTS(ch) < costs[mode]) {
    send_to_char("You don't have enough points!\r\n", ch);
    return 1;
  }
  if (GET_GOLD(ch) < money[mode]) {
    send_to_char("You don't have enough money!\r\n", ch);
    return 1;
  }

  switch (mode) {
    case 0: /* mana */
      GET_MAX_MANA(ch) += 15 + con_app[(ch->real_abils.intel + 
ch->real_abils.wis)/2].hitp ;
      GET_MANA(ch) += 15 + con_app[(ch->real_abils.intel +
ch->real_abils.wis)/2].hitp ;
      save_char(ch, NOWHERE);
      send_to_char("Your magic power increases!\r\n", ch);
      break;
    case 1: /* inteligence */
      if (ch->real_abils.intel >= 100) {
        send_to_char("You are already Einstein.\r\n", ch);
        return 1;
      }
      ch->real_abils.intel += gains[mode];
      save_char(ch, NOWHERE);
      send_to_char("You feel yourself more intelligent.\r\n", ch);
      break;
    case 2: /* wisdom */
      if (ch->real_abils.wis >= 100) {
        send_to_char("You are already wise a lot.\r\n", ch);
        return 1;
      }
      ch->real_abils.wis += gains[mode];
      save_char(ch, NOWHERE);
      send_to_char("You have become more wise.\r\n", ch);
      break;
    case 3: /* not */
    send_to_char("Your MAX stats are:\r\n", ch);
      sprintf(buf, " &CMn&c[&r%s&c] &CCha&c[&r%d&c]&n\r\n", add_points(GET_MAX_MANA(ch)), (GET_CHA(ch)));
      sprintf(buf + strlen(buf), "&CInt&c[&r%d&c] &CWis&c[&r%d&c]&n\r\n", (GET_INT(ch)), (GET_WIS(ch)));
      send_to_char(buf, ch);
      save_char(ch, NOWHERE);
      break;

    case 4: /* cha */
      if (ch->real_abils.cha >= 100) {
        send_to_char("You are already charismatic a lot.\r\n", ch);
        return 1;
      }
      ch->real_abils.cha += gains[mode];
      save_char(ch, NOWHERE);
      send_to_char("You have become more charismatic.\r\n", ch);
      break;
  }
  GET_POINTS(ch) -= costs[mode];
  GET_POINTS_GS(ch) += costs[mode];
  GET_GOLD(ch) -= money[mode];

  act("$n meditates in the Temple.", TRUE, ch, 0, FALSE, TO_ROOM);
  return 1;
}

#define NUM_ARCHERS      1              /* # of rooms archers can shoot from */
#define NUM_TARGETS      3              /* # of rooms an archer can shoot at */
#define HIT_CHANCE       60             /* accuracy 60% chance to hit */
#define ARCHER_NUM_DICE  3              /*  archer damage dice */
#define ARCHER_SIZE_DICE 3              /*  archer does 3d3 each hit */

SPECIAL(archer)
{
  struct char_data *targ;
  int i, j, k;
  int damage;

  sh_int to_from_rooms[NUM_ARCHERS][NUM_TARGETS + 1] =
{
   /* archer room     target room #1     #2       #3 */
      { 523,              524,        6092,     6091}//,/* archer room #1 */
      //{ 3041,              3501,        3500,     -1} /* room #2 */
  };

  char *mssgs[] = {
    "You feel a sharp pain in your side as an arrow finds its mark!",
    "You hear a dull thud as an arrow pierces $N!",
    "An arrow whistles by your ear, barely missing you!",
    "An arrow narrowly misses $N!"
  };

  if(cmd)
    return FALSE;

  if(GET_POS(ch) != POS_STANDING)
    return FALSE;

  for(i = 0; i < NUM_ARCHERS; i++) {
    if(real_room(to_from_rooms[i][0]) == ch->in_room) {
      for(j = 1; j <= NUM_TARGETS; j++) {
        if((k = real_room(to_from_rooms[i][j])) >= 0) {
          for(targ = world[k].people; targ; targ = targ->next_in_room) {
            if(!IS_NPC(targ) && (GET_LEVEL(targ) < LVL_IMMORT) && (GET_LEVEL(targ) > 5) &&
              (!number(0, 2))) {
              if(number(1, 100) <= HIT_CHANCE) {
                act(mssgs[0], 1, ch, 0, targ, TO_VICT);
                act(mssgs[1], 1, ch, 0, targ, TO_NOTVICT);
                damage = (ARCHER_NUM_DICE *  ARCHER_SIZE_DICE);
                GET_HIT(targ) -= damage + (number(1,6));
                /*  these above numbers can be changed for different
                 *  damage levels.
                 */
                update_pos(targ);
                return TRUE;
              } else {
                act(mssgs[2], 1, ch, 0, targ, TO_VICT);
                act(mssgs[3], 1, ch, 0, targ, TO_NOTVICT);
                return TRUE;
              }
            }
          }
        }
      }
    }
  }
  return FALSE;
}

long PET_PRICE(char_data *pet)
{
  long price;

   if (GET_LEVEL(pet) <= 5)
    price =(GET_LEVEL(pet) * 400);
   else
    price =(GET_LEVEL(pet) * 1200);
   return(price);
}


SPECIAL(pet_shops)
{
  char buf[MAX_STRING_LENGTH], pet_name[256];
  int pet_room;
  struct char_data *pet;

  pet_room = real_room(557);

  if (CMD_IS("list")) {
    send_to_char("&RAvailable pets are:&n\r\n", ch);
    for (pet = world[pet_room].people; pet; pet = pet->next_in_room) {
       sprintf(buf, "&Y%8ld &C- &G%s&n\r\n", PET_PRICE(pet), GET_NAME(pet));
       send_to_char(buf, ch);
    }
    return (TRUE);
  } else if (CMD_IS("buy")) {

    argument = one_argument(argument, buf);
    argument = one_argument(argument, pet_name);

    if (!(pet = get_char_room(buf, pet_room))) {
      send_to_char("There is no such pet!\r\n", ch);
      return (TRUE);
    }
    if (GET_GOLD(ch) < PET_PRICE(pet)) {
      send_to_char("You don't have enough gold!\r\n", ch);
      return (TRUE);
    }

    if (!allow_follower(ch, 5)) {
	send_to_char("You can't have any more followers.\n\r", ch);
	return(TRUE);
      }

    GET_GOLD(ch) -= PET_PRICE(pet);

    pet = read_mobile(GET_MOB_RNUM(pet), REAL);
    GET_EXP(pet) = 0;
    SET_BIT(AFF_FLAGS(pet), AFF_CHARM);

    if (*pet_name) {
      sprintf(buf, "%s %s", pet->player.name, pet_name);
      /* free(pet->player.name); don't free the prototype! */
      pet->player.name = str_dup(buf);

      sprintf(buf, "%sA small sign on a chain around the neck says 'My name is %s'\r\n",
              pet->player.description, pet_name);
      /* free(pet->player.description); don't free the prototype! */
      pet->player.description = str_dup(buf);
    }
    char_to_room(pet, ch->in_room);
    add_follower(pet, ch);
    load_mtrigger(pet);

    /* Be certain that pets can't get/carry/use/wield/wear items */
    IS_CARRYING_W(pet) = 1000;
    IS_CARRYING_N(pet) = 100;

    send_to_char("May you enjoy your pet.\r\n", ch);
    act("$n buys $N as a pet.", FALSE, ch, 0, pet, TO_ROOM);

    return 1;
  }
  /* All commands except list and buy */
  return 0;
}


/*sundheaven*/

SPECIAL(silktrader)
{
  ACMD(do_say);

  if (cmd)
    return 0;

  if (world[ch->in_room].sector_type == SECT_CITY)
  switch (number(0, 30)) {
   case 0:
      act("$n eyes a passing woman.", FALSE, ch, 0, 0,TO_ROOM);
      do_say(ch, "Come, m'lady, and have a look at this precious silk!", 0, 0);
      return(1);
   case 1:
      act("$n says to you, 'Wouldn't you look lovely in this!'", FALSE, ch, 0, 0,TO_ROOM);
      act("$n shows you a gown of indigo silk.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 2:
      act("$n holds a pair of silk gloves up for you to inspect.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 3:
      act("$n cries out, 'Have at this fine silk from exotic corners of the world you will likely never see!", FALSE, ch, 0, 0,TO_ROOM);
      act("$n smirks.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 4:
      do_say(ch, "Step forward, my pretty locals!", 0, 0);
      return(1);
   case 5:
      act("$n shades his eyes with his hand.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 6:
      do_say(ch, "Have you ever seen an ogre in a silken gown?", 0, 0);
      do_say(ch, "I didn't *think* so!", 0, 0);
      act("$n throws his head back and cackles with insane glee!", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 7:
      act("$n hands you a glass of wine.", FALSE, ch, 0, 0,TO_ROOM);
      do_say(ch, "Come, have a seat and view my wares.", 0, 0);
      return(1);
   case 8:
      act("$n looks at you.", FALSE, ch, 0, 0,TO_ROOM);
      act("$n shakes his head sadly.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 9:
      act("$n fiddles with some maps.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 10:
      do_say(ch, "Here here! Beggars and nobles alike come forward and make your bids!", 0, 0);
      return(1);
   case 11:
      do_say(ch, "I am in this bourgeois hamlet for a limited time only!", 0, 0);
      act("$n swirls some wine in a glass.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
  }

  if (world[ch->in_room].sector_type != SECT_CITY)
  switch (number(0, 20)) {
   case 0:
      do_say(ch, "Ah! Fellow travellers! Come have a look at the finest silk this side of the infamous Ched Razimtheth!", 0, 0);
      return(1);
   case 1:
      act("$n looks at you.", FALSE, ch, 0, 0,TO_ROOM);
      do_say(ch, "You are feebly attired for the danger that lies ahead.", 0, 0);
      do_say(ch, "Silk is the way to go.", 0, 0);
      act("$n smiles warmly.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 2:
      do_say(ch, "Worthy adventurers, hear my call!", 0, 0);
      return(1);
   case 3:
      act("$n adjusts his cloak.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 4:
      act("$n says to you, 'Certain doom awaits you, therefore shall you die in silk.'", FALSE, ch, 0, 0,TO_ROOM);
      act("$n bows respectfully.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 5:
      do_say(ch, "Can you direct me to the nearest tavern?", 0, 0);
      return(1);
   case 6:
      do_say(ch, "Heard the latest ogre joke?", 0, 0);
      act("$n snickers to himself.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 7:
      do_say(ch, "What ho, traveller! Rest your legs here for a spell and peruse the latest in fashion!", 0, 0);
      return(1);
   case 8:
      do_say(ch, "Beware ye, traveller, lest ye come to live in Exile!", 0, 0);
      act("$n grins evilly.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 9:
      act("$n touches your shoulder.", FALSE, ch, 0, 0,TO_ROOM);
      do_say(ch, "A word of advice. Beware of any ale labled 'mushroom' or 'pumpkin'.", 0, 0);
      act("$n shivers uncomfortably.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);

  }
  return(0);
}


SPECIAL(athos)
{
  ACMD(do_say);

  if(cmd)
   return 0;
    switch (number(0, 20)) {
    case 0:
      act("$n gazes into his wine gloomily.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
    case 1:
      act("$n grimaces.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
    case 2:
      act("$n asks you, 'Have you seen the lady, pale and fair, with a heart of stone?'", FALSE, ch, 0, 0,TO_ROOM);
      do_say(ch, "That monster will be the death of us all.", 0, 0);
      return(1);
    case 3:
      do_say(ch, "God save the King!", 0, 0);
      return(1);
    case 4:
      do_say(ch, "All for one and .. one for...", 0, 0);
      act("$n drowns himself in a swig of wine.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
    case 5:
      act("$n looks up with a philosophical air.", FALSE, ch, 0, 0,TO_ROOM);
      do_say(ch, "Women - God's eternal punishment on man.", 0, 0);
      return(1);
    case 6:
      act("$n downs his glass and leans heavily on the oaken table.", FALSE, ch, 0, 0,TO_ROOM);
      do_say(ch, "You know, we would best band together and wrestle the monstrous woman from her lair and home!", 0, 0);
      return(1);
  default: return(FALSE);
                break; }
    return(0);
}


/*
SPECIAL(hangman)
{
ACMD(do_say);
if(cmd) return 0;
  switch (number(0, 15)) {
  case 0:
    act("$n whirls his noose like a lasso and it lands neatly around your neck.", FALSE, ch, 0, 0,TO_ROOM);
    do_say(ch, "You're next, you ugly rogue!", 0, 0);
    do_say(ch, "Just kidding.", 0, 0);
    act("$n pats you on your head.", FALSE, ch, 0, 0,TO_ROOM);
    return(1);
  case 1:
    do_say(ch, "I was conceived in Exile and have been integrated into society!", 0, 0);
    do_say(ch, "Muahaha!", 0, 0);
    return(1);
  case 2:
    do_say(ch, "Anyone have a butterknife I can borrow?", 0, 0);
    return(1);
  case 3:
    act("$n suddenly pulls a lever.", FALSE, ch, 0, 0,TO_ROOM);
    act("With the flash of light on metal a giant guillotine comes crashing down!", FALSE, ch, 0, 0,TO_ROOM);
    act("A head drops to the ground from the platform.", FALSE, ch, 0, 0,TO_ROOM);
    act("$n looks up and shouts wildly.", FALSE, ch, 0, 0,TO_ROOM);
    act("$n shouts, 'Next!'", FALSE, ch, 0, 0, TO_ROOM);
    return(1);
  case 4:
   act("$n whistles a local tune.", FALSE, ch, 0, 0,TO_ROOM);
   return(1);
   default:
     return(FALSE);
     break;
  }
  return(0);
}

*/

SPECIAL(butcher)
{
ACMD(do_say);
if(cmd) return 0;
  switch (number(0, 40)) {
   case 0:
      do_say(ch, "I need a Union.", 0, 0);
      act("$n glares angrily.", FALSE, ch, 0, 0,TO_ROOM);
      act("$n rummages about for an axe.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 1:
      act("$n gnaws on a toothpick.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 2:
      act("$n runs a finger along the edge of a giant meat cleaver.", FALSE, ch, 0, 0,TO_ROOM);
      act("$n grins evilly.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 3:
      do_say(ch, "Pork for sale!", 0, 0);
      return(1);
   case 4:
      act("$n whispers to you, 'I've got some great damage eq in the back room. Wanna see?'", FALSE, ch, 0, 0,TO_ROOM);
      act("$n throws back his head and cackles with insane glee!", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 5:
      act("$n yawns.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 6:
      act("$n throws an arm around the headless body of an ogre and asks to have his picture taken.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 7:
      act("$n listlessly grabs a cleaver and hurls it into the wall behind your head.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 8:
      act("$n juggles some fingers.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 9:
      act("$n eyes your limbs.", FALSE, ch, 0, 0,TO_ROOM);
      act("$n chuckles.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 10:
      do_say(ch, "Hi, Alice.", 0, 0);
      return(1);
   case 11:
      do_say(ch, "Everyone looks like food to me these days.", 0, 0);
      act("$n sighs loudly.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 12:
      act("$n throws up his head and shouts wildly.", FALSE, ch, 0, 0,TO_ROOM);
      act("$n shouts, 'Bring out your dead!'", FALSE, ch, 0, 0, TO_ROOM);
      return(1);
   case 13:
      do_say(ch, "The worms crawl in, the worms crawl out..", 0, 0);
      return(1);
   case 14:
      act("$n sings 'Brave, brave Sir Patton...'", FALSE, ch, 0, 0,TO_ROOM);
      act("$n whistles a tune.", FALSE, ch, 0, 0,TO_ROOM);
      act("$n smirks.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 15:
      do_say(ch, "Get Lurch to bring me over a case and I'll sport you a year's supply of grilled ogre.", 0, 0);
      return(1);
    default: return(FALSE);
                break; }
    return(0);
}



SPECIAL(stu)
{
  ACMD(do_say);
  ACMD(do_flee);
  if(cmd)
    return 0;

  switch (number(0, 60)) {
    case 0:
      do_say(ch, "I'm so damn cool, I'm too cool to hang out with myself!", 0, 0);
      break;
    case 1:
      do_say(ch, "I'm really the NICEST guy you ever MEET!", 0, 0);
      break;
    case 2:
      do_say(ch, "Follow me for exp, gold and lessons in ADVANCED C!", 0, 0);
      break;
    case 3:
      do_say(ch, "Mind if I upload 200 megs of pregnant XXX gifs with no descriptions to your bbs?", 0, 0);
      break;
    case 4:
      do_say(ch, "Sex? No way! I'd rather jog 20 miles!", 0, 0);
      break;
    case 5:
      do_say(ch, "I'll take you OUT!!   ...tomorrow", 0, 0);
      break;
    case 6:
      do_say(ch, "I invented Mud you know...", 0, 0);
      break;
    case 7:
      do_say(ch, "Can I have a cup of water?", 0, 0);
      break;
    case 8:
      do_say(ch, "I'll be jogging down ventnor ave in 10 minutes if you want some!", 0, 0);
      break;
    case 9:
      do_say(ch, "Just let me pull a few strings and I'll get ya a site, they love me! - doesnt everyone?", 0, 0);
      break;
    case 10:
      do_say(ch, "Pssst! Someone tell Mercy to sport me some levels.", 0, 0);
      act("$n nudges you with his elbow.", FALSE, ch, 0, 0,TO_ROOM);
      break;
    case 11:
      do_say(ch, "Edgar! Buddy! Let's group and hack some ogres to tiny quivering bits!", 0, 0);
      break;
    case 12:
      act("$n tells you, 'Skylar has bad taste in women!'", FALSE, ch, 0, 0,TO_ROOM);
      act("$n screams in terror!", FALSE, ch, 0, 0,TO_ROOM);
      do_flee(ch, 0, 0, 0);
      break;
    case 13:
      if (number(0, 32767)<10){
      act("$n whispers to you, 'Dude! If you fucking say 'argle bargle' to the glowing fido he'll raise you a level!'", FALSE, ch, 0, 0,TO_ROOM);
      act("$n flexes.", FALSE, ch, 0, 0,TO_ROOM);}
      return(1);
    default:
      return(FALSE);
      break;
   return(1);
  }
  return 0;
}


SPECIAL(sund_earl)
{
  ACMD(do_say);
  if (cmd)
    return(FALSE);
  switch (number(0, 20)) {
   case 0:
      do_say(ch, "Lovely weather today.", 0, 0);
      return(1);
   case 1:
    act("$n practices a lunge with an imaginary foe.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
   case 2:
      do_say(ch, "Hot performance at the gallows tonight.", 0, 0);
     act("$n winks suggestively.", FALSE, ch, 0, 0,TO_ROOM);
     return(1);
   case 3:
      do_say(ch, "Must remember to up the taxes at my convenience.", 0, 0);
      return(1);
   case 4:
      do_say(ch, "Sundhaven is impermeable to the enemy!", 0, 0);
      act("$n growls menacingly.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
 case 5:
      do_say(ch, "Decadence is the credence of the abominable.", 0, 0);
      return(1);
 case 6:
      do_say(ch, "I look at you and get a wonderful sense of impending doom.", 0, 0);
      act("$n chortles merrily.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
 case 7:
      act("$n touches his goatee ponderously.", FALSE, ch, 0, 0,TO_ROOM);
      return(1);
 case 8:
      do_say(ch, "It's Mexican Madness night at Maynards!", 0, 0);
      act("$n bounces around.", FALSE, ch, 0, 0, TO_ROOM);
      return(1);
    default: return(FALSE);
              break;
    return(0);
 }
}


SPECIAL(blinder)
{
  ACMD(do_say);

  if (cmd)
    return FALSE;

  if (GET_POS(ch) != POS_FIGHTING)
    return FALSE;

  if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room) &&
      (number(0, 100)+GET_LEVEL(ch) >= 100)) {
    act("$n whispers, 'So, $N! You wouldst share my affliction!", 1, ch, 0, FIGHTING(ch), TO_NOTVICT);
    act("$n whispers, 'So, $N! You wouldst share my affliction!", 1, ch, 0, FIGHTING(ch), TO_VICT);
    act("$n's frayed cloak blows as he points at $N.", 1, ch, 0, FIGHTING(ch), TO_NOTVICT);
    act("$n's frayed cloak blows as he aims a bony finger at you.", 1, ch, 0, FIGHTING(ch), TO_VICT);
    act("A flash of pale fire explodes in $N's face!", 1, ch, 0, FIGHTING(ch), TO_NOTVICT);
    act("A flash of pale fire explodes in your face!", 1, ch, 0, FIGHTING(ch), TO_VICT);
    call_magic(ch, FIGHTING(ch), 0, SPELL_BLINDNESS, GET_LEVEL(ch), CAST_SPELL);
    return TRUE;
  }
  return FALSE;
}


SPECIAL(idiot)
{
  ACMD(do_say);

if(cmd) return FALSE;
  switch (number(0, 40)) {
   case 0:
      do_say(ch, "even if idiot = god", 0, 0);
      do_say(ch, "and Stu = idiot", 0, 0);
      do_say(ch, "Stu could still not = god.", 0, 0);
      act("$n smiles.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   case 1:
      act("$n balances a newbie sword on his head.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   case 2:
      act("$n doesn't think you could stand up to him in a duel.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   case 3:
      do_say(ch, "Rome really was built in a day.", 0, 0);
      act("$n snickers.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   case 4:
      act("$n flips over and walks around on his hands.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   case 5:
      act("$n cartwheels around the room.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   case 6:
      do_say(ch, "How many ogres does it take to screw in a light bulb?", 0, 0);
      act("$n stops and whaps himself upside the head.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   case 7:
      do_say(ch, "Uh huh. Uh huh huh.", 0, 0);
      return TRUE;
   case 8:
      act("$n looks at you.", FALSE, ch, 0, 0,TO_ROOM);
      act("$n whistles quietly.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   case 9:
      act("$n taps out a tune on your forehead.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   case 10:
      act("$n has a battle of wits with himself and comes out unharmed.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   case 11:
      do_say(ch, "All this and I am just a number.", 0, 0);
      act("$n cries on your shoulder.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   case 12:
      do_say(ch, "A certain hunchback I know dresses very similar to you, very similar...", 0, 0);
      return TRUE;
   default:
      return FALSE;
  }
 return FALSE;
}



/* ********************************************************************
*  Special procedures for objects                                     *
******************************************************************** */

SPECIAL(marbles)
{
  struct obj_data *tobj = me;

  if (tobj->in_room == NOWHERE)
    return 0;

  if (CMD_IS("north") || CMD_IS("south") || CMD_IS("east") || CMD_IS("west") ||
      CMD_IS("up") || CMD_IS("down")) {
    if (!AFF_FLAGGED(ch, AFF_FLY)){
     if (number(1, 100) - GET_DEX(ch) > 35) {
       act("You slip on $p and fall.", FALSE, ch, tobj, 0, TO_CHAR);
       act("$n slips on $p and falls.", FALSE, ch, tobj, 0, TO_ROOM);
       GET_POS(ch) = POS_SITTING;
       return 1;
     }
     else {
       act("You slip on $p, but manage to retain your balance.", FALSE, ch, tobj, 0, TO_CHAR);
       act("$n slips on $p, but manages to retain $s balance.", FALSE, ch, tobj, 0, TO_ROOM);
     }
    }
   }
  return 0;
}



SPECIAL(bank)
{
  int amount;

  if (CMD_IS("balance")) {
    if (GET_BANK_GOLD(ch) > 0)
      sprintf(buf, "Your current balance is %s coins.\r\n",
              add_points(GET_BANK_GOLD(ch)));
    else
      sprintf(buf, "You currently have no money deposited.\r\n");
    send_to_char(buf, ch);
    return 1;
  } else if (CMD_IS("deposit")) {
    if ((amount = atoi(argument)) <= 0) {
      send_to_char("How much do you want to deposit?\r\n", ch);
      return 1;
    }
    if (GET_GOLD(ch) < amount) {
      send_to_char("You don't have that many coins!\r\n", ch);
      return 1;
    }
    GET_GOLD(ch) -= amount;
    GET_BANK_GOLD(ch) += amount;
    sprintf(buf, "You deposit %s coins.\r\n", add_points(amount));
    send_to_char(buf, ch);
    act("$n makes a bank transaction.", TRUE, ch, 0, FALSE, TO_ROOM);
    return 1;
  } else if (CMD_IS("withdraw")) {
    if ((amount = atoi(argument)) <= 0) {
      send_to_char("How much do you want to withdraw?\r\n", ch);
      return 1;
    }
    if (GET_BANK_GOLD(ch) < amount) {
      send_to_char("You don't have that many coins deposited!\r\n", ch);
      return 1;
    }
    GET_GOLD(ch) += amount;
    GET_BANK_GOLD(ch) -= amount;
    sprintf(buf, "You withdraw %s coins.\r\n", add_points(amount));
    send_to_char(buf, ch);
    act("$n makes a bank transaction.", TRUE, ch, 0, FALSE, TO_ROOM);
    return 1;
  } else
    return 0;
}

static  char  *priest_strings[] =  {
    "$n says 'Buy HEAL from me in the name of the Gods.  Only 15000  coins!'",
    "$n says 'Live in the way of the Gods!'",
    "$n is blessed by the Gods.",
    "$n says 'Buy STRENGTH from me in the name of the Gods. Only 10000 coins!'",
    "$n says 'Buy SANCTUARY from me in the name of the Gods. Only 20000 coins!'",
    "$n looks around for customers.",
    "$n says 'Buy CURE from me in the name of the Gods.  Only 5000 coins!'",
    "$n says 'Buy MANA in the name of the Gods.  Only 15000 coins!'",
    "$n smiles happily.",
    "$n says 'Buy MOVE from me in the name of the Gods. Only 15000 coins!'",
    "$n says 'Buy RESTORE from me in the name of the Gods. Only 5000 coins per level!'",
    "$n says 'Type 'list' and find out how I may help you!'",
    "$n says 'Buy ARMOR from me in the name of the Gods. Only 7500 coins!'",
    "$n says 'Buy BLESS from me in the name of the Gods. Only 10000 coins!'",
    "$n looks compassionately at the unfaithful godless mortals."
};


SPECIAL(priest)
{
  struct char_data *vict;
  struct char_data *hitme = NULL;
  static int this_hour;
  float temp1 = 1;
  float temp2 = 1;
  struct char_data *self;

  self=(struct char_data *)me;


    if (CMD_IS("list")) {
      if (!AWAKE(ch)){
	return(0);
      }
      else {
       send_to_char("&gCommand     &YCost(Gold)   &REffect              &n\n\r", ch);
       send_to_char("&G=======     ==========   ==================  &n\n\r", ch);
       send_to_char("&g cure       &Y   5000     &R   Small Cure        &n\n\r", ch);
       send_to_char("&g armor      &Y   7500     &R   -20 AC            &n\n\r", ch);
       send_to_char("&g bless      &Y  10000     &R   +5 Hitroll         &n\n\r", ch);
       send_to_char("&g strength   &Y  10000     &R   +2 STR            &n\n\r", ch);
       send_to_char("&g move       &Y  15000     &R   100-120 Mv        &n\n\r", ch);
       send_to_char("&g mana       &Y  15000     &R   100-120 Mn	&n\n\r", ch);
       send_to_char("&g heal       &Y  15000     &R   100-120 Hp        &n\n\r", ch);
       send_to_char("&g sanctuary  &Y  20000     &R   Damage reduction  &n\n\r", ch);
       send_to_char("&g restore   &Y5000 per level&R  Full Hp/Mn/Mv     &n\n\r", ch);
       send_to_char("\n\r", ch);
       send_to_char("&BTo buy effect, type:&n\n\r", ch);
       send_to_char("&Cbuy <Command>&n\n\r", ch);
       send_to_char("&CExample: buy cure&n\n\r", ch);
       return 1;
      }
    }

    if (CMD_IS("buy")) {

     skip_spaces(&argument);

     if (!AWAKE(ch)) {
	return(0);
     }
     else {
	if (!strcmp(argument, "heal")) {
	    if (GET_GOLD(ch) >= 15000) {
		GET_GOLD(ch) -= 15000;
		GET_HIT(ch) += number(100, 120);
		if (GET_HIT(ch) > GET_MAX_HIT(ch))
		    GET_HIT(ch) = GET_MAX_HIT(ch);
		send_to_char("You have been healed by the Priest!\n\r", ch);
		act("$n has been healed by the Priest.\n\r", TRUE, ch, 0, 0, TO_ROOM);
	    } else {
		send_to_char("The Priest tells you 'You don't have enough money, Sorry!'\n\r", ch);
	    }
	    return 1;
	} else if (!strcmp(argument, "mana")) {
	    if (GET_GOLD(ch) >= 15000) {
		GET_GOLD(ch) -= 15000;
		GET_MANA(ch) += number(100, 120);
		if (GET_MANA(ch) > GET_MAX_MANA(ch))
		    GET_MANA(ch) = GET_MAX_MANA(ch);
		send_to_char("You have been vitalized by the Priest!\n\r", ch);
		act("$n has been vitalized by the Priest.", TRUE, ch, 0, 0, TO_ROOM);
	    } else {
		send_to_char("The Priest tells you 'You don't have enough money, Sorry!'\n\r", ch);
	    }
	    return 1;
	} else if (!strcmp(argument, "armor")) {
	    if (GET_GOLD(ch) >= 7500) {
		GET_GOLD(ch) -= 7500;
		cast_spell(self, ch, NULL, SPELL_ARMOR);
	    }
	    else {
		send_to_char("The Priest tells you 'You don't have enough money, Sorry!'\n\r", ch);
	    }
	    return 1;
	} else if (!strcmp(argument, "bless")) {
	    if (GET_GOLD(ch) >= 10000) {
		GET_GOLD(ch) -= 10000;
		cast_spell(self, ch, NULL, SPELL_BLESS);
	    }
	    else {
		send_to_char("The Priest tells you 'You don't have enough money, Sorry!'\n\r", ch);
	    }
	    return 1;
	} else if (!strcmp(argument, "sanctuary")) {
	    if (GET_GOLD(ch) >= 20000) {
		GET_GOLD(ch) -= 20000;
		cast_spell(self, ch, NULL, SPELL_SANCTUARY);
	    }
	    else {
		send_to_char("The Priest tells you 'You don't have enough money, Sorry!'\n\r", ch);
	    }
	    return 1;
	} else if (!strcmp(argument, "strength")) {
	    if (GET_GOLD(ch) >= 10000) {
		GET_GOLD(ch) -= 10000;
		cast_spell(self, ch, NULL, SPELL_STRENGTH);
	    }
	    else {
		send_to_char("The Priest tells you 'You don't have enough money, Sorry!'\n\r", ch);
	    }
	    return 1;
	} else if (!strcmp(argument, "cure")) {
	    if (GET_GOLD(ch) >= 5000) {
		GET_GOLD(ch) -= 5000;
		if (affected_by_spell(ch, SPELL_POISON))
		    affect_from_char(ch, SPELL_POISON);
		if (affected_by_spell(ch, SPELL_CURSE))
		    affect_from_char(ch, SPELL_CURSE);
                if (affected_by_spell(ch, SPELL_BLINDNESS))
                    affect_from_char(ch, SPELL_BLINDNESS);
		send_to_char("You have been cured by the Priest!\n\r", ch);
		act("$n has been cured by the Priest.", TRUE, ch, 0, 0, TO_ROOM);
	    }
	    else {
	        send_to_char("The Priest tells you 'You don't have enough money, Sorry!'\n\r", ch);
	    }
	    return 1;
	} else if (!strcmp(argument, "move")) {
	    if (GET_GOLD(ch) >= 15000){
		GET_GOLD(ch) -= 15000;
		GET_MOVE(ch) += number(100,120);
		if (GET_MOVE(ch) > GET_MAX_MOVE(ch))
		    GET_MOVE(ch) = GET_MAX_MOVE(ch);
		send_to_char("You have been refreshed by the priest!\n\r", ch);
		act("$n has been refreshed by the priest.", TRUE, ch, 0,0,TO_ROOM);
	    } else {
		send_to_char("The Priest tells you 'You don't have enough money, Sorry!'\n\r", ch);
	    }
	    return 1;
	} else if (!strcmp(argument, "restore")){
	    if (GET_GOLD(ch) >= (5000*GET_LEVEL(ch))){
		GET_GOLD(ch) -= (5000*GET_LEVEL(ch));
		GET_HIT(ch) = GET_MAX_HIT(ch);
		GET_MANA(ch) = GET_MAX_MANA(ch);
		GET_MOVE(ch) = GET_MAX_MOVE(ch);
		send_to_char("You have ben fully restored by the priest !!\n\r", ch);
		act("$n has been fully restored by the priest.", TRUE, ch, 0,0, TO_ROOM);
	    }
	    else {
		send_to_char("The Priest tells you 'You don't have enough money, Sorry!'\n\r", ch);
	    }
	    return 1;
	} else {
	    send_to_char("The Priest tells you 'Sorry, buy WHAT?'\n\r", ch);
	}

	return 1;
      }
    }

    if (!cmd) {
    	if (time_info.hours != this_hour) {

          this_hour = time_info.hours;

          temp2 = 1;
          hitme = NULL;
          for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
          {
           if ((GET_LEVEL(vict) < LVL_IMMORT) && (!IS_NPC(vict))){
             temp1 = GET_HIT(vict) / GET_MAX_HIT(vict);
             if (temp1 < temp2) {
              temp2 = temp1;
              hitme = vict;
             }
           }
         }

         vict = hitme;

         if (hitme != NULL) {

          if(!IS_NPC(vict) && GET_LEVEL(vict) <= 40){
            if (hitme != NULL) {
              cast_spell(self, hitme, NULL, SPELL_CURE_LIGHT);
              return 1;
            }
          }
          if(!IS_NPC(vict) && GET_LEVEL(vict) <= 110){
            if (hitme != NULL) {
             cast_spell(self, hitme, NULL, SPELL_CURE_CRITIC);
             return 1;
            }
          }
          if(!IS_NPC(vict) && GET_LEVEL(vict) <= 200){
            if (hitme != NULL) {
              cast_spell(self, hitme, NULL, SPELL_HEAL);
              return 1;
            }
          }
          if(!IS_NPC(vict) && GET_LEVEL(vict) > 200){
            if (hitme != NULL) {
              return 1;
            }
          }
         }
       }

	if (!number(0, 10)) {
	    act(priest_strings[number(0,14)], FALSE, ch, 0, 0, TO_ROOM);
	    return(1);
	}
    }

    return 0;
}


SPECIAL(priest_healer)
{
  struct char_data *vict;
  struct char_data *hitme = NULL;
  static int this_hour;
  float temp1 = 1;
  float temp2 = 1;

if (cmd) return FALSE;

if (time_info.hours != this_hour) {

  this_hour = time_info.hours;

  for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
    {
       temp1 = GET_HIT(vict) / GET_MAX_HIT(vict);
       if (temp1 < temp2) {
             temp2 = temp1;
             hitme = vict;
            }
    }

    vict = world[ch->in_room].people;

    if(!IS_NPC(vict) && GET_LEVEL(vict) <= 50){
       if (hitme != NULL) {
             cast_spell(ch, hitme, NULL, SPELL_CURE_LIGHT);
             return 1;
            }
    }
    if(!IS_NPC(vict) && GET_LEVEL(vict) <= 75){
       if (hitme != NULL) {
             cast_spell(ch, hitme, NULL, SPELL_CURE_CRITIC);
             return 1;
            }
    }
    if(!IS_NPC(vict) && GET_LEVEL(vict) <= 200){
       if (hitme != NULL) {
             cast_spell(ch, hitme, NULL, SPELL_HEAL);
             return 1;
            }
    }

  }
  return 0;
}

#define MSG_BS \
"THE BLACKSMITH\r\n" \
"You can talk to the blackmisth to his forge\r\n" \
"this items. Type FORGE <field> to him create that...\r\n" \
"\r\n" \
"You can buy the following weapons:\r\n" \
"  Field          Cost\r\n" \
"  -----------    ------------------------\r\n" \
"  Fire Sword     %2d points and %5s gold\r\n" \
"  Ice Sword      %2d points and %5s gold\r\n" \
"  Light Sword    %2d points and %5s gold\r\n" \
"\r\n" \

SPECIAL(blacksmith)
{
  struct obj_data *obj;

                    /*         Fw          Iw          Lw   */
  const int costs[] = {      1000,        850,        650 };
  const int money[] = { 100000000,  100000000,  100000000 };
  int mode;

  if (IS_NPC(ch) || !CMD_IS("forge"))
    return 0;

  skip_spaces(&argument);

  if (!*argument) { /* no argument, just display the options. */
    sprintf(buf, MSG_BS, costs[0], add_points(money[0]), costs[1], add_points(money[1]),
                          costs[2], add_points(money[2]));
    send_to_char(buf, ch);
    return 1;
  }

  if (!strcmp(argument, "fire weapon") || !strcmp(argument, "fw"))
    mode = 0;
  else if (!strcmp(argument, "ice weapon") || !strcmp(argument, "iw"))
    mode = 1;
  else if (!strcmp(argument, "light weapon") || !strcmp(argument, "lw"))
    mode = 2;
  else {
    send_to_char("Invalid weapon to forge.\r\n", ch);
    return 1;
  }

  if (GET_POINTS(ch) < costs[mode]) {
    send_to_char("You don't have enough points!\r\n", ch);
    return 1;
  }
  if (GET_GOLD(ch) < money[mode]) {
    send_to_char("You don't have enough money!\r\n", ch);
    return 1;
  }

  switch (mode) {
    case 0: /* fire sword */
      obj = read_object(1230, VIRTUAL);
      obj_to_char(obj, ch);
      send_to_char("The Blacksmith forges your weapon...\r\n", ch);
      break;
    case 1: /* ice sword */
      obj = read_object(1231, VIRTUAL);
      obj_to_char(obj, ch);
      send_to_char("The Blacksmith forges your weapon...\r\n", ch);
      break;
    case 2: /* light sword */
      obj = read_object(1233, VIRTUAL);
      obj_to_char(obj, ch);
      send_to_char("The Blacksmith forges your weapon...\r\n", ch);
      break;
  }
  GET_POINTS(ch) -= costs[mode];
  GET_GOLD(ch) -= money[mode];

  act("The blacksmith makes a weapon to $n.", TRUE, ch, 0, FALSE, TO_ROOM);
  return 1;
}

SPECIAL(icewizard)
{

  struct char_data *tch;
  int low_on_hits = 10000;

  if (cmd)
    return (FALSE);

  /* Find out who has the lowest hitpoints and burn his ass off */
  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room){
    if (!IS_NPC(tch))
      if (tch->points.hit < low_on_hits){
	low_on_hits = tch->points.hit;
      }
  }

  act("$n screams 'Bonjour! you tiny, little looser!!'", FALSE, ch, 0, 0, TO_ROOM);
  act("$n looks at $N", 1, ch, 0, tch, TO_NOTVICT);
  act("$n looks at YOU!", 1, ch, 0, tch, TO_VICT);
  cast_spell(ch, tch, 0, SPELL_FIREBALL);
  return TRUE;
}

SPECIAL(preparation_room)
{
  struct char_data *vict;

  if (cmd)
   return FALSE;

  for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
   {
    if(!IS_NPC(vict)){

     if(PRF_FLAGGED(vict, PRF_INFO_START))
        return FALSE;

     if(GET_REMORT(vict) == 0 && GET_LEVEL(vict) == 1)
     {
        send_to_char("The Wardome talk to you, 'I will teleport you to the Mud School...'\r\n\r\n", vict);
	char_from_room(vict);
	char_to_room(vict, real_room(26600));
	look_at_room(vict, 0);
	return 1;
     }
     if(GET_RACE(vict) == RACE_GORAK && GET_LEVEL(vict) < LVL_IMMORT){
        send_to_char("The Wardome talk to you, 'I will teleport you to the Temple of Goraks...'\r\n\r\n", vict);
	char_from_room(vict);
	char_to_room(vict, real_room(511));
	look_at_room(vict, 0);
	return 1;
     }
     if(GET_RACE(vict) == RACE_SEA_ELF && GET_LEVEL(vict) < LVL_IMMORT){
        send_to_char("The Wardome talk to you, 'I will teleport you to the Temple of the Seal Elfs...'\r\n\r\n", vict);
	char_from_room(vict);
	char_to_room(vict, real_room(512));
	look_at_room(vict, 0);
	return 1;
     }
     if(GET_RACE(vict) == RACE_ELF && GET_LEVEL(vict) < LVL_IMMORT){
        send_to_char("The Wardome talk to you, 'I will teleport you to the Temple of Elfs...'\r\n\r\n", vict);
	char_from_room(vict);
	char_to_room(vict, real_room(513));
	look_at_room(vict, 0);
	return 1;
     }
     if(GET_RACE(vict) == RACE_ORC && GET_LEVEL(vict) < LVL_IMMORT){
        send_to_char("The Wardome talk to you, 'I will teleport you to the Temple of Orcs...'\r\n\r\n", vict);
	char_from_room(vict);
	char_to_room(vict, real_room(514));
	look_at_room(vict, 0);
	return 1;
     }
     if(GET_RACE(vict) == RACE_TROLL && GET_LEVEL(vict) < LVL_IMMORT){
        send_to_char("The Wardome talk to you, 'I will teleport you to the Temple of Trolls...'\r\n\r\n", vict);
	char_from_room(vict);
	char_to_room(vict, real_room(515));
	look_at_room(vict, 0);
	return 1;
     }
     if(GET_RACE(vict) == RACE_VAMPIRE && GET_LEVEL(vict) < LVL_IMMORT){
        send_to_char("The Wardome talk to you, 'I will teleport you to the Temple of Humans...'\r\n\r\n", vict);
	char_from_room(vict);
	char_to_room(vict, real_room(516));
	look_at_room(vict, 0);
	return 1;
     }
     if(GET_RACE(vict) == RACE_DROW && GET_LEVEL(vict) < LVL_IMMORT){
        send_to_char("The Wardome talk to you, 'I will teleport you to the Temple of Drows...'\r\n\r\n", vict);
	char_from_room(vict);
	char_to_room(vict, real_room(517));
	look_at_room(vict, 0);
	return 1;
     }
     if(GET_RACE(vict) == RACE_DWARF && GET_LEVEL(vict) < LVL_IMMORT){
        send_to_char("The Wardome talk to you, 'I will teleport you to the Temple of Dwarfs...'\r\n\r\n", vict);
	char_from_room(vict);
	char_to_room(vict, real_room(518));
	look_at_room(vict, 0);
	return 1;
     }
     if(GET_RACE(vict) == RACE_LIZARDMAN && GET_LEVEL(vict) < LVL_IMMORT){
        send_to_char("The Wardome talk to you, 'I will teleport you to the Temple of Lizardmans...'\r\n\r\n", vict);
	char_from_room(vict);
	char_to_room(vict, real_room(519));
	look_at_room(vict, 0);
	return 1;
     }
     if(GET_RACE(vict) == RACE_OGRE && GET_LEVEL(vict) < LVL_IMMORT){
        send_to_char("The Wardome talk to you, 'I will teleport you to the Temple of Ogres...'\r\n\r\n", vict);
	char_from_room(vict);
	char_to_room(vict, real_room(520));
	look_at_room(vict, 0);
	return 1;
     }
     if(GET_RACE(vict) == RACE_GITH && GET_LEVEL(vict) < LVL_IMMORT){
        send_to_char("The Wardome talk to you, 'I will teleport you to the Temple of Giths...'\r\n\r\n", vict);
	char_from_room(vict);
	char_to_room(vict, real_room(521));
	look_at_room(vict, 0);
	return 1;
     }
     if(GET_RACE(vict) == RACE_GNOME && GET_LEVEL(vict) < LVL_IMMORT){
        send_to_char("The Wardome talk to you, 'I will teleport you to the Temple of Gnomes...'\r\n\r\n", vict);
	char_from_room(vict);
	char_to_room(vict, real_room(522));
	look_at_room(vict, 0);
	return 1;
     }
     if(GET_RACE(vict)  > 11) {  // Racas avancadas
        char_from_room(vict);
        char_to_room(vict, real_room(567));
        look_at_room(vict, 0);
        return 1;
     }
    }
   }
  return 0;
}
#define MSG_ARENA \
"&RThe Wardome&n\r\n" \
"\r\n" \
"&nHere you can open and enjoy de &RWAR&n.\r\n" \
"&WUsage: &Rwardome now&n\r\n" \
"\r\n"

extern int lo_rem;
extern int hi_rem;

SPECIAL(wardome)
{
  extern long arena_pot;

  if (IS_NPC(ch) || !CMD_IS("wardome"))
    return 0;

  if (!*argument) { /* no argument, just display the options. */
    sprintf(buf, MSG_ARENA);
    send_to_char(buf, ch);
    return 1;
  }

  half_chop(argument, buf1, buf2);

  if (in_arena != ARENA_OFF) {
	send_to_char("There is a &BWAR&n running already.\r\n", ch);
	return 1;
  }

	if (!is_number(buf1) || !is_number(buf2))
	{
		send_to_char("Write a remort number please.\r\n", ch);
		return 1;
	}

	lo_rem = atoi(buf1);
	hi_rem = atoi(buf2);
	if (lo_rem == hi_rem){
		send_to_char("The first value must be lower than the second.\r\n", ch);
		return 1;
	}
	if ((lo_rem < 0) || (hi_rem < 0)){
	send_to_char("Please use positive numbers.\r\n", ch);
	return 1;
	}
	if(lo_rem > hi_rem)
	{
		send_to_char("Setup the correct remort please.\r\n", ch);
		return 1;
	}

  in_arena = ARENA_START;
  time_to_start = start_time = 5;
  time_left_in_game = 99;
  arena_pot = 0;
  game_length = 99;
  lo_lim = MAX(1, GET_LEVEL(ch) - 15);
  hi_lim = MIN(200, GET_LEVEL(ch) + 15);
  cost_per_lev = 1000;
  start_arena();

  return 1;
}

struct quest_eqs *eqs_quest;
extern int top_qeq_table;
extern struct player_index_element *player_table;
extern struct obj_data *obj_proto;
int find_name(char *name);
char *complete_char(char *string, int num);
void save_qeq_file(void);

SPECIAL(questshop)
{
	struct obj_data *obj;
	obj_rnum r_num;
	int i = 0;

	if (IS_NPC(ch))
		return 0;
		
	if(CMD_IS("list"))
	{
		strcpy(buf, "&cEquipamentos para Aventureiros, somente itens &Cunicos&c.&n\r\n"
					"&CObjeto&c----------------------------- --- &CPreco &c--------------- &CDono&n\r\n");
		for(i = 0; i < top_qeq_table; i++)
		{
			r_num = real_object(eqs_quest[i].vnum_eq);
			sprintf(buf + strlen(buf), "%s &G%9d &R%20s&n\r\n", complete_char(reprimir(obj_proto[r_num].short_description, 35+(2*conta_cores(obj_proto[r_num].short_description))), 35),
					eqs_quest[i].preco_eq, (eqs_quest[i].dono == 0 ? "Disponivel" : get_name_by_id(eqs_quest[i].dono)));
		}
		send_to_char(buf, ch);
		return 1;
	}
	else if(CMD_IS("buy"))
	{
		if(!*argument)
			return 0;

		for(i = 0; i < top_qeq_table; i++)
		{
			r_num = real_object(eqs_quest[i].vnum_eq);
			if(isname(argument, obj_proto[r_num].name))
			{
				if(eqs_quest[i].dono > 0)
				{
					send_to_char("Este equipamento ja' possui dono.\r\n", ch);
					return 1;
				}
				if(eqs_quest[i].preco_eq > GET_QP(ch))
				{
					send_to_char("Voce nao possui QP suficiente para efetuar a compra.\r\n", ch);
					return 1;
				}
				eqs_quest[i].dono = player_table[find_name(GET_NAME(ch))].id;
				save_qeq_file();

				GET_QP(ch) -= eqs_quest[i].preco_eq;
				obj = read_object(r_num, REAL);
				obj_to_char(obj, ch);
				send_to_char("Negocio fechado. Tenha boas aventuras!\r\n", ch);
				save_char(ch, ch->in_room);
				return 1;
			}
		}
				send_to_char("Nao possuo esse equipamento.\r\n", ch);
		return 1;
	}
	return 0;
}

SPECIAL(black_dragon) // mob numero 3097
{
  struct char_data *vict;

  if (cmd || GET_POS(ch) != POS_FIGHTING)
    return (0);

  vict = FIGHTING(ch);
  if (vict == NULL || (ch->in_room != vict->in_room))
   return (1);                           // para evitar bugs

  damage(ch,vict,number(500,1000),505);  // dragao faz de 500 a 1000 de dano

  vict = FIGHTING(ch);
  if (vict == NULL || (ch->in_room != vict->in_room))
   return (1) ;

  damage(ch,vict,number(500,1000),505); // novamente dragao faz de 500 a 1000 de dano

  return(1);
}

SPECIAL(black_monster) // mob numero 3098
{
  struct char_data *vict;
  int x = 0 ;

  if (cmd || GET_POS(ch) != POS_FIGHTING)
    return (0);

  for (vict = world[real_room(3001)].people; vict; vict = vict->next_in_room)
  {
   if (GET_MOB_VNUM(vict) == 3097)
   {
    x = 1  ;
    break ;
   }
  }

  if(x)
  {
   GET_HIT(ch) = GET_MAX_HIT(ch);
   send_to_room("Black Monster says : I AM INVINCIBLE, INVINCIBLE I SAY!!!!\r\n", ch->in_room);
  }
  else
   send_to_room("Black Monster says : WHAT!! I AM HURT?!\r\n", ch->in_room);

  return (1) ;

}
#define MSG_RB \
"                        &w- &BWardome &YREBORN &BSystem &w-&n\r\n" \
"With the Powers of Gods here you can upgrade your race reborning \r\n" \
"a new powerfull race. To do This type REBORN <Cap. Letter of Race> or <Name of Race>:\r\n" \
"&w- &RATENTION &w- &wYou Can't change your race, just evoluate the actual.\r\n" \
"\r\n"\
"&b #&y Race        &b Remorts &y Add Attribs&b (*)         &yCost\r\n" \
"&b -&y ---------   &b --------&y -----------&b ---------   &y---------------\r\n" \
"&B A&Y Dunedain    &B R6     &Y +2str,+2wis &B Hero's Presence, Average Endurance, Average extra damage &Y %7s gold\r\n" \
"&B B&Y Ancient Drow&B R6      &Y +2wis,+2dex &B  Magic Resistence &Y %7s gold\r\n" \
"&B C&Y Naugrim     &B R6      &Y +2con,+2str &B Super Endurance &Y %7s gold\r\n" \
"&B D&Y High-Elf    &B R6      &Y +2wis,+dex,+2cha &B 2-effect magic &Y %7s gold\r\n" \
"&B E&Y Hill Ogre   &B R6      &Y +2str,+con,+dex &B Extra Damage&Y %7s gold\r\n" \
"&B F&Y Bugbear     &B R6      &Y +str,+con,+2dex &B Super Stamina&Y %7s gold\r\n" \
"&B G&Y Cave Troll  &B R6      &Y +str,+2con,+dex &B Regeneration&Y %7s gold\r\n" \
"&B H&Y Lich        &B R6      &Y +2int,+2con &B Power magic damage &Y %7s gold\r\n" \
"&B I&Y Tinker      &B R6      &Y +2wis,+2con &B Extra MN Regeneration &Y %7s gold\r\n" \
"&B J&Y Draconian   &B R6      &Y +2str,+4cha &B Secret :)          &Y %7s gold\r\n" \
"&B L&Y High Sea Elf&B R6      &Y +2str,+2int &B Extra summon&Y %7s gold\r\n" \
"&B M&Y Archons     &B R6      &Y +2dex,+2int &B Extra curative power &Y %7s gold&n\r\n" \
"\r\n" \


int aceita_ra(int num1, int mode)
{
  switch (num1)
 {
      case  999:  // lawful good
        if(mode == 1) return 1 ;
        if(mode == 2) return 1 ;
        if(mode == 4) return 1 ;
        return 0 ;
      case  1000:
        if(mode == 1) return 1 ;
        if(mode == 2) return 1 ;
        if(mode == 3) return 1 ;
        return 0 ;
      case  994:
        if(mode == 2) return 1 ;
        if(mode == 3) return 1 ;
        if(mode == 6) return 1 ;
        return 0 ;
      case  3:
        if(mode == 1) return 1 ;
        if(mode == 4) return 1 ;
        if(mode == 7) return 1 ;
        return 0 ;
      case  5:
     if(mode == 4) return 1 ;
        if(mode == 5) return 1 ;
        if(mode == 6) return 1 ;
        return 0 ;
      case  7:
        if(mode == 3) return 1 ;
        if(mode == 6) return 1 ;
        if(mode == 9) return 1 ;
        return 0 ;
      case  -999:
       if(mode == 4) return 1 ;
        if(mode == 7) return 1 ;
        if(mode == 8) return 1 ;
        return 0 ;
      case  -1000:
        if(mode == 7) return 1 ;
        if(mode == 8) return 1 ;
        if(mode == 9) return 1 ;
        return 0 ;
      case  -994:
        if(mode == 6) return 1 ;
        if(mode == 8) return 1 ;
        if(mode == 9) return 1 ;
        return 0 ;
  }
 return 0 ;
}



SPECIAL(religiao)
{
  int mode ;

  if (IS_NPC(ch) || !CMD_IS("religion"))
    return 0;

  skip_spaces(&argument);

  if (!*argument)
  {
   send_to_char("Type religion 0/1/2/3/4/5/6/7/8.\r\n", ch) ;
   return 1 ;
  }


  if (!strcmp(argument, "0")) mode = 0;
  else if (!strcmp(argument, "1")) mode = 1 ;
  else if (!strcmp(argument, "2")) mode = 2;
  else if (!strcmp(argument, "3")) mode = 3;
  else if (!strcmp(argument, "4")) mode = 4;
  else if (!strcmp(argument, "5")) mode = 5;
  else if (!strcmp(argument, "6")) mode = 6;
  else if (!strcmp(argument, "7")) mode = 7;
  else if (!strcmp(argument, "8")) mode = 8;
  else {
    send_to_char("Invalid religion.\r\n", ch);
    return 1;
  }


  if((GET_CON(ch) < 7) && (GET_DEX(ch) < 7))
  {
   send_to_char("You are too weak.\r\n", ch) ;
   return 1;
  }

  if(!aceita_ra(GET_ALIGNMENT(ch), (mode+1)))
  {
     send_to_char("This God will not accept you.\r\n", ch) ;
     return 1 ;
  }


  if(GET_RELIGION(ch) && GET_REMORT(ch))
  {
   ch->real_abils.con -=4;
   ch->real_abils.dex -=4;
   save_char(ch, NOWHERE);
  }


  switch (mode)
 {
      case  0:
        GET_RELIGION(ch) = 1 ;
        send_to_char("You are now a Justice Guardian of Muir.\r\n", ch) ;
        save_char(ch, NOWHERE);
        break ;
      case  1:
        GET_RELIGION(ch) = 2 ;
        send_to_char("You are now a Pain Bearer of Ilmanateur.\r\n", ch) ;
        save_char(ch, NOWHERE);
        break ;
      case  2:
        GET_RELIGION(ch) = 3 ;
        send_to_char("You are now a Moon Blade of Selune.\r\n", ch) ;
        save_char(ch, NOWHERE);
        break ;
      case  3:
        GET_RELIGION(ch) = 4 ;
        send_to_char("You are now a Law Keeper of Helm.\r\n", ch) ;
        save_char(ch, NOWHERE);
        break ;
      case  4:
        GET_RELIGION(ch) = 5 ;
        send_to_char("You are now a Wisdom Seeker of Ohgma.\r\n", ch) ;
        save_char(ch, NOWHERE);
        break ;
      case  5:
        GET_RELIGION(ch) = 6 ;
        send_to_char("You are now a Battle Guard of Tempus.\r\n", ch) ;
        save_char(ch, NOWHERE);
        break ;
      case  6:
        GET_RELIGION(ch) = 7 ;
        send_to_char("You are now Dread Master of Bane.\r\n", ch) ;
        save_char(ch, NOWHERE);
        break ;
      case  7:
        GET_RELIGION(ch) = 8 ;
        send_to_char("You are now a Killer Rose of Pandora.\r\n", ch) ;
        save_char(ch, NOWHERE);
        break ;
      case  8:
        GET_RELIGION(ch) = 9 ;
        send_to_char("You are now a Storm Lord of Talos.\r\n", ch) ;
        save_char(ch, NOWHERE);
        break ;
  }
 return 1 ;
}




SPECIAL(alinhamento)
{
  int mode ;

  if (IS_NPC(ch) || !CMD_IS("alignment"))
    return 0;

  skip_spaces(&argument);

  if (!*argument)
  {
   send_to_char("Type alignment 0/1/2/3/4/5/6/7/8.\r\n", ch) ;
   return 1 ;
  }


  if (!strcmp(argument, "0")) mode = 0;
  else if (!strcmp(argument, "1")) mode = 1 ;
  else if (!strcmp(argument, "2")) mode = 2;
  else if (!strcmp(argument, "3")) mode = 3;
  else if (!strcmp(argument, "4")) mode = 4;
  else if (!strcmp(argument, "5")) mode = 5;
  else if (!strcmp(argument, "6")) mode = 6;
  else if (!strcmp(argument, "7")) mode = 7;
  else if (!strcmp(argument, "8")) mode = 8;
  else {
    send_to_char("Invalid alignment.\r\n", ch);
    return 1;
  }

  if((GET_CHA(ch) < 5) || (GET_WIS(ch) < 5) || (GET_INT(ch) < 5))
  {
   send_to_char("You are too weak.\r\n", ch) ;
   return 1;
  }

  if(GET_REMORT(ch))
  {
   ch->real_abils.cha -=2;
   ch->real_abils.intel -=2;
   ch->real_abils.wis -=2;
   save_char(ch, NOWHERE);
  }
 

  switch (mode)
 {
      case  0:
	GET_ALIGNMENT(ch) = 999 ;
        send_to_char("You are lawful good now.\r\n", ch) ;
        GET_RELIGION(ch) = 0 ;
        break ;
      case  1:
	GET_ALIGNMENT(ch) = 1000 ;
        send_to_char("You are neutral good now.\r\n", ch) ;
        GET_RELIGION(ch) = 0 ;
        break ;
      case  2:
	GET_ALIGNMENT(ch) = 994 ;
        send_to_char("You are chaotic good now.\r\n", ch) ;
        GET_RELIGION(ch) = 0 ;
        break ;
      case  3:
	GET_ALIGNMENT(ch) = 3 ;
        send_to_char("You are lawful neutral now.\r\n", ch) ;
        GET_RELIGION(ch) = 0 ;
        break ;
      case  4:
	GET_ALIGNMENT(ch) = 5 ;
        GET_RELIGION(ch) = 0 ;
        send_to_char("You are true neutral now.\r\n", ch) ;
        break ;
      case  5:
	GET_ALIGNMENT(ch) = 7 ;
        GET_RELIGION(ch) = 0 ;
        send_to_char("You are chaotic neutral now.\r\n", ch) ;
        break ;
      case  6:
	GET_ALIGNMENT(ch) = -999 ;
        GET_RELIGION(ch) = 0 ;
        send_to_char("You are lawful evil now.\r\n", ch) ;
        break ;
      case  7:
        GET_RELIGION(ch) = 0 ;
	GET_ALIGNMENT(ch) = -1000 ;
        send_to_char("You are neutral evil now.\r\n", ch) ;
        break ;
      case  8:
	GET_ALIGNMENT(ch) = -994 ;
        GET_RELIGION(ch) = 0 ;
        send_to_char("You are chaotic evil  now.\r\n", ch) ;
        break ;
 }
 return 1 ;
}


SPECIAL(reborn)
{

                      /* 1        2           3         4        5        6         7           8       9         10        11       12  */
const int money[] = { 7000000,
                      7000000,
                      7000000,
                      7000000,
                      7000000,
                      7000000,
                      7000000,
                      7000000,
                      7000000,
                      7000000,
                      7000000,
                      7000000};


  int mode;

  if (IS_NPC(ch) || !CMD_IS("reborn"))
    return 0;

  skip_spaces(&argument);

  if (!*argument) { /* no argument, just display the options. */
    sprintf(buf, MSG_RB,  add_points(money[0]),
                          add_points(money[1]),
                          add_points(money[2]),
                          add_points(money[3]),
                          add_points(money[4]),
                          add_points(money[5]),
                          add_points(money[6]),
                          add_points(money[7]),
                          add_points(money[8]),
                          add_points(money[9]),
                          add_points(money[10]),
                          add_points(money[11]));
    send_to_char(buf, ch);
    return 1;
  }

  if (!strcmp(argument, "A") || !strcmp(argument, "dunedain"))mode = 0;
  else if (!strcmp(argument, "B") || !strcmp(argument, "ancient drow "))mode = 1;
  else if (!strcmp(argument, "C") || !strcmp(argument, "naugrim"))mode = 2;
  else if (!strcmp(argument, "D") || !strcmp(argument, "high elf "))mode = 3;
  else if (!strcmp(argument, "E") || !strcmp(argument, "hill ogre"))mode = 4;
  else if (!strcmp(argument, "F") || !strcmp(argument, "bugbear"))mode = 5;
  else if (!strcmp(argument, "G") || !strcmp(argument, "cave troll"))mode = 6;
  else if (!strcmp(argument, "H") || !strcmp(argument, "lich "))mode = 7;
  else if (!strcmp(argument, "I") || !strcmp(argument, "tinker "))mode = 8;
  else if (!strcmp(argument, "J") || !strcmp(argument, "draconian "))mode = 9;
  else if (!strcmp(argument, "L") || !strcmp(argument, "high sea elf "))mode = 10;
  else if (!strcmp(argument, "M") || !strcmp(argument, "archons"))mode = 11;
  else {
    send_to_char("Invalid Race to Reborn.\r\n", ch);
    return 1;
  }

  if (GET_GOLD(ch) < money[mode]) {
    send_to_char("You don't have enough money!\r\n", ch);
    return 1;
  }
  if (GET_LEVEL(ch) < 200) {
    send_to_char("This isn't level to Remort!\r\n", ch);
}

  switch (mode) {
    case 0: /* human */

      if (GET_REMORT(ch) < 6){
      	send_to_char("You don't have the necessary remort\r\n", ch);

      } else if (GET_RACE(ch) != RACE_VAMPIRE) {
      	send_to_char("You don't have the necessary race\r\n", ch);

      }	else {
      GET_RACE(ch) =RACE_DUNEDAIN;
      ch->real_abils.str +=2;
      ch->real_abils.wis +=2;
      send_to_char("&BYou have &YREBORNED&B into a &yDUNEDAIN&n\r\n", ch);
      GET_GOLD(ch) -= money[mode];
      }
      save_char(ch, NOWHERE);
      break;
    case 1: /* drow*/
         if (GET_REMORT(ch) < 6){
      	send_to_char("You don't have the necessary remort\r\n", ch);
      } else if (GET_RACE(ch) != RACE_DROW) {
      	send_to_char("You don't have the necessary race\r\n", ch);
      }	else {
      GET_RACE(ch) =RACE_ANCI_DROW;
      ch->real_abils.wis +=2;
      ch->real_abils.dex +=2;
      send_to_char("&BYou have &YREBORNED&B into a &yANCIENT DROW&n\r\n", ch);
      GET_GOLD(ch) -= money[mode];
      }
      save_char(ch, NOWHERE);
      break;
    case 2: /* anao */
        if (GET_REMORT(ch) < 6){
      	send_to_char("You don't have the necessary remort\r\n", ch);
      } else if (GET_RACE(ch) != RACE_DWARF) {
      	send_to_char("You don't have the necessary race\r\n", ch);
      }	else {
      GET_RACE(ch) =RACE_NAUGRIM;
      ch->real_abils.con +=2;
      ch->real_abils.str +=2;
      send_to_char("&BYou have &YREBORNED&B into a &yNAUGRIM&n\r\n", ch);
      GET_GOLD(ch) -= money[mode];
      }
      save_char(ch, NOWHERE);
      break;
   case 3: /* elf */
        if (GET_REMORT(ch) < 6){
      	send_to_char("You don't have the necessary remort\r\n", ch);
      } else if (GET_RACE(ch) != RACE_ELF) {
      	send_to_char("You don't have the necessary race\r\n", ch);
      }	else {
      GET_RACE(ch) =RACE_HIGH_ELF;
      ch->real_abils.dex +=1;
      ch->real_abils.wis +=2;
      ch->real_abils.cha +=2;
      send_to_char("&BYou have &YREBORNED&B into a &yHIGH ELF&n\r\n", ch);
      GET_GOLD(ch) -= money[mode];
      }
      save_char(ch, NOWHERE);
      break;
    case 4: /* Ogro */
        if (GET_REMORT(ch) < 6){
      	send_to_char("You don't have the necessary remort\r\n", ch);
      } else if (GET_RACE(ch) != RACE_OGRE) {
      	send_to_char("You don't have the necessary race\r\n", ch);
      }	else {
      GET_RACE(ch) =RACE_HILL_OGRE;
      ch->real_abils.str +=2;
      ch->real_abils.con +=1;
      ch->real_abils.dex +=1;
      send_to_char("&BYou have &YREBORNED&B into a &yHILL OGRE&n\r\n", ch);
      GET_GOLD(ch) -= money[mode];
      }
      save_char(ch, NOWHERE);
      break;
       case 5: /* orc */
        if (GET_REMORT(ch) < 6){
      	send_to_char("You don't have the necessary remort\r\n", ch);
      } else if (GET_RACE(ch) != RACE_ORC) {
      	send_to_char("You don't have the necessary race\r\n", ch);
      }	else {
      GET_RACE(ch) =RACE_BUGBEAR;
      ch->real_abils.str +=1;
      ch->real_abils.con +=1;
      ch->real_abils.dex +=2;
      send_to_char("&BYou have &YREBORNED&B into a &yBUGBEAR&n\r\n", ch);
      GET_GOLD(ch) -= money[mode];
      }
      save_char(ch, NOWHERE);
      break;
       case 6: /* troll */
        if (GET_REMORT(ch) < 6){
      	send_to_char("You don't have the necessary remort\r\n", ch);
      } else if (GET_RACE(ch) != RACE_TROLL) {
      	send_to_char("You don't have the necessary race\r\n", ch);
      }	else {
      GET_RACE(ch) =RACE_CAVE_TROLL;
      ch->real_abils.str +=1;
      ch->real_abils.dex +=1;
      ch->real_abils.con +=2;
      send_to_char("&BYou have &YREBORNED&B into a &yCAVE TROLL&n\r\n", ch);
      GET_GOLD(ch) -= money[mode];
      }
      save_char(ch, NOWHERE);
      break;
        if (GET_REMORT(ch) < 6){
      	send_to_char("You don't have the necessary remort\r\n", ch);
      } else if (GET_RACE(ch) != RACE_GITH) {
      	send_to_char("You don't have the necessary race\r\n", ch);
      }	else {
      GET_RACE(ch) =RACE_LICH;
      ch->real_abils.intel +=2;
      ch->real_abils.con +=2;
      send_to_char("&BYou have &YREBORNED&B into a &yLICH&n\r\n", ch);
      GET_GOLD(ch) -= money[mode];
      }
      save_char(ch, NOWHERE);
      break;
       case 8: /* gnome */
        if (GET_REMORT(ch) < 6){
      	send_to_char("You don't have the necessary remort\r\n", ch);
      } else if (GET_RACE(ch) != RACE_GNOME) {
      	send_to_char("You don't have the necessary race\r\n", ch);
      }	else {
      GET_RACE(ch) =RACE_TINKER;
      ch->real_abils.wis +=2;
      ch->real_abils.con +=2;
      send_to_char("&BYou have &YREBORNED&B into a &yTinker&n\r\n", ch);
      GET_GOLD(ch) -= money[mode];
      }
      save_char(ch, NOWHERE);
      break;
       case 9: /* lizardman*/
        if (GET_REMORT(ch) < 6){
      	send_to_char("You don't have the necessary remort\r\n", ch);
      } else if (GET_RACE(ch) != RACE_LIZARDMAN) {
      	send_to_char("You don't have the necessary race\r\n", ch);
      }	else {
      GET_RACE(ch) =RACE_DRACONIAN;
      ch->real_abils.str +=2;
      ch->real_abils.cha +=4;
      send_to_char("&BYou have &YREBORNED&B into a &yDRACONIAN&n\r\n", ch);
      GET_GOLD(ch) -= money[mode];
      }
      save_char(ch, NOWHERE);
      break;
       case 10: /* sea Elf */
         if (GET_REMORT(ch) < 6){
      	send_to_char("You don't have the necessary remort\r\n", ch);
      } else if (GET_RACE(ch) != RACE_SEA_ELF) {
      	send_to_char("You don't have the necessary race\r\n", ch);
      }	else {
      GET_RACE(ch) =RACE_H_SEA_ELF;
      ch->real_abils.str +=2;
      ch->real_abils.intel +=2;
      send_to_char("&BYou have &YREBORNED&B into a &yHIGH SEA ELF&n\r\n", ch);
      GET_GOLD(ch) -= money[mode];
      }
      save_char(ch, NOWHERE);
      break;
     case 11: /* Gorak */
        if (GET_REMORT(ch) < 6){
      	send_to_char("You don't have the necessary remort\r\n", ch);
      } else if (GET_RACE(ch) != RACE_GORAK) {
      	send_to_char("You don't have the necessary race\r\n", ch);
      }	else {
      GET_RACE(ch) =RACE_ARCHONS;
      ch->real_abils.intel +=2;
      ch->real_abils.dex +=2;
      send_to_char("&BYou have &YREBORNED&B into a &yARCHONS&n\r\n", ch);
      GET_GOLD(ch) -= money[mode];
      }
      save_char(ch, NOWHERE);
      break;
  }

  act("The Wardome Gods Reborn $n.", TRUE, ch, 0, FALSE, TO_ROOM);
  return 1;
}

SPECIAL(random_room)
{
  int num = number(1,8);
  if (IS_NPC(ch) ||(!CMD_IS("north") && !CMD_IS("south") && !CMD_IS("east") &&!CMD_IS("west")))
    return 0;
  switch(num)
 {
  case 1 : num = 2701 ; break ;
  case 2 : num = 2702 ; break ;
  case 3 : num = 2703 ; break ;
  case 4 : num = 2705 ; break ;
  case 5 : num = 2707 ; break ;
  case 6 : num = 2708 ; break ;
  case 7 : num = 2710 ; break ;
  case 8 : num = 2712 ; break ;
 }
  char_from_room(ch);
  char_to_room(ch, real_room(num));
  look_at_room(ch, 0);
  send_to_char("\r\n &BYou think the walls are always moving&n \r\n", ch);
  return 1;
}

SPECIAL(espada_fica_azul)
{
	struct char_data *tch;
	short int saida;

	if(!CMD_IS("look"))
		return 0;

	if(!*argument)
		return 0;

    skip_spaces(&argument);

	if(!str_cmp(argument, "sword") || !str_cmp(argument, "maegnas"))
	{
		for (saida = 0; saida < NUM_OF_DIRS; saida++)
		{
			if (!EXIT(ch, saida) || EXIT(ch, saida)->to_room == NOWHERE)
				continue;

			for(tch = world[EXIT(ch, saida)->to_room].people; tch; tch = tch->next_in_room)
				if(GET_RACE(tch) == RACE_ORC)
				{
					send_to_char("&GT&ghe &WS&wword &CM&caegnas &cstarts to glow with a &BB&blue &WL&wight&c.&n\r\n", ch);
					return 1;
				}
		}
	}
	return 0;
}

#define MAX_CABECAS			5

int mobs_vnums[] =
{
	2216,
	2217,
	2218,
	2219,
	2220
};

#define SALA_DAS_CABECAS			2245

short int eh_cabeca(struct char_data *vict)
{
	short int num;

	if(!IS_NPC(vict))
		return (0);

	for(num = 0; num < MAX_CABECAS; num++)
		if(GET_MOB_VNUM(vict) == mobs_vnums[num])
			return (1);
	return (0);
}

short int tem_cabeca(int sala)
{
	short int cabecas = 0;
	struct char_data *vict;

	for (vict = world[sala].people; vict; vict = vict->next_in_room)
		if(eh_cabeca(vict))
			cabecas++;

	return (cabecas);
}

void teleportar_cabecas(struct char_data *ch, int donde)
{
	struct char_data *vict, *drg;

	for (vict = world[donde].people; vict;)
	{
		drg = vict;
		vict = vict->next_in_room;
		if(IS_NPC(drg))
		{
			char_from_room(drg);
			char_to_room(drg, ch->in_room);
		}
	}
}

SPECIAL(tiamat_heads)
{
  struct char_data *vict;
  int numero ;

  switch(GET_MOB_VNUM(ch))
 {
   case 2218 : numero = 504 ; break ; //white
   case 2220 : numero = 502 ; break ; // red
   case 2219 : numero = 505 ; break ; // black
   case 2217 : numero = 506 ; break ; //blue
   default: numero = 503    ; break ; // green
 }

  if (cmd || GET_POS(ch) != POS_FIGHTING)
    return (0);

  vict = FIGHTING(ch);
  if (vict == NULL || (ch->in_room != vict->in_room))
   return (1);

  damage(ch,vict,250,numero);
  return(1);
}

SPECIAL(tiamat2)
{
	short int num;

	if (cmd || !FIGHTING(ch))
		return FALSE;

	if(tem_cabeca(ch->in_room))
        {
         GET_HIT(ch) = GET_MAX_HIT(ch);
         send_to_room("&RTiamat &cseems &Cinvulnerable&W.&n\r\n", ch->in_room);
         return FALSE ;
        }

	else
	{
		num = real_room(SALA_DAS_CABECAS);
		if(tem_cabeca(num) == 5)
		{
			teleportar_cabecas(ch, num);
			send_to_room("&yYou feel a bad feeling, and &Csuddenly &yappears &RFIVE &chuge and scare &yheads from the &Wdarkness&y!&n\r\n", ch->in_room);
			return (1);
		}
	}
	return (0);
}





SPECIAL(oracle)
{
        int y;
        

        if (CMD_IS("improve"))
          {
               	skip_spaces(&argument);        	
              if (!*argument)
                {
        		send_to_char("&GOracle: &w'what spell/skill do you wanna learn?'&n\r\n", ch);
                        send_to_char("&GOracle: &w'use improve '<name of skill/spell>'&n\r\n", ch);
                        return 1; 
                }
        	
   
                if (GET_GOLD(ch) < 10000)
                {	
           	send_to_char("&GOracle: &w'You need at least &W10.000 coins &wto improve once'&n\r\n", ch);
                return 1;
           }
        }   
        else
                return 0;   
                         
        if(((y = find_skill_num(argument)) == -1) || (GET_SKILL(ch,y) < 1))
        {
          send_to_char("&wYou can't improve that skill/spell.&n\r\n", ch);
          return 1;
        }
         
    else 
       {
      	GET_GOLD(ch) -= 10000;
      	sprintf(buf, "&WYou are practicing &G%s&n.\r\n", spells[y]); 
    	send_to_char(buf, ch);
    	improve_skill(ch, y);
        return 1; 
        }  
}

SPECIAL(clan_guard)
{

  struct char_data *vict = (struct char_data *) me;
  short int clan;
  
   if(!cmd)
   {
     if(FIGHTING(vict) && !number(0,10))  
    { 
     sprintf(buf, "&r%s chats, 'Socorro! estamos sendo atacados por &C%s!'\r\n", GET_NAME(vict), GET_NAME(FIGHTING(vict)));
     send_to_all(buf);
     
     if(!(GET_CLAN(FIGHTING(vict))))
     {
     send_to_char("Qual seu interesse fazendo isso, voce e mercenario?\r\n", ch);
     return FALSE; 
	}

     if(GET_CLAN(FIGHTING(vict))) 
     {
       clans[zone_table[world[vict->in_room].zone].owner -1].raided = GET_CLAN(FIGHTING(vict));
       clans[zone_table[world[vict->in_room].zone].owner -1].seen = 10;
       clan = GET_CLAN(FIGHTING(vict));
       salvar_clan(clans[zone_table[world[vict->in_room].zone].owner -1].clan_num);
       SET_BIT(PLR_FLAGS(FIGHTING(vict)), PLR_KILLER);
       save_char(ch, ch->in_room);
     }
    }
  }

  if ((cmd == SCMD_NORTH || cmd == SCMD_EAST || cmd == SCMD_WEST || 
       cmd == SCMD_UP || cmd == SCMD_DOWN) && GET_LEVEL(ch) < LVL_IMMORT)
        {
  if(IS_NPC(ch) || !GET_CLAN(ch)) 
  {
     send_to_char("Essa e uma Clan Sede e voce nao possui um Clan.\r\n", ch);
     return TRUE;
  }

    if (GET_CLAN(ch) != zone_table[world[IN_ROOM((struct char_data *)me)].zone].owner || GET_CLAN_POS(ch) == 0) 
    {

      act("&c$N &nbloqueia sua passagem e diz: &c'Somente Membros!'&n",
          FALSE, ch, 0, (struct char_data *)me, TO_CHAR);
      act("&c$N bloqueia &g$n e diz: &c'Somente Membros!'&n",
          FALSE, ch, 0, (struct char_data *)me, TO_ROOM);
      return TRUE;
    }
  }
  return FALSE; 
}

/*SPECIAL(cassino)
	{
	int numero, vitorias=1;

	if(CMD_IS("play"))
	{
	one_argument(argument, arg);	
        	        	
              if (!*arg)
                {
        		send_to_char("\r\nYou must choose a number from 1 to 50.\r\n", ch);
                  }
	if(*arg < 1 || *arg > 50)
	{
	send_to_char("\r\nYou must choose a number from 1 to 50.\r\n", ch);
	}
	if(GET_GOLD(ch) < 1000)
	{
	send_to_char("\r\nYou need 1000 coins to play.\r\n", ch);
	}

	numero = rand() % 52;
	
	if(*arg != numero)
	{
	send_to_char("\r\nThe roulette master: You loose, The house wins!!\r\n", ch);
	GET_GOLD(ch) -= 1000;
	}
	
	GET_GOLD(ch) += 1000*vitorias;
	vitorias++;
		
	if (vitorias > 10)
	{
	send_to_char("\r\nOkay, go home now... the house is broken.\r\n", ch);
	
	}	
}	
}

*/

SPECIAL(soundroom)
{
	struct char_data *vict;
	
	
	for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
   		{
		if ((CMD_IS("meta")))
		play_sound(ch, "healerroom.wav", SND_CHAR);

	
}
return 0;
}

SPECIAL(soundroom2)
{
	struct char_data *vict;
	
	
	for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
   		{
		if ((CMD_IS("meta")))
		play_sound(ch, "healerroom.wav", SND_CHAR);

	
}
return 0;
}
SPECIAL(anelrecall)
{
	//struct char_data *tch;
	struct obj_data *usando = GET_EQ(ch, WEAR_FINGER_R);
	
	if(!CMD_IS("say"))
		return 0;

	if(!*argument)
		return 0;

if (ROOM_FLAGGED(ch->in_room, ROOM_ARENA)) {
    send_to_char("Hehehe... Too fun... Looser!\r\n", ch);
    return 0;
  }
		
		
    skip_spaces(&argument);
if (usando){
	if(!str_cmp(argument, "sflenders rejus"))
	{
			send_to_char("Voce toca o chao com seu anel e desaparece instantaneamente!&n\r\n", ch);
					
				act("$n mumbles a pray and disappears.", TRUE, ch, 0, 0, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, r_mortal_start_room);
  act("The gods drop $n in the middle of the room.", TRUE, ch, 0, 0, TO_ROOM);
  look_at_room(ch, 0);
				return 1;
				}}
		
	
	return 0;
}
