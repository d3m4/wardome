/* ************************************************************************
*   File: magic.c                                       Part of CircleMUD *
*  Usage: low-level functions for magic; spell template code              *
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
#include "spells.h"
#include "handler.h"
#include "db.h"
#include "dg_scripts.h"

extern struct room_data *world;
extern struct obj_data *object_list;
extern struct char_data *character_list;
extern struct index_data *obj_index;

extern struct descriptor_data *descriptor_list;
extern struct zone_data *zone_table;

extern int mini_mud;
extern int pk_allowed;
extern char *spell_wear_off_msg[];
extern struct default_mobile_stats *mob_defaults;
extern struct apply_mod_defaults *apmd;

int saving_throws(int class_num, int type, int level); /* class.c */
void clearMemory(struct char_data * ch);
void weight_change_object(struct obj_data * obj, int weight);
void add_follower(struct char_data * ch, struct char_data * leader);
extern struct spell_info_type spell_info[];
int can_pk(struct char_data * ch, struct char_data * vt);
//int check_wanted(struct char_data * ch);
void play_sound(struct char_data *ch, char *sound, int type);

/* local functions */
int mag_materials(struct char_data * ch, int item0, int item1, int item2,
int extract, int verbose);
void perform_mag_groups(int level, struct char_data * ch, struct char_data *
tch, int spellnum, int savetype);
int mag_savingthrow(struct char_data * ch, int type);
void affect_update(void);

/*
* Saving throws are now in class.c (bpl13)
*/

int mag_savingthrow(struct char_data * ch, int type)
{
  int save;

  /* negative apply_saving_throw values make saving throws better! */

  if (IS_NPC(ch)) /* NPCs use warrior tables according to some book */
    save = saving_throws(CLASS_WARRIOR, type, (int) GET_LEVEL(ch));
  else
    save = saving_throws((int) GET_CLASS(ch), type, (int) GET_LEVEL(ch));

  save += GET_SAVE(ch, type);

  /* throwing a 0 is always a failure */
  if (MAX(1, save) <  number(95, 200))
    return TRUE;
  else
    return FALSE;
}



/* affect_update: called from comm.c (causes spells to wear off) */
void affect_update(void)
{
  struct affected_type *af, *next;
  struct char_data *i;
  extern struct raff_node *raff_list;
  struct raff_node *raff, *next_raff, *temp;

  for (i = character_list; i; i = i->next)

    for (af = i->affected; af; af = next) {
      next = af->next;

      if ((AFF2_FLAGGED(i, AFF2_EXTEND_SPELLS)) && (af->type != SPELL_EXTEND_SPELLS) && (af->type > SPELL_MUIR)  && (af->type < SPELL_TEMPUS) && ((af->type > 0) && (af->type < MAX_SPELLS)))
	continue;

      if(af->duration > 22)
        af->duration = 22;
      else if (af->duration >= 1)
        af->duration--;
      else if (af->duration == -1)      /* No action */
        af->duration = -1;      /* GODs only! unlimited */
      else {
        if ((af->type > 0) && (af->type <= MAX_SPELLS))
          if (!af->next || (af->next->type != af->type) ||
              (af->next->duration > 0))
            if (*spell_wear_off_msg[af->type]) {
              send_to_char(spell_wear_off_msg[af->type], i);
              send_to_char("\r\n", i);
              if (af->bitvector == AFF_TANGLED)
                act("$n is free of the vines that bind.",TRUE,i,0,0,TO_ROOM);
            }
        affect_remove(i, af);
      }
    }
	/* update the room affections */
	for (raff = raff_list; raff; raff = next_raff) {
		next_raff = raff->next;

		raff->timer--;

		if (raff->timer <= 0) {
			/* this room affection has expired */
			send_to_room(spell_wear_off_msg[raff->spell],
				raff->room);
			send_to_room("\r\n", raff->room);

			/* remove the affection */
			REMOVE_BIT(world[(int)raff->room].room_affections,
				raff->affection);
			REMOVE_FROM_LIST(raff, raff_list, next)
			free(raff);
		}
	}
}


/*
*  mag_materials:
*  Checks for up to 3 vnums (spell reagents) in the player's inventory.
*
* No spells implemented in Circle 3.0 use mag_materials, but you can use
* it to implement your own spells which require ingredients (i.e., some
* heal spell which requires a rare herb or some such.)
*/
int mag_materials(struct char_data * ch, int item0, int item1, int item2,
                      int extract, int verbose)
{
  struct obj_data *tobj;
  struct obj_data *obj0 = NULL, *obj1 = NULL, *obj2 = NULL;

  for (tobj = ch->carrying; tobj; tobj = tobj->next_content) {
    if ((item0 > 0) && (GET_OBJ_VNUM(tobj) == item0)) {
      obj0 = tobj;
      item0 = -1;
    } else if ((item1 > 0) && (GET_OBJ_VNUM(tobj) == item1)) {
      obj1 = tobj;
      item1 = -1;
    } else if ((item2 > 0) && (GET_OBJ_VNUM(tobj) == item2)) {
      obj2 = tobj;
      item2 = -1;
    }
  }
  if ((item0 > 0) || (item1 > 0) || (item2 > 0)) {
    if (verbose) {
      switch (number(0, 2)) {
      case 0:
        send_to_char("A wart sprouts on your nose.\r\n", ch);
        break;
      case 1:
        send_to_char("Your hair falls out in clumps.\r\n", ch);
        break;
      case 2:
        send_to_char("A huge corn develops on your big toe.\r\n", ch);
        break;
      }
    }
    return (FALSE);
  }
  if (extract) {
    if (item0 < 0) {
      obj_from_char(obj0);
      extract_obj(obj0);
    }
    if (item1 < 0) {
      obj_from_char(obj1);
      extract_obj(obj1);
    }
    if (item2 < 0) {
      obj_from_char(obj2);
      extract_obj(obj2);
    }
  }
  if (verbose) {
    send_to_char("A puff of smoke rises from your pack.\r\n", ch);
    act("A puff of smoke rises from $n's pack.", TRUE, ch, NULL, NULL,
TO_ROOM);
  }
  return (TRUE);
}




/*
* Every spell that does damage comes through here.  This calculates the
* amount of damage, adds in any modifiers, determines what the saves are,
* tests for save and calls damage().
*
* -1 = dead, otherwise the amount of damage done.
*/
int mag_damage(int level, struct char_data * ch, struct char_data * victim,
                     int spellnum, int savetype)
{
  int dam = 0;

  if (victim == NULL || ch == NULL)
    return 0;

  if(!IS_NPC(ch) && !IS_NPC(victim)){
    if(!can_pk(ch, victim)){
      send_to_char("&WYou can not &Rkill &Wyour opponent right now.&n\r\n",
ch);
      return 0;
    }
  }

  switch (spellnum) {

    /* MAGIAS - CLERIC */

  case SPELL_BLADEBARRIER:
      dam = dice(15, 4); /* 37.5 */
    break;

  case SPELL_LIGHTNING_SPIT:
    dam = dice(1, 8)+2; /* 6.5 */
    break;

  case SPELL_FROST_SPIT:
    dam = dice(3, 8)+3; /* 15 */
    break;

  case SPELL_FIRE_SPIT:
    dam = dice(4, 6)+10; /* 24 */
    break;

  case SPELL_GAS_SPIT:
    dam = dice(8, 5)+6; /* 30 */
    break;

  case SPELL_ACID_SPIT:
    dam = dice(16, 15)+12; 
    break;

  case SPELL_DISPEL_EVIL:
    dam = dice(30, 18); /* 285 */
    if (IS_EVIL(ch)) {
      victim = ch;
      dam = GET_HIT(ch) - 1;
    } else if (!(IS_EVIL(victim))) {
      act("The gods protect $N.", FALSE, ch, 0, victim, TO_CHAR);
      dam = 0;
      return 0;
    }
    break;

  case SPELL_DISPEL_GOOD:
    dam = dice(30, 18); /* 285 */
    if (IS_GOOD(ch)) {
      victim = ch;
      dam = GET_HIT(ch) - 1;
    } else if (!(IS_GOOD(victim))) {
      act("The gods protect $N.", FALSE, ch, 0, victim, TO_CHAR);
      dam = 0;
      return 0;
    }
    break;

  case SPELL_DISPEL_NEUTRAL:
    dam = dice(30, 18); /* 285 */
    if (!(IS_GOOD(ch) || IS_EVIL(ch))) {
      victim = ch;
      dam = GET_HIT(ch) - 1;
    } else if (IS_GOOD(victim) || IS_EVIL(victim)) {
      act("The gods protect $N.", FALSE, ch, 0, victim, TO_CHAR);
      dam = 0;
      return 0;
    }
    break;

  case SPELL_PSIONIC_BLAST:
    dam = dice(15, 18);
    if (number(0, 10))
     send_to_char("You could not trip your opponent.\r\n", ch);
    else
   {
     GET_POS(victim) = POS_SITTING;
     WAIT_STATE(victim, PULSE_VIOLENCE);
     send_to_char("You manage to trip your victim.\r\n", ch); 
   }
    break;

    /* MAGIAS - RANGER */

  case SPELL_ACIDARROW:
      dam = dice(10, 15);
    break;

  case SPELL_FLAMEARROW:
      dam = dice(15, 15);
    break;

    /* MAGIAS - NECROMANCER */

  case SPELL_DEATH_RIPPLE:
    dam = dice(12, 15);
    break;

  case SPELL_DEATH_WAVE:
    dam = dice(32, 25);
    break;

  case SPELL_DEATH_FINGER:
      if(FIGHTING(victim) != ch) {
        send_to_char("The finger of death will only help you if the victim attacks you.\r\n",ch);
        return 0;
      }
      if(((GET_HIT(victim) * 100) / GET_MAX_HIT(victim))  >  19) {
        send_to_char("Your victim must be less than 19 percent healthy.\r\n", ch);
        return 0;
      }
      GET_HIT(victim) = 1;
      dam = 1000;
    break;

  case SPELL_SIPHON_LIFE:
      dam = dice(26, 25); /* 338 */
      if(AFF_FLAGGED(victim, AFF_SANCTUARY))
        GET_HIT(ch)= MIN(GET_HIT(ch) + (dam/2), GET_MAX_HIT(ch));
      else
        GET_HIT(ch)= MIN(GET_HIT(ch) + dam, GET_MAX_HIT(ch));
      break;

    /* MAGIAS - PALADIN */

  case SPELL_HOLY_MACE:
	  if (weather_info.sunlight == SUN_DARK)
		  dam = dice(17, 25);
	  else
		  dam = dice(22, 25);
    break;

  case SPELL_HOLY_SHOUT:
    if (weather_info.sunlight == SUN_DARK)
		  dam = dice(20, 25);
	  else
		  dam = dice(25, 25);
    break;

  case SPELL_HOLY_WORD:
	if (weather_info.sunlight == SUN_DARK)
		  dam = dice(28, 20);
	else
		  dam = dice(35, 20);
   break;

  case SPELL_HOLY_BOLT:
	if (weather_info.sunlight == SUN_DARK)
		  dam = dice(24, 25);
	else
		  dam = dice(30, 25);
   break ;

    /* MAGIAS - GERAL */

  case SPELL_CREATE_LIGHT:
    dam = dice(2, 6);
    break;

  case SPELL_PHANTOM_FLAME:
    dam = dice(9, 4);
    break;

  case SPELL_CREATE_FLAMES:
    dam = dice(12, 6);
    break;

  case SPELL_MAGIC_MISSILE:
    dam = dice(27, 4);
    break;

  case SPELL_WINGED_KNIFE:
    dam = dice(15, 12);
    break;

  case SPELL_CORPOR_TREMBLING:
    dam = dice(10, 20);
    break;

  case SPELL_CHILL_TOUCH:
    dam = dice(18, 20);
    break;

  case SPELL_EGO_WHIP:
    dam = dice(18, 18);
    break;

  case SPELL_BURNING_HANDS:
    dam = dice(20, 20);
    break;

  case SPELL_GHASTLY_TOUCH:
    dam = dice(27, 15);
    break;

  case SPELL_PROJECT_FORCE:
    dam = dice(27, 27);
    break;

  case SPELL_VOLCANO:
    dam = dice(27, 20);
    break;

  case SPELL_SHOCKING_GRASP:
    dam = dice(30, 20);
    break;

  case SPELL_WRENCH:
    dam = dice(22, 30);
    break;

  case SPELL_COLOR_SPRAY:
    dam = dice(27, 30);
    break;

  case SPELL_BALISTIC_ATTACK:
    dam = dice(30, 30);
    break;

  case SPELL_SHILLELAGH:
    dam = dice(48, 18);
    break;

  case SPELL_LIGHTNING_BOLT:
    if (weather_info.sky == SKY_LIGHTNING)
		  dam = dice(40, 30);
	else
		  dam = dice(30, 30);
    break;

  case SPELL_VAMPIRIC_TOUCH:
    dam = dice(36, 25);
    break;

  case SPELL_ICE_HEART:
    dam = dice(40, 20);
    break;

  case SPELL_ICEBOLT:
    dam = dice(31, 30);
    break;

  case SPELL_MAGICAL_STONE:
    dam = dice(48, 20);
    break;

  case SPELL_DETONATE:
    dam = dice(33, 30);
    break;

  case SPELL_GLACIAL_CONE:
    dam = dice(35, 30);
    break;

  case SPELL_FIREBOLT:
    dam = dice(36, 30);
    break;

  case SPELL_PSYCHIC_RAY:
    dam = dice(40, 30);
    break;

  case SPELL_POLTEIRGEIST:
    dam = dice(48, 25);
    break;

  case SPELL_HARM:
    dam = dice(40, 30);
    break;

  case SPELL_PRISMATIC_SPHERE:
    dam = dice(42, 30);
    break;

  case SPELL_MOLEC_AGITATION:
    dam = dice(41, 30);
    break;

  case SPELL_DELAYED_FIREBALL:
    dam = dice(48, 32);
    break;

    /* MAGIAS DE AREA - GERAL */

  case SPELL_METEOR_SHOWER:
    dam = dice(4, 4);
    break;

  case SPELL_MINUTE_METEOR:
    dam = dice(8, 12);
    break;

  case SPELL_GREASE:
    dam = dice(18, 4);
    break;

  case SPELL_CROMATIC_ORB:
    dam = dice(10, 20);
    break;

  case SPELL_CALL_LIGHTNING:
	  if (weather_info.sky == SKY_LIGHTNING)
		  dam = dice(24, 18);
	  else
		  dam = dice(12, 18);
    break;

  case SPELL_AREA_LIGHTNING:
      if (weather_info.sky == SKY_LIGHTNING)
		  dam = dice(28, 20);
	  else
		  dam = dice(15, 20);
    break;

  case SPELL_SKULL_TRAP:
	   if (weather_info.sunlight == SUN_DARK)
		   dam = dice(28, 15);
	   else if (weather_info.sunlight == SUN_LIGHT)
		   dam = dice(18, 15);
	   else
		   dam = dice(22, 15);
    break;

  case SPELL_EARTHQUAKE:
    dam = dice(20, 18);
    break;

  case SPELL_ELETRICSTORM:
    if (weather_info.sky == SKY_LIGHTNING)
		  dam = dice(30, 25);
	else
		  dam = dice(20, 25);
    break;

  case SPELL_ICESTORM:
    dam = dice(27, 25);
    break;

  case SPELL_GEYSER:
    dam = dice(32, 20);
    break;

  case SPELL_FIRESTORM:
    dam = dice(33, 25);
    break;

  case SPELL_METEORSTORM:
    dam = dice(37, 25);
    break;

  case SPELL_COMMAND:
    dam = dice(44, 25);
    break;

  case SPELL_BLIZZARD:
    dam = dice(37, 30);
    break;

  case SPELL_ARMAGEDDOM:
    dam = dice(40, 25);
    break;

  case SPELL_END_WORLD:
    dam = dice(40, 32);
    break;

  case SPELL_FIREBALL:
      dam = dice(48, 30);
    break;

  case SPELL_ULTIMA:
   dam = dice(100,30) ;
   break;

  case SPELL_SUMMON_BAHAMUT:
   dam = dice(80, 30) ;
  break;

  case SPELL_TALOS:
   dam = dice(50,35);
   break ;   

  case SPELL_MANA_BURN:
	  if (  (number(1, GET_WIS(victim))) < (number(1, GET_WIS(ch))+number(0, 5))) {
		  GET_MANA(victim) -= (GET_MANA(victim)/2) + (GET_WIS(ch)*2);
		  if (GET_MANA(victim) > 1) {
			if (!IS_NPC(victim))
			  dam = number(1, GET_MANA(victim));
			else
			  dam = GET_MANA(victim);
		  } else {
			  dam = 1;
		  }
		  if (GET_MANA(victim) < 0)
			  GET_MANA(victim) = 0;
	  } else {
		  dam = 0;
	  }
  break;

  case SPELL_EXHAUSTION:
	  if (  (number(1, GET_CON(victim))) < (number(1, GET_WIS(ch))+number(0, 5))) {
		  act("&c$n &ydrains &c$N&n's constitution! $E feels tired..", FALSE, ch, 0, victim, TO_ROOM | TO_NOTVICT);
		  act("&nYou &ydrain&n &c$N&n's constitution! $E feels tired..", FALSE, ch, 0, victim, TO_CHAR);
		  act("&c$n&n &ydrains&n your constitution! You feel tired..", FALSE, ch, 0, victim, TO_VICT);
		  GET_MOVE(victim) -= (GET_MOVE(victim)/1.5) + (GET_WIS(ch)*2);
		  if (GET_MOVE(victim) < 0)
			  GET_MOVE(victim) = 0;
	  } else {
		  act("$n tries to drain $N, but $E resists.", FALSE, ch, 0, victim, TO_ROOM | TO_NOTVICT);
		  act("You try to drain $N, but $E resists", FALSE, ch, 0, victim, TO_CHAR);
		  act("$n tries to drain you, but you resist.", FALSE, ch, 0, victim, TO_VICT);
	  }
	  return 0;
  break;

  }  /* switch(spellnum) */

  /* divide damage by two if victim makes his saving throw */
  if (mag_savingthrow(victim, savetype))
    dam /= 2;

  dam = (float) dam * (1 + (((float)GET_INT(ch) + (float) GET_CHA(ch)) / 125)) ; 
  dam = dam * 1.4;

  if ((GET_RACE(ch) == RACE_LICH || GET_RACE(ch) == RACE_DRACONIAN) && (!number(0,11)))
   dam = dam * 1.2;

  dam = dam * (1 + ((float)GET_REMORT(ch)/8)); //aumenta o dano conforme o remort


   if (GET_STYLE(ch) == 6) // style battlecasting -Ips & Str
	dam *= number(13, 15) / 10.0f;

  return (GET_POS(ch) > POS_DEAD ? damage(ch, victim, dam, spellnum) : 0);
}


