/*************************************************************************
*   File: act.informative.c                          lç   Part of CircleMUD *
*  Usage: Player-level commands of an informative nature                  *
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
#include "screen.h"
#include "constants.h"
#include "olc.h"
#include "dg_scripts.h"
#include "clan.h"

/* extern variables */
extern int top_of_helpt;
extern int godmana[9];
extern int maxgodmana[9];
extern int godpower[9] ;
extern struct help_index_element *help_table;
extern char *help;
extern struct time_info_data time_info;
extern const char *weekdays[];
extern const char *month_name[];
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct spell_info_type spell_info[];
extern struct message_data mess_info;
extern int top_of_world;
extern char *npc_class_abbrevs[];
extern char *pc_class_types[];
extern char *pc_race_types[];
extern sh_int to_fly_rooms[4];
extern const char *cnome_trans[];
extern const char *ctitle_trans[];
extern sh_int total_p;

extern char *credits;
extern char *news;
extern char *info;
extern char *motd;
extern char *imotd;
extern char *wizlist;
extern char *immlist;
extern char *policies;
extern char *handbook;
extern char *spells[];
extern char *class_abbrevs[];
extern char *race_abbrevs[];
extern const char *god_wiznames[];
extern const char *remort_names[];
extern const char *cnome_trans[];

/* extern functions */
ACMD(do_action);
long find_class_bitvector(char arg);
int level_exp(int remort, int level);
char *title_male(int chclass, int level);
char *title_female(int chclass, int level);
struct time_info_data *real_time_passed(time_t t2, time_t t1);
bool affected_by_spell(struct char_data * ch, int type);
char *add_points(int value);
int parse_class(char arg);
extern struct help_index_element *help_table;
char *item_condition(struct obj_data *obj);
extern const int wear_order_index[NUM_WEARS];
char *make_mbar(int min, int val, int max, int len, int cores);
int invalid_class(struct char_data *ch, struct obj_data *obj);
int can_pk(struct char_data * ch, struct char_data * vt);
int conta_cores(const char *texto);
int saving_throws(int class_num, int type, int level);
int thaco(int class_num, int level);
int damage(struct char_data * ch, struct char_data * victim, int dam, int attacktype) ;


/* local functions */
void print_object_location(int num, struct obj_data * obj, struct char_data * ch, int recur);
void show_obj_to_char(struct obj_data * object, struct char_data * ch, int mode);
void list_obj_to_char(struct obj_data * list, struct char_data * ch, int mode, int show);
void play_sound(struct char_data *ch, char *sound, int type);
ACMD(do_look);
ACMD(do_examine);
ACMD(do_gold);
ACMD(do_score);
ACMD(do_topten);
ACMD(do_relialin) ;
ACMD(do_inventory);
ACMD(do_equipment);
ACMD(do_time);
ACMD(do_weather);
ACMD(do_help);
ACMD(do_who);
ACMD(do_users);
ACMD(do_gen_ps);
void perform_mortal_where(struct char_data * ch, char *arg);
void perform_immort_where(struct char_data * ch, char *arg);
ACMD(do_where);
ACMD(do_levels);
ACMD(do_consider);
ACMD(do_diagnose);
ACMD(do_color);
ACMD(do_toggle);
void sort_commands(void);
ACMD(do_commands);
void diag_char_to_char(struct char_data * i, struct char_data * ch);
void look_at_char(struct char_data * i, struct char_data * ch);
void list_all_char(struct char_data * i, struct char_data * ch, int num);
void list_char_to_char(struct char_data * list, struct char_data * ch);
void do_auto_exits(struct char_data * ch);
ACMD(do_exits);
void look_in_direction(struct char_data * ch, int dir);
void look_in_obj(struct char_data * ch, char *arg);
char *find_exdesc(char *word, struct extra_descr_data * list);
void look_at_target(struct char_data * ch, char *arg);
ACMD(do_spell);
ACMD(do_skill);
ACMD(do_scan);
ACMD(do_whois);
ACMD(do_player);
ACMD(do_pray);

//ACMD(do_search); //procura portas escondidas

// religion
ACMD(do_muir);
ACMD(do_ilmanateur);
ACMD(do_selune);
ACMD(do_helm);
ACMD(do_ohgma);
ACMD(do_tempus);
ACMD(do_bane);
ACMD(do_pandora);
ACMD(do_talos);

char *make_bar(int val, int max, int len, int cores);


ACMD(do_muir)
{
 if(GET_RELIGION(ch) != 1) {
  send_to_char("Essa nao eh sua religiao\r\n", ch); 
  return ;
 }

 if(godmana[0] < godpower[0])
  send_to_char("Muir nao tem poder suficiente.\r\n", ch);
 else
 {
  godmana[0] = godmana[0] - godpower[0];
  send_to_room("Grande e gloriosa deusa Muir, forneca a sua ajuda em\r\n",ch->in_room);
  send_to_room("forma de uma espada de luz que ira erradicar o mal.\r\n",ch->in_room);
  mag_affects(GET_LEVEL(ch) , ch, ch, SPELL_MUIR,4);
 }
}

ACMD(do_ilmanateur)
{
 if(GET_RELIGION(ch) != 2) {
  send_to_char("Essa nao eh sua religiao\r\n", ch);
  return ;
 } 

 if(godmana[1] < godpower[1])
  send_to_char("Ilmanateur nao tem poder suficiente.\r\n", ch);
 else
 {
  godmana[1] = godmana[1] - godpower[1];
  send_to_room("Senhor do perdao e da humildade, que seu sangue e\r\n",ch->in_room) ;
  send_to_room("carne nos proteja como um escudo da fe.\r\n",ch->in_room);
  mag_affects(GET_LEVEL(ch) , ch, ch, SPELL_ILMANATEUR,4);
 }
}

ACMD(do_selune)
{
 if(GET_RELIGION(ch) != 3) {
  send_to_char("Essa nao eh sua religiao\r\n", ch);
  return ;
 }

 if(godmana[2] < godpower[2])
  send_to_char("Selune nao tem poder suficiente.\r\n", ch);
 else
 {
  godmana[2] = godmana[2] - godpower[2];
  send_to_room("Dama da lua e criatividade, que a sua luz branca\r\n",ch->in_room);
  send_to_room("de amor enfraqueca os coracoes sanguinarios.\r\n",ch->in_room);
  mag_affects(GET_LEVEL(ch) , ch, ch, SPELL_SELUNE,4);
 }
}

ACMD(do_helm)
{
  if(GET_RELIGION(ch) != 4) {
  send_to_char("Essa nao eh sua religiao\r\n", ch);
  return ;
 } 

 if(godmana[3] < godpower[3])
  send_to_char("Helm nao tem poder suficiente.\r\n", ch);
 else
 {
  godmana[3] = godmana[3] - godpower[3];
  send_to_room("Honrado e ordeiro Helm, faca com que os seus servos\r\n",ch->in_room);
  send_to_room("se unam como uma bainha e uma lamina de espada que\r\n",ch->in_room);
  send_to_room("ira cortar o caos e a loucura.\r\n",ch->in_room);
  mag_summons(GET_LEVEL(ch) , ch, 0, SPELL_HELM,4);
 }
}

ACMD(do_ohgma)
{
 struct char_data *found_char = NULL;
 struct obj_data *found_obj = NULL;

 if(GET_RELIGION(ch) != 5) {
  send_to_char("Essa nao eh sua religiao\r\n", ch);
  return ;
 }

 one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Identify what?\r\n", ch);
    return;
  }

 generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP |FIND_CHAR_ROOM, ch,
              &found_char, &found_obj);

  if (found_char != NULL)
  {
   call_magic(ch,found_char,0,SPELL_OGHMA,GET_LEVEL(ch), CAST_SPELL) ;
   return ;  
  } 

  if (found_obj != NULL) 
  {
   call_magic(ch,0,found_obj,SPELL_IDENTIFY,GET_LEVEL(ch), CAST_SPELL) ;
   return ;  
  } 
}

ACMD(do_tempus)
{

  if(GET_RELIGION(ch) != 6) {
  send_to_char("Essa nao eh sua religiao\r\n", ch);
  return ;
 }

  if(godmana[5] < godpower[5])
  {
   send_to_char("Tempus nao tem poder suficiente.\r\n", ch);
   return ; 
  }
 
  godmana[5] = godmana[5] - godpower[5];
  send_to_room("Lorde da guerra e protetor do mais forte, forneca-me a\r\n",ch->in_room);
  send_to_room("determinacao do fogo e a coragem do aco.\r\n",ch->in_room);
  mag_affects(GET_LEVEL(ch) , ch, ch, SPELL_TEMPUS,4);
  send_to_char("You got the power of the scourge of Battle.\r\n", ch);
}

ACMD(do_bane)
{
  
  struct char_data * vict = FIGHTING(ch);
 
  if(GET_RELIGION(ch) != 7) {
  send_to_char("Essa nao eh sua religiao\r\n", ch);
  return ;
 }

 if((godmana[6] < godpower[6])  && (GET_LEVEL(ch) < 208))
  send_to_char("Bane nao tem poder suficiente.\r\n", ch);
 else
 {
  if(vict == NULL)
  {
   send_to_char("Eh necessario estar em combate para se usar o poder de Bane.\r\n",ch);
   return ;
  }
 
  if (ch->in_room != vict->in_room)
   return ;                           // para evitar bugs

  godmana[6] = godmana[6] - godpower[6];
  send_to_room("Podoroso lorde negro Bane, que o seu punho de tirania\r\n",ch->in_room);
  send_to_room("forneca-me poder para acabar com os meus inimigos.\r\n",ch->in_room);
  damage(ch,vict,1000 + (GET_REMORT(ch)* 200), 700);
 }

}

ACMD(do_pandora)
{
  struct char_data * vict = FIGHTING(ch);

  if(GET_RELIGION(ch) != 8) {
  send_to_char("Essa nao eh sua religiao\r\n", ch);
  return ;
 }

 if(godmana[7] < godpower[7])
  send_to_char("Pandora nao tem poder suficiente.\r\n", ch);
 else
 {
  if(vict == NULL)
  {
   send_to_char("Eh necessario estar em combate para se usar o poder de Pandora.\r\n",ch);
   return ;
  }

  if ((ch->in_room != vict->in_room))
   return ;                           // para evitar bugs

  godmana[7] = godmana[7] - godpower[7];
  send_to_room("Maligna e sedutora Pandora, que o seu toque limite a\r\n",ch->in_room);
  send_to_room("forca fisica e sexual de meu oponente..\r\n",ch->in_room);
  if(GET_HIT(vict) > 1)
  { 
   GET_HIT(vict) = GET_HIT(vict) * 0.5 ;
   send_to_char("O toque de Pandora retirou 50% da sua energia vital.\r\n", vict);
  }
 }
}

ACMD(do_talos)
{
 if(GET_RELIGION(ch) != 9) {
  send_to_char("Essa nao eh sua religiao\r\n", ch);
  return ;
 } 

  if(godmana[8] < godpower[8])
  send_to_char("Talos nao tem poder suficiente.\r\n", ch);
 else
 {
  godmana[8] = godmana[8] - godpower[8];
  send_to_room("Grande e poderoso Talos, que o seu sopro se transforme numa\r\n",ch->in_room);
  send_to_room("tempestade que ira livrar o mundo dos fracos e inferiores.\r\n",ch->in_room);
  mag_areas(GET_LEVEL(ch) , ch,  SPELL_TALOS,4);
 }
}


ACMD(do_pray)
{
 int i = GET_RELIGION(ch);
 if(i > 0)
 {
  sprintf(buf, "Seu Deus tem %d/%d pontos divinos.\r\n", godmana[i-1],
      maxgodmana[i-1]);
  send_to_char(buf, ch);
  sprintf(buf, "Ele precisa de %d pontos para poder ajudar.\r\n", 
     godpower[i-1]);
  send_to_char(buf, ch);

 }
 else send_to_char("You have no religion.\r\n", ch);
  
} 

ACMD(do_relialin)
{

   switch (GET_RELIGION(ch))
 {
      case  1:
        send_to_char("You are a Justice Guardian of Muir.\r\n", ch) ;
        break ;
      case  2:
        send_to_char("You are a Pain Bearer of Ilmanateur.\r\n", ch) ;
        break ;
      case  3:
        send_to_char("You are a Moon Blade of Selune.\r\n", ch) ;
        break ;
      case  4:
        send_to_char("You are a Law Keeper of Helm.\r\n", ch) ;
        break ;
      case  5:
        send_to_char("You are a Wisdom Seeker of Ohgma.\r\n", ch) ;
        break ;
      case  6:
        send_to_char("You are a Battle Guard of Tempus.\r\n", ch) ;
        break ;
      case  7:
        send_to_char("You are a Dread Master of Bane.\r\n", ch) ;
        break ;
      case  8:
        send_to_char("You are a Killer Rose of Pandora.\r\n", ch) ;
        break ;
      case  9:
        send_to_char("You are a Storm Lord of Talos.\r\n", ch) ;
        break ;
     default :
        send_to_char("You have no religion.\r\n", ch) ;
        break ;
  }

   switch (GET_ALIGNMENT(ch))
 {
      case  999:
        send_to_char("You are Lawful Good.\r\n", ch) ;
        break ;
      case  1000:
        send_to_char("You are Neutral Good.\r\n", ch) ;
        break ;
      case  994:
        send_to_char("You are Chaotic Good.\r\n", ch) ;
        break ;
      case  3:
        send_to_char("You are Lawful Neutral.\r\n", ch) ;
        break ;
      case  5:
        send_to_char("You are True Neutral.\r\n", ch) ;
        break ;
      case  7:
        send_to_char("You are Chaotic Neutral.\r\n", ch) ;
        break ;
      case  -999:
        send_to_char("You are Lawful Evil.\r\n", ch) ;
        break ;
      case  -1000:
        send_to_char("You are Neutral Evil.\r\n", ch) ;
        break ;
      case  -994:
        send_to_char("You are Chaotic Evil.\r\n", ch) ;
        break ;
     default :
        send_to_char("You have no alignment. Report to Luigi.\r\n", ch) ;
        break ;
  }
}



