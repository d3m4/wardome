/* ************************************************************************
*   File: constants.c                                   Part of CircleMUD *
*  Usage: Numeric and string contants used by the MUD                     *
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

cpp_extern const char circlemud_version[] = {
        "WarDomeMUD II, version 0.21 "
};

/* strings corresponding to ordinals/bitvectors in structs.h ***********/

/* (Note: strings for class definitions in class.c instead of here) */

/* god names */
const char *god_names[] =
{
  "Immortals",
  "Elders",
  "Divinitys",
  "Helpers",
  "Builders",
  "Gods",
  "Supreme Gods",
  "Major Gods",
  "Implementor",
  "Owner Implementor",
  "\n"
};

/*
const char *god_wiznames[] =
{
  "Im",
  "EI",
  "Lr",
  "DG",
  "Dv",
  "GG",
  "SG",
  "MG",
  "Ip",
  "OI",
  "\n"
};
*/
const char *god_wiznames[] =
{

  "  Imm  ",
  " Elder ",
  "Divinity",
  "Helper ",
  "Builder",
  "  God  ",
  "Supreme",
  " Major ",
  " Imple ",
  " Owner ",
  "\n"
};

const char *remort_names[] =
{
  " Hunter",
  " Raider",
  " Knight",
  "Avenger",
  " Master",
  " Noble ",
  "  Hero ",
  "Veteran",
  "General",
  " Prince",
  "  King ",
  " Legend",
  "\n"
};

/* cardinal directions */
const char *dirs[] =
{
  "north",
  "east",
  "south",
  "west",
  "up",
  "down",
  "\n"
};

const char *zone_bits[] = {
  "OPEN",
  "CLOSED",
  "!RECALL",
  "!SUMMON",
  "REMORTS"
 };

/* inverse cardinal directions */
const char *from_dir[] =  {
    "the south",
    "the west",
    "the north",
    "the east",
    "below",
    "above",
    "\n"
};

/* ROOM_x */
const char *room_bits[] = {
  "DARK",
  "DEATH",
  "!MOB",
  "INDOORS",
  "PEACEFUL",
  "SOUNDPROOF",
  "!TRACK",
  "!MAGIC",
  "TUNNEL",
  "PRIVATE",
  "GODROOM",
  "HOUSE",
  "HCRSH",
  "ATRIUM",
  "OLC",
  "*",                          /* BFS MARK */
  "ARENA",
  "CHALLENGE",
  "LEARN",
  "TELEPORT",
  "GOOD_REGEN",
  "NO_REGEN_MANA",
  "NO_REGEN_HIT",
  "DONATION",
  "PATH",

  "\n"
};

char *teleport_bits[] = {
  "ForceLook",
  "EntryResetsTime",
  "SetsRandomTime",
  "Spin(NOT AVAIL)",
  "HasOBJinInv",
  "NoOBJinInv",
  "NoMesgDisplay",
  "SkipMobs",
  "SkipGroundObjects",
  "\n"
};

/* EX_x */
const char *exit_bits[] = {
  "DOOR",
  "CLOSED",
  "LOCKED",
  "PICKPROOF",
  "\n"
};


/* SECT_ */
const char *sector_types[] = {
  "Inside",
  "City",
  "Field",
  "Forest",
  "Hills",
  "Mountains",
  "Water (Swim)",
  "Water (No Swim)",
  "Underwater",
  "In Flight",
  "\n"
};


/*
 * SEX_x
 * Not used in sprinttype() so no \n.
 */
const char *genders[] =
{
  "Neutral",
  "Male",
  "Female"
};


/* POS_x */
const char *position_types[] = {
  "Dead",
  "Mortally wounded",
  "Incapacitated",
  "Stunned",
  "Sleeping",
  "Resting",
  "Sitting",
  "Fighting",
  "Standing",
  "\n"
};

/* PERM_x */
const char *perm_flags[] =
{
  "BLIND",
  "INVIS",
  "DET-ALIGN",
  "DET-INVIS",
  "DET-MAGIC",
  "SENSE-LIFE",
  "WATWALK",
  "SANCT",
  "GROUP",
  "CURSE",
  "INFRA",
  "POISON",
  "PROT-EVIL",
  "PROT-GOOD",
  "SLEEP",
  "!TRACK",
  "MANA-SHIELD",
  "DAMNED-CURSE",
  "SNEAK",
  "HIDE",
  "HASTE",
  "CHARM",
  "HELD",
  "SATAN_PACT",
  "GOD_PACT",
  "MENTAL",
  "FLY",
  "WGAS",
  "VULCAN",
  "FAITHEYES",
  "\n"
};

/* PLR_x */
const char *player_bits[] = {
  "KILLER",
  "THIEF",
  "FROZEN",
  "DONTSET",
  "WRITING",
  "MAILING",
  "CSH",
  "SITEOK",
  "NOSHOUT",
  "NOTITLE",
  "DELETED",
  "LOADRM",
  "!WIZL",
  "!DEL",
  "INVST",
  "CRYO",
  "DEAD",
  "!PK",
  "MEDITATE",
  "MESSAGING",
  "BANNED",
  "MSP",
  "SECOND_PLAYER",
  "\n"
};


