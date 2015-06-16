/**************************************************************************
*   File: modify.c                                      Part of CircleMUD *
*  Usage: Run-time modification of game variables                         *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
**************************************************************************/

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "buffer.h"
#include "utils.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "comm.h"
#include "spells.h"
#include "mail.h"
#include "boards.h"
#include "olc.h"

void show_string(struct descriptor_data *d, char *input);

extern char *spells[];
extern char *MENU;
char *stripcr(char *dest, const char *src);
void qedit_disp_menu(struct descriptor_data *d);

/* local functions */
void string_add(struct descriptor_data *d, char *str);
ACMD(do_skillset);
char *next_page(char *str);
int count_pages(char *str);
void paginate_string(char *str, struct descriptor_data *d);

/*
 * Action modes for parse_action().
 */
#define PARSE_FORMAT            0
#define PARSE_REPLACE           1
#define PARSE_HELP              2
#define PARSE_DELETE            3
#define PARSE_INSERT            4
#define PARSE_LIST_NORM         5
#define PARSE_LIST_NUM          6
#define PARSE_EDIT              7

const char *string_fields[] =
{
  "name",
  "short",
  "long",
  "description",
  "title",
  "delete-description",
  "\n"
};

/*
 * Maximum length for text field x+1
 */
int length[] =
{
  15,
  60,
  256,
  240,
  60
};


/*************************************
 * Modification of malloc'ed strings.*
 *************************************/

/*
 * Handle some editor commands.
 */