ACMD(do_topten)
{
  int aux ;
  struct toptenlevel
 {
  char nome[30] ;
  int level ;
  long id ;
 } ;

 FILE *fl;

 const char *filename ;
 struct toptenlevel topvetor[10] ;

 one_argument(argument, arg);

 if (!*arg)
 {
    send_to_char("&cType: &CTopTen &c0&C/&c1&C/&c2&n\r\n", ch);
    return;
 }

 aux = atoi(arg);

 switch(aux)
 {
  case 0 : filename = TOPTEN_FILE ; break ;
  case 1 : filename = TOPTEN_FILE1 ; break ;
  case 2 : filename = TOPTEN_FILE2 ; break ;
  default: return ;
 }

 if (!(fl = fopen(filename, "r")))
 {
  send_to_char("Could not open the file. Report to Luigi.\r\n", ch);
  return;
 }

 fread(topvetor, sizeof(struct toptenlevel[10]),1, fl)   ;
 fclose(fl);


  if(aux == 0)
  {
sprintf(buf, " \r\n");send_to_char(buf, ch);
sprintf(buf, "                           &gWardome &CTOPTEN &gQuest\r\n");send_to_char(buf, ch);
sprintf(buf, "&w+------+----------------+------------------------------+---------------+\r\n");send_to_char(buf, ch);
sprintf(buf, "&w| &cRank &w| &gName           &w| &cFlagged As                   &w|  &gQuestPoints  &w|\r\n");send_to_char(buf, ch);
sprintf(buf, "&w+------+----------------+------------------------------+---------------+\r\n");send_to_char(buf, ch);
sprintf(buf, "&w|  &c1   &w| &g%-10s     &w| &cThe &Csupreme &gQuestMaster      &w|      &g%-3d     &w|  \r\n",topvetor[0].nome, topvetor[0].level);send_to_char(buf, ch);
sprintf(buf, "&w|  &c2   &w| &g%-10s     &w| &cThe &Cincredible &gQuestMaster   &w|      &g%-3d     &w|  \r\n",topvetor[1].nome, topvetor[1].level);send_to_char(buf, ch);
sprintf(buf, "&w|  &c3   &w| &g%-10s     &w| &cThe &Camazing  &gQuestMaster     &w|      &g%-3d     &w|  \r\n",topvetor[2].nome, topvetor[2].level);send_to_char(buf, ch);
sprintf(buf, "&w|  &c4   &w| &g%-10s     &w| &cThe &Cglorious &gQuestMaster     &w|      &g%-3d     &w|  \r\n",topvetor[3].nome, topvetor[3].level);send_to_char(buf, ch);
sprintf(buf, "&w|  &c5   &w| &g%-10s     &w| &cThe &Cinfamous &gQuestMaster     &w|      &g%-3d     &w|  \r\n",topvetor[4].nome, topvetor[4].level);send_to_char(buf, ch);
sprintf(buf, "&w|  &c6   &w| &g%-10s     &w| &cThe &Cwonderful &gQuestMaster    &w|      &g%-3d     &w|  \r\n",topvetor[5].nome, topvetor[5].level);send_to_char(buf, ch);
sprintf(buf, "&w|  &c7   &w| &g%-10s     &w| &cThe &Cgreat &gQuestMaster        &w|      &g%-3d     &w|  \r\n",topvetor[6].nome, topvetor[6].level);send_to_char(buf, ch);
sprintf(buf, "&w|  &c8   &w| &g%-10s     &w| &cThe &Cgood &gQuestMaster         &w|      &g%-3d     &w|  \r\n",topvetor[7].nome, topvetor[7].level);send_to_char(buf, ch);
sprintf(buf, "&w|  &c9   &w| &g%-10s     &w| &cThe &Csmart &gQuestMaster        &w|      &g%-3d     &w|  \r\n",topvetor[8].nome, topvetor[8].level);send_to_char(buf, ch);
sprintf(buf, "&w| &c10   &w| &g%-10s     &w| &cThe &CQuestMaster              &w|      &g%-3d     &w|  \r\n",topvetor[9].nome, topvetor[9].level);send_to_char(buf, ch);
sprintf(buf, "&w+------+----------------+------------------------------+---------------+&n \r\n");send_to_char(buf, ch);
}

  if(aux == 1)
  {
sprintf(buf, "\r\n");send_to_char(buf, ch);
sprintf(buf, "                             &rWardome &CTOPTEN &rArena\r\n");send_to_char(buf, ch);
sprintf(buf, "&w+------+----------------+------------------------------+---------------+ \r\n");send_to_char(buf, ch);
sprintf(buf, "&w| &cRank &w| &rName           &w| &cFlagged As                   &w|  &rArena Score  &w|\r\n");send_to_char(buf, ch);
sprintf(buf, "&w+------+----------------+------------------------------+---------------+ \r\n");send_to_char(buf, ch);
sprintf(buf, "&w|  &c1   &w| &r%-10s     &w| &cThe &Csupreme &cGladiator        &w|      &r%-5d    &w|\r\n",topvetor[0].nome, topvetor[0].level);send_to_char(buf, ch);
sprintf(buf, "&w|  &c2   &w| &r%-10s     &w| &cThe &Cincredible &cGladiator     &w|      &r%-5d    &w|\r\n",topvetor[1].nome, topvetor[1].level);send_to_char(buf, ch);
sprintf(buf, "&w|  &c3   &w| &r%-10s     &w| &cThe &Camazing  &cGladiator       &w|      &r%-5d    &w|\r\n",topvetor[2].nome, topvetor[2].level);send_to_char(buf, ch);
sprintf(buf, "&w|  &c4   &w| &r%-10s     &w| &cThe &Cglorious &cGladiator       &w|      &r%-5d    &w|\r\n",topvetor[3].nome, topvetor[3].level);send_to_char(buf, ch);
sprintf(buf, "&w|  &c5   &w| &r%-10s     &w| &cThe &Cinfamous &cGladiator       &w|      &r%-5d    &w|\r\n",topvetor[4].nome, topvetor[4].level);send_to_char(buf, ch);
sprintf(buf, "&w|  &c6   &w| &r%-10s     &w| &cThe &Cwonderful &cGladiator      &w|      &r%-5d    &w|\r\n",topvetor[5].nome, topvetor[5].level);send_to_char(buf, ch);
sprintf(buf, "&w|  &c7   &w| &r%-10s     &w| &cThe &Cgreat &cGladiator          &w|      &r%-5d    &w|\r\n",topvetor[6].nome, topvetor[6].level);send_to_char(buf, ch);
sprintf(buf, "&w|  &c8   &w| &r%-10s     &w| &cThe &Cgood &cGladiator           &w|      &r%-5d    &w|\r\n",topvetor[7].nome, topvetor[7].level);send_to_char(buf, ch);
sprintf(buf, "&w|  &c9   &w| &r%-10s     &w| &cThe &Csmart &cGladiator          &w|      &r%-5d    &w|\r\n",topvetor[8].nome, topvetor[8].level);send_to_char(buf, ch);
sprintf(buf, "&w| &c10   &w| &r%-10s     &w| &cThe &CGladiator                &w|      &r%-5d    &w|  \r\n",topvetor[9].nome, topvetor[9].level);send_to_char(buf, ch);
sprintf(buf, "&w+------+----------------+------------------------------+---------------+&n \r\n");send_to_char(buf, ch);
  }

  if(aux == 2)
  {
sprintf(buf, "\r\n");send_to_char(buf, ch);
sprintf(buf, "                               &mWardome &CTOPTEN &mLevels \r\n"); send_to_char(buf, ch);
sprintf(buf, "&w+------+----------------+------------------------------+---------------+ \r\n");send_to_char(buf, ch);
sprintf(buf, "&w| &cRank &w| &mName           &w| &cFlagged As                   &w|     &mLevels    &w| \r\n");send_to_char(buf, ch);
sprintf(buf, "&w+------+----------------+------------------------------+---------------+ \r\n");send_to_char(buf, ch);
sprintf(buf, "&w|  &c1   &w| &m%-10s     &w| &cThe &Csupreme &cLevelMaster      &w|      &m%-5d    &w| \r\n",topvetor[0].nome, topvetor[0].level);send_to_char(buf, ch);
sprintf(buf, "&w|  &c2   &w| &m%-10s     &w| &cThe &Cincredible &cLevelMaster   &w|      &m%-5d    &w| \r\n",topvetor[1].nome, topvetor[1].level);send_to_char(buf, ch);
sprintf(buf, "&w|  &c3   &w| &m%-10s     &w| &cThe &Camazing  &cLevelMaster     &w|      &m%-5d    &w| \r\n",topvetor[2].nome, topvetor[2].level);send_to_char(buf, ch);
sprintf(buf, "&w|  &c4   &w| &m%-10s     &w| &cThe &Cglorious &cLevelMaster     &w|      &m%-5d    &w| \r\n",topvetor[3].nome, topvetor[3].level);send_to_char(buf, ch);
sprintf(buf, "&w|  &c5   &w| &m%-10s     &w| &cThe &Cinfamous &cLevelMaster     &w|      &m%-5d    &w| \r\n",topvetor[4].nome, topvetor[4].level);send_to_char(buf, ch);
sprintf(buf, "&w|  &c6   &w| &m%-10s     &w| &cThe &Cwonderful &cLevelMaster    &w|      &m%-5d    &w| \r\n",topvetor[5].nome, topvetor[5].level);send_to_char(buf, ch);
sprintf(buf, "&w|  &c7   &w| &m%-10s     &w| &cThe &Cgreat &cLevelMaster        &w|      &m%-5d    &w| \r\n",topvetor[6].nome, topvetor[6].level);send_to_char(buf, ch);
sprintf(buf, "&w|  &c8   &w| &m%-10s     &w| &cThe &Cgood &cLevelMaster         &w|      &m%-5d    &w| \r\n",topvetor[7].nome, topvetor[7].level);send_to_char(buf, ch);
sprintf(buf, "&w|  &c9   &w| &m%-10s     &w| &cThe &Csmart &cLevelMaster        &w|      &m%-5d    &w| \r\n",topvetor[8].nome, topvetor[8].level);send_to_char(buf, ch);
sprintf(buf, "&w| &c10   &w| &m%-10s     &w| &cThe &CLevelMaster              &w|      &m%-5d    &w| \r\n",topvetor[9].nome, topvetor[9].level);send_to_char(buf, ch);
sprintf(buf, "&w+------+----------------+------------------------------+---------------+ \r\n");send_to_char(buf, ch);
  }

}


// Fenix - WHS
/*void list_objs_in_house(struct char_data *ch)
{
   struct obj_data *i;
   bool found;
   int how_many = 0;

   found = FALSE;
   send_to_char("\r\n&g", ch);
   for (i = world[ch->in_room].contents; i ; i = i->next_content ) {
	  if (CAN_SEE_OBJ(ch, i)) {
		  found = TRUE;
		  if (i->next_content &&
			  (i->item_number == i->next_content->item_number) &&
			  (i->obj_flags.extra_flags == i->next_content->obj_flags.extra_flags) &&
			  !str_cmp(i->short_description, i->next_content->short_description))
			  {
			  how_many++;
		  } else {
			  if (how_many) {
				  sprintf(buf, "&g[&G%2d&g] ", how_many + 1);
				  send_to_char(buf, ch);
				  how_many = 0;
			  }
			  send_to_char("&g", ch);
			  sprintf(buf, "%s.\r\n&n", i->short_description);
			  CAP(buf);
			  send_to_char(buf, ch);
	}
    }
  }
}
*/
/*
 * This function screams bitvector... -gg 6/45/98
 */
void show_obj_to_char(struct obj_data * object, struct char_data * ch, int mode)
{
  bool found;
  int remort, level;

  if (!object || !ch) {
    log("SYSERR: NULL pointer in show_obj_to_char(): obj=%p ch=%p", object, ch);
    return;
  }

  *buf = '\0';

  if ((mode == 0) && object->description)
    strcpy(buf, object->description);
  else if (object->short_description && ((mode == 1) ||
                                 (mode == 2) || (mode == 3) || (mode == 4)))
    strcpy(buf, object->short_description);
  else if (mode == 5) {
    if (GET_OBJ_TYPE(object) == ITEM_NOTE) {
      if (object->action_description) {
        strcpy(buf, "There is something written upon it:\r\n\r\n");
        strcat(buf, object->action_description);
        page_string(ch->desc, buf, 1);
      } else
         send_to_char("It's blank.\r\n", ch);
      return;
    } else if (GET_OBJ_TYPE(object) != ITEM_DRINKCON) {
      strcpy(buf, "You see nothing special..");
    } else                      /* ITEM_TYPE == ITEM_DRINKCON||FOUNTAIN */
      strcpy(buf, "It looks like a drink container.");
  }
  if (mode != 3) {
    found = FALSE;
    if (GET_OBJ_TYPE(object) == ITEM_WEAPON || GET_OBJ_TYPE(object) == ITEM_ARMOR ||
    	GET_OBJ_TYPE(object) == ITEM_WAND || GET_OBJ_TYPE(object) == ITEM_LIGHT ||
    	GET_OBJ_TYPE(object) == ITEM_STAFF || GET_OBJ_TYPE(object) == ITEM_WORN ||
    	(GET_OBJ_TYPE(object) == ITEM_CONTAINER && (object->carried_by || object->worn_by) && !IS_CORPSE(object))) {
      sprintf(buf+strlen(buf), "%s", item_condition(object));
      found = TRUE;
    } else {
      strcat(buf, " ");
      found = TRUE;
    }

    if(invalid_class(ch, object) && GET_LEVEL(ch) < LVL_GOD) {
     if (GET_OBJ_TYPE(object) == ITEM_WEAPON || GET_OBJ_TYPE(object) == ITEM_ARMOR ||
    	GET_OBJ_TYPE(object) == ITEM_WAND || GET_OBJ_TYPE(object) == ITEM_LIGHT ||
    	GET_OBJ_TYPE(object) == ITEM_STAFF || GET_OBJ_TYPE(object) == ITEM_WORN ||
    	(GET_OBJ_TYPE(object) == ITEM_CONTAINER && (object->carried_by || object->worn_by) && !IS_CORPSE(object)))
         strcat(buf, "&R(!)&n ");
      found = TRUE;
    }

    level = (GET_OBJ_LEVEL(object)-((LVL_IMMORT - 1)*(GET_OBJ_LEVEL(object)/(LVL_IMMORT - 1))));
    if ((level == 0) && (GET_OBJ_LEVEL(object) > 0)){
     level = (LVL_IMMORT - 1);
     remort = (GET_OBJ_LEVEL(object)/(LVL_IMMORT - 1)) - 1;
    }
    else{
     remort = (GET_OBJ_LEVEL(object)/(LVL_IMMORT - 1));
    }

    if (GET_REMORT(ch) < remort) {
      strcat(buf, "&y(&GR&Y+&y)&n ");
      found = TRUE;
    } else if (GET_LEVEL(ch) < level ) {
      strcat(buf, "&Y(+)&n ");
      found = TRUE;
    }

    if (IS_OBJ_STAT(object, ITEM_INVISIBLE)) {
      strcat(buf, "&W(invis&n) ");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_BLESS) && AFF_FLAGGED(ch, AFF_DETECT_ALIGN)) {
      strcat(buf, "&B(blessed)&n ");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_MAGIC) && AFF_FLAGGED(ch, AFF_DETECT_MAGIC)) {
      strcat(buf, "&C(magic)&n ");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_GLOW)) {
      strcat(buf, "&G(glow)&n ");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_HUM)) {
      strcat(buf, "&c(hum)&n ");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_QUEST)) {
    	strcat(buf, "&M(quest)&n ");
    	found = TRUE;
	}
	
	if (IS_OBJ_STAT(object, ITEM_FIRE)) {
		strcat(buf, "&R(&YF&Rl&Ya&Rm&Yi&Rn&Yg&R)&n");
		found = TRUE;
	}
	if (IS_OBJ_STAT(object, ITEM_WATER)) {
		strcat(buf, "&W(&CBub&Wling)&n");
		found = TRUE;
	}
	/*if (IS_OBJ_STAT(object, ITEM_EARTH)) {
		strcat(buf, "(Quaking)");
		found = TRUE;
	}
	if (IS_OBJ_STAT(object, ITEM_WIND)) {
		strcat(buf, "(Blowing)");
		found = TRUE;
	}*/
  }
  strcat(buf, "\r\n");
  send_to_char(buf, ch);
//  page_string(ch->desc, buf, TRUE);
}

/*
void list_obj_to_char(struct obj_data * list, struct char_data * ch, int mode,
                           int show)
{
  struct obj_data *i;
  bool found;

  found = FALSE;
  for (i = list; i; i = i->next_content) {
    if (CAN_SEE_OBJ(ch, i)) {
      show_obj_to_char(i, ch, mode);
      found = TRUE;
    }
  }
  if (!found && show)
    send_to_char(" Nothing.\r\n", ch);
}
*/

void list_obj_to_char(struct obj_data * list, struct char_data * ch, int mode, int show)
{
  struct obj_data *i, *j;
  bool found;
  int num;

  found = FALSE;
  for (i = list; i; i = i->next_content) {
      num = 0;
      for (j = list; j != i; j = j->next_content)
	if (j->item_number==NOTHING) {
	    if(strcmp(j->short_description,i->short_description)==0) break;
	}
	else if (j->item_number==i->item_number) break;
      if (j!=i) continue;
      for (j = i; j; j = j->next_content)
	if (j->item_number==NOTHING) {
	    if(strcmp(j->short_description,i->short_description)==0) num++;
	  }
	else if (j->item_number==i->item_number) num++;

      if ((CAN_SEE_OBJ(ch, i) || (GET_OBJ_TYPE(i) == ITEM_LIGHT)) && !is_abbrev(i->description, "Undefined")) {
	  if (num!=1) {
		sprintf(buf,"&g(&G%d&g) ",num);
		send_to_char(buf,ch);
          } else if(num == 1) {
		sprintf(buf,"&g");
		send_to_char(buf,ch);
	  }
	if(!is_abbrev(i->description, "Undefined")){
	  show_obj_to_char(i, ch, mode);
	  found = TRUE;
	} else
	  found = FALSE;
      }
  }
  if (!found && show)
    send_to_char(" Nothing.\r\n", ch);
}



void diag_char_to_char(struct char_data * i, struct char_data * ch)
{
  int percent;

  if (GET_MAX_HIT(i) > 0)
    percent = (100 * GET_HIT(i)) / GET_MAX_HIT(i);
  else
    percent = -1;               /* How could MAX_HIT be < 1?? */

  strcpy(buf, PERS(i, ch));
  CAP(buf);

  if (percent >= 100)
    strcat(buf, " is in excellent condition.\r\n");
  else if (percent >= 90)
    strcat(buf, " has a few scratches.\r\n");
  else if (percent >= 75)
    strcat(buf, " has some small wounds and bruises.\r\n");
  else if (percent >= 50)
    strcat(buf, " has quite a few wounds.\r\n");
  else if (percent >= 30)
    strcat(buf, " has some big nasty wounds and scratches.\r\n");
  else if (percent >= 15)
    strcat(buf, " looks pretty hurt.\r\n");
  else if (percent >= 0)
    strcat(buf, " is in awful condition.\r\n");
  else
    strcat(buf, " is bleeding awfully from big wounds.\r\n");

  send_to_char(buf, ch);
}


void look_at_char(struct char_data * i, struct char_data * ch)
{
  int j, found;

  if (!ch->desc)
    return;

   if (i->player.description)
    send_to_char(i->player.description, ch);
  else
    act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);

  diag_char_to_char(i, ch);

  found = FALSE;
  for (j = 0; !found && j < NUM_WEARS; j++)
    if (GET_EQ(i, j) && CAN_SEE_OBJ(ch, GET_EQ(i, j)))
      found = TRUE;

  if (found) {
    send_to_char("\r\n", ch);	/* act() does capitalization. */
    act("&c$U$n is using:&n", FALSE, i, 0, ch, TO_VICT);
    for (j = 0; j < NUM_WEARS; j++)
      if (GET_EQ(i, wear_order_index[j])
          && CAN_SEE_OBJ(ch, GET_EQ(i, wear_order_index[j]))) {
        send_to_char(where[wear_order_index[j]], ch);
        send_to_char("&y", ch);
        show_obj_to_char(GET_EQ(i, wear_order_index[j]), ch, 1);
        send_to_char("&n", ch);
      }
  }
  if (ch != i && (GET_SKILL(ch, SKILL_PEEK) || GET_LEVEL(ch) >= LVL_IMMORT)) {
    found = FALSE;
    act("\r\n&GYou attempt to peek at $s inventory:&n", FALSE, i, 0, ch, TO_VICT);
     if(i->carrying){
        found = TRUE;
        list_obj_to_char(i->carrying, ch, 1, TRUE);
     }
    if (!found)
      send_to_char("&yYou can't see anything.&n\r\n", ch);

   if (IS_AFFECTED(i, AFF_TANGLED))
      act("$e is entangled in vines.", FALSE, i, 0, ch, TO_VICT);
  
  }
}