/*
* Every spell that does an affect comes through here.  This determines
* the effect, whether it is added or replacement, whether it is legal or
* not, etc.
*
* affect_join(vict, aff, add_dur, avg_dur, add_mod, avg_mod)
*/

#define MAX_SPELL_AFFECTS 6     /* change if more needed */

void mag_affects(int level, struct char_data * ch, struct char_data *
victim, int spellnum, int savetype)
{
  struct affected_type af[MAX_SPELL_AFFECTS];
  bool accum_affect = FALSE, accum_duration = FALSE, imp = FALSE;
  const char *to_vict = NULL, *to_room = NULL;
  int i, x, y;

  if (victim == NULL || ch == NULL)
    return;

  for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
    af[i].type = spellnum;
    af[i].bitvector = 0;
    af[i].bitvector2 = 0;
    af[i].bitvector3 = 0;
    af[i].modifier = 0;
    af[i].location = APPLY_NONE;
  }

  switch (spellnum) {

  case SPELL_CHILL_TOUCH:
    af[0].location = APPLY_STR;
    if (mag_savingthrow(victim, savetype)){
    	if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    		af[0].duration = 2;
    	else
      		af[0].duration = 1;}
    else{
    	if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    		af[0].duration = 8;
    	else
      		af[0].duration = 4;}
    af[0].modifier = -4;
    accum_duration = TRUE;
    to_vict = "You feel your strength wither!";
    break;


  case SPELL_BARKSKIN :
    af[0].location = APPLY_AC;
    af[0].modifier = -30 - (GET_REMORT(ch)*1.5);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 10;
    else
    	af[0].duration = 5;
    to_vict = "You get a hard barkskin.";
    break;


  case SPELL_GHASTLY_TOUCH:
    af[0].location = APPLY_STR;
    if (mag_savingthrow(victim, savetype)){
    	if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    		af[0].duration = 4;
    	else
      		af[0].duration = 2;}
    else{
    	if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    		af[0].duration = 12;
    	else
      		af[0].duration = 6;}
    af[0].modifier = -6;
    accum_duration = TRUE;
    to_vict = "You feel your strength wither!";
    break;

  case SPELL_VAMPIRIC_TOUCH:
    af[0].location = APPLY_STR;
    if (mag_savingthrow(victim, savetype)){
    	if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    		af[0].duration = 5;
    	else
    		af[0].duration = 3;
	}
    else{
    	if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    		af[0].duration = 14;
    	else
    		af[0].duration = 8;}
    af[0].modifier = -10;
    accum_duration = TRUE;
    to_vict = "You feel your strength wither!";
    break;

  
  case SPELL_MUIR: // religiao
    af[0].duration = 5;
    af[0].bitvector2 = AFF2_MUIR1;
    to_vict = "You got the basic power of Muir.";
    break;

 case SPELL_TEMPUS: // religiao
    af[0].duration = 5;
    af[0].bitvector2 = AFF2_TEMPUS;
    to_vict = "You got the basic power of TEMPUS.";
    break;


  case SPELL_ILMANATEUR: // religiao
    af[0].duration = 6;
    af[0].bitvector2 = AFF2_ILMANATEUR1;
    to_vict = "You got the basic power of Ilmanateur.";
    break;

  case SPELL_SELUNE: // religiao
    af[0].duration = 8;
    af[0].bitvector2 = AFF2_SELUNE;
    to_vict = "You got the basic power of Selune.";
    break;

  case SPELL_ARMOR:
    if(affected_by_spell(victim, SPELL_ENHANCED_ARMOR))
    {
      send_to_char("You still protected with enhanced armor.\r\n", ch);
      play_sound(ch, "Armoron.wav", SND_CHAR);
      return;
    }
    af[0].location = APPLY_AC;
    af[0].modifier = -10-(GET_LEVEL(ch)/10) - (GET_REMORT(ch)*1.5);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 23;
    else
    	af[0].duration = 12;
    accum_duration = TRUE;
    to_vict = "You feel someone protecting you.";
    play_sound(ch, "Armoron.wav", SND_CHAR);
    break;

  case SPELL_BLESS:
    af[0].location = APPLY_HITROLL;
    af[0].modifier = 1+(GET_LEVEL(ch)/35)+ (GET_REMORT(ch)/4);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 12;
    else
    	af[0].duration = 6;

    af[1].location = APPLY_SAVING_SPELL;
    af[1].modifier = -1 - (GET_REMORT(ch)*2);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[1].duration = 12;
    else
    	af[1].duration = 6;

    accum_duration = TRUE;
    to_vict = "You feel righteous.";
    break;

  case SPELL_BRAVERY:
    af[0].location = APPLY_HITROLL;
    af[0].modifier = 10 + (GET_REMORT(ch)/3);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 4;
    else
    	af[0].duration = 2;

    af[1].location = APPLY_DAMROLL;
    af[1].modifier = 10 + (GET_REMORT(ch)/3);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[1].duration = 4;
    else
    	af[1].duration = 2;

    accum_duration = FALSE;
    to_vict = "You find the heavens bravery.";
    break;

  case SPELL_CONCENTRATION:
    af[0].location = APPLY_HITROLL;
    af[0].modifier = 4 + (GET_REMORT(ch)/3);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 8;
    else
    	af[0].duration = 4;

    accum_duration = FALSE;
    to_vict = "You start to concentrate better in your attacks.";
    break;

  case SPELL_CONVICTION:
    af[0].location = APPLY_DAMROLL;
    af[0].modifier = 4 + (GET_REMORT(ch)/3);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 8;
    else
    	af[0].duration = 4;

    accum_duration = FALSE;
    to_vict = "You start to attack more convicted.";
    break;

  case SPELL_EXTEND_SPELLS:
    af[0].location = APPLY_STR;
    af[0].modifier = -4;
    af[0].duration = number(2, 4);
    af[0].bitvector2 = AFF2_EXTEND_SPELLS;

    accum_duration = FALSE;
    accum_affect = FALSE;
    to_vict = "&WYour &mmagic&W is now protected from &CTime&W itself.&n";
  break;

  case SPELL_AURA_SIGHT:
    af[0].location = APPLY_HITROLL;
    af[0].modifier = 3 + (GET_REMORT(ch)/3);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 16;
    else
    	af[0].duration = 8;

    af[1].location = APPLY_SAVING_SPELL;
    af[1].modifier = -6 - (GET_REMORT(ch)*2);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[1].duration = 16;
    else
    	af[1].duration = 8;

    accum_duration = TRUE;
    to_vict = "You feel an aura helping you.";
    break;

  case SPELL_BODY_WEAPONRY:
    af[0].location = APPLY_HITROLL;
    af[0].modifier = 3 + (GET_REMORT(ch)/3);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 10;
    else
    	af[0].duration = 5;
    accum_duration = TRUE;
    to_vict = "You feel your body turning a weaponry machine!!";
    break;

  case SPELL_BODY_EQUILIBRIUM:
    af[0].location = APPLY_DAMROLL;
    af[0].modifier = 3 + (GET_REMORT(ch)/3);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 10;
    else
    	af[0].duration = 5;

    accum_duration = TRUE;
    to_vict = "You feel yourself balanced!";
    break;

  case SPELL_COMBAT_MIND:
    af[0].location = APPLY_DAMROLL;
    af[0].modifier = 2 + (GET_REMORT(ch)/3);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 12;
    else
    	af[0].duration = 6;

    af[1].location = APPLY_HITROLL;
    af[1].modifier = 2 + (GET_REMORT(ch)/3);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[1].duration = 12;
    else
    	af[1].duration = 6;

    accum_duration = TRUE;
    to_vict = "You feel your battle concentration rising";
    break;

  case SPELL_ANTIMAGIC_SHELL:
    af[0].location = APPLY_SAVING_SPELL;
    af[0].modifier = -30 - (GET_REMORT(ch)*2);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 10;
    else
    	af[0].duration = 5;

    accum_duration = TRUE;
    to_vict = "You feel your magic resistance increasing.";
    break;

  case SPELL_MINOR_GLOBE:
    af[0].location = APPLY_AC;
    af[0].modifier = -10 - (GET_REMORT(ch)/2);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 8;
    else
    	af[0].duration = 4;

    af[1].location = APPLY_DAMROLL;
    af[1].modifier = +2 + (GET_REMORT(ch)/5);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[1].duration = 8;
    else
    	af[1].duration = 4;

    af[2].location = APPLY_HITROLL;
    af[2].modifier = +2 + (GET_REMORT(ch)/5);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[2].duration = 8;
    else
    	af[2].duration = 4;

    af[3].location = APPLY_SAVING_SPELL;
    af[3].modifier = -2 - (GET_REMORT(ch)*2);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[3].duration = 8;
    else
    	af[3].duration = 4;

    accum_duration = TRUE;
    to_vict = "You feel a globe protecting you.";
    break;

  case SPELL_BLINDNESS:
	  if ((MOB_FLAGGED(victim,MOB_NOBLIND)) || (mag_savingthrow(victim, savetype))) {
		  send_to_char("You fail.\r\n", ch);
      return;
    }
    if(!IS_NPC(ch) && !IS_NPC(victim)){
      if(!can_pk(ch, victim)){
        send_to_char("&WYou can not &Rkill &Wyour opponent right now.&n\r\n", ch);
        return;
      }
    }
    af[0].location = APPLY_HITROLL;
    af[0].modifier = -4-(GET_HITROLL(victim)/4);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 4;
    else
    	af[0].duration = 2;
    af[0].bitvector = AFF_BLIND;

    af[1].location = APPLY_AC;
    af[1].modifier = 40;
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[1].duration = 4;
    else
    	af[1].duration = 2;
    af[1].bitvector = AFF_BLIND;

    to_room = "$n seems to be blinded!";
    to_vict = "You have been blinded!";
    break;

  case SPELL_FEEL_LIGHT:
    if (MOB_FLAGGED(victim,MOB_NOBLIND) || ((GET_LEVEL(victim) > GET_LEVEL(ch)) && (number(25, 50) < number(40, 65)))) {
      send_to_char("You misfire.\r\n", ch);
      return;
    }
    if(!IS_NPC(ch) && !IS_NPC(victim)){
      if(!can_pk(ch, victim)){
        send_to_char("&WYou can not &Rkill &Wyour opponent right now.&n\r\n", ch);
        return;
      }
    }
    af[0].location = APPLY_HITROLL;
    af[0].modifier = -8 - (GET_HITROLL(victim)/4);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 2;
    else
    	af[0].duration = 1;
    af[0].bitvector = AFF_BLIND;

    af[1].location = APPLY_AC;
    af[1].modifier = 30 + (GET_REMORT(ch));
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[1].duration = 2;
    else
    	af[1].duration = 1;
    af[1].bitvector = AFF_BLIND;

    to_room = "$n seems to feel the light in his eyes!";
    to_vict = "You have been blinded by the light in your eyes!";
    break;

  case SPELL_CURSE:
    if (mag_savingthrow(victim, savetype)) {
      send_to_char(NOEFFECT, ch);
      return;
    }
    if(!IS_NPC(ch) && !IS_NPC(victim)){
      if(!can_pk(ch, victim)){
        send_to_char("&WYou can not &Rkill &Wyour opponent right now.&n\r\n", ch);
        return;
      }
    }
    af[0].location = APPLY_HITROLL;
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 2 + (GET_LEVEL(ch) / 6);
    else
    	af[0].duration = 1 + (GET_LEVEL(ch) / 6);
    af[0].modifier = -1;
    af[0].bitvector = AFF_CURSE;

    af[1].location = APPLY_DAMROLL;
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 2 + (GET_LEVEL(ch) / 6);
    else
    	af[1].duration = 1 + (GET_LEVEL(ch) / 6);
    af[1].modifier = -1;
    af[1].bitvector = AFF_CURSE;

    accum_duration = TRUE;
    accum_affect = FALSE;
    to_room = "$n briefly glows red!";
    to_vict = "You feel very uncomfortable.";
    break;
    break;

  case SPELL_ASPHYXIATE:
	  
	  if (mag_savingthrow(victim, savetype)) {
		  send_to_char(NOEFFECT, ch);
		  return;
	  }
	  
	  if(!IS_NPC(ch) && !IS_NPC(victim)){
		  if(!can_pk(ch, victim)){
			  send_to_char("&WYou can not &Rkill &Wyour opponent right now.&n\r\n", ch);
			  return;
		  }
	  }

    af[0].duration = number(2, 3);
    af[0].bitvector2 = AFF2_ASPHYXIATE;

    accum_duration = TRUE;
    accum_affect = FALSE;
    to_room = "$n briefly glows blue!";
    to_vict = "You feel very uncomfortable.";
  break;


  case SPELL_GROW_ROOTS:

    if (mag_savingthrow(victim, savetype)) {
      send_to_char(NOEFFECT, ch);
      return;
    }

    if(!IS_NPC(ch) && !IS_NPC(victim)){
      if(!can_pk(ch, victim)){
        send_to_char("&WYou can not &Rharm &Wyour opponent right now.&n\r\n", ch);
        return;
      }
    }

    af[0].duration = number(2, 5) + (GET_REMORT(ch)/3);
    af[0].bitvector2 = AFF2_ROOTED;

    accum_duration = FALSE;
    accum_affect = FALSE;
    to_room = "$n's feet are suddenly held!";
    to_vict = "&gRoots suddenly come from the ground and hold you&G!&n";
  break;

  case SPELL_DETECT_ALIGN:
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 23;
    else
    	af[0].duration = 12;
    af[0].bitvector = AFF_DETECT_ALIGN;
    accum_duration = TRUE;
    to_vict = "Your eyes tingle.";
    to_room = "$n's eyes tingle.";
    break;

  case SPELL_DETECT_INVIS:
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 23;
    else
    	af[0].duration = 12;
    af[0].bitvector = AFF_DETECT_INVIS;
    accum_duration = TRUE;
    to_vict = "Your eyes tingle.";
    to_room = "$n's eyes tingle.";
    break;

  case SPELL_DETECT_MAGIC:
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 23;
    else
    	af[0].duration = 12;
    af[0].bitvector = AFF_DETECT_MAGIC;
    accum_duration = TRUE;
    to_vict = "Your eyes tingle.";
    to_room = "$n's eyes tingle.";
    break;

  case SPELL_INFRAVISION:
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 23;
    else
    	af[0].duration = 12;
    af[0].bitvector = AFF_INFRAVISION;
    accum_duration = TRUE;
    to_vict = "Your eyes glow red.";
    to_room = "$n's eyes glow red.";
    break;

  case SPELL_INVISIBLE:
    if (!victim)
      victim = ch;

    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 20;
    else
    	af[0].duration = 10;
    af[0].modifier = -40;
    af[0].location = APPLY_AC;
    af[0].bitvector = AFF_INVISIBLE;
    accum_duration = TRUE;
    to_vict = "You vanish.";
    to_room = "$n slowly fades out of existence.";
    break;

  case SPELL_SUP_INVISIBLE:
    if (!victim)
      victim = ch;
    af[0].location = APPLY_AC;
    af[0].bitvector = AFF_INVISIBLE;
    if(!affected_by_spell(victim, SPELL_INVISIBLE))
    {
    af[0].duration = 20;
    af[0].modifier = -60 - (GET_REMORT(ch)*1.5);}
    else{
    affect_from_char(victim, SPELL_INVISIBLE);
    af[0].duration = 24;
    af[0].modifier = -80 - (GET_REMORT(ch)*1.5);}
    
    accum_duration = TRUE;
    to_vict = "You vanish.";
    to_room = "$n slowly fades out of existence.";
    break;

  case SPELL_POISON:
    if (mag_savingthrow(victim, savetype)) {
      send_to_char(NOEFFECT, ch);
      return;
    }
    if(!IS_NPC(ch) && !IS_NPC(victim)){
      if(!can_pk(ch, victim)){
        send_to_char("&WYou can not &Rkill &Wyour opponent right now.&n\r\n", ch);
        return;
      }
    }
    af[0].location = APPLY_STR;
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = GET_LEVEL(ch)/2;
    else
    	af[0].duration = GET_LEVEL(ch)/4;
    af[0].modifier = -2 - (GET_REMORT(ch));
    af[0].bitvector = AFF_POISON;
    to_vict = "You feel very sick.";
    to_room = "$n gets violently ill!";
    break;

  case SPELL_PROT_FROM_EVIL:
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 12;
    else
    	af[0].duration = 6;
    af[0].bitvector = AFF_PROTECT_EVIL;
    accum_duration = TRUE;
    to_vict = "You feel invulnerable!";
    break;

  case SPELL_SANCTUARY:
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 8;
    else
    	af[0].duration = 4;
    af[0].bitvector = AFF_SANCTUARY;

    accum_duration = TRUE;
    to_vict = "A white aura momentarily surrounds you.";
    to_room = "$n is surrounded by a white aura.";
    break;

  case SPELL_SLEEP:
    if (MOB_FLAGGED(victim, MOB_NOSLEEP))
      return;
    if (mag_savingthrow(victim, savetype))
      return;
    if(!IS_NPC(ch) && !IS_NPC(victim)){
      if(!can_pk(ch, victim)){
        send_to_char("&WYou can not &Rkill &Wyour opponent right now.&n\r\n", ch);
        return;
      }
    }
    af[0].duration = number(2,4);
    af[0].bitvector = AFF_SLEEP;

    if (GET_POS(victim) > POS_SLEEPING) {
      act("You feel very sleepy...  Zzzz......", FALSE, victim, 0, 0, TO_CHAR);
      act("$n goes to sleep.", TRUE, victim, 0, 0, TO_ROOM);
      GET_POS(victim) = POS_SLEEPING;
    }
    break;

  case SPELL_STRENGTH:
    if (GET_ADD(victim) == 100)
      return;

    af[0].location = APPLY_STR;
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = (GET_LEVEL(ch) / 10) + 4;
    else
    	af[0].duration = (GET_LEVEL(ch) / 20) + 4;
    af[0].modifier = 1 + (level > 18);
    accum_duration = TRUE;
    to_vict = "You feel stronger!";
    to_room = "$n feels stronger!";
    break;

  case SPELL_MAJOR_GLOBE:
    af[0].location = APPLY_INT;
    af[0].modifier = +3 + GET_REMORT(ch)/3;
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 4;
    else
    	af[0].duration = 2;
    accum_duration = FALSE;

    af[1].location = APPLY_WIS;
    af[1].modifier = +3 + GET_REMORT(ch)/3;
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[1].duration = 4;
    else
    	af[1].duration = 2;
    accum_duration = FALSE;

    af[2].location = APPLY_CON;
    af[2].modifier = +3 + GET_REMORT(ch)/3;
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[2].duration = 4;
    else
    	af[2].duration = 2;
    accum_duration = FALSE;

    af[3].location = APPLY_DEX;
    af[3].modifier = +3 + GET_REMORT(ch)/3;
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[3].duration = 4;
    else
    	af[3].duration = 2;
    accum_duration = FALSE;
    
    af[3].location = APPLY_STR;
    af[3].modifier = +3 + GET_REMORT(ch)/3;
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[3].duration = 4;
    else
    	af[3].duration = 2;
    accum_duration = FALSE;
    
    af[3].location = APPLY_CHA;
    af[3].modifier = +3 + GET_REMORT(ch)/3;
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[3].duration = 4;
    else
    	af[3].duration = 2;
    accum_duration = FALSE;

    to_vict = "You feel a great globe rising around your body.";
    break;

  case SPELL_ENHANCED_STRENGTH:
    if (GET_ADD(victim) == 100)
      return;

    af[0].location = APPLY_STR;
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 16;
    else
    	af[0].duration = 8;
    af[0].modifier = +4;
    accum_duration = TRUE;
    to_vict = "You feel stronger!";
    to_room = "$n feels stronger!";
    break;

  case SPELL_SENSE_LIFE:
    to_vict = "Your feel your awareness improve.";
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 18;
    else
    	af[0].duration = 9;
    af[0].bitvector = AFF_SENSE_LIFE;
    accum_duration = TRUE;
    break;

  case SPELL_CLAIRAUDIENCE:
    to_vict = "Your vents hear with clairaudience now!";
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 12;
    else
    	af[0].duration = 6;
    af[0].bitvector = AFF_SENSE_LIFE;
    accum_duration = TRUE;
    break;

  case SPELL_CLAIRVOYANCE:
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 12;
    else
    	af[0].duration = 6;
    af[0].bitvector = AFF_DETECT_INVIS;
    accum_duration = TRUE;
    to_vict = "Your eyes foresee with clairvoyance now!";
    break;

  case SPELL_WATERWALK:
   if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 24;
    else
    	af[0].duration = 16;
    af[0].bitvector = AFF_WATERWALK;
    accum_duration = TRUE;
    to_vict = "You feel webbing between your toes.";
    break;

  case SPELL_HASTE:
   if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 8;
    else
    	af[0].duration = 4;
    af[0].bitvector = AFF_HASTE;

    accum_duration = FALSE;
    to_vict = "Your adrenaline is gushing!";
    to_room = "$n starts to movement faster.";
    break;

  case SPELL_MANA_SHIELD:
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 8;
    else
    	af[0].duration = 4;
    af[0].bitvector = AFF_MANA_SHIELD;

    accum_duration = FALSE;
    to_vict = "You create a great aura of energy wich surrounds you.";
    to_room = "$n creates a great aura of energy around $m.";
    break;

  case SPELL_DAMNED_CURSE:
    if (mag_savingthrow(victim, savetype)) {
      send_to_char(NOEFFECT, ch);
      return;
    }
    if(!IS_NPC(ch) && !IS_NPC(victim)){
      if(!can_pk(ch, victim)){
        send_to_char("&WYou can not &Rkill &Wyour opponent right now.&n\r\n", ch);
        return;
      }
    }
    af[0].location = APPLY_HITROLL;
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 2 + (GET_LEVEL(ch)/24);
    else
    	af[0].duration = 1 + (GET_LEVEL(ch) / 24);
    af[0].modifier = -10 - (GET_LEVEL(ch) / 45);
    af[0].bitvector = AFF_DAMNED_CURSE;

    af[1].location = APPLY_DAMROLL;
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[1].duration = 2 + (GET_LEVEL(ch)/24);
    else
    	af[1].duration = 1 + (GET_LEVEL(ch) / 24);
    af[1].modifier = -10 - (GET_LEVEL(ch) / 45);
    af[1].bitvector = AFF_DAMNED_CURSE;

    accum_duration = TRUE;
    accum_affect = FALSE;
    to_room = "$n starts to shivers and suffer!";
    to_vict = "You feel a great pain all around your body!";
    break;

  case SPELL_PESTILENCE:
    if (mag_savingthrow(victim, savetype)) {
      send_to_char(NOEFFECT, ch);
      return;
    }
    if(!IS_NPC(ch) && !IS_NPC(victim)){
      if(!can_pk(ch, victim)){
        send_to_char("&WYou can not &Rkill &Wyour opponent right now.&n\r\n", ch);
        return;
      }
    }
    af[0].location = APPLY_HITROLL;
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 3 + (GET_LEVEL(ch) / 16);
    else
    	af[0].duration = 1 + (GET_LEVEL(ch) / 16);
    af[0].modifier = -16 - (GET_LEVEL(ch) / 35);
    af[0].bitvector = AFF_DAMNED_CURSE;

    af[1].location = APPLY_DAMROLL;
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[1].duration = 3 + (GET_LEVEL(ch) / 16);
    else
    	af[1].duration = 1 + (GET_LEVEL(ch) / 16);
    af[1].modifier = -16 - (GET_LEVEL(ch) / 35);
    af[1].bitvector = AFF_DAMNED_CURSE;

    accum_duration = TRUE;
    accum_affect = FALSE;
    to_room = "$n suffers stricken with nuisance!";
    to_vict = "You feel to be infected with nuisance!";
    break;

  case SPELL_PETRIFY:

   if (mag_savingthrow(victim, savetype))
   {
    send_to_char(NOEFFECT, ch);
      return;
   }


   if (mag_savingthrow(victim, savetype))
   {
    send_to_char(NOEFFECT, ch);
      return;
   }
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 3;
    else
    	af[0].duration = 2;
    af[0].bitvector = AFF_HOLDED;

    accum_duration = FALSE;
    to_vict = "All of your muscles turns stone.";
    to_room = "$n turns a great stone.";
    break;

  case SPELL_GOD_PACT:
    if (!(IS_GOOD(ch))) {
      send_to_char("You are much evil to make a pact with God!\r\n", ch);
      return;
    }

   if(AFF_FLAGGED(ch, AFF_SATAN))
   {
    send_to_char("You are protected by SATAN already!!!!!\r\n", ch) ;
    return ;
   }
    if (weather_info.sunlight == SUN_DARK)
    	if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    		af[0].duration = 2+(GET_LEVEL(ch)/80);
    	else
		af[0].duration = 1+(GET_LEVEL(ch)/80);
    else
    	if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    		af[0].duration = 2+(GET_LEVEL(ch)/40);
    	else
		af[0].duration = 1+(GET_LEVEL(ch)/40);

    af[0].bitvector = AFF_GOD;

    accum_duration = TRUE;
    to_vict = "A yellow aura momentarily surrounds you.";
    to_room = "$n is surrounded by a yellow aura.";
    break;

  case SPELL_SATAN_PACT:
    if (!(IS_EVIL(ch))) {
      send_to_char("You are much good to make a pact with Satan!\r\n", ch);
      return;
    }

    if(AFF_FLAGGED(ch, AFF_GOD))
    {
     send_to_char("You are protected by GOD already!!!!!\r\n", ch);
     return ;
   }

    if (weather_info.sunlight == SUN_DARK)
    	if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    		af[0].duration = 2+(GET_LEVEL(ch)/40);
    	else
		af[0].duration = 1+(GET_LEVEL(ch)/40);
    else
    	if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    		af[0].duration = 2+(GET_LEVEL(ch)/80);
    	else
		af[0].duration = 1+(GET_LEVEL(ch)/80);

    af[0].bitvector = AFF_SATAN;

    accum_duration = TRUE;
    to_vict = "A red aura momentarily surrounds you.";
    to_room = "$n is surrounded by a red aura.";
    break;

  case SPELL_MENTAL_RAGE:

   if (mag_savingthrow(victim, savetype))
   {
    send_to_char(NOEFFECT, ch);
      return;
   }

   if (mag_savingthrow(victim, savetype))
   {
    send_to_char(NOEFFECT, ch);
      return;
   }


    if(!IS_NPC(ch) && !IS_NPC(victim)){
      if(!can_pk(ch, victim)){
        send_to_char("&WYou can not &Rkill &Wyour opponent right now.&n\r\n", ch);
        return;
      }
    }
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 2;
    else
    	af[0].duration = 1;
    af[0].bitvector = AFF_HOLDED;

    accum_duration = FALSE;
    to_vict = "A flash of light obscures $n's vision and paralyse them.";
    to_room = "$n stops on the time...";
    break;

  case SPELL_HOLD_PERSON:

   if (mag_savingthrow(victim, savetype))
   {
    send_to_char(NOEFFECT, ch);
      return;
   }

   if (mag_savingthrow(victim, savetype))
   {
      send_to_char(NOEFFECT, ch);
      return;
   }

    if(!IS_NPC(ch) && !IS_NPC(victim)){
      if(!can_pk(ch, victim)){
        send_to_char("&WYou can not &Rkill &Wyour opponent right now.&n\r\n", ch);
        return;
      }
    }
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 2;
    else
    	af[0].duration = 1;
    af[0].bitvector = AFF_HOLDED;

    accum_duration = FALSE;
    to_vict = "All of your movements become motionless.";
    to_room = "$n stops all $m movements like a stone.";
    break;

  case SPELL_FLY:
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 7 + (level / 15);
    else
    	af[0].duration = 3 + level / 15;
    af[0].bitvector = AFF_FLY;
    accum_duration = TRUE;
    to_vict = "Your feet rise the floor.";
    to_room = "$n feet rise the floor.";
    break;

  case SPELL_VULCAN:
    af[0].location = APPLY_DAMROLL;
	if (weather_info.sunlight == SUN_DARK)
	  if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    		af[0].duration = 3;
	  else
		af[0].duration = 2;
	else
	  if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    		af[0].duration = 2;
	  else
		af[0].duration = 1;
    af[0].modifier = (GET_LEVEL(ch)/4);
    accum_duration = FALSE;
    to_vict = "&nYou are now THIRST of &RBlood&n!";
  break;

  case SPELL_ENTANGLE:
    if(!IS_NPC(ch) && !IS_NPC(victim)){
      if(!can_pk(ch, victim)){
        send_to_char("&WYou can not &Rkill &Wyour opponent right now.&n\r\n", ch);
        return;
      }
    }
    if (ROOM_FLAGGED(IN_ROOM(ch),ROOM_INDOORS)) {
      act("You feel something pounding up into the floor beneath you.",
          FALSE, victim, 0, 0, TO_CHAR);
      act("You hear a pounding under the floor below $n.", TRUE, victim,
          0, 0, TO_ROOM);
      return;
    }
    if (MOB_FLAGGED(victim,MOB_NOENTANGLE) || (number(1, 60) > number(30, 70)))/*mag_savingthrow(victim, savetype))*/ {
      act("Vines grow from the ground to entangle you, but you shrug them off.",
          FALSE, victim, 0, 0, TO_CHAR);
      act("Vines from the ground try to entangle $n, but can't get a grip.",
          TRUE, victim, 0, 0, TO_ROOM);
      return;
    }
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 2;
    else
    	af[0].duration = 1;
   /* if (GET_LEVEL(ch) < GET_LEVEL(victim))
    af[0].duration -= 1;*/
    af[0].bitvector = AFF_TANGLED;
    to_vict = "Vines suddenly grow up from the ground and entangle you!";
    to_room = "Vines grow up from the ground and thoroughly entangle $n.";

    if (GET_POS(ch) > POS_STUNNED) {
      if (!(FIGHTING(ch)))
        set_fighting(ch, victim);

      if (IS_NPC(ch) && IS_NPC(victim) && victim->master &&
          !number(0, 10) && IS_AFFECTED(victim, AFF_CHARM) &&
          (victim->master->in_room == ch->in_room)) {
        if (FIGHTING(ch))
          stop_fighting(ch);
        hit(ch, victim->master, TYPE_UNDEFINED);
        return;
      }
    }
    if (GET_POS(victim) > POS_STUNNED && !FIGHTING(victim)) {
      set_fighting(victim, ch);
      if (MOB_FLAGGED(victim, MOB_MEMORY) && !IS_NPC(ch) &&
          (GET_LEVEL(ch) < LVL_IMMORT))
        remember(victim, ch);
    }
    break;

   case SPELL_FIRESHIELD:
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 7;
    else
    	af[0].duration  = 4;
    af[0].bitvector = AFF_FIRESHIELD;
    accum_duration  = FALSE;
    to_vict = "You start glowing red.";
    to_room = "$n is surrounded by a glowing red aura.";
    break;

  case SPELL_ADRENALINE:
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 14;
    else
    	af[0].duration = 7;
    af[0].bitvector = AFF_REGEN;
    accum_duration = FALSE;
    to_vict = "Your feel your regeneration power rising.";
    to_room = "$n starts to regenerate his body.";
    break;

  case SPELL_FORCE_SHIELD:
    af[0].location = APPLY_AC;
    af[0].modifier = -30-(GET_LEVEL(ch)/20) - (GET_REMORT(ch)*1.5);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 10;
    else
    	af[0].duration = 10;
    accum_duration = TRUE;
    to_vict = "You feel a force barrier protecting you.";
    break;

  case SPELL_ENHANCED_ARMOR:
    if(!affected_by_spell(victim, SPELL_ARMOR))
    {
      af[0].location = APPLY_AC;
      af[0].modifier = -5-(GET_LEVEL(ch)/40) - (GET_REMORT(ch)*1.5);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 24;
    else
      af[0].duration = 20;
      accum_duration = FALSE;
      to_vict = "You feel your protections rising.";
    } else {
      affect_from_char(victim, SPELL_ARMOR);
      af[0].location = APPLY_AC;
      af[0].modifier = -40  - (GET_REMORT(ch)*1.5);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 24;
    else
      af[0].duration = 20;
      accum_duration = FALSE;
      to_vict = "You feel someone protecting you.";
    }
    break;

  case SPELL_SLOW:

    if (mag_savingthrow(victim, savetype))
   {
    send_to_char(NOEFFECT, ch);
    return;
   }

    af[0].location = APPLY_HITROLL;
    af[0].modifier = -10 - (GET_REMORT(ch)*2);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 5;
    else
    	af[0].duration = 3;
    af[0].bitvector2 = AFF2_SLOW;
    accum_duration = FALSE;
    to_vict = "You feel your power lost.";
    break;

  case SPELL_THOUGHT_SHIELD:
    af[0].location = APPLY_AC;
    af[0].modifier = -10 - (GET_REMORT(ch)*1.5);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 7;
    else
    	af[0].duration = 4;
    accum_duration = FALSE;
    to_vict = "A shield of thought is making protection around you.";
    break;

case SPELL_BIOFEEDBACK:
    af[0].location = APPLY_AC;
    af[0].modifier = -15- (GET_REMORT(ch)*1.5);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 11;
    else
    	af[0].duration = 6;
    accum_duration = FALSE;
    to_vict = "You feel more armored.";
    break;

case SPELL_MENTAL_BARRIER:
    af[0].location = APPLY_AC;
    af[0].modifier = -12- (GET_REMORT(ch)*1.5);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 17;
    else
    	af[0].duration = 8;
    accum_duration = FALSE;
    to_vict = "You feel a mental barrier protecting you.";
    break;

case SPELL_INERTIAL_BARRIER:
    af[0].location = APPLY_AC;
    af[0].modifier = -18 - (GET_REMORT(ch)*1.5);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 23;
    else
    	af[0].duration = 12;
    accum_duration = FALSE;
    to_vict = "You feel an inertial barrier protecting you.";
    break;

case SPELL_CELL_ADJUSTMENT:
    af[0].location = APPLY_HIT;
    af[0].modifier = ((GET_LEVEL(ch) * 3) + (GET_REMORT(ch)*100));
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 24;
    else
    	af[0].duration = 15;
    accum_duration = FALSE;
    to_vict = "&RYou adjust your cells and feel more strong.&n";
    break;

case SPELL_IRON_SKIN:
    if(affected_by_spell(victim, SPELL_PLATINUM_SKIN) ||
    	affected_by_spell(victim, SPELL_DIAMOND_SKIN))
    {
    	send_to_char("Your skin has already changed.\r\n", victim);
    	return;
    }
    af[0].location = APPLY_AC;
    af[0].modifier = -25 - (GET_REMORT(ch)/4);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 24;
    else
    	af[0].duration = 15;
    accum_duration = FALSE;
    to_vict = "You feel your skin turning iron.";
    break;

case SPELL_PLATINUM_SKIN:
    if(!affected_by_spell(victim, SPELL_IRON_SKIN))
    {
    	send_to_char("You need to turn in iron your skin first.\r\n", victim);
    	return;
    }
    affect_from_char(victim, SPELL_IRON_SKIN);
    af[0].location = APPLY_AC;
    af[0].modifier = -50 - (GET_REMORT(ch)/2);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 24;
    else
    	af[0].duration = 15;
    accum_duration = FALSE;
    to_vict = "You feel your skin turning platinum.";
    break;

case SPELL_DIAMOND_SKIN:
    if(!affected_by_spell(victim, SPELL_PLATINUM_SKIN))
    {
    	send_to_char("You need to turn in platinum your skin first.\r\n",victim);
    	return;
    }
    affect_from_char(victim, SPELL_PLATINUM_SKIN);
    af[0].location = APPLY_AC;
    af[0].modifier = -75 - (GET_REMORT(ch));
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 24;
    else
    	af[0].duration = 15;
    accum_duration = FALSE;
    to_vict = "You feel your skin turning diamond.";
    break;

case SPELL_HOLY_SHIELD:
    af[0].location = APPLY_AC;
    af[0].modifier = -35 - (GET_REMORT(ch)*1.5);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
    	af[0].duration = 9;
    else
    	af[0].duration = 5;
    accum_duration = FALSE;
    to_vict = "You feel the Holy Shield protecting you.";
    break;

  case SPELL_PARADI_CHRYSALIS:
	  x = number(1, 8);
	  y = number(1, 8);
    if (x < y)
    {
    	af[0].location = APPLY_AC;
    	af[0].modifier = -25 - (GET_REMORT(ch));
    	if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
        	af[0].duration = 9;
	else
    		af[0].duration = 5;
    	af[1].location = APPLY_SAVING_SPELL;
    	af[1].modifier = -5 - (GET_REMORT(ch)*2);
    	if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
        	af[1].duration = 9;
	else
    		af[1].duration = 5;
    	accum_duration = FALSE;
    	to_vict = "You pray for some protection and receive a part of it!";
    }
    else if (x > y)
    {
    	af[0].location = APPLY_AC;
    	af[0].modifier = -45 - (GET_REMORT(ch)*1.5);
    	if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
        	af[0].duration = 9;
	else
    		af[0].duration = 5;
    	af[1].location = APPLY_SAVING_SPELL;
    	af[1].modifier = -7 - (GET_REMORT(ch)*2.5);
    	if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
        	af[1].duration = 9;
	else
    		af[1].duration = 5;
    	accum_duration = FALSE;
    	to_vict = "You pray for some protection and receive a part of it!";
    }
    else
    {
    	af[0].location = APPLY_AC;
    	af[0].modifier = -70 - (GET_REMORT(ch)*2);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
        af[0].duration = 13;
    else
    	af[0].duration = 7;
    	af[1].location = APPLY_SAVING_SPELL;
    	af[1].modifier = -10 - (GET_REMORT(ch)*3);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
        af[1].duration = 13;
    else
    	af[1].duration = 7;
    	accum_duration = FALSE;
    	to_vict = "You pray for some protection and receive the Paradise Chrysalis!!!";
    }
    break;

  case SPELL_ABUTILON:
    x = number(1,8);
    y = number(1,8);

	if (x < y)
    {
    	af[0].location = APPLY_DAMROLL;
    	if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
         af[0].duration = 9;
	else
	 af[0].duration = 5;
    	af[0].modifier = 5;
    	af[1].location = APPLY_HITROLL;
    	if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
         af[1].duration = 9;
	else
	 af[1].duration = 5;
    	af[1].modifier = 2;
    	accum_duration = FALSE;
    	to_vict = "You receive the abutilon of your God!";
    }
    else if (x > y)
    {
    	af[0].location = APPLY_DAMROLL;
    	if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
         af[0].duration = 9;
	else
	 af[0].duration = 5;
	af[0].modifier = 2;
    	af[1].location = APPLY_HITROLL;
    	if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
         af[1].duration = 9;
	else
	 af[1].duration = 5;
    	af[1].modifier = 5;
    	accum_duration = FALSE;
    	to_vict = "You receive the a abutilon of your God!";
    }
    else
    {
    	af[0].location = APPLY_DAMROLL;
    	if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
         af[0].duration = 13;
	else
	 af[0].duration = 7;
	af[0].modifier = 7;
    	af[1].location = APPLY_HITROLL;
    	if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
         af[1].duration = 13;
	else
	 af[1].duration = 7;	
    	af[1].modifier = 7;
    	accum_duration = FALSE;
    	to_vict = "Your God recompensates you with all abutilon of heaven!!!";
    }
    break;

  case SPELL_FRIENDS:
    af[0].location = APPLY_CHA;
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3)){
     af[0].duration = 19;
     af[0].modifier = +4 + (GET_REMORT(ch)/5);}
    else{
     af[0].modifier = +4 + (GET_REMORT(ch)/10);
     af[0].duration = 10;}
    accum_duration = FALSE;
    to_vict = "You feel your charism rising.";
    break;
 
