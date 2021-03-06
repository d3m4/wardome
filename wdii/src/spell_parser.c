/* ************************************************************************
*   File: spell_parser.c                                Part of CircleMUD *
*  Usage: top-level magic routines; outside points of entry to magic sys. *
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
#include "interpreter.h"
#include "spells.h"
#include "handler.h"
#include "comm.h"
#include "db.h"

struct spell_info_type spell_info[TOP_SPELL_DEFINE + 1];

#define SINFO spell_info[spellnum]

extern struct room_data *world;
void improve_skill(struct char_data *ch, int skill);

/* local functions */
void say_spell(struct char_data * ch, int spellnum, struct char_data * tch, 
struct obj_data * tobj);
void spello(int spl, int max_mana, int min_mana, int mana_change, int 
minpos, int targets, int violent, int routines);
int mag_manacost(struct char_data * ch, int spellnum);
ACMD(do_cast);
void unused_spell(int spl);
void mag_assign_spells(void);

/*
* This arrangement is pretty stupid, but the number of skills is limited by
* the playerfile.  We can arbitrarily increase the number of skills by
* increasing the space in the playerfile. Meanwhile, this should provide
* ample slots for skills.
*/

const char *spells[] =
{
/* 000 */  "!RESERVED!",

  /* SPELLS */

/* 001 */  "armor",
/* 002 */  "teleport",
/* 003 */  "bless",
/* 004 */  "blindness",
/* 005 */  "burning hands",
/* 006 */  "call lightning",
/* 007 */  "charm person",
/* 008 */  "chill touch",
/* 009 */  "clone",
/* 010 */  "color spray",
/* 011 */  "control weather",
/* 012 */  "create food",
/* 013 */  "create water",
/* 014 */  "cure blind",
/* 015 */  "cure critic",
/* 016 */  "cure light",
/* 017 */  "curse",
/* 018 */  "detect alignment",
/* 019 */  "detect invisibility",
/* 020 */  "detect magic",
/* 021 */  "detect poison",
/* 022 */  "dispel evil",
/* 023 */  "earthquake",
/* 024 */  "enchant weapon",
/* 025 */  "energy drain",
/* 026 */  "fireball",
/* 027 */  "harm",
/* 028 */  "heal",
/* 029 */  "invisibility",
/* 030 */  "lightning bolt",
/* 031 */  "locate object",
/* 032 */  "magic missile",
/* 033 */  "poison",
/* 034 */  "protection from evil",
/* 035 */  "remove curse",
/* 036 */  "sanctuary",
/* 037 */  "shocking grasp",
/* 038 */  "sleep",
/* 039 */  "strength",
/* 040 */  "summon",
/* 041 */  "ventriloquate",
/* 042 */  "word of recall",
/* 043 */  "remove poison",
/* 044 */  "sense life",
/* 045 */  "animate dead",
/* 046 */  "dispel good",
/* 047 */  "group armor",
/* 048 */  "group heal",
/* 049 */  "group recall",
/* 050 */  "infravision",
/* 051 */  "waterwalk",
/* 052 */  "mana shield",
/* 053 */  "damned curse",
/* 054 */  "refresh",
/* 055 */  "haste",
/* 056 */  "regeneration",
/* 057 */  "petrify muscle",
/* 058 */  "god pact",
/* 059 */  "satan pact",
/* 060 */  "end of world",
/* 061 */  "mental rage", "satanic cure", "fly", "blood thirst", "entangle",
	/* 066 */  "fireshield", "fast health", "icestorm", "meteorstorm", "firestorm",
/* 071 */  "force shield", "enhanced armor", "revitalize", "golem", "illusion",
/* 076 */  "wall of fog", "fire wall", "ice wall", "mana sword", "poison weapon",
/* 081 */  "firebolt", "icebolt", "mind sight", "elemental", "grease",
/* 086 */  "cromatic orb", "flame arrow", "slow","eletricstorm","iron skin",
/* 091 */  "platinum skin", "diamond skin", "acid arrow", "minute meteor", "glacial cone",
/* 096 */  "area lightning", "fire spit", "gas spit", "frost spit", "acid spit",
/* 101 */  "lightning spit", "blade barrier", "prismatic sphere", "armageddom", "delayed fireball",
/* 106 */  "friends", "dumbness", "phantom armor", "spectral wings", "holy fury",
/* 111 */  "champion strenght", "holy mace", "death scyth", "simulacrum", "terror",
/* 116 */  "death finger", "engulfing darkness", "banshee aura", "siphon life", "blink",
/* 121 */  "meteor shower", "holy word", "holy shout", "deathdance", "death wave",
/* 126 */  "death ripple", "peace", "protection from fire", "group fly", "wraithform",
/* 131 */  "fear", "minor globe", "major globe", "phantom flame", "sacrifice",
/* 136 */  "skull trap", "ghastly touch", "vampiric touch", "pestilence", "antimagic shell",
/* 141 */  "holy bolt", "dispel neutral", "shillelagh", "magical stone", "command",
/* 146 */  "hold person", "volcano", "geyser", "group bless", "corporal trembling",
/* 151 */  "ice heart", "polteirgeist", "blizzard", "create light", "clairvoyance",
/* 156 */  "create flames", "clairaudience", "winged knife", "enhanced strength", "ego whip",
/* 161 */  "project force", "wrench", "feel light", "balistic attack", "superior invisibility",
/* 166 */  "detonate", "aversion", "psychic ray", "molecular agitation", "empathy",
/* 171 */  "thought shield", "biofeedback", "mental barrier", "inertial barrier", "cell adjustment",
/* 176 */  "concentration", "conviction", "holy shield", "lay on hands", "cure serious",
/* 181 */  "remove fear", "flesh armor", "confusion", "bravery", "rigid thinking",
/* 186 */  "paradise chrysalis", "abutilon", "aura sight", "pain", "displacement",
/* 191 */  "intelectual fortress", "life draining", "body weaponry", "body equilibrium", "adrenaline control",
/* 196 */  "lend health", "combat mind", "psionic blast", "psychic drain", "martial trance",
/* 201 */  "hipnotic suggestion", "astral projection", "ultima", "summon bahamut", "barkskin",
/* 206 */  "call bear", "muir", "ilmanateur", "selune", "helm",
/* 211 */  "talos", "Oghma", "Tempus", "Faitheyes", "grow roots",
/* 216 */  "ancient spirit", "extend spells", "mana burn", "exhaustion", "wheel of fortune",
/* 221 */  "liquid air", "asphyxiate", "iron body", "dispel magic", "silence",
/* 226 */  "greater refresh", "raise aberration", "forest sanctuary", "illusion forest", "call night",
/* 231 */  "call daylight", "undead horde", "storm sky", "!UNUSED!", "!UNUSED!",
/* 236 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 241 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 246 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 251 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 256 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 261 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 266 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 271 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 276 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 281 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 286 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 291 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 296 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",

/* SKILLS */

/* 301 */  "backstab",
/* 302 */  "bash",
/* 303 */  "hide",
/* 304 */  "kick",
/* 305 */  "pick lock",
/* 306 */  "stroke",
/* 307 */  "rescue",
/* 308 */  "sneak",
/* 309 */  "steal",
/* 310 */  "track",
/* 311 */  "combo",
/* 312 */  "shield block",
/* 313 */  "swim", "vitalize health", "vitalize mana",
/* 316 */  "spy", "critical attack",
/* 318 */  "second attack",
/* 319 */  "third attack",
/* 320 */  "fourth attack",
/* 321 */  "scan", "sabre",
/* 323 */  "hand damage", "white gas", "harakiri", "circle arround",
/* 327 */  "second stab", "parry", "dodge", "tumble", "hit",
/* 332 */  "bludgeon especialization", "slash especialization", "focus(slash)", "pierce especialization", "focus(bludgeon)",
/* 337 */  "peek", "SEM USO", "SEM USO", "SEM USO", "SEM USO",
/* 342 */  "focus(pierce)", "SEM USO", "SEM USO", "SEM USO", "disarm",
/* 347 */  "berzerk", "first aid", "headbash", "escape", "dual wield",
/* 352 */  "meditate", "levitate", "listen", "agility", "kaiser",
/* 357 */  "warshout", "knockout", "throw", "gut", "drown",
/* 362 */  "chop", "kickflip", "whirlwind", "blood ritual", "style pro",
/* 367 */  "sprite", "werewolf", "sucubus", "behemoth", "tyrael",
/* 372 */   "slayer",  "improved berzerk", "power kick", "brew",
/* 377 */  "craft", "forest walk", "forest agility", "forest wilderness", "howl",
/* 382 */  "silent spell", "snake fight", "arm traps", "shuriken", "!UNUSED!",
/* 387 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 392 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 397 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 402 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 407 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 412 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 417 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 422 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 427 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 432 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 437 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 442 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 447 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 452 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 457 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 462 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 467 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 472 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 477 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 482 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 487 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 492 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
/* 497 */  "!UNUSED!", "!UNUSED!", "!UNUSED!", "!UNUSED!",
//    CASO AGENTE UTILIZE OS 500 PRA SKILLS AINDA TEMOS MAIS 100
//   SOH NAO COLOQUEI PRA NAO DEIXA O FILE MUITO GRANDE AINDA 500
//   ACHO Q EXAGEREI :P

/**   OBJECT SPELLS AND NPC SPELLS/SKILLS */

/* 501 */  "identify",                   /* 501 */
/* 502 */  "fire breath",
/* 503 */  "gas breath",
/* 504 */  "frost breath",
/* 505 */  "acid breath",
/* 506 */  "lightning breath",
/* 507 */  "restore mana",

  "\n"                          /* the end */
};


