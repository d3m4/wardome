/* ************************************************************************
*   File: utils.h                                       Part of CircleMUD *
*  Usage: header file: utility macros and prototypes of utility funcs     *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

/* external declarations and prototypes **********************************/

extern struct weather_data weather_info;
extern FILE *logfile;

extern const char *god_names[];

#define log                     basic_mud_log
 
/* public functions in utils.c */
//char    *str_dup(const char *source);
int     str_cmp(const char *arg1, const char *arg2);
int     strn_cmp(const char *arg1, const char *arg2, int n);
void    basic_mud_log(const char *format, ...) __attribute__ ((format (printf, 1, 2)));
int     touch(const char *path);
void    mudlog(const char *str, int type, int level, int file);
void    log_death_trap(struct char_data *ch);
int     number(int from, int to);
int     dice(int number, int size);
void    sprintbit(long vektor, const char *names[], char *result);
void    sprinttype(int type, const char *names[], char *result);
int     get_line(FILE *fl, char *buf);
int     get_filename(char *orig_name, char *filename, int mode);
struct time_info_data *age(struct char_data *ch);
int     num_pc_in_room(struct room_data *room);
void    core_dump_real(const char *, ush_int);
int     replace_str(char **string, char *pattern, char *replacement, int rep_all, int max_size);
void    format_text(char **ptr_string, int mode, struct descriptor_data *d, int maxlen);
char    *add_points(int value);
char 	*convert_num(double var);
int 	allow_follower(struct char_data *ch, int max_fol);
void 	really_free(void *ptr);
char 	*reprimir(char *vemde, int num);
void manda_som(char *som, struct char_data *ch);
#define SOUND(musica, receptor)  manda_som(musica, receptor)

/*
extern int leak_logging;
extern void leak_dump(void);
extern void leak_clear(void);

extern void *leak_malloc(size_t, const char *, int);
extern void *leak_realloc(void *, size_t, const char *, int);
extern void leak_free(void *, const char *, int);
extern void *leak_calloc(size_t, size_t, const char *, int);
extern char *leak_strdup(const char *, const char *, int);

#undef malloc
#undef realloc
#undef free
#undef calloc
#undef strdup

#define malloc(s) leak_malloc(s, __FILE__, __LINE__)
#define realloc(p, s) leak_realloc(p, s, __FILE__, __LINE__)
#define free(p)  leak_free(p, __FILE__, __LINE__)
#define calloc(s, t) leak_calloc(s, t, __FILE__, __LINE__)
#define str_dup(s) leak_strdup(s, __FILE__, __LINE__)
*/

//#define str_dup(s) leak_strdup(s, __FILE__, __LINE__)
#define str_dup(s) strdup(s)

#define core_dump()             core_dump_real(__FILE__, __LINE__)

/* random functions in random.c */
void circle_srandom(unsigned long initial_seed);
unsigned long circle_random(void);

/* undefine MAX and MIN so that our functions are used instead */
#ifdef MAX
#undef MAX
#endif

#ifdef MIN
#undef MIN
#endif

int MAX(int a, int b);
int MIN(int a, int b);

/* in magic.c */
bool    circle_follow(struct char_data *ch, struct char_data * victim);

/* in act.informative.c */
void    look_at_room(struct char_data *ch, int mode);

/* in act.movmement.c */
int     do_simple_move(struct char_data *ch, int dir, int following);
int     perform_move(struct char_data *ch, int dir, int following);

/* in limits.c */
int     mana_limit(struct char_data *ch);
int     hit_limit(struct char_data *ch);
int     move_limit(struct char_data *ch);
int     mana_gain(struct char_data *ch);
int     hit_gain(struct char_data *ch);
int     move_gain(struct char_data *ch);
void    advance_level(struct char_data *ch);
void    set_title(struct char_data *ch, char *title);
void    set_prename(struct char_data *ch, char *title);
void    gain_exp(struct char_data *ch, int gain);
void    gain_exp_regardless(struct char_data *ch, int gain);
void    gain_condition(struct char_data *ch, int condition, int value);
void    check_idling(struct char_data *ch);
void    point_update(void);
void    update_pos(struct char_data *victim);

/* various constants *****************************************************/

/* defines for mudlog() */
#define OFF     0
#define BRF     1
#define NRM     2
#define CMP     3

/* get_filename() */
#define CRASH_FILE      0
#define ETEXT_FILE      1
#define ALIAS_FILE      2
#define IGNORE_FILE     3//last

//toca som 
#define HAS_SOUND(ch) (PLR_FLAGGED(ch, PLR_MSP))
 #define SND_CHAR 1
 #define SND_ROOM 2
 #define SND_AREA 3
 #define SND_WORLD 4