void parse_action(int command, char *string, struct descriptor_data *d)
{
  int indent = 0, rep_all = 0, flags = 0, total_len, replaced;
  int j = 0;
  int i, line_low, line_high;
  char *s, *t, temp;

  switch (command) {
  case PARSE_HELP:
    sprintf(buf,
          "Editor command formats: /<letter>\r\n\r\n"
          "/a         -  aborts editor\r\n"
          "/c         -  clears buffer\r\n"
          "/d#        -  deletes a line #\r\n"
          "/e# <text> -  changes the line at # with <text>\r\n"
          "/f         -  formats text\r\n"
          "/fi        -  indented formatting of text\r\n"
          "/h         -  list text editor commands\r\n"
          "/i# <text> -  inserts <text> before line #\r\n"
          "/l         -  lists buffer\r\n"
          "/n         -  lists buffer with line numbers\r\n"
          "/r 'a' 'b' -  replace 1st occurance of text <a> in buffer with text <b>\r\n"
          "/ra 'a' 'b'-  replace all occurances of text <a> within buffer with text <b>\r\n"
          "              usage: /r[a] 'pattern' 'replacement'\r\n"
          "/s         -  saves text\r\n");
    SEND_TO_Q(buf, d);
    break;
  case PARSE_FORMAT:
    while (isalpha(string[j]) && j < 2) {
      switch (string[j]) {
      case 'i':
      if (!indent) {
        indent = TRUE;
        flags += FORMAT_INDENT;
      }
      break;
      default:
      break;
      }
      j++;
    }
    format_text(d->str, flags, d, d->max_str);
    sprintf(buf, "Text formatted with%s indent.\r\n", (indent ? "" : "out"));
    SEND_TO_Q(buf, d);
    break;
  case PARSE_REPLACE:
    while (isalpha(string[j]) && j < 2) {
      switch (string[j]) {
      case 'a':
      if (!indent)
        rep_all = 1;
      break;
      default:
      break;
      }
      j++;
    }
    if ((s = strtok(string, "'")) == NULL) {
      SEND_TO_Q("Invalid format.\r\n", d);
      return;
    } else if ((s = strtok(NULL, "'")) == NULL) {
      SEND_TO_Q("Target string must be enclosed in single quotes.\r\n", d);
      return;
    } else if ((t = strtok(NULL, "'")) == NULL) {
      SEND_TO_Q("No replacement string.\r\n", d);
      return;
    } else if ((t = strtok(NULL, "'")) == NULL) {
      SEND_TO_Q("Replacement string must be enclosed in single quotes.\r\n", d);
      return;
    } else if ((total_len = ((strlen(t) - strlen(s)) + strlen(*d->str))) <= d->max_str) {
      if ((replaced = replace_str(d->str, s, t, rep_all, d->max_str)) > 0) {
      sprintf(buf, "Replaced %d occurance%sof '%s' with '%s'.\r\n", replaced,
                ((replaced != 1) ? "s " : " "), s, t);
      SEND_TO_Q(buf, d);
      } else if (replaced == 0) {
      sprintf(buf, "String '%s' not found.\r\n", s);
      SEND_TO_Q(buf, d);
      } else
      SEND_TO_Q("ERROR: Replacement string causes buffer overflow, aborted replace.\r\n", d);
    } else
      SEND_TO_Q("Not enough space left in buffer.\r\n", d);
    break;
  case PARSE_DELETE:
    switch (sscanf(string, " %d - %d ", &line_low, &line_high)) {
    case 0:
      SEND_TO_Q("You must specify a line number or range to delete.\r\n", d);
      return;
    case 1:
      line_high = line_low;
      break;
    case 2:
      if (line_high < line_low) {
      SEND_TO_Q("That range is invalid.\r\n", d);
      return;
      }
      break;
    }

    i = 1;
    total_len = 1;
    if ((s = *d->str) == NULL) {
      SEND_TO_Q("Buffer is empty.\r\n", d);
      return;
    } else if (line_low > 0) {
      while (s && (i < line_low))
      if ((s = strchr(s, '\n')) != NULL) {
        i++;
        s++;
      }
      if ((i < line_low) || (s == NULL)) {
      SEND_TO_Q("Line(s) out of range; not deleting.\r\n", d);
      return;
      }
      t = s;
      while (s && (i < line_high))
      if ((s = strchr(s, '\n')) != NULL) {
        i++;
        total_len++;
        s++;
      }
      if ((s) && ((s = strchr(s, '\n')) != NULL)) {
      s++;
      while (*s != '\0')
        *(t++) = *(s++);
      } else
      total_len--;
      *t = '\0';
      RECREATE(*d->str, char, strlen(*d->str) + 3);
      sprintf(buf, "%d line%sdeleted.\r\n", total_len,
            ((total_len != 1) ? "s " : " "));
      SEND_TO_Q(buf, d);
    } else {
      SEND_TO_Q("Invalid line numbers to delete must be higher than 0.\r\n", d);
      return;
    }
    break;
  case PARSE_LIST_NORM:
    /*
     * Note: Rv's buf, buf1, buf2, and arg variables are defined to 32k so
     * they are probly ok for what to do here.
     */
    *buf = '\0';
    if (*string != '\0')
      switch (sscanf(string, " %d - %d ", &line_low, &line_high)) {
      case 0:
      line_low = 1;
      line_high = 999999;
      break;
      case 1:
      line_high = line_low;
      break;
    } else {
      line_low = 1;
      line_high = 999999;
    }

    if (line_low < 1) {
      SEND_TO_Q("Line numbers must be greater than 0.\r\n", d);
      return;
    } else if (line_high < line_low) {
      SEND_TO_Q("That range is invalid.\r\n", d);
      return;
    }
    *buf = '\0';
    if ((line_high < 999999) || (line_low > 1))
      sprintf(buf, "Current buffer range [%d - %d]:\r\n", line_low, line_high);
    i = 1;
    total_len = 0;
    s = *d->str;
    while (s && (i < line_low))
      if ((s = strchr(s, '\n')) != NULL) {
      i++;
      s++;
      }
    if ((i < line_low) || (s == NULL)) {
      SEND_TO_Q("Line(s) out of range; no buffer listing.\r\n", d);
      return;
    }
    t = s;
    while (s && (i <= line_high))
      if ((s = strchr(s, '\n')) != NULL) {
      i++;
      total_len++;
      s++;
      }
    if (s) {
      temp = *s;
      *s = '\0';
      strcat(buf, t);
      *s = temp;
    } else
      strcat(buf, t);
    /*
     * This is kind of annoying...but some people like it.
     */
#if 0
    sprintf(buf, "%s\r\n%d line%sshown.\r\n", buf, total_len,
      ((total_len != 1)?"s ":" "));
#endif
    SEND_TO_Q(buf, d);
//    page_string(d, buf, TRUE);
    break;
  case PARSE_LIST_NUM:
    /*
     * Note: Rv's buf, buf1, buf2, and arg variables are defined to 32k so
     * they are probly ok for what to do here.
     */
    *buf = '\0';
    if (*string != '\0')
      switch (sscanf(string, " %d - %d ", &line_low, &line_high)) {
      case 0:
      line_low = 1;
      line_high = 999999;
      break;
      case 1:
      line_high = line_low;
      break;
    } else {
      line_low = 1;
      line_high = 999999;
    }

    if (line_low < 1) {
      SEND_TO_Q("Line numbers must be greater than 0.\r\n", d);
      return;
    }
    if (line_high < line_low) {
      SEND_TO_Q("That range is invalid.\r\n", d);
      return;
    }
    *buf = '\0';
    i = 1;
    total_len = 0;
    s = *d->str;
    while (s && (i < line_low))
      if ((s = strchr(s, '\n')) != NULL) {
      i++;
      s++;
      }
    if ((i < line_low) || (s == NULL)) {
      SEND_TO_Q("Line(s) out of range; no buffer listing.\r\n", d);
      return;
    }
    t = s;
    while (s && (i <= line_high))
      if ((s = strchr(s, '\n')) != NULL) {
      i++;
      total_len++;
      s++;
      temp = *s;
      *s = '\0';
      sprintf(buf, "%s%4d:\r\n", buf, (i - 1));
      strcat(buf, t);
      *s = temp;
      t = s;
      }
    if (s && t) {
      temp = *s;
      *s = '\0';
      strcat(buf, t);
      *s = temp;
    } else if (t)
      strcat(buf, t);
    /*
     * This is kind of annoying .. seeing as the lines are numbered.
     */
#if 0
     sprintf(buf, "%s\r\n%d numbered line%slisted.\r\n", buf, total_len,
      ((total_len != 1)?"s ":" "));
#endif
    SEND_TO_Q(buf, d);
//    page_string(d, buf, TRUE);
    break;

  case PARSE_INSERT:
    half_chop(string, buf, buf2);
    if (*buf == '\0') {
      SEND_TO_Q("You must specify a line number before which to insert text.\r\n", d);
      return;
    }
    line_low = atoi(buf);
    strcat(buf2, "\r\n");

    i = 1;
    *buf = '\0';
    if ((s = *d->str) == NULL) {
      SEND_TO_Q("Buffer is empty, nowhere to insert.\r\n", d);
      return;
    }
    if (line_low > 0) {
      while (s && (i < line_low))
      if ((s = strchr(s, '\n')) != NULL) {
        i++;
        s++;
      }
      if ((i < line_low) || (s == NULL)) {
        SEND_TO_Q("Line number out of range; insert aborted.\r\n", d);
        return;
      }
      temp = *s;
      *s = '\0';
      if ((strlen(*d->str) + strlen(buf2) + strlen(s + 1) + 3) > d->max_str) {
        *s = temp;
        SEND_TO_Q("Insert text pushes buffer over maximum size, insert aborted.\r\n", d);
        return;
      }
      if (*d->str && (**d->str != '\0'))
      strcat(buf, *d->str);
      *s = temp;
      strcat(buf, buf2);
      if (s && (*s != '\0'))
      strcat(buf, s);
      RECREATE(*d->str, char, strlen(buf) + 3);
      strcpy(*d->str, buf);
      SEND_TO_Q("Line inserted.\r\n", d);
    } else {
      SEND_TO_Q("Line number must be higher than 0.\r\n", d);
      return;
    }
    break;

  case PARSE_EDIT:
    half_chop(string, buf, buf2);
    if (*buf == '\0') {
      SEND_TO_Q("You must specify a line number at which to change text.\r\n", d);
      return;
    }
    line_low = atoi(buf);
    strcat(buf2, "\r\n");

    i = 1;
    *buf = '\0';
    if ((s = *d->str) == NULL) {
      SEND_TO_Q("Buffer is empty, nothing to change.\r\n", d);
      return;
    }
    if (line_low > 0) {
      /*
       * Loop through the text counting \\n characters until we get to the line/
       */
      while (s && (i < line_low))
      if ((s = strchr(s, '\n')) != NULL) {
        i++;
        s++;
      }
      /*
       * Make sure that there was a THAT line in the text.
       */
      if ((i < line_low) || (s == NULL)) {
      SEND_TO_Q("Line number out of range; change aborted.\r\n", d);
      return;
      }
      /*
       * If s is the same as *d->str that means I'm at the beginning of the
       * message text and I don't need to put that into the changed buffer.
       */
      if (s != *d->str) {
      /*
       * First things first .. we get this part into the buffer.
       */
      temp = *s;
      *s = '\0';
      /*
       * Put the first 'good' half of the text into storage.
       */
      strcat(buf, *d->str);
      *s = temp;
      }
      /*
       * Put the new 'good' line into place.
       */
      strcat(buf, buf2);
      if ((s = strchr(s, '\n')) != NULL) {
      /*
       * This means that we are at the END of the line, we want out of
       * there, but we want s to point to the beginning of the line
       * AFTER the line we want edited
       */
      s++;
      /*
       * Now put the last 'good' half of buffer into storage.
       */
      strcat(buf, s);
      }
      /*
       * Check for buffer overflow.
       */
      if (strlen(buf) > d->max_str) {
      SEND_TO_Q("Change causes new length to exceed buffer maximum size, aborted.\r\n", d);
      return;
      }
      /*
       * Change the size of the REAL buffer to fit the new text.
       */
      RECREATE(*d->str, char, strlen(buf) + 3);
      strcpy(*d->str, buf);
      SEND_TO_Q("Line changed.\r\n", d);
    } else {
      SEND_TO_Q("Line number must be higher than 0.\r\n", d);
      return;
    }
    break;
  default:
    SEND_TO_Q("Invalid option.\r\n", d);
    mudlog("SYSERR: invalid command passed to parse_action", BRF, LVL_IMPL, TRUE);
    return;
  }
}

