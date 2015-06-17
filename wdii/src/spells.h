/* ************************************************************************
*   File: spells.h                                      Part of CircleMUD *
*  Usage: header file: constants and fn prototypes for spell system       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
#pragma once

#define DEFAULT_STAFF_LVL       12
#define DEFAULT_WAND_LVL        12
 
#define CAST_UNDEFINED  -1
#define CAST_SPELL      0
#define CAST_POTION     1
#define CAST_WAND       2
#define CAST_STAFF      3
#define CAST_SCROLL     4
#define CAST_ERB	5

#define MAG_DAMAGE      (1 << 0)
#define MAG_AFFECTS     (1 << 1)
#define MAG_UNAFFECTS   (1 << 2)
#define MAG_POINTS      (1 << 3)
#define MAG_ALTER_OBJS  (1 << 4)
#define MAG_GROUPS      (1 << 5)
#define MAG_MASSES      (1 << 6)
#define MAG_AREAS       (1 << 7)
#define MAG_SUMMONS     (1 << 8)
#define MAG_CREATIONS   (1 << 9)
#define MAG_MANUAL      (1 << 10)
#define MAG_ROOM        (1 << 11)

#define TYPE_UNDEFINED               -1
#define SPELL_RESERVED_DBC            0  /* SKILL NUMBER ZERO -- RESERVED */

/* PLAYER SPELLS -- Numbered from 1 to MAX_SPELLS */