/* breadth-first searching */
#define BFS_ERROR               -1
#define BFS_ALREADY_THERE       -2
#define BFS_NO_PATH             -3

/*
 * XXX: These constants should be configurable. See act.informative.c
 *      and utils.c for other places to change.
 */
/* mud-life time */
#define SECS_PER_MUD_HOUR       75
#define SECS_PER_MUD_DAY        (24*SECS_PER_MUD_HOUR)
#define SECS_PER_MUD_MONTH      (35*SECS_PER_MUD_DAY)
#define SECS_PER_MUD_YEAR       (17*SECS_PER_MUD_MONTH)

/* real-life time (remember Real Life?) */
#define SECS_PER_REAL_MIN       60
#define SECS_PER_REAL_HOUR      (60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY       (24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR      (365*SECS_PER_REAL_DAY)


/* string utils **********************************************************/


#define YESNO(a) ((a) ? "YES" : "NO")
#define ONOFF(a) ((a) ? "ON" : "OFF")

#define LOWER(c)   (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))
#define UPPER(c)   (((c)>='a'  && (c) <= 'z') ? ((c)+('A'-'a')) : (c) )

#define ISNEWL(ch) ((ch) == '\n' || (ch) == '\r')
#define IF_STR(st) ((st) ? (st) : "\0")
#define CAP(st)  (*(st) = UPPER(*(st)), st)

#define AN(string) (strchr("aeiouAEIOU", *string) ? "an" : "a")


/* memory utils **********************************************************/

#define CREATE(result, type, number)  do {\
	if ((number) * sizeof(type) <= 0)	\
		log("SYSERR: Zero bytes or less requested at %s:%d.", __FILE__, __LINE__);	\
	if (!((result) = (type *) calloc ((number), sizeof(type))))	\
		{ perror("SYSERR: malloc failure"); abort(); } } while(0)

#define RECREATE(result,type,number) do {\
  if (!((result) = (type *) realloc ((result), sizeof(type) * (number))))\
		{ perror("SYSERR: realloc failure"); abort(); } } while(0)

/*
 * the source previously used the same code in many places to remove an item
 * from a list: if it's the list head, change the head, else traverse the
 * list looking for the item before the one to be removed.  Now, we have a
 * macro to do this.  To use, just make sure that there is a variable 'temp'
 * declared as the same type as the list to be manipulated.  BTW, this is
 * a great application for C++ templates but, alas, this is not C++.  Maybe
 * CircleMUD 4.0 will be...
 */
#define REMOVE_FROM_LIST(item, head, next)      \
   if ((item) == (head))                \
      head = (item)->next;              \
   else {                               \
      temp = head;                      \
      while (temp && (temp->next != (item))) \
         temp = temp->next;             \
      if (temp)                         \
         temp->next = (item)->next;     \
   }                                    \


/* basic bitvector utils *************************************************/


#define IS_SET(flag,bit)  ((flag) & (bit))
#define SET_BIT(var,bit)  ((var) |= (bit))
#define REMOVE_BIT(var,bit)  ((var) &= ~(bit))
#define TOGGLE_BIT(var,bit) ((var) = (var) ^ (bit))

/*
 * Accessing player specific data structures on a mobile is a very bad thing
 * to do.  Consider that changing these variables for a single mob will change
 * it for every other single mob in the game.  If we didn't specifically check
 * for it, 'wimpy' would be an extremely bad thing for a mob to do, as an
 * example.  If you really couldn't care less, change this to a '#if 0'.
 */