case SPELL_EMPATHY:
    af[0].location = APPLY_CHA;
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3)){
     af[0].duration = 11;
     af[0].modifier = +4 + (GET_REMORT(ch)/5);}
    else{
     af[0].modifier = +4 + (GET_REMORT(ch)/10);
     af[0].duration = 6;}
    accum_duration = FALSE;
    to_vict = "You feel more charismatic now.";
    break;

case SPELL_INTEL_FORTRESS:
    af[0].location = APPLY_WIS;
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3)){
     af[0].duration = 7;
     af[0].modifier = +5 + (GET_REMORT(ch)/5);}
    else{
     af[0].modifier = +5 + (GET_REMORT(ch)/10);
     af[0].duration = 4;}
    accum_duration = FALSE;
    to_vict = "You feel your wisdom increasing";
    break;

case SPELL_ADREN_CONTROL:
    af[0].location = APPLY_DEX;
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3)){
     af[0].duration = 7;
     af[0].modifier = +4 + (GET_REMORT(ch)/5);}
    else{
     af[0].duration = 4;
     af[0].modifier = +4 + (GET_REMORT(ch)/10);}
     
    accum_duration = FALSE;
    to_vict = "You feel your dexterity rising";
    break;

case SPELL_WHEEL_OF_FORTUNE:

	send_to_char("&n..&Wthe Wheel Of Fortune turns..&n\r\n", ch);

	if (number(1, 10) > 5) {

		send_to_char("&WThe &yFate&W has come out to improve your &Cmental&W traits..&n\r\n", ch);

		if (number(1, GET_INT(ch)) > 10) {

			send_to_char("&BYou feel more Intelligent!&n\r\n", ch);
			act("&n$n &Bglows&n!", FALSE, ch, 0, 0, TO_ROOM);

			af[0].location = APPLY_INT;
			af[0].modifier = number(1, 3);
			af[0].duration = number(1, 8);

			af[3].location = APPLY_STR;
			af[3].modifier = -1;
			af[3].duration = number(1, 8);

			imp = TRUE;
		}

		if (number(1, GET_WIS(ch)) > 10) {

			send_to_char("&CYou feel more Wise!&n\r\n", ch);
			act("&n$n &Cglows&n!", FALSE, ch, 0, 0, TO_ROOM);

			af[1].location = APPLY_WIS;
			af[1].modifier = number(1, 3);
			af[1].duration = number(1, 8);

			af[4].location = APPLY_DEX;
			af[4].modifier = -1;
			af[4].duration = number(1, 8);

			imp = TRUE;
		}

		if (number(1, GET_CHA(ch)) > 10) {

			send_to_char("&YYou feel more Charismatic!&n\r\n", ch);
			act("&n$n &Yglows&n!", FALSE, ch, 0, 0, TO_ROOM);

			af[2].location = APPLY_CHA;
			af[2].modifier = number(1, 3);
			af[2].duration = number(1, 8);

			af[5].location = APPLY_CON;
			af[5].modifier = -1;
			af[5].duration = number(1, 8);

			imp = TRUE;
		}

	} else {

		send_to_char("&WThe &yFate&W has come out to improve your &Rphysical&W traits..&n\r\n", ch);

		if (number(1, GET_STR(ch)) > 10) {

			send_to_char("&RYou feel more Strong!&n\r\n", ch);
			act("&n$n &Rglows&n!", FALSE, ch, 0, 0, TO_ROOM);

			af[0].location = APPLY_STR;
			af[0].modifier = number(1, 3);
			af[0].duration = number(1, 8);

			af[3].location = APPLY_INT;
			af[3].modifier = -1;
			af[3].duration = number(1, 8);
			
			imp = TRUE;
		}

		if (number(1, GET_DEX(ch)) > 10) {

			send_to_char("&GYou feel more Agile!&n\r\n", ch);
			act("&n$n &Gglows&n!", FALSE, ch, 0, 0, TO_ROOM);

			af[1].location = APPLY_DEX;
			af[1].modifier = number(1, 3);
			af[1].duration = number(1, 8);

			af[4].location = APPLY_WIS;
			af[4].modifier = -1;
			af[4].duration = number(1, 8);

			imp = TRUE;
		}

		if (number(1, GET_CON(ch)) > 10) {

			send_to_char("&yYou feel more Energetic!&n\r\n", ch);
			act("&n$n &yglows&n!", FALSE, ch, 0, 0, TO_ROOM);

			af[2].location = APPLY_CON;
			af[2].modifier = number(1, 3);
			af[2].duration = number(1, 8);

			af[5].location = APPLY_CHA;
			af[5].modifier = -1;
			af[5].duration = number(1, 8);
			
			imp = TRUE;
		}

	}

	if (!imp) { 
		send_to_char("&W..but nothing happened. &yFate&W is a hard thing to handle, isnt it?&n\r\n", ch);
		return;
	}

	accum_duration = FALSE;
	accum_affect = FALSE;
    break;