struct syllable {
  const char *org;
  const char *news;
};


struct syllable syls[] = {
  {" ", " "},
  {"ar", "abra"},
  {"ate", "i"},
  {"cau", "kada"},
  {"blind", "nose"},
  {"bur", "mosa"},
  {"cu", "judi"},
  {"de", "oculo"},
  {"dis", "mar"},
  {"ect", "kamina"},
  {"en", "uns"},
  {"gro", "cra"},
  {"light", "dies"},
  {"lo", "hi"},
  {"magi", "kari"},
  {"mon", "bar"},
  {"mor", "zak"},
  {"move", "sido"},
  {"ness", "lacri"},
  {"ning", "illa"},
  {"per", "duda"},
  {"ra", "gru"},
  {"re", "candus"},
  {"son", "sabru"},
  {"tect", "infra"},
  {"tri", "cula"},
  {"ven", "nofo"},
  {"word of", "inset"},
  {"a", "i"}, {"b", "v"}, {"c", "q"}, {"d", "m"}, {"e", "o"}, {"f", "y"}, {"g", "t"},
  {"h", "p"}, {"i", "u"}, {"j", "y"}, {"k", "t"}, {"l", "r"}, {"m", "w"}, {"n", "b"},
  {"o", "a"}, {"p", "s"}, {"q", "d"}, {"r", "f"}, {"s", "g"}, {"t", "h"}, {"u", "e"},
  {"v", "z"}, {"w", "x"}, {"x", "n"}, {"y", "l"}, {"z", "k"}, {"", ""}
};


int mag_manacost(struct char_data * ch, int spellnum)
{
  int mana;

  mana = MAX(SINFO.mana_max - (SINFO.mana_change *
      (GET_LEVEL(ch) - (SINFO.min_level[(int) GET_CLASS(ch)]) == 0 ? GET_LEVEL(ch) : SINFO.min_level[(int) GET_CLASS(ch)])),
//                    (GET_LEVEL(ch) - SINFO.min_level[(int) GET_CLASS(ch)])),
             SINFO.mana_min);
 
  return mana;
}

 
/* say_spell erodes buf, buf1, buf2 */

void say_spell(struct char_data * ch, int spellnum, struct char_data * tch,
	            struct obj_data * tobj)
{
  char lbuf[256];

  struct char_data *i;
  int j, ofs = 0;

  *buf = '\0';
  strcpy(lbuf, spells[spellnum]);

  while (*(lbuf + ofs)) {
    for (j = 0; *(syls[j].org); j++) {
      if (!strncmp(syls[j].org, lbuf + ofs, strlen(syls[j].org))) {
	strcat(buf, syls[j].news);
	ofs += strlen(syls[j].org);
      }
    }
  }

  if (tch != NULL && tch->in_room == ch->in_room) {
    if (tch == ch)
      sprintf(lbuf, "$n closes $s eyes and utters the words, '%%s'.");
    else
      sprintf(lbuf, "$n stares at $N and utters the words, '%%s'.");
  } else if (tobj != NULL &&
	     ((tobj->in_room == ch->in_room) || (tobj->carried_by == ch)))
    sprintf(lbuf, "$n stares at $p and utters the words, '%%s'.");
  else
    sprintf(lbuf, "$n utters the words, '%%s'.");

  sprintf(buf1, lbuf, spells[spellnum]);
  sprintf(buf2, lbuf, buf);

  for (i = world[ch->in_room].people; i; i = i->next_in_room) {
    if (i == ch || i == tch || !i->desc || !AWAKE(i))
      continue;
    if (GET_CLASS(ch) == GET_CLASS(i))
      perform_act(buf1, ch, tobj, tch, i);
    else
      perform_act(buf2, ch, tobj, tch, i);
  }

  if (tch != NULL && tch != ch && tch->in_room == ch->in_room) {
/*    sprintf(buf1, "$n stares at you and utters the words, '%s'.",
	    GET_CLASS(ch) == GET_CLASS(tch) ? spells[spellnum] : buf);
    act(buf1, FALSE, ch, NULL, tch, TO_VICT);*/
    sprintf(buf1, "$n stares at you and utters the words, '%s'.",
            (!IS_NPC(tch) && (GET_SKILL(tch, spellnum) > 50)) ? 
spells[spellnum] : buf);
    act(buf1, FALSE, ch, NULL, tch, TO_VICT);
  }
}

const char *skill_name(int num)
{
  int i = 0;

  if (num <= 0) {
    if (num == -1)
      return "UNUSED";
    else
      return "UNDEFINED";
  }

  while (num && *spells[i] != '\n') {
    num--;
    i++;
  }

  if (*spells[i] != '\n')
    return spells[i];
  else
    return "UNDEFINED";
}


int find_skill_num(char *name)
{
  int index = 0, ok;
  char *temp, *temp2;
  char first[256], first2[256];

  while (*spells[++index] != '\n') {
    if (is_abbrev(name, spells[index]))
      return index;

    ok = 1;
    /* It won't be changed, but other uses of this function elsewhere may. 
*/
    temp = any_one_arg((char *)spells[index], first);
    temp2 = any_one_arg(name, first2);
    while (*first && *first2 && ok) {
      if (!is_abbrev(first2, first))
        ok = 0;
      temp = any_one_arg(temp, first);
      temp2 = any_one_arg(temp2, first2);
    }

    if (ok && !*first2)
      return index;
  }

  return -1;
}



/*
* This function is the very heart of the entire magic system.  All
* invocations of all types of magic -- objects, spoken and unspoken PC
* and NPC spells, the works -- all come through this function eventually.
* This is also the entry point for non-spoken or unrestricted spells.
* Spellnum 0 is legal but silently ignored here, to make callers simpler.
*/
int call_magic(struct char_data * caster, struct char_data * cvict,
             struct obj_data * ovict, int spellnum, int level, int casttype)
{
  int savetype;

  if (spellnum < 1 || spellnum > TOP_SPELL_DEFINE)
    return 0;

  if (ROOM_FLAGGED(caster->in_room, ROOM_NOMAGIC)) {
    send_to_char("Your magic fizzles out and dies.\r\n", caster);
    act("$n's magic fizzles out and dies.", FALSE, caster, 0, 0, TO_ROOM);
    return 0;
  }

  if ((ROOM_AFFECTED(caster->in_room, RAFF_SILENCE)) && (GET_SKILL(caster, SKILL_SILENT_SPELL) < number(1, 100))) {
	  send_to_char("&nYou try to say your spell, but fail!\r\n", caster);
	  act("&c$n&n tries to say a spell, but fails.", FALSE, caster, 0, 0, TO_ROOM);
	  return 0;
  } else {
	  improve_skill(caster, SKILL_SILENT_SPELL);
  }

  if (ROOM_FLAGGED(caster->in_room, ROOM_PEACEFUL) &&
      (SINFO.violent || IS_SET(SINFO.routines, MAG_DAMAGE))) {
    send_to_char("A flash of white light fills the room, dispelling your "
                 "violent magic!\r\n", caster);
    act("White light from no particular source suddenly fills the room, "
        "then vanishes.", FALSE, caster, 0, 0, TO_ROOM);
    return 0;
  }

