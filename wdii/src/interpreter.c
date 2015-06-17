/* ************************************************************************
*   File: interpreter.c                                 Part of CircleMUD *
*  Usage: parse user commands, search for specials, call ACMD functions   *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#define __INTERPRETER_C__

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "buffer.h"
#include "comm.h"
#include "interpreter.h"
#include "db.h"
#include "utils.h"
#include "spells.h"
#include "handler.h"
#include "mail.h"
#include "screen.h"
#include "olc.h"
#include "dg_scripts.h"
#include "winddragon.h"
#include "clan.h"
#include "masters.h"

extern sh_int r_mortal_start_room; 
extern sh_int r_immort_start_room; 
extern sh_int r_frozen_start_room; 
extern sh_int r_deadly_start_room; 
extern const char *class_menu;
extern const char *race_menu;
extern char *motd;
extern char *imotd;
extern char *background;
extern char *policies;
extern char *MENU;
extern char *WELC_MESSG;
extern char *START_MESSG; 
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct player_index_element *player_table;
extern int top_of_p_table;
extern int circle_restrict;
extern int no_specials;
extern int max_bad_pws;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct room_data *world;
extern char *pc_race_help_types[];
extern int top_of_world;
extern int selfdelete_fastwipe;

/* external functions */
void echo_on(struct descriptor_data *d);
void echo_off(struct descriptor_data *d);
void do_start(struct char_data *ch);
int parse_class(char arg);
int parse_race(char arg);
int special(struct char_data *ch, int cmd, char *arg);
int isbanned(char *hostname);
int Valid_Name(char *newname);
void oedit_parse(struct descriptor_data *d, char *arg);
void redit_parse(struct descriptor_data *d, char *arg);
void hedit_parse(struct descriptor_data *d, char *arg);
void zedit_parse(struct descriptor_data *d, char *arg);
void medit_parse(struct descriptor_data *d, char *arg);
void sedit_parse(struct descriptor_data *d, char *arg);
void aedit_parse(struct descriptor_data *d, char *arg);
void trigedit_parse(struct descriptor_data *d, char *arg);
void qedit_parse(struct descriptor_data *d, char *arg);
void read_aliases(struct char_data *ch);
void remove_player(int pfilepos);
int is_complete(int vnum);
int allowed_classes(int x, int i);
void display_classes(struct descriptor_data *d);
void write_last_command(char *arg);
int do_run(struct char_data *ch, char argument[25]);
void actualize_splskl(struct char_data *ch);
void display_races(struct descriptor_data *d);
void display_classes(struct descriptor_data *d);
void disp_abils(struct descriptor_data *d, int class);
void display_stat(struct descriptor_data *d);
void roll_real_abils(struct char_data * ch);

/* local functions */
int perform_dupe_check(struct descriptor_data *d);
struct alias *find_alias(struct alias *alias_list, char *str);
void free_alias(struct alias *a);
void perform_complex_alias(struct txt_q *input_q, char *orig, struct alias *a);
int perform_alias(struct descriptor_data *d, char *orig);
int reserved_word(char *argument);
int find_name(char *name);
int _parse_name(char *arg, char *name);
struct char_data *is_playing(char *vict_name);
void topten(struct char_data *topch) ;
void topten1(struct char_data *topch) ;
void topten2(struct char_data *topch) ;


/* prototypes for all do_x functions. */

//religiao:
ACMD(do_muir);
ACMD(do_ilmanateur);
ACMD(do_selune);
ACMD(do_helm);
ACMD(do_ohgma);
ACMD(do_tempus);
ACMD(do_bane);
ACMD(do_pandora);
ACMD(do_talos);

ACMD(do_topten);
ACMD(do_relialin);
ACMD(do_pray);
ACMD(do_action);
ACMD(do_advance);
ACMD(do_aedit);
ACMD(do_affections);
//ACMD(do_accept);
ACMD(do_aid);
ACMD(do_alias);
ACMD(do_arena);
ACMD(do_assist);
ACMD(do_at);
ACMD(do_auction);
ACMD(do_awho);
ACMD(do_ahall);
ACMD(do_backstab);
ACMD(do_ban);
ACMD(do_bash);
ACMD(do_berzerk);
ACMD(do_bid);
ACMD(do_bet);
ACMD(do_blood_ritual);
ACMD(do_cast);
ACMD(do_chaos);
ACMD(do_chown);
ACMD(do_chop);
ACMD(do_circle);
ACMD(do_clan);
//ACMD(do_challenge);
ACMD(do_color);
ACMD(do_combo);
ACMD(do_commands);
ACMD(do_compare);
ACMD(do_consider);
ACMD(do_copy);
ACMD(do_copyover);
ACMD(do_credits);
ACMD(do_date);
ACMD(do_dc);
ACMD(do_diagnose);
ACMD(do_disarm);
ACMD(do_display);
ACMD(do_drink);
ACMD(do_drown);
ACMD(do_drop);
//ACMD(do_decline);
ACMD(do_eat);
ACMD(do_echo);
ACMD(do_enter);
ACMD(do_equipment);
ACMD(do_escape);
ACMD(do_examine);
ACMD(do_exit);
ACMD(do_exits);
ACMD(do_fbi);
ACMD(do_find);
ACMD(do_flee);
ACMD(do_follow);
ACMD(do_force);
ACMD(do_gas);
ACMD(do_gecho);
ACMD(do_gmote);
ACMD(do_gen_comm);
ACMD(do_gen_door);
ACMD(do_gen_ps);
ACMD(do_gen_tog);
ACMD(do_gen_write);
ACMD(do_get);
ACMD(do_give);
ACMD(do_gold);
ACMD(do_goto);
ACMD(do_grab);
ACMD(do_group);
ACMD(do_gsay);
ACMD(do_gut);
ACMD(do_headbash);
ACMD(do_hcontrol);
ACMD(do_help);
ACMD(do_hide);
ACMD(do_hit);
ACMD(do_house);
//ACMD(do_ignore);//last
ACMD(do_info);
ACMD(do_insult);
ACMD(do_inventory);
ACMD(do_invis);
ACMD(do_kick);
ACMD(do_kickflip);
ACMD(do_kill);
ACMD(do_knockout);
ACMD(do_last);
ACMD(do_leave);
ACMD(do_levels);
ACMD(do_liblist);
ACMD(do_listen);
ACMD(do_load);
ACMD(do_look);
/* ACMD(do_move); -- interpreter.h */
ACMD(do_meditate);
ACMD(do_message);
ACMD(do_master);
ACMD(do_mobfix);
ACMD(do_not_here);
ACMD(do_offer);
ACMD(do_olc);
ACMD(do_order);
ACMD(do_page);
ACMD(do_pk);
//ACMD(do_peace);
ACMD(do_player);
ACMD(do_playerlink);
ACMD(do_poofset);
ACMD(do_pour);
ACMD(do_prename);
ACMD(do_purge);
ACMD(do_put);
ACMD(do_qlist);
ACMD(do_qstat);
ACMD(do_qcomm);
ACMD(do_quit);
ACMD(do_reboot);
ACMD(do_recall);
ACMD(do_recharge);
ACMD(do_remort);
ACMD(do_remove);
ACMD(do_rent);
ACMD(do_reply);
ACMD(do_report);
ACMD(do_rescue);
ACMD(do_rest);
ACMD(do_restore);
ACMD(do_return);
ACMD(do_revive);
ACMD(do_rlink);
ACMD(do_sabre);
ACMD(do_sacrifice);
ACMD(do_save);
ACMD(do_say);
ACMD(do_scan);
ACMD(do_score);
ACMD(do_seelog);
ACMD(do_send);
ACMD(do_set);
ACMD(do_show);
ACMD(do_shutdown);
ACMD(do_sit);
ACMD(do_skillset);
//ACMD(do_slang);
ACMD(do_sleep);
ACMD(do_slist);
ACMD(do_skill);
ACMD(do_sneak);
ACMD(do_snoop);
ACMD(do_spec_comm);
ACMD(do_spell);
ACMD(do_split);
ACMD(do_stand);
ACMD(do_stat);
ACMD(do_steal);
ACMD(do_style);
ACMD(do_stroke);
ACMD(do_summary);
ACMD(do_suicide);
ACMD(do_switch);
ACMD(do_syslog);
ACMD(do_tedit);
ACMD(do_teleport);
ACMD(do_tell);
ACMD(do_time);
ACMD(do_title);
ACMD(do_toggle);
ACMD(do_track);
ACMD(do_trans);
ACMD(do_transform);
ACMD(do_throw);
ACMD(do_unban);
ACMD(do_ungroup);
ACMD(do_use);
ACMD(do_users);
ACMD(do_visible);
ACMD(do_vnum);
ACMD(do_vstat);
ACMD(do_veq);
ACMD(do_wanted);
ACMD(do_warshout);
ACMD(do_wake);
ACMD(do_wear);
ACMD(do_weather);
ACMD(do_where);
ACMD(do_who);
ACMD(do_wield);
ACMD(do_wimpy);
ACMD(do_wizlock);
ACMD(do_wiznet);
ACMD(do_wizutil);
ACMD(do_write);
ACMD(do_whois);
ACMD(do_whirlwind);
ACMD(do_xname);
ACMD(do_zreset);

/* DG Script ACMD's */
ACMD(do_attach);
ACMD(do_detach);
ACMD(do_tlist);
ACMD(do_tstat);
ACMD(do_masound);
ACMD(do_mkill);
ACMD(do_mjunk);
ACMD(do_mechoaround);
ACMD(do_msend);
ACMD(do_mecho);
ACMD(do_mload);
ACMD(do_mpurge);
ACMD(do_mgoto);
ACMD(do_mat);
ACMD(do_mteleport);
ACMD(do_mforce);
ACMD(do_mexp);
ACMD(do_mgold);
ACMD(do_mhunt);
ACMD(do_mremember);
ACMD(do_mforget);
ACMD(do_mtransform);

/* Program run commands */
ACMD(do_swho);
ACMD(do_slast);
ACMD(do_grep);
ACMD(do_sps);
ACMD(do_ukill);
ACMD(do_sexec);

struct command_info *complete_cmd_info;

/* This is the Master Command List(tm).

 * You can put new commands in, take commands out, change the order
 * they appear in, etc.  You can adjust the "priority" of commands
 * simply by changing the order they appear in the command list.
 * (For example, if you want "as" to mean "assist" instead of "ask",
 * just put "assist" above "ask" in the Master Command List(tm).
 *
 * In general, utility commands such as "at" should have high priority;
 * infrequently used and dangerously destructive commands should have low
 * priority.
 */