case SPELL_DUMBNESS:
    af[0].location = APPLY_INT;
    if (mag_savingthrow(victim, savetype))
      af[0].duration = 1;
    else
      af[0].duration = 4;
    af[0].modifier = -6 - (GET_LEVEL(ch)/50);
    accum_duration = TRUE;
    to_vict = "You feel dumber somehow.";
    break;

  case SPELL_PAIN:
    if (mag_savingthrow(victim, savetype)) {
      send_to_char(NOEFFECT, ch);
      return;
    }
    if(!IS_NPC(ch) && !IS_NPC(victim)){
      if(!can_pk(ch, victim)){
        send_to_char("&WYou can not &Rkill &Wyour opponent right now.&n\r\n", ch);
        return;
      }
    }
    af[0].location = APPLY_DAMROLL;
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3)){
     af[0].duration = 9;
     af[0].modifier = -10 - (GET_REMORT(ch)/5);}
    else{
     af[0].duration = 5;
     af[0].modifier = -10 - (GET_REMORT(ch)/10);}

    accum_duration = FALSE;
    accum_affect = TRUE;
    to_room = "$n feels a violent grief about $m body!";
    to_vict = "You feel a violent grief taking control of your body!";
    break;

  case SPELL_DISPLACEMENT:
    if (mag_savingthrow(victim, savetype)) {
      send_to_char(NOEFFECT, ch);
      return;
    }
    if(!IS_NPC(ch) && !IS_NPC(victim)){
      if(!can_pk(ch, victim)){
        send_to_char("&WYou can not &Rkill &Wyour opponent right now.&n\r\n", ch);
        return;
      }
    }
    af[0].location = APPLY_HITROLL;
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3)){
     af[0].duration = 11;
     af[0].modifier = -5 - (GET_REMORT(ch)/5);}
    else{
     af[0].duration = 6;
     af[0].modifier = -5 - (GET_REMORT(ch)/10);}

    accum_duration = FALSE;
    accum_affect = TRUE;
    to_room = "$n feels a strange displacement!";
    to_vict = "You feel a uncomfortable displacement!";
    break;

  case SPELL_PHANTOM_ARMOR:
    af[0].location = APPLY_AC;
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
     af[0].duration = 7;
    else
     af[0].duration = 4;
    af[0].modifier = -30 - (GET_REMORT(ch) * 1.5);
    accum_duration = FALSE;
    to_vict = "The spirits on the undead come forth to protect you!";
    to_room = "$n is surrounded by a hoard of phantoms.";
    break;

  case SPELL_FLESH_ARMOR:
    af[0].location = APPLY_AC;
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
     af[0].duration = 9;
    else
     af[0].duration = 5;
    af[0].modifier = -25 - (GET_REMORT(ch) * 1.5);
    accum_duration = FALSE;
    to_vict = "A strange flesh coat starts to protect you!";
    to_room = "$n is wraped by a flesh coat.";
    break;

  case SPELL_SACRIFICE:
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3)) {
     af[0].location = APPLY_WIS;
     af[0].duration = 11;
     af[0].modifier = -1; }
    else{
     af[0].location = APPLY_WIS;
     af[0].duration = 6;
     af[0].modifier = -1;}

    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3)) {
     af[1].location = APPLY_INT;
     af[1].duration = 11;
     af[1].modifier = -1; }
    else{
    af[1].location = APPLY_INT;
    af[1].duration = 6;
    af[1].modifier = -1;}

    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3)) {
     af[2].location = APPLY_DEX;
     af[2].duration = 11;
     af[2].modifier = -1; }
    else {
     af[2].location = APPLY_DEX;
     af[2].duration = 6;
     af[2].modifier = -1;}

    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3)) {
     af[3].location = APPLY_CON;
     af[3].duration = 11;
     af[3].modifier = -1; }
    else {
     af[3].location = APPLY_CON;
     af[3].duration = 6;
     af[3].modifier = -1;}

    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3)) {
     af[4].location = APPLY_CHA;
     af[4].duration = 11;
     af[4].modifier = -1; }
    else {
     af[4].location = APPLY_CHA;
     af[4].duration = 6;
     af[4].modifier = -1;}

    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3)) {
     af[5].location = APPLY_STR;
     af[5].duration = 11;
     af[5].modifier = 5 + (GET_REMORT(ch)/2); }
    else {
     af[5].location = APPLY_STR;
     af[5].duration = 6;
     af[5].modifier = 5 + (GET_REMORT(ch)/4);}
    
    accum_duration = FALSE;
    to_vict = "You sacrifice yourself and become stronger";
    to_room = "$n makes a sacrifice and become stronger.";
    break;

  case SPELL_SPECTRAL_WINGS:
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
     af[0].duration = 24;
    else
     af[0].duration = 18;
    af[0].bitvector = AFF_FLY;
    to_vict = "You sprout a pair of spectral wings and rise from the ground!";
    to_room = "$n sprouts a pair of spectral wings and rises from the ground.";
    break;

  case SPELL_HOLY_FURY:
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
     af[0].duration = 11;
    else
     af[0].duration = 6;
    af[0].bitvector = AFF_HASTE;
    accum_duration = FALSE;
    accum_affect = FALSE;
    to_vict = "You feel like you can take on an army!";
    to_room = "$n gets a strange furious scowl on $m face and speeds up.";
    break;

  case SPELL_CHAMPION_STRENGTH:
    if (GET_ADD(victim) != 100) {
      if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3)){
       af[0].location = APPLY_STR;
       af[0].duration = 9;
       af[0].modifier = +3 + (GET_REMORT(ch)/2);}
      else{
       af[0].location = APPLY_STR;
       af[0].duration = 5;
       af[0].modifier = +3 + (GET_REMORT(ch)/4);}
    }
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3)){
       af[1].location = APPLY_DAMROLL;
       af[1].duration = 9;
       af[1].modifier = +6 + (GET_REMORT(ch)/3);}
    else{
       af[1].location = APPLY_DAMROLL;
       af[1].duration = 5;
       af[1].modifier = +3 + (GET_REMORT(ch)/6);}
    accum_duration = FALSE;
    accum_affect = FALSE;
    to_vict = "You feel like you have the strength of a champion!";
    break;

  case SPELL_TERROR:
   if (mag_savingthrow(victim, savetype))
   {
    send_to_char(NOEFFECT, ch);
      return;
   }

   if (mag_savingthrow(victim, savetype))
   {
    send_to_char(NOEFFECT, ch);
      return;
   }

    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3)){
     WAIT_STATE(victim, PULSE_VIOLENCE * 20); // pequeno tempo para recuperar-se do susto.
     af[0].duration = 2;}
    else{
     WAIT_STATE(victim, PULSE_VIOLENCE * 10); // pequeno tempo para recuperar-se do susto.
     af[0].duration = 1;} // para recuperar-se do trauma demora um pouco mais.
    af[0].bitvector2 = AFF2_TERROR;
    to_room = "$n has fear in the eyes!";
    to_vict = "You are panicky!!";
     break;

  case SPELL_AVERSION:

   if (mag_savingthrow(victim, savetype))
   {
    send_to_char(NOEFFECT, ch);
      return;
   }

   if (mag_savingthrow(victim, savetype))
   {
    send_to_char(NOEFFECT, ch);
      return;
   }

    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3)){
     WAIT_STATE(victim, PULSE_VIOLENCE * 20); // pequeno tempo para recuperar-se do susto.
     af[0].duration = 2;}
    else{
     WAIT_STATE(victim, PULSE_VIOLENCE * 10); // pequeno tempo para recuperar-se do susto.
     af[0].duration = 1;} // para recuperar-se do trauma demora um pouco mais.
    af[0].bitvector2 = AFF2_TERROR;
    to_room = "$n causes an aversion in this place!";
    to_vict = "You feel the fear taking control of your body!";
      break;

  case SPELL_CONFUSION:

   if (mag_savingthrow(victim, savetype))
   {
    send_to_char("Weak, too weak\r\n", ch);
      return;
   }

   if (mag_savingthrow(victim, savetype))
   {
    send_to_char(NOEFFECT, ch);
      return;
   }

    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3)){
     WAIT_STATE(victim, PULSE_VIOLENCE * 20); // pequeno tempo para recuperar-se do susto.
     af[0].duration = 2;}
    else{
     WAIT_STATE(victim, PULSE_VIOLENCE * 10); // pequeno tempo para recuperar-se do susto.
     af[0].duration = 1;} // para recuperar-se do trauma demora um pouco mais.
    af[0].bitvector2 = AFF2_TERROR;
    to_room = "$n gestures and everyone in this place looks confused!";
    to_vict = "You feel confused and cannot control your body!!!";
      break;

  case SPELL_ENGULFING_DARKNESS:
    af[0].location = APPLY_AC;
	if (weather_info.sunlight == SUN_DARK)
		af[0].modifier = (GET_LEVEL(ch)/3) + (GET_REMORT(ch)*5);
	else if (weather_info.sunlight == SUN_LIGHT)
		af[0].modifier = (GET_LEVEL(ch)/8) + (GET_REMORT(ch)*2);
	else
		af[0].modifier = (GET_LEVEL(ch)/4) + (GET_REMORT(ch)*3);
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
     af[0].duration = 9;
    else
     af[0].duration = 5;
    to_room = "$n is surrended by a wicked aura!";
    to_vict = "Darkness engulfs your body!";
    break;

  case SPELL_BANSHEE_AURA:
    if (weather_info.sunlight == SUN_DARK)
	if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
     	 af[0].duration = 17;
    	else
	 af[0].duration = 9;
    else
	if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
     	 af[0].duration = 9;
        else
	 af[0].duration = 5;
    af[0].bitvector2 = AFF2_BANSHEE;
    to_vict = "URGH!!! Your blood boils!";
    break;

  case SPELL_BLINK:
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
     af[0].duration = 13;
    else
     af[0].duration = 7;
    af[0].bitvector2 = AFF2_BLINK;
    to_vict = "You don't feel any different.";
    to_room = "You see $n shift a few feet away.";
    break;

  case SPELL_IRON_BODY:
  af[0].location = APPLY_AC;
  if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
   af[0].duration = number(4, 6);
  else
   af[0].duration = number(2, 3);
  af[0].modifier = -20 - (GET_REMORT(ch)*1.5); 
  af[0].bitvector2 = AFF2_IRON_BODY;
  to_vict = "&KYou feel your body turning to iron.&n";
  accum_duration = TRUE;
  break;

  case SPELL_DEATHDANCE:
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
     af[0].duration = 17;
    else
     af[0].duration = 9;
    af[0].bitvector2 = AFF2_DEATHDANCE;
    to_vict = "You feel your life take on a whole new meaning....";
    to_room = "A wave of death dances forth from $n";
    break;

  case SPELL_MARTIAL_TRANCE:
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
     af[0].duration = 15;
    else
     af[0].duration = 8;
    af[0].bitvector = AFF_REGEN;
    accum_duration = FALSE;
    to_vict = "You start doing a martial trance";
    to_room = "$n starts doing a strange trance";
    break;

  case SPELL_PROT_FIRE:
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
     af[0].duration = 19;
    else
     af[0].duration = 10;
    af[0].bitvector2 = AFF2_PROT_FIRE;
    accum_duration = FALSE;
    to_vict = "You feel a shell of insulation form around your body.";
    break;