#define SPELL_ARMOR                   1 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_TELEPORT                2 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BLESS                   3 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BLINDNESS               4 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BURNING_HANDS           5 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CALL_LIGHTNING          6 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CHARM                   7 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CHILL_TOUCH             8 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CLONE                   9 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_COLOR_SPRAY            10 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CONTROL_WEATHER        11 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CREATE_FOOD            12 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CREATE_WATER           13 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_BLIND             14 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_CRITIC            15 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_LIGHT             16 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURSE                  17 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_ALIGN           18 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_INVIS           19 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_MAGIC           20 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_POISON          21 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DISPEL_EVIL            22 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_EARTHQUAKE             23 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ENCHANT_WEAPON         24 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ENERGY_DRAIN           25 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_FIREBALL               26 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_HARM                   27 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_HEAL                   28 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_INVISIBLE              29 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_LIGHTNING_BOLT         30 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_LOCATE_OBJECT          31 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_MAGIC_MISSILE          32 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_POISON                 33 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_PROT_FROM_EVIL         34 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REMOVE_CURSE           35 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SANCTUARY              36 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SHOCKING_GRASP         37 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SLEEP                  38 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_STRENGTH               39 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SUMMON                 40 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_VENTRILOQUATE          41 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_WORD_OF_RECALL         42 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REMOVE_POISON          43 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SENSE_LIFE             44 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ANIMATE_DEAD           45 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DISPEL_GOOD            46 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_GROUP_ARMOR            47 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_GROUP_HEAL             48 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_GROUP_RECALL           49 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_INFRAVISION            50 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_WATERWALK              51 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_MANA_SHIELD            52 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DAMNED_CURSE           53 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REFRESH                54 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_HASTE                  55 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REGENERATION           56 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_PETRIFY		     57 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_GOD_PACT		     58 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SATAN_PACT	     59 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_END_WORLD		     60 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_MENTAL_RAGE	     61 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SATANIC_CURE	     62 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_FLY		     63 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_VULCAN		     64 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ENTANGLE               65
#define SPELL_FIRESHIELD             66
#define SPELL_ADRENALINE             67
#define SPELL_ICESTORM               68
#define SPELL_METEORSTORM            69
#define SPELL_FIRESTORM              70
#define SPELL_FORCE_SHIELD           71
#define SPELL_ENHANCED_ARMOR         72
#define SPELL_REVITALIZE             73
#define SPELL_GOLEM                  74
#define SPELL_ILLUSION               75
#define SPELL_WALL_OF_FOG            76
#define SPELL_FIREWALL		     77
#define SPELL_ICEWALL		     78
#define SPELL_MANA_SWORD	     79
#define SPELL_POISON_WEAPON	     80
#define SPELL_FIREBOLT		     81
#define SPELL_ICEBOLT		     82
#define SPELL_FARSIGHT		     83
#define SPELL_ELEMENTAL		     84
/* Last New Spells */ //TAEROM
#define SPELL_GREASE		     85
#define SPELL_CROMATIC_ORB	     86
#define SPELL_FLAMEARROW	     87
#define SPELL_SLOW		     88
#define SPELL_ELETRICSTORM           89
#define SPELL_IRON_SKIN              90
#define SPELL_PLATINUM_SKIN          91
#define SPELL_DIAMOND_SKIN           92
#define SPELL_ACIDARROW		     93
#define SPELL_MINUTE_METEOR          94
#define SPELL_GLACIAL_CONE           95
#define SPELL_AREA_LIGHTNING         96
#define SPELL_FIRE_SPIT              97
#define SPELL_GAS_SPIT               98
#define SPELL_FROST_SPIT             99
#define SPELL_ACID_SPIT             100
#define SPELL_LIGHTNING_SPIT        101
#define SPELL_BLADEBARRIER          102
#define SPELL_PRISMATIC_SPHERE      103
#define SPELL_ARMAGEDDOM	    104
#define SPELL_DELAYED_FIREBALL      105
#define SPELL_FRIENDS		    106
//20/10/2001
#define SPELL_DUMBNESS              107 //sorcerer
#define SPELL_PHANTOM_ARMOR         108 //necromancer
#define SPELL_SPECTRAL_WINGS        109 //psci e necro
#define SPELL_HOLY_FURY             110 //paladino
#define SPELL_CHAMPION_STRENGTH     111 //paladino
#define SPELL_HOLY_MACE             112 //paladino
#define SPELL_DEATH_SCYTHE          113 //necro
#define SPELL_SIMULACRUM            114
#define SPELL_TERROR                115
#define SPELL_DEATH_FINGER          116
#define SPELL_ENGULFING_DARKNESS    117
#define SPELL_BANSHEE_AURA          118
#define SPELL_SIPHON_LIFE           119
//27/10/2001
#define SPELL_BLINK		    120
#define SPELL_METEOR_SHOWER	    121
#define SPELL_HOLY_WORD	            122
#define SPELL_HOLY_SHOUT            123
#define SPELL_DEATHDANCE            124
#define SPELL_DEATH_RIPPLE	    125
#define SPELL_DEATH_WAVE            126
#define SPELL_PEACE		    127
#define SPELL_PROT_FIRE             128
#define SPELL_GROUP_FLY             129
#define SPELL_WRAITHFORM            130
#define SPELL_FEAR		    131
/* KELEMVOR and KARL 16/11/01 */
#define SPELL_MINOR_GLOBE           132
#define SPELL_MAJOR_GLOBE           133
#define SPELL_PHANTOM_FLAME         134
#define SPELL_SACRIFICE             135
#define SPELL_SKULL_TRAP            136
#define SPELL_GHASTLY_TOUCH         137
#define SPELL_VAMPIRIC_TOUCH        138
#define SPELL_PESTILENCE            139
#define SPELL_ANTIMAGIC_SHELL       140
#define SPELL_HOLY_BOLT             141
#define SPELL_DISPEL_NEUTRAL        142
#define SPELL_SHILLELAGH            143
#define SPELL_MAGICAL_STONE         144
#define SPELL_COMMAND               145
#define SPELL_HOLD_PERSON           146
#define SPELL_VOLCANO               147
#define SPELL_GEYSER                148
#define SPELL_GROUP_BLESS           149
#define SPELL_CORPOR_TREMBLING      150
#define SPELL_ICE_HEART             151
#define SPELL_POLTEIRGEIST          152
#define SPELL_BLIZZARD              153
#define SPELL_CREATE_LIGHT          154
#define SPELL_CLAIRVOYANCE          155
#define SPELL_CREATE_FLAMES         156
#define SPELL_CLAIRAUDIENCE         157
#define SPELL_WINGED_KNIFE          158
#define SPELL_ENHANCED_STRENGTH     159
#define SPELL_EGO_WHIP              160
#define SPELL_PROJECT_FORCE         161
#define SPELL_WRENCH                162
#define SPELL_FEEL_LIGHT            163
#define SPELL_BALISTIC_ATTACK       164
#define SPELL_SUP_INVISIBLE         165
#define SPELL_DETONATE              166
#define SPELL_AVERSION              167
#define SPELL_PSYCHIC_RAY           168
#define SPELL_MOLEC_AGITATION       169
#define SPELL_EMPATHY               170
#define SPELL_THOUGHT_SHIELD        171
#define SPELL_BIOFEEDBACK           172
#define SPELL_MENTAL_BARRIER        173
#define SPELL_INERTIAL_BARRIER      174
#define SPELL_CELL_ADJUSTMENT       175
#define SPELL_CONCENTRATION         176
#define SPELL_CONVICTION            177
#define SPELL_HOLY_SHIELD           178
#define SPELL_LAY_ON_HANDS          179
#define SPELL_CURE_SERIOUS          180
#define SPELL_REMOVE_FEAR           181  /* Falta Fazer */
#define SPELL_FLESH_ARMOR           182
#define SPELL_CONFUSION             183
#define SPELL_BRAVERY               184
#define SPELL_RIGID_THINKING        185 /* Falta Fazer --> Igual Charm */
#define SPELL_PARADI_CHRYSALIS      186
#define SPELL_ABUTILON              187
#define SPELL_AURA_SIGHT            188
#define SPELL_PAIN                  189
#define SPELL_DISPLACEMENT          190
#define SPELL_INTEL_FORTRESS        191
#define SPELL_LIFE_DRAINING         192
#define SPELL_BODY_WEAPONRY         193
#define SPELL_BODY_EQUILIBRIUM      194
#define SPELL_ADREN_CONTROL         195
#define SPELL_LEND_HEALTH           196
#define SPELL_COMBAT_MIND           197
#define SPELL_PSIONIC_BLAST         198
#define SPELL_PSYCHIC_DRAIN         199
#define SPELL_MARTIAL_TRANCE        200
#define SPELL_POST_HYPNOT_SUGG      201 /* Falta Fazer --> Igual Charm */
#define SPELL_ASTRAL_PROJECTION     202
#define SPELL_ULTIMA                203
#define SPELL_SUMMON_BAHAMUT        204
#define SPELL_BARKSKIN              205
#define SPELL_CALL_BEAR             206
#define SPELL_MUIR                  207 // religiao
#define SPELL_ILMANATEUR            208
#define SPELL_SELUNE                209
#define SPELL_HELM                  210
#define SPELL_TALOS                 211
#define SPELL_OGHMA                 212
#define SPELL_TEMPUS                213
//#define SPELL_FAITHEYES             214
#define SPELL_GROW_ROOTS			215 //-ips
#define SPELL_ANCIENT_SPIRIT	    216
#define SPELL_EXTEND_SPELLS			217
#define SPELL_MANA_BURN				218
#define SPELL_EXHAUSTION			219
#define SPELL_WHEEL_OF_FORTUNE		220
#define SPELL_LIQUID_AIR			221
#define SPELL_ASPHYXIATE			222
#define SPELL_IRON_BODY				223
#define SPELL_DISPEL_MAGIC			224
#define SPELL_SILENCE				225
#define SPELL_GREATER_REFRESH		226
#define SPELL_RAISE_ABERRATION		227
#define SPELL_FOREST_SANCTUARY		228
#define SPELL_ILLUSION_FOREST		229
#define SPELL_CALL_NIGHT			230
#define SPELL_CALL_DAYLIGHT			231
#define SPELL_UNDEAD_HORDE			232
#define SPELL_STORM_SKY				233


