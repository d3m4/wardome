/* ************************************************************************
*   File: structs.h                                     Part of CircleMUD *
*  Usage: header file for central structures and contstants               *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

/*#include "buffer_opt.h"*/		/* Catch-22 otherwise. */

/*
* Intended use of this macro is to allow external packages to work with
* a variety of CircleMUD versions without modifications.  For instance,
* an IS_CORPSE() macro was introduced in pl13.  Any future code add-ons
* could take into account the CircleMUD version and supply their own
* definition for the macro if used on an older version of CircleMUD.
* You are supposed to compare this with the macro CIRCLEMUD_VERSION()
* in utils.h.  See there for usage.
*/
#define _CIRCLEMUD      0x03000D /* Major/Minor/Patchlevel - MMmmPP */

/*
* If you want equipment to be automatically equipped to the same place
* it was when players rented, set the define below to 1.  Please note
* that this will require erasing or converting all of your rent files.
* And of course, you have to recompile everything.  We need this feature
* for CircleMUD 3.0 to be complete but we refuse to break binary file
* compatibility.
*/
#define USE_AUTOEQ	1	/* TRUE/FALSE aren't defined yet. */

/* preamble *************************************************************/

#define NOWHERE    -1    /* nil reference for room-database     */
#define NOTHING    -1    /* nil reference for objects           */
#define NOBODY     -1    /* nil reference for mobiles           */

#define SPECIAL(name) \
   int (name)(struct char_data *ch, void *me, int cmd, char *argument)


/* house drop */
#define MAX_DROP_HOUSE		40

/* pk level */
#define PK_MIN_LEVEL		10

/* misc editor defines **************************************************/

/* format modes for format_text */
#define FORMAT_INDENT           (1 << 0)
 

/* room-related defines *************************************************/


/* The cardinal directions: used as index to room_data.dir_option[] */
#define NORTH          0
#define EAST           1
#define SOUTH          2
#define WEST           3
#define UP             4
#define DOWN           5


/* Room flags: used in room_data.room_flags */
/* WARNING: In the world files, NEVER set the bits marked "R" ("Reserved") 
*/
#define ROOM_DARK               (1 << 0)   /* Dark                      */
#define ROOM_DEATH              (1 << 1)   /* Death trap                */
#define ROOM_NOMOB              (1 << 2)   /* MOBs not allowed          */
#define ROOM_INDOORS            (1 << 3)   /* Indoors                   */
#define ROOM_PEACEFUL           (1 << 4)   /* Violence not allowed      */
#define ROOM_SOUNDPROOF         (1 << 5)   /* Shouts, gossip blocked    */
#define ROOM_NOTRACK            (1 << 6)   /* Track won't go through    */
#define ROOM_NOMAGIC            (1 << 7)   /* Magic not allowed         */
#define ROOM_TUNNEL             (1 << 8)   /* room for only 1 pers      */
#define ROOM_PRIVATE            (1 << 9)   /* Can't teleport in         */
#define ROOM_GODROOM            (1 << 10)  /* LVL_GOD+ only allowed     */
#define ROOM_HOUSE              (1 << 11)  /* (R) Room is a house       */
#define ROOM_HOUSE_CRASH        (1 << 12)  /* (R) House needs saving    */
#define ROOM_ATRIUM             (1 << 13)  /* (R) The door to a house   */
#define ROOM_OLC                (1 << 14)  /* (R) Modifyable/!compress  */
#define ROOM_BFS_MARK           (1 << 15)  /* (R) breath-first srch mrk */
#define ROOM_ARENA              (1 << 16)
#define ROOM_CHALLENGE          (1 << 17)
#define ROOM_LEARN              (1 << 18)
#define ROOM_TELEPORT           (1 << 19)
#define ROOM_GOOD_REGEN		(1 << 20)  /* Good Regen Room */
#define ROOM_NO_REGEN_MANA      (1 << 21)  /* !Regen Mana Room */
#define ROOM_NO_REGEN_HIT       (1 << 22)  /* !Regen Hit Room */
#define ROOM_DONATION   	(1 << 23)
#define ROOM_PATH 		(1 << 24) //mostra path

/* Zone info: Used in zone_data.zone_flags */
#define ZONE_OPEN              (1 << 0)
#define ZONE_CLOSED            (1 << 1)
#define ZONE_NORECALL          (1 << 2)
#define ZONE_NOSUMMON          (1 << 3)
#define ZONE_REMORT_ONLY       (1 << 4)


/* Exit info: used in room_data.dir_option.exit_info */
#define EX_ISDOOR               (1 << 0)   /* Exit is a door            */
#define EX_CLOSED               (1 << 1)   /* The door is closed        */
#define EX_LOCKED               (1 << 2)   /* The door is locked        */
#define EX_PICKPROOF            (1 << 3)   /* Lock can't be picked      */
//#define EX_HIDDEN		(1 << 4)   /* portas escondidas 	*/

/* Sector types: used in room_data.sector_type */
#define SECT_INSIDE          0             /* Indoors                   */
#define SECT_CITY            1             /* In a city                 */
#define SECT_FIELD           2             /* In a field                */
#define SECT_FOREST          3             /* In a forest               */
#define SECT_HILLS           4             /* In the hills              */
#define SECT_MOUNTAIN        5             /* On a mountain             */
#define SECT_WATER_SWIM      6             /* Swimmable water           */
#define SECT_WATER_NOSWIM    7             /* Water - need a boat       */
#define SECT_UNDERWATER      8             /* Underwater                */
#define SECT_FLYING          9             /* Wheee!                    */


/* char and mob-related defines *****************************************/


#define MAX_REMORT 	  20

/* PC classes */
#define CLASS_UNDEFINED   -1
#define CLASS_MAGIC_USER   0
#define CLASS_CLERIC       1
#define CLASS_THIEF        2
#define CLASS_WARRIOR      3
#define CLASS_NECROMANCER  4
#define CLASS_PALADIN      5
#define CLASS_NINJA        6
#define CLASS_RANGER       7
#define CLASS_PSIONICIST   8
#define CLASS_WARLOCK      9
#define CLASS_BARBARIAN   10
#define CLASS_SORCERER    11

#define NUM_CLASSES       12  /* This must be the number of classes!! */

/* PC races */
#define RACE_UNDEFINED   -1
#define RACE_VAMPIRE      0
#define RACE_DROW	  1
#define RACE_DWARF	  2
#define RACE_ELF	  3
#define RACE_OGRE	  4
#define RACE_ORC	  5
#define RACE_TROLL	  6
#define RACE_GITH	  7
#define RACE_GNOME	  8
#define RACE_LIZARDMAN	  9
#define RACE_SEA_ELF	 10
#define RACE_GORAK	 11
#define RACE_DUNEDAIN    12
#define RACE_ANCI_DROW   13
#define RACE_NAUGRIM     14
#define RACE_HIGH_ELF    15
#define RACE_HILL_OGRE   16
#define RACE_BUGBEAR     17
#define RACE_CAVE_TROLL  18
#define RACE_LICH        19
#define RACE_TINKER      20
#define RACE_DRACONIAN   21
#define RACE_H_SEA_ELF   22
#define RACE_ARCHONS     23

#define NUM_RACES        24

/* NPC classes */
#define CLASS_OTHER       0
#define CLASS_UNDEAD      1
#define CLASS_DRAGON      2
#define CLASS_DEMON       3
#define CLASS_SPIRIT      4
#define CLASS_ANIMAL      5
#define CLASS_HUMAN       6
#define CLASS_GIANT       7

#define NUM_NPC_CLASSES   8  /* This must be the numbrer of classes!! */

/* Sex */
#define SEX_NEUTRAL   0
#define SEX_MALE      1
#define SEX_FEMALE    2