cpp_extern const struct command_info cmd_info[] = {

    /* this must be first -- for specprocs */
   { "RESERVED"	, "RESERVED"	, 0, 0, 0, 0 },

    /* directions must come before other commands but after RESERVED */
   { "north"	, "north"	, POS_STANDING, do_move     , 0, SCMD_NORTH },
   { "east"	, "east"	, POS_STANDING, do_move     , 0, SCMD_EAST },
   { "south"	, "south"	, POS_STANDING, do_move     , 0, SCMD_SOUTH },
   { "west"	, "west"	, POS_STANDING, do_move     , 0, SCMD_WEST },
   { "up"	, "up"		, POS_STANDING, do_move     , 0, SCMD_UP },
   { "down"	, "down"	, POS_STANDING, do_move     , 0, SCMD_DOWN },

   /* Punctuation Commands */
   { "."        , "."		, POS_SLEEPING, do_gen_comm , 10, SCMD_GOSSIP },
   { "?"        , "?"		, POS_DEAD    , do_help     , 0, 0 },


   /* now, the main list */
   { "at"       , "a"		, POS_DEAD    , do_at       , LVL_ELDER, 0 },
   { "advance"  , "ad"		, POS_DEAD    , do_advance  , LVL_GOD, 0 },
   { "aedit"    , "aed"		, POS_DEAD    , do_olc      , LVL_BUILDER, SCMD_OLC_AEDIT },
   { "affections","aff"         , POS_DEAD    , do_affections,0, 0 },
   { "afk"      , "afk"		, POS_DEAD    , do_gen_tog  , 0, SCMD_AFK },
 //  { "accept"   , "accept"     , POS_STANDING, do_accept   , 1, 0 },
   { "aid"      , "aid"		, POS_STANDING, do_aid      , 0, 0 },
   { "alias"    , "ali"		, POS_DEAD    , do_alias    , 0, 0 },
   { "alignment"  ,"alignment"  , POS_STANDING, do_relialin , 1, 0 },
   { "assist"   , "as"		, POS_FIGHTING, do_assist   , 1, 0 },
   { "ask"      , "ask"		, POS_RESTING , do_spec_comm, 0, SCMD_ASK },
   { "auction"  , "auct"	, POS_STANDING, do_auction  , 0, 0 },
   { "autoassist","autoass"	, POS_DEAD    , do_gen_tog  , 0, SCMD_AUTOASSIST },
   { "autoexit" , "autoex"	, POS_DEAD    , do_gen_tog  , 0, SCMD_AUTOEXIT },
   { "autogold" , "autogo"	, POS_DEAD    , do_gen_tog  , 0, SCMD_AUTOGOLD },
   { "autoloot" , "autolo"	, POS_DEAD    , do_gen_tog  , 0, SCMD_AUTOLOOT },
   { "autosplit", "autosp"	, POS_DEAD    , do_gen_tog  , 0, SCMD_AUTOSPLIT },
   { "ahall"    , "ahall"       , POS_STANDING, do_ahall    , 0, 0 },

   { "backstab" , "b"		, POS_STANDING, do_backstab , 1, 0 },
   { "ban"      , "ba"		, POS_DEAD    , do_ban      , LVL_SUPGOD, 0 },
   { "balance"  , "bal"		, POS_STANDING, do_not_here , 1, 0 },
   { "bash"     , "bash"	, POS_FIGHTING, do_bash     , 1, 0 },
   { "berzerk"  , "ber"		, POS_FIGHTING, do_berzerk  , 0, 0 },
   { "bid"      , "bid"		, POS_SLEEPING, do_bid	    , 0, 0 },
   { "bet"      , "bet"         , POS_RESTING , do_bet      , 0, 0 },
   { "boot"     , "boot"        , POS_DEAD    , do_copyover , LVL_GRGOD, 0 },
   { "brief"    , "brief"	, POS_DEAD    , do_gen_tog  , 0, SCMD_BRIEF },
   { "buy"      , "bu"		, POS_STANDING, do_not_here , 0, 0 },
   { "bug"      , "bug"		, POS_DEAD    , do_gen_write, 10, SCMD_BUG },
   { "blood"    , "bloo"	, POS_STANDING, do_blood_ritual , 1, 0 },

   { "cast"     , "c"		, POS_SITTING , do_cast     , 1, 0 },
   { "chaos"    , "chaos"	, POS_STANDING, do_chaos    , LVL_ELDER, 0 },
   { "chat"     , "cha"		, POS_SLEEPING, do_gen_comm , 10, SCMD_CHAT },
   { "check"    , "check"	, POS_STANDING, do_not_here , 1, 0 },
   { "chown"    , "chow"	, POS_STANDING, do_chown    , LVL_GOD, 0 },
   { "chop "    , "chop"	, POS_FIGHTING, do_chop     , 1, 0 },
   { "circle"   , "cir" 	, POS_FIGHTING, do_circle   , 0, 0 },
   { "clan"     , "clan"	, POS_SLEEPING, do_clan     , 1, 0 },
 //  { "challenge", "chal"	, POS_SLEEPING, do_challenge, 1, 0 },
   { "clear"    , "clear" 	, POS_DEAD    , do_gen_ps   , 0, SCMD_CLEAR },
   { "close"    , "close"	, POS_SITTING , do_gen_door , 0, SCMD_CLOSE },
   { "cls"      , "cls"		, POS_DEAD    , do_gen_ps   , 0, SCMD_CLEAR },
   { "consider" , "co"		, POS_RESTING , do_consider , 0, 0 },
   { "color"    , "color"	, POS_DEAD    , do_color    , 0, 0 },
   { "combo"    , "comb"	, POS_FIGHTING, do_combo    , 0, 0 },
   { "commands" , "comm"	, POS_DEAD    , do_commands , 0, SCMD_COMMANDS },
   { "compact"  , "comp"	, POS_DEAD    , do_gen_tog  , 0, SCMD_COMPACT },
   { "compare"  , "compar"	, POS_STANDING, do_compare  , 0, 0 },
   { "copy"     , "copy"	, POS_DEAD    , do_copy     , LVL_BUILDER, 0 },
   { "credits"  , "cred"	, POS_DEAD    , do_gen_ps   , 0, SCMD_CREDITS },
   { "crossdoors","crossd"	, POS_DEAD    , do_gen_tog  , LVL_ELDER, SCMD_CROSSDOORS },
   { "change"   , "chan"        , POS_SLEEPING, do_not_here , 1, 0 },

   { "date"     , "date"	, POS_DEAD    , do_date     , LVL_ELDER, SCMD_DATE },
//   { "decline"  , "decline"	, POS_STANDING, do_decline  , 1, 0 },
   { "dc"       , "dc"		, POS_DEAD    , do_dc       , LVL_SUPGOD, 0 },
   { "deposit"  , "depo"	, POS_STANDING, do_not_here , 1, 0 },
   { "destruction", "destruction" , POS_FIGHTING, do_talos    , 100 , 0 },
   { "diagnose" , "diag"	, POS_RESTING , do_diagnose , 0, 0 },
   { "disarm"   , "disa"	, POS_FIGHTING, do_disarm   , 0, 0 },
   { "display"  , "disp"	, POS_DEAD    , do_display  , 0, 0 },
   { "dlist"    , "dl"          , POS_DEAD    , do_liblist  , LVL_BUILDER, SCMD_DLIST },
   { "donate"   , "donate"	, POS_RESTING , do_drop     , 0, SCMD_DONATE },
   { "drink"    , "drink"	, POS_RESTING , do_drink    , 0, SCMD_DRINK },
   { "drown"    , "drown"	, POS_RESTING , do_drown    , 1, 0 },
   { "drop"     , "drop"	, POS_RESTING , do_drop     , 0, SCMD_DROP },

   { "eat"      , "eat"		, POS_RESTING , do_eat      , 0, SCMD_EAT },
   { "echo"     , "echo"	, POS_SLEEPING, do_echo     , LVL_LORD, SCMD_ECHO },
   { "emote"    , "emo"		, POS_RESTING , do_echo     , 1, SCMD_EMOTE },
   { "elist"    , "el" 		, POS_DEAD    , do_liblist  , LVL_BUILDER, SCMD_ELIST },
   { "enter"    , "enter"	, POS_STANDING, do_enter    , 0, 0 },
   { "equipment", "eq"		, POS_SLEEPING, do_equipment, 0, 0 },
   { "escape"   , "esc"		, POS_FIGHTING, do_escape   , 1, 0 },
   { "exits"    , "ex"		, POS_RESTING , do_exits    , 0, 0 },
   { "examine"  , "exa"		, POS_SITTING , do_examine  , 0, 0 },

   { "fbi"      , "fbi"		, POS_DEAD    , do_fbi      , LVL_SUPGOD, 0 },
   { "find"     , "fin"		, POS_DEAD    , do_find     , LVL_GOD, 0 },
   { "finger"   , "fi"  	, POS_DEAD    , do_player   , 0, 0 },
   { "force"    , "f"		, POS_SLEEPING, do_force    , LVL_GOD, 0 },
   { "forge"    , "for"		, POS_STANDING, do_not_here , 1, 0 },
   { "fill"     , "fill"	, POS_STANDING, do_pour     , 0, SCMD_FILL },
   { "flee"     , "fl"		, POS_FIGHTING, do_flee     , 1, 0 },
   { "follow"   , "fol"		, POS_RESTING , do_follow   , 0, 0 },
   { "freedom"    , "freedom"     , POS_FIGHTING, do_selune   , 0 , 0 },
   { "freeze"   , "fr"		, POS_DEAD    , do_wizutil  , LVL_GOD, SCMD_FREEZE },

   { "gas"      , "gas"		, POS_STANDING, do_gas      , 0, 0 },
   { "get"      , "g"		, POS_RESTING , do_get      , 0, 0 },
   { "gecho"    , "gecho"	, POS_DEAD    , do_gecho    , LVL_GOD, 0 },
   { "gemote"   , "gem"         , POS_SLEEPING, do_gen_comm , 0, SCMD_GMOTE },
   { "give"     , "gi"		, POS_RESTING , do_give     , 0, 0 },
   { "goto"     , "go"		, POS_SLEEPING, do_goto     , LVL_ELDER, 0 },
   { "gold"     , "gol"		, POS_RESTING , do_gold     , 0, 0 },
   { "gossip"   , "gos"		, POS_SLEEPING, do_gen_comm , 10, SCMD_GOSSIP },
   { "group"    , "gr"		, POS_RESTING , do_group    , 1, 0 },
   { "grab"     , "grab"	, POS_RESTING , do_grab     , 0, 0 },
   { "grats"    , "grats"	, POS_SLEEPING, do_gen_comm , 10, SCMD_GRATZ },
   { "grep"     , "gre"	        , POS_DEAD    , do_grep     , LVL_SUBIMPL, 0 },
   { "gsay"     , "gs"		, POS_SLEEPING, do_gsay     , 0, 0 },
   { "gtell"    , "gt"		, POS_SLEEPING, do_gsay     , 0, 0 },
   { "gut"    , "gut"		, POS_SLEEPING, do_gut      , 0, 0 },

   { "harakiri" , "hk"		, POS_FIGHTING, do_suicide  , 0, 0 },
   { "headbash" , "headba"	, POS_FIGHTING, do_headbash , 0, 0 },
   { "hedit"    , "hed" 	, POS_DEAD    , do_olc	    , LVL_BUILDER, SCMD_OLC_HEDIT },
   { "help"     , "h"		, POS_DEAD    , do_help     , 0, 0 },
   { "handbook" , "hand"	, POS_DEAD    , do_gen_ps   , LVL_ELDER, SCMD_HANDBOOK },
   { "hcontrol" , "hcontrol", POS_DEAD    , do_hcontrol , LVL_SUPGOD, 0 },
   { "hide"     , "hide"	, POS_RESTING , do_hide     , 1, 0 },
   { "hit"      , "hit"		, POS_FIGHTING, do_hit      , 0, SCMD_HIT },
   { "hold"     , "hold"	, POS_RESTING , do_grab     , 1, 0 },
   { "holler"   , "holler"	, POS_RESTING , do_gen_comm , 1, SCMD_HOLLER },
   { "holylight", "holylight"	, POS_DEAD    , do_gen_tog  , LVL_ELDER, SCMD_HOLYLIGHT },
   { "house"    , "house"	, POS_RESTING , do_house    , 0, 0 },
   { "honor"      , "honor"       , POS_FIGHTING, do_muir     , 0 , 0 },
   { "humility"   , "humility"    , POS_FIGHTING, do_ilmanateur ,50 ,0 },

   { "inventory", "i"		, POS_DEAD    , do_inventory, 0, 0 },
   { "idea"     , "idea"	, POS_DEAD    , do_gen_write, 10, SCMD_IDEA },
   { "identify"   , "identify"    , POS_FIGHTING, do_ohgma    , 0 , 0 },
   //( "ignore"   , "ignor"       , POS_DEAD    , do_ignore   , LVL_ELDER, 0 },//last
   { "imotd"    , "im"		, POS_DEAD    , do_gen_ps   , LVL_ELDER, SCMD_IMOTD },
   { "immlist"  , "imm"		, POS_DEAD    , do_gen_ps   , 0, SCMD_IMMLIST },
   { "improve"  , "improve"     , POS_STANDING, do_not_here  , 1 , 0 },
   { "info"     , "in"		, POS_SLEEPING, do_gen_ps   , 0, SCMD_INFO },
   { "insult"   , "insult"	, POS_RESTING , do_insult   , 0, 0 },
   { "invis"    , "invis"	, POS_DEAD    , do_invis    , LVL_ELDER, 0 },

   { "junk"     , "j"		, POS_RESTING , do_drop     , 0, SCMD_JUNK },
   { "join"	, "join"	, POS_STANDING, do_not_here , 0, 0},

   { "kill"     , "ki"		, POS_FIGHTING, do_kill     , 0, 0 },
   { "kick"     , "kick"	, POS_FIGHTING, do_kick     , 1, 0 },
   { "kickflip" , "kickflip"	, POS_FIGHTING, do_kickflip , 1, 0 },
   { "knockout" , "knock"	, POS_STANDING, do_knockout , 1, 0 },

   { "look"     , "l"		, POS_RESTING , do_look     , 0, SCMD_LOOK },
   { "law"        , "law"         , POS_STANDING, do_helm     , 100 , 0 },
   { "last"     , "last"	, POS_DEAD    , do_last     , LVL_GOD, 0 },
   { "leave"    , "leave"	, POS_STANDING, do_leave    , 0, 0 },
   { "levels"   , "lev"		, POS_DEAD    , do_levels   , 0, 0 },
   { "list"     , "list"	, POS_STANDING, do_not_here , 0, 0 },
   { "listen"   , "liste"	, POS_STANDING, do_listen   , 0, 0 },
   { "llist"    , "ll" 		, POS_DEAD    , do_liblist  , LVL_BUILDER, SCMD_LLIST },
   { "lock"     , "lo"		, POS_SITTING , do_gen_door , 0, SCMD_LOCK },
   { "load"     , "lock"	, POS_DEAD    , do_load     , LVL_GOD, 0},

   { "map"		,"map"		, POS_DEAD    , do_gen_tog  , 0, SCMD_MAP },
   { "meta"     , "met" 	, POS_RESTING , do_not_here , 0, 0 },
   { "mobfix"   , "mf"		, POS_DEAD    , do_mobfix   , LVL_BUILDER, 0 },
   { "motd"     , "m"		, POS_DEAD    , do_gen_ps   , 0, SCMD_MOTD },
   { "mail"     , "mail"	, POS_STANDING, do_not_here , 1, 0 },
   { "medit"    , "med"		, POS_DEAD    , do_olc      , LVL_BUILDER, SCMD_OLC_MEDIT},
   { "meditate" , "medit"	, POS_RESTING , do_meditate , 1, 0 },
   { "message"  , "mess"        , POS_DEAD    , do_message  , LVL_ELDER , 0 },
   { "mlist"    , "ml"		, POS_DEAD    , do_liblist  , LVL_BUILDER, SCMD_MLIST },
   { "mount"    , "mou" 	, POS_RESTING , do_not_here , 0, 0 },
   { "mute"     , "mu"		, POS_DEAD    , do_wizutil  , LVL_DEMIGOD, SCMD_SQUELCH },
   { "murder"   , "murder"	, POS_FIGHTING, do_hit      , 0, SCMD_MURDER },
   { "master "  , "mast"        , POS_STANDING, do_master  , 1 , 0 },

   { "newbie"   , "newb"	, POS_SLEEPING, do_gen_comm , 0, SCMD_NEWBIE },
   { "news"     , "n"		, POS_SLEEPING, do_gen_ps   , 0, SCMD_NEWS },
   { "noauction", "noauct"	, POS_DEAD    , do_gen_tog  , 0, SCMD_NOAUCTION },
   { "nochat"   , "nocha"	, POS_DEAD    , do_gen_tog  , 0, SCMD_NOCHAT },
   { "nogossip" , "nogos"	, POS_DEAD    , do_gen_tog  , 0, SCMD_NOGOSSIP },
   { "nograts"  , "nograts"	, POS_DEAD    , do_gen_tog  , 0, SCMD_NOGRATZ },
   { "nohassle" , "nohass"	, POS_DEAD    , do_gen_tog  , LVL_ELDER, SCMD_NOHASSLE },
   { "nonewbie" , "nonewb"	, POS_DEAD    , do_gen_tog  , 0, SCMD_NONEWBIE },
   { "norepeat" , "norep"	, POS_DEAD    , do_gen_tog  , 0, SCMD_NOREPEAT },
   { "noreply"  , "norepl"	, POS_DEAD    , do_gen_tog  , 1, SCMD_NOREPLY },
   { "noshout"  , "noshout"	, POS_SLEEPING, do_gen_tog  , 1, SCMD_DEAF },
   { "nosummon" , "nosum"	, POS_DEAD    , do_gen_tog  , 1, SCMD_NOSUMMON },
   { "notell"   , "notell"	, POS_DEAD    , do_gen_tog  , 1, SCMD_NOTELL },
   { "notips"   , "notip"       , POS_SLEEPING, do_gen_tog  , 1, SCMD_NOTIPS },
   { "notitle"  , "notitle"	, POS_DEAD    , do_wizutil  , LVL_GRGOD, SCMD_NOTITLE },
   { "nowiz"    , "nowiz"	, POS_DEAD    , do_gen_tog  , LVL_ELDER, SCMD_NOWIZ },
   
   { "order"    , "o"		, POS_RESTING , do_order    , 1, 0 },
   { "oedit"    , "oed"		, POS_DEAD    , do_olc      , LVL_BUILDER, SCMD_OLC_OEDIT},
   { "offer"    , "offer"	, POS_STANDING, do_not_here , 1, 0 },
   { "olc"      , "olc"		, POS_DEAD    , do_olc      , LVL_BUILDER, SCMD_OLC_SAVEINFO },
   { "olist"    , "oli" 	, POS_DEAD    , do_liblist  , LVL_BUILDER, SCMD_OLIST },
   { "open"     , "open"	, POS_SITTING , do_gen_door , 0, SCMD_OPEN },
   { "oppression"  , "oppresion"   , POS_FIGHTING, do_bane     , 50 , 0 },

   { "put"      , "p"		, POS_RESTING , do_put      , 0, 0 },
   { "page"     , "pa"		, POS_DEAD    , do_page     , LVL_GOD, 0 },
   { "pardon"   , "pard"	, POS_DEAD    , do_wizutil  , LVL_GOD, SCMD_PARDON },
   { "pick"     , "pick"	, POS_STANDING, do_gen_door , 1, SCMD_PICK },
   { "pk"	, "pk"		, POS_STANDING, do_pk	    , 0, 0 },
   { "pitbet"   , "pitb"        , POS_STANDING, do_not_here   , 0, 0 },
//   { "peace"    , "peace"       , POS_STANDING, do_peace    , 0, 0 },
   { "playerlink", "playerlink"	, POS_DEAD    , do_playerlink, LVL_MJGOD, 0 },
   { "policy"   , "pol"		, POS_DEAD    , do_gen_ps   , 0, SCMD_POLICIES },
   { "poofin"   , "poofin"	, POS_DEAD    , do_poofset  , LVL_ELDER, SCMD_POOFIN },
   { "poofout"  , "poofout"	, POS_DEAD    , do_poofset  , LVL_ELDER, SCMD_POOFOUT },
   { "pour"     , "pour"	, POS_STANDING, do_pour     , 0, SCMD_POUR },
   { "pray"	, "pray"	, POS_STANDING, do_pray	    , 0 , 0 }, 
   { "prename"  , "prenam"	, POS_DEAD    , do_prename  , 0, 0 },
   { "prompt"   , "pr"		, POS_DEAD    , do_display  , 0, 0 },
   { "purge"    , "purge"	, POS_DEAD    , do_purge    , LVL_DEMIGOD, 0 },

   { "quaff"    , "q"		, POS_RESTING , do_use      , 0, SCMD_QUAFF },
   { "qecho"    , "qecho"	, POS_DEAD    , do_qcomm    , LVL_DEMIGOD, SCMD_QECHO },
   { "qedit"	, "qedit"	, POS_DEAD    , do_olc	    , LVL_BUILDER, SCMD_OLC_QEDIT},
   { "qlist"	, "qlist"	, POS_DEAD    , do_qlist    , LVL_BUILDER, 0},
   { "qsay"     , "qs"		, POS_RESTING , do_qcomm    , 0, SCMD_QSAY },
   { "qstat"	, "qstat"	, POS_DEAD    , do_qstat    , LVL_BUILDER, 0},
   { "quest"    , "quest"	, POS_DEAD    , do_gen_tog  , 0, SCMD_QUEST },
   { "qui"      , "qui"		, POS_DEAD    , do_quit     , 0, 0 },
   { "quit"     , "quit"	, POS_DEAD    , do_quit     , 0, SCMD_QUIT },

   { "reply"    , "r"		, POS_SLEEPING, do_reply    , 0, 0 },
   { "rest"     , "re"		, POS_RESTING , do_rest     , 0, 0 },
   { "read"     , "read"	, POS_RESTING , do_look     , 0, SCMD_READ },
   { "recite"   , "rec"		, POS_RESTING , do_use      , 0, SCMD_RECITE },
   { "reborn"   , "reborn"      , POS_STANDING, do_not_here , 1, 0 },
   { "recall"   , "reca"	, POS_STANDING, do_recall   , 0, 0 },
   { "receive"  , "receive"	, POS_STANDING, do_not_here , 1, 0 },
   { "recharge" , "recharge", POS_STANDING, do_recharge , 1, 0 },
   { "redit"    , "red"		, POS_DEAD    , do_olc      , LVL_BUILDER, SCMD_OLC_REDIT},
   { "religion"  ,"religion"  , POS_STANDING, do_relialin , 1, 0 },
   { "reload"   , "rel"		, POS_DEAD    , do_reboot   , LVL_MJGOD, 0 },
   { "remove"   , "rem"		, POS_RESTING , do_remove   , 0, 0 },
   { "remort"   , "remort"	, POS_STANDING, do_remort   , 1, 0 },
   { "rent"     , "rent"	, POS_STANDING, do_not_here , 1, 0 },
   { "repair"   , "repa" 	, POS_RESTING , do_not_here , 0, 0 },
   { "report"   , "repo"	, POS_SLEEPING, do_report   , 0, 0 },
   { "reroll"   , "reroll"	, POS_DEAD    , do_wizutil  , LVL_GRGOD, SCMD_REROLL },
   { "rescue"   , "rescue"	, POS_FIGHTING, do_rescue   , 1, 0 },
   { "restore"  , "restore"	, POS_DEAD    , do_restore  , LVL_GOD, 0 },
   { "return"   , "return"	, POS_DEAD    , do_return   , 0, 0 },
   { "revive"   , "rev"		, POS_STANDING, do_revive   , 1, 0 },
   { "rlink"    , "rlink"	, POS_DEAD    , do_rlink    , LVL_BUILDER, 0 },
   { "rlist"    , "rl"		, POS_DEAD    , do_liblist  , LVL_BUILDER, SCMD_RLIST },
//   { "roomflags", "roomfl"	, POS_DEAD    , do_gen_tog  , LVL_IMMORT, SCMD_ROOMFLAGS },

   { "sabre"    , "sab"		, POS_FIGHTING, do_sabre    , 0, 0 },
   { "sacrifice", "sac"		, POS_STANDING, do_sacrifice, 0, 0 },
   { "say"      , "s"		, POS_RESTING , do_say      , 0, 0 },
   { "'"        , "s"		, POS_RESTING , do_say      , 0, 0 },
   { "save"     , "sa"		, POS_SLEEPING, do_save     , 0, 0 },
   { "score"    , "sc"		, POS_DEAD    , do_score    , 0, 0 },
   { "scan"     , "sca"		, POS_STANDING, do_scan     , 0, 0 },
   { "sedit"    , "sed"		, POS_DEAD    , do_olc      , LVL_BUILDER, SCMD_OLC_SEDIT},
   { "seduction"  , "seduction"   , POS_FIGHTING, do_pandora  , 50 , 0 },
   { "seelog"   , "see"		, POS_DEAD    , do_seelog   , LVL_GOD, 0 },
   { "sell"     , "sell"	, POS_STANDING, do_not_here , 0, 0 },
   { "send"     , "send"	, POS_SLEEPING, do_send     , LVL_GOD, 0 },
   { "set"      , "set"		, POS_DEAD    , do_set      , LVL_GOD, 0 },
   { "sexec"    , "sexec"       , POS_DEAD    , do_sexec    , 0, 0 },
   { "shout"    , "sh"		, POS_RESTING , do_gen_comm , 0, SCMD_SHOUT },
   { "show"     , "show"	, POS_DEAD    , do_show     , LVL_LORD, 0 },
   { "shutdow"  , "shutdow"	, POS_DEAD    , do_shutdown , LVL_SUBIMPL, 0 },
   { "shutdown" , "shutdown"	, POS_DEAD    , do_shutdown , LVL_IMPL, SCMD_SHUTDOWN },
   { "sip"      , "sip"		, POS_RESTING , do_drink    , 0, SCMD_SIP },
   { "sit"      , "sit"		, POS_RESTING , do_sit      , 0, 0 },
   { "skills"   , "sk"		, POS_DEAD    , do_skill    , 0, 0 },
   { "skillset" , "skillset"	, POS_SLEEPING, do_skillset , LVL_SUPGOD, 0 },
  // ( "slang"    , "slang"       , POS_FIGHTING, do_slang    , 1, 0 },
   { "slast"    , "slas"	, POS_DEAD    , do_slast    , LVL_SUBIMPL, 0 },
   { "sleep"    , "sleep"	, POS_SLEEPING, do_sleep    , 0, 0 },
   { "slist"    , "sli"		, POS_DEAD    , do_slist    , 0, 0 },
   { "slowns"   , "slowns"	, POS_DEAD    , do_gen_tog  , LVL_IMPL, SCMD_SLOWNS },
   { "sneak"    , "sn"		, POS_STANDING, do_sneak    , 1, 0 },
   { "snoop"    , "snoop"	, POS_DEAD    , do_snoop    , LVL_LORD, 0 },
   { "socials"  , "soc"		, POS_DEAD    , do_commands , 0, SCMD_SOCIALS },
   { "spells"   , "sp"		, POS_DEAD    , do_spell    , 0, 0 },
   { "split"    , "split"	, POS_SITTING , do_split    , 1, 0 },
   { "sps"      , "sps"	        , POS_DEAD    , do_sps      , LVL_SUBIMPL, 0 },
   { "stand"    , "st"		, POS_RESTING , do_stand    , 0, 0 },
   { "stat"     , "stat"	, POS_DEAD    , do_stat     , LVL_ELDER, 0 },
   { "steal"    , "ste"		, POS_STANDING, do_steal    , 1, 0 },
   { "style"    , "sty"		, POS_STANDING, do_style    , 1, 0 },
   { "stroke"   , "stro"	, POS_FIGHTING, do_stroke   , 1, 0 },
   { "summary"  , "sum"		, POS_DEAD    , do_summary  , 1, 0 },
   { "switch"   , "swit"	, POS_DEAD    , do_switch   , LVL_ELDER, 0 },
   { "swho"     , "swh"	        , POS_DEAD    , do_swho     , LVL_SUBIMPL, 0 },
   { "syslog"   , "sys"		, POS_DEAD    , do_syslog   , LVL_ELDER, 0 },

   { "tedit"    , "ted"		, POS_DEAD    , do_tedit    , LVL_GOD, 0 },
   { "tell"     , "t"		, POS_DEAD    , do_tell     , 0, 0 },
   { "take"     , "take"	, POS_RESTING , do_get      , 0, 0 },
   { "taste"    , "taste"	, POS_RESTING , do_eat      , 0, SCMD_TASTE },
   { "teleport" , "tele"	, POS_DEAD    , do_teleport , LVL_GOD, 0 },
   { "thaw"     , "thaw"	, POS_DEAD    , do_wizutil  , LVL_GOD, SCMD_THAW },
   { "title"    , "ti"		, POS_DEAD    , do_title    , 0, 0 },
   { "time"     , "time"	, POS_DEAD    , do_time     , 0, 0 },
   { "toggle"   , "tog"		, POS_DEAD    , do_toggle   , 0, 0 },
   { "track"    , "tr"		, POS_STANDING, do_track    , 0, 0 },
   { "transfer" , "trans"	, POS_SLEEPING, do_trans    , LVL_GOD, 0 },
   { "transform", "transform"   , POS_STANDING, do_transform, 0, 0 },
   { "trigedit" , "trig"	, POS_DEAD    , do_olc      , LVL_BUILDER, SCMD_OLC_TRIGEDIT},
   { "typo"     , "typo"	, POS_DEAD    , do_gen_write, 10, SCMD_TYPO },
   { "throw"    , "throw"       , POS_SLEEPING, do_throw     , 0, 0 },
   { "topten"   , "topten"      ,  POS_DEAD    , do_topten   , 0, 0},

   { "ukill"    , "ukil"        , POS_DEAD    , do_ukill    , LVL_SUBIMPL, 0 },
   { "unlock"   , "un"		, POS_SITTING , do_gen_door , 0, SCMD_UNLOCK },
   { "unban"    , "unban"       , POS_DEAD    , do_unban    , LVL_SUPGOD, 0 },
   { "ungroup"  , "ungr"	, POS_DEAD    , do_ungroup  , 0, 0 },
   { "unaffect" , "unaff"	, POS_DEAD    , do_wizutil  , LVL_GOD, SCMD_UNAFFECT },
   { "unmount"  ,  "unm" 	, POS_RESTING , do_not_here , 0, 0 },
   { "uptime"   , "upt"		, POS_DEAD    , do_date     , LVL_ELDER, SCMD_UPTIME },
   { "use"      , "use"		, POS_SITTING , do_use      , 1, SCMD_USE },
   { "users"    , "users"	, POS_DEAD    , do_users    , LVL_ELDER, 0 },

   { "value"    , "val"		, POS_STANDING, do_not_here , 0, 0 },
   { "version"  , "version"	, POS_DEAD    , do_gen_ps   , 0, SCMD_VERSION },
   { "visible"  , "vis"		, POS_RESTING , do_visible  , 1, 0 },
   { "vlist"    , "vl" 		, POS_DEAD    , do_liblist  , LVL_BUILDER, SCMD_VLIST },
   { "vnum"     , "vnum"	, POS_DEAD    , do_vnum     , LVL_BUILDER, 0 },
   { "vstat"    , "vstat"	, POS_DEAD    , do_vstat    , LVL_BUILDER, 0 },
   { "vwear"    , "vwear"	, POS_DEAD    , do_veq      , LVL_BUILDER, 0 },

   { "wanted"	, "wan"		, POS_DEAD    , do_wanted   , 0, 0 }, //comentar aqui
   { "war"	, "war"		, POS_STANDING, do_arena    , 0, 0 },
   { "wardome"  , "wardome"     , POS_STANDING, do_not_here , 1, 0 },
   { "warrior"   , "warrior"       , POS_FIGHTING, do_tempus,0,0 },
   { "warwho"	, "warw"	, POS_DEAD    , do_awho     , 0, 0 },
   { "warshout" , "wars"	, POS_FIGHTING, do_warshout , 0, 0 },
   { "wake"     , "wa"		, POS_SLEEPING, do_wake     , 0, 0 },
   { "wdprotocol","wdprot"	, POS_DEAD    , do_gen_tog  , 0, SCMD_WDPROTOCOL },
   { "wear"     , "wea"		, POS_RESTING , do_wear     , 0, 0 },
   { "weather"  , "weather"	, POS_RESTING , do_weather  , 0, 0 },
   { "who"      , "wh"		, POS_DEAD    , do_who      , 0, 0 },
   { "whois"    , "whois"       , POS_DEAD    , do_whois    , 0, 0 },
   { "where"    , "where"	, POS_RESTING , do_where    , 1, 0 },
   { "whisper"  , "whisp"	, POS_RESTING , do_spec_comm, 10, SCMD_WHISPER },
   { "whoami"   , "whoami"      , POS_DEAD    , do_gen_ps   , 0, SCMD_WHOAMI },
   { "wield"    , "wi"		, POS_RESTING , do_wield    , 0, 0 },
   { "wimpy"    , "wimp"	, POS_DEAD    , do_wimpy    , 0, 0 },
   { "withdraw" , "withdraw"	, POS_STANDING, do_not_here , 1, 0 },
   { "wiznet"   , "wiz"		, POS_DEAD    , do_wiznet   , LVL_ELDER, 0 },
   { ";"        , "wiz"		, POS_DEAD    , do_wiznet   , LVL_ELDER, 0 },
   { "wizhelp"  , "wizhelp"	, POS_SLEEPING, do_commands , LVL_ELDER, SCMD_WIZHELP },
   { "wizlist"  , "wizlist"	, POS_DEAD    , do_gen_ps   , 0, SCMD_WIZLIST },
   { "wizlock"  , "wizlock"	, POS_DEAD    , do_wizlock  , LVL_MJGOD, 0 },
   { "write"    , "wr"		, POS_STANDING, do_write    , 5, 0 },
   { "whirlwind", "whirl"	, POS_FIGHTING, do_whirlwind, 1, 0 },

   { "xname"    , "xn"		, POS_DEAD    , do_xname    , LVL_SUPGOD, 0 },

   { "zedit"    , "zed"		, POS_DEAD    , do_olc      , LVL_BUILDER, SCMD_OLC_ZEDIT},
   { "zlist"    , "zl"		, POS_DEAD    , do_liblist  , LVL_BUILDER, SCMD_ZLIST },
   { "zreset"   , "zreset"	, POS_DEAD    , do_zreset   , LVL_BUILDER, 0 },


   /* DG trigger commands */
//   { "attach"   , "attach"	, POS_DEAD    , do_attach   , LVL_IMPL, 0 },
//   { "detach"   , "detach"	, POS_DEAD    , do_detach   , LVL_IMPL, 0 },
   { "tlist"    , "tlist"	, POS_DEAD    , do_tlist    , LVL_BUILDER, 0 },
   { "tstat"    , "tstat"	, POS_DEAD    , do_tstat    , LVL_BUILDER, 0 },
   { "masound"  , "masound"	, POS_DEAD    , do_masound  , 0, 0 },
   { "mkill"    , "mkill"	, POS_STANDING, do_mkill    , 0, 0 },
   { "mjunk"    , "mjunk"	, POS_SITTING , do_mjunk    , 0, 0 },
   { "mecho"    , "mecho"	, POS_DEAD    , do_mecho    , 0, 0 },
   { "mechoaround","mechoaround", POS_DEAD    , do_mechoaround, 0, 0 },
   { "msend"    , "msend"	, POS_DEAD    , do_msend    , 0, 0 },
   { "mload"    , "mload"	, POS_DEAD    , do_mload    , 0, 0 },
   { "mpurge"   , "mpurge"	, POS_DEAD    , do_mpurge    , 0, 0 },
   { "mgoto"    , "mgoto"	, POS_DEAD    , do_mgoto    , 0, 0 },
   { "mat"      , "mat"		, POS_DEAD    , do_mat      , 0, 0 },
   { "mteleport", "mteleport"	, POS_DEAD    , do_mteleport, 0, 0 },
   { "mforce"   , "mforce"	, POS_DEAD    , do_mforce   , 0, 0 },
   { "mexp"     , "mexp"	, POS_DEAD    , do_mexp     , 0, 0 },
   { "mgold"    , "mgold"	, POS_DEAD    , do_mgold    , 0, 0 },
   { "mhunt"    , "mhunt"	, POS_DEAD    , do_mhunt    , 0, 0 },
   { "mremember", "mremember"	, POS_DEAD    , do_mremember, 0, 0 },
   { "mforget"  , "mforget"	, POS_DEAD    , do_mforget  , 0, 0 },
   { "mtransform","mtransform"	, POS_DEAD    , do_mtransform, 0, 0 },

   { "\n",	"zzzzzzz",	0, 0, 0, 0 } };	/* this must be last */

