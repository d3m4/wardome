/************************************************************************
 * Quest system - quest.c					v1.6	*
 ************************************************************************/

#include "conf.h"
#include "sysdep.h"
#include <signal.h>

#include "structs.h"
#include "buffer.h"
#include "utils.h"
#include "interpreter.h"
#include "db.h"
#include "handler.h"
#include "comm.h"
#include "olc.h"
#include "quest.h"

extern struct aq_data *aquest_table;
extern struct zone_data *zone_table;
extern struct index_data *mob_index;
extern int top_of_aquestt;
extern struct index_data *obj_index;
extern struct room_data *world;
extern long asciiflag_conv(char *flag);
extern struct player_index_element *player_table;

void smash_tilde(char *string);
void qedit_save_to_disk(int zone_num);
void free_quest(struct aq_data *quest);
int find_name(char *name);

const char *quest_types[] =
{
  "Object",
  "Room",
  "Find mob",
  "Kill mob",
  "Save mob",
  "Return object",
  "\n"
};

const char *reward_types[] =
{
  "Experience",
  "Object",
  "Money",
  "Quest Points",
  "\n"
};

/* Autoquest flags */
char *aq_flags[] =
{
  "REPEATABLE",
  "\n"
};

int real_quest(int vnum)
{
  int rnum;

  for (rnum = 0; rnum <= top_of_aquestt; rnum++)
	if (rnum <= top_of_aquestt)
		if (aquest_table[rnum].virtual == vnum)
			return (rnum);

  return (-1);
}

int is_complete(int vnum)
{
	int qual;
	
	if((qual = real_quest(vnum)) == -1)
		return FALSE;

	if (aquest_table[real_quest(vnum)].complete == 1)
		return TRUE;

  	return FALSE;
}

int find_quest_by_qmnum(int qm, int num)
{
  int i;
  int found = 0;

  for (i = 0; i <= top_of_aquestt; i++)
    if (qm == aquest_table[i].mob_vnum) {
      found++;
      if (found == num)
	return (aquest_table[i].virtual);
    }
  return -1;
}

/* PARSE_QUEST */
void parse_quest(FILE * quest_f, int nr)
{
  static char line[256];
  static int i = 0, j;
  int retval = 0, t[10];
  char f1[128];

//  sprintf(debug, "Parsing quest #%d", nr);
  //  log(debug);

  aquest_table[i].virtual = nr;
  aquest_table[i].mob_vnum = -1;
  aquest_table[i].short_desc = NULL;
  aquest_table[i].desc = NULL;
  aquest_table[i].info = NULL;
  aquest_table[i].ending = NULL;
  aquest_table[i].flags = -1;
  aquest_table[i].type = -1;
  aquest_table[i].target = -1;
  aquest_table[i].reward_type = 0;
  aquest_table[i].reward = 0;
  for (j = 0; j < 4; j++)
    aquest_table[i].value[j] = 0;
  aquest_table[i].next_quest = -1;
  aquest_table[i].complete = -1;
  aquest_table[i].min_level = 0;
  aquest_table[i].max_level = 0;
  aquest_table[i].id = -1;
  
  /* begin to parse the data */
  aquest_table[i].short_desc = fread_string(quest_f, buf2);
  aquest_table[i].desc = fread_string(quest_f, buf2);
  aquest_table[i].info = fread_string(quest_f, buf2);
  aquest_table[i].ending = fread_string(quest_f, buf2);

  if (!get_line(quest_f, line) ||
      (retval = sscanf(line, " %d %d %s %d %d %d %d %d %d %d %d", t, t + 1, f1, t + 2, t + 3, t + 4, t + 5, t + 6, t + 7, t + 8, t + 9)) != 11) {
    fprintf(stderr, "Format error in numeric line (expected 11, got %d), %s\n", retval, buf2);
    raise(SIGSEGV);
  }
  aquest_table[i].type = t[0];
  aquest_table[i].mob_vnum = t[1];
  aquest_table[i].flags = asciiflag_conv(f1);
  aquest_table[i].target = t[2];
  aquest_table[i].reward_type = t[3];
  aquest_table[i].reward = t[4];
  aquest_table[i].next_quest = t[5];
  aquest_table[i].complete = t[6];
  aquest_table[i].min_level = t[7];
  aquest_table[i].max_level = t[8];
  aquest_table[i].id = t[9];
    
  if (!get_line(quest_f, line) ||
  (retval = sscanf(line, " %d %d %d %d", t, t + 1, t + 2, t + 3)) != 4) {
    fprintf(stderr, "Format error in numeric line (expected 4, got %d), %s\n", retval, buf2);
    raise(SIGSEGV);
  }
  aquest_table[i].value[0] = t[0];
  aquest_table[i].value[1] = t[1];
  aquest_table[i].value[2] = t[2];
  aquest_table[i].value[3] = t[3];

  for (;;) {
    if (!get_line(quest_f, line)) {
      fprintf(stderr, "Format error in %s\n", buf2);
      raise(SIGSEGV);
    }
    switch (*line) {
    case 'S':
      top_of_aquestt = i++;
      return;
    }
  }
}