/*
 * Add user input to the 'current' string (as defined by d->str).
 */
  void string_add(struct descriptor_data *d, char *str)
  {
  FILE *fl;
  int terminator = 0, action = 0;
  int i = 2, j = 0;
  char actions[MAX_INPUT_LENGTH];
  extern char *MENU;
  extern struct message_data mess_info;
  extern void write_message_to_file(void);
  /*
   * Determine if this is the terminal string, and truncate if so.
   * Changed to only accept '@' at the beginning of line - J. Elson 1/17/94
   * Changed to accept '/<letter>' style editing commands - instead
   *   of solitary '@' to end. - M. Scott 10/15/96
   */

    delete_doubledollar(str);

#if 0
  /*
   * Removed old handling of '@' character, put #if 1 to re-enable it.
   */
    if ((terminator = (*str == '@')))
      *str = '\0';
#endif

  if ((action = (*str == '/'))) {
    while (str[i] != '\0') {
      actions[j] = str[i];
      i++;
      j++;
    }
    actions[j] = '\0';
    *str = '\0';
    switch (str[1]) {
    case 'a':
      terminator = 2;         /* Working on an abort message, */
      break;
    case 'c':
      if (*(d->str)) {
//      really_free(*d->str);
      free(*d->str);
      *(d->str) = NULL;
      SEND_TO_Q("Current buffer cleared.\r\n", d);
      } else
      SEND_TO_Q("Current buffer empty.\r\n", d);
      break;
    case 'd':
      parse_action(PARSE_DELETE, actions, d);
      break;
    case 'e':
      parse_action(PARSE_EDIT, actions, d);
      break;
    case 'f':
      if (*(d->str))
      parse_action(PARSE_FORMAT, actions, d);
      else
      SEND_TO_Q("Current buffer empty.\r\n", d);
      break;
    case 'i':
      if (*(d->str))
      parse_action(PARSE_INSERT, actions, d);
      else
      SEND_TO_Q("Current buffer empty.\r\n", d);
      break;
    case 'h':
      parse_action(PARSE_HELP, actions, d);
      break;
    case 'l':
      if (*d->str)
      parse_action(PARSE_LIST_NORM, actions, d);
      else
      SEND_TO_Q("Current buffer empty.\r\n", d);
      break;
    case 'n':
      if (*d->str)
      parse_action(PARSE_LIST_NUM, actions, d);
      else
      SEND_TO_Q("Current buffer empty.\r\n", d);
      break;
    case 'r':
      parse_action(PARSE_REPLACE, actions, d);
      break;
    case 's':
      terminator = 1;
      *str = '\0';
      break;
    default:
      SEND_TO_Q("Invalid option.\r\n", d);
      break;
    }
  }

    if (!(*d->str)) {
      if (strlen(str) > d->max_str) {
        send_to_char("String too long - Truncated.\r\n",
                   d->character);
        *(str + d->max_str) = '\0';
      /*
       * Changed this to NOT abort out.. just give warning.
       * terminator = 1;
       */
      }
      CREATE(*d->str, char, strlen(str) + 3);
      strcpy(*d->str, str);
    } else {
      if (strlen(str) + strlen(*d->str) > d->max_str) {
	      send_to_char("String too long, limit reached on message. Last line ignored.\r\n",
	                 d->character);
	      terminator = 1;

      } else {
/*      if (!(*d->str = (char *) realloc(*d->str, strlen(*d->str) +
                                       strlen(str) + 3))) {
       	perror("SYSERR: string_add");
        exit(1);*/
        RECREATE(*d->str, char, strlen(*d->str)+strlen(str)+3);
//      }
      strcat(*d->str, str);
      }
    }
    if (terminator) {
    /*
     * OLC Edits
     */
    extern void oedit_disp_menu(struct descriptor_data *d);
    extern void oedit_disp_extradesc_menu(struct descriptor_data *d);
    extern void redit_disp_menu(struct descriptor_data *d);
    extern void redit_disp_extradesc_menu(struct descriptor_data *d);
    extern void redit_disp_exit_menu(struct descriptor_data *d);
    extern void medit_disp_menu(struct descriptor_data *d);
    extern void hedit_disp_menu(struct descriptor_data *d);
    extern void trigedit_disp_menu(struct descriptor_data *d);

#if defined(OASIS_MPROG)
    extern void medit_change_mprog(struct descriptor_data *d);

    if (STATE(d) == CON_MEDIT) {
      switch (OLC_MODE(d)) {
      case MEDIT_D_DESC:
      medit_disp_menu(d);
      break;
      case MEDIT_MPROG_COMLIST:
      medit_change_mprog(d);
      break;
      }
    }
#endif

    /*
     * Here we check for the abort option and reset the pointers.
     */
    if ((terminator == 2) &&
      ((STATE(d) == CON_REDIT) ||
       (STATE(d) == CON_HEDIT) ||
       (STATE(d) == CON_MEDIT) ||
       (STATE(d) == CON_OEDIT) ||
       (STATE(d) == CON_TRIGEDIT) ||
       (STATE(d) == CON_EXDESC)||
       (STATE(d) == CON_TEXTED) ||
       (STATE(d) == CON_QEDIT))) {

        free(*d->str);
      if (d->backstr) {
      *d->str = d->backstr;
      } else
      *d->str = NULL;
      d->backstr = NULL;
      d->str = NULL;
      }
    /*
     * This fix causes the editor to NULL out empty messages -- M. Scott
     * Fixed to fix the fix for empty fixed messages. -- gg
     */
    else if ((d->str) && (*d->str) && (**d->str == '\0')) {
      free(*d->str);
      *d->str = NULL;
    }
    if (STATE(d) == CON_MEDIT)
      medit_disp_menu(d);
    if (STATE(d) == CON_TRIGEDIT)
      trigedit_disp_menu(d);

    if (STATE(d) == CON_OEDIT) {
      switch (OLC_MODE(d)) {
      case OEDIT_ACTDESC:
      oedit_disp_menu(d);
      break;
      case OEDIT_EXTRADESC_DESCRIPTION:
      oedit_disp_extradesc_menu(d);
      break;
      }
    } else if (STATE(d) == CON_HEDIT) {
      hedit_disp_menu(d);
    } else if (STATE(d) == CON_REDIT) {
      switch (OLC_MODE(d)) {
      case REDIT_DESC:
      redit_disp_menu(d);
      break;
      case REDIT_EXIT_DESCRIPTION:
      redit_disp_exit_menu(d);
      break;
      case REDIT_EXTRADESC_DESCRIPTION:
      redit_disp_extradesc_menu(d);
      break;
      }
    } else if (STATE(d) == CON_QEDIT) {
      switch (OLC_MODE(d)) {
      case QEDIT_INFO:
      qedit_disp_menu(d);
      break;
      case QEDIT_ENDING:
      qedit_disp_menu(d);
      break;
      }
    } else if (!d->connected && (PLR_FLAGGED(d->character, PLR_MAILING))) {
      if ((terminator == 1) && *d->str) {
      store_mail(d->mail_to, GET_IDNUM(d->character), *d->str);
      SEND_TO_Q("Message sent!\r\n", d);
      } else
      SEND_TO_Q("Mail aborted.\r\n", d);
      d->mail_to = 0;
      free(*d->str);
      free(d->str);
     } else if (!d->connected && (PLR_FLAGGED(d->character, PLR_MESSAGING))) {
      if ((terminator == 1) && *d->str) {
      strcpy(mess_info.writer, GET_NAME(d->character));
      strcpy(mess_info.message, *d->str);
      mess_info.time = time(0);
      write_message_to_file();
      d->mail_to = 0;
      free(*d->str);
      free(d->str);
      SEND_TO_Q("Message posted!\r\n", d);
      }if (!IS_NPC(d->character))
        REMOVE_BIT(PLR_FLAGS(d->character), PLR_MESSAGING | PLR_WRITING);
    } else if (d->mail_to >= BOARD_MAGIC) {
        Board_save_board(d->mail_to - BOARD_MAGIC);
      SEND_TO_Q("Post not aborted, use REMOVE <post #>.\r\n", d);
        d->mail_to = 0;
    } else if (STATE(d) == CON_EXDESC) {
      if (terminator != 1)
      SEND_TO_Q("Description aborted.\r\n", d);
        SEND_TO_Q(MENU, d);
        d->connected = CON_MENU;
    } else if (STATE(d) == CON_TEXTED) {
	if (terminator == 1) {
	   if (!(fl = fopen((char *)d->storage, "w"))) {
	      sprintf(buf, "SYSERR: Can't write file '%s'.", d->storage);
	      mudlog(buf, CMP, LVL_OLC_FULL, TRUE);
	   }
	   else {
	      if (*d->str) {
		 fputs(stripcr(buf1, *d->str), fl);
	      }
	      fclose(fl);
	      sprintf(buf, "OLC: %s saves '%s'.", GET_NAME(d->character),
 		      d->storage);
 	      mudlog(buf, CMP, LVL_OLC_FULL, TRUE);
 	      SEND_TO_Q("Saved.\r\n", d);
	      STATE(d) = CON_PLAYING;
 	   }
        } else if(terminator == 2){
		SEND_TO_Q("Text edition aborted.\r\n", d);
        	STATE(d) = CON_PLAYING;
        }
    } else if (!d->connected && d->character && !IS_NPC(d->character)) {
      if (terminator == 1) {
      if (strlen(*d->str) == 0) {
        free(*d->str);
        *d->str = NULL;
      }
    } else {
      free(*d->str);
      if (d->backstr)
        *d->str = d->backstr;
      else
        *d->str = NULL;
      d->backstr = NULL;
      SEND_TO_Q("Message aborted.\r\n", d);
      }
     }
    if (d->character && !IS_NPC(d->character))
      REMOVE_BIT(PLR_FLAGS(d->character), PLR_WRITING | PLR_MAILING);
    if (d->backstr)
      free(d->backstr);
    d->backstr = NULL;
    d->str = NULL;
  } else if (!action)
      strcat(*d->str, "\r\n");

  }