/* MOB_x */
const char *action_bits[] = {
  "SPEC",
  "SENTINEL",
  "SCAVENGER",
  "ISNPC",
  "AWARE",
  "AGGR",
  "STAY-ZONE",
  "WIMPY",
  "AGGR_EVIL",
  "AGGR_GOOD",
  "AGGR_NEUTRAL",
  "MEMORY",
  "HELPER",
  "!CHARM",
  "!SUMMN",
  "!SLEEP",
  "!BASH",
  "!BLIND",
  "!TANGLE",
  "CLAN_MOB",
  "!STEAL",
  "!DISARM",
  "FIRE_MOB",
  "WATER_MOB",
    "\n"
};


/* PRF_x */
const char *preference_bits[] = {
  "BRIEF",
  "COMPACT",
  "DEAF",
  "!TELL",
  "D_HP",
  "D_MANA",
  "D_MOVE",
  "AUTOEXITS",
  "!HASSLE",
  "QUEST",
  "SUMMONABLE",
  "!REPEAT",
  "LIGHT",
  "C1",
  "C2",
  "!WIZNET",
  "L1",
  "L2",
  "!AUCTION",
  "!GOSSIP",
  "!GRATS",
  "D_MAX",
  "CROSSD",
  "AUTOLOOT",
  "AUTOGOLD",
  "AUTOSPLIT",
  "AFK",
  "AUTOASSIST",
  "INFOSTART",
  "D_MENTAL",
  "!CHAT",
  "\n"
};

/* PRF2_x */
const char *preference2_bits[] = {
  "D_EXP",
  "WDPROT",
  "MAP",
  "!REPLY",
  "HOUSERECALL",
  "!CFALAR",
  "!TIPS", 
  "!NEWBIE",
  "!PATHS",
  "!OOC",
  "\n"
};

/* AFF_x */
const char *affected_bits[] =
{
  "BLIND",
  "INVIS",
  "DET-ALIGN",
  "DET-INVIS",
  "DET-MAGIC",
  "SENSE-LIFE",
  "WATWALK",
  "SANCT",
  "GROUP",
  "CURSE",
  "INFRA",
  "POISON",
  "PROT-EVIL",
  "PROT-GOOD",
  "SLEEP",
  "!TRACK",
  "MANA-SHIELD",
  "DAMNED-CURSE",
  "SNEAK",
  "HIDE",
  "HASTE",
  "CHARM",
  "MENTAL",
  "SATAN_PACT",
  "GOD_PACT",
  "REGEN",
  "FLY",
  "WGAS",
  "TANGLED",
  "FIRESHIELD",
  "BERZERK",
  "\n"
};

/* AFF2_x */
const char *affected2_bits[] =
{
  "TRANSFORM",
  "WARSHOUT",
  "TERROR",
  "BANSHEE-AURA",
  "BLINK",
  "DEATHDANCE",
  "PROT_FIRE",
  "PASSDOOR",
  "MORALE",
  "SLOW",
  "MUIR1",
  "TEMPUS",
  "ILMA1",
  "SELUNE",
  "ROOT",
  "EXTEND-SPELL",
  "\n"
};

/* AFF3_x */
const char *affected3_bits[] =
{
  "SEM-USO",
  "\n"
};

/* CON_x */
const char *connected_types[] = {
  "Playing",
  "Disconnecting",
  "Get name",
  "Confirm name",
  "Get password",
  "Get new PW",
  "Confirm new PW",
  "Select sex",
  "Select class",
  "Reading MOTD",
  "Main Menu",
  "Get descript.",
  "Changing PW 1",
  "Changing PW 2",
  "Changing PW 3",
  "Self-Delete 1",
  "Self-Delete 2",
  "Disconnecting",
  "Object edit",
  "Room edit",
  "Zone edit",
  "Mobile edit",
  "Shop edit",
  "Help edit",
  "Select race",
  "Entering pdesc",
  "Check Terminal",
  "Text edit",
  "Action edit",
  "See summary",
  "Trigger edit",
  "Quest edit",
  "Choose PK",
  "Confirm race",
  "Confirm character",
  "Select Master",
  "\n"
};


/*
 * WEAR_x - for eq list
 * Not use in sprinttype() so no \n.
 */