case SPELL_WRAITHFORM:
    if(GET_RACE(ch) == RACE_HIGH_ELF || (GET_RACE(ch) == RACE_DRACONIAN &&number(1,12) == 3))
     af[0].duration = 15;
    else
     af[0].duration = 8;
    af[0].bitvector2 = AFF2_PASSDOOR;
    to_vict = "You turn translucent!";
    to_room = "$n turns translucent!";
    break;
  }

  /*
   * If this is a mob that has this affect set in its mob file, do not
   * perform the affect.  This prevents people from un-sancting mobs
   * by sancting them and waiting for it to fade, for example.
   */
  if (IS_NPC(victim) && !affected_by_spell(victim, spellnum))
    for (i = 0; i < MAX_SPELL_AFFECTS; i++)
      if (AFF_FLAGGED(victim, af[i].bitvector) || AFF2_FLAGGED(victim,
af[i].bitvector2)
          || AFF3_FLAGGED(victim, af[i].bitvector3)) {
        send_to_char(NOEFFECT, ch);
        return;
      }

  /*
   * If the victim is already affected by this spell, and the spell does
   * not have an accumulative effect, then fail the spell.
   */
  if (affected_by_spell(victim,spellnum) && !(accum_duration||accum_affect))
{
    send_to_char(NOEFFECT, ch);
    return;
  }

  for (i = 0; i < MAX_SPELL_AFFECTS; i++)
    if (af[i].bitvector || af[i].bitvector2 || af[i].bitvector3 ||
(af[i].location != APPLY_NONE))
      affect_join(victim, af+i, accum_duration, FALSE, accum_affect, FALSE);

  if (to_vict != NULL)
    act(to_vict, FALSE, victim, 0, ch, TO_CHAR);
  if (to_room != NULL)
    act(to_room, TRUE, victim, 0, ch, TO_ROOM);
}


/*
* This function is used to provide services to mag_groups.  This function
* is the one you should change to add new group spells.
*/

void perform_mag_groups(int level, struct char_data * ch,
                        struct char_data * tch, int spellnum, int savetype)
{
  switch (spellnum) {
    case SPELL_GROUP_HEAL:
    mag_points(level, ch, tch, SPELL_HEAL, savetype);
    break;
  case SPELL_GROUP_ARMOR:
    mag_affects(level, ch, tch, SPELL_ARMOR, savetype);
    break;
  case SPELL_GROUP_RECALL:
    spell_recall(level, ch, tch, NULL);
    break;
  case SPELL_GROUP_FLY:
    mag_affects(level, ch, tch, SPELL_FLY, savetype);
    break;
  case SPELL_GROUP_BLESS:
    mag_affects(level, ch, tch, SPELL_BLESS, savetype);
    break;
  }
}


/*
* Every spell that affects the group should run through here
* perform_mag_groups contains the switch statement to send us to the right
* magic.
*
* group spells affect everyone grouped with the caster who is in the room,
* caster last.
*
* To add new group spells, you shouldn't have to change anything in
* mag_groups -- just add a new case to perform_mag_groups.
*/


void mag_groups(int level, struct char_data * ch, int spellnum, int
savetype)
{
  struct char_data *tch, *k;
  struct follow_type *f, *f_next;

  if (ch == NULL)
    return;

  if (!AFF_FLAGGED(ch, AFF_GROUP))
    return;
  if (ch->master != NULL)
    k = ch->master;
  else
    k = ch;
  for (f = k->followers; f; f = f_next) {
    f_next = f->next;
    tch = f->follower;
    if (tch->in_room != ch->in_room)
      continue;
    if (!AFF_FLAGGED(tch, AFF_GROUP))
      continue;
    if (AFF2_FLAGGED(tch, AFF2_TRANSFORM))
      continue;
    if (ch == tch)
      continue;
    perform_mag_groups(level, ch, tch, spellnum, savetype);
  }

  if ((k != ch) && AFF_FLAGGED(k, AFF_GROUP))
    perform_mag_groups(level, ch, k, spellnum, savetype);

  perform_mag_groups(level, ch, ch, spellnum, savetype);
}


/*
* mass spells affect every creature in the room except the caster.
*
* No spells of this class currently implemented as of Circle 3.0.
*/

void mag_masses(int level, struct char_data * ch, int spellnum, int savetype)
{
  struct char_data *tch, *tch_next;

  for (tch = world[ch->in_room].people; tch; tch = tch_next) {
    tch_next = tch->next_in_room;
    if (tch == ch)
      continue;

    switch (spellnum) {
    }
  }
}


/*
* Every spell that affects an area (room) runs through here.  These are
* generally offensive spells.  This calls mag_damage to do the actual
* damage -- all spells listed here must also have a case in mag_damage()
* in order for them to work.
*
*  area spells have limited targets within the room.
*/