/* Positions */
#define POS_DEAD       0        /* dead                 */
#define POS_MORTALLYW  1        /* mortally wounded     */
#define POS_INCAP      2        /* incapacitated        */
#define POS_STUNNED    3        /* stunned              */
#define POS_SLEEPING   4        /* sleeping             */
#define POS_RESTING    5        /* resting              */
#define POS_SITTING    6        /* sitting              */
#define POS_FIGHTING   7        /* fighting             */
#define POS_STANDING   8        /* standing             */

/* Player flags: used by char_data.char_specials.act */
#define PLR_KILLER      (1 << 0)   /* Player is a player-killer         */
#define PLR_THIEF       (1 << 1)   /* Player is a player-thief          */
#define PLR_FROZEN      (1 << 2)   /* Player is frozen                  */
#define PLR_DONTSET     (1 << 3)   /* Don't EVER set (ISNPC bit)        */
#define PLR_WRITING     (1 << 4)   /* Player writing (board/mail/olc)   */
#define PLR_MAILING     (1 << 5)   /* Player is writing mail            */
#define PLR_CRASH       (1 << 6)   /* Player needs to be crash-saved    */
#define PLR_SITEOK      (1 << 7)   /* Player has been site-cleared      */
#define PLR_NOSHOUT     (1 << 8)   /* Player not allowed to shout/goss  */
#define PLR_NOTITLE     (1 << 9)   /* Player not allowed to set title   */
#define PLR_DELETED     (1 << 10)  /* Player deleted - space reusable   */
#define PLR_LOADROOM    (1 << 11)  /* Player uses nonstandard loadroom  */
#define PLR_NOWIZLIST   (1 << 12)  /* Player shouldn't be on wizlist    */
#define PLR_NODELETE    (1 << 13)  /* Player shouldn't be deleted       */
#define PLR_INVSTART    (1 << 14)  /* Player should enter game wizinvis */
#define PLR_CRYO        (1 << 15)  /* Player is cryo-saved (purge prog) */
#define PLR_DEAD	(1 << 16)
#define PLR_NOPK	(1 << 17)
#define PLR_MEDITATE	(1 << 18)
#define PLR_MESSAGING   (1 << 19)  /* Player is writing the message of day  
*/
#define PLR_BANNED      (1 << 20)
#define PLR_MSP         (1 << 21) //toca som
#define PLR_SECPLAYER  (1 << 22) //O player eh um second player

/* Mobile flags: used by char_data.char_specials.act */
#define MOB_SPEC         (1 << 0)  /* Mob has a callable spec-proc      */
#define MOB_SENTINEL     (1 << 1)  /* Mob should not move               */
#define MOB_SCAVENGER    (1 << 2)  /* Mob picks up stuff on the ground  */
#define MOB_ISNPC        (1 << 3)  /* (R) Automatically set on all Mobs */
#define MOB_AWARE        (1 << 4)  /* Mob can't be backstabbed          */
#define MOB_AGGRESSIVE   (1 << 5)  /* Mob hits players in the room      */
#define MOB_STAY_ZONE    (1 << 6)  /* Mob shouldn't wander out of zone  */
#define MOB_WIMPY        (1 << 7)  /* Mob flees if severely injured     */
#define MOB_AGGR_EVIL    (1 << 8)  /* auto attack evil PC's             */
#define MOB_AGGR_GOOD    (1 << 9)  /* auto attack good PC's             */
#define MOB_AGGR_NEUTRAL (1 << 10) /* auto attack neutral PC's          */
#define MOB_MEMORY       (1 << 11) /* remember attackers if attacked    */
#define MOB_HELPER       (1 << 12) /* attack PCs fighting other NPCs    */
#define MOB_NOCHARM      (1 << 13) /* Mob can't be charmed              */
#define MOB_NOSUMMON     (1 << 14) /* Mob can't be summoned             */
#define MOB_NOSLEEP      (1 << 15) /* Mob can't be slept                */
#define MOB_NOBASH       (1 << 16) /* Mob can't be bashed (e.g. trees)  */
#define MOB_NOBLIND      (1 << 17) /* Mob can't be blinded              */
#define MOB_NOENTANGLE   (1 << 18) /* Mob can't be entangled            */
#define MOB_CLAN	 (1 << 19)
#define MOB_NOSTEAL	 (1 << 20)
#define MOB_NODISARM	 (1 << 21)
#define MOB_FIREMOB      (1 << 22)
#define MOB_WATERMOB     (1 << 23)


/* Preference flags: used by char_data.player_specials.pref */
#define PRF_BRIEF       (1 << 0)  /* Room descs won't normally be shown */
#define PRF_COMPACT     (1 << 1)  /* No extra CRLF pair before prompts  */
#define PRF_DEAF        (1 << 2)  /* Can't hear shouts                  */
#define PRF_NOTELL      (1 << 3)  /* Can't receive tells                */
#define PRF_DISPHP      (1 << 4)  /* Display hit points in prompt       */
#define PRF_DISPMANA    (1 << 5)  /* Display mana points in prompt      */
#define PRF_DISPMOVE    (1 << 6)  /* Display move points in prompt      */
#define PRF_AUTOEXIT    (1 << 7)  /* Display exits in a room            */
#define PRF_NOHASSLE    (1 << 8)  /* Aggr mobs won't attack             */
#define PRF_QUEST       (1 << 9)  /* On quest                           */
#define PRF_SUMMONABLE  (1 << 10) /* Can be summoned                    */
#define PRF_NOREPEAT    (1 << 11) /* No repetition of comm commands     */
#define PRF_HOLYLIGHT   (1 << 12) /* Can see in dark                    */
#define PRF_COLOR_1     (1 << 13) /* Color (low bit)                    */
#define PRF_COLOR_2     (1 << 14) /* Color (high bit)                   */
#define PRF_NOWIZ       (1 << 15) /* Can't hear wizline                 */
#define PRF_LOG1        (1 << 16) /* On-line System Log (low bit)       */
#define PRF_LOG2        (1 << 17) /* On-line System Log (high bit)      */
#define PRF_NOAUCT      (1 << 18) /* Can't hear auction channel         */
#define PRF_NOGOSS      (1 << 19) /* Can't hear gossip channel          */
#define PRF_NOGRATZ     (1 << 20) /* Can't hear grats channel           */
#define PRF_DISPMAX     (1 << 21) /* Display maxhit, maxmana and maxmove*/
#define PRF_CROSSDOORS  (1 << 22) /* Cross doors without opening them   */
#define PRF_AUTOLOOT    (1 << 23) /* Auto get all from corpses          */
#define PRF_AUTOGOLD    (1 << 24) /* Auto get gold from corpses         */
#define PRF_AUTOSPLIT   (1 << 25) /* Auto split gold between group      */
#define PRF_AFK         (1 << 26) /* Away from keyboard                 */
#define PRF_AUTOASSIST  (1 << 27) /* autoassist 		        */
#define PRF_INFO_START  (1 << 28) /* to type info when the player enter */
#define PRF_DISPMENT    (1 << 29) /* Display mental points in prompt    */
#define PRF_NOCHAT      (1 << 30) /* Can't hear chat channel            */

/* Preference flags: used by char_data.player_specials.pref2 */
#define PRF2_DISPEXP       (1 << 0)
#define PRF2_WDPROTOCOL    (1 << 1)
#define PRF2_MAP           (1 << 2)
#define PRF2_NOREPEAT	   (1 << 3)
#define PRF2_HOUSE_RECALL  (1 << 4) /* Recall to the house instead of temple 
- Fenix - WHS */
#define PRF2_CLAN_SEMFALAR (1 << 5)
#define PRF2_NOTIPS        (1 << 6)
#define PRF2_NONEWBIE      (1 << 7) /* cant hear newbie channel */
#define PRF2_NOPATHS	   (1 << 8)
#define PRF2_NOHOLLER         (1 << 9) /* cant hear OOC channel */