  if (!IS_NPC(caster) && cvict && !IS_NPC(cvict) &&
      GET_LEVEL(caster) >= LVL_IMMORT && GET_LEVEL(cvict) < LVL_IMMORT) {
    sprintf(buf2, "(GC) %s casts spell '%s' on %s at %s&g",
	    GET_NAME(caster), spells[spellnum], GET_NAME(cvict), world[cvict->in_room].name);
    mudlog(buf2, BRF, MIN(GET_LEVEL(caster)+1, LVL_IMPL), TRUE);
  }
  /* determine the type of saving throw */
  switch (casttype) {
  case CAST_STAFF:
  case CAST_SCROLL:
  case CAST_POTION:
  case CAST_ERB:
  case CAST_WAND:
    savetype = SAVING_ROD;
    break;
  case CAST_SPELL:
    savetype = SAVING_SPELL;
    break;
  default:
    savetype = SAVING_BREATH;
    break;
  }


  if(cvict)
  {
   if(GET_RACE(cvict) == RACE_ANCI_DROW || (GET_RACE(cvict) == RACE_DRACONIAN && number(1,12) == 3))
   {
    if(SINFO.violent)
    {
     send_to_char("Spell innefective\r\n",caster) ;
     send_to_char("Spell innefective\r\n",cvict) ;
     return 1 ;
    }
   }
  }
  /*
   * Hm, target could die here.  Wonder if we should move this down lower to
   * give the other spells a chance to go off first? -gg 6/24/98
   */
  if (IS_SET(SINFO.routines, MAG_DAMAGE))
    if (mag_damage(level, caster, cvict, spellnum, savetype) == -1)
      return 1;

  if (IS_SET(SINFO.routines, MAG_AFFECTS))
   {
    mag_affects(level, caster, cvict, spellnum, savetype);
//   if(GET_RACE(caster) == RACE_HIGH_ELF || (GET_RACE(caster) == RACE_DRACONIAN &&number(1,12) == 3))
  //  mag_affects(level, caster, cvict, spellnum, savetype);
   }

  if (IS_SET(SINFO.routines, MAG_UNAFFECTS))
    mag_unaffects(level, caster, cvict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_POINTS))
    mag_points(level, caster, cvict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_ALTER_OBJS))
    mag_alter_objs(level, caster, ovict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_GROUPS))
    mag_groups(level, caster, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_MASSES))
    mag_masses(level, caster, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_AREAS))
    mag_areas(level, caster, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_SUMMONS))
    mag_summons(level, caster, ovict, spellnum, savetype);

  if (IS_SET(SINFO.routines, MAG_CREATIONS))
    mag_creations(level, caster, spellnum);

  if (IS_SET(SINFO.routines, MAG_MANUAL))
    switch (spellnum) {
    case SPELL_CHARM:           MANUAL_SPELL(spell_charm); break;
    case SPELL_RIGID_THINKING:  MANUAL_SPELL(spell_rigid_thinking); break;
    case SPELL_CREATE_WATER:    MANUAL_SPELL(spell_create_water); break;
    case SPELL_DETECT_POISON:   MANUAL_SPELL(spell_detect_poison); break;
    case SPELL_ENCHANT_WEAPON:  MANUAL_SPELL(spell_enchant_weapon); break;
    case SPELL_IDENTIFY:        MANUAL_SPELL(spell_identify); break;
    case SPELL_OGHMA :          MANUAL_SPELL(spell_oghma); break;
    case SPELL_LOCATE_OBJECT:   MANUAL_SPELL(spell_locate_object); break;
    case SPELL_SUMMON:          MANUAL_SPELL(spell_summon); break;
    case SPELL_WORD_OF_RECALL:  MANUAL_SPELL(spell_recall); break;
    case SPELL_TELEPORT:        MANUAL_SPELL(spell_teleport); break;
    case SPELL_FARSIGHT:        MANUAL_SPELL(spell_farsight); break;
    case SPELL_CONTROL_WEATHER: MANUAL_SPELL(spell_control_weather); break;
    case SPELL_DEATH_SCYTHE: 	MANUAL_SPELL(spell_death_scythe); break;
    case SPELL_SIMULACRUM:		MANUAL_SPELL(spell_simulacrum); break;
    case SPELL_PEACE:			MANUAL_SPELL(spell_peace); break;
    case SPELL_MANA_SWORD:		MANUAL_SPELL(spell_mana_sword); break;
    case SPELL_FEAR:			MANUAL_SPELL(spell_fear); break;
    case SPELL_ASTRAL_PROJECTION:	MANUAL_SPELL(spell_astral_projection); 
break;
	case SPELL_CALL_NIGHT:          MANUAL_SPELL(spell_call_night); break;
	case SPELL_CALL_DAYLIGHT:       MANUAL_SPELL(spell_call_daylight); break;
	case SPELL_STORM_SKY:           MANUAL_SPELL(spell_storm_sky); break;

    //case SPELL_FAITHEYES:	MANUAL_SPELL(spell_faitheyes); break;
    }

  if (IS_SET(SINFO.routines, MAG_ROOM))
    mag_room(level, caster, spellnum);

  return 1;
}

/*
* mag_objectmagic: This is the entry-point for all magic items.  This should
* only be called by the 'quaff', 'use', 'recite', etc. routines.
*
* For reference, object values 0-3:
* staff  - [0] level   [1] max charges [2] num charges [3] spell num
* wand   - [0] level   [1] max charges [2] num charges [3] spell num
* scroll - [0] level   [1] spell num   [2] spell num   [3] spell num
* potion - [0] level   [1] spell num   [2] spell num   [3] spell num
* spbook - [0] % learn [1] spell num   [2] spell num   [3] spell num
* erbs   - [0] level   [1] spell num   [2] spell num   [3] spell num
* Staves and wands will default to level 14 if the level is not specified;
* the DikuMUD format did not specify staff and wand levels in the world
* files (this is a CircleMUD enhancement).
*/

