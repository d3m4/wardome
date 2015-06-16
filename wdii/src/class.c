/* ************************************************************************
*   File: class.c                                       Part of CircleMUD *
*  Usage: Source file for class-specific code                             *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

/*
* This file attempts to concentrate most of the code which must be changed
* in order for new classes to be added.  If you're adding a new class,
* you should go through this entire file from beginning to end and add
* the appropriate new special cases for your new class.
*/
 
#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "handler.h"
#include "db.h"
#include "utils.h"
#include "spells.h"
#include "interpreter.h"
#include "olc.h"
#include "buffer.h"
#include "clan.h"

extern struct wis_app_type wis_app[];
extern struct con_app_type con_app[];
extern int siteok_everyone;
extern const char *god_wiznames;
extern struct obj_data *object_list;
extern const char *pc_clsl_types[];

int destransformar(struct char_data *ch, int trans);
void send_to_char(const char *messg, struct char_data *ch);
void obj_to_char(struct obj_data * object, struct char_data * ch);
int find_skill_num(char *name);
void actualize_splskl(struct char_data *ch);
void stop_fighting(struct char_data * ch);
extern int race_attrib[][6];
void ganha_rp(struct char_data *ch);

/* local functions */
int parse_class(char arg);
long find_class_bitvector(char arg);
int saving_throws(int class_num, int type, int level);
int thaco(int class_num, int level);
void roll_real_abils(struct char_data * ch);
void do_start(struct char_data * ch);
int backstab_mult(int level);
int invalid_class(struct char_data *ch, struct obj_data *obj);
int level_exp(int remort, int level);
const char *title_male(int chclass, int level);
const char *title_female(int chclass, int level);
void    write_to_output(const char *txt, struct descriptor_data *d);
int allowed_classes(int x, int i);
#define SEND_TO_Q(messg, desc)  write_to_output((messg), desc)

/* Names first */

const char *class_abbrevs[] = {
  "&CSo&n",
  "&BCl&n",
  "&GTh&n",
  "&rWa&n",
  "&WNc&n",
  "&yPa&n",
  "&cNj&n",
  "&YRg&n",
  "&gPs&n",
  "&mWr&n",
  "&RBr&n",
  "&bSh&n",
  "\n"
};


const char *pc_class_types[] = {
  "Sorcerer",
  "Cleric",
  "Thief",
  "Warrior",
  "Necromancer",
  "Paladin",
  "Ninja",
  "Ranger",
  "Psionicist",
  "Warlock",
  "Barbarian",
  "Shaman",
  "\n"
};

#define STR	0
#define DEX	1
#define CON	2
#define INW	3
#define WIS	4
#define CHA	5

#define MAX_ATTRIB	6

int class_attrib[][MAX_ATTRIB] = {
//  STR DEX CON INT WIS CHA
    {-1, 0, 0,  2,  0,  2},//SOR
    { 0,-1, 1,  0,  1,  2},//CLE
    { 0, 2, 0,  0, -1,  2},//THI
    { 1, 1, 1, -1,  0,  0},//WAR
    { 0, 0, 0,  2,  1, -2},//NEC
    { 1, 0, 1, -1,  0 , 2},//PAL
    { 1, 2, 0, -1,  0 , 0},//NIN
    { 1, 1, 0, -1,  1,  0},//RAN
    { 0, 0,-1,  3,  0,  0},//PSI
    { 0, 0, 1,  1,  1, -2},//WARL
    { 1, 0, 2, -1,  0,  0},//BAR
    { 0,-1, 0,  1,  2,  0} //SHA
};

char *return_attrib_sign(int z)
{
   char sign[3];

	if(z > 0)
	   sprintf(sign, "+%d", z);
	else if(z < 0)
	   sprintf(sign, "%d", z);
	else
	   sprintf(sign, " %d", z);

   return (str_dup(sign));
}

char *attrib_line(int race)
{
   char line[MAX_INPUT_LENGTH];

   int j;

	*line = '\0';
	for(j = 0; j < MAX_ATTRIB; j++)
		sprintf(line+strlen(line), "   %s ", 
return_attrib_sign(class_attrib[race][j]));

   return (str_dup(line));
}

void display_classes(struct descriptor_data *d)
{
  int x;
  char tes[65536];

	strcpy(tes, "\r\n  &C[&cOption&C]  &WCLASS&n 		  STR   DEX   CON   INT   WIS   CHA\r\n");
	for(x = 0; x < NUM_CLASSES; x++)
          if(allowed_classes(GET_RACE(d->character), x)) {
		sprintf(tes+strlen(tes), "     %s        %s\r\n", pc_clsl_types[x], attrib_line(x));
	  }
        strcat(tes, "\r\n     [-]    Reselect Race\n\r");
	SEND_TO_Q(tes, d);
}

void display_stat(struct descriptor_data *d)
{
  char tes[65536];

  strcpy(tes, "Your stats:\r\n");
  sprintf(tes+strlen(tes), "Str: [&c%d&n/&c%d&n]  Dex: [&c%d&n]  Con: [&c%d&n]  Int: [&c%d&n]  Wis: [&c%d&n]  Cha: [&c%d&n]\r\n",
          GET_STR(d->character), GET_ADD(d->character), GET_DEX(d->character), GET_CON(d->character),
           GET_INT(d->character), GET_WIS(d->character),GET_CHA(d->character));

  SEND_TO_Q(tes, d);
}

void disp_abils(struct descriptor_data *d, int class)
{
  int i, j, c = 1;
  extern char *spells[];
  extern struct spell_info_type spell_info[];
  char tes[65536];

  strcpy(tes, "&WYou will learn:&n\r\n");
  for (j = 1; j < LVL_IMMORT; j++) {
    for (i = 1; i < MAX_SKILLS+1; i++) {
      if (spell_info[i].min_level[class] == j)
	  sprintf(tes+strlen(tes), "  &chow to &C%-24s&n   %s", spells[i], (!(c++ % 2) ? "&n\r\n" : "  "));
    }
  }

  if(c % 2)
    strcat(tes, "\r\n");
  else
    strcat(tes, "\r\n\r\n");

  SEND_TO_Q(tes, d);
}

/* The menu for choosing a class in interpreter.c: */
/*const char *class_menu =
"\r\n"
"Now you can choose you class, a class define your style of fight...\r\n"
"\r\n"
"  [a] Magic User	""  [b] Cleric		""  [c] Warrior		""\r\n"
"  [d] Thief		""  [e] Necromancer	""  [f] Paladin		""\r\n"
"  [g] Ninja		""  [h] Ranger		""  [i] Augurer		""\r\n"
"  [j] Warlock		""  [l] Barbarian		""  [m] Sorcerer	""\r\n"
"\r\n";
*/

/*
* The code to interpret a class letter -- used in interpreter.c when a
* new character is selecting a class and by 'set class' in act.wizard.c.
*/

int parse_class(char arg)
{
  arg = LOWER(arg);

  switch (arg) {
  case 'a': return CLASS_MAGIC_USER;
  case 'b': return CLASS_CLERIC;
  case 'c': return CLASS_THIEF;
  case 'd': return CLASS_WARRIOR;
  case 'e': return CLASS_NECROMANCER;
  case 'f': return CLASS_PALADIN;
  case 'g': return CLASS_NINJA;
  case 'h': return CLASS_RANGER;
  case 'i': return CLASS_PSIONICIST;
  case 'j': return CLASS_WARLOCK;
  case 'l': return CLASS_BARBARIAN;
  case 'm': return CLASS_SORCERER;
  default:  return CLASS_UNDEFINED;
  }
}

/*
* bitvectors (i.e., powers of two) for each class, mainly for use in
* do_who and do_users.  Add new classes at the end so that all classes
* use sequential powers of two (1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4,
* 1 << 5, etc.
*/

long find_class_bitvector(char arg)
{
  arg = LOWER(arg);

  switch (arg) {
    case 'a': return (1 << CLASS_MAGIC_USER);
    case 'b': return (1 << CLASS_CLERIC);
    case 'c': return (1 << CLASS_THIEF);
    case 'd': return (1 << CLASS_WARRIOR);
    case 'e': return (1 << CLASS_NECROMANCER);
    case 'f': return (1 << CLASS_PALADIN);
    case 'g': return (1 << CLASS_NINJA);
    case 'h': return (1 << CLASS_RANGER);
    case 'i': return (1 << CLASS_PSIONICIST);
    case 'j': return (1 << CLASS_WARLOCK);
    case 'l': return (1 << CLASS_BARBARIAN);
    case 'm': return (1 << CLASS_SORCERER);
    default:  return CLASS_UNDEFINED;
  }
}

/*
* Saving throws for:
* MCTW
*   PARA, ROD, PETRI, BREATH, SPELL
*     Levels 0-40
*
* Do not forget to change extern declaration in magic.c if you add to this.
*/

int saving_throws(int class_num, int type, int level)
{
  return (MAX((200-(level)/2),1)) ;
}

/* terminar */
/* THAC0 for classes and levels.  (To Hit Armor Class 0) */
int thaco(int class_num, int level)
{
    int thac;

    switch (class_num)
    {
	case CLASS_MAGIC_USER:
	    thac = (20 - level / 12.6);
	    break;
	case CLASS_CLERIC:
	    thac = (20 - (level / 12.3));
	    break;
	case CLASS_THIEF:
	    thac = (20 - (level / 11.1));
	    break;
	case CLASS_WARRIOR:
	    thac = (20 - (level / 10.5));
	    break;
	case CLASS_NECROMANCER:
	    thac = (20 - (level / 11.7));
	    break;
	case CLASS_PALADIN:
	    thac = (20 - (level / 10.8));
	    break;
	case CLASS_NINJA:
	    thac = (20 - (level / 10.8));
	    break;
	case CLASS_RANGER:
	    thac = (20 - (level / 11.1));
	    break;
	case CLASS_PSIONICIST:
	    thac = (20 - (level / 12));
	    break;
	case CLASS_WARLOCK:
	    thac = (20 - (level / 13.5));
	    break;
	case CLASS_BARBARIAN:
	    thac = (20 - (level / 9.9));
	    break;
	case CLASS_SORCERER:
	    thac = (20 - (level / 12.9));
	    break;
	default:
	    thac = 20;
    }

    return thac;
}


/*
* Roll the 6 stats for a character... each stat is made of the sum of
* the best 3 out of 4 rolls of a 6-sided die.  Each class then decides
* which priority will be given for the best to worst stats.
*/
void roll_real_abils(struct char_data * ch)
{
  int z;

  ch->real_abils.str_add = 0;
  ch->real_abils.luk = number(0, 15);
  GET_MAX_OXI(ch) = 100;

  z = GET_CLASS(ch);
  if(z >= 0) {
      ch->real_abils.str = class_attrib[z][0];
      ch->real_abils.dex = class_attrib[z][1];
      ch->real_abils.con = class_attrib[z][2];
      ch->real_abils.intel = class_attrib[z][3];
      ch->real_abils.wis = class_attrib[z][4];
      ch->real_abils.cha = class_attrib[z][5];
  }

  z = GET_RACE(ch);
  if(z >= 0) {
      ch->real_abils.str += race_attrib[z][0];
      ch->real_abils.dex += race_attrib[z][1];
      ch->real_abils.con += race_attrib[z][2];
      ch->real_abils.intel += race_attrib[z][3];
      ch->real_abils.wis += race_attrib[z][4];
      ch->real_abils.cha += race_attrib[z][5];
  }

  ch->aff_abils = ch->real_abils;
}


/* Some initializations for characters, including initial skills */
void do_start(struct char_data * ch)
{
  struct obj_data *obj;
  int ii;
  int give_obj[] = {2, 598, 508, 3076, 3081, 3086, 3043, 3021, 3042, 12008, 3010, 3010, 3002, -1 };

  if (siteok_everyone)
    SET_BIT(PLR_FLAGS(ch), PLR_SITEOK);

  SET_BIT(PRF_FLAGS(ch), PRF_AUTOEXIT);
  GET_OLC_ZONE(ch) = -1;
  GET_LEVEL(ch) = 1;
  GET_EXP(ch) = 1;
  GET_ALIGNMENT(ch) = 5 ;

  GET_GOLD(ch) = 3500;

  set_title(ch, NULL);
  set_prename(ch, NULL);
  roll_real_abils(ch);

  ch->points.max_hit = 100 ;
  ch->points.max_mana = 100 ;
  ch->points.max_move = 100 ;

  SET_BIT(PRF_FLAGS(ch), PRF_DISPHP);
  SET_BIT(PRF_FLAGS(ch), PRF_DISPMOVE);
  SET_BIT(PRF2_FLAGS(ch), PRF2_DISPEXP);
  SET_BIT(PLR_FLAGS(ch), PLR_NOPK);

  for(ii = 0; give_obj[ii] != -1; ii++) {
   obj = read_object(give_obj[ii], VIRTUAL);
    if (obj == NULL)
     continue;
    obj_to_char(obj, ch);
  }

  switch (GET_CLASS(ch)) {
   case CLASS_MAGIC_USER:
   case CLASS_CLERIC:
   case CLASS_NECROMANCER:
   case CLASS_PSIONICIST:
   case CLASS_WARLOCK:
   case CLASS_SORCERER:
   case CLASS_PALADIN:
     SET_BIT(PRF_FLAGS(ch), PRF_DISPMANA);
     break;
   case CLASS_THIEF:
     break;
   case CLASS_WARRIOR:
     break;
   case CLASS_NINJA:
     break;
   case CLASS_BARBARIAN:
     break;
   case CLASS_RANGER:
     break;

  }

  advance_level(ch);

  GET_HIT(ch) = GET_MAX_HIT(ch);
  GET_MANA(ch) = GET_MAX_MANA(ch);
  GET_MOVE(ch) = GET_MAX_MOVE(ch);
  GET_MENTAL(ch) = GET_MAX_MENTAL(ch);
  GET_OXI(ch) = GET_MAX_OXI(ch);

  GET_COND(ch, THIRST) = 24;
  GET_COND(ch, FULL) = 24;
  GET_COND(ch, DRUNK) = 0;

  ch->player.time.played = 0;
  ch->player.time.logon = time(0);
}