/* Affect bits: used in char_data.char_specials.saved.affected_by */
/* WARNING: In the world files, NEVER set the bits marked "R" ("Reserved") 
*/
#define AFF_BLIND             (1 << 0)     /* (R) Char is blind         */
#define AFF_INVISIBLE         (1 << 1)     /* Char is invisible         */
#define AFF_DETECT_ALIGN      (1 << 2)     /* Char is sensitive to align*/
#define AFF_DETECT_INVIS      (1 << 3)     /* Char can see invis chars  */
#define AFF_DETECT_MAGIC      (1 << 4)     /* Char is sensitive to magic*/
#define AFF_SENSE_LIFE        (1 << 5)     /* Char can sense hidden life*/
#define AFF_WATERWALK         (1 << 6)     /* Char can walk on water    */
#define AFF_SANCTUARY         (1 << 7)     /* Char protected by sanct.  */
#define AFF_GROUP             (1 << 8)     /* (R) Char is grouped       */
#define AFF_CURSE             (1 << 9)     /* Char is cursed            */
#define AFF_INFRAVISION       (1 << 10)    /* Char can see in dark      */
#define AFF_POISON            (1 << 11)    /* (R) Char is poisoned      */
#define AFF_PROTECT_EVIL      (1 << 12)    /* Char protected from evil  */
#define AFF_PROTECT_GOOD      (1 << 13)    /* Char protected from good  */
#define AFF_SLEEP             (1 << 14)    /* (R) Char magically asleep */
#define AFF_NOTRACK           (1 << 15)    /* Char can't be tracked     */
#define AFF_MANA_SHIELD       (1 << 16)    /* Char lose mana instead hp */
#define AFF_DAMNED_CURSE      (1 << 17)    /* Char is extremely cursed  */
#define AFF_SNEAK             (1 << 18)    /* Char can move quietly     */
#define AFF_HIDE              (1 << 19)    /* Char is hidden            */
#define AFF_HASTE             (1 << 20)    /* One more attack           */
#define AFF_CHARM             (1 << 21)    /* Char is charmed           */
#define AFF_HOLDED	      (1 << 22)    /* char is holded		*/
#define AFF_SATAN	      (1 << 23)    /* char is protected by satan*/
#define AFF_GOD		      (1 << 24)    /* char is protected by god  */
#define AFF_REGEN	      (1 << 25)
#define AFF_FLY		      (1 << 26)
#define AFF_GAS               (1 << 27)
#define AFF_TANGLED           (1 << 28)    /* (R) Char is tangled       */
#define AFF_FIRESHIELD        (1 << 29)
#define AFF_BERZERK           (1 << 30)
/* limite de campo estourado. */

/* Affect bits: used in char_data.char_specials.saved.affected_by */
/* WARNING: In the world files, NEVER set the bits marked "R" ("Reserved") 
*/
#define AFF2_TRANSFORM		(1 << 0)   /* transformation flag */
#define AFF2_WARSHOUT		(1 << 1)   /* warshout flag */
#define AFF2_TERROR             (1 << 2)   /* Terror Flag */
#define AFF2_BANSHEE            (1 << 3)   /* Banshe Aura */
#define AFF2_BLINK              (1 << 4)   /* Blink */
#define AFF2_DEATHDANCE         (1 << 5)   /* deathdance */
#define AFF2_PROT_FIRE          (1 << 6)   /* protection from fire*/
#define AFF2_PASSDOOR           (1 << 7)   /* protection from fire*/
#define AFF2_MORALE             (1 << 8)   /* morale */
#define AFF2_SLOW		(1 << 9) /* slow */
#define AFF2_MUIR1              (1 << 10) //religiao
#define AFF2_TEMPUS             (1 << 11)
#define AFF2_ILMANATEUR1        (1 << 12)
#define AFF2_SELUNE             (1 << 13)
#define AFF2_ROOTED				(1 << 14)
#define AFF2_EXTEND_SPELLS		(1 << 15)
#define AFF2_ASPHYXIATE			(1 << 16)
#define AFF2_IRON_BODY			(1 << 17)

/* Affect bits: used in char_data.char_specials.saved.affected_by */
/* WARNING: In the world files, NEVER set the bits marked "R" ("Reserved") 
*/
#define AFF3_UNUSED		(1 << 0)

/* Room affections */
#define RAFF_FOG			  (1 << 0)
#define RAFF_FIREWALL	      (1 << 1)
#define RAFF_ICEWALL	      (1 << 2)
#define RAFF_LIQUID_AIR		  (1 << 3)
#define RAFF_SILENCE		  (1 << 4)
#define RAFF_FSANCTUARY		  (1 << 5)
#define RAFF_ILLUSIONF		  (1 << 6)

/* Modes of connectedness: used by descriptor_data.state */
#define CON_PLAYING      0              /* Playing - Nominal state      */
#define CON_CLOSE        1              /* Disconnecting                */
#define CON_GET_NAME     2              /* By what name ..?             */
#define CON_NAME_CNFRM   3              /* Did I get that right, x?     */
#define CON_PASSWORD     4              /* Password:                    */
#define CON_NEWPASSWD    5              /* Give me a password for x     */
#define CON_CNFPASSWD    6              /* Please retype password:      */
#define CON_QSEX         7              /* Sex?                         */
#define CON_QCLASS       8              /* Class?                       */
#define CON_RMOTD        9              /* PRESS RETURN after MOTD      */
#define CON_MENU         10             /* Your choice: (main menu)     */
#define CON_EXDESC       11             /* Enter a new description:     */
#define CON_CHPWD_GETOLD 12             /* Changing passwd: get old     */
#define CON_CHPWD_GETNEW 13             /* Changing passwd: get new     */
#define CON_CHPWD_VRFY   14             /* Verify new password          */
#define CON_DELCNF1      15             /* Delete confirmation 1        */
#define CON_DELCNF2      16             /* Delete confirmation 2        */
#define CON_DISCONNECT   17             /* In-game disconnection        */
#define CON_OEDIT        18             /*. OLC mode - object edit      */
#define CON_REDIT        19             /*. OLC mode - room edit        */
#define CON_ZEDIT        20             /*. OLC mode - zone info edit   */
#define CON_MEDIT        21             /*. OLC mode - mobile edit      */
#define CON_SEDIT        22             /*. OLC mode - shop edit        */
#define CON_HEDIT	 23		/*. OLC mode - help edit       .*/
#define CON_QRACE        24             /* Race?                        */
#define CON_WHOISDESC    25             /* WHOISDESC                    */
#define CON_TERMINAL	 26
#define CON_TEXTED	 27
#define CON_AEDIT	 28		/*. OLC mode - action edit     .*/
#define CON_SUMMARY      29
#define CON_TRIGEDIT     30		/*. OLC mode - trigger edit    .*/
#define CON_QEDIT	 31		/*. OLC mode - quest edit      . */
#define CON_CHOOSE_PK	 32		/*. OLC mode - quest edit      . */
#define CON_QCONFIRMRACE 33             /*  Comfirm Race? */
#define CON_QCONFIRMCLAS 34             /*  Comfirm Race? */
#define CON_MASTER       35 		/*  Master System. Taerom */
#define CON_NEWMASTER    36		/*  Master System. Taerom */
#define CON_MULTIPLAY	 37		//  Is the player a second player?
#define CON_SECOND       38
#define CON_LANGUAGE     39		// Language selection -ips

/* Character equipment positions: used as index for char_data.equipment[] */
/* NOTE: Don't confuse these constants with the ITEM_ bitvectors
   which control the valid places you can wear a piece of equipment */