const char *fill[] =
{
  "in",
  "from",
  "with",
  "the",
  "on",
  "at",
  "to",
  "\n"
};

const char *reserved[] =
{
  "a",
  "an",
  "self",
  "me",
  "all",
  "room",
  "someone",
  "something",
  "\n"
};

/*
 * This is the actual command interpreter called from game_loop() in comm.c
 * It makes sure you are the proper level and position to execute the command,
 * then calls the appropriate function.
 */
void command_interpreter(struct char_data *ch, char *argument)
{
  int cmd, length;
  char *line;

  REMOVE_BIT(AFF_FLAGS(ch), AFF_HIDE);
  REMOVE_BIT(AFF_FLAGS(ch), AFF_GAS);
  REMOVE_BIT(PLR_FLAGS(ch), PLR_MEDITATE);

  /* just drop to next line for hitting CR */
  skip_spaces(&argument);
  if (!*argument)
    return;

//  processar o speedwalk aki
  for (cmd = 0; *complete_cmd_info[cmd].command != '\n'; cmd++){
     if((!IS_MOVE(cmd)) && !(PLR_FLAGGED(ch, PLR_FROZEN) && GET_LEVEL(ch) < LVL_IMPL) && (do_run(ch, argument))){
         return;
     }
  }

  /*
   * special case to handle one-character, non-alphanumeric commands;
   * requested by many people so "'hi" or ";godnet test" is possible.
   * Patch sent by Eric Green and Stefan Wasilewski.
   */
  if (!isalpha(*argument)) {
    arg[0] = argument[0];
    arg[1] = '\0';
    line = argument + 1;
  } else
    line = any_one_arg(argument, arg);

  if ((GET_LEVEL(ch)<LVL_IMMORT) &&
      (command_wtrigger(ch, arg, line) ||
       command_mtrigger(ch, arg, line) ||
       command_otrigger(ch, arg, line)))
    return; /* command trigger took over */

  /* otherwise, find the command */
  for (length = strlen(arg), cmd = 0; *complete_cmd_info[cmd].command != '\n'; cmd++)
    if (!strncmp(complete_cmd_info[cmd].command, arg, length))
      if (GET_LEVEL(ch) >= complete_cmd_info[cmd].minimum_level){
        /* New crash test system -Petrus*/
        sprintf(buf, "(LAST COMMAND) %s: \"%s\" - (%d) - %s -\r\n", GET_NAME(ch), argument,
	        GET_ROOM_VNUM(ch->in_room), world[ch->in_room].name);
        write_last_command(buf);
        break;
      }
  if (*complete_cmd_info[cmd].command == '\n') {
    switch (number(1,14)) {
      case  1: send_to_char("Huh?!?\r\n", ch); break;
      case  2: send_to_char("I can't understand you...\r\n", ch); break;
      case  3: send_to_char("What are you trying to do?\r\n", ch); break;
      case  4: send_to_char("Are you drunk?\r\n", ch); break;
      case  5: send_to_char("Your keyboard must be broken...\r\n", ch); break;
      case  6: send_to_char("Que?!?\r\n", ch); break;
      case  7: send_to_char("What are you trying to do?!?\r\n", ch); break;
      case  8: send_to_char("Try typing HELP to see the most common commands.\r\n", ch); break;
      case  9: send_to_char("Ha?!?\r\n", ch); break;
      case 10: send_to_char("ZzZzZzZz...\r\n", ch); break;
      case 11: send_to_char("Type COMMANDS to see the commands avaiable.\r\n", ch); break;
      case 12: send_to_char("Are you playing with the keyboard?\r\n", ch); break;
      case 13: send_to_char("Verify your mind and try again...\r\n", ch); break;
      case 14: send_to_char("Don't you know how to command these shit? Type HELP.\r\n", ch); break;
    }
  }

  else if (PLR_FLAGGED(ch, PLR_FROZEN) && GET_LEVEL(ch) < LVL_IMPL)
    send_to_char("You try, but the mind-numbing cold prevents you...\r\n", ch);
//  else if (PRF_FLAGGED(ch, PRF_AFK))
//    send_to_char("Try turning your AFK flag off first.\r\n", ch);
  else if (complete_cmd_info[cmd].command_pointer == NULL)
    send_to_char("Sorry, that command hasn't been implemented yet.\r\n", ch);
  else if (IS_NPC(ch) && complete_cmd_info[cmd].minimum_level >= LVL_IMMORT)
    send_to_char("You can't use immortal commands while switched.\r\n", ch);
  else if (GET_POS(ch) < complete_cmd_info[cmd].minimum_position)
    switch (GET_POS(ch)) {
    case POS_DEAD:
      send_to_char("Lie still; you are DEAD!!! :-(\r\n", ch);
      break;
    case POS_INCAP:
    case POS_MORTALLYW:
      send_to_char("You are in a pretty bad shape, unable to do anything!\r\n", ch);
      break;
    case POS_STUNNED:
      send_to_char("All you can do right now is think about the stars!\r\n", ch);
      break;
    case POS_SLEEPING:
      send_to_char("In your dreams, or what?\r\n", ch);
      break;
    case POS_RESTING:
      send_to_char("Nah... You feel too relaxed to do that..\r\n", ch);
      break;
    case POS_SITTING:
      send_to_char("Maybe you should get on your feet first?\r\n", ch);
      break;
    case POS_FIGHTING:
      send_to_char("No way!  You're fighting for your life!\r\n", ch);
      break;
  } else if (no_specials || !special(ch, cmd, line))
    ((*complete_cmd_info[cmd].command_pointer) (ch, line, cmd, complete_cmd_info[cmd].subcmd));

}