void list_quests(struct char_data *ch, int questmaster)
{
  int i;
  int number = 1;
  char envia[100];
  
  sprintf(buf, "\r\nYou can join this quests, choose with care one of them because after join you\r\n"
               "can't get out. You have two ways to get out, completing the quest or if another\r\n"
               "player complete the quest before you.\r\n\r\n"
               "To join type: join <num>\r\n"
	       "To get info: list <num>\r\n"
	       "---  ------------------------------------------------------------------------\r\n"
	       "&RNum  &WDescription&n\r\n"
	       "---  ------------------------------------------------------------------------\r\n");

  for (i = 0; i <= top_of_aquestt; i++)
    if (questmaster == aquest_table[i].mob_vnum) {
  
      if(get_name_by_id(aquest_table[i].id) != NULL)
	  sprintf(envia, "&c(completed by &R%s&c)&n", get_name_by_id(aquest_table[i].id));
      else
          strcpy(envia, "&c(completed)&n");
        
      sprintf(buf+strlen(buf), "&R%3d  &W%s %s &C[&c%d &Cto &c%d&C]&n\r\n", number, aquest_table[i].desc,
	  is_complete(aquest_table[i].virtual) ? envia : "", aquest_table[i].min_level,
	  aquest_table[i].max_level);
      number++;
    }
  page_string(ch->desc, buf, 1);
}

void quest_save_internally(int num, struct char_data *ch)
{
  struct aq_data *quest;
  int i, real_num;

  real_num = real_quest(num);

  /*. Build a copy of the quest . */
  CREATE(quest, struct aq_data, 1);
  *quest = aquest_table[real_num];
  /* allocate space for all strings  */
  if (aquest_table[real_num].short_desc)
    quest->short_desc = str_dup(aquest_table[real_num].short_desc);
  if (aquest_table[real_num].desc)
    quest->desc = str_dup(aquest_table[real_num].desc);
  if (aquest_table[real_num].info)
    quest->info = str_dup(aquest_table[real_num].info);
  if (aquest_table[real_num].ending)
    quest->ending = str_dup(aquest_table[real_num].ending);
  quest->type = aquest_table[real_num].type;
  quest->mob_vnum = aquest_table[real_num].mob_vnum;
  quest->flags = aquest_table[real_num].flags;
  quest->target = aquest_table[real_num].target;
  quest->reward_type = aquest_table[real_num].reward_type;
  quest->reward = aquest_table[real_num].reward;
  quest->next_quest = aquest_table[real_num].next_quest;
  quest->complete = 1;
  quest->min_level = aquest_table[real_num].min_level;
  quest->max_level = aquest_table[real_num].max_level;
  quest->id = player_table[find_name(GET_NAME(ch))].id;
  
  for (i = 0; i < 4; i++)
    quest->value[i] = aquest_table[real_num].value[i];

  free_quest(aquest_table + real_num);
  aquest_table[real_num] = *(quest);

  olc_add_to_save_list(zone_table[num/100].number, OLC_SAVE_QUEST);
}


void add_completed_quest(struct char_data * ch, int num)
{

   sprintf(buf, "(QUEST) %s has completed the quest %d.", GET_NAME(ch), num);
   mudlog(buf, BRF, LVL_OLC_FULL, TRUE);

   quest_save_internally(num, ch);
   qedit_save_to_disk(num/100);
   
}