const char *where[] = {
  "&C<&cused as light&C>&n      ",
  "&C<&cworn on finger&C>&n     ",
  "&C<&cworn on finger&C>&n     ",
  "&C<&cworn around neck&C>&n   ",
  "&C<&cworn around neck&C>&n   ",
  "&C<&cworn on body&C>&n       ",
  "&C<&cworn on head&C>&n       ",
  "&C<&cworn on legs&C>&n       ",
  "&C<&cworn on feet&C>&n       ",
  "&C<&cworn on hands&C>&n      ",
  "&C<&cworn on arms&C>&n       ",
  "&C<&cworn as shield&C>&n     ",
  "&C<&cworn about body&C>&n    ",
  "&C<&cworn about waist&C>&n   ",
  "&C<&cworn around wrist&C>&n  ",
  "&C<&cworn around wrist&C>&n  ",
  "&C<&cwielded&C>&n            ",
  "&C<&cheld&C>&n               ",
  "&C<&cdual wielded&C>&n       ",
  "&C<&cworn on ear&C>&n        ",
  "&C<&cworn on ear&C>&n        ",
  "&C<&cworn on face&C>&n       ",
  "&C<&cfloating in air&C>&n    "
};


/* WEAR_x - for stat */ 
const char *equipment_types[] = {
  "Used as light",
  "Worn on right finger",
  "Worn on left finger",
  "First worn around Neck",
  "Second worn around Neck",
  "Worn on body",
  "Worn on head",
  "Worn on legs",
  "Worn on feet",
  "Worn on hands",
  "Worn on arms",
  "Worn as shield",
  "Worn about body",
  "Worn around waist",
  "Worn around right wrist",
  "Worn around left wrist",
  "Wielded",
  "Held",
  "Dual Wielded",
  "Hook on right ear",
  "Hook on left ear",
  "Worn on face",
  "Floating in air",
  "\n"
};

const int wear_order_index[NUM_WEARS] = {
  WEAR_FLOAT,
  WEAR_HEAD,
  WEAR_FACE,
  WEAR_EAR_R,
  WEAR_EAR_L,
  WEAR_NECK_1,
  WEAR_NECK_2,
  WEAR_ABOUT,
  WEAR_BODY,
  WEAR_WAIST,
  WEAR_ARMS,
  WEAR_WRIST_R,
  WEAR_WRIST_L,
  WEAR_HANDS,
  WEAR_FINGER_R,
  WEAR_FINGER_L,
  WEAR_WIELD,
  WEAR_DWIELD,
  WEAR_HOLD,
  WEAR_LIGHT,
  WEAR_SHIELD,
  WEAR_LEGS,
  WEAR_FEET
};

/* ITEM_x (ordinal object types) */
const char *item_types[] = {
  "UNDEFINED",
  "LIGHT",
  "SCROLL",
  "WAND",
  "STAFF",
  "WEAPON",
  "FIRE WEAPON",
  "MISSILE",
  "TREASURE",
  "ARMOR",
  "POTION",
  "WORN",
  "OTHER",
  "TRASH",
  "TRAP",
  "CONTAINER",
  "NOTE",
  "LIQ CONTAINER",
  "KEY",
  "FOOD",
  "MONEY",
  "PEN",
  "BOAT",
  "FOUNTAIN",
  "SPELLBOOK",
  "MINERALS",
  "ERB",
  "\n"
};


/* ITEM_WEAR_ (wear bitvector) */
const char *wear_bits[] = {
  "TAKE",
  "FINGER",
  "NECK",
  "BODY",
  "HEAD",
  "LEGS",
  "FEET",
  "HANDS",
  "ARMS",
  "SHIELD",
  "ABOUT",
  "WAIST",
  "WRIST",
  "WIELD",
  "HOLD",
  "EAR",
  "FACE",
  "FLOAT",
  "\n"
};


/* ITEM_x (extra bits) */
const char *extra_bits[] = {
  "GLOW",
  "HUM",
  "!RENT",
  "!DONATE",
  "!INVIS",
  "INVISIBLE",
  "MAGIC",
  "!DROP",
  "BLESS",
  "!GOOD",
  "!EVIL",
  "!NEUTRAL",
  "!SORCERER",
  "!CLERIC",
  "!THIEF",
  "!WARRIOR",
  "!SELL",
  "!NECROMANCER",
  "!PALADIN",
  "!NINJA",
  "!RANGER",
  "!PSIONICIST",
  "!WARLOCK",
  "!SHAMAN",
  "!BARBARIAN",
  "!VAMPIRE",
  "!DROW",
  "WIND",
  "EARTH",
  "WATER",
   "FIRE", //daki pra baixo nao funciona
  "!TROLL",
  "!GITH",
  "!GNOME",
  "!LIZARDMAN",
  "!SEAELF",
  "!ORC",
  "!GORAK",
  "!MAGICUSER",
  "QUEST",
  //"WATER",
  //"EARTH",
  //"WIND",
  "\n"
};


/* APPLY_x */
const char *apply_types[] = {
  "NONE",
  "STR",
  "DEX",
  "INT",
  "WIS",
  "CON",
  "CHA",
  "CLASS",
  "LEVEL",
  "AGE",
  "CHAR_WEIGHT",
  "CHAR_HEIGHT",
  "MAXMANA",
  "MAXHIT",
  "MAXMOVE",
  "GOLD",
  "EXP",
  "ARMOR",
  "HITROLL",
  "DAMROLL",
  "SAVING_PARA",
  "SAVING_ROD",
  "SAVING_PETRI",
  "SAVING_BREATH",
  "SAVING_SPELL",
  "\n"
};