void mag_areas(int level, struct char_data * ch, int spellnum, int savetype)
{
  struct char_data *tch, *next_tch;
  const char *to_char = NULL, *to_room = NULL;
  int i;
  int num = 0 ;

  if (ch == NULL)
    return;

  /*
   * to add spells to this fn, just add the message here plus an entry
   * in mag_damage for the damaging part of the spell.
   */
  switch (spellnum) {
  case SPELL_EARTHQUAKE:
    to_char = "You gesture and the earth begins to shake all around you!";
    to_room ="$n gracefully gestures and the earth begins to shake violently!";
    break;
  case SPELL_END_WORLD:
    to_char = "You gesture and a explosion disintegrate and reintegrate the room!";
    to_room ="$n gracefully gestures and the room suddenly disappear and appear!";
    break;
  case SPELL_ICESTORM:
    to_char = "You throw a ice tempest on the room!";
    to_room ="$n throws a ice tempest on the room!";
    break;
  case SPELL_METEORSTORM:
    to_char = "You throw a meteor storm on the room!";
    to_room ="$n throws a meteor storm on the room!";
    break;
  case SPELL_FIRESTORM:
    to_char = "You gesture and a fire storm makes the room burn!";
    to_room ="$n makes strange gestures and a fire storm hit the room!";
    break;
  case SPELL_ELETRICSTORM:
    to_char = "You gesture and a eletric storm makes the room burn!";
    to_room ="$n makes strange gestures and a eletric storm hit the room!";
    break;
  case SPELL_MINUTE_METEOR:
    to_char = "You gesture and a lot of meteor falls in the room!";
    to_room ="$n makes strange gestures and a lot of Meteor falls in the room!";
    break;
  case SPELL_CALL_LIGHTNING:
    to_room ="$n calls all the lightning power of the air in the room!";
    to_char ="You call the lightning power in the room.";
    break;
  case SPELL_GREASE:
    to_char = "You gesture and 'KABUMM' explode the Grease.";
    to_room ="$n makes strange gestures and 'KABUM' explode the Grease.";
    break;
  case SPELL_CROMATIC_ORB:
    to_char = "You gesture and makes a incredible cromatic orb.";
    to_room ="$n makes strange gestures and makes a incredible cromatic orb.";
    break;
  case SPELL_AREA_LIGHTNING:
    to_char = "You gesture and call down lights in this place.";
    to_room ="$n makes strange gestures and get call down lights in this place.";
    break;
  case SPELL_SKULL_TRAP:
    to_char = "You gesture and throw a powerful skull which explodes.";
    to_room ="$n throws a strange skull in the center of the room which explodes.";
    break;
case SPELL_METEOR_SHOWER:
    to_char = "You call the iron meteors with your magic!";
    to_room = "$n glows and the rain of iron meteors comes suddenly!";
    break;
  case SPELL_BLADEBARRIER:
    to_room ="$n laughs and attempts to put up a BLADE BARRIER!";
    to_char ="You concentrate on swords.  Swords. SWORDS!!!";
    break;
  case SPELL_COMMAND:
    to_room ="$n commands and throw against all fire stones!";
    to_char ="You glow and command a rain of fire stone against all.";
    break;
  case SPELL_GEYSER:
    to_room ="$n glows and creates a greate fire EFFLORESCENSE!";
    to_char ="You concentrate and create a fire EFFLORESCENSE!!!";
    break;
  case SPELL_ARMAGEDDOM:
    to_room ="$n laughs and call down the strength of armageddom to the room!";
    to_char ="You laugh and call down the armageddom to the place!";
    break;
  case SPELL_BLIZZARD:
    to_room ="$n conjures a great number of SNOW BLOCKS falling in the  room!!";
    to_char ="You concentrate a conjure SNOW BLOCKS to fall in the room!";
    break;
  case SPELL_FIREBALL:
    to_room ="With all his power, $n explodes a FIREBALL against all! BUMMM!";
    to_char ="You utter the power of a FIREBALL which explodes BUMMM!!!";
    break;
  case SPELL_HOLY_SHOUT:
    to_char = "The world shook as you issue holy shout!";
    to_room = "$n opens the holy bible and said the holy word!";
    break;
  case SPELL_DEATH_RIPPLE:
    to_char = "Your lips utters the hymn of death!";
    to_room = "$n utters the word of DEATH!";
    break;
  case SPELL_DEATH_WAVE:
    to_char = "You sing the hymn of death!";
    to_room = "$n shouts the words of DEATH! Deadly wave destroys all life!";
  break;
  case SPELL_SUMMON_BAHAMUT:
    to_char = "You summon the dragon king to purge the land with a GIGA FLARE!";
    to_room ="$n gracefully gestures and the dragon king purges the land with its GIGA FLARE!";
  break;
  case SPELL_TALOS:
    to_char = "You sing the hymn of TALOS!";
    to_room = "$n shouts the words of TALOS!!";
    break ;
  }

  if (to_char != NULL)
    act(to_char, FALSE, ch, 0, 0, TO_CHAR);
  if (to_room != NULL)
    act(to_room, FALSE, ch, 0, 0, TO_ROOM);


  for (tch = world[ch->in_room].people; tch; tch = next_tch) {
    next_tch = tch->next_in_room;

    /*
     * The skips: 1: the caster
     *            2: immortals
     *            3: if no pk on this mud, skips over all players
     *            4: pets (charmed NPCs)
     * players can only hit players in CRIMEOK rooms 4) players can only hit
     * charmed mobs in CRIMEOK rooms
     */

    if (tch == ch)
      continue;
    if (!IS_NPC(tch) && GET_LEVEL(tch) >= LVL_IMMORT)
      continue;
    if (!pk_allowed && !IS_NPC(ch) && !IS_NPC(tch))
      continue;
    if (!IS_NPC(ch) && IS_NPC(tch) && AFF_FLAGGED(tch, AFF_CHARM))
      continue;
    i = (GET_LEVEL(tch) - GET_LEVEL(ch));
    if (!IS_NPC(ch) && !IS_NPC(tch))
      if (!(ROOM_FLAGGED(ch->in_room, ROOM_ARENA) &&
ROOM_FLAGGED(tch->in_room, ROOM_ARENA)))
        if (!(i < PK_MIN_LEVEL && i > -PK_MIN_LEVEL))
        //  if (check_wanted(tch) && (!FIGHTING(tch)))
            continue;

    if(GET_RACE(tch) == RACE_ANCI_DROW || (GET_RACE(tch) == RACE_DRACONIAN && number(1,12)
== 3))
    {
     send_to_char("Spell innefective\r\n" ,ch) ;
     send_to_char("Spell innefective\r\n" ,tch) ;
     continue ;
    }

   if(num > 3) return ; // soh acerta 3 mobs no maximo.

    num++ ; // numero de mobs afetados por magia de area
    /* Doesn't matter if they die here so we don't check. -gg 6/24/98 */
    mag_damage(GET_LEVEL(ch), ch, tch, spellnum, 1);
  }
}


/*
*  Every spell which summons/gates/conjours a mob comes through here.
*
*  None of these spells are currently implemented in Circle 3.0; these
*  were taken as examples from the JediMUD code.  Summons can be used
*  for spells like clone, ariel servant, etc.
*
* 10/15/97 (gg) - Implemented Animate Dead and Clone.
*/

/*
* These use act(), don't put the \r\n.
*/
const char *mag_summon_msgs[] = {
  "\r\n",
  "$n makes a strange magical gesture; you feel a strong breeze!",
  "$n animates a corpse!",
  "$N appears from a cloud of thick blue smoke!",
  "$N appears from a cloud of thick green smoke!", 
  "$N appears from a cloud of thick red smoke!", 
  "$N disappears in a thick black cloud!", 
  "As $n makes a strange magical gesture, you feel a strong breeze.", 
  "As $n makes a strange magical gesture, you feel a searing heat.", 
  "As $n makes a strange magical gesture, you feel a sudden chill.", 
  "As $n makes a strange magical gesture, you feel the dust swirl.", 
  "$n magically divides!", 
  "$n animates a corpse!", 
  "$n magically creates a golem!", 
  "$n creates a illusion!", 
  "$n makes magical gestures and open the earth!", 
  "$n creates a powerful black bear!", 
  "&n$n says a prayer to the &yWild Gods&n, and an &YAncient Spirit&n appears!",
  "&r$n&n gestures, and an &RAberration&n stands up.",
  "&b$n raises a horde of undeads.&n"
};

/*
* Keep the \r\n because these use send_to_char.
*/
const char *mag_summon_fail_msgs[] = {
  "\r\n", 
  "There are no such creatures.\r\n", 
  "Uh oh...\r\n", 
  "Oh dear.\r\n", 
  "Oh shit!\r\n", 
  "The elements resist!\r\n", 
  "You failed.\r\n", 
  "There is no corpse!\r\n", 
  "The Wild Gods ignore your pray..\r\n",
  "You fail to raise the &RAberration&n.\r\n"
};

/* These mobiles do not exist. */
#define MOB_MONSUM_I            130
#define MOB_MONSUM_II           140
#define MOB_MONSUM_III          150
#define MOB_GATE_I              160
#define MOB_GATE_II             170
#define MOB_GATE_III            180

/* Defined mobiles. */
#define MOB_ELEMENTAL_BASE      20      /* Only one for now. */
#define MOB_CLONE               10
#define MOB_ZOMBIE              11
#define MOB_GOLEM               15
#define MOB_ILLUSION            16
#define MOB_AERIALSERVANT       19

