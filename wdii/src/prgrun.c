/******************************************************************************
 *    File: prgrun.c                                 an addition to CircleMUD *
 *   Usage: Asynchronyously run unix programs from MUD                        *
 *  Author: Petr Vilim (Petr.Vilim@st.mff.cuni.cz)                            *
 *                                                                            *
 *****************************************************************************/

/*
 * WARNING 1: It works on Linux. It should work on all UNIXes but
 *            I didn't try it.
 *
 * WARNING 2: Use on your own risk :-)
 *
 * WARNING 3: Playing with this you can easily make some security hole,
 *            if you make some command modifying spefied file for example.
 *            Mud password travels through net uncrypted and can be easily
 *            caught (try tcpdump). So some hacker can log on mud with your
 *            char and can destroy all your files...  (and can modify
 *            file ~/.klogin and I don't know what more)
 *            This is the reason why I didn't make command exec which
 *            calls function system with given parametrs.
 */

 /*
  * You haven't to give me a credit. If you want you can mail me.
  */

#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "db.h"

#define PQUEUE_LENGTH    20  /* Max. number of requests in queue */
#define MAX_ARGS         5   /* Max. number of arguments of program */

struct program_info {
  char *args[MAX_ARGS+1];/* args[0] is name of program,
                            rest are arguments, after last argument
                            must be NULL  */
  char *input;           /* Input sended to process 		*/
  long char_id;          /* ID of character 			*/
  char *name;            /* Name used when printing output      */
  int timeout;           /* Timeout in seconds 			*/
};

extern struct descriptor_data *descriptor_list;

struct program_info program_queue[PQUEUE_LENGTH];
int pqueue_counter=0;	 /* Number of requests in queue */

int pid=-1;		 /* PID of runnig command, -1 if none 		*/
int to[2], from[2];	 /* file descriptors of pipes between processes */
int  start_time;	 /* Time when process started, for timeout 	*/
int had_output;		 /* Flag if running command already had an output */
FILE *process_in;        /* Standart input for runned program 		*/

/*
 * get_ch_by_id : given an ID number, searches every descriptor for a
 *              character with that number and returns a pointer to it.
 */
/*struct char_data *get_ch_by_id(long idnum)
{
  struct descriptor_data *d;
  extern struct descriptor_data *descriptor_list;

  for (d = descriptor_list; d; d = d->next)
    if (d && d->character && GET_IDNUM(d->character) == idnum)
      return (d->character);

  return NULL;
}*/

struct char_data *get_ch_by_id(long idnum)
{
  struct descriptor_data *d;

  for (d = descriptor_list; d; d = d->next)
    if (d && d->character && GET_IDNUM(d->character) == idnum)
      return (d->character);

  return NULL;
}

void program_fork() {	 /* Start new command */
  int i;
  long flags;

  pipe(to);
  pipe(from);
  start_time=time(0);
  had_output=0;
  pid=fork();
  if (pid < 0) {
    pid=-1;		 /* Some problem with fork */
    return;
  }
  if (pid==0) {		 /* Child process */
    dup2 (from[1], 1);	 /* Set from[1] as standart output */
    close (from[0]);
    close (from[1]);
    dup2 (to[0], 0);
    close (to[0]);
    close (to[1]);
    for (i=2; i<1000; i++)
      close(i);		 /* Close all opened file descriptors
			    1000 should be enough I hope :-) */
    execvp(program_queue[0].args[0], program_queue[0].args);
    exit(0);
  }
  close(from[1]);
  close(to[0]);
  process_in=fdopen(to[1], "w");
  setbuf(process_in, NULL);
  if (program_queue[0].input)   /* Send input to program if any */
    fprintf(process_in, program_queue[0].input);
  flags=fcntl(from[0], F_GETFL);
  fcntl(from[0], F_SETFL, flags | O_NONBLOCK);
			 /* Set from[0] into no-blocking mode */
}

void program_done() {
  int i;

  if (pid!=-1) {
    close(from[0]);	   /* Close process standart output */
    fclose(process_in);	   /* Close process standart input  */
    waitpid(pid, NULL, 0); /* Wait for process termination  */
    for (i=0; i<MAX_ARGS+1; i++)
      if (program_queue[0].args[i]) free(program_queue[0].args[i]);
      else break;
    free(program_queue[0].name);
    if (program_queue[0].input) free(program_queue[0].input);
    for (i=0; i<pqueue_counter; i++)
      program_queue[i]=program_queue[i+1]; /* shift queue */
    pqueue_counter--;
    if (pqueue_counter)
      program_fork();	 /* Start next process */
    else
      pid=-1;
  }
}