/* Generic reward character, cleanup stuff fn */
void generic_complete_quest(struct char_data *ch)
{
  int rnum, r_num;
  struct obj_data *obj;

  rnum = real_quest(GET_QUEST(ch));

  switch (aquest_table[rnum].reward_type) {
  case REWARD_EXP:
    GET_EXP(ch) += aquest_table[rnum].reward;
    sprintf(buf, "Your reward is %s points of experience.\r\n", add_points(aquest_table[rnum].reward));
    send_to_char(buf, ch);
    break;
  case REWARD_OBJ:
    if ((r_num = real_object(aquest_table[rnum].reward)) < 0) {
      log("Quest #%d attempted to load a reward obj that doesnt exist!", aquest_table[rnum].virtual);
      return;
    }
    obj = read_object(r_num, REAL);
    obj_to_char(obj, ch);
    sprintf(buf, "Your reward is %s.\r\n", obj->short_description);
    send_to_char(buf, ch);
    break;
  case REWARD_GOLD:
    GET_GOLD(ch) += aquest_table[rnum].reward;
    sprintf(buf, "Your reward is %s gold coins.\r\n", add_points(aquest_table[rnum].reward));
    send_to_char(buf, ch);
    break;
  case REWARD_QP:
    GET_QP(ch) += aquest_table[rnum].reward;
    sprintf(buf, "Your reward is %s questpoints.\r\n", add_points(aquest_table[rnum].reward));
    send_to_char(buf, ch);
    break;
  default:
    log("Quest #%d has an invalid reward type!", aquest_table[rnum].virtual);
    break;
  }
  send_to_char(aquest_table[rnum].ending, ch);
  if (IS_SET(aquest_table[rnum].flags, AQ_REPEATABLE))
    add_completed_quest(ch, GET_QUEST(ch));
  if ((real_quest(aquest_table[rnum].next_quest) >= 0) &&
      (aquest_table[rnum].next_quest != GET_QUEST(ch)) &&
      !is_complete(aquest_table[rnum].next_quest)) {
    GET_QUEST(ch) = aquest_table[real_quest((int) GET_QUEST(ch))].next_quest;
    send_to_char(aquest_table[real_quest((int) GET_QUEST(ch))].info, ch);
  } else
    GET_QUEST(ch) = -1;

  save_char(ch, ch->in_room);
}

void autoquest_trigger_check(struct char_data *ch, struct char_data *vict,
			     struct obj_data *object, int type)
{
  struct char_data *i;
  int rnum, found;

  if (IS_NPC(ch))
    return;

  if (GET_QUEST(ch) < 0)	/* No current quest, skip this */
    return;

  if (GET_QUEST_TYPE(ch) != type)
    return;

  if ((rnum = real_quest(GET_QUEST(ch))) < 0)
    return;

  switch (type) {
  case AQ_OBJECT:
    if (aquest_table[rnum].target == GET_OBJ_VNUM(object))
      generic_complete_quest(ch);
    break;
  case AQ_ROOM:
    if (aquest_table[rnum].target == world[ch->in_room].number)
      generic_complete_quest(ch);
    break;
  case AQ_MOB_FIND:
    for (i = world[ch->in_room].people; i; i = i->next_in_room)
      if (IS_NPC(i))
	if (aquest_table[rnum].target == GET_MOB_VNUM(i))
	  generic_complete_quest(ch);
    break;
  case AQ_MOB_KILL:
    if (!IS_NPC(ch) && IS_NPC(vict) && (ch != vict))
      if (aquest_table[rnum].target == GET_MOB_VNUM(vict))
	generic_complete_quest(ch);
    break;
  case AQ_MOB_SAVE:
    found = TRUE;
    if (ch == vict)
      found = FALSE;

    for (i = world[ch->in_room].people; i && found; i = i->next_in_room)
      if (i && IS_NPC(i))
	if ((GET_MOB_VNUM(i) != aquest_table[rnum].target) && !AFF_FLAGGED(i, AFF_CHARM))
	  found = FALSE;

    if (found)
      generic_complete_quest(ch);
    break;
  case AQ_RETURN_OBJ:
    if (IS_NPC(vict) && (GET_MOB_VNUM(vict) == aquest_table[rnum].value[0]))
      if (object && (GET_OBJ_VNUM(object) == aquest_table[rnum].target))
	generic_complete_quest(ch);
    break;
  default:
    log("SYSERR: Invalid quest type passed to autoquest_trigger_check");
    break;
  }
}

int can_join(struct char_data *ch, int tmp)
{
    if(GET_LEVEL(ch) > aquest_table[real_quest(tmp)].min_level &&
       GET_LEVEL(ch) < aquest_table[real_quest(tmp)].max_level)
          return (1);

    return (0);
}

SPECIAL(questmaster)
{
  int tmp, num;
  struct char_data *qm = me;

  if (CMD_IS("list")) {
    if (!*argument)
      list_quests(ch, GET_MOB_VNUM(qm));
    else {
      if ((num = find_quest_by_qmnum(GET_MOB_VNUM(qm), atoi(argument))) >= 0) {
	if (aquest_table[real_quest(num)].info)
	  send_to_char(aquest_table[real_quest(num)].info, ch);
	else
	  send_to_char("There is no further information on that quest.\r\n", ch);
      } else
	send_to_char("That is not a valid quest number!\r\n", ch);
    }
    return 1;
  }
  if (CMD_IS("join")) {
    if (!*argument) {
      send_to_char("Join what quest?\r\n", ch);
      return 1;
    }
    if (GET_QUEST(ch) != -1) {
      send_to_char("You are already part of a quest!\r\n", ch);
      return 1;
    }
    tmp = atoi(argument);
    tmp = find_quest_by_qmnum(GET_MOB_VNUM(qm), tmp);
    if ((tmp >= 0) && !is_complete(tmp) && can_join(ch, tmp)) {
      send_to_char(aquest_table[real_quest(tmp)].info, ch);
      send_to_char("You are now part of that quest!\r\n", ch);
      GET_QUEST(ch) = tmp;
      save_char(ch, ch->in_room);
      return 1;
    } else if (is_complete(tmp)) {
      send_to_char("This quest are already completed!\r\n", ch);
      return 1;
    } else if (!can_join(ch, tmp)) {
      send_to_char("You need be in the quest range to join.\r\n", ch);
      return 1;
    } else {
      send_to_char("That is not a valid quest!\r\n", ch);
      return 1;
    }
  }
  return 0;
}

