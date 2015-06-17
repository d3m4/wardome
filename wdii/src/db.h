/* ************************************************************************
*   File: db.h                                          Part of CircleMUD *
*  Usage: header file for database handling                               *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

/* arbitrary constants used by index_boot() (must be unique) */
#define DB_BOOT_WLD     0
#define DB_BOOT_MOB     1
#define DB_BOOT_OBJ     2
#define DB_BOOT_ZON     3
#define DB_BOOT_SHP     4
#define DB_BOOT_HLP     5
#define DB_BOOT_TRG	6
#define DB_BOOT_QST     7

#if defined(CIRCLE_MACINTOSH) 
#define LIB_WORLD       ":world:"
#define LIB_TEXT        ":text:"
#define LIB_TEXT_HELP   ":text:help:"
#define LIB_MISC        ":misc:"
#define LIB_ETC         ":etc:"
#define LIB_PLRTEXT     ":plrtext:"
#define LIB_PLROBJS     ":plrobjs:"
#define LIB_HOUSE       ":house:"
#define LIB_ALIAS       ":alias:"
#define SLASH           ":"
#elif defined(CIRCLE_AMIGA) || defined(CIRCLE_UNIX) || defined(CIRCLE_WINDOWS) || defined(CIRCLE_ACORN) || defined(__APPLE__) 
#define LIB_WORLD       "world/"
#define LIB_TEXT        "text/"
#define LIB_TEXT_HELP   "text/help/"
#define LIB_MISC        "misc/"
#define LIB_ETC         "etc/"
#define LIB_PLRTEXT     "plrtext/"
#define LIB_OBJS        "objs/"
#define LIB_PLROBJS     "plrobjs/"
#define LIB_HOUSE       "house/"
#define LIB_ALIAS       "alias/"
#define SLASH           "/"
#endif

#define SUF_OBJS        "objs"
#define SUF_TEXT        "text"
#define SUF_ALIAS       "alias"

#if defined(CIRCLE_AMIGA)
#define FASTBOOT_FILE   "/.fastboot"    /* autorun: boot without sleep  */
#define KILLSCRIPT_FILE "/.killscript"  /* autorun: shut mud down       */
#define PAUSE_FILE      "/pause"        /* autorun: don't restart mud   */
#elif defined(CIRCLE_MACINTOSH)
#define FASTBOOT_FILE   "::.fastboot"   /* autorun: boot without sleep  */
#define KILLSCRIPT_FILE "::.killscript" /* autorun: shut mud down       */
#define PAUSE_FILE      "::pause"       /* autorun: don't restart mud   */
#else
#define FASTBOOT_FILE   "../.fastboot"  /* autorun: boot without sleep  */
#define KILLSCRIPT_FILE "../.killscript"/* autorun: shut mud down       */
#define PAUSE_FILE      "../pause"      /* autorun: don't restart mud   */
#endif

/* names of various files and directories */
#define INDEX_FILE      "index"         /* index of world files         */
#define MINDEX_FILE     "index.mini"    /* ... and for mini-mud-mode    */
#define WLD_PREFIX      LIB_WORLD"wld"SLASH     /* room definitions     */
#define MOB_PREFIX      LIB_WORLD"mob"SLASH     /* monster prototypes   */
#define OBJ_PREFIX      LIB_WORLD"obj"SLASH     /* object prototypes    */
#define ZON_PREFIX      LIB_WORLD"zon"SLASH     /* zon defs & command tables */
#define SHP_PREFIX      LIB_WORLD"shp"SLASH     /* shop definitions     */
#define HLP_PREFIX      LIB_TEXT"help"SLASH     /* for HELP <keyword>   */
#define TRG_PREFIX	LIB_WORLD"trg"SLASH	/* trg definitions	*/
#define QST_PREFIX	LIB_WORLD"qst"SLASH     /* Quests               */
#define HELP_FILE	LIB_TEXT_HELP"help.hlp"	/* default file for help*/

#define CREDITS_FILE    LIB_TEXT"credits"/* for the 'credits' command   */
#define NEWS_FILE       LIB_TEXT"news"  /* for the 'news' command       */
#define MOTD_FILE       LIB_TEXT"motd"  /* messages of the day / mortal */
#define IMOTD_FILE      LIB_TEXT"imotd" /* messages of the day / immort */
#define HELP_PAGE_FILE  LIB_TEXT_HELP"screen" /* for HELP <CR>          */
#define INFO_FILE       LIB_TEXT"info"          /* for INFO             */
#define WIZLIST_FILE    LIB_TEXT"wizlist"       /* for WIZLIST          */
#define IMMLIST_FILE    LIB_TEXT"immlist"       /* for IMMLIST          */
#define BACKGROUND_FILE LIB_TEXT"background"/* for the background story */
#define POLICIES_FILE   LIB_TEXT"policies" /* player policies/rules     */
#define HANDBOOK_FILE   LIB_TEXT"handbook" /* handbook for new immorts  */

#define IDEA_FILE       LIB_MISC"ideas" /* for the 'idea'-command       */
#define TYPO_FILE       LIB_MISC"typos" /*         'typo'               */
#define BUG_FILE        LIB_MISC"bugs"  /*         'bug'                */
#define MESS_FILE       LIB_MISC"messages" /* damage messages           */
#define SOCMESS_FILE    LIB_MISC"socials" /* messgs for social acts     */
#define XNAME_FILE      LIB_MISC"xnames" /* invalid name substrings     */