/**************************************************************************
 * Routines to handle aliasing                                             *
  **************************************************************************/


struct alias *find_alias(struct alias *alias_list, char *str)
{
  while (alias_list != NULL) {
    if (*str == *alias_list->alias)     /* hey, every little bit counts :-) */
      if (!strcmp(str, alias_list->alias))
        return alias_list;

    alias_list = alias_list->next;
  }

  return NULL;
}


void free_alias(struct alias *a)
{
  if (a->alias)
    free(a->alias);
  if (a->replacement)
    free(a->replacement);
  free(a);
}


/* The interface to the outside world: do_alias */
ACMD(do_alias)
{
  char *repl;
  struct alias *a, *temp;

  if (IS_NPC(ch))
    return;

  repl = any_one_arg(argument, arg);

  if (!*arg) {                  /* no argument specified -- list currently defined aliases */
    send_to_char("Currently defined aliases:\r\n", ch);
    if ((a = GET_ALIASES(ch)) == NULL)
      send_to_char(" None.\r\n", ch);
    else {
      while (a != NULL) {
        sprintf(buf, "%15s: %s\r\n", a->alias, a->replacement);
        send_to_char(buf, ch);
        a = a->next;
      }
    }
  } else {                      /* otherwise, add or remove aliases */
    /* is this an alias we've already defined? */
    if ((a = find_alias(GET_ALIASES(ch), arg)) != NULL) {
      REMOVE_FROM_LIST(a, GET_ALIASES(ch), next);
      free_alias(a);
    }
    /* if no replacement string is specified, assume we want to delete */
    if (!*repl) {
      if (a == NULL)
        send_to_char("No such alias.\r\n", ch);
      else
        send_to_char("Alias deleted.\r\n", ch);
    } else {                    /* otherwise, either add or redefine an alias */
      if (!str_cmp(arg, "alias")) {
        send_to_char("You can't alias 'alias'.\r\n", ch);
        return;
      }
      CREATE(a, struct alias, 1);
      a->alias = str_dup(arg);
      delete_doubledollar(repl);
      a->replacement = str_dup(repl);
      if (strchr(repl, ALIAS_SEP_CHAR) || strchr(repl, ALIAS_VAR_CHAR))
        a->type = ALIAS_COMPLEX;
      else
        a->type = ALIAS_SIMPLE;
      a->next = GET_ALIASES(ch);
      GET_ALIASES(ch) = a;
      send_to_char("Alias added.\r\n", ch);
    }
  }
}