/* CONT_x */
const char *container_bits[] = {
  "CLOSEABLE",
  "PICKPROOF",
  "CLOSED",
  "LOCKED",
  "\n",
};


/* LIQ_x */
const char *drinks[] =
{
  "water",
  "beer",
  "wine",
  "ale",
  "dark ale",
  "whisky",
  "lemonade",
  "firebreather",
  "local speciality",
  "slime mold juice",
  "milk",
  "tea",
  "coffee",
  "blood",
  "salt water",
  "clear water",
  "\n"
};


/* other constants for liquids ******************************************/


/* one-word alias for each drink */
const char *drinknames[] =
{
  "water",
  "beer",
  "wine",
  "ale",
  "ale",
  "whisky",
  "lemonade",
  "firebreather",
  "local",
  "juice",
  "milk",
  "tea",
  "coffee",
  "blood",
  "salt",
  "water",
  "\n"
};


/* effect of drinks on hunger, thirst, and drunkenness -- see values.doc */
int drink_aff[][3] = {
  {0, 1, 10},
  {3, 2, 5},
  {5, 2, 5},
  {2, 2, 5},
  {1, 2, 5},
  {6, 1, 4},
  {0, 1, 8},
  {10, 0, 0},
  {3, 3, 3},
  {0, 4, -8},
  {0, 3, 6},
  {0, 1, 6},
  {0, 1, 6},
  {0, 2, -1},
  {0, 1, -2},
  {0, 0, 13}
};


/* color of the various drinks */
const char *color_liquid[] =
{
  "clear",
  "brown",
  "clear",
  "brown",
  "dark",
  "golden",
  "red",
  "green",
  "clear",
  "light green",
  "white",
  "brown",
  "black",
  "red",
  "clear",
  "crystal clear",
  "\n"
};


/*
 * level of fullness for drink containers
 * Not used in sprinttype() so no \n.
 */
const char *fullness[] =
{
  "less than half ",
  "about half ",
  "more than half ",
  ""
};


/* str, int, wis, dex, con applies **************************************/


/* [ch] strength apply (all) */


cpp_extern const struct str_app_type str_app[] = {
  {0, 0, 0, 0},       /* str = 0 */
  {0,0, 10, 1},       /* str = 1 */
  {0, 0, 25, 2},
  {0, 0, 50, 3},
  {0, 0, 100, 4},
  {0, 0, 120, 5},      /* str = 5 */
  {0, 0,140, 6},
  {0, 0,160, 7},
  {0, 0, 180, 8},
  {0, 0, 200, 9},
  {0, 0, 250, 10},      /* str = 10 */
  {0, 0, 300, 11},
  {0, 12, 350, 12},
  {0, 13, 400, 13},
  {0, 14, 450, 14},
  {0, 15, 500, 15},      /* str = 15 */
  {0, 16, 550, 16},
  {0, 17, 600, 18},
  {0, 18, 650, 20},      /* str = 18 */
  {0, 19, 960, 40},
  {0, 20, 970, 40},      /* str = 20 */
  {0, 21, 980, 40},
  {0, 22, 1000, 40},
  {0, 23, 1100, 40},
  {0, 24, 1200, 40},
  {0, 25, 1500, 40},    /* str = 25 */
  {0, 26, 1550, 45},      
  {0, 27, 1560, 45},      
  {0, 28, 1570, 45},      
  {0, 29, 1580, 45},      
  {0, 30, 1600, 45},      // 30
  {0, 31, 1610, 46},      
  {0, 32, 1620, 46},      
  {0, 33, 1630, 46},      
  {0, 34, 1640, 46},      
  {0, 35, 1750, 46},       //  35
  {0, 36, 1760, 46},      
  {0, 37, 1770, 46},      
  {0, 38, 1780, 46},      
  {0, 39, 1790, 46},      
  {0, 40, 1820, 48},       //  40
  {0, 41, 1830, 48},      
  {0, 42, 1860, 48},      
  {0, 43, 1870, 48},      
  {0, 44, 1880, 48},      
  {0, 45, 1920, 50},       //  45
  {0, 46, 1940, 50},      
  {0, 47, 1960, 50},      
  {0, 48, 1970, 50},      
  {0, 49, 1980, 50},      
  {0, 50, 2010, 50},       //  50
  {0, 52, 2020, 50},      
  {0, 54, 2040, 50},      
  {0, 56, 2060, 50},      
  {0, 58, 2080, 50},      
  {0, 60, 2130, 55},       //  55
  {0, 62, 2140, 55},      
  {0, 64, 2150, 55},      
  {0, 66, 2160, 55},      
  {0, 68, 2170, 55},      
  {0, 70, 2180, 55},       //  60
  {0, 72, 2190, 55},      
  {0, 74, 2195, 55},      
  {0, 76, 2199, 55},      
  {0, 78, 2200, 55},      
  {0, 80, 2201, 60},       //  65
  {0, 82, 2200, 60},      
  {0, 84, 2204, 60},      
  {0, 86, 2230, 60},      
  {0, 88, 2240, 60},      
  {0, 90, 2350, 70},       //  70
  {0, 91, 2460, 70},      
  {0, 92, 2470, 70},      
  {0, 93, 2480, 70},      
  {0, 94, 2480, 70},      
  {0, 95, 2490, 70},       //  75
  {0, 96, 2510, 70},      
  {0, 97, 2520, 70},      
  {0, 98, 2530, 70},      
  {0, 99, 2540, 70},      
  {0, 100, 2670, 80},       //  80
  {0, 101, 2670, 80},      
  {0, 102, 2670, 80},      
  {0, 103, 2670, 80},      
  {0, 104, 2670, 80},      
  {0, 105, 2680, 80},       //  85
  {0, 106, 2710, 80},      
  {0, 107, 2720, 80},      
  {0, 108, 2730, 80},      
  {0, 109, 2740, 80},      
  {0, 110, 2820, 85},       //  90
  {0, 111, 2830, 85},      
  {0, 112, 2840, 85},      
  {0, 113, 2850, 85},      
  {0, 114, 2960, 85},      
  {0, 115, 2970, 90},       //  95
  {0, 116, 2980, 90},      
  {0, 117, 2980, 90},      
  {0, 118, 2950, 90},      
  {0, 119, 3000, 90},      
  {0, 120, 3050, 100}       //  100

} ;