/*
* This function controls the change to maxmove, maxmana, and maxhp for
* each class every time they gain a level.
*/
void advance_level(struct char_data * ch)
{
  //int add_hp = 0, add_mana = 0, add_move = 0;
  int add_mental = 0;
  int i, x;
  //add_hp = con_app[GET_CON(ch)].hitp;

  if(AFF2_FLAGGED(ch, AFF2_TRANSFORM)) // evita bugs qdo se
   destransformar(ch, GET_TRANS(ch));  // passa de level com transform

switch (GET_CLASS(ch)) {
  case CLASS_MAGIC_USER:
    //add_hp += number(3, 8);
    //add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
    //add_mana = MIN(add_mana, 10);
    //add_move = number(0, 2);
    break;

  case CLASS_CLERIC:
    //add_hp += number(5, 10);
    //add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
    //add_mana = MIN(add_mana, 10);
    //add_move = number(0, 2);
    break;

  case CLASS_THIEF:
    //add_hp += number(9, 13);
    //add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
    //add_move = number(1, 3);
    break;

  case CLASS_WARRIOR:
    //add_hp += number(10, 15);
    //add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
    //add_move = number(1, 3);
    break;

  case CLASS_NECROMANCER:
    //add_hp += number(7, 12);
    //add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
    //add_mana = MIN(add_mana, 10);
    //add_move = number(0, 2);
    break;

  case CLASS_PALADIN:
    //add_hp += number(8, 12);
    //add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
    //add_mana = MIN(add_mana, 10);
    //add_move = number(0, 2);
    break;

  case CLASS_NINJA:
    //add_hp += number(12, 15);
    //add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
    //add_move = number(2, 5);
    break;

  case CLASS_RANGER:
    //add_hp += number(12, 15);
    //add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
    //add_move = number(2, 5);
    break;

  case CLASS_PSIONICIST:
    //add_hp += number(12, 15);
    //add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
    //add_move = number(2, 5);
    break;

  case CLASS_SORCERER:
    //add_hp += number(12, 15);
    //add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
    //add_move = number(2, 5);
    break;

  case CLASS_WARLOCK:
    //add_hp += number(12, 15);
    //add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
    //add_move = number(2, 5);
    break;

  case CLASS_BARBARIAN:
    //add_hp += number(12, 15);
    //add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
    //add_move = number(2, 5);
    break;
}

switch (GET_RACE(ch)) {
  case RACE_VAMPIRE:
    add_mental = number(7, 11);
    break;

  case RACE_DROW:
    add_mental = number(5, 8);
    break;

  case RACE_DWARF:
    add_mental = number(4, 9);
    break;

  case RACE_ELF:
    add_mental = number(8, 10);
    break;

  case RACE_OGRE:
    add_mental = number(3, 6);
    break;

  case RACE_ORC:
    add_mental = number(6, 9);
    break;

  case RACE_TROLL:
    add_mental = number(2, 4);
    break;

  case RACE_GITH:
    add_mental = number(7, 12);
    break;

  case RACE_GNOME:
    add_mental = number(8, 12);
    break;

  case RACE_LIZARDMAN:
    add_mental = number(10, 16);
    break;

  case RACE_SEA_ELF:
    add_mental = number(4, 8);
    break;

  case RACE_GORAK:
    add_mental = number(10, 20);
    break;

  default:
    add_mental = number(5, 20);
   break;

}
  //ch->points.max_hit += MAX(1, add_hp);
  //ch->points.max_move += MAX(1, add_move);
  if(GET_LEVEL(ch) < LVL_IMMORT)
   ch->points.max_mental += MAX(1, add_mental);

  GET_LVL_SUM(ch) += 1;

  GET_POINTS(ch) += (9+(GET_REMORT(ch)/2));
  GET_POINTS_S(ch) += (9+(GET_REMORT(ch)/2));

  actualize_splskl(ch);

  if (IS_MAGIC_USER(ch) || IS_CLERIC(ch) || IS_NECROMANCER(ch) || 
IS_PALADIN(ch) || IS_WARLOCK(ch) || IS_PSIONICIST(ch) || IS_SORCERER(ch))
    GET_PRACTICES(ch) += MAX(2, wis_app[GET_WIS(ch)].bonus);
  else
    GET_PRACTICES(ch) += MIN(2, MAX(1, wis_app[GET_WIS(ch)].bonus));

  if (GET_LEVEL(ch) >= LVL_ELDER) {
    for (i = 0; i < 3; i++)
      GET_COND(ch, i) = (char) -1;
    SET_BIT(PRF_FLAGS(ch), PRF_HOLYLIGHT);
  }

  if (GET_LEVEL(ch) == 10) {
    stop_fighting(ch);
    if (ch->in_room != NOWHERE){
      char_from_room(ch);
      char_to_room(ch, real_room(500));
      look_at_room(ch, 0);
    }
    sprintf(buf, "\r\n&WNow, &C%s&W, you need choose.\r\nDo you want be a &RPlayer killer&W? &RType: &W'PK'&W. &RDo not forget, you can not choose again...&n\r\n", GET_NAME(ch));
    send_to_char(buf, ch);
  }

  if (GET_LEVEL(ch) == LVL_IMMORT) {
    SET_BIT(PRF_FLAGS(ch), PRF_NOHASSLE);
    sprintf(buf, "Welcome to the immortality, %s!!\r\n", GET_NAME(ch));
    send_to_char(buf, ch);
    if (ch->in_room != NOWHERE)
      char_from_room(ch);
      char_to_room(ch, real_room(500));
      look_at_room(ch, 0);
  }


	if (GET_CLAN(ch) && GET_CLAN_POS(ch))
		if((x = posicao_clan(GET_CLAN(ch))) != -1)
		{
			clans[x].poder++;
			salvar_clan(GET_CLAN(ch));
		}


   GET_EXP(ch) = 0;
   ganha_rp(ch);
//  sprintf(buf, "%s advanced to level %d", GET_NAME(ch), GET_LEVEL(ch));
//  mudlog(buf, BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
}

/*
* This simply calculates the backstab multiplier based on a character's
* level.  This used to be an array, but was changed to be a function so
* that it would be easier to add more levels to your MUD.  This doesn't
* really create a big performance hit because it's not used very often.
*/
int backstab_mult(int level)
{
  if (level <= 0)
    return 1.4;
  else if (level <= 20)
    return 1.8;
  else if (level <= 50)
    return 2.2;
  else if (level <= 100)
    return 2.6;
  else if (level <= 150)
    return 3;
  else if (level <= 200)
    return 3.4;
  else if (level <= 250)
    return 3.8;
  else if (level <= 300)
    return 4.2;
  else if (level <= 350)
    return 4.6;
  else
    return 5;
}


/*
* invalid_class is used by handler.c to determine if a piece of equipment is
* usable by a particular class, based on the ITEM_ANTI_{class} bitvectors.
*/

int invalid_class(struct char_data *ch, struct obj_data *obj) {
  if (GET_LEVEL(ch) >= LVL_GOD ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_MAGIC_USER)  && IS_MAGIC_USER(ch) )  ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_CLERIC)      && IS_CLERIC(ch)     )  ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_NECROMANCER) && IS_NECROMANCER(ch))  ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_PALADIN)     && IS_PALADIN(ch)    )  ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_WARRIOR)     && IS_WARRIOR(ch)    )  ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_THIEF)       && IS_THIEF(ch)      )  ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_NINJA)       && IS_NINJA(ch)      )  ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_RANGER)      && IS_RANGER(ch)     )  ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_PSIONICIST)  && IS_PSIONICIST(ch) )  ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_WARLOCK)     && IS_WARLOCK(ch)    )  ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_SORCERER)    && IS_SORCERER(ch)   )  ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_BARBARIAN)   && IS_BARBARIAN(ch)) )

        return 1;
  else
        return 0;
}