#define WEAR_LIGHT      0
#define WEAR_FINGER_R   1
#define WEAR_FINGER_L   2
#define WEAR_NECK_1     3
#define WEAR_NECK_2     4
#define WEAR_BODY       5
#define WEAR_HEAD       6
#define WEAR_LEGS       7
#define WEAR_FEET       8
#define WEAR_HANDS      9
#define WEAR_ARMS      10
#define WEAR_SHIELD    11
#define WEAR_ABOUT     12
#define WEAR_WAIST     13
#define WEAR_WRIST_R   14
#define WEAR_WRIST_L   15
#define WEAR_WIELD     16
#define WEAR_HOLD      17
#define WEAR_DWIELD    18
#define WEAR_EAR_R     19
#define WEAR_EAR_L     20
#define WEAR_FACE      21
#define WEAR_FLOAT     22

#define NUM_WEARS      23       /* This must be the # of eq positions!! */


/* object-related defines ********************************************/


/* Item types: used by obj_data.obj_flags.type_flag */
#define ITEM_LIGHT      1               /* Item is a light source       */
#define ITEM_SCROLL     2               /* Item is a scroll             */
#define ITEM_WAND       3               /* Item is a wand               */
#define ITEM_STAFF      4               /* Item is a staff              */
#define ITEM_WEAPON     5               /* Item is a weapon             */
#define ITEM_FIREWEAPON 6               /* Unimplemented                */
#define ITEM_MISSILE    7               /* Unimplemented                */
#define ITEM_TREASURE   8               /* Item is a treasure, not gold */
#define ITEM_ARMOR      9               /* Item is armor                */
#define ITEM_POTION    10               /* Item is a potion             */
#define ITEM_WORN      11               /* Unimplemented                */
#define ITEM_OTHER     12               /* Misc object                  */
#define ITEM_TRASH     13               /* Trash - shopkeeps won't buy  */
#define ITEM_TRAP      14               /* Unimplemented                */
#define ITEM_CONTAINER 15               /* Item is a container          */
#define ITEM_NOTE      16               /* Item is note                 */
#define ITEM_DRINKCON  17               /* Item is a drink container    */
#define ITEM_KEY       18               /* Item is a key                */
#define ITEM_FOOD      19               /* Item is food                 */
#define ITEM_MONEY     20               /* Item is money (gold)         */
#define ITEM_PEN       21               /* Item is a pen                */
#define ITEM_BOAT      22               /* Item is a boat               */
#define ITEM_FOUNTAIN  23               /* Item is a fountain           */
#define ITEM_SPELLBOOK 24               // Livro de spell
#define ITEM_MINERALS  25               // Clan EQ - Mineral
#define ITEM_ERB       26		// raizes

/* Take/Wear flags: used by obj_data.obj_flags.wear_flags */
#define ITEM_WEAR_TAKE          (1 << 0)  /* Item can be takes          */
#define ITEM_WEAR_FINGER        (1 << 1)  /* Can be worn on finger      */
#define ITEM_WEAR_NECK          (1 << 2)  /* Can be worn around neck    */
#define ITEM_WEAR_BODY          (1 << 3)  /* Can be worn on body        */
#define ITEM_WEAR_HEAD          (1 << 4)  /* Can be worn on head        */
#define ITEM_WEAR_LEGS          (1 << 5)  /* Can be worn on legs        */
#define ITEM_WEAR_FEET          (1 << 6)  /* Can be worn on feet        */
#define ITEM_WEAR_HANDS         (1 << 7)  /* Can be worn on hands       */
#define ITEM_WEAR_ARMS          (1 << 8)  /* Can be worn on arms        */
#define ITEM_WEAR_SHIELD        (1 << 9)  /* Can be used as a shield    */
#define ITEM_WEAR_ABOUT         (1 << 10) /* Can be worn about body     */
#define ITEM_WEAR_WAIST         (1 << 11) /* Can be worn around waist   */
#define ITEM_WEAR_WRIST         (1 << 12) /* Can be worn on wrist       */
#define ITEM_WEAR_WIELD         (1 << 13) /* Can be wielded             */
#define ITEM_WEAR_HOLD          (1 << 14) /* Can be held                */
#define ITEM_WEAR_EAR           (1 << 15)
#define ITEM_WEAR_FACE          (1 << 16)
#define ITEM_WEAR_FLOAT         (1 << 17)

/* Extra object flags: used by obj_data.obj_flags.extra_flags */
#define ITEM_GLOW             (1 << 0)     /* Item is glowing              
*/
#define ITEM_HUM              (1 << 1)     /* Item is humming              
*/
#define ITEM_NORENT           (1 << 2)     /* Item cannot be rented        
*/
#define ITEM_NODONATE         (1 << 3)     /* Item cannot be donated       
*/
#define ITEM_NOINVIS          (1 << 4)     /* Item cannot be made invis    
*/
#define ITEM_INVISIBLE        (1 << 5)     /* Item is invisible            
*/
#define ITEM_MAGIC            (1 << 6)     /* Item is magical              
*/
#define ITEM_NODROP           (1 << 7)     /* Item is cursed: can't drop   
*/
#define ITEM_BLESS            (1 << 8)     /* Item is blessed              
*/
#define ITEM_ANTI_GOOD        (1 << 9)     /* Not usable by good people    
*/
#define ITEM_ANTI_EVIL        (1 << 10)    /* Not usable by evil people    
*/
#define ITEM_ANTI_NEUTRAL     (1 << 11)    /* Not usable by neutral people 
*/
#define ITEM_ANTI_SORCERER    (1 << 12)    /* Not usable by mages          
*/
#define ITEM_ANTI_CLERIC      (1 << 13)    /* Not usable by clerics        
*/
#define ITEM_ANTI_THIEF       (1 << 14)    /* Not usable by thieves        
*/
#define ITEM_ANTI_WARRIOR     (1 << 15)    /* Not usable by warriors       
*/
#define ITEM_NOSELL           (1 << 16)    /* Shopkeepers won't touch it   
*/
#define ITEM_ANTI_NECROMANCER (1 << 17)    /* Not usable by necromancers   
*/
#define ITEM_ANTI_PALADIN     (1 << 18)    /* Not usable by paladins       
*/
#define ITEM_ANTI_NINJA       (1 << 19)    /* Not usable by ninja          
*/
#define ITEM_ANTI_RANGER      (1 << 20)    /* Not usable by ranger         
*/
#define ITEM_ANTI_PSIONICIST  (1 << 21)    /* Not usable by psionicist     
*/
#define ITEM_ANTI_WARLOCK     (1 << 22)    /* Not usable by warlock        
*/
#define ITEM_ANTI_SHAMAN      (1 << 23)    /* Not usable by sorcerer       
*/
#define ITEM_ANTI_BARBARIAN   (1 << 24)    /* Not usable by barbarian      
*/
#define ITEM_ANTI_VAMPIRE     (1 << 25)
#define ITEM_ANTI_DROW        (1 << 26)
#define ITEM_WIND	      (1 << 27)
#define ITEM_EARTH            (1 << 28)
#define ITEM_WATER	      (1 << 29)
#define ITEM_FIRE	      (1 << 30)
#define ITEM_ANTI_TROLL       (1 << 31)
#define ITEM_ANTI_GITH        (1 << 32)
#define ITEM_ANTI_GNOME       (1 << 33)
#define ITEM_ANTI_LIZARDMAN   (1 << 34)
#define ITEM_ANTI_SEA_ELF     (1 << 35)
#define ITEM_ANTI_ORC         (1 << 36)
#define ITEM_ANTI_GORAK       (1 << 37)
#define ITEM_ANTI_MAGIC_USER  (1 << 38)
#define ITEM_QUEST             (1 << 39)//adicionado por ultimo