/*
 * Valid numeric replacements are only $1 .. $9 (makes parsing a little
 * easier, and it's not that much of a limitation anyway.)  Also valid
 * is "$*", which stands for the entire original line after the alias.
 * ";" is used to delimit commands.
 */
#define NUM_TOKENS       9

void perform_complex_alias(struct txt_q *input_q, char *orig, struct alias *a)
{
  struct txt_q temp_queue;
  char *tokens[NUM_TOKENS], *temp, *write_point;
  int num_of_tokens = 0, num;

  /* First, parse the original string */
  temp = strtok(strcpy(buf2, orig), " ");
  while (temp != NULL && num_of_tokens < NUM_TOKENS) {
    tokens[num_of_tokens++] = temp;
    temp = strtok(NULL, " ");
  }

  /* initialize */
  write_point = buf;
  temp_queue.head = temp_queue.tail = NULL;

  /* now parse the alias */
  for (temp = a->replacement; *temp; temp++) {
    if (*temp == ALIAS_SEP_CHAR) {
      *write_point = '\0';
      buf[MAX_INPUT_LENGTH - 1] = '\0';
      write_to_q(buf, &temp_queue, 1);
      write_point = buf;
    } else if (*temp == ALIAS_VAR_CHAR) {
      temp++;
      if ((num = *temp - '1') < num_of_tokens && num >= 0) {
        strcpy(write_point, tokens[num]);
        write_point += strlen(tokens[num]);
      } else if (*temp == ALIAS_GLOB_CHAR) {
        strcpy(write_point, orig);
        write_point += strlen(orig);
      } else if ((*(write_point++) = *temp) == '$')     /* redouble $ for act safety */
        *(write_point++) = '$';
    } else
      *(write_point++) = *temp;
  }

  *write_point = '\0';
  buf[MAX_INPUT_LENGTH - 1] = '\0';
  write_to_q(buf, &temp_queue, 1);

  /* push our temp_queue on to the _front_ of the input queue */
  if (input_q->head == NULL)
    *input_q = temp_queue;
  else {
    temp_queue.tail->next = input_q->head;
    input_q->head = temp_queue.head;
  }
}


/*
 * Given a character and a string, perform alias replacement on it.
 *
 * Return values:
 *   0: String was modified in place; call command_interpreter immediately.
 *   1: String was _not_ modified in place; rather, the expanded aliases
 *      have been placed at the front of the character's input queue.
 */
int perform_alias(struct descriptor_data *d, char *orig)
{
  char first_arg[MAX_INPUT_LENGTH], *ptr;
  struct alias *a, *tmp;

  /* Mobs don't have alaises. */
  if (IS_NPC(d->character))
    return 0;

  /* bail out immediately if the guy doesn't have any aliases */
  if ((tmp = GET_ALIASES(d->character)) == NULL)
    return 0;

  /* find the alias we're supposed to match */
  ptr = any_one_arg(orig, first_arg);

  /* bail out if it's null */
  if (!*first_arg)
    return 0;

  /* if the first arg is not an alias, return without doing anything */
  if ((a = find_alias(tmp, first_arg)) == NULL)
    return 0;

  if (a->type == ALIAS_SIMPLE) {
    strcpy(orig, a->replacement);
    return 0;
  } else {
    perform_complex_alias(&d->input, ptr, a);
    return 1;
  }
}



/***************************************************************************
 * Various other parsing utilities                                         *
 **************************************************************************/

/*
 * searches an array of strings for a target string.  "exact" can be
 * 0 or non-0, depending on whether or not the match must be exact for
 * it to be returned.  Returns -1 if not found; 0..n otherwise.  Array
 * must be terminated with a '\n' so it knows to stop searching.
 */
int search_block(char *arg, const char **list, int exact)
{
  int i, l;

  /* Make into lower case, and get length of string */
  for (l = 0; *(arg + l); l++)
    *(arg + l) = LOWER(*(arg + l));

  if (exact) {
    for (i = 0; **(list + i) != '\n'; i++)
      if (!strcmp(arg, *(list + i)))
        return (i);
  } else {
    if (!l)
      l = 1;                    /* Avoid "" to match the first available
                                 * string */
    for (i = 0; **(list + i) != '\n'; i++)
      if (!strncmp(arg, *(list + i), l))
        return (i);
  }

  return -1;
}


/*
 * This function does the same thing as search_block, but it
 * uses strcasecmp.    */
int search_block_case(char *arg, char **list, bool exact)
{
  int i, l;

  /* Make into lower case, and get length of string */
  for (l = 0; *(arg + l); l++)
    *(arg + l) = LOWER(*(arg + l));

  if (exact) {
    for (i = 0; **(list + i) != '\n'; i++)
      if (!strcasecmp(arg, *(list + i)))
	return (i);
  } else {
    if (!l)
      l = 1;			/* Avoid "" to match the first available
				 * string */
    for (i = 0; **(list + i) != '\n'; i++)
      if (!strncasecmp(arg, *(list + i), l))
	return (i);
  }

  return -1;
}


int is_number(const char *str)
{
  if (!str || !*str)	/* Test for NULL pointer or string. */
    return FALSE;

  if (*str == '-')	/* -'s in front are valid. */
    str++;

  while (*str)
    if (!isdigit(*(str++)))
      return FALSE;

  return TRUE;
}

/*
 * Function to skip over the leading spaces of a string.
 */
void skip_spaces(char **string)
{
  for (; **string && isspace(**string); (*string)++);
}


/*
 * Given a string, change all instances of double dollar signs ($$) to
 * single dollar signs ($).  When strings come in, all $'s are changed
 * to $$'s to avoid having users be able to crash the system if the
 * inputted string is eventually sent to act().  If you are using user
 * input to produce screen output AND YOU ARE SURE IT WILL NOT BE SENT
 * THROUGH THE act() FUNCTION (i.e., do_gecho, do_title, but NOT do_say),
 * you can call delete_doubledollar() to make the output look correct.
 *
 * Modifies the string in-place.
 */