/*
* SPELLS AND SKILLS.  This area defines which spells are assigned to
* which classes, and the minimum level the character must be to use
* the spell or skill.
*/
void init_spell_levels(void)
{
  /* SORCERER */
  spell_level(SPELL_ARMOR,            CLASS_MAGIC_USER,  1);
  spell_level(SKILL_VITALIZE_MANA,    CLASS_MAGIC_USER,  4);
  spell_level(SPELL_INFRAVISION,      CLASS_MAGIC_USER,  8);
  spell_level(SPELL_ACIDARROW,        CLASS_MAGIC_USER,  11);
  spell_level(SPELL_MINOR_GLOBE,      CLASS_MAGIC_USER,  13);
  spell_level(SPELL_FLAMEARROW,       CLASS_MAGIC_USER,  18);
  spell_level(SPELL_REMOVE_CURSE,     CLASS_MAGIC_USER,  19);
  spell_level(SPELL_DETECT_INVIS,     CLASS_MAGIC_USER,  24);
  spell_level(SKILL_MEDITATE      ,   CLASS_MAGIC_USER,  25);
  spell_level(SPELL_GREASE,           CLASS_MAGIC_USER,  26);
  spell_level(SPELL_DETECT_POISON,    CLASS_MAGIC_USER,  30);
  spell_level(SPELL_MAGIC_MISSILE,    CLASS_MAGIC_USER,  35);
  spell_level(SPELL_DETECT_MAGIC,     CLASS_MAGIC_USER,  37);
  spell_level(SPELL_REMOVE_POISON,    CLASS_MAGIC_USER,  43);
  spell_level(SPELL_CROMATIC_ORB,     CLASS_MAGIC_USER,  45);
  spell_level(SPELL_DETECT_ALIGN,     CLASS_MAGIC_USER,  50);
  spell_level(SPELL_STRENGTH,         CLASS_MAGIC_USER,  55);
  spell_level(SPELL_CHILL_TOUCH,      CLASS_MAGIC_USER,  56);
  spell_level(SPELL_IRON_SKIN,        CLASS_MAGIC_USER,  58);
  spell_level(SPELL_CURE_BLIND,       CLASS_MAGIC_USER,  67);
  spell_level(SPELL_AREA_LIGHTNING,   CLASS_MAGIC_USER,  68);
  spell_level(SPELL_INVISIBLE,        CLASS_MAGIC_USER,  70);
  spell_level(SPELL_LOCATE_OBJECT,    CLASS_MAGIC_USER,  77);
  spell_level(SPELL_SLEEP,            CLASS_MAGIC_USER,  78);
  spell_level(SPELL_BURNING_HANDS,    CLASS_MAGIC_USER,  81);
  spell_level(SPELL_MANA_SWORD,       CLASS_MAGIC_USER,  87);
  spell_level(SPELL_SHOCKING_GRASP,   CLASS_MAGIC_USER,  95);
  spell_level(SPELL_DUMBNESS,         CLASS_MAGIC_USER,  97);
  spell_level(SKILL_SILENT_SPELL,     CLASS_MAGIC_USER,  99);
  spell_level(SKILL_MEDITATE,         CLASS_MAGIC_USER,  100);
//spell_level(SPELL_GATE,             CLASS_MAGIC_USER,  102); /* Igual Wall of Force */
  spell_level(SKILL_SECOND_ATTACK,    CLASS_MAGIC_USER,  108);
  spell_level(SPELL_COLOR_SPRAY,      CLASS_MAGIC_USER,  110);
  spell_level(SPELL_PLATINUM_SKIN,    CLASS_MAGIC_USER,  113);
//spell_level(SPELL_BLAZE,            CLASS_MAGIC_USER,  117); /* As entradas da sala onde o Player se encontra fica sob chamas durante 5 segundos a partir do instante que ele entrou na sala. Resultado: outro player q entrar la nesse periodo sera atingido pelo fogo. */
  spell_level(SPELL_EXTEND_SPELLS,    CLASS_MAGIC_USER,  120);
  spell_level(SPELL_LIGHTNING_BOLT,   CLASS_MAGIC_USER,  126);
  spell_level(SPELL_BLINDNESS,        CLASS_MAGIC_USER,  127);
  spell_level(SPELL_FRIENDS,          CLASS_MAGIC_USER,  133);
  spell_level(SPELL_ENCHANT_WEAPON,   CLASS_MAGIC_USER,  138);
  spell_level(SPELL_MANA_SHIELD,      CLASS_MAGIC_USER,  140);
  spell_level(SPELL_ICEBOLT,          CLASS_MAGIC_USER,  142);
  spell_level(SPELL_ANTIMAGIC_SHELL,  CLASS_MAGIC_USER,  145);
  spell_level(SPELL_WALL_OF_FOG,      CLASS_MAGIC_USER,  147);
  spell_level(SPELL_CHARM,            CLASS_MAGIC_USER,  153);
  spell_level(SPELL_BLINK,            CLASS_MAGIC_USER,  155);
  spell_level(SPELL_FIREBOLT,         CLASS_MAGIC_USER,  158);
  spell_level(SPELL_DIAMOND_SKIN,     CLASS_MAGIC_USER,  164);
//spell_level(SPELL_SPIDER_WEB,       CLASS_MAGIC_USER,  169); /* Magia que nao causa dano nenhum ao oponente mas o impede de dar FLEE durante 1 tic. */
  spell_level(SPELL_DISPEL_MAGIC,     CLASS_MAGIC_USER,  169);
  spell_level(SPELL_SILENCE,          CLASS_MAGIC_USER,  171);
  spell_level(SPELL_PRISMATIC_SPHERE, CLASS_MAGIC_USER,  173);
  spell_level(SPELL_FLY,              CLASS_MAGIC_USER,  178);
//spell_level(SPELL_INVULNERABILITY   CLASS_MAGIC_USER,  181); /* Spell que impede outras criaturas de o atacarem durante 2 tics. */
  spell_level(SPELL_MAJOR_GLOBE,      CLASS_MAGIC_USER,  184);
  spell_level(SPELL_FIREBALL,         CLASS_MAGIC_USER,  187);
//spell_level(SPELL_WISH              CLASS_MAGIC_USER,  193); /* O player faz um desejo dentro de uma lista de dez que sera relacionado, como aumentar dam, hit, ac, dano da arma, str, etc. Ele podera usar 1 desejo de cada vez, ou seja, so podera usar outro depois q acabar o primeiro. */
//spell_level(SPELL_MAGIC_MIRROR,     CLASS_MAGIC_USER,  197); /* Essa magia deveria gastar muita MANA pois cria um espelho com uma imagem igual do player. Resultado: todas as SPELLS castadas pelo player serao duplicadas. Obs: So as magias serao duplicadas, os ataques armados nao serao pois e so um MIRROR. */

  /* NECROMANCERS */
  spell_level(SPELL_METEOR_SHOWER,     CLASS_NECROMANCER,   1);
  spell_level(SPELL_POISON,            CLASS_NECROMANCER,   4);
//  spell_level(SPELL_DETECT_GOOD,       CLASS_NECROMANCER,   6);
  spell_level(SPELL_CURSE,             CLASS_NECROMANCER,   8);
  spell_level(SPELL_PHANTOM_FLAME,     CLASS_NECROMANCER,  12);
  spell_level(SKILL_VITALIZE_MANA,     CLASS_NECROMANCER,  13);
  spell_level(SPELL_SIMULACRUM,        CLASS_NECROMANCER,  19);
  spell_level(SPELL_MINUTE_METEOR,     CLASS_NECROMANCER,  23);
  spell_level(SKILL_BLOOD_RITUAL,      CLASS_NECROMANCER,  26);
  spell_level(SPELL_BANSHEE_AURA,      CLASS_NECROMANCER,  30);
  spell_level(SPELL_SPECTRAL_WINGS,    CLASS_NECROMANCER,  34);
  spell_level(SPELL_DEATH_RIPPLE,      CLASS_NECROMANCER,  38);
  spell_level(SPELL_PHANTOM_ARMOR,     CLASS_NECROMANCER,  41);
  spell_level(SPELL_GOLEM,             CLASS_NECROMANCER,  49);
  spell_level(SPELL_CHILL_TOUCH,       CLASS_NECROMANCER,  55);
//spell_level(SPELL_REPULSION,         CLASS_NECROMANCER,  58); /* O player que usar essa magia cria uma repulsao na sala que faz com que criaturas de 10 ou mais levels para baixo do player nao consiga permanecer na sala por mais de 5 segundos. Resultado: a criatura e expelida para uma outra sala q nao seja DT. */
  spell_level(SPELL_ENERGY_DRAIN,      CLASS_NECROMANCER,  66);
  spell_level(SPELL_SKULL_TRAP,        CLASS_NECROMANCER,  71);
  spell_level(SKILL_MEDITATE,          CLASS_NECROMANCER,  75);
  spell_level(SPELL_UNDEAD_HORDE,      CLASS_NECROMANCER,  80);
//spell_level(SPELL_SPECTER,           CLASS_NECROMANCER,  79); /* Identico a spell GOLEM porem o SPECTER e um pouco mais forte. */
  spell_level(SPELL_GHASTLY_TOUCH,     CLASS_NECROMANCER,  86);
  spell_level(SKILL_MEDITATE,          CLASS_NECROMANCER,  90);
  spell_level(SPELL_TERROR,            CLASS_NECROMANCER,  91);
  spell_level(SPELL_DISPEL_GOOD,       CLASS_NECROMANCER,  99);
  spell_level(SPELL_ENGULFING_DARKNESS,CLASS_NECROMANCER,  100);
  spell_level(SKILL_SECOND_ATTACK,     CLASS_NECROMANCER,  104);
  spell_level(SPELL_CALL_NIGHT,        CLASS_NECROMANCER,  110);
  spell_level(SPELL_DEATH_WAVE,        CLASS_NECROMANCER,  113);
//spell_level(SPELL_SKELETON,          CLASS_NECROMANCER,  118); /* Identico a spell GOLEM porem o SKELETON e mais forte. */
  spell_level(SPELL_DAMNED_CURSE,      CLASS_NECROMANCER,  119);
//spell_level(SKILL_DEVILISH_RITUAL,   CLASS_NECROMANCER,  123); /* Essa SKILL so poderia ser usada por players com align EVIL. E funcionaria assim: o player sacrificaria um corpo e receberia em troca uma pequena quantia de HitPoints. Seria um ritual que o necromante faria com o corpo para entrega-lo ao SATAN. */
  spell_level(SPELL_DEATH_FINGER,      CLASS_NECROMANCER,  125);
  spell_level(SPELL_DEATH_SCYTHE,      CLASS_NECROMANCER,  133);
  spell_level(SPELL_SATAN_PACT,        CLASS_NECROMANCER,  136);
  spell_level(SPELL_VAMPIRIC_TOUCH,    CLASS_NECROMANCER,  139);
//spell_level(SPELL_DEMON,             CLASS_NECROMANCER,  149); /* Identico a spell GOLEM porem o DEMON e bem mais forte. */
//spell_level(SPELL_HOLD_UNDEAD,       CLASS_NECROMANCER,  150); /* Para essa SPELL funcionar deveremos criar uma class para mobs UNDEAD que abrangeria todos os seres mortos do mundo. Essa spell funcionaria como MENTAL RAGE mas so funcionaria com mobs UNDEAD. */
  spell_level(SPELL_VULCAN,            CLASS_NECROMANCER,  155);
  spell_level(SPELL_DEATHDANCE,        CLASS_NECROMANCER,  163);
//spell_level(SPELL_CHAOS,             CLASS_NECROMANCER,  166); /* Essa spell funciona ao contrario da spell PEACE. Ao fazer uso dessa magia o player faz com que as outras criaturas da sala com 10 ou mais levels de diferenca comecem a lutar umas com as outras. */
  spell_level(SPELL_ARMAGEDDOM,        CLASS_NECROMANCER,  170);
//spell_level(SPELL_CHARM_UNDEAD,      CLASS_NECROMANCER,  175); /* Como a spell HOLD UNDEAD essa magia dependeria da flag UNDEAD dos mobs para funcionar como CHARM. */
  spell_level(SPELL_SIPHON_LIFE,       CLASS_NECROMANCER,  178);
  spell_level(SPELL_ANIMATE_DEAD,      CLASS_NECROMANCER,  182);
  spell_level(SPELL_PESTILENCE,        CLASS_NECROMANCER,  185);
  spell_level(SPELL_END_WORLD,         CLASS_NECROMANCER,  191);
  spell_level(SPELL_RAISE_ABERRATION,  CLASS_NECROMANCER,  193);


  /* PALADINS */
//  spell_level(SPELL_DETECT_EVIL,      CLASS_PALADIN,  1);
  spell_level(SPELL_CURE_LIGHT,       CLASS_PALADIN,  3);
//spell_level(SPELL_CLEANSING,        CLASS_PALADIN,  6); /* Faz com que todos os objetos ou criaturas fiquem inertes a magias como POISON, CURSE, PESTILENCE, e outras durante 7 tics. */
  spell_level(SPELL_CREATE_WATER,     CLASS_PALADIN,  10);
  spell_level(SPELL_CREATE_FOOD,      CLASS_PALADIN,  12);
  spell_level(SKILL_KICK,             CLASS_PALADIN, 15);
  spell_level(SPELL_BLESS,            CLASS_PALADIN,  21);
//spell_level(SKILL_PRAY,             CLASS_PALADIN,  45); /* SKILL semelhante a MEDITATE mas que so poderia ser usada por align GOOD. */
  spell_level(SKILL_BLUDGEON,		  CLASS_PALADIN,  50);
  spell_level(SPELL_SACRIFICE,        CLASS_PALADIN,  51);
  spell_level(SKILL_SECOND_ATTACK,    CLASS_PALADIN,  53);
  spell_level(SPELL_CURE_CRITIC,      CLASS_PALADIN,  55);
//spell_level(SPELL_SALVATION,        CLASS_PALADIN,  57);
  spell_level(SKILL_RESCUE,           CLASS_PALADIN,  66);
//spell_level(SPELL_CONVERTION,       CLASS_PALADIN,  72); /* Spell que converteria inimigos com 10 ou mais levels de diferenca a se tornar aliados durante a batalha. */
  spell_level(SPELL_CONCENTRATION,    CLASS_PALADIN,  81);
  spell_level(SPELL_ARMOR,            CLASS_PALADIN,  91);
  spell_level(SPELL_REVITALIZE,       CLASS_PALADIN,  92);
  spell_level(SPELL_BLINDNESS,        CLASS_PALADIN,  95);
  spell_level(SKILL_STROKE,           CLASS_PALADIN,  97);
  spell_level(SPELL_DISPEL_EVIL,      CLASS_PALADIN,  99);
  spell_level(SPELL_PEACE,            CLASS_PALADIN,  103);
  spell_level(SPELL_CALL_DAYLIGHT,    CLASS_PALADIN,  110);
  spell_level(SPELL_CONVICTION,       CLASS_PALADIN,  112);
//  spell_level(SKILL_REDEMPTION,       CLASS_PALADIN,  115); /* SKILL semelhante a skill DEVILISH RITUAL de necromante. A diferenca e que so poderia ser utilizada por players com align GOOD. O funcionamento e identico: sacrificar o corpo para seu DEUS em troca de um pouco de HitPoints. */
  spell_level(SPELL_CHAMPION_STRENGTH,CLASS_PALADIN,  126);
//spell_level(SPELL_ICESHIELD,         CLASS_PALADIN,  128); /* Spell que tiraria um ataque do oponente. Ou seja se este tivesse 4 ataques teria 3. So nao funcionaria se o oponente tivesse apenas 1 ataque. */
//spell_level(SPELL_HOLY_CLUB          CLASS_PALADIN   133); /* Spell identica a MANA SWORD. Ou seja, criaria um CLUB sagrado para poder usar e gastaria um pouco de sua MANA. */
  spell_level(SPELL_GOD_PACT,         CLASS_PALADIN,  135);
  spell_level(SPELL_HOLY_MACE,        CLASS_PALADIN,  136);
  spell_level(SKILL_SHIELD_BLOCK,     CLASS_PALADIN,  139);
  spell_level(SKILL_THIRD_ATTACK,     CLASS_PALADIN,  140);
  spell_level(SPELL_HOLY_SHOUT,       CLASS_PALADIN,  145);
  spell_level(SPELL_HOLY_SHIELD,      CLASS_PALADIN,  153);
// spell_level(SPELL_SANCTUARY,        CLASS_PALADIN,  159);
  spell_level(SPELL_LAY_ON_HANDS,     CLASS_PALADIN,  166);
  spell_level(SPELL_HOLY_WORD,        CLASS_PALADIN,  171);
//spell_level(SPELL_HOLY_FREEZE,       CLASS_PALADIN,  177); /* Essa spell so poderia ser usada por align GOOD e congelaria todos os outros integrantes da sala que nao fossem GOOD. Obs: congelar e diferente de petrificar, ou seja, se depois de congelar 5 oponentes ele atacasse 1 deles, esse se descongelaria com a primeira pancada e lutaria normalmente. A vantagem nessa spell esta para matar bichos HELPER numa sala cheia de criaturas, por exemplo. */
//  spell_level(SKILL_COMBO,           CLASS_PALADIN,  180);
  spell_level(SPELL_HOLY_FURY,        CLASS_PALADIN,  184);
//  spell_level(SPELL_TURN_UNDEAD,     CLASS_PALADIN,  188);
  spell_level(SPELL_HOLY_BOLT,        CLASS_PALADIN,  190);

  /* CLERICS */
  spell_level(SPELL_CURE_LIGHT,       CLASS_CLERIC,   1);
  spell_level(SPELL_INFRAVISION,      CLASS_CLERIC,   2);
  spell_level(SPELL_DETECT_INVIS,     CLASS_CLERIC,   4);
  spell_level(SPELL_LIGHTNING_SPIT,   CLASS_CLERIC,   5);
  spell_level(SPELL_DETECT_POISON,    CLASS_CLERIC,   7);
  spell_level(SKILL_VITALIZE_MANA,    CLASS_CLERIC,   8);
  spell_level(SPELL_DETECT_MAGIC,     CLASS_CLERIC,  11);
  spell_level(SPELL_FROST_SPIT,       CLASS_CLERIC,  12);
  spell_level(SPELL_DETECT_ALIGN,     CLASS_CLERIC,  16);
  spell_level(SPELL_FIRE_SPIT,        CLASS_CLERIC,  19);
  spell_level(SPELL_CREATE_WATER,     CLASS_CLERIC,  22);
  spell_level(SKILL_MEDITATE      ,   CLASS_CLERIC,  24);
  spell_level(SPELL_GAS_SPIT,         CLASS_CLERIC,  26);
  spell_level(SPELL_CREATE_FOOD,      CLASS_CLERIC,  29);
  spell_level(SPELL_ARMOR,            CLASS_CLERIC,  30);
  spell_level(SPELL_ACID_SPIT,        CLASS_CLERIC,  33);
  spell_level(SPELL_BLESS,            CLASS_CLERIC,  36);
  spell_level(SPELL_SENSE_LIFE,       CLASS_CLERIC,  37);
  spell_level(SPELL_REMOVE_POISON,    CLASS_CLERIC,  40);
  spell_level(SPELL_CURE_CRITIC,      CLASS_CLERIC,  43);
  spell_level(SPELL_BLADEBARRIER,     CLASS_CLERIC,  44);
  spell_level(SPELL_REMOVE_CURSE,     CLASS_CLERIC,  47);
  spell_level(SPELL_IDENTIFY,         CLASS_CLERIC,  48);
  spell_level(SPELL_CALL_LIGHTNING,   CLASS_CLERIC,  55);
  spell_level(SPELL_WATERWALK,        CLASS_CLERIC,  58);
  spell_level(SPELL_WORD_OF_RECALL,   CLASS_CLERIC,  66);
  spell_level(SPELL_CURE_BLIND,       CLASS_CLERIC,  69);
//spell_level(SPELL_ICE_TONGUE,       CLASS_CLERIC,  72); /* Funciona como ENCHANT WEAPON mas tem duracao de 3 tics apenas e da 3 de HITROLL a arma. */
  spell_level(SKILL_SECOND_ATTACK,    CLASS_CLERIC,  75);
  spell_level(SPELL_EARTHQUAKE,       CLASS_CLERIC,  77);
  spell_level(SPELL_CONTROL_WEATHER,  CLASS_CLERIC,  81);
  spell_level(SPELL_CURE_SERIOUS,     CLASS_CLERIC,  85);
  spell_level(SPELL_BLINDNESS,        CLASS_CLERIC,  88);
//spell_level(SPELL_FIRE_TONGUE,      CLASS_CLERIC,  96); /* Funciona como ENCHANT WEAPON mas tem duracao de 3 tics apenas e da 3 de DAMROLL a arma. */
//spell_level(SPELL_REMOVE_FEAR,      CLASS_CLERIC,  102); /* Deixa a criatura sem sofrer consequencias das magias TERROR, AVERSION ou CONFUSION. Alem disso livra a criatura se ela ja tiver com algumas dessas affections. */
  spell_level(SPELL_HEAL,             CLASS_CLERIC,  100);
  spell_level(SPELL_FEAR,             CLASS_CLERIC,  107);
  spell_level(SPELL_STORM_SKY,        CLASS_CLERIC,  110);
  spell_level(SPELL_SANCTUARY,        CLASS_CLERIC,  115);
  spell_level(SPELL_DISPEL_EVIL,      CLASS_CLERIC,  120);
  spell_level(SPELL_DISPEL_GOOD,      CLASS_CLERIC,  120);
  spell_level(SPELL_FLESH_ARMOR,      CLASS_CLERIC,  123);
  spell_level(SPELL_SHILLELAGH,       CLASS_CLERIC,  130);
  spell_level(SPELL_REFRESH,          CLASS_CLERIC,  135);
  spell_level(SPELL_GOD_PACT,         CLASS_CLERIC,  136);
// spell_level(SKILL_THIRD_ATTACK,     CLASS_CLERIC, 145);
  spell_level(SPELL_MAGICAL_STONE,    CLASS_CLERIC,  146);
//spell_level(SPELL_INVERT_ALIGN,     CLASS_CLERIC,  147); /* Inverte o align do player */
//spell_level(SKILL_WATER_BREATHING,  CLASS_CLERIC,  151); /* Seria uma SKILL que funcionaria como SWIM, ja que o player poderia respirar dentro da agua. */
  spell_level(SPELL_GREATER_REFRESH,  CLASS_CLERIC,  150);
  spell_level(SPELL_ENTANGLE,         CLASS_CLERIC,  155);
  spell_level(SPELL_HASTE,            CLASS_CLERIC,  157);
  spell_level(SPELL_COMMAND,          CLASS_CLERIC,  162);
  spell_level(SPELL_SUMMON,           CLASS_CLERIC,  166);
  spell_level(SPELL_REGENERATION,     CLASS_CLERIC,  169);
// spell_level(SPELL_FAITHEYES,	      CLASS_CLERIC,  170);
//spell_level(SPELL_REJUVENATION,     CLASS_CLERIC,  177); /* Diminui em 20 a idade do player. */
  spell_level(SPELL_HARM,             CLASS_CLERIC,  184);
//spell_level(SPELL_GOOD_BERRIES,     CLASS_CLERIC,  194); /* Cria pequenas pocoes que gastam bastante MANA e recuperam pouco HitPoints. A vantagem e que pode ser carregada e nao pesa muito. */
  spell_level(SPELL_HOLD_PERSON,      CLASS_CLERIC,  198);

  /* THIEVES */
  spell_level(SKILL_SNEAK,            CLASS_THIEF,   1);
  spell_level(SKILL_PICK_LOCK,        CLASS_THIEF,   4);
//spell_level(SKILL_SENSE_DANGER,     CLASS_THIEF,   8); /* Arte de detectar se ha uma DT ao redor da sala onde ele se encontra. */
  spell_level(SKILL_SWIM,             CLASS_THIEF,  11);
  spell_level(SKILL_STEAL,            CLASS_THIEF,  13);
  spell_level(SKILL_PEEK,             CLASS_THIEF,  15);
//spell_level(SKILL_FORAGE,           CLASS_THIEF,  17); /* Procurar comida. */
  spell_level(SKILL_HIDE,             CLASS_THIEF,  19);
  spell_level(SKILL_KNOCKOUT,         CLASS_THIEF,  24);
  spell_level(SKILL_BACKSTAB,         CLASS_THIEF,  26);
  spell_level(SKILL_TRACK,            CLASS_THIEF,  32);
  spell_level(SKILL_LISTEN,           CLASS_THIEF,  34);
//spell_level(SKILL_INCREASE_SPEED,   CLASS_THIEF,  43); /* Ao utilizar essa SKILL o player recupera mais rapido os MovePoints */
  spell_level(SKILL_ESCAPE,           CLASS_THIEF,  51);
//spell_level(SKILL_SLIT,             CLASS_THIEF,  55); /* Funcionaria como a skill KICK. Slit significa uma cutilada, ou seja, um BACKSTAB bem mais fraco. A vantagem e que enquanto o player nao tem CIRCLE AROUND ele pode usar o artificio SLIT no meio da luta. */
//spell_level(SKILL_POISON_JAVELIN,   CLASS_THIEF,  62); /* Infectaria um equipamento com uma pocao que duraria 10 tics e tiraria -5 dam -5 hit e + 30 AC. Para fazer uso disso o thief precisaria estar com align EVIL. */
  spell_level(SKILL_SECOND_ATTACK,    CLASS_THIEF,  74);
  spell_level(SKILL_DISARM,           CLASS_THIEF,  82);
  spell_level(SKILL_SCAN,             CLASS_THIEF,  87);
  spell_level(SKILL_MAKE_TRAPS,       CLASS_THIEF,  99);
//spell_level(SKILL_POLISH,           CLASS_THIEF,  97); /* Arte de polir algum equipamento com a finalidade de ele demorar mais tempo para se danificar. */
//spell_level(SKILL_STEALTH,          CLASS_THIEF,  104); /* Faz com que o player use SNEAK e HIDE ao mesmo tempo. Ou seja: permaneca HIDE mesmo quando ande. */
//spell_level(SKILL_KILLING_BLOW,     CLASS_THIEF,  111); /* Identico a CRITICAL ATTACK com uso automatico. */
  spell_level(SKILL_PIERCE_E,         CLASS_THIEF,  110);
  spell_level(SKILL_CIRCLE_ARROUND,   CLASS_THIEF,  116);
  spell_level(SKILL_THROW,            CLASS_THIEF,  124);
  spell_level(SKILL_FIRST_AID,        CLASS_THIEF,  132);
//spell_level(SKILL_IMP_PICK_LOCK,    CLASS_THIEF,  136); /* Pick Lock que funcionaria em bem maior numero de fechaduras. */
  spell_level(SKILL_THIRD_ATTACK,     CLASS_THIEF,  145);
  spell_level(SKILL_DROWN,            CLASS_THIEF,  147);
//spell_level(SKILL_TRICKERY,         CLASS_THIEF,  149); /* O thief usaria todas sua desonestidade e aumentaria em +3 sua DEX. Mas para conseguir trapacear dessa forma ele teria que estar com o align EVIL. */
  spell_level(SKILL_TUMBLE,           CLASS_THIEF,  155);
//spell_level(SKILL_FIND_TRAPS,       CLASS_THIEF,  158); /* Procurar ARMADILHAS feitas por outros players. Veja mais em ARM TRAPS. */
//spell_level(SKILL_DISARM_TRAPS,     CLASS_THIEF,  161); /* Desarmas ARMADILHAS feitas por ele mesmo ou por outros players. Veja mais em ARM TRAPS. */
//spell_level(SKILL_ARM_TRAPS,        CLASS_THIEF,  164); /* Armar ARMADILHAS. Funcionaria assim: cada player com essa skill teria direito a fazer uma unica armadilha, que ele deixaria em qualquer sala. Todas as criaturas que por la passassem sofreriam um dano se passassem por la rapido ou ficariam presos se ficassem mais de 5 segundos naquela sala. Como foi dito, cada player tem direito a uma unica armadilha, ou seja, se quiser colocar em outro lugar tera que desarmar aquela que fez anteriormente. Isso nao quer dizer que se desarmar de um outro player vai ter 2. Todos tem no maximo 1. */
//spell_level(SKILL_VENGEANCE,        CLASS_THIEF,  169); /* O thief para usar essa skill teria que estar com align EVIL. Para usar ele teria que digitar VENGEANCE (nome de alguma criatyra) no maximo 1 tic depois de lutar com essa criatura. Essa skill duraria 12 tics e se esse player voltasse a lutar contra aquele oponente novamente nesse periodo teria a forca de todos os seus golpes multiplicada por 1,5. */
  spell_level(SKILL_CHOP,             CLASS_THIEF,  173);
//spell_level(SKILL_PLAGUE_JAVELIN,   CLASS_THIEF,  177); /* Igual ao POISON JAVELIN. Ou seja, so podendo ser usado por align EVIL, so q o player infectaria um equipamente que duraria 24 tics e causaria -12 DAM, -12 HIT e + 60 AC em quem a usasse. */
  spell_level(SKILL_SECOND_STAB,      CLASS_THIEF,  186);
//spell_level(SKILL_FEND,             CLASS_THIEF,  195); /* Igual ao COMBO mas acertaria um golpe em cada outro oponente que estivesse na sala e bem mais fraco. */

  /* WARRIORS */
  spell_level(SKILL_SWIM,             CLASS_WARRIOR,   1);
  spell_level(SKILL_KICK,             CLASS_WARRIOR,   5);
//spell_level(SKILL_COMBAT_THEORY,    CLASS_WARRIOR,  11); /* Faria com que o player tivesse a skill TANK e nao deixasse os mobs HELPER que ajudassem o oponente atacar seus aliados. */
//spell_level(SKILL_MIGHT,            CLASS_WARRIOR,  18); /* Skill que sacrificaria 1 ponto de cada um dos atributos (WIS, CHA, CON, DEX e INT) que seriam convertidos em +5 em STR durante 2 tics. */
  spell_level(SKILL_TRACK,            CLASS_WARRIOR,  22);
  spell_level(SKILL_KNOCKOUT,         CLASS_WARRIOR,  30);
  spell_level(SKILL_RESCUE,           CLASS_WARRIOR,  31);
//spell_level(SKILL_BATTLECRY,        CLASS_WARRIOR,  39); /* Grito que tiraria certa quantidade de MovePoints e seria convertido em +2 de Hitroll ao player. */
//spell_level(SKILL_WARCRY,           CLASS_WARRIOR,  47); /* Grito que tiraria certa quantidade de MovePoints e seria convertido em +2 de Damroll ao player. */
//spell_level(SKILL_VIGOR,            CLASS_WARRIOR,  52); /* Skill que aumentaria o AC em +30 e aumentaria a CON em +2. */
  spell_level(SKILL_SECOND_ATTACK,    CLASS_WARRIOR,  45);
  spell_level(SKILL_SLASH,            CLASS_WARRIOR,  50);
  spell_level(SKILL_WHIRLWIND,        CLASS_WARRIOR,  60);
  spell_level(SKILL_STYLE_AIMING,     CLASS_WARRIOR,  70);
//spell_level(SKILL_SHOUT,            CLASS_WARRIOR,  79); /* Grito que tiraria certa quantidade de MovePoints e seria convertido em -30 de AC. */
//spell_level(SKILL_SMITE,            CLASS_WARRIOR,  80); /* Funcionaria igual KICK mas seria um pouco mais forte. SMITE significa uma pancada dada com o escudo, por isso e necessario q o player tenha. */
  spell_level(SKILL_THIRD_ATTACK,     CLASS_WARRIOR,  95);
  spell_level(SKILL_BASH,             CLASS_WARRIOR,  97);
  spell_level(SKILL_GUT,              CLASS_WARRIOR,  100);
//spell_level(SKILL_IMPALE,           CLASS_WARRIOR,  117); /* Funciona quase igual ao DISARM, mas em vez de desarmar, o player desgasta a arma do oponente. */
//spell_level(SKILL_CONCENTRATE,      CLASS_WARRIOR,  127); /* Daria +5 ao Hitroll ao player. Poderia ter alguma outra skill que tirasse a concentracao de um player. */
  spell_level(SKILL_SCAN,             CLASS_WARRIOR,  133);
  spell_level(SKILL_CRAFT,  	      CLASS_WARRIOR,  135);
  spell_level(SKILL_THROW,            CLASS_WARRIOR,  140);
  spell_level(SKILL_PARRY,            CLASS_WARRIOR,  149);
  spell_level(SKILL_COMBO,            CLASS_WARRIOR,  150);
  spell_level(SKILL_DROWN,            CLASS_WARRIOR,  155);
  spell_level(SKILL_DUAL_WIELD,       CLASS_WARRIOR,  155);
  spell_level(SKILL_CHOP,             CLASS_WARRIOR,  172);
//spell_level(SKILL_CHARGE,           CLASS_WARRIOR,  184); /* So poderia ser usado no inicio da luta. Teria um dano muito alto, como um ataque multiplicado por 4, ja q o player vem correndo para acertar o oponente. */

  /* NINJA */
  spell_level(SKILL_HAND_DAMAGE,      CLASS_NINJA,  1);
//spell_level(SKILL_DARKFIGHT,        CLASS_NINJA,  4); /* Lutar no escuro, como se estivesse claro. */
//spell_level(SKILL_BLINDFIGHT,       CLASS_NINJA,  9); /* Lutar cego, como se estivesse normal. */
  spell_level(SKILL_KICK,             CLASS_NINJA,  11);
  spell_level(SKILL_SNEAK,            CLASS_NINJA,  23);
  spell_level(SKILL_KNOCKOUT,         CLASS_NINJA,  28);
//spell_level(SKILL_SLACKEN,          CLASS_NINJA,  31); /* Meditacao ninja, que seria como MEDITATE mas recuperaria HitPoints em vez de ManaPoints mais rapidamente. */
  spell_level(SKILL_SHURIKEN,         CLASS_NINJA,  38); // Atirar estrelinhas, - Ips
  spell_level(SKILL_KICKFLIP,         CLASS_NINJA,  45);
  spell_level(SKILL_SUICIDE,          CLASS_NINJA,  50);
  spell_level(SKILL_LISTEN,           CLASS_NINJA,  55);
  spell_level(SKILL_SECOND_ATTACK,    CLASS_NINJA,  61);
  spell_level(SKILL_STYLE_PRO,        CLASS_NINJA,  65);
//spell_level(SKILL_LICK,             CLASS_NINJA,  62); /* Lick e uma pancada ninja. Funcionaria como o KICK mas o dano seria um pouco mais forte. */
//spell_level(SKILL_INVISIBILITY,     CLASS_NINJA,  66); /* SKILL que funcionaria como a Spell INVISIBLE. */
  spell_level(SKILL_DISARM,           CLASS_NINJA,  73);
  spell_level(SKILL_TRACK,            CLASS_NINJA,  78);
//spell_level(SKILL_MENTAL_EXERC,     CLASS_NINJA,  79); /* Aumentaria seu INT +2 durante 2 tics. */
  spell_level(SKILL_PICK_LOCK,        CLASS_NINJA,  91);
//spell_level(SKILL_SPURT,            CLASS_NINJA,  96); /* Lick e um arremesso ninja. O ideal e q seja dificil de ser efetuado, mas quando consegue deixa o oponente SITTING durante 1 turno e o ataque e um pouco mais forte pois o oponente esta tonto devido a queda no arremesso. */
  spell_level(SKILL_WHITE_GAS,        CLASS_NINJA,  100);
  spell_level(SKILL_SLASH_E,          CLASS_NINJA,  110);
//spell_level(SKILL_SHARPEN,          CLASS_NINJA,  105); /* Seria como o ENCHANT WEAPON mas daria +8 dam e +8 hit e duraria 1 tic so. SHARPEN significa afiar uma espada, portanto so funcionaria em armas SLASH. */
//spell_level(SKILL_CONCENTRATE,      CLASS_NINJA,  113); /* Daria +5 ao Hitroll ao player. Poderia ter alguma outra skill que tirasse a concentracao de um player. */
  spell_level(SKILL_DUAL_WIELD,       CLASS_NINJA,  115);
  spell_level(SKILL_THROW,            CLASS_NINJA,  130);
//spell_level(SKILL_IMPALE,           CLASS_NINJA,  136); /* Funciona quase igual ao DISARM, mas em vez de desarmar, o player desgasta a arma do oponente. */
  spell_level(SKILL_DROWN,            CLASS_NINJA,  147);
  spell_level(SKILL_ESCAPE,           CLASS_NINJA,  148);
//spell_level(SKILL_IMPROVED_SLASH,   CLASS_NINJA,  153); /* Maior poder ofensivo com armas SLASH. */
//spell_level(SKILL_IMMOBILIZATION,   CLASS_NINJA,  164); /* Imobilizacao ninja. Faria com que o player imobilizasse o oponente que fosse 10 ou mais levels abaixo do seu. Mas, para imobilizar seu oponente o player tb fica parado. Ou seja os 2 ficam sem se movimentar. A vantagem e que um outro player pode atacar esse oponente. Deveria ter duracao de alguns turnos somente e ser muito dificil de ser concretizado. */
  spell_level(SKILL_AGILITY,          CLASS_NINJA,  166);
  spell_level(SKILL_SABRE,            CLASS_NINJA,  181);
  spell_level(SKILL_THIRD_ATTACK,     CLASS_NINJA,  185);
//spell_level(SKILL_MORTAL_DASH,      CLASS_NINJA,  190); /* Golpe Mortal. Um golpe dado que mataria o player instantaneamente. Mas para ser utilizado o oponente deve estar com menos de 30% do seu HitPoint total e ser 10 ou mais levels abaixo. */
//spell_level(SKILL_NJ_MOTIONS,       CLASS_NINJA,  198); /* Ninjitsu Motions. O ninja usaria isso automaticamente e ficaria muito mais agil de acordo com o peso que carrega. Isso faria com que acertasse mais golpes e mais fortes, desviasse mais e mais dificil de tomar danos de magis. */

   /* RANGER */
  spell_level(SKILL_SWIM,             CLASS_RANGER,  1);
  spell_level(SPELL_INFRAVISION,      CLASS_RANGER,  4);
//spell_level(SKILL_FORAGE,           CLASS_RANGER,  8); /* Procurar comida. */
//spell_level(SKILL_ANIMAL_AFFINITY,  CLASS_RANGER, 11); /* Como a flag UNDEAD que citei na classe necromante, deveria existir uma flag pra mobs ANIMAL. Essa skill faria com que o Ranger nao fosse atacado por animais de nenhuma forma, ao menos que ele atacasse. */
  spell_level(SKILL_KICK,             CLASS_RANGER, 12);
  spell_level(SKILL_TRACK,            CLASS_RANGER, 15);
  spell_level(SPELL_BARKSKIN,         CLASS_RANGER, 18);
//spell_level(SKILL_CAMP,             CLASS_RANGER, 23); /* Arte de acampar. Ao digitar CAMP o ranger faz com que a sala fique com regeneracao quase igual de temple e pode descansar ali. Porem, isso nao deixa a sala PEACE e ele pode ser atacado por outras criaturas. */
  spell_level(SPELL_DETECT_INVIS,     CLASS_RANGER, 23);
  spell_level(SPELL_ACIDARROW,        CLASS_RANGER, 25);
  spell_level(SPELL_ARMOR,            CLASS_RANGER, 30);
  spell_level(SKILL_SNAKE_FIGHT,      CLASS_RANGER, 35);
  spell_level(SKILL_FOREST_AGILITY,   CLASS_RANGER, 38); // ips -- 20 AC
//spell_level(SKILL_BOWSHOT,          CLASS_RANGER, 35); /* Conseguir atingir uma criatura a uma sala ao lado. */
//spell_level(SKILL_DECOY,            CLASS_RANGER, 43); /* Provocar o oponente com a finalidade de ele lutar com toda a sua furia (mais forte) mas que o impede de dar FLEE. */
//spell_level(SKILL_NATURAL_RESIST,   CLASS_RANGER, 47); /* Fazer com que em salas com flag de FOREST, FIELD, e outras que nao sejam desconhecidas por um ranger, faca com que o player gaste menos MovePoints q o normal e se recupere mais rapidamente. */
//spell_level(SKILL_NATURE_FORCE,     CLASS_RANGER, 53); /* Comando que faz com que o ranger ganhe +2 em CON se ele estiver na FOREST. Dura 2 tics se ele sair desse ambiente. */
  spell_level(SKILL_FOREST_WALK,      CLASS_RANGER, 44); // ips -- Nao perde move
  spell_level(SPELL_ILLUSION_FOREST,  CLASS_RANGER, 48);
  spell_level(SKILL_PIERCE,           CLASS_RANGER, 50);
  spell_level(SKILL_HIDE,             CLASS_RANGER, 52);
  spell_level(SPELL_FLAMEARROW,       CLASS_RANGER, 59);
  spell_level(SPELL_GROW_ROOTS,       CLASS_RANGER, 66); // ips -- Prende uma pessoa no chao
  spell_level(SKILL_DUAL_WIELD,       CLASS_RANGER, 68);
//spell_level(SPELL_HOLD_ANIMAL,      CLASS_RANGER, 71); /* Funcionaria como MENTAL RAGE, mas so os mobs com flag ANIMAL poderiam ser holded. */
  spell_level(SPELL_FOREST_SANCTUARY, CLASS_RANGER, 72);
  spell_level(SKILL_SECOND_ATTACK,    CLASS_RANGER, 76);
  spell_level(SKILL_WHIRLWIND,        CLASS_RANGER, 78);
//spell_level(SKILL_SHOUT,            CLASS_RANGER, 83); /* Grito que tiraria certa quantidade de MovePoints e seria convertido em -30 de AC. */
  spell_level(SPELL_INVISIBLE,        CLASS_RANGER, 89);
  spell_level(SPELL_VOLCANO,          CLASS_RANGER, 93);
//spell_level(SKILL_ICREASE_STAMINA,  CLASS_RANGER, 95); /* Faz com que o player recupere mais rapido HitPoints */
//spell_level(SKILL_BANDAGE           CLASS_RANGER, 100); /* Fazer bandagens, ou seja, curar ferimentos. Basta trocar move points por hit points. */
//spell_level(SPELL_CHARM_ANIMAL,     CLASS_RANGER, 107); /* Funcionaria como CHARM PERSON, mas so os mobs com flag ANIMAL poderiam sofrer essa magia. */
  spell_level(SKILL_THROW,            CLASS_RANGER, 109);
//spell_level(SPELL_FLAME_BLADE,      CLASS_RANGER, 118); /* Uso identico a MANA SWORD mas o player criaria uma BLADE em chamas. */
//spell_level(SKILL_SCENT,            CLASS_RANGER, 119); /* Uso parecido com TRACK. A diferenca e que so funcionaria se a criatura estivesse na mesma ZONE e nesse caso funcionaria tb para mobs !TRACK. */
  spell_level(SKILL_FOREST_WILDERNESS,CLASS_RANGER, 113); // ips -- dano = dano * 1.2;
  spell_level(SPELL_CALL_BEAR,        CLASS_RANGER, 120);
  spell_level(SPELL_GEYSER,           CLASS_RANGER, 124);
  spell_level(SKILL_THIRD_ATTACK,     CLASS_RANGER, 125);
  spell_level(SKILL_FIRST_AID,        CLASS_RANGER, 131);
  spell_level(SKILL_DROWN,            CLASS_RANGER, 140);
  spell_level(SKILL_BREW,	      CLASS_RANGER, 145);
  spell_level(SPELL_SHILLELAGH,       CLASS_RANGER, 155);
  spell_level(SPELL_FIRESHIELD,       CLASS_RANGER, 167);
  spell_level(SPELL_HASTE,            CLASS_RANGER, 169);
//spell_level(SKILL_FEND,             CLASS_RANGER, 171); /* Parecido ao COMBO mas acertaria um golpe em cada outro oponente que estivesse na sala e bem mais fraco. */
  spell_level(SPELL_ENTANGLE,         CLASS_RANGER, 179);
  spell_level(SKILL_SABRE,            CLASS_RANGER, 185);
//spell_level(SKILL_ZEAL,             CLASS_RANGER, 187); /* Identico ao COMBO, mas so funcionaria se o player estivesse com align GOOD. */
//spell_level(SKILL_METAMORPHOSIS,    CLASS_RANGER, 191); /* O player ganharia metamorfose e poderia se transformar em alguns animais como WOLF, TIGER ou BEAR, cada um com alguns beneficios. Ele so conseguiria fazer isso se estivesse GOOD e recebesse a graca divina para fazer isso. */

  /* BARBARIAN */
  spell_level(SKILL_HAND_DAMAGE,      CLASS_BARBARIAN, 1);
  spell_level(SKILL_VITALIZE_HEALTH,  CLASS_BARBARIAN, 4);
  spell_level(SKILL_SWIM,             CLASS_BARBARIAN, 13);
  spell_level(SKILL_KICK,             CLASS_BARBARIAN, 15);
//spell_level(SKILL_INCREASE_SPEED,   CLASS_BARBARIAN, 19); /* Ao utilizar essa SKILL o player recupera mais rapido os MovePoints */
  spell_level(SKILL_HOWL,             CLASS_BARBARIAN, 22); // Faz o mob fugir. -ips
//spell_level(SKILL_SLANG,            CLASS_BARBARIAN, 35); /* Slang e um golpe baixo. Funciona igual ao KICK mas tira um pouco mais de dano. */
  spell_level(SKILL_SECOND_ATTACK,    CLASS_BARBARIAN, 53);
  spell_level(SKILL_WHIRLWIND,        CLASS_BARBARIAN, 55);
  spell_level(SKILL_POWER_KICK,       CLASS_BARBARIAN, 60);
//spell_level(SKILL_WARORDER,         CLASS_BARBARIAN, 64); /* Funciona igual ao WARSHOUT mas da +5 de HIT ao grupo. */
  spell_level(SKILL_STYLE_SAVAGE,     CLASS_BARBARIAN, 70);
  spell_level(SKILL_CRITICAL_ATTACK,  CLASS_BARBARIAN, 75);
//spell_level(SKILL_NAPEBASH,         CLASS_BARBARIAN, 75); /* Igual ao HEADBASH mas funcionaria de 4 em 4 turnos apenas e nao daria o ataque mais forte qdo o oponente estivesse caido. */
//spell_level(SKILL_FURY_ATTACK,      CLASS_BARBARIAN, 84); /* Igual ao CRITICAL ATTACK. */
  spell_level(SKILL_WARSHOUT,         CLASS_BARBARIAN, 91);
//spell_level(SKILL_FRENZY,           CLASS_BARBARIAN, 95); /* Funcionaria igual ao KICK, mas seria uma porrada dada um pouco mais forte com a base da arma. Teria o valor 1,5 de uma pancada normal. */
  spell_level(SKILL_THIRD_ATTACK,     CLASS_BARBARIAN, 105);
  spell_level(SKILL_FIRST_AID,        CLASS_BARBARIAN, 108);
  spell_level(SKILL_BLUDGEON_E,       CLASS_BARBARIAN, 110);
//spell_level(SKILL_DOUBLE_SWING,     CLASS_BARBARIAN, 115); /* Funcionaria igual ao KICK, mas o player daria 2 pancadas equivalente a 1 ataque normal. O primeiro acertaria o oponente com o qual esta lutando e o segundo um outro oponente que o esteja atacando tb. */
  //spell_level(SKILL_TAUNT,            CLASS_BARBARIAN, 117); /* Desafiar o oponente a uma luta de vida ou morte sem que nenhum dos lutadores possa dar FLEE. */
//spell_level(SKILL_GRIM_WARD,        CLASS_BARBARIAN, 127); /* Grito dado em direcao a um corpo que causa panico nas outras criaturas da sala. */
  spell_level(SKILL_HEADBASH,         CLASS_BARBARIAN, 135);
//spell_level(SKILL_INCREASE_STAMINA, CLASS_BARBARIAN, 136); /* Faz com que o player recupere mais rapido HitPoints */
//spell_level(SKILL_FORCE_LOCK,       CLASS_BARBARIAN, 144); /* Igual Pick Lock sem Improved. */
  spell_level(SKILL_DODGE,            CLASS_BARBARIAN, 150);
  spell_level(SKILL_BERZERK,          CLASS_BARBARIAN, 153);
  spell_level(SKILL_THROW,            CLASS_BARBARIAN, 155);
//spell_level(SKILL_RAGE,             CLASS_BARBARIAN, 163); /* Furia. Aumenta o Damroll em +20 e Hitroll em +20 e aumenta AC em +100. */
  spell_level(SKILL_FOURTH_ATTACK,    CLASS_BARBARIAN, 171);
  spell_level(SKILL_CHOP,             CLASS_BARBARIAN, 175);
//spell_level(SKILL_BATTLE_TATICS,    CLASS_BARBARIAN, 188); /* Uso de artimanhas durante a luta, como jogar areia nos olhos do adversario, confundir o oponente e outros. Daria pequenos danos e funcionaria automaticamente. */
//spell_level(SKILL_LEAP,             CLASS_BARBARIAN, 195); /* Significa se jogar sobre os oponentes. Resultado: tanto o player como todos os outros q estejam lutando contra ele fica SITTING 1 turno. A vantagem dessa skill e quando se luta em grupo. Nesse caso todos ficariam sentados e os aliados do player q executou LEAP ficariam em pe. */
  spell_level(SKILL_IMPROVED_BERZERK, CLASS_BARBARIAN, 195) ;

  /* SHAMAN */
  spell_level(SPELL_METEOR_SHOWER,    CLASS_SORCERER,  1);
  spell_level(SPELL_ARMOR,            CLASS_SORCERER,  4);
  spell_level(SKILL_VITALIZE_MANA,    CLASS_SORCERER,  9);
//spell_level(SPELL_LIGHT,            CLASS_SORCERER,  11); /* Faz com que uma sala escura se torne luminosa por 12 horas. */
  spell_level(SPELL_BLESS,            CLASS_SORCERER,  15);
//spell_level(SPELL_REPULSION,        CLASS_SORCERER,  17); /* O player que usar essa magia cria uma repulsao na sala que faz com que criaturas de 10 ou mais levels para baixo do player nao consiga permanecer na sala por mais de 5 segundos. Resultado: a criatura e expelida para uma outra sala q nao seja DT. */
  spell_level(SPELL_GROUP_ARMOR,      CLASS_SORCERER,  20);
  spell_level(SPELL_MINUTE_METEOR,    CLASS_SORCERER,  22);
//  spell_level(SPELL_DETECT_EVIL,      CLASS_SORCERER,  26);
  spell_level(SPELL_SENSE_LIFE,       CLASS_SORCERER,  32);
  spell_level(SPELL_CURE_CRITIC,      CLASS_SORCERER,  33);
  spell_level(SPELL_WHEEL_OF_FORTUNE, CLASS_SORCERER,  38);
  spell_level(SPELL_GROUP_BLESS,      CLASS_SORCERER,  41);
  spell_level(SPELL_CORPOR_TREMBLING, CLASS_SORCERER,  43);
  spell_level(SPELL_ELEMENTAL,        CLASS_SORCERER,  50);
  spell_level(SPELL_INVISIBLE,        CLASS_SORCERER,  51);
  spell_level(SKILL_MEDITATE      ,   CLASS_SORCERER,  62);
  spell_level(SPELL_AREA_LIGHTNING,   CLASS_SORCERER,  64);
  spell_level(SPELL_FIREWALL,         CLASS_SORCERER,  71);
  spell_level(SKILL_LEVITATE,         CLASS_SORCERER,  83);
  spell_level(SPELL_VOLCANO,          CLASS_SORCERER,  85);
  spell_level(SPELL_CONFUSION,        CLASS_SORCERER,  87);
  spell_level(SPELL_BRAVERY,          CLASS_SORCERER,  96);
  spell_level(SKILL_SECOND_ATTACK,    CLASS_SORCERER,  101);
  spell_level(SPELL_GEYSER,           CLASS_SORCERER,  106);
  spell_level(SPELL_STORM_SKY,	      CLASS_SORCERER,  110);
  spell_level(SPELL_PEACE,            CLASS_SORCERER,  111);
//spell_level(SPELL_SPIRITUAL_HAMMER, CLASS_SORCERER,  111); /* Funciona igual MANA SWORD, mas o castador cria um martelo espiritual. */
  spell_level(SPELL_LOCATE_OBJECT,    CLASS_SORCERER,  116);
  spell_level(SPELL_ICEWALL,          CLASS_SORCERER,  123);
  spell_level(SPELL_ICE_HEART,        CLASS_SORCERER,  127);
//spell_level(SPELL_SHIVER_ARMOR,     CLASS_SORCERER,  132); /* Armadura que causa tremor ao adversario. Isso faz com que o oponente erre o primeiro ataque que tente dar, seja ele uma magia, uma skill qualquer, mental rage, etc. O detalhe e q depois q o adversario erra o golpe a armadura se desfaz, precisando ser castada novamente se desejar se defender novamente. Alem isso ela so pode ser utilizada antes do combate e nao durante ele. Ela so poderia ser usada qdo o castador estivesse GOOD. */
  spell_level(SPELL_FORCE_SHIELD,     CLASS_SORCERER,  135);
  spell_level(SPELL_GLACIAL_CONE,     CLASS_SORCERER,  148);
  spell_level(SPELL_BLINK,            CLASS_SORCERER,  155);
  spell_level(SPELL_GOD_PACT,         CLASS_SORCERER,  156);
  spell_level(SPELL_HEAL,             CLASS_SORCERER,  156);
  spell_level(SPELL_RIGID_THINKING,   CLASS_SORCERER,  161);
  spell_level(SPELL_FLY,              CLASS_SORCERER,  165);
  spell_level(SPELL_POLTEIRGEIST,     CLASS_SORCERER,  169);
  spell_level(SPELL_GROUP_HEAL,       CLASS_SORCERER,  173);
  spell_level(SPELL_ANCIENT_SPIRIT,   CLASS_SORCERER,  175); // conjuração, -ips
//spell_level(SPELL_GROUP_INVISIBLE,  CLASS_SORCERER,  177); /* O grupo inteiro fica com a spell INVISIBLE. */
  spell_level(SPELL_GROUP_FLY,        CLASS_SORCERER,  180);
  spell_level(SPELL_ADRENALINE,       CLASS_SORCERER,  186);
//spell_level(SPELL_SHOOTING_STARS,   CLASS_SORCERER,  190); /* O player q utilizar essa magia invoca meteoros automaticos durante 1 tic. A cada 2 turnos um meteoro de dano igual a um MAGIC MISSILE cai automaticamente causando dano a todos as criaturas, fora o castador, da sala. */
  spell_level(SPELL_PARADI_CHRYSALIS, CLASS_SORCERER,  195);
  spell_level(SPELL_MANA_SHIELD,      CLASS_SORCERER,  196);
  spell_level(SPELL_ABUTILON,         CLASS_SORCERER,  197);
  spell_level(SPELL_PETRIFY,          CLASS_SORCERER,  198);

  /* WARLOCK */
  spell_level(SPELL_METEOR_SHOWER,    CLASS_WARLOCK,  1);
  spell_level(SKILL_VITALIZE_MANA,    CLASS_WARLOCK,  3);
  spell_level(SPELL_ACIDARROW,        CLASS_WARLOCK,  6);
  spell_level(SPELL_DETECT_INVIS,     CLASS_WARLOCK,  9);
  spell_level(SPELL_BLADEBARRIER,     CLASS_WARLOCK, 11);
  spell_level(SKILL_MEDITATE,         CLASS_WARLOCK, 12);
  spell_level(SPELL_DETECT_POISON,    CLASS_WARLOCK, 16);
  spell_level(SPELL_FLAMEARROW,       CLASS_WARLOCK, 17);
  spell_level(SPELL_GREASE,           CLASS_WARLOCK, 23);
  spell_level(SPELL_DETECT_MAGIC,     CLASS_WARLOCK, 24);
  spell_level(SPELL_MAGIC_MISSILE,    CLASS_WARLOCK, 30);
  spell_level(SPELL_ARMOR,            CLASS_WARLOCK, 33);
  spell_level(SPELL_CROMATIC_ORB,     CLASS_WARLOCK, 37);
  spell_level(SPELL_LOCATE_OBJECT,    CLASS_WARLOCK, 43);
  spell_level(SPELL_CHILL_TOUCH,      CLASS_WARLOCK, 45);
  spell_level(SPELL_EXHAUSTION,       CLASS_WARLOCK, 49);
  spell_level(SPELL_AREA_LIGHTNING,   CLASS_WARLOCK, 53);
  spell_level(SPELL_BURNING_HANDS,    CLASS_WARLOCK, 62);
  spell_level(SPELL_CONTROL_WEATHER,  CLASS_WARLOCK, 66);
  spell_level(SKILL_STYLE_BATTLECASTING, CLASS_WARLOCK, 70);
  spell_level(SPELL_ELETRICSTORM,     CLASS_WARLOCK, 71);
  spell_level(SKILL_LEVITATE,         CLASS_WARLOCK, 79);
  spell_level(SPELL_SHOCKING_GRASP,   CLASS_WARLOCK, 81);
  spell_level(SPELL_ICESTORM,         CLASS_WARLOCK, 91);
  spell_level(SPELL_SLOW,             CLASS_WARLOCK, 93);
  spell_level(SKILL_SILENT_SPELL,     CLASS_WARLOCK, 100);
  spell_level(SPELL_ASPHYXIATE,       CLASS_WARLOCK, 104);
  spell_level(SPELL_COLOR_SPRAY,      CLASS_WARLOCK, 102);
  spell_level(SKILL_SECOND_ATTACK,    CLASS_WARLOCK, 108);
  spell_level(SPELL_FIRESTORM,        CLASS_WARLOCK, 113);
  spell_level(SPELL_LIGHTNING_BOLT,   CLASS_WARLOCK, 125);
  spell_level(SPELL_IRON_BODY,        CLASS_WARLOCK, 128);
  spell_level(SPELL_MANA_BURN,        CLASS_WARLOCK, 130);
//spell_level(SPELL_OPEN_LOCK,        CLASS_WARLOCK, 133); /* Igual Pick Lock sem improved. */
  spell_level(SPELL_METEORSTORM,      CLASS_WARLOCK, 137);
  spell_level(SPELL_ENHANCED_ARMOR,   CLASS_WARLOCK, 141);
  spell_level(SPELL_GLACIAL_CONE,     CLASS_WARLOCK, 148);
  spell_level(SPELL_LIQUID_AIR,       CLASS_WARLOCK, 152);
  spell_level(SPELL_STRENGTH,         CLASS_WARLOCK, 157);
  spell_level(SPELL_BLIZZARD,         CLASS_WARLOCK, 159);
  spell_level(SPELL_CLONE,            CLASS_WARLOCK, 166);
  spell_level(SPELL_PRISMATIC_SPHERE, CLASS_WARLOCK, 169);
  spell_level(SPELL_FIREBALL,         CLASS_WARLOCK, 179);
  spell_level(SPELL_TELEPORT,         CLASS_WARLOCK, 186);
  spell_level(SPELL_DELAYED_FIREBALL, CLASS_WARLOCK, 197);
//spell_level(SPELL_FURY,             CLASS_WARLOCK, 198); /* Spell que dobra o numero de ataques do castador. Deve-se perder um pouco de MANA a cada ataque acrescentado que o player efetuar. */

/* PSIONICIST */
  spell_level(SPELL_CREATE_LIGHT,     CLASS_PSIONICIST,   1);
  spell_level(SPELL_THOUGHT_SHIELD,   CLASS_PSIONICIST,   3);
  spell_level(SKILL_VITALIZE_MANA,    CLASS_PSIONICIST,   5);
  spell_level(SPELL_AURA_SIGHT,       CLASS_PSIONICIST,   6);
  spell_level(SPELL_CLAIRVOYANCE,     CLASS_PSIONICIST,  10);
//spell_level(SPELL_DANGER_SENSE,     CLASS_PSIONICIST,  11); /* Pressentir durante 8 tics se ha DTs ao redor da sala. */
//spell_level(SPELL_ECTOPLASMIC_FORM, CLASS_PSIONICIST,  14); /* Transformacao em uma forma Ectoplasmica. A unica vantagem e o fato de se regenerar mais rapidamente. */
//spell_level(SPELL_POISON_SENSE,     CLASS_PSIONICIST,  15); /* Igual DETECT POISON. */
  spell_level(SPELL_CREATE_FLAMES,    CLASS_PSIONICIST,  17);
//spell_level(SPELL_LIFE_DETECTION,   CLASS_PSIONICIST,  18); /* Quando utilizado em uma criatura, mostra qto de HitPoints a criatura possui. */
  spell_level(SPELL_CLAIRAUDIENCE,    CLASS_PSIONICIST,  21);
//spell_level(SPELL_DIMENSIONAL_DOOR, CLASS_PSIONICIST,  23); /* Cria uma porta bidimensional que leva ao templo. A porta dura 1 tic, e deve gastar 10x mais MANA que um WORD OF RECALL. */
//spell_level(SPELL_VISIONS,          CLASS_PSIONICIST,  25); /* Faz com que o player que sofra essa spell tenha visoes durante 1 tic de coisas como? XXX is standing here, XXX has arrived, XXX goes to north, etc., onde XXX e o player q utilizou a magia. */
  spell_level(SPELL_ILLUSION,         CLASS_PSIONICIST,  28);
//spell_level(SPELL_SUMMON_PLANAR,    CLASS_PSIONICIST,  29); /* Igual a spell ELEMENTAL, mas nesse caso ele invoca uma PLANAR CREATURE. */
  spell_level(SPELL_WINGED_KNIFE,     CLASS_PSIONICIST,  33);
  spell_level(SPELL_BIOFEEDBACK,      CLASS_PSIONICIST,  34);
  spell_level(SPELL_PAIN,             CLASS_PSIONICIST,  48);
  spell_level(SPELL_ENHANCED_STRENGTH,CLASS_PSIONICIST,  45);
  spell_level(SPELL_EGO_WHIP,         CLASS_PSIONICIST,  49);
  spell_level(SPELL_FARSIGHT,         CLASS_PSIONICIST,  51);
  spell_level(SPELL_EMPATHY,          CLASS_PSIONICIST,  53);
  spell_level(SPELL_DISPLACEMENT,     CLASS_PSIONICIST,  55);
  spell_level(SPELL_INTEL_FORTRESS,   CLASS_PSIONICIST,  63);
  spell_level(SPELL_LIFE_DRAINING,    CLASS_PSIONICIST,  65);
  spell_level(SKILL_LEVITATE,         CLASS_PSIONICIST,  66);
  spell_level(SPELL_BODY_WEAPONRY,    CLASS_PSIONICIST,  74);
  spell_level(SKILL_MEDITATE,         CLASS_PSIONICIST,  75);
  spell_level(SPELL_BODY_EQUILIBRIUM, CLASS_PSIONICIST,  78);
  spell_level(SPELL_PROJECT_FORCE,    CLASS_PSIONICIST,  81);
  spell_level(SPELL_ADREN_CONTROL,    CLASS_PSIONICIST,  85);
//spell_level(SPELL_CHAMELEON_POWER,  CLASS_PSIONICIST,  91); /* Spell que funciona igual a skill HIDE. */
//spell_level(SPELL_SHADOW_FORM,      CLASS_PSIONICIST,  96); /* Spell de transformacao. Faz com que o player nao gaste MOVE enquanto se locomove. */
  spell_level(SPELL_WRENCH,           CLASS_PSIONICIST,  97);
  spell_level(SKILL_SECOND_ATTACK,    CLASS_PSIONICIST,  105);
//  spell_level(SPELL_MENTAL BARRIER,   CLASS_PSIONICIST,  108);
  spell_level(SPELL_LEND_HEALTH,      CLASS_PSIONICIST,  113);
//spell_level(SPELL_INNER_SIGHT,      CLASS_PSIONICIST,  117); /* Magia que impede durante 3 tics que uma criatura fique HIDE ou INVISIBLE. */
//spell_level(SPELL_ILLUSIVE_DAGGER,  CLASS_PSIONICIST,  120); /* Spell igual MANA SWORD, mas nesse caso o player cria uma dagger ilusiva. */
  spell_level(SPELL_FEEL_LIGHT,       CLASS_PSIONICIST,  121);
  spell_level(SPELL_BALISTIC_ATTACK,  CLASS_PSIONICIST,  129);
  spell_level(SPELL_COMBAT_MIND,      CLASS_PSIONICIST,  135);
//spell_level(SPELL_BODY_CONTROL,     CLASS_PSIONICIST,  136); /* Faz com que o player nao sinta fome, sede ou bebado. Vale por 1 WAYBREAD, 2 goles de agua e 4 horas de recuperacao de bebedeira. */
  spell_level(SPELL_PSIONIC_BLAST,    CLASS_PSIONICIST,  141);
  spell_level(SPELL_PSYCHIC_DRAIN,    CLASS_PSIONICIST,  145);
  spell_level(SPELL_WRAITHFORM,       CLASS_PSIONICIST,  149);
  spell_level(SPELL_INERTIAL_BARRIER, CLASS_PSIONICIST,  150);
//spell_level(SPELL_PROBAB_TRAVEL,    CLASS_PSIONICIST,  153); /* Spell que funciona como a Skill TRACK. */
  spell_level(SPELL_SUP_INVISIBLE,    CLASS_PSIONICIST,  159);
  spell_level(SPELL_DETONATE,         CLASS_PSIONICIST,  161);
  spell_level(SPELL_AVERSION,         CLASS_PSIONICIST,  166);
//spell_level(SPELL_DAYDREAM,         CLASS_PSIONICIST,  171); /* O player que usar essa spell dorme por 8 horas com boa recuperacao sem ficar com sede ou com fome. */
  spell_level(SPELL_MENTAL_RAGE,      CLASS_PSIONICIST,  175);
  spell_level(SPELL_PSYCHIC_RAY,      CLASS_PSIONICIST,  177);
  spell_level(SPELL_CELL_ADJUSTMENT,  CLASS_PSIONICIST,  181);
  spell_level(SPELL_MARTIAL_TRANCE,   CLASS_PSIONICIST,  188);
  spell_level(SPELL_MOLEC_AGITATION,  CLASS_PSIONICIST,  193);
  spell_level(SPELL_ASTRAL_PROJECTION,  CLASS_PSIONICIST,  195);
//spell_level(SPELL_MIND_BLANK,       CLASS_PSIONICIST,  198); /* Faz com que a criatura ou player atingido por essa spell esqueca 70% de todas as skills e spells q ele conhecer por 1 tic. */
}

