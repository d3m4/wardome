/* ************************************************************************
*  file:  autowiz.c                                     Part of CircleMUD *
*  Usage: self-updating wizlists                                          *
*  Written by Jeremy Elson                                                *
*  All Rights Reserved                                                    *
*  Copyright (C) 1993 The Trustees of The Johns Hopkins University        *
************************************************************************* */

/* 
   WARNING:  THIS CODE IS A HACK.  WE CAN NOT AND WILL NOT BE RESPONSIBLE
   FOR ANY NASUEA, DIZZINESS, VOMITING, OR SHORTNESS OF BREATH RESULTING
   FROM READING THIS CODE.  PREGNANT WOMEN AND INDIVIDUALS WITH BACK
   INJURIES, HEART CONDITIONS, OR ARE UNDER THE CARE OF A PHYSICIAN SHOULD
   NOT READ THIS CODE.
   -- The Management
 */

#include "conf.h"
#include "sysdep.h"

#include <signal.h>

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "screen.h"

#define IMM_LMARG "   "
#define IMM_NSIZE  16
#define LINE_LEN   64
#define MIN_LEVEL LVL_IMMORT

struct player_index_element *player_table2 = NULL;       /* index to plr file     */

/* max level that should be in columns instead of centered */
#define COL_LEVEL LVL_IMMORT

struct name_rec {
  char name[25];
  struct name_rec *next;
};

struct control_rec {
  int level;
  char *level_name;
};

struct level_rec {
  struct control_rec *params;
  struct level_rec *next;
  struct name_rec *names;
};

struct control_rec level_params[] =
{
  {LVL_IMMORT, "Immortals"},
  {LVL_ELDER,  "Elders"},
  {LVL_LORD,   "Divinitys"},
  {LVL_DEMIGOD,"Helpers"},
  {LVL_GOD,    "Builders"},
  {LVL_GRGOD,  "Gods"},
  {LVL_SUPGOD, "Supreme Gods"},
  {LVL_MJGOD,  "Major Gods"},
  {LVL_SUBIMPL,"Implementors"},
  {LVL_IMPL,   "Owner Implementors"},
  {0, ""}
};

struct level_rec *levels = 0;

void initialize(void)
{
  struct level_rec *tmp;
  int i = 0;
  
  while (level_params[i].level > 0) {
    tmp = (struct level_rec *) malloc(sizeof(struct level_rec));
    tmp->names = 0;
    tmp->params = &(level_params[i++]);
    tmp->next = levels;
    levels = tmp;
  }
}

int get_line(FILE * fl, char *buf)
{
  char temp[256];
  int lines = 0;

  do {
    lines++;
    fgets(temp, 256, fl);
    if (*temp)
      temp[strlen(temp) - 1] = '\0';
  } while (!feof(fl) && (*temp == '*' || !*temp));

  if (feof(fl)) {
    *buf = '\0';
    return (0);
  } else {
    strcpy(buf, temp);
    return (lines);
  }
}

void read_file(void)
{

  void add_name(int level, char *name);
  int rec_count = 0, i;
  FILE *plr_index;
  char index_name[40], line[256], bits[64];
  char arg2[80];
  long id = 0, last = 0;
  int level = 0;

  sprintf(index_name, "%s", PLR_INDEX_FILE);
  if(!(plr_index = fopen(index_name, "r"))) {
    return;
  }

  while(get_line(plr_index, line))
    if(*line != '~')
      rec_count++;

  rewind(plr_index);

  if(rec_count == 0) {
    return;
  }

  for(i = 0; i < rec_count; i++) {
    get_line(plr_index, line);
    sscanf(line, "%ld %s %d %s %ld", &id, arg2, &level, bits, &last);
    if(level >= MIN_LEVEL)
       add_name(level, arg2);
  }
  fclose(plr_index);

}

void add_name(int level, char *name)
{
  struct name_rec *tmp;
  struct level_rec *curr_level;
  char *ptr;

  if (!*name)
    return;

  for (ptr = name; *ptr; ptr++)
    if (!isalpha(*ptr))
      return;

  tmp = (struct name_rec *) malloc(sizeof(struct name_rec));
  strcpy(tmp->name, name);
  tmp->next = 0;
  curr_level = levels;

  while (curr_level->params->level > level)
    curr_level = curr_level->next;

  tmp->next = curr_level->names;
  curr_level->names = tmp;
}