void mag_summons(int level, struct char_data * ch, struct obj_data * obj,
                      int spellnum, int savetype)
{
  struct char_data *mob = NULL;
  struct obj_data *tobj, *next_obj;
  int   pfail = 0, msg = 0, fmsg = 0, mob_num = 0,
        num = 1, handle_corpse = FALSE, i, x;

  if (ch == NULL)
    return;

  switch (spellnum) {
  case SPELL_CLONE:
    msg = 10;
    fmsg = number(2, 6);        /* Random fail message. */
    mob_num = MOB_CLONE;
    pfail = 50; /* 50% failure, should be based on something later. */
    break;

  case SPELL_CALL_BEAR :
   msg = 15 ;
   fmsg = 3 ;
   mob_num = 43 ; // white bear, by Luigi.
   pfail = 25 ;
   break ;

  case SPELL_HELM : // religion
   msg = 15 ;
   fmsg = 3 ;
   mob_num = 444 ; // by Luigi.
   pfail = 0 ;
   break ;

  case SPELL_ANIMATE_DEAD:
    if (obj == NULL || !IS_CORPSE(obj)) {
      act(mag_summon_fail_msgs[7], FALSE, ch, 0, 0, TO_CHAR);
      return;
    }
    handle_corpse = TRUE;
    msg = 11;
    fmsg = number(2, 6);        /* Random fail message. */
    mob_num = MOB_ZOMBIE;
    pfail = 10; /* 10% failure, should vary in the future. */
    break;

  case SPELL_GOLEM:
    msg = 12;
    fmsg = number(2, 6);        /* Random fail message. */
    mob_num = MOB_GOLEM;
    pfail = 50; /* 50% failure, should be based on something later. */
    break;

  case SPELL_ILLUSION:
    msg = 13;
    fmsg = number(2, 6);        /* Random fail message. */
    mob_num = MOB_ILLUSION;
    pfail = 50; /* 50% failure, should be based on something later. */
    break;

  case SPELL_ELEMENTAL:
    msg = 14;
    fmsg = number(2, 6);        /* Random fail message. */
    mob_num = MOB_ELEMENTAL_BASE;
    pfail = 50; /* 50% failure, should be based on something later. */
    break;

  case SPELL_ANCIENT_SPIRIT:
    msg = 17;
    fmsg = 8;
    mob_num = MOB_CLONE;
    pfail = 30; // 30% failure
  break;

  case SPELL_RAISE_ABERRATION:
    msg = 18;
    fmsg = 9;
    mob_num = MOB_ZOMBIE;
    pfail = 25; // 25% failure
  break;

  case SPELL_UNDEAD_HORDE:
    msg = 19;
    fmsg = number(2,6);
    mob_num = MOB_CLONE;
    pfail = 0; // 30% failure
	num = number(8, 12);
  break;

  default:
    return;
  }

  if (AFF_FLAGGED(ch, AFF_CHARM)) {
    send_to_char("You are too giddy to have any followers!\r\n", ch);
    return;
  }
  if (number(0, 101) < pfail) {
    send_to_char(mag_summon_fail_msgs[fmsg], ch);
    return;
  }
  if (!allow_follower(ch, 5)) {
    send_to_char("You can't have any more followers.\n\r", ch);
    return;
  }

  if (spellnum == SPELL_ANCIENT_SPIRIT) {
	if (ch->followers) {
		send_to_char("&nThe &YAncient Spirit&n says, 'Kill your followers, then call me again.'\r\n", ch);
		return;
	} else {
		GET_MANA(ch) = 0;
	}
  } else if (spellnum == SPELL_RAISE_ABERRATION) {
	if (ch->followers) {
		send_to_char("&nYou can't have any other followers to summon the &RAberration&n.\r\n", ch);
		return;
	} else {
		GET_MANA(ch) = 0;
		GET_HIT(ch) = 1;
	}
  }

  for (i = 0; i < num; i++) {
    if (!(mob = read_mobile(mob_num, VIRTUAL))) {
      send_to_char("You don't quite remember how to make that creature.\r\n", ch);
      return;
    }

    char_to_room(mob, ch->in_room);
    IS_CARRYING_W(mob) = 0;
    IS_CARRYING_N(mob) = 0;
    SET_BIT(AFF_FLAGS(mob), AFF_CHARM);
    if (spellnum == SPELL_CLONE) {      /* Don't mess up the proto with strcpy. */
      mob->player.name = str_dup(GET_NAME(ch));
      mob->player.short_descr = str_dup(GET_NAME(ch));
      GET_MAX_HIT(mob) = GET_MAX_HIT(ch);
      GET_HIT(mob) = GET_MAX_HIT(ch);
      GET_DAMROLL(mob) = GET_DAMROLL(ch);
      GET_HITROLL(mob) = GET_HITROLL(ch);
      GET_LEVEL(mob) = GET_LEVEL(ch);
	  GET_REMORT(mob) = GET_REMORT(ch);
      GET_EXP(mob) = 0;
      GET_MANA(ch) = 0;
      GET_GOLD(mob) = 0;
      GET_CLASS(mob) = -1;
    } else if (spellnum == SPELL_ANCIENT_SPIRIT) { 
      mob->player.name = str_dup("ancient spirit shaman");
      mob->player.short_descr = str_dup("the Ancient Spirit");
      mob->player.long_descr = str_dup("&yThe Ancient Spirit of the wild is here, floating.\r\n");
      mob->player.description = str_dup("&nThis creature represents the power of the chaotic, the wild.\r\n");
      GET_MAX_HIT(mob) = (GET_LEVEL(ch)* 25) + (GET_REMORT(ch) * 180);
      GET_HIT(mob) = GET_MAX_HIT(mob);
      GET_DAMROLL(mob) = (GET_LEVEL(ch)/5) + (GET_REMORT(ch)*2);
      GET_HITROLL(mob) = (GET_LEVEL(ch)/5) + (GET_REMORT(ch)*2);
      GET_AC(mob) = GET_AC(ch);
      GET_LEVEL(mob) = GET_LEVEL(ch);
          GET_REMORT(mob) = GET_REMORT(ch);
      mob->mob_specials.damnodice = 10;
      mob->mob_specials.damsizedice = number(1, 3) + (GET_LEVEL(ch)/10) + GET_REMORT(ch);
      SET_BIT(MOB_FLAGS(mob), MOB_AWARE);
      SET_BIT(MOB_FLAGS(mob), MOB_NOBLIND);
      SET_BIT(MOB_FLAGS(mob), MOB_NOSUMMON);
      SET_BIT(MOB_FLAGS(mob), MOB_NOSLEEP);
      SET_BIT(AFF_FLAGS(mob), AFF_FLY);
      SET_BIT(AFF_FLAGS(mob), AFF_DETECT_INVIS);
      GET_EXP(mob) = 0;
      GET_MANA(ch) = 0;
      GET_GOLD(mob) = 0;
      GET_CLASS(mob) = -1;


	} else if (spellnum == SPELL_UNDEAD_HORDE) { 
      mob->player.name = str_dup("zombie undead");
      mob->player.short_descr = str_dup("the night zombie");
      mob->player.long_descr = str_dup("&yA night zombie is here.&n\r\n");
      mob->player.description = str_dup("&nA zombie&n.\r\n");
      GET_MAX_HIT(mob) = 10;
      GET_HIT(mob) = GET_MAX_HIT(mob);
      GET_DAMROLL(mob) = 1;
      GET_HITROLL(mob) = 1;
      GET_AC(mob) = 100;
      GET_LEVEL(mob) = 1;
	  GET_REMORT(mob) = 0;
      mob->mob_specials.damnodice = 1;
      mob->mob_specials.damsizedice = 1;
      GET_EXP(mob) = 0;
      GET_MANA(ch) = 0;
      GET_GOLD(mob) = 0;
      GET_CLASS(mob) = -1;
	  SET_BIT(MOB_FLAGS(mob), MOB_AGGRESSIVE);

    } else if (spellnum == SPELL_RAISE_ABERRATION) {

	x = 0;
	
	for (tobj = world[ch->in_room].contents; tobj; tobj = next_obj) {
		next_obj = tobj->next_content;
		if (IS_CORPSE(tobj)) {
		    x++;
		    extract_obj(tobj);
		}
	}
	
	if (x == 0) {
		send_to_char("You could't create an &RAberration&n without corpses.\r\n", ch);
		return;
	}

	mob->player.name = str_dup("aberration undead");
      	mob->player.description = str_dup("&nIts body is made of dead bodies; Its eyes are made of dead eyes; And its hands are made from deadly hands.\r\n");
	GET_AC(mob) = 10;
      	GET_LEVEL(mob) = GET_LEVEL(ch);
	GET_REMORT(mob) = GET_REMORT(ch);

      	GET_EXP(mob) = 0;
      	GET_MANA(ch) = 0;
      	GET_GOLD(mob) = 0;
      	GET_CLASS(mob) = -1;

      	SET_BIT(MOB_FLAGS(mob), MOB_NOSUMMON);
      	SET_BIT(MOB_FLAGS(mob), MOB_NOSLEEP);
      	SET_BIT(MOB_FLAGS(mob), MOB_NOCHARM);

	if (x <= 5) {		// small

      	mob->player.short_descr = str_dup("the small Aberration");
      	mob->player.long_descr = str_dup("&yA small Undead made of corpses is here.\r\n");
	GET_MAX_HIT(mob) = (GET_LEVEL(ch)*10) + (x*10) + (GET_REMORT(ch)*15);
      	GET_DAMROLL(mob) = x;
      	GET_HITROLL(mob) = x;
      	mob->mob_specials.damnodice = 5;
     	mob->mob_specials.damsizedice = 4 + (GET_REMORT(ch)/2);

	} else if (x <= 10) {		// medium

      	mob->player.short_descr = str_dup("the medium Aberration");
      	mob->player.long_descr = str_dup("&yA medium Undead made of corpses is here.\r\n");
	GET_MAX_HIT(mob) = (GET_LEVEL(ch)*12) + (x*30) + (GET_REMORT(ch)*20);
      	GET_DAMROLL(mob) = (x/2) + (GET_LEVEL(ch)/6) + GET_REMORT(ch);
      	GET_HITROLL(mob) = (x/2) + (GET_LEVEL(ch)/6) + GET_REMORT(ch);
      	mob->mob_specials.damnodice = 8;
     	mob->mob_specials.damsizedice = 6 + (GET_REMORT(ch)/2);

	} else if (x <= 20) {		// large

      	mob->player.short_descr = str_dup("the large Aberration");
      	mob->player.long_descr = str_dup("&yA large Undead made of corpses is here.\r\n");
	GET_MAX_HIT(mob) = (GET_LEVEL(ch)*15) + (x*50) + (GET_REMORT(ch)*25);
      	GET_DAMROLL(mob) = (x/2) + (GET_LEVEL(ch)/5) + GET_REMORT(ch);
      	GET_HITROLL(mob) = (x/2) + (GET_LEVEL(ch)/5) + GET_REMORT(ch);
      	mob->mob_specials.damnodice = 12;
     	mob->mob_specials.damsizedice = 8 + (GET_REMORT(ch)/2);

	} else if (x <= 30) {		// spectral

      	mob->player.short_descr = str_dup("the spectral Aberration");
      	mob->player.long_descr = str_dup("&yA spectral Undead made of corpses is here.\r\n");
	GET_MAX_HIT(mob) = (GET_LEVEL(ch)*14) + (x*40) + (GET_REMORT(ch)*22);
      	GET_DAMROLL(mob) = (x/2) + (GET_LEVEL(ch)/4) + GET_REMORT(ch);
      	GET_HITROLL(mob) = (x/2) + (GET_LEVEL(ch)/4) + GET_REMORT(ch);
      	mob->mob_specials.damnodice = 10;
     	mob->mob_specials.damsizedice = 9 + (GET_REMORT(ch)/2);
      	SET_BIT(AFF_FLAGS(mob), AFF_FLY);
      	SET_BIT(AFF_FLAGS(mob), AFF_DETECT_INVIS);
      	SET_BIT(AFF_FLAGS(mob), AFF_INVISIBLE);

	} else if (x <= 35) {		// magical

      	mob->player.short_descr = str_dup("the magical Aberration");
      	mob->player.long_descr = str_dup("&yA magical Undead made of corpses is here.\r\n");
	GET_MAX_HIT(mob) = (GET_LEVEL(ch)*12) + (x*35) + (GET_REMORT(ch)*20);
      	GET_DAMROLL(mob) = (x/2) + (GET_LEVEL(ch)/4) + GET_REMORT(ch);
      	GET_HITROLL(mob) = (x/2) + (GET_LEVEL(ch)/4) + GET_REMORT(ch);
      	mob->mob_specials.damnodice = 9;
     	mob->mob_specials.damsizedice = 9 + (GET_REMORT(ch)/2);
      	SET_BIT(AFF_FLAGS(mob), AFF_FLY);
      	SET_BIT(AFF_FLAGS(mob), AFF_DETECT_INVIS);
      	SET_BIT(AFF_FLAGS(mob), AFF_SANCTUARY);
      	SET_BIT(AFF_FLAGS(mob), AFF_FIRESHIELD);
      	SET_BIT(AFF_FLAGS(mob), AFF_SATAN);
      	SET_BIT(AFF_FLAGS(mob), AFF_REGEN);
      	SET_BIT(AFF_FLAGS(mob), AFF_HASTE);

	} else if (x <= 40) {		// dark

      	mob->player.short_descr = str_dup("the dark Aberration");
      	mob->player.long_descr = str_dup("&yA dark Undead made of corpses is here.\r\n");
	GET_MAX_HIT(mob) = (GET_LEVEL(ch)*13) + (x*40) + (GET_REMORT(ch)*22);
      	GET_DAMROLL(mob) = (x/2) + (GET_LEVEL(ch)/3) + GET_REMORT(ch);
      	GET_HITROLL(mob) = (x/2) + (GET_LEVEL(ch)/3) + GET_REMORT(ch);
      	mob->mob_specials.damnodice = 15;
     	mob->mob_specials.damsizedice = 12 + (GET_REMORT(ch)/2);

      	SET_BIT(AFF_FLAGS(mob), AFF_FLY);
      	SET_BIT(AFF_FLAGS(mob), AFF_DETECT_INVIS);
	SET_BIT(AFF2_FLAGS(mob), AFF2_DEATHDANCE);

	} else if (x <= 50) {		// colossal

      	mob->player.short_descr = str_dup("the colossal Aberration");
      	mob->player.long_descr = str_dup("&yA colossal Undead made of corpses is here.\r\n");
	GET_MAX_HIT(mob) = (GET_LEVEL(ch)*20) + (x*60) + (GET_REMORT(ch)*25);
      	GET_DAMROLL(mob) = (x/2) + (GET_LEVEL(ch)/4) + GET_REMORT(ch);
      	GET_HITROLL(mob) = (x/2) + (GET_LEVEL(ch)/5) + GET_REMORT(ch);
      	mob->mob_specials.damnodice = 14;
     	mob->mob_specials.damsizedice = 11 + (GET_REMORT(ch)/2);

      	SET_BIT(AFF_FLAGS(mob), AFF_FLY);
      	SET_BIT(AFF_FLAGS(mob), AFF_DETECT_INVIS);
	SET_BIT(AFF_FLAGS(mob), AFF_BERZERK);
	SET_BIT(AFF_FLAGS(mob), AFF_SANCTUARY);

	} else if (x <= 80) {		// infernal

      	mob->player.short_descr = str_dup("the infernal Aberration");
      	mob->player.long_descr = str_dup("&yAn infernal Undead made of corpses is here.\r\n");
	GET_MAX_HIT(mob) = (GET_LEVEL(ch)*16) + (x*50) + (GET_REMORT(ch)*22);
      	GET_DAMROLL(mob) = (x/2) + (GET_LEVEL(ch)/3) + GET_REMORT(ch);
      	GET_HITROLL(mob) = (x/2) + (GET_LEVEL(ch)/4) + GET_REMORT(ch);
      	mob->mob_specials.damnodice = 16;
     	mob->mob_specials.damsizedice = 14 + (GET_REMORT(ch)/2);

      	SET_BIT(AFF_FLAGS(mob), AFF_FLY);
      	SET_BIT(AFF_FLAGS(mob), AFF_DETECT_INVIS);
	SET_BIT(AFF_FLAGS(mob), AFF_BERZERK);
	SET_BIT(AFF_FLAGS(mob), AFF_SANCTUARY);
      	SET_BIT(AFF_FLAGS(mob), AFF_FIRESHIELD);
	SET_BIT(AFF2_FLAGS(mob), AFF2_DEATHDANCE);

	} else {		// abyssal

      	mob->player.short_descr = str_dup("the abyssal Aberration");
      	mob->player.long_descr = str_dup("&yAn abyssal Undead made of corpses is here.\r\n");
	GET_MAX_HIT(mob) = (GET_LEVEL(ch)*30) + (x*70) + (GET_REMORT(ch)*50);
      	GET_DAMROLL(mob) = (x/2) + (GET_LEVEL(ch)/7) + GET_REMORT(ch);
      	GET_HITROLL(mob) = (x/2) + (GET_LEVEL(ch)/7) + GET_REMORT(ch);
      	mob->mob_specials.damnodice = 8;
     	mob->mob_specials.damsizedice = 8 + (GET_REMORT(ch)/2);

	SET_BIT(AFF_FLAGS(mob), AFF_SANCTUARY);
	SET_BIT(AFF2_FLAGS(mob), AFF2_BANSHEE);

	}

      	GET_HIT(mob) = GET_MAX_HIT(mob);

    } else if (spellnum == SPELL_GOLEM) {      /* Don't mess up the proto with strcpy. */
      GET_MAX_HIT(mob) = GET_LEVEL(ch)*10 + (GET_REMORT(ch) * 150);
      GET_HIT(mob) = GET_MAX_HIT(mob);
      GET_AC(mob) = GET_AC(ch);
      GET_DAMROLL(mob) = (GET_LEVEL(ch)/6) + GET_REMORT(ch);
      GET_HITROLL(mob) = (GET_LEVEL(ch)/6) + GET_REMORT(ch);
      GET_LEVEL(mob) = GET_LEVEL(ch);
	  GET_REMORT(mob) = GET_REMORT(ch);
      GET_EXP(mob) = 0;
      GET_GOLD(mob) = 0;
      GET_CLASS(mob) = -1;
    } else if (spellnum == SPELL_ANIMATE_DEAD) {      /* Don't mess up the proto with strcpy. */
      GET_MAX_HIT(mob) = (GET_LEVEL(ch)*3) + (GET_REMORT(ch) * 20);
      GET_HIT(mob) = GET_MAX_HIT(mob);
      GET_AC(mob) = GET_AC(ch)/5;
      GET_DAMROLL(mob) = GET_LEVEL(ch)/19.5;
      GET_HITROLL(mob) = GET_LEVEL(ch)/19.5;
      GET_LEVEL(mob) = GET_LEVEL(ch);
	  GET_REMORT(mob) = GET_REMORT(ch);
      GET_EXP(mob) = 0;
      GET_GOLD(mob) = 0;
      GET_CLASS(mob) = -1;
    } else if (spellnum == SPELL_ILLUSION) {      /* Don't mess up the proto with strcpy. */
      mob->player.name = str_dup(GET_NAME(ch));
      mob->player.short_descr = str_dup(GET_NAME(ch));
      sprintf(buf, "&y%s&c%s &y%s&y is standing here.\r\n", (GET_PRENAME(ch)
!= NULL ? GET_PRENAME(ch) : ""), GET_NAME(ch), GET_TITLE(ch));
      mob->player.long_descr = str_dup(buf);
      GET_EXP(mob) = 0;
      GET_GOLD(mob) = 0;
      GET_CLASS(mob) = -1;
      REMOVE_BIT(AFF_FLAGS(mob), AFF_CHARM);
      send_to_char("You create an illusion of yourself.\r\n", ch);
    } else if (spellnum == SPELL_ELEMENTAL) {      /* Don't mess up the proto with strcpy. */
      GET_MAX_HIT(mob) = GET_LEVEL(ch)*7 + (GET_REMORT(ch)*50);
      GET_HIT(mob) = GET_MAX_HIT(mob);
      GET_AC(mob) = GET_AC(ch)/3;
      GET_DAMROLL(mob) = GET_LEVEL(ch)/15 + (GET_REMORT(ch)/2);
      GET_HITROLL(mob) = GET_LEVEL(ch)/15 + (GET_REMORT(ch)/2);
      GET_LEVEL(mob) = GET_LEVEL(ch);
	  GET_REMORT(mob) = GET_REMORT(ch);
      GET_EXP(mob) = 0;
      GET_GOLD(mob) = 0;
      GET_CLASS(mob) = -1;
    } else if (spellnum == SPELL_CALL_BEAR || spellnum == SPELL_HELM) {
      GET_EXP(mob) = 0;
    }

    act(mag_summon_msgs[msg], FALSE, ch, 0, mob, TO_ROOM);
    load_mtrigger(mob);
    if (spellnum != SPELL_ILLUSION && spellnum != SPELL_UNDEAD_HORDE)
      add_follower(mob, ch);
  }
  if (handle_corpse) {
    for (tobj = obj->contains; tobj; tobj = next_obj) {
      next_obj = tobj->next_content;
      obj_from_obj(tobj);
      obj_to_char(tobj, mob);
    }
    extract_obj(obj);
  }
}