/* Function to return the exp required for each class/level */
int level_exp_norm(int level)
{
    switch (level) {
      case   0: return (0);
      case   1: return (3443);
      case   2: return (6593);
      case   3: return (11711);
      case   4: return (19061);
      case   5: return (28904);
      case   6: return (41504);
      case   7: return (57122);
      case   8: return (76022);
      case   9: return (98465);
      case  10: return (124715);
      case  11: return (155033);
      case  12: return (189683);
      case  13: return (228926);
      case  14: return (273026);
      case  15: return (322244);
      case  16: return (376844);
      case  17: return (437087);
      case  18: return (503237);
      case  19: return (575555);
      case  20: return (654305);
      case  21: return (739748);
      case  22: return (832148);
      case  23: return (931766);
      case  24: return (1038866);
      case  25: return (1153709);
      case  26: return (1276559);
      case  27: return (1407677);
      case  28: return (1547327);
      case  29: return (1695770);
      case  30: return (1853270);
      case  31: return (2020088);
      case  32: return (2196488);
      case  33: return (2382731);
      case  34: return (2579081);
      case  35: return (2785799);
      case  36: return (3003149);
      case  37: return (3231392);
      case  38: return (3470792);
      case  39: return (3721610);
      case  40: return (3984110);
      case  41: return (4258553);
      case  42: return (4545203);
      case  43: return (4844321);
      case  44: return (5156171);
      case  45: return (5481014);
      case  46: return (5819114);
      case  47: return (6170732);
      case  48: return (6536132);
      case  49: return (6915575);
      case  50: return (7309325);
      case  51: return (7717643);
      case  52: return (8140793);
      case  53: return (8579036);
      case  54: return (9032636);
      case  55: return (9501854);
      case  56: return (9986954);
      case  57: return (10488197);
      case  58: return (11005847);
      case  59: return (11540165);
      case  60: return (12091415);
      case  61: return (12659858);
      case  62: return (13245758);
      case  63: return (13849376);
      case  64: return (14470976);
      case  65: return (15110819);
      case  66: return (15769169);
      case  67: return (16446287);
      case  68: return (17142437);
      case  69: return (17857880);
      case  70: return (18592880);
      case  71: return (19347698);
      case  72: return (20122598);
      case  73: return (20917841);
      case  74: return (21733691);
      case  75: return (22570409);
      case  76: return (23428259);
      case  77: return (24307502);
      case  78: return (25208402);
      case  79: return (26131220);
      case  80: return (27076220);
      case  81: return (28043663);
      case  82: return (29033813);
      case  83: return (30046931);
      case  84: return (31083281);
      case  85: return (32143124);
      case  86: return (33226724);
      case  87: return (34334342);
      case  88: return (35466242);
      case  89: return (36622685);
      case  90: return (37803935);
      case  91: return (39010253);
      case  92: return (40241903);
      case  93: return (41499146);
      case  94: return (42782246);
      case  95: return (44091464);
      case  96: return (45427064);
      case  97: return (46789307);
      case  98: return (48178457);
      case  99: return (49594775);
      case 100: return (51038525);
      case 101: return (52509968);
      case 102: return (54009368);
      case 103: return (55536986);
      case 104: return (57093086);
      case 105: return (58677929);
      case 106: return (60291779);
      case 107: return (61934897);
      case 108: return (63607547);
      case 109: return (65309990);
      case 110: return (67042490);
      case 111: return (68805308);
      case 112: return (70598708);
      case 113: return (72422951);
      case 114: return (74278301);
      case 115: return (76165019);
      case 116: return (78083369);
      case 117: return (80033612);
      case 118: return (82016012);
      case 119: return (84030830);
      case 120: return (86078330);
      case 121: return (88158773);
      case 122: return (90272423);
      case 123: return (92419541);
      case 124: return (94600391);
      case 125: return (96815234);
      case 126: return (99064334);
      case 127: return (101347952);
      case 128: return (103666352);
      case 129: return (106019795);
      case 130: return (108408545);
      case 131: return (110832863);
      case 132: return (113293013);
      case 133: return (115789256);
      case 134: return (118321856);
      case 135: return (120891074);
      case 136: return (123497174);
      case 137: return (126140417);
      case 138: return (128821067);
      case 139: return (131539385);
      case 140: return (134295635);
      case 141: return (137090078);
      case 142: return (139922978);
      case 143: return (142794596);
      case 144: return (145705196);
      case 145: return (148655039);
      case 146: return (151644389);
      case 147: return (154673507);
      case 148: return (157742657);
      case 149: return (160852100);
      case 150: return (164002100);
      case 151: return (167192918);
      case 152: return (170424818);
      case 153: return (173698061);
      case 154: return (177012911);
      case 155: return (180369629);
      case 156: return (183768479);
      case 157: return (187209722);
      case 158: return (190693622);
      case 159: return (194220440);
      case 160: return (197790440);
      case 161: return (201403883);
      case 162: return (205061033);
      case 163: return (208762151);
      case 164: return (212507501);
      case 165: return (216297344);
      case 166: return (220131944);
      case 167: return (224011562);
      case 168: return (227936462);
      case 169: return (231906905);
      case 170: return (235923155);
      case 171: return (239985473);
      case 172: return (244094123);
      case 173: return (248249366);
      case 174: return (252451466);
      case 175: return (256700684);
      case 176: return (260997284);
      case 177: return (265341527);
      case 178: return (269733677);
      case 179: return (274173995);
      case 180: return (278662745);
      case 181: return (283200188);
      case 182: return (287786588);
      case 183: return (292422206);
      case 184: return (297107306);
      case 185: return (301842149);
      case 186: return (306626999);
      case 187: return (311462117);
      case 188: return (316347767);
      case 189: return (321284210);
      case 190: return (326271710);
      case 191: return (331310528);
      case 192: return (336400928);
      case 193: return (341543171);
      case 194: return (346737521);
      case 195: return (351984239);
      case 196: return (357283589);
      case 197: return (362635832);
      case 198: return (368041232);
      case 199: return (373500050);
      case 200: return (379012550);
      case LVL_IMMORT: return (904526713);
    }

  log("SYSERR: XP tables not set up correctly in class.c!");
  return (0);
}