/* **********************************************************************
*  Modification of character skills                                     *
********************************************************************** */

ACMD(do_skillset)
{
  struct char_data *vict;
  char name[MAX_INPUT_LENGTH], buf2[128];
  char buf[MAX_INPUT_LENGTH], help[MAX_STRING_LENGTH];
  int skill, value, i, qend;

  argument = one_argument(argument, name);

  /*
   * No arguments. print an informative text.
   */
  if (!*name) {
    send_to_char("Syntax: skillset <name> '<skill>' <value>\r\n", ch);
    strcpy(help, "Skill being one of the following:\r\n");
    for (i = 0; *spells[i] != '\n'; i++) {
      if (*spells[i] == '!')
        continue;
      sprintf(help + strlen(help), "%18s", spells[i]);
      if (i % 4 == 3) {
        strcat(help, "\r\n");
        send_to_char(help, ch);
        *help = '\0';
      }
    }
    if (*help)
      send_to_char(help, ch);
    send_to_char("\r\n", ch);
    return;
  }
  if (!(vict = get_char_vis(ch, name))) {
    send_to_char(NOPERSON, ch);
    return;
  }
  skip_spaces(&argument);

  /* If there is no chars in argument */
  if (!*argument) {
    send_to_char("Skill name expected.\r\n", ch);
    return;
  }
  if (*argument != '\'') {
    send_to_char("Skill must be enclosed in: ''\r\n", ch);
    return;
  }
  /* Locate the last quote && lowercase the magic words (if any) */

  for (qend = 1; *(argument + qend) && (*(argument + qend) != '\''); qend++)
    *(argument + qend) = LOWER(*(argument + qend));

  if (*(argument + qend) != '\'') {
    send_to_char("Skill must be enclosed in: ''\r\n", ch);
    return;
  }
  strcpy(help, (argument + 1));
  help[qend - 1] = '\0';
  if ((skill = find_skill_num(help)) <= 0) {
    send_to_char("Unrecognized skill.\r\n", ch);
    return;
  }
  argument += qend + 1;         /* skip to next parameter */
  argument = one_argument(argument, buf);

  if (!*buf) {
    send_to_char("Learned value expected.\r\n", ch);
    return;
  }
  value = atoi(buf);
  if (value < 0) {
    send_to_char("Minimum value for learned is 0.\r\n", ch);
    return;
  }
  if (value > 100) {
    send_to_char("Max value for learned is 100.\r\n", ch);
    return;
  }
  if (IS_NPC(vict)) {
    send_to_char("You can't set NPC skills.\r\n", ch);
    return;
  }
  sprintf(buf2, "(GC) %s changed %s's %s to %d.", GET_NAME(ch), GET_NAME(vict),
          spells[skill], value);
  mudlog(buf2, BRF, LVL_MJGOD, TRUE);

  SET_SKILL(vict, skill, value);

  sprintf(buf2, "You change %s's %s to %d.\r\n", GET_NAME(vict),
          spells[skill], value);
  send_to_char(buf2, ch);
}