char *delete_doubledollar(char *string)
{
  char *read, *write;

  /* If the string has no dollar signs, return immediately */
  if ((write = strchr(string, '$')) == NULL)
    return string;

  /* Start from the location of the first dollar sign */
  read = write;


  while (*read)   /* Until we reach the end of the string... */
    if ((*(write++) = *(read++)) == '$') /* copy one char */
      if (*read == '$')
        read++; /* skip if we saw 2 $'s in a row */

  *write = '\0';

  return string;
}


int fill_word(char *argument)
{
  return (search_block(argument, fill, TRUE) >= 0);
}


int reserved_word(char *argument)
{
  return (search_block(argument, reserved, TRUE) >= 0);
}


/*
 * copy the first non-fill-word, space-delimited argument of 'argument'
 * to 'first_arg'; return a pointer to the remainder of the string.
 */
char *one_argument(char *argument, char *first_arg)
{
  char *begin = first_arg;

  if (!argument) {
    log("SYSERR: one_argument received a NULL pointer!");
    *first_arg = '\0';
    return NULL;
  }

  do {
    skip_spaces(&argument);

    first_arg = begin;
    while (*argument && !isspace(*argument)) {
      *(first_arg++) = LOWER(*argument);
      argument++;
    }

    *first_arg = '\0';
  } while (fill_word(begin));

  return argument;
}


/*
 * one_word is like one_argument, except that words in quotes ("") are
 * considered one word.
 */
char *one_word(char *argument, char *first_arg)
{
  char *begin = first_arg;

  do {
    skip_spaces(&argument);

    first_arg = begin;

    if (*argument == '\"') {
      argument++;
      while (*argument && *argument != '\"') {
        *(first_arg++) = LOWER(*argument);
        argument++;
      }
      argument++;
    } else {
      while (*argument && !isspace(*argument)) {
        *(first_arg++) = LOWER(*argument);
        argument++;
      }
    }

    *first_arg = '\0';
  } while (fill_word(begin));

  return argument;
}


/* same as one_argument except that it doesn't ignore fill words */
char *any_one_arg(char *argument, char *first_arg)
{
  skip_spaces(&argument);

  while (*argument && !isspace(*argument)) {
    *(first_arg++) = LOWER(*argument);
    argument++;
  }

  *first_arg = '\0';

  return argument;
}


/*
 * Same as one_argument except that it takes two args and returns the rest;
 * ignores fill words
 */
char *two_arguments(char *argument, char *first_arg, char *second_arg)
{
  return one_argument(one_argument(argument, first_arg), second_arg); /* :-) */
}



/*
 * determine if a given string is an abbreviation of another
 * (now works symmetrically -- JE 7/25/94)
 *
 * that was dumb.  it shouldn't be symmetrical.  JE 5/1/95
 *
 * returnss 1 if arg1 is an abbreviation of arg2
 */
int is_abbrev(const char *arg1, const char *arg2)
{
  if (!*arg1)
    return 0;

  for (; *arg1 && *arg2; arg1++, arg2++)
    if (LOWER(*arg1) != LOWER(*arg2))
      return 0;

  if (!*arg1)
    return 1;
  else
    return 0;
}



/* return first space-delimited token in arg1; remainder of string in arg2 */
void half_chop(char *string, char *arg1, char *arg2)
{
  char *temp;

  temp = any_one_arg(string, arg1);
  skip_spaces(&temp);
  strcpy(arg2, temp);
}



/* Used in specprocs, mostly.  (Exactly) matches "command" to cmd number */
int find_command(const char *command)
{
  int cmd;

  for (cmd = 0; *complete_cmd_info[cmd].command != '\n'; cmd++)
    if (!strcmp(complete_cmd_info[cmd].command, command))
      return cmd;

  return -1;
}


int special(struct char_data *ch, int cmd, char *arg)
{
  struct obj_data *i;
  struct char_data *k;
  int j;

  /* special in room? */
  if (GET_ROOM_SPEC(ch->in_room) != NULL)
    if (GET_ROOM_SPEC(ch->in_room) (ch, world + ch->in_room, cmd, arg))
      return 1;

  /* special in equipment list? */
  for (j = 0; j < NUM_WEARS; j++)
    if (GET_EQ(ch, j) && GET_OBJ_SPEC(GET_EQ(ch, j)) != NULL)
      if (GET_OBJ_SPEC(GET_EQ(ch, j)) (ch, GET_EQ(ch, j), cmd, arg))
        return 1;

  /* special in inventory? */
  for (i = ch->carrying; i; i = i->next_content)
    if (GET_OBJ_SPEC(i) != NULL)
      if (GET_OBJ_SPEC(i) (ch, i, cmd, arg))
        return 1;

  /* special in mobile present? */
  for (k = world[ch->in_room].people; k; k = k->next_in_room)
    if (GET_MOB_SPEC(k) != NULL)
      if (GET_MOB_SPEC(k) (ch, k, cmd, arg))
        return 1;

  /* special in object present? */
  for (i = world[ch->in_room].contents; i; i = i->next_content)
    if (GET_OBJ_SPEC(i) != NULL)
      if (GET_OBJ_SPEC(i) (ch, i, cmd, arg))
        return 1;

  return 0;
}



/* *************************************************************************
*  Stuff for controlling the non-playing sockets (get name, pwd etc)       *
************************************************************************* */


/* locate entry in p_table with entry->name == name. -1 mrks failed search */
int find_name(char *name)
{
  int i;

  for (i = 0; i <= top_of_p_table; i++) {
    if (!str_cmp((player_table + i)->name, name))
      return i;
  }

  return -1;
}


int _parse_name(char *arg, char *name)
{
  int i;

  /* skip whitespaces */
  for (; isspace(*arg); arg++);

  for (i = 0; (*name = *arg); arg++, i++, name++)
    if (!isalpha(*arg))
      return 1;

  if (!i)
    return 1;

  return 0;
}


#define RECON           1
#define USURP           2
#define UNSWITCH        3

/*
 * XXX: Make immortals 'return' instead of being disconnected when switched
 *      into person returns.  This function seems a bit over-extended too.
 */
int perform_dupe_check(struct descriptor_data *d)
{
  struct descriptor_data *k, *next_k;
  struct char_data *target = NULL, *ch, *next_ch;
  int mode = 0;

  int id = GET_IDNUM(d->character);

  /*
   * Now that this descriptor has successfully logged in, disconnect all
   * other descriptors controlling a character with the same ID number.
   */

  for (k = descriptor_list; k; k = next_k) {
    next_k = k->next;

    if (k == d)
      continue;

    if (k->original && (GET_IDNUM(k->original) == id)) {    /* switched char */
      SEND_TO_Q("\r\nMultiple login detected -- disconnecting.\r\n", k);
      STATE(k) = CON_CLOSE;
      if (!target) {
        target = k->original;
        mode = UNSWITCH;
      }
      if (k->character)
        k->character->desc = NULL;
      k->character = NULL;
      k->original = NULL;
    } else if (k->character && (GET_IDNUM(k->character) == id)) {
      if (!target && STATE(k) == CON_PLAYING) {
        SEND_TO_Q("\r\nThis body has been usurped!\r\n", k);
        target = k->character;
        mode = USURP;
      }
      k->character->desc = NULL;
      k->character = NULL;
      k->original = NULL;
      SEND_TO_Q("\r\nMultiple login detected -- disconnecting.\r\n", k);
      STATE(k) = CON_CLOSE;
    }
  }

 /*
  * now, go through the character list, deleting all characters that
  * are not already marked for deletion from the above step (i.e., in the
  * CON_HANGUP state), and have not already been selected as a target for
  * switching into.  In addition, if we haven't already found a target,
  * choose one if one is available (while still deleting the other
  * duplicates, though theoretically none should be able to exist).
  */

  for (ch = character_list; ch; ch = next_ch) {
    next_ch = ch->next;

    if (IS_NPC(ch))
      continue;
    if (GET_IDNUM(ch) != id)
      continue;

    /* ignore chars with descriptors (already handled by above step) */
    if (ch->desc)
      continue;

    /* don't extract the target char we've found one already */
    if (ch == target)
      continue;

    /* we don't already have a target and found a candidate for switching */
    if (!target) {
      target = ch;
      mode = RECON;
      continue;
    }

    /* we've found a duplicate - blow him away, dumping his eq in limbo. */
    if (ch->in_room != NOWHERE)
      char_from_room(ch);
    char_to_room(ch, 1);
    extract_char(ch);
  }

  /* no target for swicthing into was found - allow login to continue */
  if (!target)
    return 0;

  /* Okay, we've found a target.  Connect d to target. */
  free_char(d->character); /* get rid of the old char */
  d->character = target;
  d->character->desc = d;
  d->original = NULL;
  d->character->char_specials.timer = 0;
  REMOVE_BIT(PLR_FLAGS(d->character), PLR_MAILING | PLR_WRITING);
  STATE(d) = CON_PLAYING;

  switch (mode) {
  case RECON:
    SEND_TO_Q("Reconnecting.\r\n", d);
    act("$n has reconnected.", TRUE, d->character, 0, 0, TO_ROOM);
    sprintf(buf, "%s [%s] has reconnected.", GET_NAME(d->character), d->host);
    mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
    break;
  case USURP:
    SEND_TO_Q("You take over your own body, already in use!\r\n", d);
    act("$n suddenly keels over in pain, surrounded by a white aura...\r\n"
        "$n's body has been taken over by a new spirit!",
        TRUE, d->character, 0, 0, TO_ROOM);
    sprintf(buf, "%s has re-logged in ... disconnecting old socket.",
            GET_NAME(d->character));
    mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
    break;
  case UNSWITCH:
    SEND_TO_Q("Reconnecting to unswitched char.", d);
    sprintf(buf, "%s [%s] has reconnected.", GET_NAME(d->character), d->host);
    mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
    break;
  }

  return 1;
}