void mag_objectmagic(struct char_data * ch, struct obj_data * obj,
                          char *argument)
{
  int i, k;
  struct char_data *tch = NULL, *next_tch;
  struct obj_data *tobj = NULL;
  int spellnum;

  one_argument(argument, arg);

  k = generic_find(arg, FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM |
                   FIND_OBJ_EQUIP, ch, &tch, &tobj);

  switch (GET_OBJ_TYPE(obj)) {
  case ITEM_STAFF:
    act("You tap $p three times on the ground.", FALSE, ch, obj, 0, TO_CHAR);
    if (obj->action_description)
      act(obj->action_description, FALSE, ch, obj, 0, TO_ROOM);
    else
      act("$n taps $p three times on the ground.", FALSE, ch, obj, 0, TO_ROOM);

    if (GET_OBJ_VAL(obj, 2) <= 0) {
      act("It seems powerless.", FALSE, ch, obj, 0, TO_CHAR);
      act("Nothing seems to happen.", FALSE, ch, obj, 0, TO_ROOM);
    } else {
      GET_OBJ_VAL(obj, 2)--;
      WAIT_STATE(ch, PULSE_VIOLENCE);
      spellnum = GET_OBJ_VAL(obj, 3);
      if (!IS_SET(SINFO.routines, MAG_AREAS))
       for (tch = world[ch->in_room].people; tch; tch = next_tch) {
         next_tch = tch->next_in_room;
         if (ch == tch)
           continue;
         if (GET_OBJ_VAL(obj, 0))
           call_magic(ch, tch, NULL, GET_OBJ_VAL(obj, 3), GET_OBJ_VAL(obj, 0), CAST_STAFF);
         else
           call_magic(ch, tch, NULL, GET_OBJ_VAL(obj, 3), DEFAULT_STAFF_LVL, CAST_STAFF);
       }
      else
       if (GET_OBJ_VAL(obj, 0))
           call_magic(ch, tch, NULL, GET_OBJ_VAL(obj, 3), GET_OBJ_VAL(obj, 0), CAST_STAFF);
       else
           call_magic(ch, tch, NULL, GET_OBJ_VAL(obj, 3), DEFAULT_STAFF_LVL, CAST_STAFF);

    }
    break;
  case ITEM_WAND:
    if (k == FIND_CHAR_ROOM) {
      if (tch == ch) {
        act("You point $p at yourself.", FALSE, ch, obj, 0, TO_CHAR);
        act("$n points $p at $mself.", FALSE, ch, obj, 0, TO_ROOM);
      } else {
        act("You point $p at $N.", FALSE, ch, obj, tch, TO_CHAR);
        if (obj->action_description != NULL)
          act(obj->action_description, FALSE, ch, obj, tch, TO_ROOM);
        else
          act("$n points $p at $N.", TRUE, ch, obj, tch, TO_ROOM);
      }
    } else if (tobj != NULL) {
      act("You point $p at $P.", FALSE, ch, obj, tobj, TO_CHAR);
      if (obj->action_description != NULL)
        act(obj->action_description, FALSE, ch, obj, tobj, TO_ROOM);
      else
        act("$n points $p at $P.", TRUE, ch, obj, tobj, TO_ROOM);
    } else {
      act("At what should $p be pointed?", FALSE, ch, obj, NULL, TO_CHAR);
      return;
    }

    if (GET_OBJ_VAL(obj, 2) <= 0) {
      act("It seems powerless.", FALSE, ch, obj, 0, TO_CHAR);
      act("Nothing seems to happen.", FALSE, ch, obj, 0, TO_ROOM);
      return;
    }
    GET_OBJ_VAL(obj, 2)--;
    WAIT_STATE(ch, PULSE_VIOLENCE);
    if (GET_OBJ_VAL(obj, 0))
      call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, 3), GET_OBJ_VAL(obj, 0), CAST_WAND);
    else
      call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, 3), DEFAULT_WAND_LVL, CAST_WAND);
    break;
  case ITEM_SCROLL:
    if (*arg) {
      if (!k) {
        act("There is nothing to here to affect with $p.", FALSE, ch, obj, NULL, TO_CHAR);
        return;
      }
    } else
      tch = ch;

    act("You recite $p which dissolves.", TRUE, ch, obj, 0, TO_CHAR);
    if (obj->action_description)
      act(obj->action_description, FALSE, ch, obj, NULL, TO_ROOM);
    else
      act("$n recites $p.", FALSE, ch, obj, NULL, TO_ROOM);

    WAIT_STATE(ch, PULSE_VIOLENCE);
    for (i = 1; i < 4; i++)
      if (!(call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, i), GET_OBJ_VAL(obj, 0), CAST_SCROLL)))
        break;

    if (obj != NULL)
      extract_obj(obj);
    break;
  case ITEM_POTION:
    tch = ch;
    act("You quaff $p.", FALSE, ch, obj, NULL, TO_CHAR);
    if (obj->action_description)
      act(obj->action_description, FALSE, ch, obj, NULL, TO_ROOM);
    else
      act("$n quaffs $p.", TRUE, ch, obj, NULL, TO_ROOM);

    WAIT_STATE(ch, PULSE_VIOLENCE);
    for (i = 1; i < 4; i++)
      if (!(call_magic(ch, ch, NULL, GET_OBJ_VAL(obj, i), GET_OBJ_VAL(obj, 0), CAST_POTION)))
        break;

    if (obj != NULL)
      extract_obj(obj);
    break;
    case ITEM_ERB:
    tch = ch;
    act("You brew $p.", FALSE, ch, obj, NULL, TO_CHAR);
    if (obj->action_description)
      act(obj->action_description, FALSE, ch, obj, NULL, TO_ROOM);
    else
      act("$n brews $p.", TRUE, ch, obj, NULL, TO_ROOM);

    WAIT_STATE(ch, PULSE_VIOLENCE);
    for (i = 1; i < 4; i++)
      if (!(call_magic(ch, ch, NULL, GET_OBJ_VAL(obj, i), GET_OBJ_VAL(obj, 0), CAST_ERB)))
    //    improve_skill(ch, SKILL_BREW);
        break;

    if (obj != NULL)
      extract_obj(obj);
    break;
    case ITEM_SPELLBOOK:
    tch = ch;
    act("You study $p.", FALSE, ch, obj, NULL, TO_CHAR);
    if (obj->action_description)
      act(obj->action_description, FALSE, ch, obj, NULL, TO_ROOM);
    else
      act("$n studies $p.", TRUE, ch, obj, NULL, TO_ROOM);

    WAIT_STATE(ch, PULSE_VIOLENCE);
    for (i = 1; i <= 3; i++) {
      if (GET_OBJ_VAL(obj, i) < 0) break;
      if (GET_SKILL(ch, GET_OBJ_VAL(obj, i)) > 1)
        SET_SKILL(ch, GET_OBJ_VAL(obj, i), GET_OBJ_VAL(obj, 0));
    }

    if (obj != NULL)
      extract_obj(obj);
    break;
  default:
    log("SYSERR: Unknown object_type %d in mag_objectmagic.",
        GET_OBJ_TYPE(obj));
    break;
  }
}


/*
* cast_spell is used generically to cast any spoken spell, assuming we
* already have the target char/obj and spell number.  It checks all
* restrictions, etc., prints the words, etc.
*
* Entry point for NPC casts.  Recommended entry point for spells cast
* by NPCs via specprocs.
*/

int cast_spell(struct char_data * ch, struct char_data * tch,
                   struct obj_data * tobj, int spellnum)
{
  int result;
  if (spellnum < 0 || spellnum > TOP_SPELL_DEFINE) {
    log("SYSERR: cast_spell trying to call spellnum %d/%d.\n", spellnum, TOP_SPELL_DEFINE);
    return 0;
  }

  if (GET_POS(ch) < SINFO.min_position) {
    switch (GET_POS(ch)) {
      case POS_SLEEPING:
      send_to_char("You dream about great magical powers.\r\n", ch);
      break;
    case POS_RESTING:
      send_to_char("You cannot concentrate while resting.\r\n", ch);
      break;
    case POS_SITTING:
      send_to_char("You can't do this sitting!\r\n", ch);
      break;
    case POS_FIGHTING:
      send_to_char("Impossible!  You can't concentrate enough!\r\n", ch);
      break;
    default:
      send_to_char("You can't do much of anything like this!\r\n", ch);
      break;
    }
    return 0;
  }
  if (AFF_FLAGGED(ch, AFF_CHARM) && (ch->master == tch)) {
    send_to_char("&rYou are afraid you might hurt your master!&n\r\n", ch);
    return 0;
  }
  if ((tch != ch) && IS_SET(SINFO.targets, TAR_SELF_ONLY)) {
    send_to_char("&WYou can only cast this spell upon yourself!&n\r\n", ch);
    return 0;
  }
  if ((tch == ch) && IS_SET(SINFO.targets, TAR_NOT_SELF)) {
    send_to_char("&RYou cannot cast this spell upon yourself!&n\r\n", ch);
    return 0;
  }
  if (IS_SET(SINFO.routines, MAG_GROUPS) && !AFF_FLAGGED(ch, AFF_GROUP)) {
    send_to_char("&WYou can't cast this spell if you're not in a group!&n\r\n",ch);
    return 0;
  }
  if (tch && !(SINFO.violent || IS_SET(SINFO.routines, MAG_DAMAGE)) && !IS_SET(SINFO.routines, MAG_GROUPS) && AFF2_FLAGGED(tch, AFF2_TRANSFORM)) {
    send_to_char("&WNo one can be affected by mortal spells while &ctransformed&W!&n\r\n", ch);
    return 0;
  }

  send_to_char(OK, ch);
  say_spell(ch, spellnum, tch, tobj);

  result = call_magic(ch, tch, tobj, spellnum, GET_LEVEL(ch), CAST_SPELL);
  if (result != 0)
    improve_skill(ch, spellnum);
  return (result);
//  return (call_magic(ch, tch, tobj, spellnum, GET_LEVEL(ch), CAST_SPELL));
}


/*
* do_cast is the entry point for PC-casted spells.  It parses the arguments,
* determines the spell number and finds a target, throws the die to see if
* the spell can be cast, checks for sufficient mana and subtracts it, and
* passes control to cast_spell().
*/