/*void look_at_char(struct char_data * i, struct char_data * ch)
{
  int j, found;
  struct obj_data *tmp_obj;

  if (!ch->desc)
    return;

   if (i->player.description)
    send_to_char(i->player.description, ch);
  else
    act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);

  diag_char_to_char(i, ch);

  found = FALSE;
  for (j = 0; !found && j < NUM_WEARS; j++)
    if (GET_EQ(i, j) && CAN_SEE_OBJ(ch, GET_EQ(i, j)))
      found = TRUE;

  if (found) {
    act("\r\n&c$n is using:&n", FALSE, i, 0, ch, TO_VICT);
    for (j = 0; j < NUM_WEARS; j++)
      if (GET_EQ(i, j) && CAN_SEE_OBJ(ch, GET_EQ(i, j))) {
	send_to_char(where[j], ch);
        send_to_char("&y", ch);
	show_obj_to_char(GET_EQ(i, j), ch, 1);
        send_to_char("&n", ch);
      }
  }
  if (ch != i && (IS_THIEF(ch) || GET_LEVEL(ch) >= LVL_IMMORT)) {
    found = FALSE;
    act("\r\n&GYou attempt to peek at $s inventory:&n", FALSE, i, 0, ch, TO_VICT);
    for (tmp_obj = i->carrying; tmp_obj; tmp_obj = tmp_obj->next_content) {
      if (CAN_SEE_OBJ(ch, tmp_obj) && (number(0, 20) < GET_LEVEL(ch))) {
	show_obj_to_char(tmp_obj, ch, 1);
	found = TRUE;
      }
    }

    if (!found)
      send_to_char("&yYou can't see anything.&n\r\n", ch);

   if (IS_AFFECTED(i, AFF_TANGLED))
      act("$e is entangled in vines.", FALSE, i, 0, ch, TO_VICT);
  }
}
*/
void list_all_char(struct char_data * i, struct char_data * ch, int num)
{
  const char *positions[] = {
    " is lying here, dead.",
    " is lying here, mortally wounded.",
    " is lying here, incapacitated.",
    " is lying here, stunned.",
    " is sleeping here. (...ZzZzZ)",
    " is resting here.",
    " is sitting here.",
    "!FIGHTING!",
    " is standing here.",
    " is flying here mounting a winged dragon."
  };

  if (IS_NPC(i) && i->player.long_descr && GET_POS(i) == GET_DEFAULT_POS(i))
  {
    if (AFF_FLAGGED(i, AFF_INVISIBLE))
      strcpy(buf, "*");
    else
      *buf = '\0';

    if (AFF_FLAGGED(ch, AFF_DETECT_ALIGN))
    {
      if (IS_EVIL(i))
        strcat(buf, "&R(Red Aura)&n ");
      else if (IS_GOOD(i))
        strcat(buf, "&B(Blue Aura)&n ");
    }
    if (num > 1)
            {
                while ((buf[strlen(buf)-1]=='\r') ||
                       (buf[strlen(buf)-1]=='\n') ||
                       (buf[strlen(buf)-1]==' '))
                {
                    buf[strlen(buf)-1] = '\0';
                }
                sprintf(buf2," &y[&Y%2d&y]&n ", num);
                strcat(buf, buf2);
            }
    sprintf(buf + strlen(buf), "&y%s&n", i->player.long_descr);
    send_to_char(buf, ch);

   if (AFF_FLAGGED(ch, AFF_DETECT_MAGIC) || GET_LEVEL(ch) > LVL_ELDER)
    {
     if (AFF_FLAGGED(i, AFF_SANCTUARY))
       act("&c...&w$e glows with a bright light!&n", FALSE, i, 0, ch, TO_VICT);
     if (AFF_FLAGGED(i, AFF_SATAN))
       act("&c...&R$e glows with a red light!&n", FALSE, i, 0, ch, TO_VICT);
     if (AFF_FLAGGED(i, AFF_GOD))
       act("&c...&Y$e glows with a yellow light!&n", FALSE, i, 0, ch, TO_VICT);
     if (IS_AFFECTED(i, AFF_TANGLED))
       act("&c...$e is entangled in vines!&n", FALSE, i, 0, ch, TO_VICT);
     if (IS_AFFECTED(i, AFF_FIRESHIELD))
       act("&c...&r$e is surrounded by burning flames!&n", FALSE, i, 0, ch, TO_VICT);
     if (AFF_FLAGGED(i, AFF_BLIND))
       act("&c...&W$e is groping around blindly!&n", FALSE, i, 0, ch, TO_VICT);
    if (AFF2_FLAGGED(i, AFF2_BANSHEE))
       act("&c...&m$e glows with a dark light!&n", FALSE, i, 0, ch, TO_VICT);
    }
    return;
  }
  if (IS_NPC(i)) {
    *buf = '\0';
    if (num > 1)
            {
                while ((buf[strlen(buf)-1]=='\r') ||
                       (buf[strlen(buf)-1]=='\n') ||
                       (buf[strlen(buf)-1]==' '))
                {
                    buf[strlen(buf)-1] = '\0';
                }
                sprintf(buf2," &y[&Y%2d&y]&n ", num);
                strcpy(buf, buf2);
            }
    sprintf(buf1, "&y%s&n", CAP(i->player.short_descr));
    strcat(buf, buf1);
//    CAP(buf);
  } else
    if(AFF2_FLAGGED(i, AFF2_TRANSFORM))
      sprintf(buf, "%s &Y%s %s&n", cnome_trans[GET_TRANS(i)], i->player.name,
       ctitle_trans[GET_TRANS(i)]);
    else
      sprintf(buf, "&y%s&c%s&n &y%s&n", (GET_PRENAME(i) ? GET_PRENAME(i) : ""),
      i->player.name, GET_TITLE(i));

  if (AFF_FLAGGED(i, AFF_INVISIBLE))
    strcat(buf, " &W(invisible)&n");
  if (AFF_FLAGGED(i, AFF_HIDE))
    strcat(buf, " &G(hidden)&n");
  if (!IS_NPC(i) && !i->desc)
    strcat(buf, " &B(linkless)&n");
  if (PLR_FLAGGED(i, PLR_WRITING))
    strcat(buf, " &n(writing)&n");
  if (PLR_FLAGGED(i, PLR_MEDITATE))
    strcat(buf, " &C(mediting)&n");

  if (GET_POS(i) != POS_FIGHTING){
    if ((real_room(to_fly_rooms[0]) == i->in_room) ||
        (real_room(to_fly_rooms[1]) == i->in_room) ||
        (real_room(to_fly_rooms[2]) == i->in_room))
      {
    sprintf(buf + strlen(buf), "&y%s&n", positions[(int) 9 ]);
    }
    else{
     sprintf(buf + strlen(buf), "&y%s&n", positions[(int) GET_POS(i)]);
    }
  }

//    strcat(buf, buf2);
  else {
    if (FIGHTING(i)) {
      strcat(buf, " &yis here, fighting&n ");
      if (FIGHTING(i) == ch)
        strcat(buf, "&WYOU!&n");
      else {
        if (i->in_room == FIGHTING(i)->in_room)
          strcat(buf, PERS(FIGHTING(i), ch));
        else
          strcat(buf, "&ysomeone who has already left&n");
        strcat(buf, "!");
      }
    } else                      /* NIL fighting pointer */
      strcat(buf, " &cis here struggling with thin air.&n");
  }

  if (AFF_FLAGGED(ch, AFF_DETECT_ALIGN)) {
    if (IS_EVIL(i))
      strcat(buf, " &R(Red Aura)&n");
    else if (IS_GOOD(i))
      strcat(buf, " &B(Blue Aura)&n");
  }
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  if(!AFF2_FLAGGED(i, AFF2_TRANSFORM)) {
   if (AFF_FLAGGED(ch, AFF_DETECT_MAGIC) || GET_LEVEL(ch) > LVL_ELDER) {
     if (AFF_FLAGGED(i, AFF_SANCTUARY))
       act("&c...&w$e glows with a bright light!&n", FALSE, i, 0, ch, TO_VICT);
     if (AFF_FLAGGED(i, AFF_SATAN))
       act("&c...&R$e glows with a red light!&n", FALSE, i, 0, ch, TO_VICT);
     if (AFF_FLAGGED(i, AFF_GOD))
       act("&c...&Y$e glows with a yellow light!&n", FALSE, i, 0, ch, TO_VICT);
     if (IS_AFFECTED(i, AFF_TANGLED))
       act("&c...$e is entangled in vines!&n", FALSE, i, 0, ch, TO_VICT);
     if (IS_AFFECTED(i, AFF_FIRESHIELD))
       act("&c...&r$e is surrounded by burning flames!&n", FALSE, i, 0, ch, TO_VICT);
     if (AFF_FLAGGED(i, AFF_BLIND))
       act("&c...&c$e is groping around blindly!&n", FALSE, i, 0, ch, TO_VICT);
     if (AFF2_FLAGGED(i, AFF2_BANSHEE))
       act("&c...&m$e glows with a dark light!&n", FALSE, i, 0, ch, TO_VICT);
   }
  } else
       act("&y...&Y$e is surrounded by a shine aura!&n", FALSE, i, 0, ch, TO_VICT);
}

/*
void list_char_to_char(struct char_data * list, struct char_data * ch)
{
  struct char_data *i;

  for (i = list; i; i = i->next_in_room)
    if (ch != i) {
      if (CAN_SEE(ch, i))
        list_one_char(i, ch);
      else if (IS_DARK(ch->in_room) && !CAN_SEE_IN_DARK(ch) &&
               AFF_FLAGGED(i, AFF_INFRAVISION))
        send_to_char("You see a pair of glowing red eyes looking your way.\r\n", ch);
    }
}*/

void list_char_to_char(struct char_data *list, struct char_data *ch)
{
    struct char_data *i, *plr_list[100];
    int num, counter, locate_list[100], found=FALSE;

    num = 0;

    for (i=list; i; i = i->next_in_room)
    {
      if(i != ch)
      {
        if (CAN_SEE(ch, i))
        {
            if (num < 100)
            {
                found = FALSE;
                for (counter=0;(counter<num&& !found);counter++)
                {

                  if (AFF_FLAGGED(ch, AFF_DETECT_MAGIC))
                   {
                    if (i->nr == plr_list[counter]->nr &&
                        (GET_POS(i) == GET_POS(plr_list[counter])) &&
                        (AFF_FLAGS(i)==AFF_FLAGS(plr_list[counter])) &&
                        (FIGHTING(i) == FIGHTING(plr_list[counter])) &&
                        !strcmp(GET_NAME(i), GET_NAME(plr_list[counter])))
                     {
                        locate_list[counter] += 1;
                        found=TRUE;
                    }
                   }
                   else
                   {
                    if (i->nr == plr_list[counter]->nr &&
                        (GET_POS(i) == GET_POS(plr_list[counter])) &&
                        (FIGHTING(i) == FIGHTING(plr_list[counter])) &&
                        !strcmp(GET_NAME(i), GET_NAME(plr_list[counter])))
                     {
                        locate_list[counter] += 1;
                        found=TRUE;
                    }
                   }

                }
                if (!found)
                {
                    plr_list[num] = i;
                    locate_list[num] = 1;
                    num++;
                }
            }
            else
            {
                list_all_char(i,ch,0);
            }
        }
        else if (IS_DARK(ch->in_room) && !CAN_SEE_IN_DARK(ch) &&
               AFF_FLAGGED(i, AFF_INFRAVISION))
             send_to_char("You see a pair of glowing red eyes looking your way.\r\n", ch);

      }
    }
    if (num) {
        for (counter=0; counter<num; counter++) {
            if (locate_list[counter] > 1) {
                list_all_char(plr_list[counter],ch,locate_list[counter]);
            } else {
                list_all_char(plr_list[counter],ch,0);
            }
        }
    }
}


void do_auto_exits(struct char_data * ch)
{
  int door, slen = 0;

  *buf = '\0';

  for (door = 0; door < NUM_OF_DIRS; door++)
    if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE) {
      if (EXIT_FLAGGED(EXIT(ch, door), EX_CLOSED))
        slen += sprintf(buf + slen, "%c ", UPPER(*dirs[door]));
     // else if (EXIT_FLAGGED(EXIT(ch, door), EX_HIDDEN)) //HIDDEN DOORS
     // 	slen += sprintf(buf + slen, "%s^%c ",LOWER(*dirs[door]));
      else
        slen += sprintf(buf + slen, "%c ", LOWER(*dirs[door]));
    }

  sprintf(buf2, "%s[ Exits: %s]%s\r\n", CCCYN(ch, C_NRM),
          *buf ? buf : "None! ", CCNRM(ch, C_NRM));

  send_to_char(buf2, ch);
}


ACMD(do_exits)
{
  int door;

  *buf = '\0';

  if (AFF_FLAGGED(ch, AFF_BLIND)) {
    send_to_char("You can't see a damned thing, you're blinded!\r\n", ch);
    return;
  }
  for (door = 0; door < NUM_OF_DIRS; door++)
    if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE &&
        !EXIT_FLAGGED(EXIT(ch, door), EX_CLOSED)) {
      if (GET_LEVEL(ch) >= LVL_IMMORT)
        sprintf(buf2, "%-5s - [%5d] %s\r\n", dirs[door],
                GET_ROOM_VNUM(EXIT(ch, door)->to_room),
                world[EXIT(ch, door)->to_room].name);
      else {
        sprintf(buf2, "%-5s - ", dirs[door]);
        if (IS_DARK(EXIT(ch, door)->to_room) && !CAN_SEE_IN_DARK(ch))
          strcat(buf2, "Too dark to tell\r\n");
        else {
          strcat(buf2, world[EXIT(ch, door)->to_room].name);
          strcat(buf2, "\r\n");
        }
      }
      strcat(buf, CAP(buf2));
    }
  send_to_char("Obvious exits:\r\n", ch);

  if (*buf)
    send_to_char(buf, ch);
  else
    send_to_char(" None.\r\n", ch);
}

#define ESIT(num, door)  (world[num].dir_option[door])

#define CAN_IR(num, door)(ESIT(num, door) && \
                         (ESIT(num, door)->to_room != NOWHERE) && \
                         !IS_SET(ESIT(num, door)->exit_info, EX_CLOSED))


int vai(struct char_data *ch, int dir)
{

   if(!EXIT(ch, dir) || EXIT(ch, dir)->to_room == NOWHERE || !CAN_GO(ch, dir))
    return 0;

   if(!ESIT(EXIT(ch, dir)->to_room, dir) ||
      (!ESIT(EXIT(ch, dir)->to_room, dir)->to_room == NOWHERE) ||
      !CAN_IR(EXIT(ch, dir)->to_room, dir))
    return 0;

   return 1;
}

void make_map(struct char_data *ch)
{
sprintf(buf3,
"\r\n"
"                        &C|      %s      &C|                \r\n"
"                        &C|      %s      &C|                \r\n"
"                        &C|      %s      &C|          &YN     \r\n"
"                        &C|      %s      &C|          &y|     \r\n"
"                        &C|  %s%s&R@%s%s  &C|      &YW &y- &R@ &y- &YE \r\n"
"                        &C|      %s      &C|          &y|     \r\n"
"                        &C|      %s      &C|          &YS     \r\n"
"                        &C|      %s      &C|                \r\n"
"                        &C|      %s      &C|                \r\n\r\n",
(vai(ch, 0) ? (CAN_IR(EXIT(ch, 0)->to_room, 0) ? "&Y^" : " ") : " "),
(vai(ch, 0) ? (CAN_IR(EXIT(ch, 0)->to_room, 0) ? "&y|" : " ") : " "),
(CAN_GO(ch, 0) ? "&Y?" : " "),
(CAN_GO(ch, 0) ? "&y|" : " "),
(vai(ch, 3) ? (CAN_IR(EXIT(ch, 3)->to_room, 3) ? "&Y<&y-" : "  ") : "  "),
(CAN_GO(ch, 3) ? "&Y?&y-" : "  "),
(CAN_GO(ch, 1) ? "&y-&Y?" : "  "),
(vai(ch, 1) ? (CAN_IR(EXIT(ch, 1)->to_room, 1) ? "&y-&Y>" : "  ") : "  "),
(CAN_GO(ch, 2) ? "&y|" : " "),
(CAN_GO(ch, 2) ? "&Y?" : " "),
(vai(ch, 2) ? (CAN_IR(EXIT(ch, 2)->to_room, 2) ? "&y|" : " ") : " "),
(vai(ch, 2) ? (CAN_IR(EXIT(ch, 2)->to_room, 2) ? "&Yo" : " ") : " "));

send_to_char(buf3, ch);

}

void look_at_room(struct char_data * ch, int ignore_brief)
{
  if (!ch->desc)
    return;

  if (IS_DARK(ch->in_room) && !CAN_SEE_IN_DARK(ch)) {
    send_to_char("It is pitch black...\r\n", ch);
    return;
  } else if (AFF_FLAGGED(ch, AFF_BLIND)) {
    send_to_char("You see nothing but infinite darkness...\r\n", ch);
    return;
  }
	if (!IS_NPC(ch) && !PRF_FLAGGED(ch, PRF_HOLYLIGHT) && ROOM_AFFECTED(ch->in_room, RAFF_FOG)) {
		/* NOTE: you might wish to change so that wizards,
		 * or the use of some 'see through fog' makes you see
		 * through the fog
		 */
		send_to_char("Your view is obscured by a thick fog.\r\n", ch);
		return;
	}

  send_to_char(CCCYN(ch, C_NRM), ch);
  if (!IS_NPC(ch) && GET_LEVEL(ch) >= LVL_ROOMFLAGS) {
    sprintbit(ROOM_FLAGS(ch->in_room), room_bits, buf);
    sprintf(buf2, "[%5d] %s &G[ %s]&n", GET_ROOM_VNUM(IN_ROOM(ch)),
            world[ch->in_room].name, buf);
    send_to_char(buf2, ch);
  } else
    send_to_char(world[ch->in_room].name, ch);

  send_to_char(CCNRM(ch, C_NRM), ch);
  send_to_char("\r\n", ch);

  if ((!IS_NPC(ch) && !PRF_FLAGGED(ch, PRF_BRIEF)) || ignore_brief ||
      ROOM_FLAGGED(ch->in_room, ROOM_DEATH))
    send_to_char(world[ch->in_room].description, ch);

  if (ROOM_AFFECTED(ch->in_room, RAFF_FIREWALL))
    send_to_char("&RYou see a large &Yfirewall&R here.&n\r\n", ch);
  if (ROOM_AFFECTED(ch->in_room, RAFF_ICEWALL))
    send_to_char("&CYou see a &Bicewall&C here.&n\r\n", ch);
  if (ROOM_AFFECTED(ch->in_room, RAFF_ILLUSIONF))
    send_to_char("&gYou see many &Gtrees&g here.&n\r\n", ch);


  /* mini map */
  if (!IS_NPC(ch) && PRF2_FLAGGED(ch, PRF2_MAP))
    make_map(ch);

  /* autoexits */
  if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_AUTOEXIT))
    do_auto_exits(ch);

  /* now list characters & objects */
  send_to_char(CCGRN(ch, C_NRM), ch);
 /* if (ROOM_FLAGGED(ch->in_room, ROOM_HOUSE)) // Fenix - WHS
	  list_objs_in_house(ch);*/
  list_obj_to_char(world[ch->in_room].contents, ch, 0, FALSE);
  send_to_char(CCYEL(ch, C_NRM), ch);
  list_char_to_char(world[ch->in_room].people, ch);
  send_to_char(CCNRM(ch, C_NRM), ch);
}



void look_in_direction(struct char_data * ch, int dir)
{
  if (EXIT(ch, dir)) {
    if (EXIT(ch, dir)->general_description)
      send_to_char(EXIT(ch, dir)->general_description, ch);
    else
      send_to_char("You see nothing special.\r\n", ch);

    if (EXIT_FLAGGED(EXIT(ch, dir), EX_CLOSED) && EXIT(ch, dir)->keyword) {
      sprintf(buf, "The %s is closed.\r\n", fname(EXIT(ch, dir)->keyword));
      send_to_char(buf, ch);
    } else if (EXIT_FLAGGED(EXIT(ch, dir), EX_ISDOOR) && EXIT(ch, dir)->keyword) {
      sprintf(buf, "The %s is open.\r\n", fname(EXIT(ch, dir)->keyword));
      send_to_char(buf, ch);
    }
  } else
    send_to_char("Nothing special there...\r\n", ch);
}