/* Insert new spells here, up to MAX_SPELLS */
#define MAX_SPELLS                  300

/* PLAYER SKILLS - Numbered from MAX_SPELLS+1 to MAX_SKILLS */
#define SKILL_BACKSTAB              301 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_BASH                  302 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_HIDE                  303 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_KICK                  304 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_PICK_LOCK             305 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_STROKE                306 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_RESCUE                307 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_SNEAK                 308 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_STEAL                 309 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_TRACK                 310 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_COMBO                 311 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_SHIELD_BLOCK          312 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_SWIM                  313
#define SKILL_VITALIZE_HEALTH       314
#define SKILL_VITALIZE_MANA         315
#define SKILL_SPY					316
#define SKILL_CRITICAL_ATTACK       317
#define SKILL_SECOND_ATTACK         318 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_THIRD_ATTACK          319 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_FOURTH_ATTACK         320 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_SCAN                  321 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_SABRE                 322 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_HAND_DAMAGE           323 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_WHITE_GAS             324 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_SUICIDE               325 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_CIRCLE_ARROUND        326 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_SECOND_STAB           327 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_PARRY                 328
#define SKILL_DODGE                 329
#define SKILL_TUMBLE                330
#define SKILL_HIT                   331
#define SKILL_BLUDGEON_E            332
#define SKILL_SLASH_E               333
#define SKILL_SLASH                 334
#define SKILL_PIERCE_E              335
#define SKILL_BLUDGEON              336
#define SKILL_PEEK                  337
#define SKILL_STYLE_SAVAGE          338
#define SKILL_STYLE_AIMING          339
#define SKILL_STYLE_BATTLECASTING   340
#define SKILL_THRASH                341
#define SKILL_PIERCE                342
#define SKILL_BLAST                 343
#define SKILL_PUNCH                 344
#define SKILL_STAB                  345
#define SKILL_DISARM                346
#define SKILL_BERZERK               347
#define SKILL_FIRST_AID             348
#define SKILL_HEADBASH              349
#define SKILL_ESCAPE                350
#define SKILL_DUAL_WIELD            351
#define SKILL_MEDITATE              352
#define SKILL_LEVITATE              353
#define SKILL_LISTEN                354
#define SKILL_AGILITY               355
#define SKILL_KAISER		    356
#define SKILL_WARSHOUT		    357
#define SKILL_KNOCKOUT              358
#define SKILL_THROW		    359
#define SKILL_GUT                   360
#define SKILL_DROWN		    361
#define SKILL_CHOP                  362
#define SKILL_KICKFLIP              363
#define SKILL_WHIRLWIND             364
#define SKILL_BLOOD_RITUAL          365
#define SKILL_STYLE_PRO		    366
#define SKILL_SPRITE		    367
#define SKILL_WEREWOLF		    368
#define SKILL_SUCUBUS		    369
#define SKILL_BEHEMOTH		    370
#define SKILL_TYRAEL  		    371
#define SKILL_SLAYER		    372
#define SKILL_IMPROVED_BERZERK      373
#define SKILL_POWER_KICK            374
#define SKILL_BREW			375
#define SKILL_CRAFT			376
#define SKILL_FOREST_WALK		377
#define SKILL_FOREST_AGILITY	    378
#define SKILL_FOREST_WILDERNESS	    379
#define SKILL_HOWL			380
#define SKILL_SILENT_SPELL	    381
#define SKILL_SNAKE_FIGHT	382
#define SKILL_MAKE_TRAPS	383
#define SKILL_SHURIKEN		384