#if 0
/* Subtle bug in the '#var', but works well for now. */
#define CHECK_PLAYER_SPECIAL(ch, var) \
        (*(((ch)->player_specials == &dummy_mob) ? (log("SYSERR: Mob using '"#var"' at %s:%d.", __FILE__, __LINE__), &(var)) : &(var)))
#else
#define CHECK_PLAYER_SPECIAL(ch, var)   (var)
#endif

#define MOB_FLAGS(ch) ((ch)->char_specials.saved.act)
#define PLR_FLAGS(ch) ((ch)->char_specials.saved.act)
#define PLR2_FLAGS(ch) ((ch)->char_specials.saved.act2)
#define PRF_FLAGS(ch) CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.pref))
#define PRF2_FLAGS(ch) CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.pref2))
#define AFF_FLAGS(ch) ((ch)->char_specials.saved.affected_by)
#define AFF2_FLAGS(ch) ((ch)->char_specials.saved.affected2_by)
#define AFF3_FLAGS(ch) ((ch)->char_specials.saved.affected3_by)
#define ROOM_FLAGS(loc) (world[(loc)].room_flags)
#define ROOM_AFFECTIONS(loc)    (world[(loc)].room_affections)
#define ZONE_FLAGS(loc)		(zone_table[(loc)].zone_flags)


#define IS_NPC(ch)  (IS_SET(MOB_FLAGS(ch), MOB_ISNPC))
#define IS_MOB(ch)  (IS_NPC(ch) && ((ch)->nr >-1))

#define MOB_FLAGGED(ch, flag) (IS_NPC(ch) && IS_SET(MOB_FLAGS(ch), (flag)))
#define PLR_FLAGGED(ch, flag) (!IS_NPC(ch) && IS_SET(PLR_FLAGS(ch), (flag)))
#define AFF_FLAGGED(ch, flag) (IS_SET(AFF_FLAGS(ch), (flag)))
#define AFF2_FLAGGED(ch, flag) (IS_SET(AFF2_FLAGS(ch), (flag)))
#define AFF3_FLAGGED(ch, flag) (IS_SET(AFF3_FLAGS(ch), (flag)))
#define PRF_FLAGGED(ch, flag) (IS_SET(PRF_FLAGS(ch), (flag)))
#define PRF2_FLAGGED(ch, flag) (IS_SET(PRF2_FLAGS(ch), (flag)))
#define ROOM_FLAGGED(loc, flag) (IS_SET(ROOM_FLAGS(loc), (flag)))
#define ZONE_FLAGGED(loc, flag)  (IS_SET(ZONE_FLAGS(loc), (flag)))
#define EXIT_FLAGGED(exit, flag) (IS_SET((exit)->exit_info, (flag)))
#define OBJVAL_FLAGGED(obj, flag) (IS_SET(GET_OBJ_VAL((obj), 1), (flag)))
#define OBJWEAR_FLAGGED(obj, flag) (IS_SET((obj)->obj_flags.wear_flags, (flag)))
#define OBJ_FLAGGED(obj, flag) (IS_SET(GET_OBJ_EXTRA(obj), (flag)))
#define ROOM_AFFECTED(loc, aff) (IS_SET(ROOM_AFFECTIONS(loc), (aff)))

/* IS_AFFECTED for backwards compatibility */
#define IS_AFFECTED(ch, skill) (AFF_FLAGGED((ch), (skill)))
#define IS_AFFECTED2(ch, skill) (AFF2_FLAGGED((ch), (skill)))
#define IS_AFFECTED3(ch, skill) (AFF3_FLAGGED((ch), (skill)))

#define PLR_TOG_CHK(ch,flag) ((TOGGLE_BIT(PLR_FLAGS(ch), (flag))) & (flag))
#define PLR2_TOG_CHK(ch,flag) ((TOGGLE_BIT(PLR2_FLAGS(ch), (flag))) & (flag))
#define PRF_TOG_CHK(ch,flag) ((TOGGLE_BIT(PRF_FLAGS(ch), (flag))) & (flag))
#define PRF2_TOG_CHK(ch,flag) ((TOGGLE_BIT(PRF2_FLAGS(ch), (flag))) & (flag))

/* room utils ************************************************************/


#define SECT(room)      (world[(room)].sector_type)

#define IS_DARK(room)  ( !world[room].light && \
                         (ROOM_FLAGGED(room, ROOM_DARK) || \
                          ( ( SECT(room) != SECT_INSIDE && \
                              SECT(room) != SECT_CITY ) && \
                            (weather_info.sunlight == SUN_SET || \
                             weather_info.sunlight == SUN_DARK)) ) )

#define IS_LIGHT(room)  (!IS_DARK(room))

#define GET_ROOM_VNUM(rnum)     ((rnum) >= 0 && (rnum) <= top_of_world ? world[(rnum)].number : NOWHERE)
#define GET_ROOM_SPEC(room) ((room) >= 0 ? world[(room)].func : NULL)

/* char utils ************************************************************/


#define GET_GOD_NAME(ch)     (god_names[((ch)->player.level) - LVL_IMMORT])
#define GET_GOD_WIZNAME(ch)  (god_wiznames[((ch)->player.level) - LVL_IMMORT])
#define GET_REMORT_NAME(ch)  (remort_names[((ch)->player.remort) - 1])

#define GET_LANGUAGE(ch)        ((ch)->player_specials->saved.language)
#define GET_CLAN(ch)		((ch)->player_specials->saved.clan)
#define GET_CLAN_POS(ch)	((ch)->player_specials->saved.clan_rank)
#define GET_POINTS(ch) 		((ch)->player_specials->saved.points)
#define GET_QP(ch)     		((ch)->player_specials->saved.qpoints)
#define GET_CP(ch)     		((ch)->player_specials->saved.cpoints)


#define IN_ROOM(ch)     ((ch)->in_room)
#define GET_WAS_IN(ch)  ((ch)->was_in_room)
#define GET_AGE(ch)     (age(ch)->year)

#define GET_NAME(ch)    (IS_NPC(ch) ? \
                         (ch)->player.short_descr : (ch)->player.name)
#define GET_TITLE(ch)   ((ch)->player.title)
#define GET_PRENAME(ch) ((ch)->player.prename)
#define GET_LEVEL(ch)   ((ch)->player.level)
#define GET_PASSWD(ch)  ((ch)->player.passwd)
#define GET_PFILEPOS(ch)((ch)->pfilepos)
#define GET_REMORT(ch)  ((ch)->player.remort)
#define GET_MASTER(ch)	((ch)->player.id_master)
#define GET_SECOND(ch)  ((ch)->player.id_second) // nome do second player


#define GET_QUEST(ch)       ((ch)->current_quest)
#define GET_QUEST_TYPE(ch)  (aquest_table[real_quest((int)GET_QUEST((ch)))].type)

#define GET_STYLE(ch)		((ch)->player_specials->saved.style)
#define GET_RELIGION(ch)	((ch)->player_specials->saved.religion)
#define GET_RP(ch)		((ch)->player_specials->saved.rp)

/*
 * I wonder if this definition of GET_REAL_LEVEL should be the definition
 * of GET_LEVEL?  JE
 */
#define GET_REAL_LEVEL(ch) \
   (ch->desc && ch->desc->original ? GET_LEVEL(ch->desc->original) : \
    GET_LEVEL(ch))

#define GET_CLASS(ch)   ((ch)->player.chclass)
#define GET_RACE(ch)    ((ch)->player.race)
#define GET_HOME(ch)    ((ch)->player.hometown)
#define GET_HEIGHT(ch)  ((ch)->player.height)
#define GET_WEIGHT(ch)  ((ch)->player.weight)
#define GET_SEX(ch)     ((ch)->player.sex)

#define GET_KILLED(ch)  ((ch)->player.killed)
#define GET_FBI_POS(ch) ((ch)->player.fbi_pos)
#define GET_DIED(ch)    ((ch)->player.died)
#define GET_ARENA_KILLED(ch)  ((ch)->player.a_killed)
#define GET_ARENA_DIED(ch)    ((ch)->player.a_died)

#define GET_TRANS(ch)		((ch)->player.trans)
#define GET_TRANS_HP(ch)	((ch)->player.trans_hp)
#define GET_TRANS_MANA(ch)	((ch)->player.trans_mana)
#define GET_TRANS_MOVE(ch)	((ch)->player.trans_move)

#define LIBERDADE(ch)		((ch)->player.liberdade)

// definido para summary
#define GET_LVL_SUM(ch)		((ch)->player.lvl_sum)
#define GET_EXP_SUM(ch)		((ch)->player.exp_sum)
#define GET_GOLD_SUM(ch)	((ch)->player.gold_sum)
#define GET_KMS(ch)		((ch)->player.kills_mob_sum)
#define GET_KPS(ch)		((ch)->player.kills_ppl_sum)
#define GET_DIED_S(ch)		((ch)->player.died_sum)
#define GET_POINTS_S(ch)	((ch)->player.points_sum)
#define GET_POINTS_GS(ch)	((ch)->player.points_g_sum)
// fim do summary

#define GET_STR(ch)     ((ch)->aff_abils.str)
#define GET_ADD(ch)     ((ch)->aff_abils.str_add)
#define GET_DEX(ch)     ((ch)->aff_abils.dex)
#define GET_INT(ch)     ((ch)->aff_abils.intel)
#define GET_WIS(ch)     ((ch)->aff_abils.wis)
#define GET_CON(ch)     ((ch)->aff_abils.con)
#define GET_CHA(ch)     ((ch)->aff_abils.cha)
#define GET_LUK(ch)     ((ch)->aff_abils.luk)

#define GET_AC(ch)        ((ch)->points.armor)
#define GET_HIT(ch)       ((ch)->points.hit)
#define GET_MAX_HIT(ch)   ((ch)->points.max_hit)
#define GET_MOVE(ch)      ((ch)->points.move)
#define GET_MAX_MOVE(ch)  ((ch)->points.max_move)
#define GET_MANA(ch)      ((ch)->points.mana)
#define GET_MAX_MANA(ch)  ((ch)->points.max_mana)
#define GET_OXI(ch)       ((ch)->points.oxigen)
#define GET_MAX_OXI(ch)   ((ch)->points.max_oxigen)
#define GET_MENTAL(ch)    ((ch)->points.mental)
#define GET_MAX_MENTAL(ch)((ch)->points.max_mental)
#define GET_GOLD(ch)      ((ch)->points.gold)
#define GET_BANK_GOLD(ch) ((ch)->points.bank_gold)
#define GET_HITROLL(ch)   ((ch)->points.hitroll)
#define GET_DAMROLL(ch)   ((ch)->points.damroll)
#define GET_EXP(ch)       ((ch)->points.exp)

#define GET_POS(ch)       ((ch)->char_specials.position)
#define GET_IDNUM(ch)     ((ch)->char_specials.saved.idnum)
#define GET_ID(x)         ((x)->id)
#define GET_CHALLENGE(ch) ((ch)->char_specials.saved.challenge)
#define GET_CHALLENGER(ch)  ((ch)->player.challenger)
#define IS_CARRYING_W(ch) ((ch)->char_specials.carry_weight)
#define IS_CARRYING_N(ch) ((ch)->char_specials.carry_items)
#define FIGHTING(ch)      ((ch)->char_specials.fighting)
#define BASHTIME(ch)      ((ch)->char_specials.nobashtime)
#define HUNTING(ch)       ((ch)->char_specials.hunting)
#define GET_SAVE(ch, i)   ((ch)->char_specials.saved.apply_saving_throw[i])
#define GET_ALIGNMENT(ch) ((ch)->char_specials.saved.alignment)
#define GET_BETTED_ON(ch) ((ch)->player_specials->betted_on)
#define GET_BET_AMT(ch)   ((ch)->player_specials->bet_amt)

#define GET_COND(ch, i)         CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.conditions[(i)]))
#define GET_LOADROOM(ch)        CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.load_room))
#define GET_PRACTICES(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.spells_to_learn))
#define GET_INVIS_LEV(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.invis_level))
#define GET_WIMP_LEV(ch)        CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.wimp_level))
#define GET_ICQ(ch)             CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.icq))
#define GET_FREEZE_LEV(ch)      CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.freeze_level))
#define GET_BAD_PWS(ch)         CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.bad_pws))
#define GET_TALK(ch, i)         CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.talks[i]))
#define POOFIN(ch)              CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->poofin))
#define POOFOUT(ch)             CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->poofout))
#define GET_LAST_OLC_TARG(ch)   CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->last_olc_targ))
#define GET_LAST_OLC_MODE(ch)   CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->last_olc_mode))
#define GET_ALIASES(ch)         CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->aliases))
#define GET_LAST_TELL(ch)       CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->last_tell))



#define GET_SKILL(ch, i)        (AFF2_FLAGGED(ch, AFF2_TRANSFORM) ? 0 : CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.skills[i])))

#define GET_SKILL_LS(ch, i)     CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.skills[i]))

#define SET_SKILL(ch, i, pct)   do { CHECK_PLAYER_SPECIAL((ch), (ch)->player_specials->saved.skills[i]) = pct; } while(0)

#define GET_RSKILL(ch, i, num)  CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.rskills[i][num+1]))
#define GET_CLASS_REMORT(ch, i) CHECK_PLAYER_SPECIAL((ch), ((ch)->player_specials->saved.rskills[i][0]))

#define GET_EQ(ch, i)           ((ch)->equipment[i])
#define IS_BLUDGEON(eq)			( (GET_OBJ_VAL(eq, 3) == 5)   || (GET_OBJ_VAL(eq, 3) == 6) || (GET_OBJ_VAL(eq, 3) == 10) || (GET_OBJ_VAL(eq, 3) == 13) || (GET_OBJ_VAL(eq, 3) == 9) || (GET_OBJ_VAL(eq, 3) == 7) || (GET_OBJ_VAL(eq, 3) == 12) )
#define IS_SLASH(eq)			( (GET_OBJ_VAL(eq, 3) == 2) || (GET_OBJ_VAL(eq, 3) == 8) || (GET_OBJ_VAL(eq, 3) == 3) ) 
#define IS_PIERCE(eq)			( (GET_OBJ_VAL(eq, 3) == 1) || (GET_OBJ_VAL(eq, 3) == 4) || (GET_OBJ_VAL(eq, 3) == 11) || (GET_OBJ_VAL(eq, 3) == 14) ) 


#define GET_MOB_SPEC(ch)        (IS_MOB(ch) ? mob_index[(ch)->nr].func : NULL)
#define GET_MOB_RNUM(mob)       ((mob)->nr)
#define GET_MOB_VNUM(mob)       (IS_MOB(mob) ? \
                                 mob_index[GET_MOB_RNUM(mob)].vnum : -1)

#define GET_MOB_WAIT(ch)        ((ch)->mob_specials.wait_state)
#define GET_DEFAULT_POS(ch)     ((ch)->mob_specials.default_pos)
#define MEMORY(ch)              ((ch)->mob_specials.memory)

#define STRENGTH_APPLY_INDEX(ch) GET_STR(ch)

#define CAN_CARRY_W(ch) (str_app[STRENGTH_APPLY_INDEX(ch)].carry_w)
#define CAN_CARRY_N(ch) (int) (1 + ((GET_DEX(ch)) * (1+(GET_LEVEL(ch)/200))))
#define AWAKE(ch) (GET_POS(ch) > POS_SLEEPING)
#define CAN_SEE_IN_DARK(ch) \
   (AFF_FLAGGED(ch, AFF_INFRAVISION) || (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_HOLYLIGHT)))