/* Modifier constants used with obj affects ('A' fields) */
#define APPLY_NONE              0       /* No effect                    */
#define APPLY_STR               1       /* Apply to strength            */
#define APPLY_DEX               2       /* Apply to dexterity           */
#define APPLY_INT               3       /* Apply to constitution        */
#define APPLY_WIS               4       /* Apply to wisdom              */
#define APPLY_CON               5       /* Apply to constitution        */
#define APPLY_CHA               6       /* Apply to charisma            */
#define APPLY_CLASS             7       /* Reserved                     */
#define APPLY_LEVEL             8       /* Reserved                     */
#define APPLY_AGE               9       /* Apply to age                 */
#define APPLY_CHAR_WEIGHT      10       /* Apply to weight              */
#define APPLY_CHAR_HEIGHT      11       /* Apply to height              */
#define APPLY_MANA             12       /* Apply to max mana            */
#define APPLY_HIT              13       /* Apply to max hit points      */
#define APPLY_MOVE             14       /* Apply to max move points     */
#define APPLY_GOLD             15       /* Reserved                     */
#define APPLY_EXP              16       /* Reserved                     */
#define APPLY_AC               17       /* Apply to Armor Class         */
#define APPLY_HITROLL          18       /* Apply to hitroll             */
#define APPLY_DAMROLL          19       /* Apply to damage roll         */
#define APPLY_SAVING_PARA      20       /* Apply to save throw: paralz  */
#define APPLY_SAVING_ROD       21       /* Apply to save throw: rods    */
#define APPLY_SAVING_PETRI     22       /* Apply to save throw: petrif  */
#define APPLY_SAVING_BREATH    23       /* Apply to save throw: breath  */
#define APPLY_SAVING_SPELL     24       /* Apply to save throw: spells  */


/* Container flags - value[1] */
#define CONT_CLOSEABLE      (1 << 0)    /* Container can be closed      */
#define CONT_PICKPROOF      (1 << 1)    /* Container is pickproof       */
#define CONT_CLOSED         (1 << 2)    /* Container is closed          */
#define CONT_LOCKED         (1 << 3)    /* Container is locked          */


/* Some different kind of liquids for use in values of drink containers */
#define LIQ_WATER      0
#define LIQ_BEER       1
#define LIQ_WINE       2
#define LIQ_ALE        3
#define LIQ_DARKALE    4
#define LIQ_WHISKY     5
#define LIQ_LEMONADE   6
#define LIQ_FIREBRT    7
#define LIQ_LOCALSPC   8
#define LIQ_SLIME      9
#define LIQ_MILK       10
#define LIQ_TEA        11
#define LIQ_COFFE      12
#define LIQ_BLOOD      13
#define LIQ_SALTWATER  14
#define LIQ_CLEARWATER 15


/* other miscellaneous defines *******************************************/


/* Player conditions */
#define DRUNK        0
#define FULL         1
#define THIRST       2


/* Sun state for weather_data */
#define SUN_DARK        0
#define SUN_RISE        1
#define SUN_LIGHT       2
#define SUN_SET         3


/* Sky conditions for weather_data */
#define SKY_CLOUDLESS   0
#define SKY_CLOUDY      1
#define SKY_RAINING     2
#define SKY_LIGHTNING   3


/* Rent codes */
#define RENT_UNDEF      0
#define RENT_CRASH      1
#define RENT_RENTED     2
#define RENT_CRYO       3
#define RENT_FORCED     4
#define RENT_TIMEDOUT   5


/* other #defined constants **********************************************/

/*
* **DO**NOT** blindly change the number of levels in your MUD merely by
* changing these numbers and without changing the rest of the code to match.
* Other changes throughout the code are required.  See coding.doc for
* details.
*
* LVL_IMPL should always be the HIGHEST possible immortal level, and
* LVL_IMMORT should always be the LOWEST immortal level.  The number of
* mortal levels will always be LVL_IMMORT - 1.
*/
#define LVL_IMPL        210
#define LVL_SUBIMPL     209
#define LVL_MJGOD	208 // Supreme Coder
#define LVL_SUPGOD      207 // Major Builder
#define LVL_GRGOD       206 // Coder
#define LVL_GOD         205 // Builder
#define LVL_DEMIGOD	204 // Helper
#define LVL_LORD	203
#define LVL_ELDER       202
#define LVL_IMMORT      201

/* Level of the 'freeze' command */
#define LVL_FREEZE      LVL_GRGOD

/* Level of the 'remort' command */
#define LVL_GOREMORT    LVL_IMMORT

/* Level of some commands */
#define LVL_ROOMFLAGS   LVL_ELDER
#define LVL_NEWBIE      10
#define LVL_NEWBIE_WARN 8

#define NUM_OF_DIRS     6       /* number of directions in a room (nsewud) 
*/
#define MAGIC_NUMBER    (0x06)  /* Arbitrary number that won't be in a 
string */

#define OPT_USEC        100000  /* 10 passes per second */
#define PASSES_PER_SEC  (1000000 / OPT_USEC)
#define RL_SEC          * PASSES_PER_SEC

#define PULSE_ZONE      (10 RL_SEC)
#define PULSE_MOBILE    (10 RL_SEC)
#define PULSE_VIOLENCE  (2 RL_SEC)

/* Variables for the output buffering system */
#define MAX_SOCK_BUF            (12 * 1024) /* Size of kernel's sock buf   
*/
#define MAX_PROMPT_LENGTH       340         /* Max length of prompt        
*/
#define GARBAGE_SPACE           32          /* Space for **OVERFLOW** etc  
*/
#define SMALL_BUFSIZE           1024        /* Static output buffer size   
*/
/* Max amount of output that can be buffered */
#define LARGE_BUFSIZE      (MAX_SOCK_BUF - GARBAGE_SPACE - MAX_PROMPT_LENGTH)

/*
* --- WARNING ---
* If you are using a BSD-derived UNIX with MD5 passwords, you _must_
* make MAX_PWD_LENGTH larger.  A length of 20 should be good. If
* you leave it at the default value of 10, then any character with
* a name longer than about 5 characters will be able to log in with
* _any_ password.  This has not (yet) been changed to ensure pfile
* compatibility for those unaffected.
*/
#define HISTORY_SIZE            5   /* Keep last 5 commands. */
#define MAX_STRING_LENGTH   65536
#define MAX_INPUT_LENGTH      256   /* Max length per *line* of input */
#define MAX_RAW_INPUT_LENGTH  512   /* Max size of *raw* input */
#define MAX_MESSAGES           60
#define MAX_NAME_LENGTH        12   /* Used in char_file_u *DO*NOT*CHANGE* 
*/
/* ** MAX_PWD_LENGTH changed from 10 to 30 for ascii test - Sam ** */
#define MAX_PWD_LENGTH	       30  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_TITLE_LENGTH       30   /* Used in char_file_u *DO*NOT*CHANGE* 
*/
#define HOST_LENGTH            30   /* Used in char_file_u *DO*NOT*CHANGE* 
*/
#define EXDSCR_LENGTH        3072   /* Used in char_file_u *DO*NOT*CHANGE* 
*/
#define MAX_TONGUE              3   /* Used in char_file_u *DO*NOT*CHANGE* 
*/
#define MAX_SKILLS            500   /* Used in char_file_u *DO*NOT*CHANGE* 
*/
#define MAX_AFFECT             31  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_OBJ_AFFECT          6   /* Used in obj_file_elem *DO*NOT*CHANGE* 
*/
#define MAX_RSKILLS             5   /* Used in char_file_u *DO*NOT*CHANGE* 
*/

/**********************************************************************
* Structures                                                          *
**********************************************************************/


typedef signed char             sbyte;
typedef unsigned char           ubyte;
typedef signed short int        sh_int;
typedef unsigned short int      ush_int;
typedef signed long		sh_long;
typedef unsigned long		ush_long;

#if !defined(__cplusplus)       /* Anyone know a portable method? */
typedef char                    bool;
#endif