void look_in_obj(struct char_data * ch, char *arg)
{
  struct obj_data *obj = NULL;
  struct char_data *dummy = NULL;
  int amt, bits;

  if (!*arg)
    send_to_char("Look in what?\r\n", ch);
  else if (!(bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM |
                                 FIND_OBJ_EQUIP, ch, &dummy, &obj))) {
    sprintf(buf, "There doesn't seem to be %s %s here.\r\n", AN(arg), arg);
    send_to_char(buf, ch);
  } else if ((GET_OBJ_TYPE(obj) != ITEM_DRINKCON) &&
             (GET_OBJ_TYPE(obj) != ITEM_FOUNTAIN) &&
             (GET_OBJ_TYPE(obj) != ITEM_CONTAINER))
    send_to_char("There's nothing inside that!\r\n", ch);
  else {
    if (GET_OBJ_TYPE(obj) == ITEM_CONTAINER) {
      if (OBJVAL_FLAGGED(obj, CONT_CLOSED))
        send_to_char("It is closed.\r\n", ch);
      else {
        send_to_char(fname(obj->name), ch);
        switch (bits) {
        case FIND_OBJ_INV:
          send_to_char(" (carried): \r\n", ch);
          break;
        case FIND_OBJ_ROOM:
          send_to_char(" (here): \r\n", ch);
          break;
        case FIND_OBJ_EQUIP:
          send_to_char(" (used): \r\n", ch);
          break;
        }

        list_obj_to_char(obj->contains, ch, 2, TRUE);
      }
    } else {            /* item must be a fountain or drink container */
      if (GET_OBJ_VAL(obj, 1) <= 0)
        send_to_char("It is empty.\r\n", ch);
      else {
        if (GET_OBJ_VAL(obj,0) <= 0 || GET_OBJ_VAL(obj,1)>GET_OBJ_VAL(obj,0)) {
          sprintf(buf, "Its contents seem somewhat murky.\r\n"); /* BUG */
        } else {
          amt = (GET_OBJ_VAL(obj, 1) * 3) / GET_OBJ_VAL(obj, 0);
          sprinttype(GET_OBJ_VAL(obj, 2), color_liquid, buf2);
          sprintf(buf, "It's %sfull of a %s liquid.\r\n", fullness[amt], buf2);
        }
        send_to_char(buf, ch);
      }
    }
  }
}



char *find_exdesc(char *word, struct extra_descr_data * list)
{
  struct extra_descr_data *i;

  for (i = list; i; i = i->next)
    if (isname(word, i->keyword))
      return (i->description);

  return (NULL);
}


/*
 * Given the argument "look at <target>", figure out what object or char
 * matches the target.  First, see if there is another char in the room
 * with the name.  Then check local objs for exdescs.
 */

/*
 * BUG BUG: If fed an argument like '2.bread', the extra description
 *          search will fail when it works on 'bread'!
 * -gg 6/24/98 (I'd do a fix, but it's late and non-critical.)
 */
void look_at_target(struct char_data * ch, char *arg)
{
  int bits, found = FALSE, j, fnum, i = 0;
  struct char_data *found_char = NULL;
  struct obj_data *obj, *found_obj = NULL;
  char *desc;

  if (!ch->desc)
    return;

  if (!*arg) {
    send_to_char("Look at what?\r\n", ch);
    return;
  }

  bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP |
                      FIND_CHAR_ROOM, ch, &found_char, &found_obj);

  /* Is the target a character? */
  if (found_char != NULL) {
    look_at_char(found_char, ch);
    if (ch != found_char) {
      if (CAN_SEE(found_char, ch))
        act("$n looks at you.", TRUE, ch, 0, found_char, TO_VICT);
      act("$n looks at $N.", TRUE, ch, 0, found_char, TO_NOTVICT);
    }
    return;
  }

  /* Strip off "number." from 2.foo and friends. */
  if (!(fnum = get_number(&arg))) {
    send_to_char("Look at what?\r\n", ch);
    return;
  }

  /* Does the argument match an extra desc in the room? */
  if ((desc = find_exdesc(arg, world[ch->in_room].ex_description)) != NULL && ++i == fnum) {
    page_string(ch->desc, desc, FALSE);
    return;
  }

  /* Does the argument match an extra desc in the char's equipment? */
  for (j = 0; j < NUM_WEARS && !found; j++)
    if (GET_EQ(ch, j) && CAN_SEE_OBJ(ch, GET_EQ(ch, j)))
      if ((desc = find_exdesc(arg, GET_EQ(ch, j)->ex_description)) != NULL && ++i == fnum) {
	send_to_char(desc, ch);
	found = TRUE;
      }

  /* Does the argument match an extra desc in the char's inventory? */
  for (obj = ch->carrying; obj && !found; obj = obj->next_content) {
    if (CAN_SEE_OBJ(ch, obj))
      if ((desc = find_exdesc(arg, obj->ex_description)) != NULL && ++i == fnum) {
	send_to_char(desc, ch);
	found = TRUE;
      }
  }

  /* Does the argument match an extra desc of an object in the room? */
  for (obj = world[ch->in_room].contents; obj && !found; obj = obj->next_content)
    if (CAN_SEE_OBJ(ch, obj))
      if ((desc = find_exdesc(arg, obj->ex_description)) != NULL && ++i == fnum) {
	send_to_char(desc, ch);
	found = TRUE;
      }

  /* If an object was found back in generic_find */
  if (bits) {
    if (!found)
      show_obj_to_char(found_obj, ch, 5);       /* Show no-description */
    else
      show_obj_to_char(found_obj, ch, 6);       /* Find hum, glow etc */
  } else if (!found)
    send_to_char("You do not see that here.\r\n", ch);
}


ACMD(do_look)
{
  char arg2[MAX_INPUT_LENGTH];
  int look_type;

  if (!ch->desc)
    return;

  if (GET_POS(ch) < POS_SLEEPING)
    send_to_char("You can't see anything but stars!\r\n", ch);
  else if (AFF_FLAGGED(ch, AFF_BLIND))
    send_to_char("You can't see a damned thing, you're blind!\r\n", ch);
  else if (IS_DARK(ch->in_room) && !CAN_SEE_IN_DARK(ch)) {
    send_to_char("It is pitch black...\r\n", ch);
    list_char_to_char(world[ch->in_room].people, ch);   /* glowing red eyes */
  } else {
    half_chop(argument, arg, arg2);

    if (subcmd == SCMD_READ) {
      if (!*arg)
        send_to_char("Read what?\r\n", ch);
      else
        look_at_target(ch, arg);
      return;
    }
    if (!*arg)                  /* "look" alone, without an argument at all */
      look_at_room(ch, 1);
    else if (is_abbrev(arg, "in"))
      look_in_obj(ch, arg2);
    /* did the char type 'look <direction>?' */
    else if ((look_type = search_block(arg, dirs, FALSE)) >= 0)
      look_in_direction(ch, look_type);
    else if (is_abbrev(arg, "at"))
      look_at_target(ch, arg2);
    else
      look_at_target(ch, arg);
  }
}



ACMD(do_examine)
{
  int bits;
  struct char_data *tmp_char = NULL;
  struct obj_data *tmp_object = NULL;

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Examine what?\r\n", ch);
    return;
  }

  look_at_target(ch, arg);

  bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_CHAR_ROOM |
                      FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

  if (tmp_object) {
    if ((GET_OBJ_TYPE(tmp_object) == ITEM_DRINKCON) ||
        (GET_OBJ_TYPE(tmp_object) == ITEM_FOUNTAIN) ||
        (GET_OBJ_TYPE(tmp_object) == ITEM_CONTAINER)) {
      send_to_char("When you look inside, you see:\r\n", ch);
      look_in_obj(ch, arg);
    }
  }
}



ACMD(do_gold)
{
  if (GET_GOLD(ch) == 0)
    send_to_char("You're broke!\r\n", ch);
  else if (GET_GOLD(ch) == 1)
    send_to_char("You have one miserable little gold coin.\r\n", ch);
  else {
    sprintf(buf, "You have %s gold coins and %d roleplay points.\r\n", add_points(GET_GOLD(ch)), GET_RP(ch));
    send_to_char(buf, ch);
  }
}
#define GET_NEWSAVE(ch,x) (saving_throws(GET_CLASS(ch),x,GET_LEVEL(ch)) + GET_SAVE(ch,x))
#define MOSTRA_THACO(ch) (thaco(GET_CLASS(ch), GET_LEVEL(ch)) - GET_HITROLL(ch)/2)

char *howstat(int percent)
{
  static char buf[256];

  if (percent == 0)
    strcpy(buf, "&nnothing&n");
  else if (percent <= 4)
    strcpy(buf, " &cwicked&n");
  else if (percent <= 6)
    strcpy(buf, "    &ybad&n");
  else if (percent <= 8)
    strcpy(buf, "   &gless&n");
  else if (percent <= 10)
    strcpy(buf, "   &Gfair&n");
  else if (percent <= 12)
    strcpy(buf, " &Ymedium&n");
  else if (percent <= 14)
    strcpy(buf, "&Caverage&n");
  else if (percent <= 16)
    strcpy(buf, "   &Bgood&n");
  else
    strcpy(buf, "&Rmaximum&n");

  return (buf);
}

char *complete_char(char *string, int num)
{
   int y;

   y = num;
   y -= strlen(string);
   y += (conta_cores(string)+conta_cores(string));

   if(y >= num)
     return (string);

   strcpy(buf3, string);
   for(; y > 0; y--)
     strcat(buf3, " ");

   return(buf3);
}

ACMD(do_score)
{
  	double max, norm;
  	char nome_title[1000];
	int ac = GET_AC(ch);

  if (IS_NPC(ch))
    return;

  if (FOREST(ch->in_room) && GET_SKILL(ch, SKILL_FOREST_AGILITY)) {
	ac -= GET_LEVEL(ch)/4;
  }

  max = CAN_CARRY_W(ch);
  norm = IS_CARRYING_W(ch);

  max /= 10;
  norm /= 10;
  sprintf(nome_title, "%s %s", GET_NAME(ch), GET_TITLE(ch));
  sprintf(buf, "\r\n       &w+----------------------------------------------------------------+\r\n");
  sprintf(buf + strlen(buf), "       &w|&BName/Title: &C%-s &C%3d &Byears old &w|\r\n", complete_char(reprimir(nome_title, 37+conta_cores(GET_TITLE(ch))+conta_cores(GET_TITLE(ch))), 37), GET_AGE(ch));
  sprintf(buf + strlen(buf), "       &w+=============+=======================+==========================+\r\n");
  sprintf(buf + strlen(buf), "       &w|&BArmor: &C%5d &w| &BRace : &C%14s &w|       &BPoints Scored      &w|\r\n", ac, pc_race_types[(int) GET_RACE(ch)]);
  sprintf(buf + strlen(buf), "       &w|&BLevel: &C%5d &w| &BClass: &C%14s &w| &mHoly Points :  &C%9s &w|\r\n", GET_LEVEL(ch), pc_class_types[(int) GET_CLASS(ch)], add_points(GET_POINTS(ch)));
  sprintf(buf + strlen(buf), "       &w|%s| &BSex  :     &C%10s &w| &mQuest Points:  &C%9s &w|\r\n", make_mbar(-1000, GET_ALIGNMENT(ch), 1000, 10, (clr(ch, C_CMP) ? TRUE : FALSE)), genders[(int) GET_SEX(ch)], add_points(GET_QP(ch)));
  sprintf(buf + strlen(buf), "       &w+=============+=======================+==========================+\r\n");
  sprintf(buf + strlen(buf), "       &w|&BStr: &R%7d &w|&bQuest    :         &C%3d &w|      &BGeneral Stats       &w|\r\n", GET_STR(ch), GET_QUEST(ch));
  sprintf(buf + strlen(buf), "       &w|&BInt: &B%7d &w|&bRemorts  :         &C%3d &w| &bHit Points  &C%s &w|\r\n", GET_INT(ch), GET_REMORT(ch), make_bar(GET_HIT(ch), GET_MAX_HIT(ch), 10, (clr(ch, C_CMP) ? TRUE : FALSE)));
  sprintf(buf + strlen(buf), "       &w|&BWis: &C%7d &w|&bLucky    :     %s &w| &bMana Points &C%s &w|\r\n", GET_WIS(ch), howstat(GET_LUK(ch)), make_bar(GET_MANA(ch), GET_MAX_MANA(ch), 10, (clr(ch, C_CMP) ? TRUE : FALSE)));
  sprintf(buf + strlen(buf), "       &w|&BCha: &Y%7d &w+-----------------------+ &bMove Points &C%s &w|\r\n", GET_CHA(ch), make_bar(GET_MOVE(ch), GET_MAX_MOVE(ch), 10, (clr(ch, C_CMP) ? TRUE : FALSE)));
  sprintf(buf + strlen(buf), "       &w|&BDex: &g%7d &w|&BGold : &C%15s &w| &bMental      &C%s &w|\r\n", GET_DEX(ch), add_points(GET_GOLD(ch)), make_bar(GET_MENTAL(ch), GET_MAX_MENTAL(ch), 10, (clr(ch, C_CMP) ? TRUE : FALSE)));
  sprintf(buf + strlen(buf), "       &w|&BCon: &y%7d &w|&BBank : &C%15s &w| &bBreath      &C%s &w|\r\n", GET_CON(ch), add_points(GET_BANK_GOLD(ch)), make_bar(GET_OXI(ch), GET_MAX_OXI(ch), 10, (clr(ch, C_CMP) ? TRUE : FALSE)));
  sprintf(buf + strlen(buf), "       &w+=============+=======================+==========================+\r\n");
  sprintf(buf + strlen(buf), "       &w|             &BPontuation              &w|  &BPK &BInfo                 &w|\r\n");
  sprintf(buf + strlen(buf), "       &w|&bCurrent exp   :      &C%15s &w|&bKilled:&C%17d  &w|\r\n", add_points(GET_EXP(ch)), GET_KILLED(ch));
  sprintf(buf + strlen(buf), "       &w|&bExp to level  :      &C%15s &w|&bDied  :&C%17d  &w|\r\n", (GET_LEVEL(ch) >= LVL_IMMORT ? "0" : add_points(level_exp(GET_REMORT(ch), GET_LEVEL(ch)+1))), GET_DIED(ch));
  sprintf(buf + strlen(buf), "       &w+=====================================+==========================+\r\n");
  sprintf(buf + strlen(buf), "       &w|&bInventory     :                  &C%3d &w|          &BToggle          &w|\r\n", IS_CARRYING_N(ch));
  sprintf(buf + strlen(buf), "       &w|&bWeight Carried:         %s&C%4.1f&c/&C%4.1f&ckg &w|&bAutoassist    &C%s&w|\r\n", (CAN_CARRY_W(ch) > 999 ? "" : " "), norm, max, (PRF_FLAGGED(ch, PRF_AUTOASSIST) ? "(&c*&C)ON ( )OFF" : "( )ON (&c*&C)OFF"));
  sprintf(buf + strlen(buf), "       &w|&bHitroll       :                 &C%4d &w|&bAutoloot      &C%s&w|\r\n", GET_HITROLL(ch), (PRF_FLAGGED(ch, PRF_AUTOLOOT) ? "(&c*&C)ON ( )OFF" : "( )ON (&c*&C)OFF"));
  sprintf(buf + strlen(buf), "       &w|&bDamroll       :                 &C%4d &w|&bAutoexit      &C%s&w|\r\n", GET_DAMROLL(ch), (PRF_FLAGGED(ch, PRF_AUTOEXIT) ? "(&c*&C)ON ( )OFF" : "( )ON (&c*&C)OFF"));
  sprintf(buf + strlen(buf), "       &w|&bThaco         :                 &C%4d &w|&bAutodamage    &C%s&w|\r\n", MOSTRA_THACO(ch), (PRF_FLAGGED(ch, PRF_AUTOEXIT) ? "(&c*&C)ON ( )OFF" : "( )ON (&c*&C)OFF"));
  sprintf(buf + strlen(buf), "       &w|&bSaving throws : &C%3d&c/&C%3d&c/&C%3d&c/&C%3d&c/&C%3d&n  &w|                          |&n\r\n", GET_NEWSAVE(ch, 0),GET_NEWSAVE(ch, 1), GET_NEWSAVE(ch, 2), GET_NEWSAVE(ch, 3), GET_NEWSAVE(ch, 4));
  sprintf(buf + strlen(buf), "       &w+-------------------------------------+--------------------------+\r\n");

  send_to_char(buf, ch);
}

ACMD(do_summary)
{
    if(IS_NPC(ch)){
       send_to_char("You can't do this...", ch);
       return;
    }

    make_summary(ch);
}

ACMD(do_affections)
{
  struct affected_type *aff;

  if(IS_NPC(ch))
    return;

  if (ch->affected) {
    send_to_char("&WYou feel that you are affected by:&n\r\n", ch);
     for (aff = ch->affected; aff; aff = aff->next) {
       *buf2 = '\0';
		if(aff->duration == -1)
         sprintf(buf, "&RAFF&W: &n(innate) %s%-21s%s ", CCCYN(ch, C_NRM), spells[aff->type], CCNRM(ch, C_NRM));
        else
         sprintf(buf, "&RAFF&W: &n(%4dhr) %s%-21s%s ", (aff->duration > 24 ? 24 : aff->duration + 1),
               CCCYN(ch, C_NRM), spells[aff->type], CCNRM(ch, C_NRM));

       if (aff->modifier) {
         sprintf(buf2, "%+d to %s", aff->modifier, apply_types[(int) aff->location]);
         strcat(buf, buf2);
       }
       send_to_char(strcat(buf, "\r\n"), ch);
     }
   } else {
    send_to_char("&WYou feel yourself normal.&n\r\n", ch);
    return;
  }
}

ACMD(do_inventory)
{
	double i, j;
	
	i = CAN_CARRY_W(ch);
 	j = IS_CARRYING_W(ch);
  	
 // send_to_char("&YYou are carrying:&n\r\n", ch);
  sprintf(buf, "&WYou are carrying: &c%4.1f&w/&C%4.1f&wkg&n\r\n", j/10,i/10);
  send_to_char(buf, ch);
 // sprintf(buf + strlen(buf), "&yWeight Carried:%s&C%4.1f&c/&C%4.1f&ckg&n\r\n", (CAN_CARRY_W(ch) > 999 ? "" : " "), norm, max);
 // send_to_char(buf, ch);
  list_obj_to_char(ch->carrying, ch, 1, TRUE);
}


ACMD(do_equipment)
{
  int i, num = 0;

  send_to_char("&cYou are using:&n\r\n", ch);

  for (i = 0; i < NUM_WEARS; i++) {
	if (GET_EQ(ch, wear_order_index[i])) {
		num++;
        	if (CAN_SEE_OBJ(ch, GET_EQ(ch, wear_order_index[i]))) {
        		send_to_char(where[wear_order_index[i]], ch);
        		send_to_char("&y", ch);
        		show_obj_to_char(GET_EQ(ch, wear_order_index[i]), ch, 1);
        		send_to_char("&n", ch);
       		} else {
			send_to_char(where[wear_order_index[i]], ch);
        		send_to_char("Something.\r\n", ch);
      		}
    	} else {
			send_to_char(where[wear_order_index[i]], ch);
			send_to_char("&n\n", ch);
	}
  }
}