#define IS_GOOD(ch)    (GET_ALIGNMENT(ch) >= 350)
#define IS_EVIL(ch)    (GET_ALIGNMENT(ch) <= -350)
#define IS_NEUTRAL(ch) (!IS_GOOD(ch) && !IS_EVIL(ch))


/* descriptor-based utils ************************************************/

/* These three deprecated. */
#define WAIT_STATE(ch, cycle) { \
        if ((ch)->desc) (ch)->desc->wait = (cycle); \
        else if (IS_NPC(ch)) GET_MOB_WAIT(ch) = (cycle); }

#define CHECK_WAIT(ch)  (((ch)->desc) ? ((ch)->desc->wait > 1) : 0)
/* New, preferred macro. */
#define GET_WAIT_STATE(d)    ((d)->wait)

#define STATE(d)        ((d)->connected)


/* object utils **********************************************************/


#define GET_OBJ_TYPE(obj)       ((obj)->obj_flags.type_flag)
#define GET_OBJ_COST(obj)       ((obj)->obj_flags.cost)
#define GET_OBJ_RENT(obj)       ((obj)->obj_flags.cost_per_day)
#define GET_OBJ_EXTRA(obj)      ((obj)->obj_flags.extra_flags)
#define GET_OBJ_WEAR(obj)       ((obj)->obj_flags.wear_flags)
#define GET_OBJ_VAL(obj, val)   ((obj)->obj_flags.value[(val)])
#define GET_OBJ_WEIGHT(obj)     ((obj)->obj_flags.weight)
#define GET_OBJ_TIMER(obj)      ((obj)->obj_flags.timer)
#define GET_OBJ_LEVEL(obj)      ((obj)->obj_flags.obj_level)
#define GET_OBJ_PERM(obj)       ((obj)->obj_flags.bitvector)
#define GET_OBJ_COND(obj)       ((obj)->obj_flags.cond)
#define GET_OBJ_RNUM(obj)       ((obj)->item_number)
#define GET_OBJ_VNUM(obj)       (GET_OBJ_RNUM(obj) >= 0 ? \
                                 obj_index[GET_OBJ_RNUM(obj)].vnum : -1)