#ifndef CIRCLE_WINDOWS
typedef char                    byte;
#endif

typedef sh_int  room_vnum;      /* A room's vnum type */
typedef sh_int  obj_vnum;       /* An object's vnum type */
typedef sh_int  mob_vnum;       /* A mob's vnum type */

typedef sh_int  room_rnum;      /* A room's real (internal) number type */
typedef sh_int  obj_rnum;       /* An object's real (internal) num type */
typedef sh_int  mob_rnum;       /* A mobile's real (internal) num type */


/* Extra description: used in objects, mobiles, and rooms */
struct extra_descr_data {
   char *keyword;                 /* Keyword in look/examine          */
   char *description;             /* What to see                      */
   struct extra_descr_data *next; /* Next in list                     */
};


/* object-related structures ******************************************/


/* object flags; used in obj_data */
struct obj_flag_data {
   int  value[4];       /* Values of the item (see list)    */
   byte type_flag;      /* Type of item                     */
   int  wear_flags;     /* Where you can wear it            */
   long int extra_flags;    /* If it hums, glows, etc.          */
   int  weight;         /* Weigt what else                  */
   int  cost;           /* Value when sold (gp.)            */
   int  cost_per_day;   /* Cost to keep pr. real day        */
   int  timer;          /* Timer for object                 */
   int  obj_level;      /* Minumum Level for object         */
   long bitvector;      /* To set chars bits                */
   int  cond;           /* condiction of the obj            */
};


/* Used in obj_file_elem *DO*NOT*CHANGE* */
struct obj_affected_type {
   byte location;      /* Which ability to change (APPLY_XXX) */
   sbyte modifier;     /* How much it changes by              */
};


/* ================== Memory Structure for Objects ================== */
struct obj_data {
   obj_vnum item_number;        /* Where in data-base                   */
   room_rnum in_room;           /* In what room -1 when conta/carr      */

   struct obj_flag_data obj_flags;/* Object information               */
   struct obj_affected_type affected[MAX_OBJ_AFFECT];  /* affects */

   char *name;                    /* Title of object :get etc.        */
   char *description;             /* When in room                     */
   char *short_description;       /* when worn/carry/in cont.         */
   char *action_description;      /* What to write when used          */
   struct extra_descr_data *ex_description; /* extra descriptions     */
   struct char_data *carried_by;  /* Carried by :NULL in room/conta   */
   struct char_data *worn_by;     /* Worn by?                         */
   sh_int worn_on;                /* Worn where?                      */
   sh_int worned;                 /* Worned before renting            */

   struct obj_data *in_obj;       /* In what object NULL when none    */
   struct obj_data *contains;     /* Contains objects                 */

   long id;                       /* used by DG triggers              */
   struct trig_proto_list *proto_script; /* list of default triggers  */
   struct script_data *script;    /* script info for the object       */

   struct obj_data *next_content; /* For 'contains' lists             */
   struct obj_data *next;         /* For the object list              */
};
/* ======================================================================= 
*/


/* ====================== File Element for Objects ======================= 
*/
/*                 BEWARE: Changing it will ruin rent files                
*/
struct obj_file_elem {
   obj_vnum item_number;

#if USE_AUTOEQ
   sh_int location;
#endif
   int  value[4];
   long int extra_flags;
   int  weight;
   int  timer;
   sh_int worned;
   long bitvector;
   int cond;
   struct obj_affected_type affected[MAX_OBJ_AFFECT];

};


/* header block for rent files.  BEWARE: Changing it will ruin rent files  
*/
struct rent_info {
   int  time;
   int  rentcode;
   int  net_cost_per_diem;
   int  gold;
   int  account;
   int  nitems;
   int  spare0;
   int  spare1;
   int  spare2;
   int  spare3;
   int  spare4;
   int  spare5;
   int  spare6;
   int  spare7;
};
/* ======================================================================= 
*/


/* room-related structures ************************************************/


struct room_direction_data {
   char *general_description;       /* When look DIR.                   */

   char *keyword;               /* for open/close                       */

   sh_int exit_info;            /* Exit info                            */
   obj_vnum key;                /* Key's number (-1 for no key)         */
   room_rnum to_room;           /* Where direction leads (NOWHERE)      */
};


/* ================== Memory Structure for room ======================= */
struct room_data {
   room_vnum number;            /* Rooms number (vnum)                */
   sh_int zone;                 /* Room zone (for resetting)          */
   int  sector_type;            /* sector type (move/hide)            */
   char *name;                  /* Rooms name 'You are ...'           */
   char *description;           /* Shown when entered                 */
   struct extra_descr_data *ex_description; /* for examine/look       */
   struct room_direction_data *dir_option[NUM_OF_DIRS]; /* Directions */
   int room_flags;              /* DEATH,DARK ... etc                 */

   byte light;                  /* Number of lightsources in room     */
   SPECIAL(*func);

   struct trig_proto_list *proto_script; /* list of default triggers  */
   struct script_data *script;  /* script info for the object         */

   struct obj_data *contents;   /* List of items in room              */
   struct char_data *people;    /* List of NPC / PC in room           */

   struct teleport_data *tele;	/* teleport info */

   long  room_affections;    /* bitvector for spells/skills */
};
/* ====================================================================== */


/* char-related structures ************************************************/


/* memory structure for characters */
struct memory_rec_struct {
   long id;
   struct memory_rec_struct *next;
};

typedef struct memory_rec_struct memory_rec;


/* This structure is purely intended to be an easy way to transfer */
/* and return information about time (real or mudwise).            */
struct time_info_data {
   byte hours, day, month;
   sh_int year;
};


/* These data contain information about a players time data */
struct time_data {
   time_t birth;    /* This represents the characters age                */
   time_t logon;    /* Time of the last logon (used to calculate played) */
   int  played;     /* This is the total accumulated time played in secs */
};

/* The pclean_criteria_data is set up in config.c and used in db.c to
   determine the conditions which will cause a player character to be
   deleted from disk if the automagic pwipe system is enabled (see 
config.c).
*/
struct pclean_criteria_data {
  int level;		/* max level for this time limit	*/
  int days;		/* time limit in days			*/
};

/* general player-related info, usually PC's and NPC's */
struct char_player_data {
   char passwd[MAX_PWD_LENGTH+1]; /* character's password      */
   char *name;         /* PC / NPC s name (kill ...  )         */
   char *short_descr;  /* for NPC 'actions'                    */
   char *long_descr;   /* for 'look'                           */
   char *description;  /* Extra descriptions                   */
   char *whoisdesc;    /* Extra whoisdescriptions              */
   char *title;        /* PC / NPC's title                     */
   char *prename;        /* PC / NPC's title                     */
   int id_second;	//second player name
   int id_master;
      byte sex;           /* PC / NPC's sex                       */
   byte chclass;       /* PC / NPC's class                     */
   byte race;
   short int level;         /* PC / NPC's level                     */
   byte remort;
   short int  hometown;      /* PC s Hometown (zone)                 */
   struct time_data time;  /* PC's AGE in days                 */
   ubyte weight;       /* PC / NPC's weight                    */
   ubyte height;       /* PC / NPC's height                    */
   byte fbi_pos;
   int killed;
   int died;
   int a_killed;
   int a_died;
   struct char_data *challenger;

   int trans;
   int trans_hp;
   int trans_mana;
   int trans_move;

   short int liberdade;

/* For the final sumary of the game */
   byte lvl_sum;
   int exp_sum;
   int gold_sum;
   int kills_mob_sum;
   int kills_ppl_sum;
   int died_sum;
   int points_sum;
   int points_g_sum;

};


/* Char's abilities.  Used in char_file_u *DO*NOT*CHANGE* */
struct char_ability_data {
   sbyte str;
   sbyte str_add;      /* 000 - 100 if strength 18             */
   sbyte intel;
   sbyte wis;
   sbyte dex;
   sbyte con;
   sbyte cha;
   sbyte luk;
};