ACMD(do_time)
{
  char suf[10];
  int weekday, day;

  sprintf(buf, "\r\n&cIt's &w%d &co'clock&n %s&c, on &w",
          ((time_info.hours % 12 == 0) ? 12 : ((time_info.hours) % 12)),
          ((time_info.hours >= 12) ? "&wpm&n" : "&wam&n"));

  day = time_info.day + 1;      // day in [1..35]

  weekday = ((35 * time_info.month) + day) % 7;// 35 days in a month, 7 days a week


  strcat(buf, weekdays[weekday]);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);


  if (day == 1)
    strcpy(suf,"&Wst&n");
  else if (day == 2)
    strcpy(suf,"&Wnd&n");
  else if (day == 3)
    strcpy(suf,"&Wrd&n");
  else if (day < 20)
    strcpy(suf,"&Wth&n");
  else if ((day % 10) == 1)
    strcpy(suf,"&Wst&n");
  else if ((day % 10) == 2)
    strcpy(suf,"&Wnd&n");
  else if ((day % 10) == 3)
    strcpy(suf,"&Wrd&n");
  else
    strcpy(suf,"&Wth&n");

  sprintf(buf, "&cThe &w%d%s &cDay of the &w%s&c, Year &w%d&n.\r\n",
          day, suf, month_name[(int) time_info.month], time_info.year);

  send_to_char(buf, ch);
}

ACMD(do_weather)
{
  const char *sky_look[] = {
    "cloudless",
    "cloudy",
    "rainy",
    "lit by flashes of lightning"
  };

  if (OUTSIDE(ch)) {
    sprintf(buf, "The sky is %s and %s.\r\n", sky_look[weather_info.sky],
            (weather_info.change >= 0 ? "you feel a warm wind from south" :
             "your foot tells you bad weather is due"));
    send_to_char(buf, ch);
  } else
    send_to_char("You have no feeling about the weather at all.\r\n", ch);
}

struct help_index_element *find_help(char *keyword)
{
  extern int top_of_helpt;
  int i;

  for (i = 0; i < top_of_helpt; i++)
    if (isname(keyword, help_table[i].keywords))
      return (help_table + i);

  return NULL;
}

struct help_index_element *find_exact_help(char *keyword)
{
  extern int top_of_helpt;
  int i;

  for (i = 0; i < top_of_helpt; i++)
    if (exact_isname(keyword, help_table[i].keywords))
      return (help_table + i);

  return NULL;
}

int contar_helps(char *palavra, struct char_data *ch)
{
  extern int top_of_helpt;
  int i, x = 0;

  for (i = 0; i < top_of_helpt; i++) {
    if (is_name(palavra, help_table[i].keywords))
     if (help_table[i].min_level <= GET_LEVEL(ch))
       x++;
  }
  return (x);
}

ACMD(do_help)
{
  extern char *help;
  struct help_index_element *this_help;
  char entry[MAX_STRING_LENGTH];
  extern int top_of_helpt;
  int i, x;

  if (!ch->desc)
    return;

  skip_spaces(&argument);

  if (!*argument) {
    send_to_char(help, ch);
    return;
  }
  if (!help_table) {
    send_to_char("No help available.\r\n", ch);
    return;
  }

  if (!(this_help = find_exact_help(argument)))
  {
    if((x=contar_helps(argument, ch)) > 1){
	  sprintf(buf, "  &C%d &ctopics related:\r\n\r\n", x);
	  for (i = 0; i < top_of_helpt; i++) {
	    if (is_name(argument, help_table[i].keywords))
	      if (help_table[i].min_level <= GET_LEVEL(ch))
		sprintf(buf+strlen(buf), "      &y%s&n\r\n", help_table[i].keywords);
	  }
	  send_to_char(buf, ch);
	  return;
    }

    if (!(this_help = find_help(argument))) {
       send_to_char("There is no help on that word.\r\n", ch);
       sprintf(buf, "HELP: %s tried to get help on %s", GET_NAME(ch), argument);
       log(buf);
       return;
    }
  }

  if (this_help->min_level > GET_LEVEL(ch)) {
     send_to_char("There is no help on that word.\r\n", ch);
     return;
  }
  sprintf(entry, "&y%s&n\r\n%s", this_help->keywords, this_help->entry);
  send_to_char("\r\n\r\n", ch);
  page_string(ch->desc, entry, 0);
//  send_to_char(entry, ch);
}


#define WHO_USAGE \
"Format: who [minlev[-maxlev]] [-n name] [-c classlist] [-o] [-q] [-r] [-z]\r\n"
/*
ACMD(do_who)
{
  struct descriptor_data *d;
  struct char_data *wch;
  char godbuf[MAX_STRING_LENGTH];
  char mortbuf[MAX_STRING_LENGTH];
  char name_search[MAX_NAME_LENGTH+1];
  char mode;

  int low = 0, high = LVL_IMPL, showclass = 0;//, boot_high = 0;
  bool who_room = FALSE, who_zone = FALSE, who_quest = 0;
  bool outlaws = FALSE, noimm = FALSE, nomort = FALSE;

  int gods = 0, morts = 0;
  size_t i;

  skip_spaces(&argument);
  strcpy(buf, argument);
  name_search[0] = '\0';

  while (*buf) {
    half_chop(buf, arg, buf1);
    if (isdigit(*arg)) {
      sscanf(arg, "%d-%d", &low, &high);
      strcpy(buf, buf1);
    } else if (*arg == '-') {
      mode = *(arg + 1);
      switch (mode) {
      case 'o':
	outlaws = TRUE;
	strcpy(buf, buf1);
	break;
      case 'z':
	who_zone = TRUE;
	strcpy(buf, buf1);
	break;
      case 'q':
	who_quest = TRUE;
	strcpy(buf, buf1);
	break;
      case 'l':
	half_chop(buf1, arg, buf);
	sscanf(arg, "%d-%d", &low, &high);
	break;
      case 'n':
	half_chop(buf1, name_search, buf);
	break;
      case 'r':
	who_room = TRUE;
	strcpy(buf, buf1);
	break;
      case 'c':
	half_chop(buf1, arg, buf);
	for (i = 0; i < strlen(arg); i++)
	  showclass |= find_class_bitvector(arg[i]);
	break;
      case 'i':
        nomort = TRUE;
        strcpy(buf, buf1);
        break;
      case 'm':
        noimm = TRUE;
        strcpy(buf, buf1);
        break;
      default:
	send_to_char(WHO_FORMAT, ch);
	return;
	break;
      }

    } else {
      send_to_char(WHO_FORMAT, ch);
      return;
    }
  }

  sprintf(godbuf,  "		 &b-------------------------------------------------&n \r\n"
  		   "		&b|   	           &CMasters&c of &CWar&n                 &b|&n\r\n"
  		   "		&b|   	                                          |&n\r\n");

  sprintf(mortbuf, "		 &b-------------------------------------------------&n \r\n"
  		   "		&b|   	        &CGladiators&c on &CWarDome&n             &b|&n\r\n"
  		   "		&b|   	                                          |&n\r\n");

  for (d = descriptor_list; d; d = d->next) {
    if (d->connected)
      continue;

    if (d->original)
      wch = d->original;
    else if (!(wch = d->character))
      continue;

    if (!CAN_SEE(ch, wch))
      continue;
    if (GET_LEVEL(wch) < low || GET_LEVEL(wch) > high)
      continue;
    if ((noimm && GET_LEVEL(wch) >= LVL_IMMORT) || (nomort && GET_LEVEL(wch) < LVL_IMMORT))
      continue;
    if (*name_search && str_cmp(GET_NAME(wch), name_search) && !strstr(GET_TITLE(wch), name_search))
      continue;
    if (outlaws && !PLR_FLAGGED(wch, PLR_KILLER) && !PLR_FLAGGED(wch, PLR_THIEF))
      continue;
    if (who_quest && !PRF_FLAGGED(wch, PRF_QUEST))
      continue;
    if (who_zone && world[ch->in_room].zone != world[wch->in_room].zone)
      continue;
    if (who_room && (wch->in_room != ch->in_room))
      continue;
    if (showclass && !(showclass & (1 << GET_CLASS(wch))))
      continue;

    if (GET_LEVEL(wch) >= LVL_IMMORT) {
	 gods++;
         sprintf(godbuf,  "%s		&b|&Y%s&C{&c%2s&C}&n &B%-12.12s&n  %4s %3s %5s %8s %3s  &b|&n", godbuf, (GET_OLC_ZONE(wch) != -1 ? "&Y*&n" : " "),
         GET_GOD_WIZNAME(wch), GET_NAME(wch), (GET_CLAN_RANK(wch) > 0 ? "&GClan&n" : "    "), (GET_INVIS_LEV(wch) > 0 ? "&mInv&n" : "   "),
         (IS_AFFECTED(wch, AFF_INVISIBLE) ? "&MInvis&n" : "     "), (PLR_FLAGGED(wch, PLR_KILLER) ? "&RAssassin&n" : "        "),
         (PRF_FLAGGED(wch, PRF_AFK) ? "&gAfk&n" : "   "));
    } else {
         morts++;
         sprintf(mortbuf, "%s		&b| &C[&c%2d&C]&n &B%-12.12s&n  %4s %3s %5s %8s %3s  &b|&n", mortbuf,
         GET_LEVEL(wch), GET_NAME(wch), (GET_CLAN_RANK(wch) > 0 ? "&GClan&n" : "    "), (GET_INVIS_LEV(wch) > 0 ? "&mInv&n" : "   "),
         (IS_AFFECTED(wch, AFF_INVISIBLE) ? "&MInvis&n" : "     "), (PLR_FLAGGED(wch, PLR_KILLER) ? "&RAssassin&n" : "        "),
         (PRF_FLAGGED(wch, PRF_AFK) ? "&gAfk&n" : "   "));

    }

    sprintf(buf, "%s\r\n", CCNRM(ch, C_NRM));

    if (GET_LEVEL(wch) >= LVL_IMMORT)
      strcat(godbuf, buf);
    else
      strcat(mortbuf, buf);

  }

  send_to_char("\r\n", ch);

  if (gods > 0)
    send_to_char(godbuf, ch);

  if (morts > 0)
    send_to_char(mortbuf, ch);

  sprintf(buf,  "		 &b-------------------------------------------------&n\r\n"
  		"\r\n"
  		"		    &bWe have &B%d&b Gladiator%s and &B%d&b God%s of War &BOnline&b.&n\r\n", morts, (morts > 1 ? "s" : ""), gods, (gods > 1 ? "s" : ""));
  send_to_char(buf, ch);
}
*/

struct who_list {
  char *name;
  int level;
  struct who_list *next;
};

struct who_list *add_to_who(struct who_list *head, char *str,
        struct char_data *ch)
{
  struct who_list *tmp, *prev = NULL, *to_add = NULL;

  if (str && ch) {
    CREATE(to_add, struct who_list, 1);
    to_add->name = str_dup(str);
    to_add->level = GET_LEVEL(ch);
    to_add->next = NULL;
  } else {
    log("SYSERR: NULL str or ch in add_to_who");
    return NULL;
  }

  if (!head)
    return to_add;

  for (tmp = head; tmp; tmp = tmp->next) {
    if (to_add->level > tmp->level) {   /* ok, add record */
      if (prev)
        prev->next = to_add;
      if (head == tmp)
        head = to_add;
      to_add->next = tmp;
      return head;
    }
    prev = tmp;
  }
  prev->next = to_add;
  return head;
}

void output_who(struct who_list *head, struct char_data *ch)
{
  struct who_list *tmp, *next;

  if (!head) {
    log("SYSERR: output_who: hey, no head?");
    return;
  }

  for (tmp = head; tmp; tmp = next) {
    next = tmp->next;
    send_to_char(tmp->name, ch);

    if (!tmp || !tmp->name)
      log("SYSERR: output_who: trying to free invalid tmp->name");
    else {
      free(tmp->name);
    }
    if (!tmp)
      log("SYSERR: output_who: trying to free invalid tmp struct");
    else {
      free(tmp);
    }
  }
}
/*
ACMD(do_who)
{
  struct descriptor_data *d;
  struct char_data *tch;
  struct who_list *who_head = NULL;
  char name_search[MAX_INPUT_LENGTH];
  char mode;
  size_t i;
  int low = 0, high = LVL_IMPL, localwho = 0, questwho = 0;
  int showclass = 0, outlaws = 0, imm = 0, mort = 0;
  int who_room = 0, clan_num = 0;

  skip_spaces(&argument);
  strcpy(buf, argument);
  name_search[0] = '\0';

  while (*buf) {
    half_chop(buf, arg, buf1);
    if (isdigit(*arg)) {
      sscanf(arg, "%d-%d", &low, &high);
      strcpy(buf, buf1);
    } else if (*arg == '-') {
      mode = *(arg + 1);
      switch (mode) {
      case 'o':
      case 'k':
	outlaws = 1;
	strcpy(buf, buf1);
	break;
      case 'z':
	localwho = 1;
	strcpy(buf, buf1);
	break;
      case 'q':
	questwho = 1;
	strcpy(buf, buf1);
	break;
      case 'l':
	half_chop(buf1, arg, buf);
	sscanf(arg, "%d-%d", &low, &high);
	break;
      case 'n':
	half_chop(buf1, name_search, buf);
	break;
      case 'r':
	who_room = 1;
	strcpy(buf, buf1);
	break;
      case 'c':
	half_chop(buf1, arg, buf);
	for (i = 0; i < strlen(arg); i++)
	  showclass |= find_class_bitvector(arg[i]);
	break;
      default:
	send_to_char(WHO_USAGE, ch);
	return;
      }

    } else {
      send_to_char(WHO_USAGE, ch);
      return;
    }
  }

  send_to_char("\r\n&WWardome &wOnline&W Players&n\r\n\r\n", ch);

  for (d = descriptor_list; d; d = d->next) {
    if (STATE(d) != CON_PLAYING && !(d->connected == CON_OEDIT || d->connected == CON_REDIT  ||
        d->connected == CON_ZEDIT || d->connected == CON_MEDIT || d->connected == CON_SEDIT ||
        d->connected == CON_TEXTED || d->connected == CON_QEDIT || d->connected == CON_TRIGEDIT ||
        d->connected == CON_AEDIT || d->connected == CON_HEDIT))
      continue;

    if (d->original)
      tch = d->original;
    else if (!(tch = d->character))
      continue;

    if (*name_search && str_cmp(GET_NAME(tch), name_search) &&
	!strstr(GET_TITLE(tch), name_search))
      continue;
    if (!CAN_SEE(ch, tch) || GET_LEVEL(tch) < low || GET_LEVEL(tch) > high)
      continue;
    if (outlaws && !PLR_FLAGGED(tch, PLR_KILLER) &&
	!PLR_FLAGGED(tch, PLR_THIEF))
      continue;
    if (questwho && !PRF_FLAGGED(tch, PRF_QUEST))
      continue;
    if (localwho && world[ch->in_room].zone != world[tch->in_room].zone)
      continue;
    if (who_room && (tch->in_room != ch->in_room))
      continue;
    if (showclass && !(showclass & (1 << GET_CLASS(tch))))
      continue;

    if(GET_LEVEL(tch) >= LVL_IMMORT){
        sprintf(buf, "&W[&y%s&W]&n %s&c%s&n%s",
	      GET_GOD_WIZNAME(tch), (GET_PRENAME(tch) != NULL ? GET_PRENAME(tch) : ""),
	      GET_NAME(tch), GET_TITLE(tch));
        imm++;
    } else {
       if(GET_REMORT(tch) > 0 && GET_REMORT(tch) <= 12) {
        sprintf(buf, "&W[&n&y%s&W]&n %s&c%s&n%s",
	      GET_REMORT_NAME(tch),
	      (GET_PRENAME(tch) != NULL ? GET_PRENAME(tch) : ""),
	      GET_NAME(tch), GET_TITLE(tch));
       } else {
        sprintf(buf, "&W[&n %2d %s &W]&n %s&c%s&n%s",
	      GET_LEVEL(tch), CLASS_ABBR(tch),
	      (GET_PRENAME(tch) != NULL ? GET_PRENAME(tch) : ""),
	      GET_NAME(tch), GET_TITLE(tch));
//       }
        mort++;
    }

      if (GET_LEVEL(tch) >= LVL_ELDER)
        strcat(buf, " &G(&YSTAFF&G)&n");

      if (!PLR_FLAGGED(tch, PLR_NOPK) && GET_LEVEL(tch) < LVL_ELDER)
	strcat(buf, " &B(&RPK&B)&n");

      if ((clan_num=find_clan_by_id(GET_CLAN(tch)))>=0&&clan_num<num_of_clans) {
        if(GET_CLAN_RANK(tch)>0)
          sprintf(buf + strlen(buf), " &w[&n%s&n of &W%s&w]&n",
		clan[clan_num].rank_name[GET_CLAN_RANK(tch) -1],
                clan[clan_num].name);
      }

      if (GET_INVIS_LEV(tch))
	sprintf(buf + strlen(buf), " &n(i%d)", GET_INVIS_LEV(tch));
      else if (AFF_FLAGGED(tch, AFF_INVISIBLE))
	strcat(buf, " &n(invis)");

      if (d->connected == CON_OEDIT || d->connected == CON_REDIT
        || d->connected == CON_ZEDIT || d->connected == CON_MEDIT
        || d->connected == CON_SEDIT || d->connected == CON_QEDIT
        || d->connected == CON_HEDIT || d->connected == CON_AEDIT
        || d->connected == CON_TEXTED || d->connected == CON_TRIGEDIT)
        strcat(buf, " &g(&GOLC&g)&n");

      if (tch->char_specials.timer > 5)
        sprintf(buf + strlen(buf), " &R(idle: %d)&n", tch->char_specials.timer);

      if (PLR_FLAGGED(tch, PLR_MAILING))
	strcat(buf, " &n(mailing)");
      else if (PLR_FLAGGED(tch, PLR_WRITING))
	strcat(buf, " &n(writing)");

      if (PRF_FLAGGED(tch, PRF_DEAF))
	strcat(buf, " &n(deaf)");
      if (PRF_FLAGGED(tch, PRF_NOTELL))
	strcat(buf, " &W(notell)&n");
      if (PRF_FLAGGED(tch, PRF_QUEST))
	strcat(buf, " &C(quest)&n");
      if (PLR_FLAGGED(tch, PLR_THIEF))
	strcat(buf, " &n(THIEF)");
      if (PLR_FLAGGED(tch, PLR_KILLER))
	strcat(buf, " &n(KILLER)");
      if (PRF_FLAGGED(tch, PRF_AFK))
	strcat(buf, " &R(AFK)&n");

      if (world[tch->in_room].number <= 0)
        strcat(buf, " &W(&wvoided&W)&n");

      strcat(buf, CCNRM(ch, C_SPR));
      strcat(buf, "\r\n");

	who_head = add_to_who(who_head, buf, tch);
//      send_to_char(buf, ch);

  }
  if (who_head)
    output_who(who_head, ch);

  sprintf(buf1, "\r\n&WWe have &w%d&W Gladiator%s and &w%d&W God%s of War Online.&n\r\n", mort, (mort != 1 ? "s" : ""), imm, (imm != 1 ? "s" : ""));
  send_to_char(buf1, ch);

}
*/