//#define SKILL_SLANG                 373

//#define SKILL_BOW               145
//#define SKILL_SLING             146
//#define SKILL_CROSSBOW          147
//#define SKILL_SECOND_ATTACK	148 /* Second attack (DAK) */
//#define SKILL_THIRD_ATTACK	149 /* Third attack (DAK)  */
//#define SKILL_DISARM		150
//#define SKILL_SPRING            151 /* Ability to create springs */
//#define SKILL_FORAGE            152 /* Forage for food */
//#define SKILL_FISSION		153 /* Ability to turn object into energy */
//#define SKILL_EXAMINE		154 /* Ability of using examination unit */
//#define SKILL_HAND_TO_HAND	155 /* Hand 2 hand combat                */
//#define SKILL_UNARMED_COMBAT	156 /* Unarmed combat */
//#define SKILL_TRAP_AWARE	157 /* Trap aware */
//#define SKILL_PARRY		158 /* Can parry attacks */
//#define SKILL_RETREAT		159 /* The art of retreat */
//#define SKILL_PATHFINDING	160 /* The skill of pathfinding */

/* New skills may be added here up to MAX_SKILLS (500) */

/*
*  NON-PLAYER AND OBJECT SPELLS AND SKILLS
*  The practice levels for the spells and skills below are _not_ recorded
*  in the playerfile; therefore, the intended use is for spells and skills
*  associated with objects (such as SPELL_IDENTIFY used with scrolls of
*  identify) or non-players (such as NPC-only spells).
*/

#define SPELL_IDENTIFY               501
#define SPELL_FIRE_BREATH            502
#define SPELL_GAS_BREATH             503
#define SPELL_FROST_BREATH           504
#define SPELL_ACID_BREATH            505
#define SPELL_LIGHTNING_BREATH       506
#define SPELL_RESTORE_MANA           507

#define TRUE_TOP_SPELL_DEFINE        507
#define TOP_SPELL_DEFINE             599
/* NEW NPC/OBJECT SPELLS can be inserted here up to 699 */


/* WEAPON ATTACK TYPES */

#define TYPE_HIT                     600
#define TYPE_STING                   601
#define TYPE_WHIP                    602
#define TYPE_SLASH                   603
#define TYPE_BITE                    604
#define TYPE_BLUDGEON                605
#define TYPE_CRUSH                   606
#define TYPE_POUND                   607
#define TYPE_CLAW                    608
#define TYPE_MAUL                    609
#define TYPE_THRASH                  610
#define TYPE_PIERCE                  611
#define TYPE_BLAST                   612
#define TYPE_PUNCH                   613
#define TYPE_STAB                    614

/* new attack types can be added here - up to TYPE_SUFFERING */
#define TYPE_SUFFERING               699



#define SAVING_PARA   0
#define SAVING_ROD    1
#define SAVING_PETRI  2
#define SAVING_BREATH 3
#define SAVING_SPELL  4