#define PLAYER_FILE     LIB_ETC"players" /* the player database         */
#define MAIL_FILE       LIB_ETC"plrmail" /* for the mudmail system      */
#define BAN_FILE        LIB_ETC"badsites" /* for the siteban system     */
#define HCONTROL_FILE   LIB_ETC"hcontrol"  /* for the house system      */
#define CLAN_FILE       LIB_ETC"clans"    /* armazenador dos clans */
#define TOPTEN_FILE     LIB_MISC"topten" //topten quest
#define TOPTEN_FILE1    LIB_MISC"topten1"
#define TOPTEN_FILE2    LIB_MISC"topten2"

#define HOUSES_FILE     LIB_MISC"houses.mud" // Fenix - WHS

/* ascii pfiles file defines */
#define PLR_PREFIX	"pfiles"
#define PLR_INDEX_FILE	"pfiles"SLASH"plr_index"

/* change these if you want to put all files in the same directory (or if
   you just like big file names
*/
#define PLR_SUFFIX	""

/* new bitvector data for use in player_index_element */
#define PINDEX_DELETED		(1 << 0)	/* deleted player	*/
#define PINDEX_NODELETE		(1 << 1)	/* protected player	*/
#define PINDEX_SELFDELETE	(1 << 2)	/* player is selfdeleting*/


/* public procedures in db.c */
void    boot_db(void);
int     create_entry(char *name);
void    zone_update(void);
int     real_room(int vnum);
char    *fread_string(FILE *fl, char *error);
long    get_id_by_name(char *name);
char    *get_name_by_id(long id);

int	load_char(char *name, struct char_data *ch);
void    save_char(struct char_data *ch, sh_int load_room);
void    init_char(struct char_data *ch);
struct char_data* create_char(void);
struct char_data *read_mobile(int nr, int type);
int     real_mobile(int vnum);
int     vnum_mobile(char *searchname, struct char_data *ch);
void    clear_char(struct char_data *ch);
void    reset_char(struct char_data *ch);
void    free_char(struct char_data *ch);
void	save_player_index(void);
void    read_message_from_file(void);
void    write_message_to_file(void);

void    vwear_object(int wearpos, struct char_data * ch);
struct  obj_data *create_obj(void);
void    clear_object(struct obj_data *obj);
void    free_obj(struct obj_data *obj);
int     real_object(int vnum);
struct obj_data *read_object(int nr, int type);
int     vnum_object(char *searchname, struct char_data *ch);

extern int teleport_on;

#define REAL 0
#define VIRTUAL 1

/* structure for the reset commands */
struct reset_com {
   char command;   /* current command                      */

   bool if_flag;        /* if TRUE: exe only if preceding exe'd */
   int  arg1;           /*                                      */
   int  arg2;           /* Arguments to the command             */
   int  arg3;           /*                                      */
   int line;            /* line number this command appears on  */

   /*
        *  Commands:              *
        *  'M': Read a mobile     *
        *  'O': Read an object    *
        *  'G': Give obj to mob   *
        *  'P': Put obj in obj    *
        *  'G': Obj to char       *
        *  'E': Obj to char equip *
        *  'D': Set state of door *
   */
};



/* zone definition structure. for the 'zone-table'   */
struct zone_data {
   char *name;              /* name of this zone                  */
   int  lifespan;           /* how long between resets (minutes)  */
   int  age;                /* current age of this zone (minutes) */
   int  top;                /* upper limit for rooms in this zone */

   int  reset_mode;         /* conditions for reset (see below)   */
   int  number;             /* virtual number of this zone        */
   struct reset_com *cmd;   /* command table for reset            */
   int  zone_flags;        /* for zone flags                     */
   int owner;


   /*
        *  Reset mode:                              *
        *  0: Don't reset, and don't update age.    *
        *  1: Reset if no PC's are located in zone. *
        *  2: Just reset.                           *
   */
};



/* for queueing zones for update   */
struct reset_q_element {
   int  zone_to_reset;            /* ref to zone_data */
   struct reset_q_element *next;
};



/* structure for the update queue     */
struct reset_q_type {
   struct reset_q_element *head;
   struct reset_q_element *tail;
};


/* Added level, flags, and last, primarily for pfile autocleaning.  You
   can also use them to keep online statistics, and can add race, class,
   etc if you like.
*/
struct player_index_element {
   char *name;
   long id;
   int level;
   int flags;
   time_t last;
};


struct help_index_element {
//   char *keyword;
   char	*keywords;
   char *entry;
//   int duplicate;
   int min_level;
};

struct quest_eqs {
	int vnum_eq;
	int preco_eq;
	int dono;
};

/* don't change these */
#define BAN_NOT         0
#define BAN_NEW         1
#define BAN_SELECT      2
#define BAN_ALL         3

#define BANNED_SITE_LENGTH    50
struct ban_list_element {
   char site[BANNED_SITE_LENGTH+1];
   int  type;
   time_t date;
   char name[MAX_NAME_LENGTH+1];
   struct ban_list_element *next;
};


/* global buffering system */

#ifdef __DB_C__
char    buf[MAX_STRING_LENGTH];
char    buf1[MAX_STRING_LENGTH];
char    buf2[MAX_STRING_LENGTH];
char    buf3[MAX_STRING_LENGTH];
char    arg[MAX_STRING_LENGTH];
#else
extern struct player_special_data dummy_mob;
extern char     buf[MAX_STRING_LENGTH];
extern char     buf1[MAX_STRING_LENGTH];
extern char     buf2[MAX_STRING_LENGTH];
extern char     buf3[MAX_STRING_LENGTH];
extern char     arg[MAX_STRING_LENGTH];
#endif

#ifndef __CONFIG_C__
extern char     *OK;
extern char     *NOPERSON;
extern char     *NOEFFECT;
#endif

#define CUR_WORLD_VERSION 1
#define CUR_ZONE_VERSION 2