ACMD(do_who)
{
  struct descriptor_data *d;
  struct char_data *tch;
//  struct who_list *who_head = NULL;
  char name_search[MAX_INPUT_LENGTH];
  char mess_buf[MAX_STRING_LENGTH];
  char mode;
  size_t i;
  int low = 0, high = LVL_IMPL, localwho = 0, questwho = 0;
  int showclass = 0, outlaws = 0, imm = 0, mort = 0;
  int who_room = 0, clan_num = 0, is_pk = 0;
  int l;

  skip_spaces(&argument);
  strcpy(buf, argument);
  name_search[0] = '\0';
  *mess_buf = '\0';

  while (*buf) {
    half_chop(buf, arg, buf1);
    if (isdigit(*arg)) {
      sscanf(arg, "%d-%d", &low, &high);
      strcpy(buf, buf1);
    } else if (*arg == '-') {
      mode = *(arg + 1);       /* just in case; we destroy arg in the switch */
      switch (mode) {
      case 'o':
      case 'k':
	outlaws = 1;
	strcpy(buf, buf1);
	break;
      case 'z':
	localwho = 1;
	strcpy(buf, buf1);
	break;
      case 'q':
	questwho = 1;
	strcpy(buf, buf1);
	break;
      case 'l':
	half_chop(buf1, arg, buf);
	sscanf(arg, "%d-%d", &low, &high);
	break;
      case 'n':
	half_chop(buf1, name_search, buf);
	break;
      case 'r':
	who_room = 1;
	strcpy(buf, buf1);
	break;
      case 'c':
	half_chop(buf1, arg, buf);
	for (i = 0; i < strlen(arg); i++)
	  showclass |= find_class_bitvector(arg[i]);
	break;
      default:
	send_to_char(WHO_USAGE, ch);
	return;
      }				/* end of switch */

    } else {			/* endif */
      send_to_char(WHO_USAGE, ch);
      return;
    }
  }				/* end while (parser) */

  send_to_char("\r\n&WWardome &wOnline&W Players&n\r\n\r\n", ch);

for(l = LVL_IMPL+1; l >= 0; l--){
  for (d = descriptor_list; d; d = d->next) {
    if (STATE(d) != CON_PLAYING && !(d->connected == CON_OEDIT || d->connected == CON_REDIT  ||
        d->connected == CON_ZEDIT || d->connected == CON_MEDIT || d->connected == CON_SEDIT ||
        d->connected == CON_TEXTED || d->connected == CON_QEDIT || d->connected == CON_TRIGEDIT ||
        d->connected == CON_AEDIT || d->connected == CON_HEDIT))
      continue;

    if (d->original)
      tch = d->original;
    else if (!(tch = d->character))
      continue;

    if(l != GET_LEVEL(tch))
      continue;
    if (*name_search && str_cmp(GET_NAME(tch), name_search) &&
	!strstr(GET_TITLE(tch), name_search))
      continue;
    if (!CAN_SEE(ch, tch) || GET_LEVEL(tch) < low || GET_LEVEL(tch) > high)
      continue;
    if (outlaws && !PLR_FLAGGED(tch, PLR_KILLER) &&
	!PLR_FLAGGED(tch, PLR_THIEF))
      continue;
    if (questwho && !PRF_FLAGGED(tch, PRF_QUEST))
      continue;
    if (localwho && world[ch->in_room].zone != world[tch->in_room].zone)
      continue;
    if (who_room && (tch->in_room != ch->in_room))
      continue;
    if (showclass && !(showclass & (1 << GET_CLASS(tch))))
      continue;
    if (is_pk && !PLR_FLAGGED(tch, PLR_NOPK) && GET_LEVEL(tch) < LVL_ELDER)
      continue;

        if(GET_LEVEL(tch) >= LVL_IMMORT){
        sprintf(buf, "&W[&y%s&W]&n %s&C%s &n%s",
	      GET_GOD_WIZNAME(tch), (GET_PRENAME(tch) != NULL ? GET_PRENAME(tch) : ""),
	      GET_NAME(tch), GET_TITLE(tch));
        imm++;
    } else {
       if(PLR_FLAGGED(tch, PLR_KILLER)) {
       	sprintf(buf, "&W[&r&RKILLER&r&W%s]&n %s&c%s &n%s",
       	     (can_pk(ch, tch) && ch != tch ? "&R*&W" : " "),
       	     (GET_PRENAME(tch) !=NULL ? GET_PRENAME(tch) : ""),
       	     GET_NAME(tch), GET_TITLE(tch));
     }  else if(!AFF2_FLAGGED(tch, AFF2_TRANSFORM)) {
         sprintf(buf, "&W[&n%3d &Y%s&W%s]&n %s&c%s &n%s",
 	      GET_LEVEL(tch), CLASS_ABBR(tch),
 	      (can_pk(ch, tch) && ch != tch ? "&R*&W" : " "),
	      (GET_PRENAME(tch) != NULL ? GET_PRENAME(tch) : ""),
	      GET_NAME(tch), GET_TITLE(tch));

       } else  {
         sprintf(buf, "&W[&n%s&W]&n %s &Y%s &n%s",
 	      (IS_EVIL(tch) ? " &WDemon&n " : (IS_GOOD(tch) ? " &CLight&n " : "&nNeutral&n")),
	      cnome_trans[GET_TRANS(tch)],
	      GET_NAME(tch), GET_TITLE(tch));
       }
      mort++;
    }


	if(GET_CLAN(tch) && GET_CLAN_POS(tch))
	{
		clan_num = posicao_clan(GET_CLAN(tch));
		sprintf(buf + strlen(buf), " &W[&n%s&W:&n %s&W]&n",
			clans[clan_num].nome_clan, clans[clan_num].pos_nome[GET_CLAN_POS(tch)-1]);
	}


      if (GET_INVIS_LEV(tch))
	sprintf(buf + strlen(buf), " &n(i%d)", GET_INVIS_LEV(tch));
      else if (AFF_FLAGGED(tch, AFF_INVISIBLE))
	strcat(buf, " &n(invis)");

      if (d->connected == CON_OEDIT || d->connected == CON_REDIT
        || d->connected == CON_ZEDIT || d->connected == CON_MEDIT
        || d->connected == CON_SEDIT || d->connected == CON_QEDIT
        || d->connected == CON_HEDIT || d->connected == CON_AEDIT
        || d->connected == CON_TEXTED || d->connected == CON_TRIGEDIT)
        strcat(buf, " &g(&GOLC&g)&n");

      if (tch->char_specials.timer > 5)
        sprintf(buf + strlen(buf), " &R(idle: %d)&n", tch->char_specials.timer);
      if (PLR_FLAGGED(tch, PLR_MAILING))
	strcat(buf, " &n(mailing)");
      else if (PLR_FLAGGED(tch, PLR_WRITING))
	strcat(buf, " &n(writing)");
	if (PLR_FLAGGED(tch, PLR_SECPLAYER))
	strcat(buf, " &w(&GS&w)&n");
      if (PRF_FLAGGED(tch, PRF_DEAF))
	strcat(buf, " &n(deaf)");
      if (PRF_FLAGGED(tch, PRF_NOTELL))
	strcat(buf, " &W(notell)&n");
      if (PRF_FLAGGED(tch, PRF_QUEST))
	strcat(buf, " &C(quest)&n");
      if (PLR_FLAGGED(tch, PLR_THIEF))
	strcat(buf, " &n(THIEF)");
      if (PLR_FLAGGED(tch, PLR_BANNED))
	strcat(buf, " &n(Banned)");
      if (PRF_FLAGGED(tch, PRF_AFK))
	strcat(buf, " &R(AFK)&n");
      if (ROOM_FLAGGED(IN_ROOM(tch), ROOM_ARENA))
        strcat(buf, " &g(GLADIATOR)&n");
      if ((!PLR_FLAGGED(tch, PLR_NOPK)))
		  strcat(buf, " &W(&RPK&W)&n");

      if (world[tch->in_room].number <= 0)
        strcat(buf, " &W(&wvoided&W)&n");

      strcat(buf, CCNRM(ch, C_SPR));
      strcat(buf, "\r\n");

      send_to_char(buf, ch);

  }				/* end of for */
}
  sprintf(buf1, "\r\n&WWe have &w%d&W Gladiator%s and &w%d&W God%s of War Online.&n\r\n", mort, (mort != 1 ? "s" : ""), imm, (imm != 1 ? "s" : ""));
  send_to_char(buf1, ch);

  if((imm + mort) > total_p)
    total_p = (imm + mort);

  if(GET_LEVEL(ch) >= LVL_ELDER) {
    sprintf(buf1, "&WMax players on this boot: &R%d&n\r\n", total_p);
    send_to_char(buf1, ch);
  }
 if (*mess_info.message && *mess_info.writer && mess_info.time) {
    sprintf(mess_buf, "\r\n%s&cMessage from &C%s, &cwritten on &C%-20s&n",
            mess_info.message, mess_info.writer, ctime(&mess_info.time));
    send_to_char(mess_buf, ch);
  }
}

int count_ips(char *ip)
{
	struct descriptor_data *d;
	short int count = 0;

	for (d = descriptor_list; d; d = d->next)
		if(!strcmp(d->host, ip))
			count++;
	return (count);
}

#define USERS_FORMAT \
"Format: users [-l minlevel[-maxlevel]] [-n name] [-h host] [-c classlist] [-o] [-p]\r\n"

ACMD(do_users)
{
  const char *format = "%3d %-7s %-12s %-14s %-3s %-8s ";
  char line[200], line2[220], idletime[10], classname[20];
  char state[30], *timeptr, mode;
  char name_search[MAX_INPUT_LENGTH], host_search[MAX_INPUT_LENGTH];
  struct char_data *tch;
  struct descriptor_data *d;
  size_t i;
  int low = 0, high = LVL_IMPL, num_can_see = 0;
  int showclass = 0, outlaws = 0, playing = 0, deadweight = 0;

  host_search[0] = name_search[0] = '\0';

  strcpy(buf, argument);
  while (*buf) {
    half_chop(buf, arg, buf1);
    if (*arg == '-') {
      mode = *(arg + 1);  /* just in case; we destroy arg in the switch */
      switch (mode) {
      case 'o':
      case 'k':
        outlaws = 1;
        playing = 1;
        strcpy(buf, buf1);
        break;
      case 'p':
        playing = 1;
        strcpy(buf, buf1);
        break;
      case 'd':
        deadweight = 1;
        strcpy(buf, buf1);
        break;
      case 'l':
        playing = 1;
        half_chop(buf1, arg, buf);
        sscanf(arg, "%d-%d", &low, &high);
        break;
      case 'n':
        playing = 1;
        half_chop(buf1, name_search, buf);
        break;
      case 'h':
        playing = 1;
        half_chop(buf1, host_search, buf);
        break;
      case 'c':
        playing = 1;
        half_chop(buf1, arg, buf);
        for (i = 0; i < strlen(arg); i++)
          showclass |= find_class_bitvector(arg[i]);
        break;
      default:
        send_to_char(USERS_FORMAT, ch);
        return;
      }                         /* end of switch */

    } else {                    /* endif */
      send_to_char(USERS_FORMAT, ch);
      return;
    }
  }                             /* end while (parser) */
  strcpy(line,
         "Num Class    Name         State          Idl Login@   Site\r\n");
  strcat(line,
         "--- -------- ------------ -------------- --- -------- ------------------------\r\n");
  send_to_char(line, ch);

  one_argument(argument, arg);

  for (d = descriptor_list; d; d = d->next) {
    if (STATE(d) != CON_PLAYING && playing)
      continue;
    if (STATE(d) == CON_PLAYING && deadweight)
      continue;
    if (STATE(d) == CON_PLAYING) {
      if (d->original)
        tch = d->original;
      else if (!(tch = d->character))
        continue;

      if (*host_search && !strstr(d->host, host_search))
        continue;
      if (*name_search && str_cmp(GET_NAME(tch), name_search))
        continue;
      if (!CAN_SEE(ch, tch) || GET_LEVEL(tch) < low || GET_LEVEL(tch) > high)
        continue;
      if (outlaws && !PLR_FLAGGED(tch, PLR_KILLER) &&
          !PLR_FLAGGED(tch, PLR_THIEF))
        continue;
      if (showclass && !(showclass & (1 << GET_CLASS(tch))))
        continue;
      if (GET_INVIS_LEV(ch) > GET_LEVEL(ch))
        continue;

      if (d->original)
        sprintf(classname, "[%3d %s]", GET_LEVEL(d->original),
                CLASS_ABBR(d->original));
      else
        sprintf(classname, "[%3d %s]", GET_LEVEL(d->character),
                CLASS_ABBR(d->character));
    } else
      strcpy(classname, "   -   ");

    timeptr = asctime(localtime(&d->login_time));
    timeptr += 11;
    *(timeptr + 8) = '\0';

    if (STATE(d) == CON_PLAYING && d->original)
      strcpy(state, "Switched");
    else
      strcpy(state, connected_types[STATE(d)]);
    
    if (d->character && STATE(d) == CON_PLAYING && GET_LEVEL(d->character) < LVL_GOD)
      sprintf(idletime, "%3d", d->character->char_specials.timer *
              SECS_PER_MUD_HOUR / SECS_PER_REAL_MIN);
    
    else
      strcpy(idletime, "");

    if (d->character && d->character->player.name) {
      if (d->original)
        sprintf(line, format, d->desc_num, classname,
                d->original->player.name, state, idletime, timeptr);
      else
        sprintf(line, format, d->desc_num, classname,
                d->character->player.name, state, idletime, timeptr);
    } else
      sprintf(line, format, d->desc_num, "    -   ", "UNDEFINED",
              state, idletime, timeptr);
		if (PLR_FLAGGED(d->character, PLR_SECPLAYER))
    sprintf(line + strlen(line), "[&C%s&n]%s&n\r\n", d->host, (count_ips(d->host) > 1 ? "&R*" : ""));
    
    if (!PLR_FLAGGED(d->character, PLR_SECPLAYER))
    sprintf(line + strlen(line), "[&W%s&n]%s&n\r\n", d->host, (count_ips(d->host) > 1 ? "&R*" : "&Y*"));
   
                
    //else
   //   strcat(line, "[Hostname unknown]\r\n");

    if (STATE(d) != CON_PLAYING) {
      sprintf(line2, "%s%s%s", CCGRN(ch, C_SPR), line, CCNRM(ch, C_SPR));
      strcpy(line, line2);
    }
    if (STATE(d) != CON_PLAYING ||
                (STATE(d) == CON_PLAYING && CAN_SEE(ch, d->character))) {
      send_to_char(line, ch);
      num_can_see++;
    }
  }

  sprintf(line, "\r\n%d visible sockets connected.\r\n", num_can_see);
  send_to_char(line, ch);
}


/* Generic page_string function for displaying text */
ACMD(do_gen_ps)
{
  switch (subcmd) {
  case SCMD_CREDITS:
    page_string(ch->desc, credits, 0);
    break;
  case SCMD_NEWS:
    page_string(ch->desc, news, 0);
    break;
  case SCMD_INFO:
    if(PRF_FLAGGED(ch, PRF_INFO_START))
	REMOVE_BIT(PRF_FLAGS(ch), PRF_INFO_START);
    page_string(ch->desc, info, 0);
    break;
  case SCMD_WIZLIST:
    send_to_char(wizlist, ch);
    break;
  case SCMD_IMMLIST:
    send_to_char(immlist, ch);
    break;
  case SCMD_HANDBOOK:
    page_string(ch->desc, handbook, 0);
    break;
  case SCMD_POLICIES:
    page_string(ch->desc, policies, 0);
    break;
  case SCMD_MOTD:
    send_to_char(motd, ch);
    break;
  case SCMD_IMOTD:
    send_to_char(imotd, ch);
    break;
  case SCMD_CLEAR:
    send_to_char("\033[H\033[J", ch);
    break;
  case SCMD_VERSION:
    send_to_char(CIRCLEMUD_VERSION, ch);
    send_to_char("\r\n", ch);
    break;
  case SCMD_WHOAMI:
    send_to_char(strcat(strcpy(buf, GET_NAME(ch)), "\r\n"), ch);
    break;
  default:
    log("SYSERR: Unhandled case in do_gen_ps. (%d)", subcmd);
    return;
  }
}


void perform_mortal_where(struct char_data * ch, char *arg)
{
  struct char_data *i;
  struct descriptor_data *d;

  if (!*arg) {
    send_to_char("Players in your Zone\r\n--------------------\r\n", ch);
    for (d = descriptor_list; d; d = d->next) {
      if (STATE(d) != CON_PLAYING || d->character == ch)
        continue;
      if ((i = (d->original ? d->original : d->character)) == NULL)
        continue;
      if (i->in_room == NOWHERE || !CAN_SEE(ch, i))
        continue;
      if (world[ch->in_room].zone != world[i->in_room].zone)
        continue;
      sprintf(buf, "%-20s - %s\r\n", GET_NAME(i), world[i->in_room].name);
      send_to_char(buf, ch);
    }
  } else {                      /* print only FIRST char, not all. */
    for (i = character_list; i; i = i->next) {
      if (i->in_room == NOWHERE || i == ch)
        continue;
      if (!CAN_SEE(ch, i) || world[i->in_room].zone != world[ch->in_room].zone)
        continue;
      if (!isname(arg, i->player.name))
        continue;
      sprintf(buf, "%-25s - %s\r\n", GET_NAME(i), world[i->in_room].name);
      send_to_char(buf, ch);
      return;
    }
    send_to_char("No-one around by that name.\r\n", ch);
  }
}


void print_object_location(int num, struct obj_data * obj, struct char_data * ch,
                                int recur)
{

  if (num > 0)
    sprintf(buf, "O%3d. %-25s - ", num, obj->short_description);
  else
    sprintf(buf, "%33s", " - ");

  if (obj->in_room > NOWHERE) {
    sprintf(buf + strlen(buf), "[%5d] %s\r\n",
            GET_ROOM_VNUM(IN_ROOM(obj)), world[obj->in_room].name);
    send_to_char(buf, ch);
  } else if (obj->carried_by) {
    sprintf(buf + strlen(buf), "carried by %s\r\n",
            PERS(obj->carried_by, ch));
    send_to_char(buf, ch);
  } else if (obj->worn_by) {
    sprintf(buf + strlen(buf), "worn by %s\r\n",
            PERS(obj->worn_by, ch));
    send_to_char(buf, ch);
  } else if (obj->in_obj) {
    sprintf(buf + strlen(buf), "inside %s%s\r\n",
            obj->in_obj->short_description, (recur ? ", which is" : " "));
    send_to_char(buf, ch);
    if (recur)
      print_object_location(0, obj->in_obj, ch, recur);
  } else {
    sprintf(buf + strlen(buf), "in an unknown location\r\n");
    send_to_char(buf, ch);
  }

}