int level_exp(int remort, int level)
{
  int factor, exp;

  if (level > LVL_IMMORT || level < 0) {
    return (0);
  }

  factor = (15 - remort);

  if(remort == 0)
    exp = level_exp_norm(level);
  else
    exp = (level_exp_norm(level) + ((level_exp_norm(level)/factor)*3));

  return (exp);

}

/*
* Default titles of male characters.
*/
const char *title_male(int chclass, int level)
{
  if (level <= 0 || level > LVL_IMPL)
    return "the Man";
  if (level == LVL_IMPL)
    return "the Owner Implementor";

  switch (chclass) {

    case CLASS_MAGIC_USER:
    switch (level) {
      case LVL_IMMORT: return "the Immortal";
      case LVL_ELDER:
      case LVL_LORD:
      case LVL_DEMIGOD:
      case LVL_GOD:
      case LVL_GRGOD:
      case LVL_SUPGOD:
      case LVL_MJGOD:
      case LVL_SUBIMPL:
        return "the member of the WarDome Staff";
      default: return "the Sorcerer";
    }
    break;

    case CLASS_NECROMANCER:
    switch (level) {
      case LVL_IMMORT: return "the Immortal";
      case LVL_ELDER:
      case LVL_LORD:
      case LVL_DEMIGOD:
      case LVL_GOD:
      case LVL_GRGOD:
      case LVL_SUPGOD:
      case LVL_MJGOD:
      case LVL_SUBIMPL:
        return "the member of the WarDome Staff";
      default: return "the Necromancer";
    }
    break;

    case CLASS_PALADIN:
    switch (level) {
      case LVL_IMMORT: return "the Immortal";
      case LVL_ELDER:
      case LVL_LORD:
      case LVL_DEMIGOD:
      case LVL_GOD:
      case LVL_GRGOD:
      case LVL_SUPGOD:
      case LVL_MJGOD:
      case LVL_SUBIMPL:
        return "the member of the WarDome Staff";
      default: return "the Paladin";
    }
    break;

    case CLASS_NINJA:
    switch (level) {
      case LVL_IMMORT: return "the Immortal";
      case LVL_ELDER:
      case LVL_LORD:
      case LVL_DEMIGOD:
      case LVL_GOD:
      case LVL_GRGOD:
      case LVL_SUPGOD:
      case LVL_MJGOD:
      case LVL_SUBIMPL:
        return "the member of the WarDome Staff";
      default: return "the Ninja";
    }
    break;

    case CLASS_CLERIC:
    switch (level) {
      case LVL_IMMORT: return "the Immortal";
      case LVL_ELDER:
      case LVL_LORD:
      case LVL_DEMIGOD:
      case LVL_GOD:
      case LVL_GRGOD:
      case LVL_SUPGOD:
      case LVL_MJGOD:
      case LVL_SUBIMPL:
        return "the member of the WarDome Staff";
      default: return "the Cleric";
    }
    break;

    case CLASS_THIEF:
    switch (level) {
      case LVL_IMMORT: return "the Immortal";
      case LVL_ELDER:
      case LVL_LORD:
      case LVL_DEMIGOD:
      case LVL_GOD:
      case LVL_GRGOD:
      case LVL_SUPGOD:
      case LVL_MJGOD:
      case LVL_SUBIMPL:
        return "the member of the WarDome Staff";
      default: return "the Thief";
    }
    break;

    case CLASS_WARRIOR:
    switch(level) {
      case LVL_IMMORT: return "the Immortal";
      case LVL_ELDER:
      case LVL_LORD:
      case LVL_DEMIGOD:
      case LVL_GOD:
      case LVL_GRGOD:
      case LVL_SUPGOD:
      case LVL_MJGOD:
      case LVL_SUBIMPL:
        return "the member of the WarDome Staff";
      default: return "the Warrior";
    }
    break;

    case CLASS_RANGER:
    switch(level) {
      case LVL_IMMORT: return "the Immortal";
      case LVL_ELDER:
      case LVL_LORD:
      case LVL_DEMIGOD:
      case LVL_GOD:
      case LVL_GRGOD:
      case LVL_SUPGOD:
      case LVL_MJGOD:
      case LVL_SUBIMPL:
        return "the member of the WarDome Staff";
      default: return "the Ranger";
    }
    break;

    case CLASS_PSIONICIST:
    switch(level) {
      case LVL_IMMORT: return "the Immortal";
      case LVL_ELDER:
      case LVL_LORD:
      case LVL_DEMIGOD:
      case LVL_GOD:
      case LVL_GRGOD:
      case LVL_SUPGOD:
      case LVL_MJGOD:
      case LVL_SUBIMPL:
        return "the member of the WarDome Staff";
      default: return "the Psionicist";
    }
    break;

    case CLASS_WARLOCK:
    switch(level) {
      case LVL_IMMORT: return "the Immortal";
      case LVL_ELDER:
      case LVL_LORD:
      case LVL_DEMIGOD:
      case LVL_GOD:
      case LVL_GRGOD:
      case LVL_SUPGOD:
      case LVL_MJGOD:
      case LVL_SUBIMPL:
        return "the member of the WarDome Staff";
      default: return "the Warlock";
    }
    break;

    case CLASS_BARBARIAN:
    switch(level) {
      case LVL_IMMORT: return "the Immortal";
      case LVL_ELDER:
      case LVL_LORD:
      case LVL_DEMIGOD:
      case LVL_GOD:
      case LVL_GRGOD:
      case LVL_SUPGOD:
      case LVL_MJGOD:
      case LVL_SUBIMPL:
        return "the member of the WarDome Staff";
      default: return "the Barbarian";
    }
    break;

    case CLASS_SORCERER:
    switch(level) {
      case LVL_IMMORT: return "the Immortal";
      case LVL_ELDER:
      case LVL_LORD:
      case LVL_DEMIGOD:
      case LVL_GOD:
      case LVL_GRGOD:
      case LVL_SUPGOD:
      case LVL_MJGOD:
      case LVL_SUBIMPL:
        return "the member of the WarDome Staff";
      default: return "the Shaman";
    }
    break;
  }

  /* Default title for classes which do not have titles defined */
  return "the Classless";
}