#define TAR_IGNORE        1
#define TAR_CHAR_ROOM     2
#define TAR_CHAR_WORLD    4
#define TAR_FIGHT_SELF    8
#define TAR_FIGHT_VICT   16
#define TAR_SELF_ONLY    32 /* Only a check, use with i.e. TAR_CHAR_ROOM */
#define TAR_NOT_SELF     64 /* Only a check, use with i.e. TAR_CHAR_ROOM */
#define TAR_OBJ_INV     128
#define TAR_OBJ_ROOM    256
#define TAR_OBJ_WORLD   512
#define TAR_OBJ_EQUIP  1024

struct spell_info_type {
   byte min_position;   /* Position for caster   */
   int mana_min;        /* Min amount of mana used by a spell (highest lev) 
*/
   int mana_max;        /* Max amount of mana used by a spell (lowest lev) 
*/
   int mana_change;     /* Change in mana used by spell from lev to lev */

   int min_level[NUM_CLASSES];
   int routines;
   byte violent;
   int targets;         /* See below for use with TAR_XXX  */
};

/* Possible Targets:

   bit 0 : IGNORE TARGET
   bit 1 : PC/NPC in room
   bit 2 : PC/NPC in world
   bit 3 : Object held
   bit 4 : Object in inventory
   bit 5 : Object in room
   bit 6 : Object in world
   bit 7 : If fighting, and no argument, select tar_char as self
   bit 8 : If fighting, and no argument, select tar_char as victim 
(fighting)
   bit 9 : If no argument, select self, if argument check that it IS self.

*/

#define SPELL_TYPE_SPELL   0
#define SPELL_TYPE_POTION  1
#define SPELL_TYPE_WAND    2
#define SPELL_TYPE_STAFF   3
#define SPELL_TYPE_SCROLL  4
#define SPELL_TYPE_ERB	   5

/* Attacktypes with grammar */

struct attack_hit_type {
   const char   *singular;
   const char   *plural;
};


#define ASPELL(spellname) \
void    spellname(int level, struct char_data *ch, \
                  struct char_data *victim, struct obj_data *obj)

#define MANUAL_SPELL(spellname) spellname(level, caster, cvict, ovict);

ASPELL(spell_create_water);
ASPELL(spell_recall);
ASPELL(spell_teleport);
ASPELL(spell_summon);
ASPELL(spell_locate_object);
ASPELL(spell_charm);
ASPELL(spell_rigid_thinking);
ASPELL(spell_information);
ASPELL(spell_identify);
ASPELL(spell_enchant_weapon);
ASPELL(spell_detect_poison);
ASPELL(spell_farsight);
ASPELL(spell_control_weather);
ASPELL(spell_death_scythe);
ASPELL(spell_simulacrum);
ASPELL(spell_peace);
ASPELL(spell_mana_sword);
ASPELL(spell_fear);
ASPELL(spell_oghma) ;
ASPELL(spell_astral_projection) ;
ASPELL(spell_call_night);
ASPELL(spell_call_daylight);
ASPELL(spell_storm_sky);

//ASPELL(spell_faitheyes) ;
/* basic magic calling functions */

int find_skill_num(char *name);

int mag_damage(int level, struct char_data *ch, struct char_data *victim,
  int spellnum, int savetype);

void mag_affects(int level, struct char_data *ch, struct char_data *victim,
  int spellnum, int savetype);

void mag_groups(int level, struct char_data *ch, int spellnum, int 
savetype);

void mag_masses(int level, struct char_data *ch, int spellnum, int 
savetype);

void mag_areas(int level, struct char_data *ch, int spellnum, int savetype);

void mag_summons(int level, struct char_data *ch, struct obj_data *obj,
int spellnum, int savetype);

void mag_points(int level, struct char_data *ch, struct char_data *victim,
int spellnum, int savetype);

void mag_unaffects(int level, struct char_data *ch, struct char_data 
*victim,
  int spellnum, int type);

void mag_alter_objs(int level, struct char_data *ch, struct obj_data *obj,
  int spellnum, int type);

void mag_creations(int level, struct char_data *ch, int spellnum);

void mag_room(int level, struct char_data * ch, int spellnum);

int     call_magic(struct char_data *caster, struct char_data *cvict,
  struct obj_data *ovict, int spellnum, int level, int casttype);

void    mag_objectmagic(struct char_data *ch, struct obj_data *obj,
                        char *argument);

int     cast_spell(struct char_data *ch, struct char_data *tch,
  struct obj_data *tobj, int spellnum);


/* other prototypes */
void spell_level(int spell, int chclass, int level);
void init_spell_levels(void);
const char *skill_name(int num);
void improve_skill(struct char_data *ch, int skill);