#define IS_OBJ_STAT(obj,stat)   (IS_SET((obj)->obj_flags.extra_flags,stat))
#define IS_CORPSE(obj)          (GET_OBJ_TYPE(obj) == ITEM_CONTAINER && \
                                        (GET_OBJ_VAL((obj), 3) == 1 || \
                                         GET_OBJ_VAL((obj), 3) == 2))
#define IS_PC_CORPSE(obj)          (GET_OBJ_TYPE(obj) == ITEM_CONTAINER && \
                                         GET_OBJ_VAL((obj), 3) == 1)
#define IS_NPC_CORPSE(obj)          (GET_OBJ_TYPE(obj) == ITEM_CONTAINER && \
                                         GET_OBJ_VAL((obj), 3) == 2)

#define GET_OBJ_SPEC(obj) ((obj)->item_number >= 0 ? \
        (obj_index[(obj)->item_number].func) : NULL)

#define CAN_WEAR(obj, part) (IS_SET((obj)->obj_flags.wear_flags, (part)))

/* compound utilities and other macros **********************************/

/*
 * Used to compute CircleMUD version. To see if the code running is newer
 * than 3.0pl13, you would use: #if _CIRCLEMUD > CIRCLEMUD_VERSION(3,0,13)
 */
#define CIRCLEMUD_VERSION	"&RBattle&n &BW&bar&BD&bome &BII,&bVersion &R2.0&n"