ACMD(do_cast)
{
  struct char_data *tch = NULL;
  struct obj_data *tobj = NULL;
  char *s, *t;
  int mana, spellnum, i, target = 0;

  if (IS_NPC(ch))
    return;

  /* get: blank, spell name, target name */
  s = strtok(argument, "'");

  if (s == NULL) {
    send_to_char("Cast what where?\r\n", ch);
    return;
  }
  s = strtok(NULL, "'");
  if (s == NULL) {
    send_to_char("Spell names must be enclosed in the Holy Magic Symbols: '\r\n", ch);
    return;
  }
  t = strtok(NULL, "\0");

  /* spellnum = search_block(s, spells, 0); */
  spellnum = find_skill_num(s);

  if ((spellnum < 1) || (spellnum > MAX_SPELLS)) {
    send_to_char("Cast what?!?\r\n", ch);
    return;
  }


  if((spellnum > 206) && (spellnum < 214)) // religiao
   return ;

/*if (GET_LEVEL(ch) < SINFO.min_level[(int) GET_CLASS(ch)]) {
    send_to_char("You do not know that spell!\r\n", ch);
    return;
  }*/
  if (GET_SKILL(ch, spellnum) == 0) {
    send_to_char("You are unfamiliar with that spell.\r\n", ch);
    return;
  }
  /* Find the target */
  if (t != NULL) {
    one_argument(strcpy(arg, t), t);
    skip_spaces(&t);
  }
  if (IS_SET(SINFO.targets, TAR_IGNORE)) {
    target = TRUE;
  } else if (t != NULL && *t) {
    if (!target && (IS_SET(SINFO.targets, TAR_CHAR_ROOM))) {
      if ((tch = get_char_room_vis(ch, t)) != NULL)
        target = TRUE;
    }
    if (!target && IS_SET(SINFO.targets, TAR_CHAR_WORLD))
      if ((tch = get_char_vis(ch, t)))
        target = TRUE;

    if (!target && IS_SET(SINFO.targets, TAR_OBJ_INV))
      if ((tobj = get_obj_in_list_vis(ch, t, ch->carrying)))
        target = TRUE;

    if (!target && IS_SET(SINFO.targets, TAR_OBJ_EQUIP)) {
      for (i = 0; !target && i < NUM_WEARS; i++)
        if (GET_EQ(ch, i) && isname(t, GET_EQ(ch, i)->name)) {
          tobj = GET_EQ(ch, i);
          target = TRUE;
        }
    }
    if (!target && IS_SET(SINFO.targets, TAR_OBJ_ROOM))
      if ((tobj = get_obj_in_list_vis(ch, t, world[ch->in_room].contents)))
        target = TRUE;

    if (!target && IS_SET(SINFO.targets, TAR_OBJ_WORLD))
      if ((tobj = get_obj_vis(ch, t)))
        target = TRUE;

  } else {                      /* if target string is empty */
    if (!target && IS_SET(SINFO.targets, TAR_FIGHT_SELF))
      if (FIGHTING(ch) != NULL) {
        tch = ch;
        target = TRUE;
      }
    if (!target && IS_SET(SINFO.targets, TAR_FIGHT_VICT))
      if (FIGHTING(ch) != NULL) {
        tch = FIGHTING(ch);
        target = TRUE;
      }
    /* if no target specified, and the spell isn't violent, default to self 
*/
    if (!target && IS_SET(SINFO.targets, TAR_CHAR_ROOM) &&
        !SINFO.violent) {
      tch = ch;
      target = TRUE;
    }
    if (!target) {
      sprintf(buf, "Upon %s should the spell be cast?\r\n",
         IS_SET(SINFO.targets, TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_WORLD) ? "what" : "who");
      send_to_char(buf, ch);
      return;
    }
  }

  if (target && (tch == ch) && SINFO.violent) {
    send_to_char("You shouldn't cast that on yourself -- could be bad for your health!\r\n", ch);
    return;
  }

  if (!target) {
    send_to_char("Cannot find the target of your spell!\r\n", ch);
    return;
  }
  mana = mag_manacost(ch, spellnum);
  if ((mana > 0) && (GET_MANA(ch) < mana) && (GET_LEVEL(ch) < LVL_IMMORT)) {
    send_to_char("You haven't the energy to cast that spell!\r\n", ch);
    return;
  }

  /* You throws the dice and you takes your chances.. 101% is total failure 
*/
  if (number(0, 101) > GET_SKILL(ch, spellnum)) {
    WAIT_STATE(ch, PULSE_VIOLENCE);
    if (!tch || !skill_message(0, ch, tch, spellnum))
      send_to_char("You lost your concentration!\r\n", ch);
    if (mana > 0)
      GET_MANA(ch) = MAX(0, MIN(GET_MAX_MANA(ch), GET_MANA(ch) - (mana / 2)));
    if (SINFO.violent && tch && IS_NPC(tch))
      hit(tch, ch, TYPE_UNDEFINED);
  } else { /* cast spell returns 1 on success; subtract mana & set waitstate 
*/
    if (cast_spell(ch, tch, tobj, spellnum)) {
      WAIT_STATE(ch, PULSE_VIOLENCE);
      if (mana > 0)
        GET_MANA(ch) = MAX(0, MIN(GET_MAX_MANA(ch), GET_MANA(ch) - mana));


    }
  }
}



void spell_level(int spell, int chclass, int level)
{
  int bad = 0;

  if (spell < 0 || spell > TOP_SPELL_DEFINE) {
    log("SYSERR: attempting assign to illegal spellnum %d/%d", spell, TOP_SPELL_DEFINE);
    return;
  }

  if (chclass < 0 || chclass >= NUM_CLASSES) {
    log("SYSERR: assigning '%s' to illegal class %d/%d.", skill_name(spell), chclass, NUM_CLASSES - 1);
    bad = 1;
  }

  if (level < 1 || level > LVL_IMPL) {
    log("SYSERR: assigning '%s' to illegal level %d/%d.", skill_name(spell),level, LVL_IMPL);
    bad = 1;
  }

  if (!bad)
    spell_info[spell].min_level[chclass] = level;
}


/* Assign the spells on boot up */
void spello(int spl, int max_mana, int min_mana, int mana_change, int 
minpos,
                 int targets, int violent, int routines)
{
  int i;

  for (i = 0; i < NUM_CLASSES; i++)
    spell_info[spl].min_level[i] = LVL_ELDER;
  spell_info[spl].mana_max = max_mana;
  spell_info[spl].mana_min = min_mana;
  spell_info[spl].mana_change = mana_change;
  spell_info[spl].min_position = minpos;
  spell_info[spl].targets = targets;
  spell_info[spl].violent = violent;
  spell_info[spl].routines = routines;
}


void unused_spell(int spl)
{
  int i;

  for (i = 0; i < NUM_CLASSES; i++)
    spell_info[spl].min_level[i] = LVL_IMPL + 1;
  spell_info[spl].mana_max = 0;
  spell_info[spl].mana_min = 0;
  spell_info[spl].mana_change = 0;
  spell_info[spl].min_position = 0;
  spell_info[spl].targets = 0;
  spell_info[spl].violent = 0;
  spell_info[spl].routines = 0;
}

#define skillo(skill) spello(skill, 0, 0, 0, 0, 0, 0, 0);


/*
* Arguments for spello calls:
*
* spellnum, maxmana, minmana, manachng, minpos, targets, violent?, routines.
*
* spellnum:  Number of the spell.  Usually the symbolic name as defined in
* spells.h (such as SPELL_HEAL).
*
* maxmana :  The maximum mana this spell will take (i.e., the mana it
* will take when the player first gets the spell).
*
* minmana :  The minimum mana this spell will take, no matter how high
* level the caster is.
*
* manachng:  The change in mana for the spell from level to level.  This
* number should be positive, but represents the reduction in mana cost as
* the caster's level increases.
*
* minpos  :  Minimum position the caster must be in for the spell to work
* (usually fighting or standing). targets :  A "list" of the valid targets
* for the spell, joined with bitwise OR ('|').
*
* violent :  TRUE or FALSE, depending on if this is considered a violent
* spell and should not be cast in PEACEFUL rooms or on yourself.  Should be
* set on any spell that inflicts damage, is considered aggressive (i.e.
* charm, curse), or is otherwise nasty.
*
* routines:  A list of magic routines which are associated with this spell
* if the spell uses spell templates.  Also joined with bitwise OR ('|').
*
* See the CircleMUD documentation for a more detailed description of these
* fields.
*/

/*
* NOTE: SPELL LEVELS ARE NO LONGER ASSIGNED HERE AS OF Circle 3.0 bpl9.
* In order to make this cleaner, as well as to make adding new classes
* much easier, spell levels are now assigned in class.c.  You only need
* a spello() call to define a new spell; to decide who gets to use a spell
* or skill, look in class.c.  -JE 5 Feb 1996
*/