/*********************************************************************
* New Pagination Code
* Michael Buselli submitted the following code for an enhanced pager
* for CircleMUD.  All functions below are his.  --JE 8 Mar 96
*
*********************************************************************/

#define PAGE_LENGTH     22
#define PAGE_WIDTH      80

/* Traverse down the string until the begining of the next page has been
 * reached.  Return NULL if this is the last page of the string.
 */
char *next_page(char *str)
{
  int col = 1, line = 1, spec_code = FALSE;

  for (;; str++) {
    /* If end of string, return NULL. */
    if (*str == '\0')
      return (NULL);

    /* If we're at the start of the next page, return this fact. */
    else if (line > PAGE_LENGTH)
      return (str);

    /* Check for the begining of an ANSI color code block. */
    else if (*str == '\x1B' && !spec_code)
      spec_code = TRUE;

    /* Check for the end of an ANSI color code block. */
    else if (*str == 'm' && spec_code)
      spec_code = FALSE;

    /* Check for everything else. */
    else if (!spec_code) {
      /* Carriage return puts us in column one. */
      if (*str == '\r')
	col = 1;
      /* Newline puts us on the next line. */
      else if (*str == '\n')
	line++;

      /* We need to check here and see if we are over the page width,
       * and if so, compensate by going to the begining of the next line.
       */
      else if (col++ > PAGE_WIDTH) {
	col = 1;
	line++;
      }
    }
  }
}