#define HSHR(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "his":"her") :"its")
#define HSSH(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "he" :"she") : "it")
#define HMHR(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "him":"her") : "it")

#define ANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "An" : "A")
#define SANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "an" : "a")


/* Various macros building up to CAN_SEE */

#define LIGHT_OK(sub)   (!AFF_FLAGGED(sub, AFF_BLIND) && \
   (IS_LIGHT((sub)->in_room) || AFF_FLAGGED((sub), AFF_INFRAVISION)))

#define INVIS_OK(sub, obj) \
 ((!AFF_FLAGGED((obj),AFF_INVISIBLE) || AFF_FLAGGED(sub,AFF_DETECT_INVIS)) && \
 (!AFF_FLAGGED((obj), AFF_HIDE) || AFF_FLAGGED(sub, AFF_SENSE_LIFE)) && \
  !AFF_FLAGGED((obj), AFF_GAS) && !ROOM_AFFECTED(sub->in_room, RAFF_FOG))

#define MORT_CAN_SEE(sub, obj) (LIGHT_OK(sub) && INVIS_OK(sub, obj))

#define IMM_CAN_SEE(sub, obj) \
   (MORT_CAN_SEE(sub, obj) || (!IS_NPC(sub) && PRF_FLAGGED(sub, PRF_HOLYLIGHT)))