void sort_names(void)
{
  struct level_rec *curr_level;
  struct name_rec *a, *b;
  char temp[100];

  for (curr_level = levels; curr_level; curr_level = curr_level->next) {
    for (a = curr_level->names; a && a->next; a = a->next) {
      for (b = a->next; b; b = b->next) {
	if (strcmp(a->name, b->name) > 0) {
	  strcpy(temp, a->name);
	  strcpy(a->name, b->name);
	  strcpy(b->name, temp);
	}
      }
    }
  }
}

char *maximo_nome(char *name)
{
    char nome[80];

    strcpy(nome, CAP(name));    
//    strcpy(nome, name);

    return(strdup(nome));
}

void write_wizlist(FILE * out, int minlev, int maxlev)
{
  char buf[100];
  struct level_rec *curr_level;
  struct name_rec *curr_name;
  int i, j;

  fprintf(out,
"\r\n"
"&b-------------------------------------------------------------------------------&n\n"
"&c       The following people have reached the immortality on &CWarDome&c MUD.   &n\n"
"&c    They are to be treated with respect and awe. Occasional prayers to them    &n\n"
"&c              are advisable. Annoying them is not recommended.                 &n\n"
"&c         Their wishes are orders, and - remember - they are the law.           &n\n"
"&b-------------------------------------------------------------------------------&n\n"
"\n\n");

  for (curr_level = levels; curr_level; curr_level = curr_level->next) {
    if (curr_level->params->level < minlev ||
	curr_level->params->level > maxlev)
      continue;

    i = 39 - (strlen(curr_level->params->level_name) >> 1);

    for (j = 1; j <= i; j++)
      fputc(' ', out);

    fprintf(out, "%s\n", curr_level->params->level_name);

    for (j = 1; j <= i; j++)
      fputc(' ', out);

    for (j = 1; j <= strlen(curr_level->params->level_name); j++)
      fputc('~', out);

    fprintf(out, "\n");

    strcpy(buf, "");

    curr_name = curr_level->names;

    while (curr_name) {
      strcat(buf, maximo_nome(curr_name->name));
      if (strlen(buf) > LINE_LEN) {
	if (curr_level->params->level <= COL_LEVEL)
	  fprintf(out, IMM_LMARG);
	else {
	  i = 40 - (strlen(buf) >> 1);
	  for (j = 1; j <= i; j++)
	    fputc(' ', out);
	}
	fprintf(out, "%s\n", buf);
	strcpy(buf, "");
      } else {
	if (curr_level->params->level <= COL_LEVEL) {
	  for (j = 1; j <= (IMM_NSIZE - strlen(curr_name->name)); j++)
	    strcat(buf, " ");
	}
	if (curr_level->params->level > COL_LEVEL)
	  strcat(buf, "   ");
      }
      curr_name = curr_name->next;
    }

    if (*buf) {
      if (curr_level->params->level <= COL_LEVEL)
	fprintf(out, "%s%s\n", IMM_LMARG, buf);
      else {
	i = 40 - (strlen(buf) >> 1);
	for (j = 1; j <= i; j++)
	  fputc(' ', out);
	fprintf(out, "%s\n", buf);
      }
    }
    fprintf(out, "\n");
  }
}

int main(int argc, char **argv)
{
  int wizlevel, immlevel, pid = 0;
  FILE *fl;

  if (argc != 5 && argc != 6) {
    printf("Format: %s wizlev wizlistfile immlev immlistfile [pid to signal]\n",
	   argv[0]);
    exit(0);
  }
  wizlevel = atoi(argv[1]);
  immlevel = atoi(argv[3]);

  if (argc == 6)
    pid = atoi(argv[5]);

  initialize();
  read_file();
  sort_names();

  fl = fopen(argv[2], "w");
  write_wizlist(fl, wizlevel, LVL_IMPL);
  fclose(fl);
  fl = fopen(argv[4], "w");
  write_wizlist(fl, immlevel, wizlevel - 1);

  fclose(fl);

  if (pid)
    kill(pid, SIGUSR1);

  return 0;
}