/*
* Default titles of female characters.
*/
const char *title_female(int chclass, int level)
{
  if (level <= 0 || level > LVL_IMPL)
    return "the Woman";
  if (level == LVL_IMPL)
    return "the Owner Implementress";

  switch (chclass) {

    case CLASS_MAGIC_USER:
    switch (level) {
      case LVL_IMMORT: return "the Immortal";
      case LVL_ELDER:
      case LVL_LORD:
      case LVL_DEMIGOD:
      case LVL_GOD:
      case LVL_GRGOD:
      case LVL_SUPGOD:
      case LVL_MJGOD:
      case LVL_SUBIMPL:
        return "the member of the WarDome Staff";
      default: return "the Sorceress";
    }
    break;

    case CLASS_CLERIC:
    switch (level) {
      case LVL_IMMORT: return "the Immortal";
      case LVL_ELDER:
      case LVL_LORD:
      case LVL_DEMIGOD:
      case LVL_GOD:
      case LVL_GRGOD:
      case LVL_SUPGOD:
      case LVL_MJGOD:
      case LVL_SUBIMPL:
        return "the member of the WarDome Staff";
      default: return "the Cleric";
    }
    break;

    case CLASS_NECROMANCER:
    switch (level) {
      case LVL_IMMORT: return "the Immortal";
      case LVL_ELDER:
      case LVL_LORD:
      case LVL_DEMIGOD:
      case LVL_GOD:
      case LVL_GRGOD:
      case LVL_SUPGOD:
      case LVL_MJGOD:
      case LVL_SUBIMPL:
        return "the member of the WarDome Staff";
      default: return "the Necromanceress";
    }
    break;

    case CLASS_PALADIN:
    switch (level) {
      case LVL_IMMORT: return "the Immortal";
      case LVL_ELDER:
      case LVL_LORD:
      case LVL_DEMIGOD:
      case LVL_GOD:
      case LVL_GRGOD:
      case LVL_SUPGOD:
      case LVL_MJGOD:
      case LVL_SUBIMPL:
        return "the member of the WarDome Staff";
      default: return "the Paladin";
    }
    break;

    case CLASS_NINJA:
    switch (level) {
      case LVL_IMMORT: return "the Immortal";
      case LVL_ELDER:
      case LVL_LORD:
      case LVL_DEMIGOD:
      case LVL_GOD:
      case LVL_GRGOD:
      case LVL_SUPGOD:
      case LVL_MJGOD:
      case LVL_SUBIMPL:
        return "the member of the WarDome Staff";
      default: return "the Ninja";
    }
    break;

    case CLASS_THIEF:
    switch (level) {
      case LVL_IMMORT: return "the Immortal";
      case LVL_ELDER:
      case LVL_LORD:
      case LVL_DEMIGOD:
      case LVL_GOD:
      case LVL_GRGOD:
      case LVL_SUPGOD:
      case LVL_MJGOD:
      case LVL_SUBIMPL:
        return "the member of the WarDome Staff";
      default: return "the Thief";
    }
    break;

    case CLASS_WARRIOR:
    switch(level) {
      case LVL_IMMORT: return "the Immortal";
      case LVL_ELDER:
      case LVL_LORD:
      case LVL_DEMIGOD:
      case LVL_GOD:
      case LVL_GRGOD:
      case LVL_SUPGOD:
      case LVL_MJGOD:
      case LVL_SUBIMPL:
        return "the member of the WarDome Staff";
      default: return "the Warrior";
    }
    break;

    case CLASS_RANGER:
    switch(level) {
      case LVL_IMMORT: return "the Immortal";
      case LVL_ELDER:
      case LVL_LORD:
      case LVL_DEMIGOD:
      case LVL_GOD:
      case LVL_GRGOD:
      case LVL_SUPGOD:
      case LVL_MJGOD:
      case LVL_SUBIMPL:
        return "the member of the WarDome Staff";
      default: return "the Ranger";
    }
    break;

    case CLASS_PSIONICIST:
    switch(level) {
      case LVL_IMMORT: return "the Immortal";
      case LVL_ELDER:
      case LVL_LORD:
      case LVL_DEMIGOD:
      case LVL_GOD:
      case LVL_GRGOD:
      case LVL_SUPGOD:
      case LVL_MJGOD:
      case LVL_SUBIMPL:
        return "the member of the WarDome Staff";
      default: return "the Psionicist";
    }
    break;

    case CLASS_WARLOCK:
    switch(level) {
      case LVL_IMMORT: return "the Immortal";
      case LVL_ELDER:
      case LVL_LORD:
      case LVL_DEMIGOD:
      case LVL_GOD:
      case LVL_GRGOD:
      case LVL_SUPGOD:
      case LVL_MJGOD:
      case LVL_SUBIMPL:
        return "the member of the WarDome Staff";
      default: return "the Warlock";
    }
    break;

    case CLASS_BARBARIAN:
    switch(level) {
      case LVL_IMMORT: return "the Immortal";
      case LVL_ELDER:
      case LVL_LORD:
      case LVL_DEMIGOD:
      case LVL_GOD:
      case LVL_GRGOD:
      case LVL_SUPGOD:
      case LVL_MJGOD:
      case LVL_SUBIMPL:
        return "the member of the WarDome Staff";
      default: return "the Barbarian";
    }
    break;

    case CLASS_SORCERER:
    switch(level) {
      case LVL_IMMORT: return "the Immortal";
      case LVL_ELDER:
      case LVL_LORD:
      case LVL_DEMIGOD:
      case LVL_GOD:
      case LVL_GRGOD:
      case LVL_SUPGOD:
      case LVL_MJGOD:
      case LVL_SUBIMPL:
        return "the member of the WarDome Staff";
      default: return "the Shaman";
    }
    break;
  }

  /* Default title for classes which do not have titles defined */
  return "the Classless";
}