#define SELF(sub, obj)  ((sub) == (obj))

/* Can subject see character "obj"? */
#define CAN_SEE(sub, obj) (SELF(sub, obj) || \
   ((GET_REAL_LEVEL(sub) >= (IS_NPC(obj) ? 0 : GET_INVIS_LEV(obj))) && \
   IMM_CAN_SEE(sub, obj)))

/* End of CAN_SEE */


#define INVIS_OK_OBJ(sub, obj) \
  (!IS_OBJ_STAT((obj), ITEM_INVISIBLE) || AFF_FLAGGED((sub), AFF_DETECT_INVIS))

/* Is anyone carrying this object and if so, are they visible? */
#define CAN_SEE_OBJ_CARRIER(sub, obj) \
  ((!obj->carried_by || CAN_SEE(sub, obj->carried_by)) &&       \
   (!obj->worn_by || CAN_SEE(sub, obj->worn_by)))

#define MORT_CAN_SEE_OBJ(sub, obj) \
  (LIGHT_OK(sub) && INVIS_OK_OBJ(sub, obj) && CAN_SEE_OBJ_CARRIER(sub, obj))

#define CAN_SEE_OBJ(sub, obj) \
   (MORT_CAN_SEE_OBJ(sub, obj) || (!IS_NPC(sub) && PRF_FLAGGED((sub), PRF_HOLYLIGHT)))

#define CAN_CARRY_OBJ(ch,obj)  \
   (((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) <= CAN_CARRY_W(ch)) &&   \
    ((IS_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch)))

#define CAN_GET_OBJ(ch, obj)   \
   (CAN_WEAR((obj), ITEM_WEAR_TAKE) && CAN_CARRY_OBJ((ch),(obj)) && \
    CAN_SEE_OBJ((ch),(obj)))

#define PERS(ch, vict)   (CAN_SEE(vict, ch) ? GET_NAME(ch) : "Someone")

#define OBJS(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
        (obj)->short_description  : "something")

#define OBJN(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
        fname((obj)->name) : "something")


#define EXIT(ch, door)  (world[(ch)->in_room].dir_option[door])

#define CAN_GO(ch, door) (EXIT(ch,door) && \
                         (EXIT(ch,door)->to_room != NOWHERE) && \
                         !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))


#define CLASS_ABBR(ch) (IS_NPC(ch) ? npc_class_abbrevs[(int)GET_CLASS(ch)] : \
                                         class_abbrevs[(int)GET_CLASS(ch)])

#define RACE_ABBR(ch)  (IS_NPC(ch) ? "--" : race_abbrevs[(int)GET_RACE(ch)])

#define IS_MAGIC_USER(ch)       (GET_CLASS(ch) == CLASS_MAGIC_USER)

#define IS_CLERIC(ch)           (GET_CLASS(ch) == CLASS_CLERIC)

#define IS_THIEF(ch)            (GET_CLASS(ch) == CLASS_THIEF)

#define IS_WARRIOR(ch)          (GET_CLASS(ch) == CLASS_WARRIOR)

#define IS_NECROMANCER(ch)      (GET_CLASS(ch) == CLASS_NECROMANCER)

#define IS_PALADIN(ch)          (GET_CLASS(ch) == CLASS_PALADIN)

#define IS_NINJA(ch)            (GET_CLASS(ch) == CLASS_NINJA)

#define IS_RANGER(ch)           (GET_CLASS(ch) == CLASS_RANGER)

#define IS_PSIONICIST(ch)       (GET_CLASS(ch) == CLASS_PSIONICIST)

#define IS_WARLOCK(ch)          (GET_CLASS(ch) == CLASS_WARLOCK)

#define IS_BARBARIAN(ch)        (GET_CLASS(ch) == CLASS_BARBARIAN)

#define IS_SORCERER(ch)         (GET_CLASS(ch) == CLASS_SORCERER)


#define IS_VAMPIRE(ch)          (GET_RACE(ch) == RACE_VAMPIRE)

#define IS_DROW(ch)             (GET_RACE(ch) == RACE_DROW)

#define IS_DWARF(ch)            (GET_RACE(ch) == RACE_DWARF)