/* [dex] skill apply (thieves only) */
cpp_extern const struct dex_skill_type dex_app_skill[] = {
  {-99, -99, -90, -99, -60},    /* dex = 0 */
  {-90, -90, -60, -90, -50},    /* dex = 1 */
  {-80, -80, -40, -80, -45},
  {-70, -70, -30, -70, -40},
  {-60, -60, -30, -60, -35},
  {-50, -50, -20, -50, -30},    /* dex = 5 */
  {-40, -40, -20, -40, -25},
  {-30, -30, -15, -30, -20},
  {-20, -20, -15, -20, -15},
  {-15, -10, -10, -20, -10},
  {-10, -5, -10, -15, -5},      /* dex = 10 */
  {-5, 0, -5, -10, 0},
  {0, 0, 0, -5, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},              /* dex = 15 */
  {0, 5, 0, 0, 0},
  {5, 10, 0, 5, 5},
  {10, 15, 5, 10, 10},          /* dex = 18 */
  {15, 20, 10, 15, 15},
  {15, 20, 10, 15, 15},         /* dex = 20 */
  {20, 25, 10, 15, 20},
  {20, 25, 15, 20, 20},
  {25, 25, 15, 20, 20},
  {25, 30, 15, 25, 25},
  {25, 30, 15, 25, 25}          /* dex = 25 */
};



/* [dex] apply (all) */
cpp_extern const struct dex_app_type dex_app[] = {
  {-7, -7, 6},          /* dex = 0 */
  {-6, -6, 5},          /* dex = 1 */
  {-4, -4, 5},
  {-3, -3, 4},
  {-2, -2, 3},
  {-1, -1, 2},          /* dex = 5 */
  {0, 0, 1},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},            /* dex = 10 */
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, 0},
  {0, 0, -1},           /* dex = 15 */
  {1, 1, -2},
  {2, 2, -3},
  {2, 2, -4},           /* dex = 18 */
  {3, 3, -4},
  {3, 3, -4},           /* dex = 20 */
  {4, 4, -5},
  {4, 4, -5},
  {4, 4, -5},
  {5, 5, -6},
  {5, 5, -6}            /* dex = 25 */
};



