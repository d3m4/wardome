/*#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "interpreter.h"

struct ignore *find_ignore(struct ignore *ignore_list, char *str)
{
  while (ignore_list != NULL) {
    if (*str == *ignore_list->ignore)
      if (!strcmp(str, ignore_list->ignore))
    return ignore_list;
    ignore_list = ignore_list->next;
  }
  return NULL;
}

int is_ignoring(struct char_data *ch, struct char_data *vict)
{
  struct ignore *temp;
  char buf[127];

  sprintf(buf, "%s", GET_NAME(ch));
  temp = GET_IGNORELIST(vict);
  while (temp != NULL) {
    if (!str_cmp(buf, temp->ignore))
      return 1;
  temp = temp->next;
  }
  return 0;
}

void write_ignorelist(struct char_data *ch)
{
  FILE *file;
  char ignoref[127];
  struct ignore *ignoretemp;
  int ignorelength;

  get_filename(GET_NAME(ch), ignoref, IGNORE_FILE);
  unlink(ignoref);
  if (!GET_IGNORELIST(ch))
    return;
  file = fopen(ignoref, "wt");
  ignoretemp = GET_IGNORELIST(ch);
  while(ignoretemp) {
    ignorelength = strlen(ignoretemp->ignore);
    fprintf(file, "%d\n", ignorelength);
    fprintf(file, "%s\n", ignoretemp->ignore);
    ignoretemp = ignoretemp->next;
  }
  fclose(file);
}

void read_ignorelist(struct char_data *ch)
{
  FILE *file;
  char ignoref[127];
  struct ignore *ignoretemp2;
  char buf[127];
  int ignorelength;

  get_filename(GET_NAME(ch), ignoref, IGNORE_FILE);
  file = fopen(ignoref, "r");
  if(!file)
    return;
  CREATE(GET_IGNORELIST(ch), struct ignore, 1);
  ignoretemp2 = GET_IGNORELIST(ch);
  do {
    fscanf(file, "%d\n", &ignorelength);
    fgets(buf, ignorelength + 1, file);
    ignoretemp2->ignore = strdup(buf);
    if(!feof(file)) {
      CREATE(ignoretemp2->next, struct ignore, 1);
      ignoretemp2 = ignoretemp2->next;
    }
  } while(!feof(file));
  fclose(file);
}


*/