sh_int race_class[][NUM_CLASSES] = {
//  SO  CL  TH  WA  NE  PA  NI  RA  PS  WR  BA  SH
   { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, //HUMA
   { 1,  1,  1,  0,  1,  0,  0,  1,  1,  1,  0,  0 }, //DROW
   { 1,  1,  0,  1,  0,  0,  0,  0,  0,  0,  1,  0 }, //DWAR
   { 1,  1,  1,  1,  0,  0,  0,  1,  1,  0,  0,  1 }, //ELF
   { 0,  0,  0,  1,  0,  0,  0,  0,  0,  1,  1,  1 }, //OGRE
   { 0,  0,  0,  1,  1,  0,  0,  0,  0,  0,  1,  1 }, //ORC
   { 1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  1,  1 }, //TROL
   { 1,  1,  0,  0,  1,  0,  1,  0,  1,  1,  0,  0 }, //GITH
   { 0,  1,  1,  0,  0,  0,  0,  1,  1,  0,  0,  0 }, //GNOM
   { 0,  0,  1,  1,  0,  0,  0,  0,  0,  0,  1,  1 }, //LIZA
   { 0,  1,  1,  0,  0,  0,  1,  0,  0,  1,  0,  0 }, //SEA
   { 0,  0,  1,  1,  1,  0,  1,  0,  0,  0,  0,  0 }, //GORA
   { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, //DUNE
   { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, //ANCI
   { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, //Nau
   { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, //Hig
   { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, //Hil
   { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, //Bug
   { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, //Cav
   { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, //Lic
   { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, //Tin
   { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, //Dra
   { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }, //Hse
   { 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1 }  //Arc
};

int allowed_classes(int x, int i)
{
	if(race_class[x][i])
    	  	return (TRUE);

    return (FALSE);
}

/* load the player, put them in the right room - used by copyover_recover too */
int enter_player_game (struct descriptor_data *d)
{
    extern sh_int r_mortal_start_room;
    extern sh_int r_immort_start_room;
    extern sh_int r_frozen_start_room;
    int i;

    sh_int load_room;
    int load_result;

    reset_char(d->character);

    if (PLR_FLAGGED(d->character, PLR_INVSTART))
        GET_INVIS_LEV(d->character) = GET_LEVEL(d->character);

    if ((load_result = Crash_load(d->character)))
        d->character->in_room = NOWHERE;

    read_aliases(d->character);
    GET_ID(d->character) = GET_IDNUM(d->character);
    save_char(d->character, NOWHERE);

    d->character->next = character_list;
    character_list = d->character;

      load_room = GET_LOADROOM(d->character);

      for(i=0;i<NUM_ROOMS;i++)
       if (load_room == to_fly_rooms[i]){
        load_room = NOWHERE;
       }

      if (load_room == NOWHERE) {
        if (GET_LEVEL(d->character) >= LVL_ELDER) {
          load_room = r_immort_start_room;
        } else {
          load_room = r_mortal_start_room;
        }
      } else {
        load_room = real_room(load_room);
      }

    if (PLR_FLAGGED(d->character, PLR_FROZEN))
        load_room = r_frozen_start_room;

    if (PLR_FLAGGED(d->character, PLR_DEAD))
        load_room = r_deadly_start_room;

    char_to_room(d->character, load_room);

    return load_result;
}


/* deal with newcomers and other non-playing sockets */
void nanny(struct descriptor_data *d, char *arg)
{
  char buf[128];
  int player_i, load_result;
  char tmp_name[MAX_INPUT_LENGTH];

  skip_spaces(&arg);

  switch (STATE(d)) {

  /*. OLC states .*/
  case CON_HEDIT:
    hedit_parse(d, arg);
    break;
  case CON_OEDIT:
    oedit_parse(d, arg);
    break;
  case CON_REDIT:
    redit_parse(d, arg);
    break;
  case CON_ZEDIT:
    zedit_parse(d, arg);
    break;
  case CON_MEDIT:
    medit_parse(d, arg);
    break;
  case CON_SEDIT:
    sedit_parse(d, arg);
    break;
  case CON_AEDIT:
    aedit_parse(d, arg);
    break;
  case CON_TRIGEDIT:
    trigedit_parse(d, arg);
    break;
  case CON_QEDIT:
    qedit_parse(d, arg);
    break;
  /*. End of OLC states .*/

  case CON_GET_NAME:            /* wait for input of name */
    if (d->character == NULL) {
      CREATE(d->character, struct char_data, 1);
      clear_char(d->character);
      CREATE(d->character->player_specials, struct player_special_data, 1);
      d->character->desc = d;
    }
    if (!*arg)
      STATE(d) = CON_CLOSE;
    else {
      if ((_parse_name(arg, tmp_name)) || strlen(tmp_name) < 2 ||
          strlen(tmp_name) > MAX_NAME_LENGTH || !Valid_Name(tmp_name) ||
          fill_word(strcpy(buf, tmp_name)) || reserved_word(buf)) {
        SEND_TO_Q("Invalid name, please try another.\r\n"
                  "Name: ", d);
        return;
      }
      if ((player_i = load_char(tmp_name, d->character)) > -1) {
        GET_PFILEPOS(d->character) = player_i;

        if (PLR_FLAGGED(d->character, PLR_DELETED)) {

	  /* make sure old files are removed so the new player doesn't get
	     the deleted player's equipment (this should probably be a
	     stock behavior)
	  */
	  if((player_i = find_name(tmp_name)) >= 0)
	    remove_player(player_i);

          /* We get a false positive from the original deleted character. */
          free_char(d->character);
          d->character = NULL;
          /* Check for multiple creations... */
          if (!Valid_Name(tmp_name)) {
            SEND_TO_Q("Invalid name, please try another.\r\nName: ", d);
            return;
          }
          CREATE(d->character, struct char_data, 1);
          clear_char(d->character);
          CREATE(d->character->player_specials, struct player_special_data, 1);
          d->character->desc = d;
          CREATE(d->character->player.name, char, strlen(tmp_name) + 1);
          strcpy(d->character->player.name, CAP(tmp_name));
          GET_PFILEPOS(d->character) = player_i;
          sprintf(buf, "Did I get that right, %s (Y/N)? ", tmp_name);
          SEND_TO_Q(buf, d);
          STATE(d) = CON_NAME_CNFRM;
        } else {
          /* undo it just in case they are set */
          REMOVE_BIT(PLR_FLAGS(d->character),
                     PLR_WRITING | PLR_MAILING | PLR_CRYO);

          SEND_TO_Q("Password: ", d);
          echo_off(d);
          d->idle_tics = 0;
          STATE(d) = CON_PASSWORD;
        }
      } else {
        /* player unknown -- make new character */

        /* Check for multiple creations of a character. */
        if (!Valid_Name(tmp_name)) {
          SEND_TO_Q("Invalid name, please try another.\r\nName: ", d);
          return;
        }
        CREATE(d->character->player.name, char, strlen(tmp_name) + 1);
        strcpy(d->character->player.name, CAP(tmp_name));

        sprintf(buf, "Did I get that right, %s (Y/N)? ", tmp_name);
        SEND_TO_Q(buf, d);
        STATE(d) = CON_NAME_CNFRM;
      }
    }
    break;
  case CON_NAME_CNFRM:          /* wait for conf. of new name    */
    if (UPPER(*arg) == 'Y') {
      if (isbanned(d->host) >= BAN_NEW) {
        sprintf(buf, "Request for new char %s denied from [%s] (siteban)",
                GET_NAME(d->character), d->host);
        mudlog(buf, NRM, LVL_GOD, TRUE);
        SEND_TO_Q("Sorry, new characters are not allowed from your site!\r\n", d);
        STATE(d) = CON_CLOSE;
        return;
      }
      if (circle_restrict) {
        SEND_TO_Q("Sorry, new players can't be created at the moment.\r\n", d);
        sprintf(buf, "Request for new char %s denied from [%s] (wizlock)",
                GET_NAME(d->character), d->host);
        mudlog(buf, NRM, LVL_GOD, TRUE);
        STATE(d) = CON_CLOSE;
        return;
      }
      SEND_TO_Q("New character.\r\n", d);
      sprintf(buf, "Give me a password for %s: ", GET_NAME(d->character));
      SEND_TO_Q(buf, d);
      echo_off(d);
      STATE(d) = CON_NEWPASSWD;
    } else if (*arg == 'n' || *arg == 'N') {
      SEND_TO_Q("Okay, what IS it, then? ", d);
      free(d->character->player.name);
      d->character->player.name = NULL;
      STATE(d) = CON_GET_NAME;
    } else {
      SEND_TO_Q("Please type Yes or No: ", d);
    }
    break;
  case CON_PASSWORD:            /* get pwd for known player      */
    /*
     * To really prevent duping correctly, the player's record should
     * be reloaded from disk at this point (after the password has been
     * typed).  However I'm afraid that trying to load a character over
     * an already loaded character is going to cause some problem down the
     * road that I can't see at the moment.  So to compensate, I'm going to
     * (1) add a 15 or 20-second time limit for entering a password, and (2)
     * re-add the code to cut off duplicates when a player quits.  JE 6 Feb 96
     */

    echo_on(d);    /* turn echo back on */

    if (!*arg)
      STATE(d) = CON_CLOSE;
    else {
      if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character), MAX_PWD_LENGTH)) {
        sprintf(buf, "Bad PW: %s [%s]", GET_NAME(d->character), d->host);
        mudlog(buf, BRF, LVL_GOD, TRUE);
        GET_BAD_PWS(d->character)++;
        save_char(d->character, NOWHERE);
        if (++(d->bad_pws) >= max_bad_pws) {    /* 3 strikes and you're out. */
          SEND_TO_Q("Wrong password... disconnecting.\r\n", d);
          STATE(d) = CON_CLOSE;
        } else {
          SEND_TO_Q("Wrong password.\r\nPassword: ", d);
          echo_off(d);
        }
        return;
      }

      /* Password was correct. */
      load_result = GET_BAD_PWS(d->character);
      GET_BAD_PWS(d->character) = 0;
      d->bad_pws = 0;

      if (isbanned(d->host) == BAN_SELECT &&
          !PLR_FLAGGED(d->character, PLR_SITEOK)) {
        SEND_TO_Q("Sorry, this char has not been cleared for login from your site!\r\n", d);
        STATE(d) = CON_CLOSE;
        sprintf(buf, "Connection attempt for %s denied from %s",
                GET_NAME(d->character), d->host);
        mudlog(buf, NRM, LVL_GOD, TRUE);
        return;
      }
      if (GET_LEVEL(d->character) < circle_restrict) {
        SEND_TO_Q("The game is temporarily restricted.. try again later.\r\n", d);
        STATE(d) = CON_CLOSE;
        sprintf(buf, "Request for login denied for %s [%s] (wizlock)",
                GET_NAME(d->character), d->host);
        mudlog(buf, NRM, LVL_GOD, TRUE);
        return;
      }
      /* check and make sure no other copies of this player are logged in */
      if (perform_dupe_check(d))
        return;

      if (GET_LEVEL(d->character) >= LVL_ELDER)
        SEND_TO_Q(imotd, d);
      else
        SEND_TO_Q(motd, d);

      sprintf(buf, "%s [%s] has connected.", GET_NAME(d->character), d->host);
      mudlog(buf, BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);

      if (load_result) {
        sprintf(buf, "\r\n\r\n\007\007\007"
                "%s%d LOGIN FAILURE%s SINCE LAST SUCCESSFUL LOGIN.%s\r\n",
                CCRED(d->character, C_SPR), load_result,
                (load_result > 1) ? "S" : "", CCNRM(d->character, C_SPR));
        SEND_TO_Q(buf, d);
        GET_BAD_PWS(d->character) = 0;
      }
      SEND_TO_Q("\r\n\n&n*** PRESS RETURN *** ", d);
      STATE(d) = CON_RMOTD;
    }
    break;

  case CON_NEWPASSWD:
  case CON_CHPWD_GETNEW:
    if (!*arg || strlen(arg) > MAX_PWD_LENGTH || strlen(arg) < 3 ||
        !str_cmp(arg, GET_NAME(d->character))) {
      SEND_TO_Q("\r\nIllegal password.\r\n", d);
      SEND_TO_Q("Password: ", d);
      return;
    }
    strncpy(GET_PASSWD(d->character), CRYPT(arg, GET_NAME(d->character)), MAX_PWD_LENGTH);
    *(GET_PASSWD(d->character) + MAX_PWD_LENGTH) = '\0';

    SEND_TO_Q("\r\nPlease retype password: ", d);
    if (STATE(d) == CON_NEWPASSWD)
      STATE(d) = CON_CNFPASSWD;
    else
      STATE(d) = CON_CHPWD_VRFY;

    break;

  case CON_CNFPASSWD:
  case CON_CHPWD_VRFY:
    if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character),
                MAX_PWD_LENGTH)) {
      SEND_TO_Q("\r\nPasswords don't match... start over.\r\n", d);
      SEND_TO_Q("Password: ", d);
      if (STATE(d) == CON_CNFPASSWD)
        STATE(d) = CON_NEWPASSWD;
      else
        STATE(d) = CON_CHPWD_GETNEW;
      return;
    }
    echo_on(d);

    if (STATE(d) == CON_CNFPASSWD) {
      SEND_TO_Q("ANSI terminal is a terminal with color suport.\r\n"
		"Normal Terminal is a terminal with no color suport.\r\n"
		"\r\n"
		"You only have color suport when you use a MUD client...\r\n"
		"If you aren't using a MUD client select the Normal Terminal.\r\n"
		"\r\n"
		"IMPORTANT: You can change this using the color command on the game.\r\n"
      		"\r\n"
      		"Select 'a' for a ANSI Terminal or 'n' for a Normal Terminal.\r\n"
                "You use a ANSI Terminal or a Normal Terminal? ", d);
      STATE(d) = CON_TERMINAL;
    } else {
      echo_on(d);
      SEND_TO_Q("\r\nDone.\r\n", d);
      SEND_TO_Q(MENU, d);
      STATE(d) = CON_MENU;
    }

    break;

  case CON_TERMINAL:          /* query terminal of new user         */
    switch (*arg) {
    case 'a':
    case 'A':
      SET_BIT(PRF_FLAGS(d->character), PRF_COLOR_1);
      SET_BIT(PRF_FLAGS(d->character), PRF_COLOR_2);
      break;
    case 'n':
    case 'N':
      break;
    default:
      SEND_TO_Q("\r\n"
      		"That is not a choice...\r\n"
                "Select 'a' to ANSI or 'n' to Normal Terminal.\r\n"
                "You are using a ANSI Terminal or a Normal Terminal? ", d);
      return;
    }
      GET_MASTER(d->character) = -1;
      SEND_TO_Q("\r\nWhat is your sex (M/F)? ", d);
      STATE(d) = CON_QSEX;
    break;
    
    case CON_MASTER:
    criacao(d, arg);
    break;
    
    case CON_NEWMASTER:
    criacao_new(d, arg);
    break;

  case CON_QSEX:                /* query sex of new user         */
    switch (*arg) {
    case 'm':
    case 'M':
      d->character->player.sex = SEX_MALE;
      break;
    case 'f':
    case 'F':
      d->character->player.sex = SEX_FEMALE;
      break;
    default:
      SEND_TO_Q("\r\n"
      		"That is not a sex..\r\n"
                "What IS your sex? ", d);
      return;
    }

    display_races(d);
    SEND_TO_Q("\r\nOption: ", d);
    STATE(d) = CON_QRACE;
    break;

  case CON_QRACE:
    load_result = parse_race(*arg);
    if (load_result == RACE_UNDEFINED ) {
      SEND_TO_Q("\r\nThat's not a valid option.\r\nOption: ", d);
      return;
    }

    if(load_result > 11)
    {
     SEND_TO_Q("\r\nThis is a very advanced race. Choose another.\r\nOption: ", d);
     return ;

    } else
      GET_RACE(d->character) = load_result;

    SEND_TO_Q("\r\n", d);
    do_help(d->character, pc_race_help_types[(int)load_result], 0, 0);
    SEND_TO_Q("&WDo you accept this race(&wy&W/&wn&W)? &w", d);
    STATE(d) = CON_QCONFIRMRACE;
    break;

  case CON_QCONFIRMRACE:

    switch (*arg) {

    case 'y':
    case 'Y':
      display_classes(d);
      SEND_TO_Q("\r\nOption: ", d);
      STATE(d) = CON_QCLASS;
      break;
    default:
      display_races(d);
      SEND_TO_Q("\r\nOption: ", d);
      STATE(d) = CON_QRACE;
      break;
    }
    break;


  case CON_QCLASS:
    load_result = parse_class(*arg);

    if (*arg == '-') {
      display_races(d);
      SEND_TO_Q("\r\nOption: ", d);
      STATE(d) = CON_QRACE;
      return;
    }

    if (load_result == CLASS_UNDEFINED ||
     !allowed_classes(GET_RACE(d->character), load_result)) {
      SEND_TO_Q("\r\nThat's not a valid option.\r\nOption: ", d);
      return;
    }
    GET_CLASS(d->character) = load_result;
    roll_real_abils(d->character);
    SEND_TO_Q("\r\n", d);
    display_stat(d);
    SEND_TO_Q("\r\n", d);
    disp_abils(d, load_result);
    SEND_TO_Q("&WDo you accept this character(&wy&W/&wn&W)? &w", d);
    STATE(d) = CON_QCONFIRMCLAS;
    break;

  case CON_QCONFIRMCLAS:

    switch (*arg) {
      case 'y':
      case 'Y':
        if (GET_PFILEPOS(d->character) < 0)
          GET_PFILEPOS(d->character) = create_entry(GET_NAME(d->character));

//        if(number(1,75) == 34)
//        GET_RACE(d->character) =  GET_RACE(d->character) + 12 ;

        init_char(d->character);
        save_char(d->character, NOWHERE);
        save_player_index();
        SEND_TO_Q(motd, d);
        SEND_TO_Q("\r\n\n&n*** PRESS RETURN *** ", d);
        STATE(d) = CON_RMOTD;
        sprintf(buf, "%s [%s] new player.", GET_NAME(d->character), d->host);
        mudlog(buf, NRM, LVL_IMMORT, TRUE);
        break;
      default:
        display_classes(d);
        SEND_TO_Q("\r\nOption: ", d);
        STATE(d) = CON_QCLASS;
	return;
//        break;
    }
   break;

  case CON_RMOTD:               /* read CR after printing motd   */
    SEND_TO_Q(MENU, d);
    STATE(d) = CON_MENU;
    break;

  case CON_SUMMARY:
    SEND_TO_Q(MENU, d);
    STATE(d) = CON_MENU;
    break;

  case CON_MENU:                /* get selection from main menu  */
    switch (*arg) {
    case '0':
      SEND_TO_Q("\r\n\r\nA strange voice whispers, 'The WarDome will wait your return...'\r\n\r\nWARDOME.FUGSPBR.ORG 2022 !!!\r\n\r\n", d);
      SEND_TO_Q("VISIT THE WARDOME MUD LIST IN http://www2.fugspbr.org/mailman/listinfo/wardome .\r\n\r\n", d);
      STATE(d) = CON_CLOSE;
      break;

    case '1': // join in

      load_result = enter_player_game(d);
      act("$n is dropped from a vortex of light and enters in the WarDome.", TRUE, d->character, 0, 0, TO_ROOM);

      STATE(d) = CON_PLAYING;
      if (!GET_LEVEL(d->character)) {
        do_start(d->character);
	SET_BIT(PRF_FLAGS(d->character), PRF_INFO_START);
	save_char(d->character, NOWHERE);
        send_to_char(START_MESSG, d->character);
      }
      look_at_room(d->character, 0);
      if (has_mail(GET_IDNUM(d->character)))
        send_to_char("&yYou have &Ymail&y waiting.&n\r\n", d->character);
      if (is_complete(GET_QUEST(d->character))) {
        send_to_char("&CYour current quest are already completed. Try another.&n\r\n", d->character);
	GET_QUEST(d->character) = -1;
      }
      actualize_splskl(d->character);
      save_char(d->character, NOWHERE);
        if (load_result == 2) {   /* rented items lost */
        send_to_char("\r\n\007You could not afford your rent!\r\n"
          "Your possesions have been donated to the Salvation Army!\r\n",
                     d->character);
      }
      d->has_prompt = 0;
      topten(d->character);
      topten1(d->character);
      verifica(d);
      
      topten2(d->character);
     if (GET_CLAN(d->character))
      {
       if (clans[posicao_clan(GET_CLAN(d->character))].em_guerra == -2) 
       {
      	    send_to_char("\r\n\a&RCLAN INFO&W: Se Prepare seu clan pode entrar em GUERRA!\r\n", d->character);
       }
       if (clans[posicao_clan(GET_CLAN(d->character))].em_guerra == -3) 
       {
      	    send_to_char("\r\n\a&RCLAN INFO&W: Declararam Guerra ao Seu Clan, Comunique ao Clan!\r\n", d->character);
       }
       if(clans[posicao_clan(GET_CLAN(d->character))].raided && clans[posicao_clan(GET_CLAN(d->character))].seen) 
       {
      	sprintf(buf, "\r\n\a&RCLAN INFO&W: Sua Sede foi atacada por %s!\r\n", clans[posicao_clan(clans[posicao_clan(GET_CLAN(d->character))].raided)].nome_clan);
        send_to_char(buf, d->character);
	clans[posicao_clan(GET_CLAN(d->character))].seen--;
        salvar_clan(GET_CLAN(d->character));
       } 
      }
break;
    case '2':  // read rules
      page_string(d, policies, 0);
      STATE(d) = CON_RMOTD;
      break;

    case '3': // quest of the week
      page_string(d, background, 0);
      STATE(d) = CON_RMOTD;
      break;

    case '4': // change password
      SEND_TO_Q("\r\nEnter your old password: ", d);
      echo_off(d);
      STATE(d) = CON_CHPWD_GETOLD;
      break;

    case '5': // enter description
        STATE(d) = CON_RMOTD;
        break ;

      if (d->character->player.description) {
        SEND_TO_Q("Current description:\r\n", d);
        SEND_TO_Q(d->character->player.description, d);
        /*
         * Don't free this now... so that the old description gets loaded
         * as the current buffer in the editor.  Do setup the ABORT buffer
         * here, however.
         *
         * free(d->character->player.description);
         * d->character->player.description = NULL;
         */
        d->backstr = str_dup(d->character->player.description);
      }
      SEND_TO_Q("Enter the new text you'd like others to see when they look at you.\r\n", d);
      SEND_TO_Q("(/s saves /h for help)\r\n", d);
      d->str = &d->character->player.description;
      d->max_str = EXDSCR_LENGTH;
      STATE(d) = CON_EXDESC;
      break;

    case '6': // enter whoisdescription
      if (d->character->player.whoisdesc) {
        SEND_TO_Q("Current text description:\r\n", d);
        SEND_TO_Q(d->character->player.whoisdesc, d);
        /*
         * Don't free this now... so that the old description gets loaded
         * as the current buffer in the editor.  Do setup the ABORT buffer
         * here, however.
         *
         * free(d->character->player.description);
         * d->character->player.description = NULL;
         */
        d->backstr = str_dup(d->character->player.whoisdesc);
      }
      SEND_TO_Q("Enter the new text you'd like others to see when they ask for you in a player command.\r\n", d);
      SEND_TO_Q("(/s saves /h for help)\r\n", d);
      d->str = &d->character->player.whoisdesc;
      d->max_str = EXDSCR_LENGTH;
      STATE(d) = CON_EXDESC;
      break;

    case '7': // summary
      make_summary(d->character);
      STATE(d) = CON_SUMMARY;
      break;

    case '8': // delete
      SEND_TO_Q("\r\nEnter your password for verification: ", d);
      echo_off(d);
      STATE(d) = CON_DELCNF1;
      break;

    default: // error
      SEND_TO_Q("\r\nThat's not a menu choice!\r\n", d);
      SEND_TO_Q(MENU, d);
      break;
    }

    break;

  case CON_CHPWD_GETOLD:
    if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character), MAX_PWD_LENGTH)) {
      echo_on(d);
      SEND_TO_Q("\r\nIncorrect password.\r\n", d);
      SEND_TO_Q(MENU, d);
      STATE(d) = CON_MENU;
    } else {
      SEND_TO_Q("\r\nEnter a new password: ", d);
      STATE(d) = CON_CHPWD_GETNEW;
    }
    return;

  case CON_DELCNF1:
    echo_on(d);
    if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character), MAX_PWD_LENGTH)) {
      SEND_TO_Q("\r\nIncorrect password.\r\n", d);
      SEND_TO_Q(MENU, d);
      STATE(d) = CON_MENU;
    } else {
      SEND_TO_Q("\r\nYOU ARE ABOUT TO DELETE THIS CHARACTER PERMANENTLY.\r\n"
                "ARE YOU ABSOLUTELY SURE?\r\n\r\n"
                "Please type \"yes\" to confirm: ", d);
      STATE(d) = CON_DELCNF2;
    }
    break;

  case CON_DELCNF2:
    if (!strcmp(arg, "yes") || !strcmp(arg, "YES")) {
      if (PLR_FLAGGED(d->character, PLR_FROZEN)) {
        SEND_TO_Q("You try to kill yourself, but the ice stops you.\r\n", d);
        SEND_TO_Q("Character not deleted.\r\n\r\n", d);
        STATE(d) = CON_CLOSE;
        return;
      }
      if (GET_LEVEL(d->character) < LVL_GRGOD)
        SET_BIT(PLR_FLAGS(d->character), PLR_DELETED);
      save_char(d->character, NOWHERE);
      Crash_delete_file(GET_NAME(d->character));

      /* If the selfdelete_fastwipe flag is set (in config.c), remove all
	 the player's immediately
      */
      if(selfdelete_fastwipe)
	if((player_i = find_name(GET_NAME(d->character))) >= 0) {
	  SET_BIT(player_table[player_i].flags, PINDEX_SELFDELETE);
	  remove_player(player_i);
	}

      sprintf(buf, "Character '%s' deleted!\r\n"
              "Goodbye.\r\n", GET_NAME(d->character));
      SEND_TO_Q(buf, d);
      sprintf(buf, "%s (lev %d) has self-deleted.", GET_NAME(d->character),
              GET_LEVEL(d->character));
      mudlog(buf, NRM, LVL_GOD, TRUE);
      STATE(d) = CON_CLOSE;
      return;
    } else {
      SEND_TO_Q("\r\nCharacter not deleted.\r\n", d);
      SEND_TO_Q(MENU, d);
      STATE(d) = CON_MENU;
    }
    break;

/*      Taken care of in game_loop()
  case CON_CLOSE:
    close_socket(d);
    break;
*/

  default:
    log("SYSERR: Nanny: illegal state of con'ness (%d) for '%s'; closing connection.",
        STATE(d), d->character ? GET_NAME(d->character) : "<unknown>");
    STATE(d) = CON_DISCONNECT;  /* Safest to do. */
    break;
  }
}

struct char_data *is_playing(char *vict_name)
{
  extern struct descriptor_data *descriptor_list;
  struct descriptor_data *i, *next_i;

  for (i = descriptor_list; i; i = next_i) {
    next_i = i->next;
    if(i->connected == CON_PLAYING && !str_cmp(i->character->player.name, vict_name))
      return (i->character);
  }
  return (NULL);
}