/* Char's points.  Used in char_file_u *DO*NOT*CHANGE* */
struct char_point_data {
   int mana;
   int max_mana;     /* Max move for PC/NPC                     */
   int hit;
   int max_hit;      /* Max hit for PC/NPC                      */
   int move;
   int max_move;     /* Max move for PC/NPC                     */
   sh_int oxigen;
   sh_int max_oxigen;   /* Max oxigenation			   */
   int mental;
   int max_mental;	/* Max mental energy			   */

   sh_int armor;        /* Internal -100..100, external -10..10 AC */
   int  gold;           /* Money carried                           */
   int  bank_gold;      /* Gold the char has in a bank account     */
   int  exp;            /* The experience of the player            */

   sh_int hitroll;       /* Any bonus or penalty to the hit roll    */
   sh_int damroll;       /* Any bonus or penalty to the damage roll */

};


/*
* char_special_data_saved: specials which both a PC and an NPC have in
* common, but which must be saved to the playerfile for PC's.
*
* WARNING:  Do not change this structure.  Doing so will ruin the
* playerfile.  If you want to add to the playerfile, use the spares
* in player_special_data.
*/
struct char_special_data_saved {
   int  alignment;              /* +-1000 for alignments                */
   long idnum;                  /* player's idnum; -1 for mobiles       */
   long challenge;		/* player's challenger			*/
   long act;                    /* act flag for NPC's; player flag for PC's 
*/
   long act2;                    /* act flag for NPC's; player flag for PC's 
*/
   long long int affected_by;            /* Bitvector for spells/skills affected by 
*/
   long long int affected2_by;            /* Bitvector for spells/skills affected by 
*/
   long long int affected3_by;            /* Bitvector for spells/skills affected by 
*/
   sh_int apply_saving_throw[5]; /* Saving throw (Bonuses)              */
};


/* Special playing constants shared by PCs and NPCs which aren't in pfile */
struct char_special_data {
   struct char_data *fighting;  /* Opponent                             */
   struct char_data *hunting;   /* Char hunted by this char             */

   byte position;               /* Standing, fighting, sleeping, etc.   */
   byte nobashtime;		// !Alternar - Ips & Str

   int  carry_weight;           /* Carried weight                       */
   byte carry_items;            /* Number of items carried              */
   int  timer;                  /* Timer for update                     */

   struct char_special_data_saved saved; /* constants saved in plrfile  */
};


/*
*  If you want to add new values to the playerfile, do it here.  DO NOT
* ADD, DELETE OR MOVE ANY OF THE VARIABLES - doing so will change the
* size of the structure and ruin the playerfile.  However, you can change
* the names of the spares to something more meaningful, and then use them
* in your new code.  They will automatically be transferred from the
* playerfile into memory when players log in.
*/
struct player_special_data_saved {
   byte skills[MAX_SKILLS+1];   /* array of skills plus skill 0         */
   byte PADDING0;               /* used to be spells_to_learn           */
   bool talks[MAX_TONGUE];      /* PC s Tongues 0 for NPC               */
   int  wimp_level;             /* Below this # of hit points, flee!    */
   int icq;
     byte freeze_level;           /* Level of god who froze char, if any  */
   sh_int invis_level;          /* level of invisibility                */
   room_vnum load_room;         /* Which room to place char in          */
   long pref;                   /* preference flags for PC's.           */
   long pref2;                   /* preference flags for PC's.           */
   ubyte bad_pws;               /* number of bad password attemps       */
   sbyte conditions[3];         /* Drunk, full, thirsty                 */

   /* spares below for future expansion.  You can change the names from
      'sparen' to something meaningful, but don't change the order.  */

   int spells_to_learn;         /* How many can you learn yet this level*/
   int olc_zone;
   int points;
   int qpoints;
   int clan;  /* clan number */
   int clan_rank; /*  clan rank  */
   int cpoints;
   int house_vnum;
   ubyte uextra[10];

   short int style;
   sh_int rskills[MAX_REMORT][MAX_RSKILLS+1];   /* array of skills plus 
skill 0         */

   unsigned short int religion;
   short int rp;

   bool language;		// Idioma; TRUE = English; -ips

};

/*
* Specials needed only by PCs, not NPCs.  Space for this structure is
* not allocated in memory for NPCs, but it is for PCs and the portion
* of it labelled 'saved' is saved in the playerfile.  This structure can
* be changed freely; beware, though, that changing the contents of
* player_special_data_saved will corrupt the playerfile.
*/
struct player_special_data {
   struct player_special_data_saved saved;


   char *poofin;                /* Description on arrival of a god.     */
   char *poofout;               /* Description upon a god's exit.       */
   struct alias *aliases;       /* Character's aliases                  */
   long last_tell;              /* idnum of last tell from              */
   void *last_olc_targ;		/* olc control                          */
   struct char_data *betted_on; /* arena bet on who?                    */
   int bet_amt;                 /* arena bet amount                     */
   int last_olc_mode;		/* olc control                          */
   char *host;			/* player host				*/
   int new_wait;		/* guardar wait_state no personagem.. -ips */
};


/* Specials used by NPCs, not PCs */
struct mob_special_data {
   byte last_direction;     /* The last direction the monster went     */
   int  attack_type;        /* The Attack Type Bitvector for NPC's     */
   byte default_pos;        /* Default position for NPC                */
   memory_rec *memory;      /* List of attackers to remember           */
   sh_int damnodice;          /* The number of damage dice's             */
   sh_int damsizedice;        /* The size of the damage dice's           */
   int wait_state;          /* Wait state for bashed mobs              */
   int attack1;
   int attack2;
   int attack3;
};


/* An affect structure.  Used in char_file_u *DO*NOT*CHANGE* */
struct affected_type {
   sh_int type;          /* The type of spell that caused this      */
   sh_int duration;      /* For how long its effects will last      */
   int modifier;       /* This is added to apropriate ability     */
   byte location;        /* Tells which ability to change(APPLY_XXX)*/
   long bitvector;       /* Tells which bits to set (AFF_XXX)       */
   long bitvector2;       /* Tells which bits to set (AFF_XXX)       */
   long bitvector3;       /* Tells which bits to set (AFF_XXX)       */

   struct affected_type *next;
};


/* Structure used for chars following other chars */
struct follow_type {
   struct char_data *follower;
   struct follow_type *next;
};


/* ================== Structure for player/non-player ===================== 
*/
struct char_data {
   int pfilepos;                         /* playerfile pos                */
   sh_int nr;                            /* Mob's rnum                    */
   room_rnum in_room;                    /* Location (real room number)   */
   room_rnum was_in_room;                /* location for linkdead people  */

   struct char_player_data player;       /* Normal data                   */
   struct char_ability_data real_abils;  /* Abilities without modifiers   */
   struct char_ability_data aff_abils;   /* Abils with spells/stones/etc  */
   struct char_point_data points;        /* Points                        */
   struct char_special_data char_specials;      /* PC/NPC specials        */
   struct player_special_data *player_specials; /* PC specials            */
   struct mob_special_data mob_specials;        /* NPC specials           */

   struct affected_type *affected;       /* affected by what spells       */
   struct obj_data *equipment[NUM_WEARS];/* Equipment array               */

   struct obj_data *carrying;            /* Head of list                  */
   struct descriptor_data *desc;         /* NULL for mobiles              */

   long id;                            /* used by DG triggers             */
   struct trig_proto_list *proto_script; /* list of default triggers      */
   struct script_data *script;         /* script info for the object      */
   struct script_memory *memory;       /* for mob memory triggers         */

   int current_quest;			/* vnum of current quest          */