/* Function that returns the number of pages in the string. */
int count_pages(char *str)
{
  int pages;

  for (pages = 1; (str = next_page(str)); pages++);
  return (pages);
}


/* This function assigns all the pointers for showstr_vector for the
 * page_string function, after showstr_vector has been allocated and
 * showstr_count set.
 */
void paginate_string(char *str, struct descriptor_data *d)
{
  int i;

  if (d->showstr_count)
    *(d->showstr_vector) = str;

  for (i = 1; i < d->showstr_count && str; i++)
    str = d->showstr_vector[i] = next_page(str);

  d->showstr_page = 0;
}


/* The call that gets the paging ball rolling... */
void page_string(struct descriptor_data *d, char *str, int keep_internal)
{
  char actbuf[MAX_INPUT_LENGTH] = "";

  if (!d)
    return;

  if (!str || !*str)
    return;

  d->showstr_count = count_pages(str);
  CREATE(d->showstr_vector, char *, d->showstr_count);

  if (keep_internal) {
    d->showstr_head = strdup(str);
    paginate_string(d->showstr_head, d);
  } else
    paginate_string(str, d);

  show_string(d, actbuf);
}


/* The call that displays the next page. */
void show_string(struct descriptor_data *d, char *input)
{
  char buffer[MAX_STRING_LENGTH], buf[MAX_INPUT_LENGTH], texto[MAX_INPUT_LENGTH];
  int diff;

  any_one_arg(input, buf);

  /* Q is for quit. :) */
  if (LOWER(*buf) == 'q') {
    free(d->showstr_vector);
    d->showstr_vector = NULL;
    d->showstr_count = 0;
    if (d->showstr_head) {
      free(d->showstr_head);
      d->showstr_head = NULL;
    }
    return;
  }
  /* R is for refresh, so back up one page internally so we can display
   * it again.
   */
  else if (LOWER(*buf) == 'r')
    d->showstr_page = MAX(0, d->showstr_page - 1);

  /* B is for back, so back up two pages internally so we can display the
   * correct page here.
   */
  else if (LOWER(*buf) == 'b')
    d->showstr_page = MAX(0, d->showstr_page - 2);

  else if (*buf) {
    send_to_char("&WValid commands while paging are &RRETURN&W, &RQ&W, &RR&W and &RB&W.&n\r\n", d->character);
    return;
  }
  /* If we're displaying the last page, just send it to the character, and
   * then free up the space we used.
   */
  if (d->showstr_page + 1 >= d->showstr_count) {
    sprintf(texto, "%s", d->showstr_vector[d->showstr_page]);
	send_to_char(texto, d->character);
    free(d->showstr_vector);
    d->showstr_vector = NULL;
    d->showstr_count = 0;
    if (d->showstr_head) {
      free(d->showstr_head);
      d->showstr_head = NULL;
    }
  }
  /* Or if we have more to show.... */
  else {
    diff = d->showstr_vector[d->showstr_page + 1] - d->showstr_vector[d->showstr_page];
    if (diff > MAX_STRING_LENGTH - 3) /* 3=\r\n\0 */
      diff = MAX_STRING_LENGTH - 3;
    strncpy(buffer, d->showstr_vector[d->showstr_page], diff);	/* strncpy: OK (size truncated above) */
    /*
     * Fix for prompt overwriting last line in compact mode submitted by
     * Peter Ajamian <peter@pajamian.dhs.org> on 04/21/2001
     */
    if (buffer[diff - 2] == '\r' && buffer[diff - 1]=='\n')
      buffer[diff] = '\0';
    else if (buffer[diff - 2] == '\n' && buffer[diff - 1] == '\r')
      /* This is backwards.  Fix it. */
      strcpy(buffer + diff - 2, "\r\n");	/* strcpy: OK (size checked) */
    else if (buffer[diff - 1] == '\r' || buffer[diff - 1] == '\n')
      /* Just one of \r\n.  Overwrite it. */
      strcpy(buffer + diff - 1, "\r\n");	/* strcpy: OK (size checked) */
    else
      /* Tack \r\n onto the end to fix bug with prompt overwriting last line. */
      strcpy(buffer + diff, "\r\n");	/* strcpy: OK (size checked) */
	strcpy(texto, buffer);
    send_to_char(texto, d->character);
    d->showstr_page++;
  }
}

void smash_tilde(char *string)
{
  /*
   * Erase any ~'s inserted by people in the editor.  This prevents anyone
   * using online creation from causing parse errors in the world files.
   * Derived from an idea by Sammy <samedi@dhc.net> (who happens to like
   * his tildes thank you very much.), -gg 2/20/98
   */
	char *str = string;
    while ((str = strchr(str, '~')) != NULL)
      *str = ' ';
}

/* temp func -Petrus */
void write_last_command(char *arg)
{
  FILE *fp;

  if (!(fp = fopen("../log/.lastcommand", "wb"))) {
    return;
  }

  fputs(arg, fp);
  fputc('\n', fp);

  fclose(fp);
}