ACMD(do_qstat)
{
  int vnum, rnum;
  char str[MAX_INPUT_LENGTH];
  struct char_data *tmp;

  if (GET_LEVEL(ch) < LVL_OLC_FULL){
   if(GET_OLC_ZONE(ch) != QEDIT_PERMISSION){
    send_to_char("You do not have permission to edit quests.\r\n", ch);
    return;
   }
  }
  
  half_chop(argument, str, argument);
  if (*str) {
    vnum = atoi(str);
    rnum = real_quest(vnum);
    if (rnum < 0) {
      send_to_char("That vnum does not exist.\r\n", ch);
      return;
    }
    *buf = '\0';
    tmp = read_mobile(aquest_table[rnum].mob_vnum, VIRTUAL);
    sprintf(buf, "VNum: [%5d], RNum: [%5d] -- Questmaster: %s\r\n",
	    vnum, rnum, GET_NAME(tmp));
    sprintf(buf, "%sName: %s\r\n", buf, aquest_table[rnum].short_desc);
    sprintf(buf, "%sDesc: %s\r\n", buf, aquest_table[rnum].desc);
    sprintf(buf, "%sInformation:\r\n%s", buf, aquest_table[rnum].info);
    sprintf(buf, "%sEnding:\r\n%s", buf, aquest_table[rnum].ending);
    sprintf(buf, "%sType : %s, Target: %d, Reward Type: %s\r\n Reward: %d, Next quest: %d\r\n",
    buf, quest_types[aquest_table[rnum].type], aquest_table[rnum].target,
	    reward_types[aquest_table[rnum].reward_type],
	    aquest_table[rnum].reward, aquest_table[rnum].next_quest);
    sprintf(buf, "%sLevel Range: %d to %d\r\n", buf, aquest_table[rnum].min_level,
	    aquest_table[rnum].max_level);
    sprintf(buf, "%sValue: %d %d %d %d\r\n", buf, aquest_table[rnum].value[0],
	    aquest_table[rnum].value[1], aquest_table[rnum].value[2],
	    aquest_table[rnum].value[3]);
    sprintf(buf, "%sFlags: %s\r\n", buf, aq_flags[aquest_table[rnum].flags]);
    send_to_char(buf, ch);

  } else
    send_to_char("Usage: qstat <vnum>\r\n", ch);
}

ACMD(do_qlist)
{

  int first, last, nr, found = 0;
  char pagebuf[65536];

  strcpy(pagebuf, "");

  two_arguments(argument, buf, buf2);

  if (!*buf) {
    send_to_char("Usage: qlist <begining number or zone> [<ending number>]\r\n", ch);
    return;
  }
  first = atoi(buf);
  if (*buf2)
    last = atoi(buf2);
  else {
    first *= 100;
    last = first + 99;
  }

  if ((first < 0) || (first > 9999) || (last < 0) || (last > 99999)) {
    send_to_char("Values must be between 0 and 99999.\n\r", ch);
    return;
  }
  if (first >= last) {
    send_to_char("Second value must be greater than first.\n\r", ch);
    return;
  }
  for (nr = 0; nr <= top_of_aquestt && (aquest_table[nr].virtual <= last); nr++) {
    if (aquest_table[nr].virtual >= first) {
      sprintf(buf, "%5d. [%5d] %s\r\n", ++found,
	      aquest_table[nr].virtual,
	      aquest_table[nr].short_desc);
      strcat(pagebuf, buf);
    }
  }

  if (!found)
    send_to_char("No quests were found in those parameters.\n\r", ch);
  else
    page_string(ch->desc, pagebuf, TRUE);
}

/* reads quest from mob file, and attaches it */
void read_quest(FILE * fp, int i)
{
  char line[256];
  char junk[8];
  int vnum, count;

  get_line(fp, line);
  count = sscanf(line, "%s %d", junk, &vnum);

  if (count != 2) {
    /* should do a better job of making this message */
    log("SYSERR: Error assigning quest!");
    return;
  }
  mob_index[i].func = questmaster;
}

void quest_save_to_file(FILE * fp, struct char_data *mob)
{
  if (mob_index[mob->nr].func == (questmaster))
    fprintf(fp, "Q 1\n");
}