   struct char_data *next_in_room;      /* For room->people - list         
*/
   struct char_data *next;              /* For either monster or ppl-list  
*/
   struct char_data *next_fighting;     /* For fighting list               
*/

   struct follow_type *followers;       /* List of chars followers       */
   struct char_data *master;            /* Who is char following?        */
};
/* ====================================================================== */


/* ==================== File Structure for Player ======================= */
/*             BEWARE: Changing it will ruin the playerfile               */
struct char_file_u {
   /* char_player_data */
   char name[MAX_NAME_LENGTH+1];
   char description[EXDSCR_LENGTH];
   char whoisdesc[EXDSCR_LENGTH];
   char title[MAX_TITLE_LENGTH+1];
   char prename[MAX_TITLE_LENGTH+1];
      int id_second;
   int id_master;

   byte sex;
   byte chclass;
   byte race;
   int level;
   byte remort;
   sh_int hometown;
   time_t birth;   /* Time of birth of character     */
   int  played;    /* Number of secs played in total */
   ubyte weight;
   ubyte height;
   byte fbi_pos;
   int killed;
   int died;
   int a_killed;
   int a_died;

   int trans;
   int trans_hp;
   int trans_mana;
   int trans_move;

   short int liberdade;

   char pwd[MAX_PWD_LENGTH+1];         /* character's password */


   struct char_special_data_saved char_specials_saved;
   struct player_special_data_saved player_specials_saved;
   struct char_ability_data abilities;
   struct char_point_data points;
   struct affected_type affected[MAX_AFFECT];

   int current_quest;

   time_t last_logon;           /* Time (in secs) of last logon */
   char host[HOST_LENGTH+1];    /* host of last logon */
};
/* ====================================================================== */


/* descriptor-related structures ******************************************/


struct txt_block {
   char *text;
   int aliased;
   struct txt_block *next;
};


struct txt_q {
   struct txt_block *head;
   struct txt_block *tail;
};


struct descriptor_data {
   socket_t     descriptor;     /* file descriptor for socket           */
   char host[HOST_LENGTH+1];    /* hostname                             */
   byte bad_pws;                /* number of bad pw attemps this login  */
   byte idle_tics;              /* tics idle at password prompt         */
   int  connected;              /* mode of 'connectedness'              */
   int  wait;                   /* wait for how many loops              */
   int  desc_num;               /* unique num assigned to desc          */
   time_t login_time;           /* when the person connected            */
   char *showstr_head;          /* for keeping track of an internal str */
   char **showstr_vector;       /* for paging through texts             */
   int  showstr_count;          /* number of pages to page through      */
   int  showstr_page;           /* which page are we currently showing? */
   char **str;                  /* for the modify-str system            */
   size_t max_str;              /*              -                       */
   char *backstr;               /* added for handling abort buffers     */
   long mail_to;                /* name for mail system                 */
   int  has_prompt;             /* is the user at a prompt?             */
   char inbuf[MAX_RAW_INPUT_LENGTH];  /* buffer for raw input           */
   char last_input[MAX_INPUT_LENGTH]; /* the last input                 */
   char small_outbuf[SMALL_BUFSIZE];  /* standard output buffer		*/
   char *output;                /* ptr to the current output buffer     */
   char **history;              /* History of commands, for ! mostly.   */
   int  history_pos;            /* Circular array position.             */
   int  bufptr;                 /* ptr to end of current output         */
   int  bufspace;               /* space left in the output buffer      */
   struct txt_block *large_outbuf; /* ptr to large buffer, if we need it */
   struct txt_q input;          /* q of unprocessed input               */
   struct char_data *character; /* linked to char                       */
   struct char_data *original;  /* original char if switched            */
   struct descriptor_data *snooping; /* Who is this char snooping       */
   struct descriptor_data *snoop_by; /* And who is snooping this char   */
   struct descriptor_data *next; /* link to next descriptor             */
   struct olc_data *olc;             /*. OLC info - defined in olc.h   .*/
   char   *storage;
   int hp, mana, move;
};


/* other miscellaneous structures ***************************************/


struct msg_type {
   char *attacker_msg;  /* message to attacker */
   char *victim_msg;    /* message to victim   */
   char *room_msg;      /* message to room     */
};


struct message_type {
   struct msg_type die_msg;     /* messages when death                  */
   struct msg_type miss_msg;    /* messages when miss                   */
   struct msg_type hit_msg;     /* messages when hit                    */
   struct msg_type god_msg;     /* messages when hit on god             */
   struct message_type *next;   /* to next messages of this kind.       */
};


struct message_list {
   int  a_type;                 /* Attack type                          */
   int  number_of_attacks;      /* How many attack messages to chose from. 
*/
   struct message_type *msg;    /* List of messages.                    */
};


struct dex_skill_type {
   sh_int p_pocket;
   sh_int p_locks;
   sh_int traps;
   sh_int sneak;
   sh_int hide;
};


struct dex_app_type {
   sh_int reaction;
   sh_int miss_att;
   sh_int defensive;
};


struct str_app_type {
   sh_int tohit;    /* To Hit (THAC0) Bonus/Penalty        */
   sh_int todam;    /* Damage Bonus/Penalty                */
   sh_int carry_w;  /* Maximum weight that can be carrried */
   sh_int wield_w;  /* Maximum weight that can be wielded  */
};


struct wis_app_type {
   byte bonus;       /* how many practices player gains per lev */
};


struct int_app_type {
   byte learn;       /* how many % a player learns a spell/skill */
};


struct con_app_type {
   sh_int hitp;
   sh_int shock;
};

struct message_data {
  char   message[MAX_STRING_LENGTH];
  char   writer[MAX_STRING_LENGTH];
  time_t time;
};


struct weather_data {
   int  pressure;       /* How is the pressure ( Mb ) */
   int  change; /* How fast and what way does it change. */
   int  sky;    /* How is the sky. */
   int  sunlight;       /* And how much sun. */
};


struct title_type {
   char *title_m;
   char *title_f;
   int  exp;
};


/* element in monster and object index-tables   */
struct index_data {
   int  vnum;           /* virtual number of this mob/obj               */
   int  number;         /* number of existing units of this mob/obj     */
   SPECIAL(*func);

   char *farg;         /* string argument for special function     */
   struct trig_data *proto;     /* for triggers... the trigger     */
};

/* linked list for mob/object prototype trigger lists */
struct trig_proto_list {
  int vnum;                             /* vnum of the trigger   */
  struct trig_proto_list *next;         /* next trigger          */
};

/* used in the socials */
struct social_messg {
   int act_nr;
   char *command;               /* holds copy of activating command */
   char *sort_as;		/* holds a copy of a similar command or
				 * abbreviation to sort by for the parser */
   int hide;			/* ? */
   int min_victim_position;	/* Position of victim */
   int min_char_position;	/* Position of char */
   int min_level_char;          /* Minimum level of socialing char */

   /* No argument was supplied */
   char *char_no_arg;
   char *others_no_arg;

   /* An argument was there, and a victim was found */
   char *char_found;
   char *others_found;
   char *vict_found;

   /* An argument was there, as well as a body part, and a victim was found 
*/
   char *char_body_found;
   char *others_body_found;
   char *vict_body_found;

   /* An argument was there, but no victim was found */
   char *not_found;

   /* The victim turned out to be the character */
   char *char_auto;
   char *others_auto;

   /* If the char cant be found search the char's inven and do these: */
   char *char_obj_found;
   char *others_obj_found;
};

struct teleport_data {
   int time;
   int targ;
   long mask;
   int cnt;
   int obj;
};

struct time_write {
   int year, month, day;
};

struct raff_node {
	room_rnum room;        /* location in the world[] array of the room */
	int      timer;       /* how many ticks this affection lasts */
	long     affection;   /* which affection does this room have */
	int      spell;       /* the spell number */

	struct raff_node *next; /* link to the next node */
};