/* [con] apply (all) */
cpp_extern const struct con_app_type con_app[] = {

  {0, 20},             /* con = 0 */
  {0, 25},             
  {0, 30},
  {0, 35},
  {0, 40},
  {0, 45},             /* con = 5 */
  {0, 50},
  {0, 55},
  {0, 60},
  {0, 65},
  {0, 70},              /* con = 10 */
  {0, 25},             
  {0, 30},
  {0, 35},
  {0, 40},
  {0, 45},             /* con = 15 */
  {1, 50},
  {2, 55},
  {3, 60},
  {4, 65},
  {5, 70},              /* con = 20 */
  {6, 25},             
  {6, 30},
  {6, 35},
  {6, 40},
  {6, 45},             /* con = 25 */
  {6, 50},
  {6, 55},
  {6, 60},
  {6, 65},
  {7, 70},              /* con = 30 */
  {7, 25},             
  {7, 30},
  {7, 35},
  {7, 40},
  {7, 45},             /* con = 35 */
  {7, 50},
  {7, 55},
  {7, 60},
  {7, 65},
  {8, 70},              /* con = 40 */
  {8, 25},             
  {8, 30},
  {8, 35},
  {8, 40},
  {8, 45},             /* con = 45 */
  {8, 50},
  {8, 55},
  {8, 60},
  {8, 65},
  {9, 70},              /* con = 50 */
  {9, 25},             
  {9, 30},
  {9, 35},
  {9, 40},
  {9, 45},             /* con = 55 */
  {9, 50},
  {9, 55},
  {9, 60},
  {9, 65},
  {10, 70},              /* con = 60 */
  {10, 25},             
  {10, 30},
  {10, 35},
  {10, 40},
  {10, 45},             /* con = 65 */
  {10, 50},
  {10, 55},
  {10, 60},
  {10, 65},
  {11, 70},              /* con = 70 */
  {11, 25},             
  {11, 30},
  {11, 35},
  {11, 40},
  {11, 45},             /* con = 75 */
  {11, 50},
  {11, 55},
  {11, 60},
  {11, 65},
  {11, 70},              /* con = 80 */
  {12, 25},             
  {12, 30},
  {12, 35},
  {12, 40},
  {12, 45},             /* con = 85 */
  {12, 50},
  {12, 55},
  {12, 60},
  {12, 65},
  {13, 70},              /* con = 90 */
  {13, 25},             
  {13, 30},
  {13, 35},
  {13, 40},
  {13, 45},             /* con = 95 */
  {13, 50},
  {13, 55},
  {13, 60},
  {13, 65},
  {14, 70}              /* con = 100 */
  
};



/* [int] apply (all) */
cpp_extern const struct int_app_type int_app[] = {
  {3},          /* int = 0 */
  {5},          /* int = 1 */
  {7},
  {8},
  {9},
  {10},         /* int = 5 */
  {11},
  {12},
  {13},
  {15},
  {17},         /* int = 10 */
  {19},
  {22},
  {25},
  {30},
  {35},         /* int = 15 */
  {40},
  {45},
  {50},         /* int = 18 */
  {53},
  {55},         /* int = 20 */
  {56},
  {57},
  {58},
  {59},
  {60} ,         /* int = 25 */
  {60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60},
{60}

};


/* [wis] apply (all) */
cpp_extern const struct wis_app_type wis_app[] = {
  {0},  /* wis = 0 */
  {0},  /* wis = 1 */
  {0},
  {0},
  {0},
  {0},  /* wis = 5 */
  {0},
  {0},
  {0},
  {0},
  {0},  /* wis = 10 */
  {0},
  {2},
  {2},
  {3},
  {3},  /* wis = 15 */
  {3},
  {4},
  {5},  /* wis = 18 */
  {6},
  {6},  /* wis = 20 */
  {6},
  {6},
  {7},
  {7},
  {7} , /* wis = 25 */
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7},
{7}
};