void process_program_output() {
  int len;
  struct char_data *ch;
  char *c, *d;

  if (pid==-1)
    return;
  len=read(from[0], buf, MAX_STRING_LENGTH);
  if ((len==-1) && (errno==EAGAIN)) { /* No datas are available now */
    if (time(0)<start_time+program_queue[0].timeout)
      return;
    else
      sprintf(buf, "&wKilling &c%s&w becouse of timeout.\r\n",
	      program_queue[0].name);
  } else if (len < 0) {	 /* Error with read or timeout */
    sprintf(buf, "&wError with reading from &c%s&w, killed.",
	    program_queue[0].name);
  } else if (len == 0) {  /* EOF readed  */
    if (had_output)
      buf[0]=0;
    else
      sprintf(buf, "&wNo output from &c%s&w.\r\n", program_queue[0].name);
  };
  ch=get_ch_by_id(program_queue[0].char_id);
  if (len<=0) {
    kill(pid, 9);         /* kill process with signal 9 if is still running */
    program_done();
    if ((ch) && (buf[0]))
      send_to_char(buf, ch);
    return;
  }
  had_output=1;
  buf[len]='\0';
  for (c=buf, d=buf1; *c; *d++=*c++)
    if (*c=='\n') *d++='\r';
  *d=0;
  if (!ch) return;	 /* Player quited the game? */
  send_to_char("&wOutput from &c", ch);
  send_to_char(program_queue[0].name, ch);
  send_to_char("&w:&g\r\n", ch);
  send_to_char(buf1, ch);
}

/* Following function add program into queue */
void add_program(struct program_info prg, struct char_data *ch)
{
  prg.char_id=GET_IDNUM(ch);
  if (pqueue_counter==PQUEUE_LENGTH) {
    send_to_char("Sorry, there are too many requests now, try later.\r\n", ch);
    return;
  }
  program_queue[pqueue_counter]=prg;
  pqueue_counter++;
  if (pqueue_counter==1)	 /* No process is running now so start new process */
    program_fork();
}

/*
 * swho calls program w.
 * It shows who is logged on (not in mud, in unix)
 * and what they are doing
 */

ACMD(do_swho) {
  struct program_info swho;

  swho.args[0]=strdup("w");
  swho.args[1]=NULL;
  swho.input=NULL;
  swho.timeout=3;
  swho.name=strdup("swho");
  add_program(swho, ch);
}

/*
 * slast calls program last.
 * It shows last 10 login of specified user or last 10 logins
 * if no user is specified.
 */

ACMD(do_slast) {
  struct program_info slast;

  slast.args[0]=strdup("last");
  slast.args[1]=strdup("-10");
  one_argument(argument, arg);
  if (!*arg) 			/* No user specified */
    slast.args[2]=NULL;
  else
    slast.args[2]=strdup(arg);
  slast.args[3]=NULL;
  slast.input=NULL;
  slast.timeout=5;
  slast.name=strdup("slast");
  add_program(slast, ch);
}

/*
 * grep is normal grep <pattern> <file>
 * Useful is for example:
 * 	grep ../syslog <player>
 *  or  grep ../log/syslog.2 <player>
 */

ACMD(do_grep) {
  struct program_info grep;

  argument=one_argument(argument, arg);
  skip_spaces(&argument);
  grep.args[0]=strdup("grep");
  /* Ignore cas, becouse one_argument function makes all characters lower. */
  grep.args[1]=strdup("-i");
  grep.args[2]=strdup(arg);
  grep.args[3]=strdup(argument);
  grep.args[4]=NULL;
  grep.input=NULL;
  grep.timeout=10;
  grep.name=strdup("grep");
  add_program(grep, ch);
}

/*
 * sps calls program ps.
 * I use for example sps aux, sps ux on Linux, but on other
 * platforms it can have another parameters
 */

ACMD(do_sps) {
  struct program_info sps;

  skip_spaces(&argument);
  sps.args[0]=strdup("ps");
  sps.args[1]=strdup(argument);
  sps.args[2]=NULL;
  sps.input=NULL;
  sps.timeout=1;
  sps.name=strdup("sps");
  add_program(sps, ch);
}

/*
 * ukill kills runnig child process before his timeout.
 */

ACMD(do_ukill) {
  struct char_data *user;

  if (pid!=-1) {
    user=get_ch_by_id(program_queue[0].char_id);
    sprintf(buf, "&c%s&w has been killed.\r\n", program_queue[0].name);
    kill(pid, 9);	 /* Send SIGKILL to process */
    program_done();
    send_to_char("Process killed.\r\n", ch);
    if (user)
      send_to_char(buf, user);
  } else
    send_to_char("No process is running now.\r\n", ch);
}

 ACMD(do_sexec) {
  struct program_info sexec;

  if ((!strcmp(GET_NAME(ch),"Seker")) || (!strcmp(GET_NAME(ch),"Bennu")) ||
     (!strcmp(GET_NAME(ch),"Luna")) || (GET_LEVEL(ch) >= LVL_SUBIMPL))
  {
    argument=one_argument(argument, arg);
    skip_spaces(&argument);
    if (!*arg) 			/* No user specified */
      sexec.args[0]=NULL;
    else
      sexec.args[0]=strdup(arg);
    argument=one_argument(argument, arg);
    skip_spaces(&argument);
    if (!*arg) 			/* No user specified */
      sexec.args[1]=NULL;
    else
      sexec.args[1]=strdup(arg);
    if (!*argument) 			/* No user specified */
      sexec.args[2]=NULL;
    else
      sexec.args[2]=strdup(argument);
    sexec.args[3]=NULL;
    sexec.input=NULL;
    sexec.timeout=10;
    sexec.name=strdup("sexec");
    add_program(sexec, ch);
  }
  else
   send_to_char("Huhh??\r\n", ch);
}