#define IS_ELF(ch)              (GET_RACE(ch) == RACE_ELF)

#define IS_OGRE(ch)             (GET_RACE(ch) == RACE_OGRE)

#define IS_ORC(ch)              (GET_RACE(ch) == RACE_ORC)

#define IS_TROLL(ch)            (GET_RACE(ch) == RACE_TROLL)

#define IS_GITH(ch)             (GET_RACE(ch) == RACE_GITH)

#define IS_GNOME(ch)            (GET_RACE(ch) == RACE_GNOME)

#define IS_LIZARDMAN(ch)        (GET_RACE(ch) == RACE_LIZARDMAN)

#define IS_SEA_ELF(ch)          (GET_RACE(ch) == RACE_SEA_ELF)

#define IS_GORAK(ch)            (GET_RACE(ch) == RACE_GORAK)

#define IS_DUNEDAIN(ch)         (GET_RACE(ch) == RACE_DUNEDAIN)

#define IS_ANCI_DROW(ch)        (GET_RACE(ch) == RACE_ANCI_DROW)

#define IS_NAUGRIM(ch)          (GET_RACE(ch) == RACE_NAUGRIM)

#define IS_HIGH_ELF(ch)         (GET_RACE(ch) == RACE_HIGH_ELF)

#define IS_HILL_OGRE(ch)        (GET_RACE(ch) == RACE_HILL_OGRE)

#define IS_BUGBEAR(ch)          (GET_RACE(ch) == RACE_BUGBEAR)

#define IS_CAVE_TROLL(ch)       (GET_RACE(ch) == RACE_CAVE_TROLL)

#define IS_LICH(ch)             (GET_RACE(ch) == RACE_LICH)

#define IS_TINKER(ch)           (GET_RACE(ch) == RACE_TINKER)

#define IS_DRACONIAN(ch)        (GET_RACE(ch) == RACE_DRACONIAN)

#define IS_H_SEA_ELF(ch)        (GET_RACE(ch) == RACE_H_SEA_ELF)

#define IS_ARCHONS(ch)          (GET_RACE(ch) == RACE_ARCHONS)


#define OUTSIDE(ch) (!ROOM_FLAGGED((ch)->in_room, ROOM_INDOORS))

#define FOREST(room)			(world[room].sector_type == SECT_FOREST || ROOM_AFFECTED(room, RAFF_ILLUSIONF))


/* OS compatibility ******************************************************/


/* there could be some strange OS which doesn't have NULL... */
#ifndef NULL
#define NULL (void *)0
#endif

#if !defined(FALSE)
#define FALSE 0
#endif

#if !defined(TRUE)
#define TRUE  (!FALSE)
#endif

/* defines for fseek */
#ifndef SEEK_SET
#define SEEK_SET        0
#define SEEK_CUR        1
#define SEEK_END        2
#endif

/*
 * NOCRYPT can be defined by an implementor manually in sysdep.h.
 * CIRCLE_CRYPT is a variable that the 'configure' script
 * automatically sets when it determines whether or not the system is
 * capable of encrypting.
 */
#if defined(NOCRYPT) || !defined(CIRCLE_CRYPT)
#define CRYPT(a,b) (a)
#else
#define CRYPT(a,b) ((char *) crypt((a),(b)))
#endif

#define SENDOK(ch)	((ch)->desc && (to_sleeping || AWAKE(ch)) && \
			!PLR_FLAGGED((ch), PLR_WRITING))

/* arena */
#define ARENA_OFF	0
#define ARENA_START	1
#define ARENA_RUNNING	2
#define HALL_FAME_FILE "text/hallfame" /* the arena hall of fame */

#define PULSE_ARENA	(75 RL_SEC)

#define MIN_ARENA_COST	1000	/* minimum cost per level */

#define sportschan(buf)   send_to_all(buf)

/* in arena.c */
struct hall_of_fame_element {
   char name[MAX_NAME_LENGTH+1];
   char lastname[MAX_NAME_LENGTH+1];
   time_t date;
   long award;
   struct  hall_of_fame_element *next;
};

void start_arena();
void show_jack_pot();
void do_game();
int num_in_arena();
void find_game_winner();
void do_end_game();
void start_game();
void silent_end();
void write_fame_list(void);
void find_bet_winners(struct char_data *winner);
void write_one_fame_node(FILE *fp, struct hall_of_fame_element *node);
void load_hall_of_fame(void);

extern int in_arena;
extern int game_length;
extern int time_to_start;
extern int time_left_in_game;
extern int lo_lim;
extern int hi_lim;
extern int lo_rem;
extern int hi_rem;
extern int cost_per_lev;
extern int start_time;

extern int leak_logging;

#define HOUSE(ch) (ch->player_specials->saved.house_vnum) /* Fenix - WHS */