const char *spell_wear_off_msg[] = {
  "RESERVED DB.C",              /* 0 */
  "You feel less protected.",   /* 1 */
  "!Teleport!",
  "You feel less righteous.",
  "You feel a cloak of blindness disolve.",
  "!Burning Hands!",            /* 5 */
  "!Call Lightning",
  "You feel more self-confident.",
  "You feel your strength return.",
  "!Clone!",
  "!Color Spray!",              /* 10 */
  "!Control Weather!",
  "!Create Food!",
  "!Create Water!",
  "!Cure Blind!",
  "!Cure Critic!",              /* 15 */
  "!Cure Light!",
  "You feel more optimistic.",
  "You feel less aware.",
  "Your eyes stop tingling.",
  "The detect magic wears off.",/* 20 */
  "The detect poison wears off.",
  "!Dispel Evil!",
  "!Earthquake!",
  "!Enchant Weapon!",
  "!Energy Drain!",             /* 25 */
  "!Fireball!",
  "!Harm!",
  "!Heal!",
  "You feel yourself exposed.",
  "!Lightning Bolt!",           /* 30 */
  "!Locate object!",
  "!Magic Missile!",
  "You feel less sick.",
  "You feel less protected.",
  "!Remove Curse!",             /* 35 */
  "The white aura around your body fades.",
  "!Shocking Grasp!",
  "You feel less tired.",
  "You feel weaker.",
  "!Summon!",                   /* 40 */
  "!Ventriloquate!",
  "!Word of Recall!",
  "!Remove Poison!",
  "You feel less aware of your suroundings.",
  "!Animate Dead!",             /* 45 */
  "!Dispel Good!",
  "!Group Armor!",
  "!Group Heal!",
  "!Group Recall!",
  "Your night vision seems to fade.",   /* 50 */
  "Your feet seem less boyant.",
  "Your mana shield slowly fades out of existence.",
  "You feel really better!",
  "!Refresh!",
  "You feel slower.", /* 55 */
  "!REGENERATION!",
  "!MENTAL RAGE!",
  "The yellow aura around your body fades.",
  "The red aura around your body fades.",
  "!ENDWORLD!", /* 60 */
  "You starts to move slower...",
  "!SATANICCURE!",
  "Your feets slowly touch the floor.",
  "You feel you are not thirsty of blood now.",
  "You are finally free of the vines that bind.", /* 65 */
  "Your magic shield fades away happily.",
  "Your feel yourself normal again.",
  "!ICESTORM!",
  "!METEORSTORM!",
  "!FIRESTORM!", /* 70 */
  "Your force shield fades away happily.",
  "You feel your protection fading.",
  "!REVITALIZE!",
  "!GOLEM!",
  "!ILLUSION!", /* 75 */
  "The fog seams to clear out.",
  "The fire wall disappear in the air.",
  "The ice wall melts by your eyes.",
  "!MANASWORD!",
  "!POISONWEAPON!", /* 80 */
  "!FIREBOLT!",
  "!ICEBOLT!",
  "!FARSIGHT!",
  "!ELEMENTAL!",
  "!GREASE",   /* 85 */
  "!CROMATIC_ORB",
  "!FLAMEARROW",
  "You feel yourself fast.",
  "!ELETRICSTORM",
  "You feel your skin normal.",  /* 90 */
  "You feel your skin normal.",
  "You feel your skin normal.",
  "!ACIDARROW",
  "!MINUTE_METEOR",
  "!CONE_OF_COLD",  /* 95 */
  "!AREA_LIGHTNING",
  "!FIRE_SPIT",
  "!GAS_SPIT",
  "!FROST_SPIT",
  "!ACID_SPIT",    /* 100 */
  "!LIGHTNING_SPIT",
  "!BLADEBARRIER",
  "!PRIZMATIC_SPHERE",
  "!ARMAGEDDOM",
  "!DELAYED_FIREBALL",     /* 105*/
  "You feel you arent charismatic anymore.",
  "You feel smarter.",
  "The phantoms leave you.",
  "Your spectral wings vanish and you fall to the ground.",
  "You feel your fighting ability decrease.", /* 110 */
  "You no longer feel like you can take on an army.",
  "!HOLY_MACE",
  "!Death scythe!",
  "!Simulacrum!",
  "Your fear is gone.",/* 115 */
  "!Death finger!",
  "Darkness leaves your body.",
  "The banshee aura no longer exists.",
  "!SIPHON_LIFE!",
  "You feel more in tune with your surroundings.",/* 120 */
  "!Meteor Shower!",
  "!Holy Word!",
  "!Holy Shout!",
  "You feel your life force return to you!",
  "!Death Ripple!",/* 125 */
  "!Death Wave!",
  "!Peace!",
  "You feel less insulated.",
  "!Group Fly!",
  "You feel yourself becoming solid again.", //130
  "Your fear fades.",
  "The minor globe no longer protects you.",
  "The major globe no longer protects you.",
  "!Phanton flame!",
  "You no longer needs to sacrifice yourself.",     //135
  "!Skull trap!",
  "Your strength returns.",
  "Your strength returns.",
  "The pestilence leaves your body.",
  "The antimagic shell no longer protects you.", //140
  "!Holy Bolt!",
  "!Dispel Neutral!",
  "!Shilelaugh!",
  "!Magic Stone!",
  "!Command!",     //145
  "You are no longer held.",
  "!Volcano!",
  "!Geyser!",
  "The group bless no longer works.",
  "!Corpor Trembling!",           // 150
  "!Ice Heart!",
  "!Poltersgeist!",
  "!Blizzard!",
  "!Create Light!",
  "Your eyes return to normal.",    // 155
  "!Create Flames!",
  "Your ears return to normal.",
  "!Winged Knife!",
  "Your strength returns to its original form.",
  "!Ego Whip!",    // 160
  "!Project force!",
  "!Wrench!",
  "The lights leave your eyes.",
  "!Balistic Attack!",
  "Your invisibility no longer exists.", // 165
  "!Detonate!",
  "The aversion leaves your body, you can fight now.",
  "!Psychic Ray!",
  "!Molec Agitation!",
  "Your empathy no longer exists.",       //170
  "The thought shield no longer protects you.",
  "The biofeedback no longer protects you.",
  "The barrier of mental forcer no longer protects you.",
  "The inertial barrier no longer protects you.",
  "The cell adjustment is over.",  //175
  "You are no longer concentrated.",
  "You lost your conviction.",
  "You lost your holy shield.",
  "!Lay on hands!",
  "!Cure Serious!",           // 180
  "!Remove Fear!",
  "The flesh armor leaves your body.",
  "You are no longer confused.",
  "You lost your bravery.",
  "!Rigid Thinking!",      //185
  "The power of Paradi Chrysalis leaves your soul.",
  "The power of Abutilion leaves your soul.",
  "The helper aura vanishes.",
  "You no longer fells pain.",
  "Your body is not displaced anymore.",  //190
  "Your wisdom return to normal.",
  "!Life Drainning!",
  "Your body is no longer a weapon.",
  "You lost the equilibrium of your body.",
  "Your dexterity returns to normal.",  //195
  "!Lend Health!",
  "Your mind is no longer in combat mode.",
  "!Psyonic blast!",
  "!Psyonic drain!",
  "You are no longer in martial trance.",  //200
  "!Post Hypno!",
  "!Astral Projection!",
  "!ULTIMA!",
  "!Summon Bahamut!",
  "You feel your skin back to normal.",//205
  "!CALL_BEAR!",
  "The power of Muir fades!",
  "The power of Ilmanateur fades",
  "The power of Selune fades",
  "!SPELL_HELM!",
  "!SPELL_TALOS!",
  "!SPELL_OGHMA!",
  "The power of Tempus is over!",
  "!SPELL_FAITHEYES!",
  "!SPELL_GROW_ROOTS!",
  "!ANCIENT_SPIRIT!",
  "Your spells are no longer extended.",
  "!MANA_BURN!",
  "!EXHAUSTION!",
  "&WThe effects of &yThe Wheel&W leave your soul.",
  "The &Bwater&n here turns into &Cair&n again.",
  "&WYou feel your breathing normal again.&n",
  "&nYour body is no longer made of &Kiron&n.",
  "!DISPEL_MAGIC!",
  "&nThis place is no more silent.",
  "!GREATER_REFRESH!",
  "!RAISE_ABERRATION!",
  "&gThis place is no longer protected by the spirits of the forest.&n",
  "&gThis place is no longer a forest.&n",
  "!CALL_NIGHT!",
  "!CALL_DAYLIGHT!",
  "!UNDEAD_HORDE!",
  "!STORM_SKY!",
  "\n"
};