void mag_points(int level, struct char_data * ch, struct char_data * victim,
                     int spellnum, int savetype)
{
  int hit = 0;
  int move = 0;
  int mana = 0;

  if (victim == NULL)
    return;

  switch (spellnum) {
  case SPELL_CURE_LIGHT:
    hit = dice(2, 10) + (GET_REMORT(victim) * 10);
    send_to_char("You feel better.\r\n", victim);
        play_sound(ch, "splcure.wav", SND_CHAR);
    break;
  case SPELL_CURE_CRITIC:
    hit = dice(7, 10) + (GET_REMORT(victim) * 10);
    send_to_char("You feel a lot better!\r\n", victim);
        play_sound(ch, "splcure.wav", SND_CHAR);
    break;
  case SPELL_CURE_SERIOUS:
    hit = dice(12, 10) + (GET_REMORT(victim) * 10);
    send_to_char("You feel much better!\r\n", victim);
    play_sound(ch, "splcure.wav", SND_CHAR);
    break;
  case SPELL_HEAL:
    hit = dice(20, 10) + (GET_REMORT(victim) * 20);
    send_to_char("A warm feeling floods your body.\r\n", victim);
    play_sound(ch, "splcure.wav", SND_CHAR);
    break;
  case SPELL_REGENERATION: //"A spell regeneration pelo que eu saiba, seta a flag regen nos players".
    hit = dice(25, 20) + (GET_REMORT(victim) * 20);
    send_to_char("You feel in your body the goddly power...\r\n", victim);
    play_sound(ch, "splcure.wav", SND_CHAR);
    break;
  case SPELL_REVITALIZE:
    hit = dice(16, 10) + (GET_REMORT(victim) * 20);
    send_to_char("You feel your health revitalizing!\r\n", victim);
        play_sound(ch, "splcure.wav", SND_CHAR);
    break;
  case SPELL_LAY_ON_HANDS:
    hit = level + (GET_REMORT(victim) * 20);
    send_to_char("You receive the godly mercy and feel better!\r\n", victim);
        play_sound(ch, "splcure.wav", SND_CHAR);
    break;

  case SPELL_GREATER_REFRESH:
    move = level;
	GET_OXI(victim) += number(5, 10);
	if (GET_OXI(victim) > GET_MAX_OXI(victim))
		GET_OXI(victim) = GET_MAX_OXI(victim);
    send_to_char("&WYou feel &YRefreshed&W!&n\r\n", victim);
  break;

  case SPELL_ENERGY_DRAIN:

	  if(!IS_NPC(ch) && !IS_NPC(victim)){
		  if(!can_pk(ch, victim)){
			  send_to_char("&WYou can not &Rkill &Wyour opponent right now.&n\r\n", ch);
			  return;
		  }
	  }

     if (!IS_NPC(victim)) {
		  if ((GET_HIT(victim) < (GET_MAX_HIT(victim)/6)) || (number(0, 6) < number(0, 8))) {
			  act("$N's will resists your drain!", FALSE, ch, 0, victim, TO_CHAR);
			  return;
		  }

     } else {

		  if ((number(4, 8) < number(5, 10)) || GET_HIT(victim) < (GET_MAX_HIT(victim)/6)) {
			  act("$N's will resists your drain!", FALSE, ch, 0, victim, TO_CHAR);
			  return;
      
		  }
     }

	 GET_HIT(ch) += (GET_HIT(victim)/6);
	 
	 if (GET_HIT(ch) > GET_MAX_HIT(ch)) {
		 GET_HIT(ch) = GET_MAX_HIT(ch);
     }
	 
	 GET_HIT(victim) -= (GET_HIT(victim)/6);

    send_to_char("You feel a less of health!\r\n", victim);
    send_to_char("You feel your health revitalized!\r\n", ch);

    break;

  case SPELL_LIFE_DRAINING:
    if (GET_HIT(victim) < 70) {
      send_to_char("The victim does not have life to drain!\r\n", ch);
      return;
    }
    if(!IS_NPC(ch) && !IS_NPC(victim)){
		if(!can_pk(ch, victim)){
			send_to_char("&WYou can not &Rkill &Wyour opponent right now.&n\r\n", ch);
			return;
      }
    }
    GET_HIT(ch) = GET_HIT(ch) + dice(20, 12);
    if (GET_HIT(ch) > GET_MAX_HIT(ch)) {
      GET_HIT(ch) = GET_MAX_HIT(ch);
    }
    GET_HIT(victim) = MAX(GET_HIT(victim) - dice(20, 12), 1);
    send_to_char("You feel someone draining your life!\r\n", victim);
    send_to_char("You drain some life!\r\n", ch);
    break;

  case SPELL_LEND_HEALTH:
    if (GET_HIT(victim) < 100) {
      send_to_char("The victim do not have health to lend!\r\n", ch);
      return;
    }
    if(!IS_NPC(ch) && !IS_NPC(victim)){
      if(!can_pk(ch, victim)){
        send_to_char("&WYou can not &Rkill &Wyour opponent right now.&n\r\n", ch);
        return;
      }
    }
    GET_HIT(ch) = GET_HIT(ch) + dice(20, 15);
    if (GET_HIT(ch) > GET_MAX_HIT(ch)) {
      GET_HIT(ch) = GET_MAX_HIT(ch);
    }
    GET_HIT(victim) = MAX(GET_HIT(victim) - dice(20, 15), 1);

    send_to_char("You feel someone lending your health!\r\n", victim);
    send_to_char("You lend the health of someone!\r\n", ch);
    break;

  case SPELL_PSYCHIC_DRAIN:
    if (GET_MANA(victim) < 70) {
      send_to_char("The mana of the victim is not sufficient to you drain!\r\n", ch);
      return;
    }
    if(!IS_NPC(ch) && !IS_NPC(victim)){
      if(!can_pk(ch, victim)){
        send_to_char("&WYou can not &Rkill &Wyour opponent right now.&n\r\n", ch);
        return;
      }
    }
    GET_MANA(ch) = GET_MANA(ch) + dice(30, 15);
    if (GET_MANA(ch) > GET_MAX_MANA(ch)) {
      GET_MANA(ch) = GET_MAX_MANA(ch);
    }
    GET_MANA(victim) = MAX(GET_MANA(victim) - dice(30, 15), 1);

    send_to_char("You feel your energy going down!\r\n", victim);
    send_to_char("You feel the power of energy!\r\n", ch);
    break;

  case SPELL_RESTORE_MANA:
    mana = GET_LEVEL(victim) + dice(8, 10) + (GET_REMORT(victim) * 20);
    send_to_char("&nYour &Mmagic power&n has been restored.\r\n", victim);
    break;
  case SPELL_REFRESH:
    move = GET_MAX_MOVE(victim) / 5;
    send_to_char("You feel less tired.\r\n", victim);
    break;
  case SPELL_SATANIC_CURE:
    hit = 175 + dice(5, 10) + (GET_REMORT(victim) * 20);
    send_to_char("You feel in your body the power of satan..\r\n", victim);
    break;
  }

  if(GET_RACE(ch) == RACE_ARCHONS || (GET_RACE(ch) == RACE_DRACONIAN &&
number(1,12) == 3))
   hit = hit * 1.5 ;

  GET_HIT(victim) = MIN(GET_MAX_HIT(victim), GET_HIT(victim) + hit);
  GET_MOVE(victim) = MIN(GET_MAX_MOVE(victim), GET_MOVE(victim) + move);
  GET_MANA(victim) = MIN(GET_MAX_MANA(victim), GET_MANA(victim) + mana);
  update_pos(victim);
}


void mag_unaffects(int level, struct char_data * ch, struct char_data *
victim,
                        int spellnum, int type)
{
  int spell = 0;
  const char *to_vict = NULL, *to_room = NULL;

  if (victim == NULL)
    return;

  switch (spellnum) {
  case SPELL_CURE_BLIND:
  case SPELL_HEAL:
    spell = SPELL_BLINDNESS;
    to_vict = "Your vision returns!";
    to_room = "There's a momentary gleam in $n's eyes.";
    break;
  case SPELL_REMOVE_POISON:
    spell = SPELL_POISON;
    to_vict = "A warm feeling runs through your body!";
    to_room = "$n looks better.";
    break;
  case SPELL_REMOVE_CURSE:
    spell = SPELL_CURSE;
    to_vict = "You don't feel so unlucky.";
    break;

  case SPELL_DISPEL_MAGIC:
	  if (!IS_NPC(victim)) {
		if ((GET_WIS(ch) + GET_SKILL(ch, SPELL_DISPEL_MAGIC)) > (number(0,95) + GET_WIS(victim))) {
			if (victim->affected) {
				while (victim->affected)
					affect_remove(victim, victim->affected);
			}
		} else {
			send_to_char("You have failed to dispel your victim's magic.\r\n", ch);
			return;
		}
	  } else {

		  if (AFF_FLAGGED(victim, AFF_INVISIBLE))
			  AFF_FLAGS(victim) -= AFF_INVISIBLE;
		  else if (AFF_FLAGGED(victim, AFF_PROTECT_EVIL))
			  AFF_FLAGS(victim) -= AFF_PROTECT_EVIL;
		  else if (AFF_FLAGGED(victim, AFF_PROTECT_GOOD))
			  AFF_FLAGS(victim) -= AFF_PROTECT_GOOD;
		  else if (AFF_FLAGGED(victim, AFF_SANCTUARY))
			  AFF_FLAGS(victim) -= AFF_SANCTUARY;
		  else if (AFF_FLAGGED(victim, AFF_BLIND))
			  AFF_FLAGS(victim) -= AFF_BLIND;
		  else if (AFF_FLAGGED(victim, AFF_MANA_SHIELD))
			  AFF_FLAGS(victim) -= AFF_MANA_SHIELD;
		  else if (AFF_FLAGGED(victim, AFF_DAMNED_CURSE))
			  AFF_FLAGS(victim) -= AFF_DAMNED_CURSE;
		  else if (AFF_FLAGGED(victim, AFF_HASTE))
			  AFF_FLAGS(victim) -= AFF_HASTE;
		  else if (AFF_FLAGGED(victim, AFF_HOLDED))
			  AFF_FLAGS(victim) -= AFF_HOLDED;
		  else if (AFF_FLAGGED(victim, AFF_SATAN))
			  AFF_FLAGS(victim) -= AFF_SATAN;
		  else if (AFF_FLAGGED(victim, AFF_GOD))
			  AFF_FLAGS(victim) -= AFF_GOD;
		  else if (AFF_FLAGGED(victim, AFF_REGEN))
			  AFF_FLAGS(victim) -= AFF_REGEN;
		  else if (AFF_FLAGGED(victim, AFF_FLY))
			  AFF_FLAGS(victim) -= AFF_FLY;
		  else if (AFF_FLAGGED(victim, AFF_FIRESHIELD))
			  AFF_FLAGS(victim) -= AFF_FIRESHIELD;
		  else if (AFF_FLAGGED(victim, AFF_TANGLED))
			  AFF_FLAGS(victim) -= AFF_TANGLED;
		  else if (AFF2_FLAGGED(victim, AFF2_TERROR))
			  AFF2_FLAGS(victim) -= AFF2_TERROR;
		  else if (AFF2_FLAGGED(victim, AFF2_BANSHEE))
			  AFF2_FLAGS(victim) -= AFF2_BANSHEE;
		  else if (AFF2_FLAGGED(victim, AFF2_BLINK))
			  AFF2_FLAGS(victim) -= AFF2_BLINK;
		  else if (AFF2_FLAGGED(victim, AFF2_DEATHDANCE))
			  AFF2_FLAGS(victim) -= AFF2_DEATHDANCE;
		  else if (AFF2_FLAGGED(victim, AFF2_TERROR))
			  AFF2_FLAGS(victim) -= AFF2_TERROR;

		  if (GET_POS(victim) != POS_FIGHTING)
			  hit(victim, ch, TYPE_UNDEFINED);
	  }

	  to_vict = "&WYour &mmagic&W vanishes&n.";
      to_room = "&c$n&W dispels the &mmagical&n effects of &c$m&n.";
	  spell = SPELL_DISPEL_MAGIC;
  break;
  default:
    log("SYSERR: unknown spellnum %d passed to mag_unaffects.", spellnum);
    return;
  }

  if (!affected_by_spell(victim, spell) && spellnum != SPELL_HEAL && spell != SPELL_DISPEL_MAGIC) {
    send_to_char(NOEFFECT, ch);
    return;
  }

  if (spell != SPELL_DISPEL_MAGIC)
  	affect_from_char(victim, spell);
  if (to_vict != NULL)
    act(to_vict, FALSE, victim, 0, ch, TO_CHAR);
  if (to_room != NULL)
    act(to_room, TRUE, victim, 0, ch, TO_ROOM);

}


void mag_alter_objs(int level, struct char_data * ch, struct obj_data * obj,
                         int spellnum, int savetype)
{
  const char *to_char = NULL, *to_room = NULL;

  if (obj == NULL)
    return;

  switch (spellnum) {
    case SPELL_BLESS:
      if (!IS_OBJ_STAT(obj, ITEM_BLESS) &&
          (GET_OBJ_WEIGHT(obj) <= 5 * GET_LEVEL(ch))) {
        SET_BIT(GET_OBJ_EXTRA(obj), ITEM_BLESS);
        to_char = "$p glows briefly.";
      }
      break;
    case SPELL_CURSE:
      if (!IS_OBJ_STAT(obj, ITEM_NODROP)) {
        SET_BIT(GET_OBJ_EXTRA(obj), ITEM_NODROP);
        if (GET_OBJ_TYPE(obj) == ITEM_WEAPON)
          GET_OBJ_VAL(obj, 2)--;
        to_char = "$p briefly glows red.";
      }
      break;
    case SPELL_INVISIBLE:
      if (!IS_OBJ_STAT(obj, ITEM_NOINVIS | ITEM_INVISIBLE)) {
        SET_BIT(obj->obj_flags.extra_flags, ITEM_INVISIBLE);
        to_char = "$p vanishes.";
      }
      break;
    case SPELL_POISON:
      if (((GET_OBJ_TYPE(obj) == ITEM_DRINKCON) || (GET_OBJ_TYPE(obj) == ITEM_FOUNTAIN) || (GET_OBJ_TYPE(obj) == ITEM_FOOD)) && !GET_OBJ_VAL(obj, 3)) {
      GET_OBJ_VAL(obj, 3) = 1;
      to_char = "$p steams briefly.";
      }
      break;
    case SPELL_REMOVE_CURSE:
      if (IS_OBJ_STAT(obj, ITEM_NODROP)) {
        REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);
        if (GET_OBJ_TYPE(obj) == ITEM_WEAPON)
          GET_OBJ_VAL(obj, 2)++;
        to_char = "$p briefly glows blue.";
      }
      break;
    case SPELL_REMOVE_POISON:
      if (((GET_OBJ_TYPE(obj) == ITEM_DRINKCON) ||
         (GET_OBJ_TYPE(obj) == ITEM_FOUNTAIN) ||
         (GET_OBJ_TYPE(obj) == ITEM_FOOD)) && GET_OBJ_VAL(obj, 3)) {
        GET_OBJ_VAL(obj, 3) = 0;
        to_char = "$p steams briefly.";
      }
      break;
  }

  if (to_char == NULL)
    send_to_char(NOEFFECT, ch);
  else
    act(to_char, TRUE, ch, obj, 0, TO_CHAR);

  if (to_room != NULL)
    act(to_room, TRUE, ch, obj, 0, TO_ROOM);
  else if (to_char != NULL)
    act(to_char, TRUE, ch, obj, 0, TO_ROOM);

}



void mag_creations(int level, struct char_data * ch, int spellnum)
{
  struct obj_data *tobj;
  int z;

  if (ch == NULL)
    return;
  level = MAX(MIN(level, LVL_IMPL), 1);

  switch (spellnum) {
  case SPELL_CREATE_FOOD:
    z = 3009;
    break;
  default:
    send_to_char("Spell unimplemented, it would seem.\r\n", ch);
    return;
  }

  if (!(tobj = read_object(z, VIRTUAL))) {
    send_to_char("I seem to have goofed.\r\n", ch);
    log("SYSERR: spell_creations, spell %d, obj %d: obj not found",
            spellnum, z);
    return;
  }

  obj_to_char(tobj, ch);
  act("$n creates $p.", FALSE, ch, tobj, 0, TO_ROOM);
  act("You create $p.", FALSE, ch, tobj, 0, TO_CHAR);
  load_otrigger(tobj);
}

void mag_room(int level, struct char_data * ch, int spellnum)
{
	long aff; /* what affection */
	int ticks; /* how many ticks this spell lasts */
	char *to_char = NULL;
	char *to_room = NULL;
	struct raff_node *raff;

	extern struct raff_node *raff_list;

	aff = ticks =0;

	if (ch == NULL)
		return;
	level = MAX(MIN(level, LVL_IMPL), 1);

	switch (spellnum) {
	case SPELL_WALL_OF_FOG:
		to_char = "You create a fog out of nowhere.";
		to_room = "$n creates a fog out of nowhere.";
		aff = RAFF_FOG;
		ticks = 1; /* this spell lasts one tick */
		break;
	case SPELL_FIREWALL:
		to_char = "You create a firewall.";
		to_room = "$n creates a firewall.";
		aff = RAFF_FIREWALL;
		ticks = 1; /* this spell lasts one tick */
		break;
	case SPELL_ICEWALL:
		to_char = "You create a icewall.";
		to_room = "$n creates a icewall.";
		aff = RAFF_ICEWALL;
		ticks = 1; /* this spell lasts one tick */
		break;
	case SPELL_LIQUID_AIR:
		if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
			send_to_char("&nYou cannot cast this spell here.\r\n", ch);
			return;
		}
		to_char = "&nYou turn the &Cair&n into &Bwater&n.";
		to_room = "&c$n&n turns the &Cair&n to &Bwater&n.";
		aff = RAFF_LIQUID_AIR;
		ticks = 3; /* this spell lasts one tick */
	break;
	case SPELL_SILENCE:
		if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
			send_to_char("&nYou cannot cast this spell here.\r\n", ch);
			return;
		}
		to_char = "&nEverything seems to be quite &Wquiet&n now.";
		to_room = "&nEverything seems to be quite &Wquiet&n now.";
		aff = RAFF_SILENCE;
		ticks = 2; /* this spell lasts one tick */
	break;
	case SPELL_FOREST_SANCTUARY:
		if (!FOREST(ch->in_room)) {
			send_to_char("You can only use this spell on forests.\r\n", ch);
			return;
		}
		to_char = "&gThe forest spirits come to protect this place.&n\r\n";
		to_room = to_char;
		aff = RAFF_FSANCTUARY;
		ticks = 4;
	break;
	case SPELL_ILLUSION_FOREST:
		if (FOREST(ch->in_room)) {
			send_to_char("You cannot use this spell on forests.\r\n", ch);
			return;
		}
		to_char = "&gSuddenly, many trees arise from the ground.&n\r\n";
		to_room = to_char;
		aff = RAFF_ILLUSIONF;
		ticks = 8;
	break;
	default:
		sprintf(buf, "SYSERR: unknown spellnum %d passed to mag_unaffects",
spellnum);
		log(buf);
		break;
	}

	/* create, initialize, and link a room-affection node */
	CREATE(raff, struct raff_node, 1);
	raff->room = ch->in_room;
	raff->timer = ticks;
	raff->affection = aff;
	raff->spell = spellnum;
	raff->next = raff_list;
	raff_list = raff;

	/* set the affection */
	SET_BIT(ROOM_AFFECTIONS(raff->room), aff);

	if (to_char == NULL)
		send_to_char(NOEFFECT, ch);
	else
		act(to_char, TRUE, ch, 0, 0, TO_CHAR);

	if (to_room != NULL)
		act(to_room, TRUE, ch, 0, 0, TO_ROOM);
	else if (to_char != NULL)
		act(to_char, TRUE, ch, 0, 0, TO_ROOM);
}