void mag_assign_spells(void)
{
  int i;

  /* Do not change the loop below */
  for (i = 1; i <= TOP_SPELL_DEFINE; i++)
    unused_spell(i);
  /* Do not change the loop above */

  /* Magias de Ataque */

  spello(SPELL_CREATE_LIGHT, 5, 5, 0, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_LIGHTNING_SPIT, 15, 7, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_METEOR_SHOWER, 5, 5, 0, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_FROST_SPIT, 25, 14, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_ACIDARROW, 50, 40, 1, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_FIRE_SPIT, 42, 21, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_PHANTOM_FLAME, 12, 6, 1, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_MINUTE_METEOR, 20, 10, 1, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_CREATE_FLAMES, 20, 10, 1, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_MAGIC_MISSILE, 20, 30, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_WINGED_KNIFE, 50, 40, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_GAS_SPIT, 60, 40, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_GREASE, 15, 15, 0, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_BLADEBARRIER, 50, 15, 2, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_FLAMEARROW, 50, 30, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_ACID_SPIT, 56, 35, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_CORPOR_TREMBLING, 45, 30, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_DEATH_RIPPLE, 50, 25, 2, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_CHILL_TOUCH, 70, 55, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_AFFECTS);

  spello(SPELL_CROMATIC_ORB, 40, 30, 1, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_EGO_WHIP, 50, 40, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_BURNING_HANDS, 100, 60, 1, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_GHASTLY_TOUCH, 112, 100, 1, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_AFFECTS);

  spello(SPELL_PROJECT_FORCE, 160, 140, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_CALL_LIGHTNING, 160, 80, 2, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_VOLCANO, 170, 150, 1, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_SHOCKING_GRASP, 200, 190, 1, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_WRENCH, 200, 190, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_COLOR_SPRAY, 240, 200, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_AREA_LIGHTNING, 50, 50, 0, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_DEATH_WAVE, 200, 190, 1, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_BALISTIC_ATTACK, 260, 240, 1, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_SHILLELAGH, 260, 240, 1, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_HOLY_MACE, 80, 60, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_PSIONIC_BLAST, 50, 40, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_SKULL_TRAP, 60, 60, 0, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_DISPEL_EVIL, 200, 120, 2, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_DISPEL_GOOD, 200, 120, 2, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_DISPEL_NEUTRAL, 200, 120, 2, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_LIGHTNING_BOLT, 300, 280, 0, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_VAMPIRIC_TOUCH, 320, 280, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_AFFECTS);

  spello(SPELL_ICE_HEART, 320, 280, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_ICEBOLT, 320, 300, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_EARTHQUAKE, 100, 60, 1, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_MAGICAL_STONE, 340, 300, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_DETONATE, 360, 335, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_GLACIAL_CONE, 380, 300, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_HOLY_SHOUT, 140, 80, 2, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_ELETRICSTORM, 180, 120, 2, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_FIREBOLT, 380, 300, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_PSYCHIC_RAY, 400, 380, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_DEATH_FINGER, 400, 240, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_ICESTORM, 220, 210, 1, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_POLTEIRGEIST, 400, 360, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_HARM, 450, 360, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_GEYSER, 240, 200, 2, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_PRISMATIC_SPHERE, 500, 400, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_HOLY_WORD, 175, 135, 2, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_FIRESTORM, 300, 280, 2, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_MOLEC_AGITATION, 480, 440, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_METEORSTORM, 340, 320 , 2, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_COMMAND, 530, 400, 2, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_BLIZZARD, 360, 340, 2, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_DELAYED_FIREBALL, 520, 500, 1, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_SIPHON_LIFE, 600, 340, 2, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_ARMAGEDDOM, 360, 330, 2, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_END_WORLD, 500, 450, 1, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_FIREBALL, 450, 440, 1, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_HOLY_BOLT, 255, 200, 2, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_ULTIMA, 400, 300, 1, POS_FIGHTING,
         TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_SUMMON_BAHAMUT, 400, 300, 1, POS_FIGHTING,
	       TAR_IGNORE, TRUE, MAG_AREAS);


  spello(SPELL_ENERGY_DRAIN, 300, 200, 1, POS_STANDING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_POINTS);

  spello(SPELL_LIFE_DRAINING, 300, 180, 1, POS_STANDING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_POINTS);

  spello(SPELL_LEND_HEALTH, 240, 180, 1, POS_STANDING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_POINTS);

  spello(SPELL_PSYCHIC_DRAIN, 500, 420, 1, POS_STANDING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_POINTS);

  spello(SPELL_ARMOR, 18, 18, 0, POS_FIGHTING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_MUIR, 0, 0, 0, POS_FIGHTING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

spello(SPELL_TEMPUS, 0, 0, 0, POS_FIGHTING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_SELUNE, 0, 0, 0, POS_FIGHTING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_ILMANATEUR, 0, 0, 0, POS_FIGHTING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_BLESS, 60, 30, 2, POS_STANDING,
        TAR_CHAR_ROOM | TAR_OBJ_INV, FALSE, MAG_AFFECTS | MAG_ALTER_OBJS);

  spello(SPELL_AURA_SIGHT, 80, 25, 1, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_CONVICTION, 70, 50, 3, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_CONCENTRATION, 70, 40, 3, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_BODY_WEAPONRY, 180, 100, 3, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_BODY_EQUILIBRIUM, 200, 120, 3, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_BRAVERY, 350, 200, 5, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_COMBAT_MIND, 250, 150, 2, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_ANTIMAGIC_SHELL, 140, 70, 2, POS_FIGHTING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_MINOR_GLOBE, 100, 50, 2, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_MAJOR_GLOBE, 400, 200, 2, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_BLINDNESS, 60, 20, 1, POS_STANDING,
        TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_AFFECTS);

  spello(SPELL_FEEL_LIGHT, 80, 40, 1, POS_STANDING,
        TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_AFFECTS);

  spello(SPELL_CURSE, 80, 50, 2, POS_STANDING,
        TAR_CHAR_ROOM | TAR_OBJ_INV, TRUE, MAG_AFFECTS | MAG_ALTER_OBJS);

  spello(SPELL_DETECT_ALIGN, 25, 15, 2, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_DETECT_INVIS, 30, 10, 2, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_DETECT_MAGIC, 35, 5, 2, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_DETECT_POISON, 15, 5, 2, POS_STANDING,
        TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_INFRAVISION, 30, 10, 1, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_INVISIBLE, 60, 30, 1, POS_STANDING,
        TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_AFFECTS | 
MAG_ALTER_OBJS);

  spello(SPELL_SUP_INVISIBLE, 120, 60, 1, POS_STANDING,
        TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_AFFECTS | 
MAG_ALTER_OBJS);

  spello(SPELL_POISON, 70, 10, 3, POS_STANDING,
        TAR_CHAR_ROOM | TAR_NOT_SELF | TAR_OBJ_INV, TRUE, MAG_AFFECTS | 
MAG_ALTER_OBJS);

  spello(SPELL_PROT_FROM_EVIL, 130, 90, 3, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_SANCTUARY, 210, 130, 5, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_SLEEP, 600, 500, 3, POS_STANDING,
        TAR_CHAR_ROOM, TRUE, MAG_AFFECTS);

  spello(SPELL_STRENGTH, 40, 30, 1, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_ENHANCED_STRENGTH, 180, 140, 1, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_SENSE_LIFE, 70, 50, 2, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_CLAIRAUDIENCE, 100, 80, 2, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_CLAIRVOYANCE, 70, 40, 2, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_WATERWALK, 110, 70, 1, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_HASTE, 220, 160, 3, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_MANA_SHIELD, 150, 100, 2, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_DAMNED_CURSE, 130, 105, 2, POS_STANDING,
        TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_AFFECTS);

  spello(SPELL_PESTILENCE, 260, 210, 2, POS_STANDING,
        TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_AFFECTS);

  spello(SPELL_GOD_PACT, 250, 100, 5, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_SATAN_PACT, 250, 100, 5, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_MENTAL_RAGE, 600, 500, 10, POS_STANDING,
        TAR_CHAR_ROOM, TRUE, MAG_AFFECTS);

  spello(SPELL_HOLD_PERSON, 600, 500, 10, POS_STANDING,
        TAR_CHAR_ROOM, TRUE, MAG_AFFECTS);

  spello(SPELL_FLY, 220, 110, 2, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_VULCAN, 300, 200, 2, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_ENTANGLE, 350, 250, 1, POS_STANDING,
        TAR_CHAR_ROOM, TRUE, MAG_AFFECTS);

  spello(SPELL_FIRESHIELD, 360, 260, 2, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_ADRENALINE, 420, 280, 2, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_FORCE_SHIELD, 120, 75, 3, POS_FIGHTING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_ENHANCED_ARMOR, 110, 60, 3, POS_FIGHTING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_SLOW, 120, 80, 5, POS_STANDING,
        TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_AFFECTS);

  spello(SPELL_THOUGHT_SHIELD, 65, 20, 2, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_BIOFEEDBACK, 85, 30, 2, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_MENTAL_BARRIER, 95, 40, 2, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_INERTIAL_BARRIER, 110, 50, 2, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_CELL_ADJUSTMENT, 200, 100, 2, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_IRON_SKIN, 150, 80, 3, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_PLATINUM_SKIN, 220, 150, 3, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_DIAMOND_SKIN, 290, 220, 3, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_HOLY_SHIELD, 200, 115, 3, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_PARADI_CHRYSALIS, 500, 200, 7, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_ABUTILON, 450, 200, 7, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_FRIENDS, 70, 85, 3, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_EMPATHY, 75, 50, 3, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_INTEL_FORTRESS, 185, 145, 3, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_ADREN_CONTROL, 215, 165, 3, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_DUMBNESS, 50, 15, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_AFFECTS);

  spello(SPELL_PAIN, 85, 40, 2, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_AFFECTS);

  spello(SPELL_DISPLACEMENT, 95, 50, 2, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_AFFECTS);

  spello(SPELL_PHANTOM_ARMOR, 90, 45, 2, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_FLESH_ARMOR, 110, 55, 2, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_SACRIFICE, 50, 25, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_SPECTRAL_WINGS, 150, 100, 2, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_HOLY_FURY, 150, 120, 3, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_CHAMPION_STRENGTH, 200, 130, 4, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_TERROR,  600, 500, 10, POS_FIGHTING,
	TAR_CHAR_ROOM, TRUE, MAG_AFFECTS);

  spello(SPELL_AVERSION, 600, 500, 10, POS_FIGHTING,
	TAR_CHAR_ROOM, TRUE, MAG_AFFECTS);

  spello(SPELL_CONFUSION,  600,500, 10, POS_FIGHTING,
	TAR_CHAR_ROOM, TRUE, MAG_AFFECTS);

  spello(SPELL_ENGULFING_DARKNESS,  40, 15, 1, POS_FIGHTING,
    TAR_CHAR_ROOM, TRUE, MAG_AFFECTS);

  spello(SPELL_BANSHEE_AURA, 80, 50, 1, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_BLINK, 50, 35, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_DEATHDANCE, 160, 120, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS | MAG_POINTS);

  spello(SPELL_MARTIAL_TRANCE, 330, 220, 4, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

spello(SPELL_PROT_FIRE, 20, 10, 2,
	 POS_STANDING, TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_PETRIFY, 2400, 2000, 20, POS_STANDING,
        TAR_CHAR_ROOM, TRUE, MAG_AFFECTS);

  spello(SPELL_SATANIC_CURE, 200, 180, 3, POS_FIGHTING,
        TAR_CHAR_ROOM, FALSE, MAG_POINTS);

spello(SPELL_WRAITHFORM, 295, 155, 2, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_GROUP_ARMOR, 100, 50, 2, POS_STANDING,
        TAR_IGNORE, FALSE, MAG_GROUPS);

  spello(SPELL_GROUP_HEAL, 300, 160, 3, POS_STANDING,
        TAR_IGNORE, FALSE, MAG_GROUPS);

  spello(SPELL_GROUP_RECALL, 80, 60, 2, POS_STANDING,
        TAR_IGNORE, FALSE, MAG_GROUPS);

spello(SPELL_GROUP_FLY, 320, 200, 2, POS_STANDING,
        TAR_IGNORE, FALSE, MAG_GROUPS);

  spello(SPELL_GROUP_BLESS, 96, 65, 2, POS_STANDING,
        TAR_IGNORE, FALSE, MAG_GROUPS);

  spello(SPELL_CLONE, 650, 450, 5, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_SUMMONS);

  spello(SPELL_ANIMATE_DEAD, 170, 120, 3, POS_STANDING,
        TAR_OBJ_ROOM, FALSE, MAG_SUMMONS);

  spello(SPELL_GOLEM, 300, 220, 2, POS_STANDING,
        TAR_IGNORE, FALSE, MAG_SUMMONS); /* read one of the  */

  spello(SPELL_ILLUSION, 75, 55, 3, POS_STANDING,
        TAR_IGNORE, FALSE, MAG_SUMMONS); /* read one of the  */

  spello(SPELL_ELEMENTAL, 300, 250, 2, POS_STANDING,
        TAR_IGNORE, FALSE, MAG_SUMMONS);

  spello(SPELL_CALL_BEAR, 300, 250, 2, POS_STANDING,
  	TAR_IGNORE, FALSE, MAG_SUMMONS);

  spello(SPELL_CHARM, 300, 200, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_MANUAL);

  spello(SPELL_CONTROL_WEATHER, 75, 25, 5, POS_STANDING,
        TAR_IGNORE, FALSE, MAG_MANUAL);

  spello(SPELL_CREATE_FOOD, 30, 15, 4, POS_STANDING,
        TAR_IGNORE, FALSE, MAG_CREATIONS);

  spello(SPELL_CREATE_WATER, 30, 15, 4, POS_STANDING,
        TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE, MAG_MANUAL);

  spello(SPELL_CURE_LIGHT, 30, 10, 2, POS_FIGHTING,
        TAR_CHAR_ROOM, FALSE, MAG_POINTS);

  spello(SPELL_CURE_CRITIC, 60, 35, 2, POS_FIGHTING,
        TAR_CHAR_ROOM, FALSE, MAG_POINTS);

  spello(SPELL_CURE_SERIOUS, 100, 50, 2, POS_FIGHTING,
        TAR_CHAR_ROOM, FALSE, MAG_POINTS);

  spello(SPELL_HEAL, 150, 80, 2, POS_FIGHTING,
         TAR_CHAR_ROOM, FALSE, MAG_POINTS);

  spello(SPELL_REGENERATION, 280, 200, 3, POS_FIGHTING,
        TAR_CHAR_ROOM, FALSE, MAG_POINTS);

  spello(SPELL_REVITALIZE, 120, 70, 2, POS_FIGHTING,
        TAR_CHAR_ROOM, FALSE, MAG_POINTS);

  spello(SPELL_LAY_ON_HANDS, 130, 85, 4, POS_FIGHTING,
        TAR_CHAR_ROOM, FALSE, MAG_POINTS);

  spello(SPELL_REFRESH, 100, 80, 2, POS_RESTING,
        TAR_CHAR_ROOM, FALSE, MAG_POINTS);

  spello(SPELL_CURE_BLIND, 40, 15, 2, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_UNAFFECTS);

  spello(SPELL_REMOVE_CURSE, 65, 45, 3, POS_STANDING,
        TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE,
        MAG_UNAFFECTS | MAG_ALTER_OBJS);

  spello(SPELL_REMOVE_POISON, 60, 50, 2, POS_STANDING,
        TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_UNAFFECTS | 
MAG_ALTER_OBJS);

  spello(SPELL_RIGID_THINKING, 210, 190, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_MANUAL);

  spello(SPELL_WALL_OF_FOG, 130, 90, 3, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_ROOM);

  spello(SPELL_FIREWALL, 110, 65, 3, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_ROOM);

  spello(SPELL_ICEWALL, 170, 115, 3, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_ROOM);

  spello(SPELL_ENCHANT_WEAPON, 350, 100, 10, POS_STANDING,
        TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE, MAG_MANUAL);

  spello(SPELL_LOCATE_OBJECT, 85, 50, 1, POS_STANDING,
        TAR_OBJ_WORLD, FALSE, MAG_MANUAL);

  spello(SPELL_SUMMON, 225, 150, 3, POS_STANDING,
        TAR_CHAR_WORLD | TAR_NOT_SELF, FALSE, MAG_MANUAL);

spello(SPELL_ASTRAL_PROJECTION, 800, 750, 3, POS_STANDING,
        TAR_CHAR_WORLD | TAR_NOT_SELF, FALSE, MAG_MANUAL);

//spello(SPELL_FAITHEYES, 500, 450, 3, POS_STANDING,
//       TAR_IGNORE, TRUE, MAG_MANUAL);

  spello(SPELL_TELEPORT, 275, 135, 3, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_MANUAL);

  spello(SPELL_WORD_OF_RECALL, 35, 20, 2, POS_FIGHTING,
        TAR_CHAR_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_MANA_SWORD, 350, 250, 10, POS_STANDING,
        TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE, MAG_MANUAL);

  spello(SPELL_FARSIGHT, 185, 155, 3, POS_STANDING,
        TAR_CHAR_WORLD | TAR_NOT_SELF, FALSE, MAG_MANUAL);

  spello(SPELL_DEATH_SCYTHE, 800, 500, 1, POS_FIGHTING,
   TAR_IGNORE , FALSE, MAG_MANUAL);

  spello(SPELL_SIMULACRUM, 400, 300, 2, POS_STANDING,
   TAR_IGNORE, FALSE, MAG_MANUAL);

spello(SPELL_PEACE, 90, 50, 0, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_MANUAL);

  spello(SPELL_FEAR, 200, 150, 3, POS_FIGHTING,
	TAR_IGNORE, TRUE, MAG_MANUAL);

  spello(SPELL_OGHMA, 0, 0, 0, 0,
       TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_BARKSKIN, 50, 15, 1, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_GROW_ROOTS, 140, 100, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_NOT_SELF, FALSE, MAG_AFFECTS);

  spello(SPELL_ANCIENT_SPIRIT, 400, 320, 0, POS_STANDING,
        TAR_IGNORE, FALSE, MAG_SUMMONS);

  spello(SPELL_EXTEND_SPELLS, 610, 570, 1, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_MANA_BURN, 450, 380, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_DAMAGE);

  spello(SPELL_EXHAUSTION, 250, 190, 1, POS_STANDING,
        TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_DAMAGE);

  spello(SPELL_WHEEL_OF_FORTUNE, 300, 100, 3, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_LIQUID_AIR, 300, 200, 2, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_ROOM);

  spello(SPELL_ASPHYXIATE, 290, 200, 1, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_NOT_SELF, FALSE, MAG_AFFECTS);

  spello(SPELL_IRON_BODY, 380, 300, 2, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_DISPEL_MAGIC, 410, 250, 2, POS_STANDING,
			TAR_CHAR_ROOM, FALSE, MAG_UNAFFECTS);

  spello(SPELL_SILENCE, 330, 230, 2, POS_STANDING,
		TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_ROOM);

  spello(SPELL_GREATER_REFRESH, 250, 200, 1, POS_FIGHTING,
			TAR_CHAR_ROOM, FALSE, MAG_POINTS);

  spello(SPELL_RAISE_ABERRATION, 750, 650, 0, POS_STANDING,
	       TAR_IGNORE, FALSE, MAG_SUMMONS);

  spello(SPELL_FOREST_SANCTUARY, 200, 100, 1, POS_STANDING,
	       TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_ROOM);

  spello(SPELL_ILLUSION_FOREST, 100, 50, 1, POS_STANDING,
	       TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_ROOM);

  spello(SPELL_CALL_NIGHT, 100, 50, 1, POS_STANDING,
	       TAR_IGNORE, FALSE, MAG_MANUAL);

  spello(SPELL_CALL_DAYLIGHT, 100, 50, 1, POS_STANDING,
	       TAR_IGNORE, FALSE, MAG_MANUAL);

  spello(SPELL_UNDEAD_HORDE, 200, 150, 1, POS_STANDING,
	       TAR_IGNORE, FALSE, MAG_SUMMONS);

  spello(SPELL_STORM_SKY, 90, 40, 1, POS_STANDING,
	       TAR_IGNORE, FALSE, MAG_MANUAL);


  /* NON-castable spells should appear here */
  spello(SPELL_IDENTIFY, 600, 550, 1, POS_STANDING,
        TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_RESTORE_MANA, 0, 0, 0, 0,
        TAR_SELF_ONLY, FALSE, MAG_POINTS);


  /*
   * Declaration of skills - this actually doesn't do anything except
   * set it up so that immortals can use these skills by default.  The
   * min level to use the skill for other classes is set up in class.c.
   */

  skillo(SKILL_BACKSTAB);
  skillo(SKILL_BASH);
  skillo(SKILL_HIDE);
  skillo(SKILL_KICK);
  skillo(SKILL_PICK_LOCK);
  skillo(SKILL_STROKE);
  skillo(SKILL_RESCUE);
  skillo(SKILL_SNEAK);
  skillo(SKILL_STEAL);
  skillo(SKILL_TRACK);
  skillo(SKILL_COMBO);
  skillo(SKILL_SHIELD_BLOCK);
  skillo(SKILL_BREW);
  skillo(SKILL_CRAFT);
  skillo(SKILL_SWIM);
  skillo(SKILL_VITALIZE_HEALTH);
  skillo(SKILL_VITALIZE_MANA);
  skillo(SKILL_SPY);
  skillo(SKILL_HOWL);

  skillo(SKILL_SECOND_ATTACK);
  skillo(SKILL_THIRD_ATTACK);
  skillo(SKILL_FOURTH_ATTACK);
//  skillo(SKILL_FIFTH_ATTACK);
  skillo(SKILL_SCAN);
  skillo(SKILL_SABRE);
  skillo(SKILL_HAND_DAMAGE);
  skillo(SKILL_WHITE_GAS);
  skillo(SKILL_SUICIDE);
  skillo(SKILL_CIRCLE_ARROUND);
  skillo(SKILL_SECOND_STAB);
  skillo(SKILL_PARRY);
  skillo(SKILL_DODGE);
  skillo(SKILL_TUMBLE);
  skillo(SKILL_HIT);
  skillo(SKILL_BLUDGEON_E);
  skillo(SKILL_SLASH_E);
  skillo(SKILL_SLASH);
  skillo(SKILL_PIERCE_E);
  skillo(SKILL_BLUDGEON);
  skillo(SKILL_THRASH);
  skillo(SKILL_PIERCE);
  skillo(SKILL_BLAST);
  skillo(SKILL_PUNCH);
  skillo(SKILL_STAB);
  skillo(SKILL_DISARM);
  skillo(SKILL_BERZERK);
  skillo(SKILL_FIRST_AID);
  skillo(SKILL_HEADBASH);
  skillo(SKILL_ESCAPE);
  skillo(SKILL_DUAL_WIELD);
  skillo(SKILL_MEDITATE);
  skillo(SKILL_LEVITATE);
  skillo(SKILL_LISTEN);
  skillo(SKILL_WARSHOUT);
  skillo(SKILL_KNOCKOUT);
  skillo(SKILL_THROW);
  skillo(SKILL_GUT);
  skillo(SKILL_DROWN);
  skillo(SKILL_CHOP);
  skillo(SKILL_KICKFLIP);
  skillo(SKILL_WHIRLWIND);
  skillo(SKILL_IMPROVED_BERZERK);
  skillo(SKILL_POWER_KICK);
  skillo(SKILL_FOREST_WALK);
  skillo(SKILL_FOREST_AGILITY);
  skillo(SKILL_FOREST_WILDERNESS);
  skillo(SKILL_SNAKE_FIGHT);
  skillo(SKILL_MAKE_TRAPS);
  skillo(SKILL_SHURIKEN);
}

void improve_skill(struct char_data *ch, int skill)
{
  int percent;
  int newpercent;
  char skillbuf[MAX_STRING_LENGTH];

  if(IS_NPC(ch))
     return;

  percent = GET_SKILL(ch, skill);
  if (number(1, 200) > MIN(25,GET_WIS(ch)) + MIN(25,GET_INT(ch)))
     return;
  if (percent >= 95 || percent <= 0)
     return;
  newpercent = number(3, 5);
  percent += newpercent;
  SET_SKILL(ch, skill, percent);
  if (newpercent >= 4) {
     sprintf(skillbuf, "&GYou feel your %s in &Y%s&G improving.&n\r\n", (skill > MAX_SPELLS ? "skill" : "spell"), spells[skill]);
     send_to_char(skillbuf, ch);
  }
}