const char *npc_class_types[] = {
  "Normal",
  "Undead",
  "Dragon",
  "Demon",
  "Spirit",
  "Animal",
  "Human",
  "Giant",
  "\n"
};

const char *npc_class_abbrevs[] = {
  "Nm",
  "Un",
  "Dr",
  "De",
  "Sp",
  "An",
  "Hu",
  "Gi",
  "\n"
};



int rev_dir[] =
{
  2,
  3,
  0,
  1,
  5,
  4
};


int movement_loss[] =
{
  1,    /* Inside     */
  1,    /* City       */
  2,    /* Field      */
  3,    /* Forest     */
  4,    /* Hills      */
  6,    /* Mountains  */
  4,    /* Swimming   */
  1,    /* Unswimable */
  1,    /* Flying     */
  5     /* Underwater */
};

#if defined(OASIS_MPROG)
/*
 * Definitions necessary for MobProg support in OasisOLC
 */
const char *mobprog_types[] = {
  "INFILE",
  "ACT",
  "SPEECH",
  "RAND",
  "FIGHT",
  "DEATH",
  "HITPRCNT",
  "ENTRY",
  "GREET",
  "ALL_GREET",
  "GIVE",
  "BRIBE",
  "\n"
};
#endif

/* Not used in sprinttype(). */
const char *weekdays[] = {
  "the Day of the Moon",
  "the Day of the Bull",
  "the Day of the Deception",
  "the Day of Thunder",
  "the Day of Freedom",
  "the day of the Great Gods",
  "the Day of the Sun"
};


/* Not used in sprinttype(). */
const char *month_name[] = {
  "Month of Winter",            /* 0 */
  "Month of the Winter Wolf",
  "Month of the Frost Giant",
  "Month of the Old Forces",
  "Month of the Grand Struggle",
  "Month of the Spring",
  "Month of Nature",
  "Month of Futility",
  "Month of the Dragon",
  "Month of the Sun",
  "Month of the Heat",
  "Month of the Battle",
  "Month of the Dark Shades",
  "Month of the Shadows",
  "Month of the Long Shadows",
  "Month of the Ancient Darkness",
  "Month of the Great Evil"
};

const char *pc_clsl_types[] = {
  "[A]    &CSorcerer&n	",
  "[B]    &BCleric&n	",
  "[C]    &GThief&n 	",
  "[D]    &rWarrior&n	",
  "[E]    &WNecromancer&n	",
  "[F]    &yPaladin&n	",
  "[G]    &cNinja&n 	",
  "[H]    &YRanger&n	",
  "[I]    &gPsionicist&n	",
  "[J]    &mWarlock&n	",
  "[L]    &RBarbarian&n	",
  "[M]    &bShaman&n	",
  "\n"
};

const char *room_affections[] = {
	"FOG",
	"FIREWALL",
	"ICEWALL",
	"LIQUID_AIR!",
	"SILENCE",
	"\n"
};

const char *class_types[] = {
  "Undefined",
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

const char *race_types[] = {
  "Undefined",
  "Humans",
  "Drow",
  "Dwarf",
  "Elf",
  "Ogre",
  "Orc",
  "Troll",
  "Gith",
  "Gnome",
  "Lizardman",
  "Seaelf",
  "Gorak",
  "Dunedain",
  "AncientDrow",
  "Naugrim",
  "HighElf",
  "HillOgre",
  "BugBear",
  "CaveTroll",
  "Lich",
  "Tinker",
  "Draconian",
  "HighSeaElf",
  "Archons",
  "\n"
};