void perform_immort_where(struct char_data * ch, char *arg)
{
  struct char_data *i;
  struct obj_data *k;
  struct descriptor_data *d;
  int num = 0, found = 0;

  if (!*arg) {
    send_to_char("Players\r\n-------\r\n", ch);
    for (d = descriptor_list; d; d = d->next)
      if (STATE(d) == CON_PLAYING) {
        i = (d->original ? d->original : d->character);
        if (i && CAN_SEE(ch, i) && (i->in_room != NOWHERE)) {
          if (d->original)
            sprintf(buf, "%-20s - [%5d] %s (in %s)\r\n",
                    GET_NAME(i), GET_ROOM_VNUM(IN_ROOM(d->character)),
                 world[d->character->in_room].name, GET_NAME(d->character));
          else
            sprintf(buf, "%-20s - [%5d] %s\r\n", GET_NAME(i),
                    GET_ROOM_VNUM(IN_ROOM(i)), world[i->in_room].name);
          send_to_char(buf, ch);
        }
      }
  } else {
    for (i = character_list; i; i = i->next)
      if (CAN_SEE(ch, i) && i->in_room != NOWHERE && isname(arg, i->player.name)) {
        found = 1;
        sprintf(buf, "M%3d. %-25s - [%5d] %s\r\n", ++num, GET_NAME(i),
                GET_ROOM_VNUM(IN_ROOM(i)), world[IN_ROOM(i)].name);
        send_to_char(buf, ch);
      }
    for (num = 0, k = object_list; k; k = k->next)
      if (CAN_SEE_OBJ(ch, k) && isname(arg, k->name)) {
        found = 1;
        print_object_location(++num, k, ch, TRUE);
      }

    if (!found){
      sprintf(buf,"Couldn't find any such thing.\r\n");
      send_to_char(buf, ch);
      return;
    }
  }
}

ACMD(do_where)
{
  one_argument(argument, arg);

  if (GET_LEVEL(ch) >= LVL_ELDER)
    perform_immort_where(ch, arg);
  else
    perform_mortal_where(ch, arg);
}



ACMD(do_levels)
{
  int i;

  if (IS_NPC(ch)) {
    send_to_char("You ain't nothin' but a hound-dog.\r\n", ch);
    return;
  }
  strcpy(buf, "&cXP &WTable&n\r\n\r\n");

  for (i = 1; i < LVL_IMMORT; i++)
    sprintf(buf + strlen(buf), "&W[&c%3d&W] &C%12s&n%s", i,
            add_points(level_exp(GET_REMORT(ch), i+1)),
            ((!(i % 4)) ? "\r\n" : "  "));

  page_string(ch->desc, buf, 1);
}


/* Function to calculate a char's combat rating */
int combat_rating(struct char_data *ch)
{
    int rating;

    if (IS_NPC(ch))
	rating = GET_LEVEL(ch);
    else
	rating = GET_LEVEL(ch);

    rating = (int)(rating * (float)GET_HIT(ch)/(float)GET_MAX_HIT(ch));

    return rating;
}

ACMD(do_consider)
{
   struct char_data *victim;
   int	diff;

   one_argument(argument, buf);

   if (!(victim = get_char_room_vis(ch, buf))) {
      send_to_char("Consider killing who?\n\r", ch);
      return;
   }

   if (victim == ch) {
      send_to_char("Easy!  Very easy indeed!\n\r", ch);
      return;
   }

   if (!IS_NPC(victim)) {
      send_to_char("Would you like to borrow a cross and a shovel?\n\r", ch);
      return;
   }

    if (GET_HIT(victim) < 1) {
       send_to_char("I think it's dead already.\n\r", ch);
       return;
    }

    diff = GET_AC(victim) - GET_AC(ch);     /*Print AC comparison */
    if (diff <= -140)
        send_to_char("Your victim is massively better protected than you.\n\r", ch);
    else if (diff <= -80)
        send_to_char("Your victim is well better armored than you!\n\r", ch);
    else if (diff <= -20)
        send_to_char("Your victim is better armored than you.\n\r", ch);
    else if (diff <=  30)
        send_to_char("Your victim is about evenly armored with you.\n\r", ch);
    else if (diff <=  90)
        send_to_char("Your victim lacks some of your protection.\n\r", ch);
    else if (diff <= 150)
        send_to_char("Your victim lacks much of your protection.\n\r", ch);
    else
        send_to_char("Your victim is grossly under armored compared to you.\n\r", ch);


    diff = (int) (100 - GET_HIT(victim)*100/GET_HIT(ch)); /* HP % Compare */
    if (diff <= -49)
        send_to_char("Your victim is massively healthier than you!\n\r", ch);
    else if (diff <= -29)
        send_to_char("Your victim is considerably healthier than you.\n\r", ch);
    else if (diff <=  -9)
        send_to_char("Your victim is healthier than you.\n\r", ch);
    else if (diff <=  10)
        send_to_char("Your victim is about the same health as you.\n\r", ch);
    else if (diff <=  30)
        send_to_char("Your victim is not as healthy as you.\n\r", ch);
    else if (diff <=  50)
        send_to_char("Your victim lacks your vigor.\n\r", ch);
    else
        send_to_char("Your victim is puny in comparison.\n\r", ch);


   diff = (combat_rating(victim) - combat_rating(ch));

   if (diff <= -10)
       send_to_char("Now where did that chicken go?\n\r", ch);
   else if (diff <= -5)
       send_to_char("You could do it with a needle!\n\r", ch);
   else if (diff <= -2)
       send_to_char("Easy.\n\r", ch);
   else if (diff <= -1)
       send_to_char("Fairly easy.\n\r", ch);
   else if (diff == 0)
       send_to_char("The perfect match!\n\r", ch);
   else if (diff <= 1)
       send_to_char("You would need some luck!\n\r", ch);
   else if (diff <= 2)
       send_to_char("You would need a lot of luck!\n\r", ch);
   else if (diff <= 3)
       send_to_char("You would need a lot of luck and great equipment!\n\r", ch);
   else if (diff <= 5)
       send_to_char("Do you feel lucky, punk?\n\r", ch);
   else if (diff <= 10)
       send_to_char("Are you mad!?\n\r", ch);
   else if (diff <= 15)
       send_to_char("You ARE mad!\n\r", ch);
   else if (diff <= 20)
       send_to_char("Why not pretend you are dead instead?\n\r", ch);
   else if (diff <= 30)
       send_to_char("Your brain will be a nice decoration on the walls!!\n\r", ch);
   else
       send_to_char("You are a very dumb player for even considering.\n\r", ch);
       play_sound(ch, "consider.wav", SND_CHAR);
}

ACMD(do_diagnose)
{
  struct char_data *vict;

  one_argument(argument, buf);

  if (*buf) {
    if (!(vict = get_char_room_vis(ch, buf)))
      send_to_char(NOPERSON, ch);
    else
      diag_char_to_char(vict, ch);
  } else {
    if (FIGHTING(ch))
      diag_char_to_char(FIGHTING(ch), ch);
    else
      send_to_char("Diagnose who?\r\n", ch);
  }
}

ACMD(do_color)
{
  if (IS_NPC(ch))
    return;

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Usage: color <light|off>\r\n", ch);
    return;
  }

  if(isname(arg, "light")) {
    SET_BIT(PRF_FLAGS(ch), PRF_COLOR_1 | PRF_COLOR_2);
    sprintf(buf, "You light your %scolors%s.\r\n", CCBWHT(ch, C_SPR),
          CCNRM(ch, C_OFF));
  } else if(isname(arg, "off")) {
    REMOVE_BIT(PRF_FLAGS(ch), PRF_COLOR_1 | PRF_COLOR_2);
    sprintf(buf, "Your %scolor%s is now OFF.\r\n", CCBWHT(ch, C_SPR),
          CCNRM(ch, C_OFF));
  } else
      sprintf(buf, "Usage: color <light|off>\r\n");

  send_to_char(buf, ch);
}


ACMD(do_toggle)
{
  if (IS_NPC(ch))
    return;
  if (GET_WIMP_LEV(ch) == 0)
    strcpy(buf2, "OFF");
  else
    sprintf(buf2, "%-3d", GET_WIMP_LEV(ch));

  sprintf(buf,
  "&gHit Pnt Display: &c[&r%-3s&c]         &gBrief Mode: &c[&r%-3s&c]     &gSummon Protect: &c[&r%-3s&c]\r\n"
  "   &gMove Display: &c[&r%-3s&c]       &gCompact Mode: &c[&r%-3s&c]           &gOn Quest: &c[&r%-3s&c]\r\n"
  "   &gMana Display: &c[&r%-3s&c]             &gNoTell: &c[&r%-3s&c]       &gRepeat Comm.: &c[&r%-3s&c]\r\n"
  " &gAuto Show Exit: &c[&r%-3s&c]               &gDeaf: &c[&r%-3s&c]         &gWimp Level: &c[&r%-3s&c]\r\n"
  " &gGossip Channel: &c[&r%-3s&c]    &gAuction Channel: &c[&r%-3s&c]      &gGrats Channel: &c[&r%-3s&c]\r\n"
  "  &gAuto Get Gold: &c[&r%-3s&c]  &gAuto Loot Corpses: &c[&r%-3s&c]    &gAuto Split Gold: &c[&r%-3s&c]\r\n"
  "    &gAuto Assist: &c[&r%-3s&c]        &gColor Level: &c[&r%-3s&c]     &gNewbie Channel: &c[&r%-3s&c]\r\n"
  "&g            MSP: &c[&r%-3s&c]	 &gPaths Advice: &c[&r%-3s&c]					\r\n",	
      ONOFF(PRF_FLAGGED(ch, PRF_DISPHP)),
      ONOFF(PRF_FLAGGED(ch, PRF_BRIEF)),
      ONOFF(!PRF_FLAGGED(ch, PRF_SUMMONABLE)),

      ONOFF(PRF_FLAGGED(ch, PRF_DISPMOVE)),
      ONOFF(PRF_FLAGGED(ch, PRF_COMPACT)),
      YESNO(PRF_FLAGGED(ch, PRF_QUEST)),

      ONOFF(PRF_FLAGGED(ch, PRF_DISPMANA)),
      ONOFF(PRF_FLAGGED(ch, PRF_NOTELL)),
      YESNO(!PRF_FLAGGED(ch, PRF_NOREPEAT)),

      ONOFF(PRF_FLAGGED(ch, PRF_AUTOEXIT)),
      YESNO(PRF_FLAGGED(ch, PRF_DEAF)),
      buf2,

      ONOFF(!PRF_FLAGGED(ch, PRF_NOGOSS)),
      ONOFF(!PRF_FLAGGED(ch, PRF_NOAUCT)),
      ONOFF(!PRF_FLAGGED(ch, PRF_NOGRATZ)),

      ONOFF(PRF_FLAGGED(ch, PRF_AUTOGOLD)),
      ONOFF(PRF_FLAGGED(ch, PRF_AUTOLOOT)),
      ONOFF(PRF_FLAGGED(ch, PRF_AUTOSPLIT)),
      ONOFF(PRF_FLAGGED(ch, PRF_AUTOASSIST)),

      ONOFF(PRF_FLAGGED(ch, PRF_COLOR_1)),
      ONOFF(!PRF2_FLAGGED(ch, PRF2_NONEWBIE)),
      ONOFF(PLR_FLAGGED(ch, PLR_MSP)),	
      ONOFF(!PRF2_FLAGGED(ch, PRF2_NOPATHS)));
  send_to_char(buf, ch);
}


struct sort_struct {
  int sort_pos;
  byte is_social;
} *cmd_sort_info = NULL;

int num_of_cmds;


void sort_commands(void){
  int a, b, tmp;

  num_of_cmds = 0;

  /*
   * first, count commands (num_of_commands is actually one greater than the
   * number of commands; it inclues the '\n'.
   */
  while (*complete_cmd_info[num_of_cmds].command != '\n')
    num_of_cmds++;

  /* check if there was an old sort info.. then free it -- aedit -- M. Scott*/
  if (cmd_sort_info)
   free(cmd_sort_info);

  /* create data array */
  CREATE(cmd_sort_info, struct sort_struct, num_of_cmds);

  /* initialize it */
  for (a = 1; a < num_of_cmds; a++) {
    cmd_sort_info[a].sort_pos = a;
    cmd_sort_info[a].is_social = (complete_cmd_info[a].command_pointer == do_action);
  }

  /* the infernal special case */
  cmd_sort_info[find_command("insult")].is_social = TRUE;

  /* Sort.  'a' starts at 1, not 0, to remove 'RESERVED' */
  for (a = 1; a < num_of_cmds - 1; a++)
    for (b = a + 1; b < num_of_cmds; b++)
      if (strcmp(complete_cmd_info[cmd_sort_info[a].sort_pos].command,
		complete_cmd_info[cmd_sort_info[b].sort_pos].command) > 0) {
         tmp = cmd_sort_info[a].sort_pos;
        cmd_sort_info[a].sort_pos = cmd_sort_info[b].sort_pos;
        cmd_sort_info[b].sort_pos = tmp;
      }
}



ACMD(do_commands)
{
  int no, i, cmd_num;
  int wizhelp = 0, socials = 0;
  struct char_data *vict;

  one_argument(argument, arg);

  if (*arg) {
    if (!(vict = get_char_vis(ch, arg)) || IS_NPC(vict)) {
      send_to_char("Who is that?\r\n", ch);
      return;
    }
    if (GET_LEVEL(ch) < GET_LEVEL(vict)) {
      send_to_char("You can't see the commands of people above your level.\r\n", ch);
      return;
    }
  } else
    vict = ch;

  if (subcmd == SCMD_SOCIALS)
    socials = 1;
  else if (subcmd == SCMD_WIZHELP)
    wizhelp = 1;

  sprintf(buf, "The following %s%s are available to %s:\r\n",
          wizhelp ? "privileged " : "",
          socials ? "socials" : "commands",
          vict == ch ? "you" : GET_NAME(vict));

  /* cmd_num starts at 1, not 0, to remove 'RESERVED' */
  for (no = 1, cmd_num = 1; cmd_num < num_of_cmds; cmd_num++) {
    i = cmd_sort_info[cmd_num].sort_pos;
    if (complete_cmd_info[i].minimum_level >= 0 &&
	GET_LEVEL(vict) >= complete_cmd_info[i].minimum_level &&
	(complete_cmd_info[i].minimum_level >= LVL_IMMORT) == wizhelp &&
        (wizhelp || socials == cmd_sort_info[i].is_social)) {
      if (wizhelp)
        sprintf(buf + strlen(buf), "[%-3d] %-12s ", complete_cmd_info[i].minimum_level, complete_cmd_info[i].command);
      else
        sprintf(buf + strlen(buf), "%-19s", complete_cmd_info[i].command);
      if (!(no % 4))
        strcat(buf, "\r\n");
      else
        strcat(buf, " ");
      no++;
    }
  }

  strcat(buf, "\r\n");
  page_string(ch->desc, buf, 1);
//  send_to_char(buf, ch);
}

/*
char *howgood(int percent)
{
  static char buf[256];

  if (percent == 0)
    strcpy(buf, " &R(error)&n");
  else if (percent <= 10)
    strcpy(buf, " &B( &R10%&B)&n");
  else if (percent <= 20)
    strcpy(buf, " &B( &R20%&B)&n");
  else if (percent <= 40)
    strcpy(buf, " &B( &R40%&B)&n");
  else if (percent <= 55)
    strcpy(buf, " &B( &R55%&B)&n");
  else if (percent <= 70)
    strcpy(buf, " &B( &R70%&B)&n");
  else if (percent <= 80)
    strcpy(buf, " &B( &R80%&B)&n");
  else if (percent <= 90)
    strcpy(buf, " &B( &R95%&B)&n");
  else
    strcpy(buf, " &B(&R100%&B)&n");

  return (buf);
}
*/

char *howgood(int percent)
{
  if (percent < 0)
    return " &R(error)&n";
  if (percent == 0)
    return " &B(&Rnot learned&B)&n";
  if (percent <= 10)
    return "     &B(&Rawful&B)&n";
  if (percent <= 20)
    return "       &B(&Rbad&B)&n";
  if (percent <= 40)
    return "      &B(&Rpoor&B)&n";
  if (percent <= 55)
    return "      &B(&Rfair&B)&n";
  if (percent <= 70)
    return "   &B(&Raverage&B)&n";
  if (percent <= 80)
    return "      &B(&Rgood&B)&n";
  if (percent <= 85)
    return " &B(&Rvery good&B)&n";

    return "    &B(&Rsuperb&B)&n";
}

ACMD(do_spell)
{
 extern char *spells[];
 int i, c=0, x=0;

 if(IS_NPC(ch)){
 	send_to_char("Mobs checking spells???\r\n", ch);
 	return;
 }

 send_to_char("&BThese are the spells you know: (&W*&B = Special/Extra Spell)&n\r\n", ch);

 *buf2 = '\0';
 for (i = 1; i < MAX_SPELLS+1; i++)
 {
  if (GET_SKILL(ch, i) > 0 && strncmp(spells[i], "!UNUSED!", strlen(spells[i]))) {
    x++;
    c++;
    sprintf(buf, "%s&c%-22s&n  &W%s&n %s", (((GET_LEVEL(ch) <= LVL_IMMORT) && (GET_LEVEL(ch) < spell_info[i].min_level[(int) GET_CLASS(ch)])) ? "&W*&n" : " "),
    spells[i], howgood(GET_SKILL(ch, i)), (((c == 2)) ? "&n\r\n" : "&w|&n "));
    strcat(buf2, buf);
    if(c==2)
       c=0;
  }
 }
 if(c % 2)
    strcat(buf2, "\r\n\r\n");

 if(!x)
   strcat(buf2, "\r\n&WYou have no &cspells&W learned yet.\r\n");

 page_string(ch->desc, buf2, 1);
}

ACMD(do_skill)
{
 extern char *spells[];
 int i, c=0, x=0;

 if(IS_NPC(ch)){
 	send_to_char("Mobs checking skills???\r\n", ch);
 	return;
 }

 send_to_char("&BThese are the skills you know: (&W*&B = Special/Extra Skill)&n\r\n", ch);

 *buf2 = '\0';
 for (i = MAX_SPELLS +1; i < MAX_SKILLS+1; i++)
 {
  if (GET_SKILL(ch, i) > 0 && strncmp(spells[i], "!UNUSED!", strlen(spells[i]))) {
    x++;
    c++;
    sprintf(buf, "%s&c%-22s&n  &W%s&n %s", (((GET_LEVEL(ch) <= LVL_IMMORT) && (GET_LEVEL(ch) < spell_info[i].min_level[(int) GET_CLASS(ch)])) ? "&W*&n" : " "),
    spells[i], howgood(GET_SKILL(ch, i)), (((c == 2)) ? "&n\r\n" : "&w|&n "));
    strcat(buf2, buf);
    if(c==2)
       c=0;
  }
 }
 if(c % 2)
    strcat(buf2, "\r\n\r\n");

 if(!x)
   strcat(buf2, "\r\n&WYou have no &cskills&W learned yet.\r\n");

 page_string(ch->desc, buf2, 1);
}

ACMD(do_scan)
{
  struct char_data *i;
  int is_in, dir, dis, maxdis, found = 0;

  const char *distance[] = {
    "right here",
    "immediately ",
    "nearby ",
    "a ways ",
    "far ",
    "very far ",
    "extremely far ",
    "impossibly far ",
  };

  if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_SCAN)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  if (IS_AFFECTED(ch, AFF_BLIND)) {
    act("You can't see anything, you're blind!", TRUE, ch, 0, 0, TO_CHAR);
    return;
  }
  if ((GET_MOVE(ch) < 3) && (GET_LEVEL(ch) < LVL_IMMORT)) {
    act("You are too exhausted.", TRUE, ch, 0, 0, TO_CHAR);
    return;
  }

  maxdis = (1 + ((GET_SKILL(ch, SKILL_SCAN) * 5) / 100));
  if (GET_LEVEL(ch) >= LVL_IMMORT)
    maxdis = 7;

  act("You quickly scan the area and see:", TRUE, ch, 0, 0, TO_CHAR);
  act("$n quickly scans the area.", FALSE, ch, 0, 0, TO_ROOM);
  if (GET_LEVEL(ch) < LVL_IMMORT)
    GET_MOVE(ch) -= GET_LEVEL(ch) / 9;

  is_in = ch->in_room;
  for (dir = 0; dir < NUM_OF_DIRS; dir++) {
    ch->in_room = is_in;
    for (dis = 0; dis <= maxdis; dis++) {
      if (((dis == 0) && (dir == 0)) || (dis > 0)) {
        for (i = world[ch->in_room].people; i; i = i->next_in_room) {
          if ((!((ch == i) && (dis == 0))) && CAN_SEE(ch, i)) {
            sprintf(buf, "%33s, %s%s%s%s", GET_NAME(i), distance[dis],
                    ((dis > 0) && (dir < (NUM_OF_DIRS - 2))) ? "to the " : "",
                    (dis > 0) ? dirs[dir] : "",
                    ((dis > 0) && (dir > (NUM_OF_DIRS - 3))) ? "wards" : "");
            act(buf, TRUE, ch, 0, 0, TO_CHAR);
            found++;
	    improve_skill(ch, SKILL_SCAN);
          }
        }
      }
      if (!CAN_GO(ch, dir) || (world[ch->in_room].dir_option[dir]->to_room == is_in))
        break;
      else
        ch->in_room = world[ch->in_room].dir_option[dir]->to_room;
    }
  }
  if (found == 0)
    act("Nobody anywhere near you.", TRUE, ch, 0, 0, TO_CHAR);
  ch->in_room = is_in;
}

ACMD(do_player)
{
  struct char_data *victim = NULL;

  skip_spaces(&argument);

  if (!*argument) {
      sprintf(buf, "Find info on which player?\r\n");
      send_to_char(buf, ch);
    } else {
      CREATE(victim, struct char_data, 1);
      clear_char(victim);
      CREATE(victim->player_specials, struct player_special_data, 1);
      if (load_char(argument, victim) > -1) {
         if(PLR_FLAGGED(victim, PLR_DELETED)){
            send_to_char("Find info on which player?\r\n", ch);
            return;
         }
	sprintf(buf, "\r\n&C%s&n is a great &c%s&n with the race &c%s&n.\r\n", GET_NAME(victim), pc_class_types[(int) GET_CLASS(victim)], pc_race_types[(int) GET_RACE(victim)]);
	sprintf(buf + strlen(buf), "%s is at level &G%d&n. &GR&Y%d&n.\r\n", (GET_SEX(victim) ? (GET_SEX(victim)==SEX_MALE ? "He" :"She") : "It"), GET_LEVEL(victim), GET_REMORT(victim));
	sprintf(buf + strlen(buf), "&RWAR&n -> Died [&g%d&n] Killed [&G%d&n]\r\n", GET_ARENA_DIED(victim), GET_ARENA_KILLED(victim));
	sprintf(buf + strlen(buf), "He has died &R%d&n times and killed &B%d&n opponents.\r\n%s", GET_DIED(victim), GET_KILLED(victim), (victim->player.whoisdesc == NULL ? "" : "\r\n"));
	if(victim->player.whoisdesc)
	  sprintf(buf + strlen(buf), "%s\r\n", victim->player.whoisdesc);
	send_to_char(buf, ch);
      } else {
	sprintf(buf, "There is no such player.\r\n");
	send_to_char(buf, ch);
      }
      free_char(victim);
    }

}

ACMD(do_slist)
{
 extern char *pc_class_types[];
 extern char *spells[];
 extern struct spell_info_type spell_info[];
 int i, j, class, x = 1;
 bool flag = FALSE;

 *buf = '\0';

 one_argument(argument, arg);

  if (!*arg) {
   class = GET_CLASS(ch);
 } else {
     class = search_block_case(arg, pc_class_types, FALSE);
     send_to_char(buf, ch);
     if (class < 0 || class >= NUM_CLASSES) {
       send_to_char("Not a valid class.\n\r", ch);
       return;
     }
 }
 send_to_char("\r\n", ch);
 for (j = 1; j < LVL_IMMORT; j++) {
   for (i = 1; i < MAX_SKILLS+1; i++)
   {
      if (spell_info[i].min_level[class] == j)
        {
          sprintf(buf+strlen(buf), "&w[&W%3d&w] &C%-19s&n%s", spell_info[i].min_level[class], reprimir(spells[i], 19), (!(x++ % 3) ? "\r\n" : " "));
          flag = TRUE;
         }
   }
 }

 if (flag == FALSE) {
   if (!*arg)
     sprintf(buf, "&CYou will not learn anything in this class.&n\r\n");
   else
      sprintf(buf, "&CThe %s class will not learn anything.&n\r\n", pc_class_types[class]);
 }

 send_to_char(buf, ch);
 send_to_char("\r\n", ch);
}

/*
ACMD(do_sklist)
{
 extern char *pc_class_types[];
 extern char *spells[];
 extern struct spell_info_type spell_info[];
 int i, j, class;
 bool flag = FALSE;

 *buf = '\0';
 *buf2 = '\0';

 class = GET_CLASS(ch);
 strcpy(buf, "&WYou will learn these skills:&n\r\n\r\n"
             "&gSkill                  Level&n\r\n");
 strcpy(buf2, buf);

 for (j = 1; j < LVL_IMMORT; j++) {
   for (i = MAX_SPELLS +1; i < MAX_SKILLS+1; i++)
   {
      if (spell_info[i].min_level[class] == j)
      {
        sprintf(buf, "&G%-20s&n   &g[&G%3d&g]&n\r\n", spells[i], spell_info[i].min_level[class]);
        strcat(buf2, buf);
        flag = TRUE;
      }
   }
 }

 if (flag == FALSE) {
   if (!*arg)
     sprintf(buf2, "You will not learn skills in this class.\r\n");
     else
      sprintf(buf2, "The %s class will not learn skills.\r\n", pc_class_types[class]);
 }

 page_string(ch->desc, buf2, 1);
}
*/
char *arena(int points)
{
  if (points == 0)
    return "No Fights";
  if (points < 10)
    return "Arena Winner";
  if (points < 20)
    return "Fighter";
  if (points <= 30)
    return "Knight";
  if (points <= 40)
    return "Great Knight";
  if (points <= 50)
    return "Prince of War";
  if (points <= 60)
    return "King of War";
  if (points <= 70)
    return "Master of War";
  if (points <= 80)
    return "Eliminator";
  else
    return "MAXIMUS";
}

ACMD(do_whois)
{
  struct char_data *victim;
  //struct char_data *ch = NULL;
  int clan_num = 0;

  skip_spaces(&argument);

if (!*argument)
 {
  send_to_char("Do a WhoIS on which player?\r\n", ch);
  
 }

   else
  {
    CREATE(victim, struct char_data, 1);
    clear_char(victim);
    CREATE(victim->player_specials, struct player_special_data, 1);
   
       if (load_char(argument, victim) > -1)
     {

       if(PLR_FLAGGED(victim, PLR_DELETED))
       {
         send_to_char("Do a WhoIS on which player?\r\n", ch);
         return;
       }

        sprintf(buf, "\r\n         &RWARDOME PERSONAL INFORMATION - &RW&rH&RO&rI&RS&r - \r\n");
        sprintf(buf + strlen(buf), "\n&bInformation of %s&C%s, &n%s. &b%s is at level &B%d.\r\n",(GET_PRENAME(victim) != NULL ? GET_PRENAME(victim) : ""), GET_NAME(victim),GET_TITLE(victim),(GET_SEX(victim) ? (GET_SEX(victim)==SEX_MALE ? "He" :"She") : "It"), GET_LEVEL(victim));
        sprintf(buf + strlen(buf), "&bThe greater &B%s &bwith the race &B%s &band have &B%d years old.\r\n", pc_class_types[(int) GET_CLASS(victim)], pc_race_types[(int) GET_RACE(victim)], GET_AGE(victim));
        
        sprintf(buf + strlen(buf), "\n&RPk&r Information        &RWar&r Information    \r\n");
        sprintf(buf + strlen(buf), "&wKills  : &Y%-5d          &wWins : &Y%-5d                 \r\n", GET_KILLED(victim),GET_ARENA_KILLED(victim));
//      sprintf(buf + strlen(buf), "&wDied   : &Y%-5d          &wLoses: &Y%-5d                 \r\n", GET_DIED(victim),GET_ARENA_DIED(victim));
        sprintf(buf + strlen(buf), "&wRemorts: &G%-2d&n        &wArena Flag: &R%s&n                         \r\n", GET_REMORT(victim), arena(GET_ARENA_KILLED(victim)));
	
	
	if(GET_ICQ(victim) > 1)
	{
	sprintf(buf + strlen(buf), "\n&WICQ: &Y%d&n	                                       \r\n", (GET_ICQ(victim)));
	}
		if(GET_CLAN(victim) && GET_CLAN_POS(victim))
		{
			clan_num = posicao_clan(GET_CLAN(victim));
			sprintf(buf + strlen(buf), "&W%s is a &n%s &Win the &n%s&W.&n\r\n",
				(GET_SEX(victim) ? (GET_SEX(victim)==SEX_MALE ? "He" :"She") : "It"),
				clans[clan_num].pos_nome[GET_CLAN_POS(victim)-1], clans[clan_num].nome_clan);
		}

	  if(victim->player.whoisdesc)
	  sprintf(buf + strlen(buf), "%s\r\n", victim->player.whoisdesc);
          send_to_char(buf, ch);
     }    else
        {
	  send_to_char("&RThere is no such player.\r\n", ch);
        }
         free_char(victim);
  }
}


/*#define W_ENTER				10

int check_wanted(struct char_data * ch) //descomentei aki
{
 if(GET_LEVEL(ch) > LVL_IMMORT)
   return FALSE;
 if(GET_KILLED(ch) < W_ENTER)
   return FALSE;

 return TRUE;
}

int check_wanted_gain_gold(struct char_data * ch) //essa funcao tbm
{
 int gold = 0;

 if(!check_wanted)
   return FALSE;

 gold = GET_KILLED(ch)*10000;

 return gold;
}

int check_wanted_gain_qp(struct char_data * ch) //essa tbm
{
 int qp = 0;

 if(!check_wanted)
   return FALSE;

 qp = GET_KILLED(ch)/2;

 return qp;
}


char *wanted_c(int percent) //comentar aki tmb
{
  if (percent == 0)
    return NULL;
  if (percent < 20)
    return "Killer";
  if (percent < 30)
    return "Assassin";
  if (percent <= 40)
    return "Son of Carnage";
  if (percent <= 50)
    return "Butcher";
  if (percent <= 60)
    return "Death Messenger";
  if (percent <= 70)
    return "Terminator";
  if (percent <= 80)
    return "Eliminator";
  else
    return "Head Hunter";
}*/



ACMD(do_wanted){ //essa eh o wanted do loiro, soh descomentar.

struct char_data *j;
struct descriptor_data *d;
int soma=0, pontos=0 ;

send_to_char("\r\n&wCurrent &Won-line &wplayers &Rkillers:&n&n\r\n",ch);
for (d = descriptor_list; d; d = d->next){
		
	if (STATE(d) != CON_PLAYING)
        continue;
	if ((j = (d->original ? d->original : d->character)) == NULL)
        continue;
      if (j->in_room == NOWHERE || !CAN_SEE(ch, j))
        continue;
	j = d->character;
        if (PLR_FLAGGED(j, PLR_NOPK))
        continue;
	soma = ((GET_LEVEL(j)*GET_KILLED(j))); //gold e pontos serao transferidos do player morto pro vivo
	pontos = (GET_REMORT(j)*GET_KILLED(j)/10);
	
	
	
		if (GET_LEVEL(j) < 201)
		{
		if (!PLR_FLAGGED(j, PLR_NOPK))
		sprintf(buf, "&C%-11s&w: level &C%-3d&w, wanted for &Y%-7d &wgold coins and &G%-3d &wholy points.&n\r\n", GET_NAME(j), GET_LEVEL(j), soma, pontos);
	     	send_to_char(buf, ch);
	    	
	    }
	     
	     
	     }
		
}

/*ACMD(do_wanted) //esse eh o wanted antigo qualquer coisa comenta ele
{
 int j, w = 0;
 extern int top_of_p_table;
 extern struct player_index_element *player_table;
 struct char_data *victim = NULL;
 char wanted[MAX_STRING_LENGTH];

 skip_spaces(&argument);

 *buf = '\0';
 *wanted = '\0';

 if(!*argument){
  for (j = 0; j <= top_of_p_table; j++){
    CREATE(victim, struct char_data, 1);
    clear_char(victim);
    CREATE(victim->player_specials, struct player_special_data, 1);
    if(load_char((player_table + j)->name, victim) > -1)
     if(check_wanted(victim) && !PLR_FLAGGED(victim, PLR_DELETED)){
	sprintf(wanted + strlen(wanted), " |     &c%-40s&n    &g%6d    &y%13s&n    |\r\n", GET_NAME(victim), check_wanted_gain_qp(victim), add_points(check_wanted_gain_gold(victim)));
        w++;
     }
  }
  free_char(victim);
  if(!w)
    send_to_char("&WNo &ROut&BLaws&W at this moment&n.\r\n", ch);
  else {
	strcpy(buf, " .~'~.:.~'~.:.~'~.:.~'~.:.~'~.:.~'~.:.~'~.:.~'~.:.~'~.:.~'~.:.~'~.:.~'~.:.~'~.:\r\n"
		" | &WList of &RWANTED&Ws in the &cW&bar&cD&bome &WLands.&n                                      |\r\n"
		" | &WHunt them. &CAlive &Wor &RDEAD&W, and off course, &RDEAD&W is MUCH better!&n             |\r\n"
		" .~'~.:.~'~.:.~'~.:.~'~.:.~'~.:.~'~.:.~'~.:.~'~.:.~'~.:.~'~.:.~'~.:.~'~.:.~'~.:\r\n"
		" | -=- &CName&n -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- &GQuestPoints&n -=-=-=-=-= &YGold&n -=- |\r\n"
		" .~'~.:.~'~.:.~'~.:.~'~.:.~'~.:.~'~.:.~'~.:.~'~.:.~'~.:.~'~.:.~'~.:.~'~.:.~'~.:\r\n");
	strcat(buf, wanted);
	strcat(buf, " .~'~.:.~'~.:.~'~.:.~'~.:.~'~.:.~'~.:.~'~.:.~'~.:.~'~.:.~'~.:.~'~.:.~'~.:.~'~.:\r\n");
	send_to_char(buf, ch);
  }
 } else {
      CREATE(victim, struct char_data, 1);
      clear_char(victim);
      CREATE(victim->player_specials, struct player_special_data, 1);
      if (load_char(argument, victim) > -1) {
       if(check_wanted(victim) && !PLR_FLAGGED(victim, PLR_DELETED)){
        sprintf(buf, "&CW&ca&Cr&cD&Co&cm&Ce&n &ROut&BLaw&n &GINFO&n\r\n\r\n");
        sprintf(buf + strlen(buf), "&WWith &c%s&W's head you can gain:\r\n", GET_NAME(victim));
        sprintf(buf + strlen(buf), "&gQP: &G%d        &yGold: &Y%s&n\r\n", check_wanted_gain_qp(victim), add_points(check_wanted_gain_gold(victim)));
		send_to_char(buf, ch);
       } else {
       	 send_to_char("Your target are not a OutLaw.\r\n", ch);
       	 return;
       }
      } else {
        send_to_char("There is no such player.\r\n", ch);
      }
      free_char(victim);
 }
}*/

const char *nome[] = {
	"&Bfirst&n",
	"&Csecond&n",
	"&Gthird&n",
	"&Yfourth&n",
	"&Rfifth&n",
	"\n"
};

ACMD(do_rskills)
{
   int x, i, quebra = 1;

          if(!GET_REMORT(ch)) {
            send_to_char("You have to remort first.\r\n", ch);
            return;
          }

          x = GET_REMORT(ch);
          strcpy(buf, "&WRemort Info&n\r\n\r\n");
          for(;x > 0;x--)
          {
	        if(GET_CLASS_REMORT(ch, x) < 0) {
	            	send_to_char("Call an implementor, you have no remort class.\r\n", ch);
	            	return;
	        }
          	sprintf(buf + strlen(buf), "&W[&g%d&W] &Cremort &Was %s &y%s&W:&n\r\n", x,
            	AN(pc_class_types[GET_CLASS_REMORT(ch, x)]),
          	pc_class_types[GET_CLASS_REMORT(ch, x)]);
          	for(i = 0; i < MAX_RSKILLS; i++)
          		if(GET_RSKILL(ch, x, i))
          			sprintf(buf + strlen(buf), "%s: &c%20s&n [%3d]%s", nome[i],
          			reprimir(spells[GET_RSKILL(ch, x, i)], 20),
           			spell_info[GET_RSKILL(ch, x, i)].min_level[(int)GET_CLASS_REMORT(ch, x)],
           			(!(quebra++ % 2) ? "\r\n" : "   "));
		strcat(buf, "\r\n\r\n");

	   }
           send_to_char(buf, ch);
}

/*ACMD(do_search)
{
  int door, chance = 1;

  *buf = '\0';

  if (IS_AFFECTED(ch, AFF_BLIND)) {
    send_to_char("You're blind, you can't see a damned thing!", ch);
    return;
  }

  send_to_char("\r\nYou begin to search the room...\r\n", ch);

  for (door = 0; door < NUM_OF_DIRS; door++) {
    if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE) {
      if (IS_SET(EXIT(ch, door)->exit_info, EX_HIDDEN)) {
        if (GET_RACE(ch) == RACE_ELF)
          chance += 1;
        if (GET_INT(ch) >= 15)
          chance += 1;
        if (number(1,6) <= chance) {
          sprintf(buf, "\r\n&WYou have found a secret door %s.&n\r\n", dirs[door]);
          send_to_char(buf, ch);
          REMOVE_BIT(EXIT(ch, door)->exit_info, EX_HIDDEN);
        }
      }
    }
  }
  return;
}
*/



