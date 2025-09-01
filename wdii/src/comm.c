/*************************************************************************
*   File: comm.c                                        Part of CircleMUD *
*  Usage: Communication, socket handling, main(), central game loop       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#define __COMM_C__

#include "conf.h"
#include "sysdep.h"

#ifdef CIRCLE_MACINTOSH         /* Includes for the Macintosh */
# define SIGPIPE 13
# define SIGALRM 14
  /* GUSI headers */
# include <sys/ioctl.h>
  /* Codewarrior dependant */
# include <SIOUX.h>
# include <console.h>
#endif

#ifdef CIRCLE_WINDOWS           /* Includes for Win32 */
# ifdef __BORLANDC__
#  include <dir.h>
# else /* MSVC */
#  include <direct.h>
# endif
# include <mmsystem.h>
#endif /* CIRCLE_WINDOWS */

#ifdef CIRCLE_AMIGA             /* Includes for the Amiga */
# include <sys/ioctl.h>
# include <clib/socket_protos.h>
#endif /* CIRCLE_AMIGA */

#ifdef CIRCLE_ACORN             /* Includes for the Acorn (RiscOS) */
# include <socklib.h>
# include <inetlib.h>
# include <sys/ioctl.h>
#endif

/*
 * Note, most includes for all platforms are in sysdep.h.  The list of
 * files that is included is controlled by conf.h for that platform.
 */

#include "structs.h"
#include "buffer.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "house.h"
#include "olc.h"
#include "screen.h"
#include "dg_scripts.h"
#include "teleport.h"
#include "clan.h"

#ifdef HAVE_ARPA_TELNET_H
#include <arpa/telnet.h>
#else
#include "telnet.h"
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

/* Global pulse counter (referenced from multiple modules) */
int pulse = 0;

/* externs */
extern struct ban_list_element *ban_list;
extern int num_invalid;
extern char *GREETINGS;
extern const char circlemud_version[];
extern int circle_restrict;
extern int mini_mud;
extern int no_rent_check;
extern ush_int DFLT_PORT;
extern char *DFLT_DIR;
extern char *DFLT_IP;
extern char *LOGNAME;
extern int MAX_PLAYERS;
extern int buffer_opt;
extern struct zone_data *zone_table;
extern const char *save_info_msg[];   /* In olc.c */
extern int top_of_zone_table;
extern struct room_data *world; /* In db.c */
extern int top_of_world;        /* In db.c */
extern struct time_info_data time_info;         /* In db.c */
extern char *help;
extern struct zone_data *zone_table;
extern char *npc_class_abbrevs[];
extern char *class_abbrevs[];
	
/* local globals */
struct descriptor_data *descriptor_list = NULL;         /* master desc list */
extern struct char_data *character_list;
struct txt_block *bufpool = 0;	/* pool of large output buffers */
int buf_largecount = 0;         /* # of large buffers which exist */
int buf_overflows = 0;          /* # of overflows of output */
int buf_switches = 0;           /* # of switches from small to large buf */
int circle_shutdown = 0;        /* clean shutdown */
int circle_reboot = 0;          /* reboot the game after a shutdown */
int no_specials = 0;            /* Suppress ass. of special routines */
int max_players = 0;            /* max descriptors available */
int tics = 0;                   /* for extern checkpointing */
int scheck = 0;                 /* for syntax checking mode */
int dg_act_check;               /* toggle for act_trigger */
unsigned long dg_global_pulse = 0; /* number of pulses since game start */
extern int nameserver_is_slow;  /* see config.c */
extern int auto_save;           /* see config.c */
extern int autosave_time;       /* see config.c */
extern struct char_data *tch;
struct timeval null_time;       /* zero-valued time structure */
FILE *logfile = NULL;           /* Where to send the log messages. */
extern int multi_player;	/* max players per host */
static bool fCopyOver;          /* Are we booting in copyover mode? */
int  mother_desc;        /* Now a global */
int     port;


/* functions in this file */
RETSIGTYPE reread_wizlists(int sig);
RETSIGTYPE unrestrict_game(int sig);
RETSIGTYPE reap(int sig);
RETSIGTYPE checkpointing(int sig);
RETSIGTYPE hupsig(int sig);
ssize_t perform_socket_read(socket_t desc, char *read_point,size_t space_left);
ssize_t perform_socket_write(socket_t desc, const char *txt,size_t length);
void echo_off(struct descriptor_data *d);
void echo_on(struct descriptor_data *d);
void sanity_check(void);
void circle_sleep(struct timeval *timeout);
int get_from_q(struct txt_q *queue, char *dest, int *aliased);
void init_game(int port);
void signal_setup(void);
void game_loop(int mother_desc);
socket_t init_socket(ush_int port);
int new_descriptor(socket_t s);
int get_max_players(void);
int process_output(struct descriptor_data *t);
int process_input(struct descriptor_data *t);
void timediff(struct timeval *diff, struct timeval *a, struct timeval *b);
void timeadd(struct timeval *sum, struct timeval *a, struct timeval *b);
void flush_queues(struct descriptor_data *d);
void nonblock(socket_t s);
int perform_subst(struct descriptor_data *t, char *orig, char *subst);
int perform_alias(struct descriptor_data *d, char *orig);
void record_usage(void);
char *make_prompt(struct descriptor_data *point);
void check_idle_passwords(void);
void heartbeat(int pulse);
struct in_addr *get_bind_addr(void);
int parse_ip(const char *addr, struct in_addr *inaddr);
int set_sendbuf(socket_t s);
#if defined(POSIX)
sigfunc *my_signal(int signo, sigfunc * func);
#endif
void    init_descriptor (struct descriptor_data *newd, int desc);
void tips(void);

/* extern fcnts */
void reboot_wizlists(void);
void boot_world(void);
void affect_update(void);       /* In spells.c */
void mobile_activity(void);
void string_add(struct descriptor_data *d, char *str);
void perform_violence(void);
void show_string(struct descriptor_data *d, char *input);
int isbanned(char *hostname);
void weather_and_time(int mode);
void redit_save_to_disk(int zone_num);
void oedit_save_to_disk(int zone_num);
void medit_save_to_disk(int zone_num);
void sedit_save_to_disk(int zone_num);
void zedit_save_to_disk(int zone_num);
void qedit_save_to_disk(int zone_num);
int real_zone(int number);
char *make_bar(int val, int max, int len, int cores);
void mental_update(void);
void hmm_update(void);
void update_breath(void);
void gods_update(void);
void write_mud_date_to_file(void);
void dragon_upd(void); /* in winddrago.c */
int level_exp(int remort, int level);
void custo_clan(void);

#ifdef __CXREF__
#undef FD_ZERO
#undef FD_SET
#undef FD_ISSET
#undef FD_CLR
#define FD_ZERO(x)
#define FD_SET(x, y) 0
#define FD_ISSET(x, y) 0
#define FD_CLR(x, y)
#endif


/***********************************************************************
*  main game loop and related stuff                                    *
***********************************************************************/

#if defined(CIRCLE_WINDOWS) || defined(CIRCLE_MACINTOSH)

/* Windows doesn't have gettimeofday, so we'll simulate it. */
/* The Mac doesn't have gettimeofday either. */
void gettimeofday(struct timeval *t, struct timezone *dummy)
{
#if defined(CIRCLE_WINDOWS)
  DWORD millisec = GetTickCount();
#elif defined(CIRCLE_MACINTOSH)
  unsigned long int millisec;
  millisec = (int)((float)TickCount() * 1000.0 / 60.0);
#endif

  t->tv_sec = (int) (millisec / 1000);
  t->tv_usec = (millisec % 1000) * 1000;
}

#endif  /* CIRCLE_WINDOWS || CIRCLE_MACINTOSH */

int main(int argc, char **argv)
{
  int pos = 1;
  char *dir;  
  
//  leak_logging = 0;
  
  /* Initialize these to check for overruns later. */
  buf[MAX_STRING_LENGTH - 1] = buf1[MAX_STRING_LENGTH - 1] = MAGIC_NUMBER;
  buf2[MAX_STRING_LENGTH - 1] = arg[MAX_STRING_LENGTH - 1] = MAGIC_NUMBER;

#ifdef CIRCLE_MACINTOSH
  /*
   * ccommand() calls the command line/io redirection dialog box from
   * Codewarriors's SIOUX library
   */
  argc = ccommand(&argv);
  /* Initialize the GUSI library calls.  */
  GUSIDefaultSetup();
#endif

  
  port = DFLT_PORT;
  dir = DFLT_DIR;

  /*
   * It would be nice to make this a command line option but the parser uses
   * the log() function, maybe later. -gg
   */
  if (LOGNAME == NULL || *LOGNAME == '\0')
    logfile = fdopen(STDERR_FILENO, "w");
  else
    logfile = freopen(LOGNAME, "w", stderr);

  if (logfile == NULL) {
      printf("error opening log file %s: %s\n",
                LOGNAME ? LOGNAME : "stderr", strerror(errno));
      exit(1);
  }

  /* Start up the better buffer system. */  
 /* init_buffers(); */
  
  log(circlemud_version);
      
  while ((pos < argc) && (*(argv[pos]) == '-')) {
    switch (*(argv[pos] + 1)) {
    case 'C': /* -C<socket number> - recover from copyover, this is the control socket */
	fCopyOver = TRUE;
	mother_desc = atoi(argv[pos]+2);
	break;
    case 'v':
      if (*(argv[pos] + 2))
	buffer_opt = atoi(argv[pos] + 2);
      else if (++pos < argc)
	buffer_opt = atoi(argv[pos]);
      else {
	log("SYSERR: Number expected after option -v.");
	exit(1);
      }
      break;		
    case 'd':
      if (*(argv[pos] + 2))
        dir = argv[pos] + 2;
      else if (++pos < argc)
        dir = argv[pos];
      else {
        log("SYSERR: Directory arg expected after option -d.");
        exit(1);
      }
      break;
    case 'm':
      mini_mud = 1;
      no_rent_check = 1;
      log("Running in minimized mode & with no rent check.");
      break;
    case 'c':
      scheck = 1;
      log("Syntax check mode enabled.");
      break;
    case 'q':
      no_rent_check = 1;
      log("Quick boot mode -- rent check supressed.");
      break;
    case 'r':
      circle_restrict = 1;
      log("Restricting game -- no new players allowed.");
      break;
    case 's':
      no_specials = 1;
      log("Suppressing assignment of special routines.");
      break;
    default:
      log("SYSERR: Unknown option -%c in argument string.", *(argv[pos] + 1));
      break;
    }
    pos++;
  }

  if (pos < argc) {
    if (!isdigit(*argv[pos])) {
      log("Usage: %s [-c] [-m] [-q] [-r] [-s] [-d pathname] [-v val] [port #]\n", argv[0]);
      exit(1);
    } else if ((port = atoi(argv[pos])) <= 1024) {
      log("SYSERR: Illegal port number %d.", port);
      exit(1);
    }
  }

  if (chdir(dir) < 0) {
    perror("SYSERR: Fatal error changing to data directory");
    exit(1);
  }
  log("Using %s as data directory.", dir);

  if (scheck) {
    boot_world();
    log("Done.");
  } else {
    log("Running game on port %d.", port);      
    init_game(port);
  }
 /* exit_buffers();*/

  return 0;
}

int enter_player_game(struct descriptor_data *d);

/* Reload players after a copyover */
void copyover_recover()
{
	struct descriptor_data *d;
	FILE *fp;
	char host[1024];
	int desc, player_i;
   	bool fOld;
    	char name[MAX_INPUT_LENGTH];
	
	log ("Copyover recovery initiated");
	
	fp = fopen (COPYOVER_FILE, "r");
	
	if (!fp) /* there are some descriptors open which will hang forever then ? */
	{
		perror ("copyover_recover:fopen");
		log ("Copyover file not found. Exitting.\n\r");
		exit (1);
	}

	unlink (COPYOVER_FILE); /* In case something crashes - doesn't prevent reading	*/
	
	for (;;)
    {
        fOld = TRUE;
		fscanf (fp, "%d %s %s\n", &desc, name, host);
		if (desc == -1)
			break;

		/* Write something, and check if it goes error-free */		
		if (write_to_descriptor (desc, "\n\rUpdate initiated...\n\r") < 0)
		{
			close (desc); /* nope */
			continue;
		}
		
        /* create a new descriptor */
        CREATE (d, struct descriptor_data, 1);
       /* memset ((char *) d, 0, sizeof (struct descriptor_data));*/
		init_descriptor (d,desc); /* set up various stuff */
        CREATE(d->history, char *, HISTORY_SIZE);
		strcpy(d->host, host);
		d->next = descriptor_list;
		descriptor_list = d;

        d->connected = CON_CLOSE;
	
		/* Now, find the pfile */
		
        CREATE(d->character, struct char_data, 1);
        clear_char(d->character);
        CREATE(d->character->player_specials, struct player_special_data, 1);
        d->character->desc = d;

        if ((player_i = load_char(name, d->character)) >= 0)
        {
            GET_PFILEPOS(d->character) = player_i;
            if (!PLR_FLAGGED(d->character, PLR_DELETED))
                REMOVE_BIT(PLR_FLAGS(d->character),PLR_WRITING | PLR_MAILING | PLR_CRYO);
            else
                fOld = FALSE;
        }
        else
            fOld = FALSE;
		
		if (!fOld) /* Player file not found?! */
		{
			write_to_descriptor (desc, "\n\rSomehow, your character was lost during the folding. Sorry.\n\r");
			close_socket (d);			
		}
		else /* ok! */
		{
            write_to_descriptor (desc, "\n\rUpdate complete.\n\r");
            enter_player_game(d);
            d->connected = CON_PLAYING;
            look_at_room(d->character, 0);
		}
		
	}
	
	fclose (fp);
}

/* Init sockets, run game, and cleanup sockets */
void init_game(int port)
{ 
  /* We don't want to restart if we crash before we get up. */
  touch(KILLSCRIPT_FILE);

  circle_srandom(time(0));

  log("Finding player limit.");
  max_players = get_max_players();

  if (!fCopyOver) /* If copyover mother_desc is already set up */
  {
	log ("Opening mother connection.");
	mother_desc = init_socket (port);
  }

  boot_db();

#ifdef CIRCLE_UNIX
  log("Signal trapping.");
  signal_setup();
#endif

  if (fCopyOver) /* reload players */
	copyover_recover();

  /* If we made it this far, we will be able to restart without problem. */
  remove(KILLSCRIPT_FILE);

  log("Entering game loop.");

  game_loop(mother_desc);
  
  Crash_save_all();

  log("Closing all sockets.");
  while (descriptor_list)
    close_socket(descriptor_list);

  CLOSE_SOCKET(mother_desc);

  if (circle_reboot != 2 && olc_save_list) { /* Don't save zones. */
    struct olc_save_info *entry, *next_entry;
    int rznum;

    for (entry = olc_save_list; entry; entry = next_entry) {
      next_entry = entry->next;
      if (entry->type < 0 || entry->type > 5) {
        sprintf(buf, "OLC: Illegal save type %d!", entry->type);
        log(buf);
      } else if ((rznum = real_zone(entry->zone * 100)) == -1) {
        sprintf(buf, "OLC: Illegal save zone %d!", entry->zone);
        log(buf);
      } else if (rznum < 0 || rznum > top_of_zone_table) {
        sprintf(buf, "OLC: Invalid real zone number %d!", rznum);
        log(buf);
      } else {
        sprintf(buf, "OLC: Reboot saving %s for zone %d.",
                save_info_msg[(int)entry->type], zone_table[rznum].number);
        log(buf);
        switch (entry->type) {
        case OLC_SAVE_ROOM: redit_save_to_disk(rznum); break;
        case OLC_SAVE_OBJ:  oedit_save_to_disk(rznum); break;
        case OLC_SAVE_MOB:  medit_save_to_disk(rznum); break;
        case OLC_SAVE_ZONE: zedit_save_to_disk(rznum); break;
        case OLC_SAVE_SHOP: sedit_save_to_disk(rznum); break;
        case OLC_SAVE_QUEST:  qedit_save_to_disk(rznum); break;
        default:      log("Unexpected olc_save_list->type"); break;
        }
      }
    }
  }

  if (circle_reboot) {
    log("Rebooting.");    
    exit(52);                   /* what's so great about HHGTTG, anyhow? */
  }
   log("Normal termination of game.");  
}



/*
 * init_socket sets up the mother descriptor - creates the socket, sets
 * its options up, binds it, and listens.
 */
socket_t init_socket(ush_int port)
{
  socket_t s;
  struct sockaddr_in sa;
  int opt;

#ifdef CIRCLE_WINDOWS
  {
    WORD wVersionRequested;
    WSADATA wsaData;

    wVersionRequested = MAKEWORD(1, 1);

    if (WSAStartup(wVersionRequested, &wsaData) != 0) {
      log("SYSERR: WinSock not available!");
      exit(1);
    }
    if ((wsaData.iMaxSockets - 4) < max_players) {
      max_players = wsaData.iMaxSockets - 4;
    }
    log("Max players set to %d", max_players);
    
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
      log("SYSERR: Error opening network connection: Winsock error #%d",
          WSAGetLastError());
      exit(1);
    }
  }
#else
  /*
   * Should the first argument to socket() be AF_INET or PF_INET?  I don't
   * know, take your pick.  PF_INET seems to be more widely adopted, and
   * Comer (_Internetworking with TCP/IP_) even makes a point to say that
   * people erroneously use AF_INET with socket() when they should be using
   * PF_INET.  However, the man pages of some systems indicate that AF_INET
   * is correct; some such as ConvexOS even say that you can use either one.
   * All implementations I've seen define AF_INET and PF_INET to be the same
   * number anyway, so the point is (hopefully) moot.
   */

  if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("SYSERR: Error creating socket");
    exit(1);
  }
#endif                          /* CIRCLE_WINDOWS */

#if defined(SO_REUSEADDR) && !defined(CIRCLE_MACINTOSH)
  opt = 1;
  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)) < 0){
    perror("SYSERR: setsockopt REUSEADDR");
    exit(1);
  }
#endif

  set_sendbuf(s);

#if defined(SO_LINGER) && !defined(CIRCLE_MACINTOSH)
  {
    struct linger ld;

    ld.l_onoff = 0;
    ld.l_linger = 0;
    if (setsockopt(s, SOL_SOCKET, SO_LINGER, (char *) &ld, sizeof(ld)) < 0) {
      perror("SYSERR: setsockopt LINGER");
      exit(1);
    }
  }
#endif

  /* Clear the structure */
  memset((char *)&sa, 0, sizeof(sa));

  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  sa.sin_addr = *(get_bind_addr());

  if (bind(s, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
    perror("SYSERR: bind");
    CLOSE_SOCKET(s);
    exit(1);
  }
  nonblock(s);
  listen(s, 5);
  return (s);
}


int get_max_players(void)
{
#ifndef CIRCLE_UNIX
  return (MAX_PLAYERS);
#else

  int max_descs = 0;
  const char *method;
   
/*
 * First, we'll try using getrlimit/setrlimit.  This will probably work
 * on most systems.  HAS_RLIMIT is defined in sysdep.h.
 */
#ifdef HAS_RLIMIT
  {
    struct rlimit limit;

    /* find the limit of file descs */
    method = "rlimit";
    if (getrlimit(RLIMIT_NOFILE, &limit) < 0) {
      perror("SYSERR: calling getrlimit (resource RLIMIT_NOFILE)");      
      exit(1);
    }

    /* set the current to the maximum */
    limit.rlim_cur = limit.rlim_max;
    if (setrlimit(RLIMIT_NOFILE, &limit) < 0) {
      perror("SYSERR: calling setrlimit (resource RLIMIT_NOFILE)");      
      exit(1);
    }

    /* Aloysio -- muds tem q gerar core dumps! */
    if (getrlimit(RLIMIT_CORE, &limit) < 0) {
      perror("SYSERR: calling getrlimit (resource RLIMIT_CORE)");
      exit(1);
    }

    limit.rlim_cur = limit.rlim_max;
    if (setrlimit(RLIMIT_CORE, &limit) < 0) {
      perror("SYSERR: calling setrlimit (resource RLIMIT_CORE)");
      exit(1);
    }
    
#ifdef RLIM_INFINITY
    if (limit.rlim_max == RLIM_INFINITY)
      max_descs = MAX_PLAYERS + NUM_RESERVED_DESCS;
    else
      max_descs = MIN(MAX_PLAYERS + NUM_RESERVED_DESCS, limit.rlim_max);
#else
    max_descs = MIN(MAX_PLAYERS + NUM_RESERVED_DESCS, limit.rlim_max);
#endif
  }

#elif defined (OPEN_MAX) || defined(FOPEN_MAX)
#if !defined(OPEN_MAX)
#define OPEN_MAX FOPEN_MAX
#endif
  method = "OPEN_MAX";
  max_descs = OPEN_MAX;         /* Uh oh.. rlimit didn't work, but we have
                                 * OPEN_MAX */
#elif defined (_SC_OPEN_MAX)
  /*
   * Okay, you don't have getrlimit() and you don't have OPEN_MAX.  Time to
   * try the POSIX sysconf() function.  (See Stevens' _Advanced Programming
   * in the UNIX Environment_).
   */
  method = "POSIX sysconf";
  errno = 0;
  if ((max_descs = sysconf(_SC_OPEN_MAX)) < 0) {
    if (errno == 0)
      max_descs = MAX_PLAYERS + NUM_RESERVED_DESCS;
    else {
      perror("SYSERR: Error calling sysconf");
      exit(1);
    }
  }
#else
  /* if everything has failed, we'll just take a guess */
  method = "random guess"
  max_descs = MAX_PLAYERS + NUM_RESERVED_DESCS;
#endif

  /* now calculate max _players_ based on max descs */
  max_descs = MIN(MAX_PLAYERS, max_descs - NUM_RESERVED_DESCS);

  if (max_descs <= 0) {
    log("SYSERR: Non-positive max player limit!  (Set at %d using %s).",
            max_descs, method);    
    exit(1);
  }
  log("Setting player limit to %d using %s.", max_descs, method);  
  return (max_descs);
#endif /* CIRCLE_UNIX */
}



/*
 * game_loop contains the main loop which drives the entire MUD.  It
 * cycles once every 0.10 seconds and is responsible for accepting new
 * new connections, polling existing connections for input, dequeueing
 * output and sending it out to players, and calling "heartbeat" functions
 * such as mobile_activity().
 */
void game_loop(socket_t mother_desc)
{
  fd_set input_set, output_set, exc_set, null_set;
  struct timeval last_time, before_sleep, opt_time, process_time, now, timeout, temp_time;
  char comm[MAX_INPUT_LENGTH];
  struct descriptor_data *d, *next_d;
  int missed_pulses, maxdesc, aliased;
  pulse = 0;
 
  /* initialize various time values */
  null_time.tv_sec = 0;
  null_time.tv_usec = 0;
  opt_time.tv_usec = OPT_USEC;
  opt_time.tv_sec = 0;
  FD_ZERO(&null_set);

  gettimeofday(&last_time, (struct timezone *) 0);

  /* The Main Loop.  The Big Cheese.  The Top Dog.  The Head Honcho.  The.. */
  while (!circle_shutdown) {

    /* Sleep if we don't have any connections */
    if (descriptor_list == NULL) {
      log("No connections.  Going to sleep.");
      /*exit(2); - for bootup profiling*/
      FD_ZERO(&input_set);
      FD_SET(mother_desc, &input_set);
      if (select(mother_desc + 1, &input_set, (fd_set *) 0, (fd_set *) 0, NULL) < 0) {
        if (errno == EINTR)
          log("Waking up to process signal.");
        else
          perror("SYSERR: Select coma");
      } else
        log("New connection.  Waking up.");
      gettimeofday(&last_time, (struct timezone *) 0);
    }
    /* Set up the input, output, and exception sets for select(). */
    FD_ZERO(&input_set);
    FD_ZERO(&output_set);
    FD_ZERO(&exc_set);
    FD_SET(mother_desc, &input_set);

    maxdesc = mother_desc;
    for (d = descriptor_list; d; d = d->next) {
#ifndef CIRCLE_WINDOWS
      if (d->descriptor > maxdesc)
        maxdesc = d->descriptor;
#endif
      FD_SET(d->descriptor, &input_set);
      FD_SET(d->descriptor, &output_set);
      FD_SET(d->descriptor, &exc_set);
    }

    /*
     * At this point, we have completed all input, output and heartbeat
     * activity from the previous iteration, so we have to put ourselves
     * to sleep until the next 0.1 second tick.  The first step is to
     * calculate how long we took processing the previous iteration.
     */
    
    gettimeofday(&before_sleep, (struct timezone *) 0); /* current time */
    timediff(&process_time, &before_sleep, &last_time);

    /*
     * If we were asleep for more than one pass, count missed pulses and sleep
     * until we're resynchronized with the next upcoming pulse.
     */
    if (process_time.tv_sec == 0 && process_time.tv_usec < OPT_USEC) {
      missed_pulses = 0;
    } else {
      missed_pulses = process_time.tv_sec * PASSES_PER_SEC;
      missed_pulses += process_time.tv_usec / OPT_USEC;
      process_time.tv_sec = 0;
      process_time.tv_usec = process_time.tv_usec % OPT_USEC;
    }

    /* Calculate the time we should wake up */
    timediff(&temp_time, &opt_time, &process_time);
    timeadd(&last_time, &before_sleep, &temp_time);
    
    /* Now keep sleeping until that time has come */
    gettimeofday(&now, (struct timezone *) 0);
    timediff(&timeout, &last_time, &now);

    /* Go to sleep */
    do {
      circle_sleep(&timeout);
      gettimeofday(&now, (struct timezone *) 0);
      timediff(&timeout, &last_time, &now);
    } while (timeout.tv_usec || timeout.tv_sec);

    /* Poll (without blocking) for new input, output, and exceptions */
    if (select(maxdesc + 1, &input_set, &output_set, &exc_set, &null_time) < 0) {
      perror("SYSERR: Select poll");
      return;
    }
    /* If there are new connections waiting, accept them. */
    if (FD_ISSET(mother_desc, &input_set))
      new_descriptor(mother_desc);

    /* Kick out the freaky folks in the exception set and marked for close */
    for (d = descriptor_list; d; d = next_d) {
      next_d = d->next;
      if (FD_ISSET(d->descriptor, &exc_set)) {
        FD_CLR(d->descriptor, &input_set);
        FD_CLR(d->descriptor, &output_set);
        close_socket(d);
      }
    }

    /* Process descriptors with input pending */
    for (d = descriptor_list; d; d = next_d) {
      next_d = d->next;
      if (FD_ISSET(d->descriptor, &input_set))
        if (process_input(d) < 0)
          close_socket(d);
    }
    
    /* Process commands we just read from process_input */
    for (d = descriptor_list; d; d = next_d) {
      next_d = d->next;

      /* Not combined to retain --(d->wait) behavior. -gg 2/20/98 */
       if (d->character) {
        GET_WAIT_STATE(d) -= (GET_WAIT_STATE(d) > 0);

        if (GET_WAIT_STATE(d))
          continue;
      }

      if (!get_from_q(&d->input, comm, &aliased))
        continue;

      if (d->character) {
        /* Reset the idle timer & pull char back from void if necessary */
        d->character->char_specials.timer = 0;
        if (STATE(d) == CON_PLAYING && GET_WAS_IN(d->character) != NOWHERE) {
          if (d->character->in_room != NOWHERE)
            char_from_room(d->character);
          char_to_room(d->character, GET_WAS_IN(d->character));
          GET_WAS_IN(d->character) = NOWHERE;
          act("$n has returned.", TRUE, d->character, 0, 0, TO_ROOM);
        }
        GET_WAIT_STATE(d) = 1;
      }      
      d->has_prompt = 0;

      if (d->str)		/* Writing boards, mail, etc. */
	string_add(d, comm);
      else if (d->showstr_count) /* Reading something w/ pager */
	show_string(d, comm);
      else if (STATE(d) != CON_PLAYING) /* In menus, etc. */
	nanny(d, comm);
      else {			/* else: we're playing normally. */
	if (aliased)		/* To prevent recursive aliases. */
	  d->has_prompt = 1;	/* To get newline before next cmd output. */
	else if (perform_alias(d, comm))    /* Run it through aliasing system */
	  get_from_q(&d->input, comm, &aliased);
	command_interpreter(d->character, comm); /* Send it to interpreter */
      }
    }  
   
    /* Send queued output out to the operating system (ultimately to user) */
    for (d = descriptor_list; d; d = next_d)
     {
      next_d = d->next;
      if ((d->output) && (d->descriptor))
       {
        if (*(d->output) && FD_ISSET(d->descriptor, &output_set))
         {
          /* Output for this player is ready */
          if (process_output(d) < 0)
            close_socket(d);
          else
            d->has_prompt = 1;
         }
        }
       
       else
        {
         /*
          * if we were using a large buffer, put the large buffer on the buffer pool
          * and switch back to the small one
         */
         if (d->large_outbuf)
          {   
           d->large_outbuf->next = bufpool;
           bufpool = d->large_outbuf;
           d->large_outbuf = NULL;
           d->output = d->small_outbuf;      
          }         
         /* reset total bufspace back to that of a small buffer */
         d->bufspace = SMALL_BUFSIZE - 1;
         d->bufptr = 0;
         *(d->output) = '\0';
       }  
    }

    /* Print prompts for other descriptors who had no other output */
    for (d = descriptor_list; d; d = d->next) {
      if (!d->has_prompt) {
        write_to_descriptor(d->descriptor, make_prompt(d));
        d->has_prompt = 1;
      }
    }

    /* Kick out folks in the CON_CLOSE or CON_DISCONNECT state */
    for (d = descriptor_list; d; d = next_d) {
      next_d = d->next;
      if (STATE(d) == CON_CLOSE || STATE(d) == CON_DISCONNECT)
        close_socket(d);
    }

    /*
     * Now, we execute as many pulses as necessary--just one if we haven't
     * missed any pulses, or make up for lost time if we missed a few
     * pulses by sleeping for too long.
     */
    missed_pulses++;

    if (missed_pulses <= 0) {
      log("SYSERR: **BAD** MISSED_PULSES NONPOSITIVE (%d), TIME GOING BACKWARDS!!", missed_pulses);
      missed_pulses = 1;
    }

    /* If we missed more than 30 seconds worth of pulses, just do 30 secs */
    if (missed_pulses > (30 * PASSES_PER_SEC)) {
      log("SYSERR: Missed %d seconds worth of pulses.", missed_pulses / PASSES_PER_SEC);
      missed_pulses = 30 * PASSES_PER_SEC;
    }

    /* Now execute the heartbeat functions */
    while (missed_pulses--)
      heartbeat(++pulse);

    /* Roll pulse over after 10 hours */
    if (pulse >= (600 * 60 * PASSES_PER_SEC))
      pulse = 0;

#ifdef CIRCLE_UNIX
    /* Update tics for deadlock protection (UNIX only) */
    tics++;
    write_mud_date_to_file();
#endif
  }
}

void save_all_chars(void)
{
  struct char_data *i, *next_char;
  void write_aliases(struct char_data *ch);
  
  for (i = character_list; i; i = next_char)
  {
    next_char = i->next;
    if(!i)
    	return;	
    if(IS_NPC(i))
    	return;
    if(LIBERDADE(i) > 0)
    	LIBERDADE(i)--;
    save_char(i, i->in_room);
    write_aliases(i);
  }
}

void heartbeat(int pulse)
{
  static int mins_since_crashsave = 0;
  void auction_update();
  void process_events(void);
  void process_program_output(void);
	
  process_program_output();

  /* Clear out all the global buffers now in case someone forgot. */
 /* if (!(pulse % PULSE_BUFFER))
    release_all_buffers();*/
  
  dg_global_pulse++;

  process_events();

  if (!(pulse % PULSE_DG_SCRIPT))
    script_trigger_check();

  if (teleport_on && !(pulse % PULSE_TELEPORT))
    TeleportPulseStuff();
    
  if (!(pulse % (30 * PASSES_PER_SEC)))
    sanity_check();

  if (!(pulse % PULSE_ZONE))
    zone_update();

  if (!(pulse % (15 * PASSES_PER_SEC)))         /* 15 seconds */
    check_idle_passwords();

  if (!(pulse % (15 * PASSES_PER_SEC)))
    auction_update();

  if (!(pulse % PULSE_MOBILE))
    mobile_activity();

  if (!(pulse % PULSE_VIOLENCE))
    perform_violence();

  if (!(pulse % (5 * PASSES_PER_SEC)))       /* 5 seconds */
    update_breath();

  if (!(pulse % (10 * PASSES_PER_SEC)))       /* 10 seconds */
    mental_update();
  
  if (!(pulse % (60 * PASSES_PER_SEC)))
   tips();
    
  if (!(pulse % (15 * PASSES_PER_SEC)))       /* 15 seconds */
    hmm_update();
    
   if (!(pulse % (3600 * PASSES_PER_SEC)))     /* 1 hora */
    custo_clan();   

   if (!(pulse % (60 * PASSES_PER_SEC)))     /* 1 minuto */
    gods_update();

  
  /* Do Arena related stuff */
  if (in_arena == ARENA_START)
    if(!(pulse % PULSE_ARENA))
        start_arena(); 
  if (in_arena == ARENA_RUNNING)
    if(!(pulse % PULSE_ARENA))
        do_game();
	        
  if (!(pulse % (SECS_PER_MUD_HOUR * PASSES_PER_SEC))) {
    weather_and_time(1);
    affect_update();
    point_update();
  }
  
  if (!(pulse % (8 * PASSES_PER_SEC)))  
         dragon_upd();  
         
  if (auto_save && !(pulse % (60 * PASSES_PER_SEC))) {  /* 1 minute */
    if (++mins_since_crashsave >= autosave_time) {
      mins_since_crashsave = 0;
      save_all_chars();
      Crash_save_all();
      House_save_all();
    }
  }
  if (!(pulse % (5 * 60 * PASSES_PER_SEC)))     /* 5 minutes */
    record_usage();


}


/* ******************************************************************
*  general utility stuff (for local use)                            *
****************************************************************** */

/*
 *  new code to calculate time differences, which works on systems
 *  for which tv_usec is unsigned (and thus comparisons for something
 *  being < 0 fail).  Based on code submitted by ss@sirocco.cup.hp.com.
 */

/*
 * code to return the time difference between a and b (a-b).
 * always returns a nonnegative value (floors at 0).
 */
void timediff(struct timeval *rslt, struct timeval *a, struct timeval *b)
{
  if (a->tv_sec < b->tv_sec)
    *rslt = null_time;
  else if (a->tv_sec == b->tv_sec) {
    if (a->tv_usec < b->tv_usec)
      *rslt = null_time;
    else {
      rslt->tv_sec = 0;
      rslt->tv_usec = a->tv_usec - b->tv_usec;
    }
  } else {			/* a->tv_sec > b->tv_sec */
    rslt->tv_sec = a->tv_sec - b->tv_sec;
    if (a->tv_usec < b->tv_usec) {
      rslt->tv_usec = a->tv_usec + 1000000 - b->tv_usec;
      rslt->tv_sec--;
    } else
      rslt->tv_usec = a->tv_usec - b->tv_usec;
  }
}

/*
 * Add 2 time values.
 *
 * Patch sent by "d. hall" <dhall@OOI.NET> to fix 'static' usage.
 */
void timeadd(struct timeval *rslt, struct timeval *a, struct timeval *b)
{
  rslt->tv_sec = a->tv_sec + b->tv_sec;
  rslt->tv_usec = a->tv_usec + b->tv_usec;

  while (rslt->tv_usec >= 1000000) {
    rslt->tv_usec -= 1000000;
    rslt->tv_sec++;
  }
}


void record_usage(void)
{
  int sockets_connected = 0, sockets_playing = 0;
  struct descriptor_data *d;

  for (d = descriptor_list; d; d = d->next) {
    sockets_connected++;
    if (STATE(d) == CON_PLAYING)
      sockets_playing++;
  }

  log("nusage: %-3d sockets connected, %-3d sockets playing",
          sockets_connected, sockets_playing);

#ifdef RUSAGE
  {
    struct rusage ru;

    getrusage(RUSAGE_SELF, &ru);
    log("rusage: user time: %ld sec, system time: %ld sec, max res size: %ld",
            ru.ru_utime.tv_sec, ru.ru_stime.tv_sec, ru.ru_maxrss);
  }
#endif

}



/*
 * Turn off echoing (specific to telnet client)
 */
void echo_off(struct descriptor_data *d)
{
  char off_string[] =
  {
    (char) IAC,
    (char) WILL,
    (char) TELOPT_ECHO,
    (char) 0,
  };

  SEND_TO_Q(off_string, d);
}


/*
 * Turn on echoing (specific to telnet client)
 */
void echo_on(struct descriptor_data *d)
{
  char on_string[] =
  {
    (char) IAC,
    (char) WONT,
    (char) TELOPT_ECHO,
    (char) TELOPT_NAOFFD,
    (char) TELOPT_NAOCRD,
    (char) 0,
  };

  SEND_TO_Q(on_string, d);
}

char *make_prompt(struct descriptor_data *d)
{
  static char prompt[MAX_PROMPT_LENGTH + 1]; 
 
  /* Note, prompt is truncated at MAX_PROMPT_LENGTH chars (structs.h )*/

  if (d->showstr_count)
    sprintf(prompt,
            "%s<<< Press ENTER (%d%s) >>>%s ",
            KNRM, (100-((d->showstr_page*100)/d->showstr_count)),"%", KNRM);
  else if (d->str)
    strcpy(prompt, "] ");
  else if (STATE(d) == CON_PLAYING && !IS_NPC(d->character)) {
    *prompt = '\0';

   if(PRF_FLAGGED(d->character, PRF_INFO_START))
      sprintf(prompt, "%sDigite/Type [INFO]%s ", CCBYEL(d->character, C_NRM), KNRM);
   else if(PRF_FLAGGED(d->character, PRF_AFK))
      sprintf(prompt, "%s[AFK]%s ", CCBRED(d->character, C_NRM), KNRM);
   else if(PLR_FLAGGED(d->character, PLR_DEAD))
      sprintf(prompt, "%s[YOU ARE DEAD]%s ", CCBRED(d->character, C_NRM), KNRM);
   else {
    if (GET_INVIS_LEV(d->character))
      sprintf(prompt, "%si%d ",
        CCCYN(d->character, C_NRM), GET_INVIS_LEV(d->character));

    if (PRF2_FLAGGED(d->character, PRF2_DISPEXP))
      sprintf(prompt + strlen(prompt), "%s[%s%s%s] ", CCCYN(d->character, C_NRM), 
        (GET_HIT(d->character) == GET_MAX_HIT(d->character) ? CCBWHT(d->character, C_NRM) :
          (GET_EXP(d->character) < level_exp(GET_REMORT(d->character), GET_LEVEL(d->character) + 1)*0.9 ? CCBWHT(d->character, C_NRM) : CCBCYN(d->character, C_NRM))),
        convert_num(level_exp(GET_REMORT(d->character), GET_LEVEL(d->character) + 1)-GET_EXP(d->character)), CCCYN(d->character, C_NRM));
        
    if (PRF_FLAGGED(d->character, PRF_DISPHP))
      sprintf(prompt + strlen(prompt), "%s[%s%s%s]%sHp", CCWHT(d->character, C_NRM), 
        (GET_HIT(d->character) == GET_MAX_HIT(d->character) ? CCBWHT(d->character, C_NRM) :
          (GET_HIT(d->character) > GET_MAX_HIT(d->character) / 10 ? CCBWHT(d->character, C_NRM) : CCBRED(d->character, C_NRM))),
        add_points(GET_HIT(d->character)), CCWHT(d->character, C_NRM), CCBBLU(d->character, C_NRM));

    if (PRF_FLAGGED(d->character, PRF_DISPMANA))
      sprintf(prompt + strlen(prompt), "%s[%s%s%s]%sMn", CCWHT(d->character, C_NRM), 
        (GET_MANA(d->character) == GET_MAX_MANA(d->character) ? CCBWHT(d->character, C_NRM) :
          (GET_MANA(d->character) > GET_MAX_MANA(d->character) / 10 ? CCBWHT(d->character, C_NRM) : CCBRED(d->character, C_NRM))),
        add_points(GET_MANA(d->character)), CCWHT(d->character, C_NRM), CCBBLU(d->character, C_NRM));

    if (PRF_FLAGGED(d->character, PRF_DISPMENT))
      sprintf(prompt + strlen(prompt), "%s[%s%s%s]%sMp", CCWHT(d->character, C_NRM), 
        (GET_MENTAL(d->character) == GET_MAX_MENTAL(d->character) ? CCBWHT(d->character, C_NRM) :
          (GET_MENTAL(d->character) > GET_MAX_MENTAL(d->character) / 10 ? CCBWHT(d->character, C_NRM) : CCBRED(d->character, C_NRM))),
        add_points(GET_MENTAL(d->character)), CCWHT(d->character, C_NRM), CCBBLU(d->character, C_NRM));
    
    if (PRF_FLAGGED(d->character, PRF_DISPMOVE))
      sprintf(prompt + strlen(prompt), "%s[%s%s%s]%sMv", CCWHT(d->character, C_NRM), 
        (GET_MOVE(d->character) == GET_MAX_MOVE(d->character) ? CCBWHT(d->character, C_NRM) :
          (GET_MOVE(d->character) > GET_MAX_MOVE(d->character) / 10 ? CCBWHT(d->character, C_NRM) : CCBRED(d->character, C_NRM))),
        add_points(GET_MOVE(d->character)), CCWHT(d->character, C_NRM), CCBBLU(d->character, C_NRM));
   }
   
   if(!PLR_FLAGGED(d->character, PLR_DEAD) && GET_OXI(d->character) < 100 && GET_LEVEL(d->character) < LVL_IMMORT && !PRF_FLAGGED(d->character, PRF_INFO_START))
      sprintf(prompt + strlen(prompt), "%s[%s%d%s]%s%sOx", CCWHT(d->character, C_NRM), 
        (GET_OXI(d->character) == GET_MAX_OXI(d->character) ? CCBWHT(d->character, C_NRM) :
          (GET_OXI(d->character) > GET_MAX_OXI(d->character) / 10 ? CCBWHT(d->character, C_NRM) : CCBRED(d->character, C_NRM))),
        GET_OXI(d->character), CCWHT(d->character, C_NRM), "%", CCBBLU(d->character, C_NRM));

   if(!PLR_FLAGGED(d->character, PLR_DEAD) && FIGHTING(d->character))
	sprintf(prompt + strlen(prompt), " %s%s%s", CCBWHT(d->character, C_NRM), make_bar(GET_HIT(FIGHTING(d->character)), GET_MAX_HIT(FIGHTING(d->character)), 10, (clr(d->character, C_CMP) ? TRUE : FALSE)), KNRM);
   
    sprintf(prompt + strlen(prompt), "%s>%s ", CCBBLU(d->character, C_NRM), KNRM);
  } else if (STATE(d) == CON_PLAYING && IS_NPC(d->character))
    sprintf(prompt, "%s> ", GET_NAME(d->character));
  else
    *prompt = '\0';

 return (prompt);  
 
}

void write_to_q(const char *txt, struct txt_q *queue, int aliased)
{
  struct txt_block *newt;

  CREATE(newt, struct txt_block, 1);
  newt->text = str_dup(txt);
  newt->aliased = aliased;

  /* queue empty? */
  if (!queue->head) {
    newt->next = NULL;
    queue->head = queue->tail = newt;
  } else {
    queue->tail->next = newt;
    queue->tail = newt;
    newt->next = NULL;
  }
}



int get_from_q(struct txt_q *queue, char *dest, int *aliased)
{
  struct txt_block *tmp;

  /* queue empty? */
  if (!queue->head)
    return (0);

  tmp = queue->head;
  strcpy(dest, queue->head->text);
  *aliased = queue->head->aliased;
  queue->head = queue->head->next;

  free(tmp->text);
  free(tmp);

  return (1);
}



/* Empty the queues before closing connection */
void flush_queues(struct descriptor_data *d)
{
  int dummy;
  
  if (d->large_outbuf) {   
    d->large_outbuf->next = bufpool;
    bufpool = d->large_outbuf;
  }
  while (get_from_q(&d->input, buf2, &dummy));  
}

int count_chars(const char *txt, char character)
{
   int i, cnt = 0;

   for(i = 0; txt[i]; i++)
      if(txt[i] == character)
         cnt++;

   return cnt;
}

/*
char *parse_color(const char *txt, struct descriptor_data *t)
{
   char *new_txt;
   char *toret;
   register int i, j = 0;

   i = count_chars(txt, '&');

   new_txt = malloc(i * 5 + strlen(txt) + 1);

   if (!i || t->connected == CON_GET_NAME){
   	strcpy(new_txt, txt);
   	toret = strdup(new_txt);
        really_free(new_txt);

        return toret;
	
//	strcpy(new_txt, txt);
//	return new_txt;
   }

   if (clr(t->character, C_NRM)){
     for(i = 0; txt[i]; i++)
     {
        if(txt[i] == '&'){
        i++;

	 switch(txt[i])
	 {
	    case 'n' :
	      strcpy(new_txt + j, KNRM);
	      j += 6;
              break;
            case 'r' : 
	      strcpy(new_txt + j, KRED);
              j += 7;
              break;
            case 'g' :
	      strcpy(new_txt + j, KGRN);
              j += 7;
              break;
	    case 'y' :
	      strcpy(new_txt + j, KYEL);
	      j += 7;
	      break;
	    case 'b' :
	      strcpy(new_txt + j, KBLU);
	      j += 7;
	      break;
            case 'm' : 
              strcpy(new_txt + j, KMAG);
              j += 7;
              break;
            case 'c' : 
              strcpy(new_txt + j, KCYN);
              j += 7;
              break;
            case 'w' : 
              strcpy(new_txt + j, KWHT);
              j += 7;
              break;

            case 'R' : 
              strcpy(new_txt + j, KBRED);
              j += 7;
              break;
            case 'G' : 
              strcpy(new_txt + j, KBGRN);
              j += 7;
              break;
            case 'Y' :
              strcpy(new_txt + j, KBYEL);
              j += 7;
              break;
            case 'B' : 
              strcpy(new_txt + j, KBBLU);
              j += 7;
              break;
            case 'M' : 
              strcpy(new_txt + j, KBMAG);
              j += 7;
              break;
            case 'C' : 
              strcpy(new_txt + j, KBCYN);
              j += 7;
              break;
            case 'W' : 
              strcpy(new_txt + j, KBWHT);
              j += 7;
              break;

            case '&' : new_txt[j] = txt[i]; j++; break;

            default:  break;
         }

      }
      else
      {
         new_txt[j] = txt[i]; j++;
      }
   }

   } else {
      for(i = 0; txt[i]; i++)
        if(txt[i] == '&'){
           i++;
	   if (txt[i] == '&'){
	      new_txt[j] = txt[i];
	      j++;
           }
        } else {
           new_txt[j] = txt[i]; j++;
        }
   }



   new_txt[j] = '\0';
   toret = strdup(new_txt);
   really_free(new_txt);

   return toret;
}
*/


char *parse_color(const char *txt, struct descriptor_data *t)
{
  char *new_txt;
  char *toret;
  int i, j = 0;

   i = count_chars(txt, '&');
       
   new_txt = malloc(i * 5 + strlen(txt) + 1);
   
   if (!i || t->connected == CON_GET_NAME){  	
	strcpy(new_txt, txt);
	toret = str_dup(new_txt);
        free(new_txt);
        return (toret);
    }
  
   if (clr(t->character, C_NRM)){
     for(i = 0; txt[i]; i++)
     {
        if(txt[i] == '&'){
        i++;

	 switch(txt[i])
	 {
	    case 'n' :
	      strcpy(new_txt + j, KNRM);
	      j += 6;
              break;
            case 'r' : 
	      strcpy(new_txt + j, KRED);
              j += 7;
              break;
            case 'g' :
	      strcpy(new_txt + j, KGRN);
              j += 7;
              break;
	    case 'y' :
	      strcpy(new_txt + j, KYEL);
	      j += 7;
	      break;
	    case 'b' :
	      strcpy(new_txt + j, KBLU);
	      j += 7;
	      break;
            case 'm' : 
              strcpy(new_txt + j, KMAG);
              j += 7;
              break;
            case 'c' : 
              strcpy(new_txt + j, KCYN);
              j += 7;
              break;
            case 'w' : 
              strcpy(new_txt + j, KWHT);
              j += 7;
              break;

            case 'R' : 
              strcpy(new_txt + j, KBRED);
              j += 7;
              break;
            case 'G' : 
              strcpy(new_txt + j, KBGRN);
              j += 7;
              break;
            case 'Y' :
              strcpy(new_txt + j, KBYEL);
              j += 7;
              break;
            case 'B' : 
              strcpy(new_txt + j, KBBLU);
              j += 7;
              break;
            case 'M' : 
              strcpy(new_txt + j, KBMAG);
              j += 7;
              break;
            case 'C' : 
              strcpy(new_txt + j, KBCYN);
              j += 7;
              break;
            case 'W' : 
              strcpy(new_txt + j, KBWHT);
              j += 7;
              break;
	    case 'u' ://underline
              strcpy(new_txt + j, KUND);
              j += 4;
              break;
            case 'd' : 
              strcpy(new_txt + j, KDAR);
              j += 4;
              break;
              case 'f' : 
              strcpy(new_txt + j, FLASH);
              j += 4;
              break;
	      	
            case '&' : new_txt[j] = txt[i]; j++; break;

            default:  break;
         }

      }
      else
      {
         new_txt[j] = txt[i]; j++;
      }
   }

   } else {
      for(i = 0; txt[i]; i++)
        if(txt[i] == '&'){
           i++;
	   if (txt[i] == '&'){
	      new_txt[j] = txt[i];
	      j++;
           }
        } else {
           new_txt[j] = txt[i]; j++;
        }
   }

   new_txt[j] = '\0';
   toret = str_dup(new_txt);
   free(new_txt);

   return (toret);
}



void write_to_output(const char *txt, struct descriptor_data *t)
{
  int size;
  char *new_txt;

  /* if we're in the overflow state already, ignore this new output */
  if (t->bufptr < 0)
    return;
  
  new_txt = parse_color(txt, t);

  size = strlen(new_txt);  
  
  /* if we have enough space, just write to buffer and that's it! */
  if (t->bufspace >= size) {
    strcpy(t->output + t->bufptr, new_txt);
    t->bufspace -= size;
    t->bufptr += size;
    free(new_txt);
    return;
  }
  /*
   * If we're already using the large buffer, or if even the large buffer
   * is too small to handle this new text, chuck the text and switch to the
   * overflow state.
   */
   if ((size + t->bufptr) > (LARGE_BUFSIZE - 1)) {
    t->bufptr = -1;
    buf_overflows++;
    free(new_txt);
    return;
  }
  buf_switches++;
  
  /* if the pool has a buffer in it, grab it */
  if (bufpool != NULL) {
    t->large_outbuf = bufpool;
    bufpool = bufpool->next;
  } else {  				
    /* else create a new one */
   // leak_logging = 0;
    CREATE(t->large_outbuf, struct txt_block, 1);
    CREATE(t->large_outbuf->text, char, LARGE_BUFSIZE);
   // leak_logging = 1;
    buf_largecount++;
  }

  strcpy(t->large_outbuf->text, t->output);	/* copy to big buffer */
  t->output = t->large_outbuf->text;	/* make big buffer primary */
  strcat(t->output, new_txt);	/* now add new text */

  /* set the pointer for the next write */
  t->bufptr = strlen(t->output);

  /* calculate how much space is left in the buffer */
  t->bufspace = LARGE_BUFSIZE - 1 - t->bufptr;
 
  free(new_txt);
    
}

/* ******************************************************************
*  socket handling                                                  *
****************************************************************** */


/*
 * get_bind_addr: Return a struct in_addr that should be used in our
 * call to bind().  If the user has specified a desired binding
 * address, we try to bind to it; otherwise, we bind to INADDR_ANY.
 * Note that inet_aton() is preferred over inet_addr() so we use it if
 * we can.  If neither is available, we always bind to INADDR_ANY.
 */
struct in_addr *get_bind_addr()
{
  static struct in_addr bind_addr;

  /* Clear the structure */
  memset((char *) &bind_addr, 0, sizeof(bind_addr));

  /* If DLFT_IP is unspecified, use INADDR_ANY */
  if (DFLT_IP == NULL)
  {
    bind_addr.s_addr = htonl(INADDR_ANY);
  } /* else 
   {
    If the parsing fails, use INADDR_ANY 
    if (!parse_ip(DFLT_IP, &bind_addr))
    {
      log("SYSERR: DFLT_IP of %s appears to be an invalid IP address",DFLT_IP);
      bind_addr.s_addr = htonl(INADDR_ANY);
    }
   } */ 

  /* Put the address that we've finally decided on into the logs */
  if (bind_addr.s_addr == htonl(INADDR_ANY))
    log("Binding to all IP interfaces on this host.");
  else
    log("Binding only to IP address %s", inet_ntoa(bind_addr));

  return (&bind_addr);
}

#ifdef HAVE_INET_ATON
//#define CRYPT
#define inet_aton
/*
 * inet_aton's interface is the same as parse_ip's: 0 on failure, non-0 if
 * successful
 */
/*int parse_ip(const char *addr, struct in_addr *inaddr)
{
  return(inet_aton(addr, inaddr));
}
*/
#elif HAVE_INET_ADDR

/* inet_addr has a different interface, so we emulate inet_aton's */
int parse_ip(const char *addr, struct in_addr *inaddr)
{
  long ip;

  if ((ip = inet_addr(addr)) == -1) {
    return (0);
  } else {
    inaddr->s_addr = (unsigned long) ip;
    return (1);
  }
}

#else

/* If you have neither function - sorry, you can't do specific binding. */
int parse_ip(const char *addr, struct in_addr *inaddr)
{
  log("SYSERR: warning: you're trying to set DFLT_IP but your system has no\n"
      "functions to parse IP addresses (how bizarre!)");
  return (0);
}

#endif /* INET_ATON and INET_ADDR */



/* Sets the kernel's send buffer size for the descriptor */
int set_sendbuf(socket_t s)
{
#if defined(SO_SNDBUF) && !defined(CIRCLE_MACINTOSH)
  int opt = MAX_SOCK_BUF;

  if (setsockopt(s, SOL_SOCKET, SO_SNDBUF, (char *) &opt, sizeof(opt)) < 0) {
    perror("SYSERR: setsockopt SNDBUF");
    return (-1);
  }

#if 0
  if (setsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *) &opt, sizeof(opt)) < 0) {
    perror("SYSERR: setsockopt RCVBUF");
    return (-1);
  }
#endif

#endif

  return (0);
}

/* Initialize a descriptor */
void init_descriptor (struct descriptor_data *newd, int desc)
{
    static int last_desc = 0;	/* last descriptor number */

	
	newd->descriptor = desc;	
	newd->idle_tics = 0;
	newd->wait = 1;
	newd->output = newd->small_outbuf;
	newd->bufspace = SMALL_BUFSIZE - 1;
	newd->login_time = time(0);
	*newd->output = '\0';
	newd->bufptr = 0;
	newd->has_prompt = 1;
	STATE(newd) = CON_GET_NAME;
      
	if (++last_desc == 1000)
		last_desc = 1;
	newd->desc_num = last_desc;
}

int new_descriptor(int s) {
  socket_t desc;
  int sockets_connected = 0;
  int i/*, connections = 0*/;
  struct descriptor_data *newd/*, *k, *next_k*/;
  struct sockaddr_in peer;
  struct hostent *from;
  
  /* accept the new connection */
  i = sizeof(peer);
  if ((desc = accept(s, (struct sockaddr *) &peer, &i)) == INVALID_SOCKET) {
    perror("SYSERR: accept");
    return (-1);
  }
  /* keep it from blocking */
  nonblock(desc);

  /* set the send buffer size */
  if (set_sendbuf(desc) < 0) {
    CLOSE_SOCKET(desc);
    return (0);
  }

  /* make sure we have room for it */
  for (newd = descriptor_list; newd; newd = newd->next)
    sockets_connected++;

  if (sockets_connected >= max_players) {
    write_to_descriptor(desc, "Sorry, CircleMUD is full right now... please try again later!\r\n");
    CLOSE_SOCKET(desc);
    return (0);
  }

  /* create a new descriptor */
  CREATE(newd, struct descriptor_data, 1);
 /* memset((char *) newd, 0, sizeof(struct descriptor_data));*/

  /* find the sitename */
  if (nameserver_is_slow || !(from = gethostbyaddr((char *) &peer.sin_addr,
                                      sizeof(peer.sin_addr), AF_INET))) {

    /* resolution failed */
    if (!nameserver_is_slow)
      perror("SYSERR: gethostbyaddr");

    /* find the numeric site address */
    strncpy(newd->host, (char *)inet_ntoa(peer.sin_addr), HOST_LENGTH);
    *(newd->host + HOST_LENGTH) = '\0';
  } else {
    strncpy(newd->host, from->h_name, HOST_LENGTH);
    *(newd->host + HOST_LENGTH) = '\0';
  }

  /* determine if the site is banned */
  if (isbanned(newd->host) == BAN_ALL) {   	
    CLOSE_SOCKET(desc);
    sprintf(buf2, "Connection attempt denied from [%s]", newd->host);
    mudlog(buf2, CMP, LVL_GOD, TRUE);
    free(newd);    
    return (0);
  }

#if 0
  /* Log new connections - probably unnecessary, but you may want it */  
  sprintf(buf2, "New connection from [%s]", newd->host);
  mudlog(buf2, CMP, LVL_GOD, FALSE);  
#endif
/*

  newd->descriptor = desc;
  newd->connected = CON_GET_NAME;
  newd->idle_tics = 0;
  newd->wait = 1;
  newd->output = newd->small_outbuf;
  newd->bufspace = SMALL_BUFSIZE - 1;
  newd->login_time = time(0);
  *newd->output = '\0';
  newd->bufptr = 0;
  newd->has_prompt = 1;
*/
  init_descriptor(newd, desc);

  /*
   * This isn't exactly optimal but allows us to make a design choice.
   * Do we embed the history in descriptor_data or keep it dynamically
   * allocated and allow a user defined history size?
   */
  CREATE(newd->history, char *, HISTORY_SIZE);

  /* prepend to list */
  newd->next = descriptor_list;
  descriptor_list = newd;

  write_to_descriptor(desc, GREETINGS);

  return (0);
}


/*
 * Send all of the output that we've accumulated for a player out to
 * the player's descriptor.
 * FIXME - This will be rewritten before 3.1, this code is dumb.
 */
int process_output(struct descriptor_data *t)
{  
  int result;   
  char i[MAX_SOCK_BUF];
  
  /* we may need this \r\n for later -- see below */
  strcpy(i, "\r\n");

  /* now, append the 'real' output */
  strcpy(i + 2, t->output);

  /* if we're in the overflow state, notify the user */
  if (t->bufptr < 0)
    strcat(i, "**OVERFLOW**\r\n");

  /* add the extra CRLF if the person isn't in compact mode */
  if (STATE(t) == CON_PLAYING && t->character && !IS_NPC(t->character) && !PRF_FLAGGED(t->character, PRF_COMPACT))
    strcat(i + 2, "\r\n");

  /* add a prompt */
  strncat(i + 2, make_prompt(t), MAX_PROMPT_LENGTH);
  
  strcat(i, "\0");

  /*
   * now, send the output.  If this is an 'interruption', use the prepended
   * CRLF, otherwise send the straight output sans CRLF.
   */
  if (t->has_prompt)            /* && !t->connected) */
    result = write_to_descriptor(t->descriptor, i);
  else
    result = write_to_descriptor(t->descriptor, (i + 2));

  /* handle snooping: prepend "% " and send to snooper */
  if (t->snoop_by) {
    SEND_TO_Q("% ", t->snoop_by);
    SEND_TO_Q(t->output, t->snoop_by);
    SEND_TO_Q("%%", t->snoop_by);
  }
  /*
   * if we were using a large buffer, put the large buffer on the buffer pool
   * and switch back to the small one
   */
  if (t->large_outbuf) {   
     t->large_outbuf->next = bufpool;
     bufpool = t->large_outbuf;
     t->large_outbuf = NULL;
     t->output = t->small_outbuf;      
  }
  /* reset total bufspace back to that of a small buffer */
  t->bufspace = SMALL_BUFSIZE - 1;
  t->bufptr = 0;
  *(t->output) = '\0';

  return (result);
}


/*
 * perform_socket_write: takes a descriptor, a pointer to text, and a
 * text length, and tries once to send that text to the OS.  This is
 * where we stuff all the platform-dependent stuff that used to be
 * ugly #ifdef's in write_to_descriptor().
 *
 * This function must return:
 *
 * -1  If a fatal error was encountered in writing to the descriptor.
 *  0  If a transient failure was encountered (e.g. socket buffer full).
 * >0  To indicate the number of bytes successfully written, possibly
 *     fewer than the number the caller requested be written.
 *
 * Right now there are two versions of this function: one for Windows,
 * and one for all other platforms.
 */

#if defined(CIRCLE_WINDOWS)

ssize_t perform_socket_write(socket_t desc, const char *txt, size_t length)
{
  ssize_t result;

  result = send(desc, txt, length, 0);

  if (result > 0) {
    /* Write was sucessful */
    return (result);
  }

  if (result == 0) {
    /* This should never happen! */
    log("SYSERR: Huh??  write() returned 0???  Please report this!");
    return (-1);
  }

  /* result < 0: An error was encountered. */

  /* Transient error? */
  if (WSAGetLastError() == WSAEWOULDBLOCK || WSAGetLastError() == WSAEINTR)
    return (0);

  /* Must be a fatal error. */
  return (-1);
}

#else

#if defined(CIRCLE_ACORN)
#define write   socketwrite
#endif

/* perform_socket_write for all Non-Windows platforms */
ssize_t perform_socket_write(socket_t desc, const char *txt, size_t length)
{
  ssize_t result;

  result = write(desc, txt, length);

  if (result > 0) {
    /* Write was successful. */
    return (result);
  }

  if (result == 0) {
    /* This should never happen! */
    log("SYSERR: Huh??  write() returned 0???  Please report this!");
    return (-1);
  }

  /*
   * result < 0, so an error was encountered - is it transient?
   * Unfortunately, different systems use different constants to
   * indicate this.
   */

#ifdef EAGAIN           /* POSIX */
  if (errno == EAGAIN)
    return (0);
#endif

#ifdef EWOULDBLOCK      /* BSD */
  if (errno == EWOULDBLOCK)
    return (0);
#endif

#ifdef EDEADLK          /* Macintosh */
  if (errno == EDEADLK)
    return (0);
#endif

  /* Looks like the error was fatal.  Too bad. */
  return (-1);
}

#endif /* CIRCLE_WINDOWS */

    
/*
 * write_to_descriptor takes a descriptor, and text to write to the
 * descriptor.  It keeps calling the system-level write() until all
 * the text has been delivered to the OS, or until an error is
 * encountered.
 *
 * Returns:
 *  0  If all is well and good,
 * -1  If an error was encountered, so that the player should be cut off
 */
int write_to_descriptor(socket_t desc, const char *txt)
{
  size_t total;
  ssize_t bytes_written;

  total = strlen(txt);
  
  if (total > MAX_SOCK_BUF)
   total = MAX_SOCK_BUF; 

  while (total > 0) {
    bytes_written = perform_socket_write(desc, txt, total);

    if (bytes_written < 0) {
      /* Fatal error.  Disconnect the player. */
      perror("SYSERR: Write to socket");
      return (-1);
    } else if (bytes_written == 0) {
      /*
       * Temporary failure -- socket buffer full.  For now we'll just
       * cut off the player, but eventually we'll stuff the unsent
       * text into a buffer and retry the write later.  JE 30 June 98.
       */
      log("WARNING: write_to_descriptor: socket write would block, about to close");
      return (-1);
    } else {
      txt += bytes_written;
      total -= bytes_written;
    }
  }

  return (0);
}


/*
 * Same information about perform_socket_write applies here. I like
 * standards, there are so many of them. -gg 6/30/98
 */
ssize_t perform_socket_read(socket_t desc, char *read_point, size_t space_left)
{
  ssize_t ret;

#if defined(CIRCLE_ACORN)
  ret = recv(desc, read_point, space_left, MSG_DONTWAIT);
#elif defined(CIRCLE_WINDOWS)
  ret = recv(desc, read_point, space_left, 0);
#else
  ret = read(desc, read_point, space_left);
#endif

  /* Read was successful. */
  if (ret > 0)
    return (ret);

  /* read() returned 0, meaning we got an EOF. */
  if (ret == 0) {
    log("WARNING: EOF on socket read (connection broken by peer)");
    return (-1);
  }

  /*
   * read returned a value < 0: there was an error
   */

#if defined(CIRCLE_WINDOWS)     /* Windows */
  if (WSAGetLastError() == WSAEWOULDBLOCK || WSAGetLastError() == WSAEINTR)
    return (0);
#else

#ifdef EINTR            /* Interrupted system call - various platforms */
  if (errno == EINTR)
    return (0);
#endif

#ifdef EAGAIN           /* POSIX */
  if (errno == EAGAIN)
    return (0);
#endif

#ifdef EWOULDBLOCK      /* BSD */
  if (errno == EWOULDBLOCK)
    return (0);
#endif /* EWOULDBLOCK */

#ifdef EDEADLK          /* Macintosh */
  if (errno == EDEADLK)
    return (0);
#endif

#ifdef ECONNRESET
  if (errno == ECONNRESET)
    return (-1);
#endif

#endif /* CIRCLE_WINDOWS */

  /* We don't know what happened, cut them off. */
  perror("SYSERR: perform_socket_read: about to lose connection");
  return (-1);
}

int process_input(struct descriptor_data *t)
{
  int buf_length, failed_subst;
  ssize_t bytes_read;
  size_t space_left;
  char *ptr, *read_point, *write_point, *nl_pos = NULL;
  char tmp[MAX_INPUT_LENGTH];

  /* first, find the point where we left off reading data */
  buf_length = strlen(t->inbuf);
  read_point = t->inbuf + buf_length;
  space_left = MAX_RAW_INPUT_LENGTH - buf_length - 1;

  do {
    if (space_left <= 0) {
      log("WARNING: process_input: about to close connection: input overflow");
      return (-1);
    }

    bytes_read = perform_socket_read(t->descriptor, read_point, space_left);

    if (bytes_read < 0) /* Error, disconnect them. */
      return (-1);
    else if (bytes_read == 0)   /* Just blocking, no problems. */
      return (0);

    /* at this point, we know we got some data from the read */

    *(read_point + bytes_read) = '\0';  /* terminate the string */

    /* search for a newline in the data we just read */
    for (ptr = read_point; *ptr && !nl_pos; ptr++)
      if (ISNEWL(*ptr))
        nl_pos = ptr;

    read_point += bytes_read;
    space_left -= bytes_read;

/*
 * on some systems such as AIX, POSIX-standard nonblocking I/O is broken,
 * causing the MUD to hang when it encounters input not terminated by a
 * newline.  This was causing hangs at the Password: prompt, for example.
 * I attempt to compensate by always returning after the _first_ read, instead
 * of looping forever until a read returns -1.  This simulates non-blocking
 * I/O because the result is we never call read unless we know from select()
 * that data is ready (process_input is only called if select indicates that
 * this descriptor is in the read set).  JE 2/23/95.
 */
#if !defined(POSIX_NONBLOCK_BROKEN)
  } while (nl_pos == NULL);
#else
  } while (0);

  if (nl_pos == NULL)
    return (0);
#endif /* POSIX_NONBLOCK_BROKEN */

  /*
   * okay, at this point we have at least one newline in the string; now we
   * can copy the formatted data to a new array for further processing.
   */

  read_point = t->inbuf;

  while (nl_pos != NULL) {
    write_point = tmp;
    space_left = MAX_INPUT_LENGTH - 1;

    for (ptr = read_point; (space_left > 1) && (ptr < nl_pos); ptr++) {
      if (*ptr == '\b' || *ptr == 127) { /* handle backspacing or delete key */
        if (write_point > tmp) {
          if (*(--write_point) == '$') {
            write_point--;
            space_left += 2;
          } else
            space_left++;
        }
      } else if ((isascii(*ptr) && isprint(*ptr))) {
        if ((*(write_point++) = *ptr) == '$') {         /* copy one character */
          *(write_point++) = '$';       /* if it's a $, double it */
          space_left -= 2;
        } else
          space_left--;
      }
    }

    *write_point = '\0';

    if ((space_left <= 0) && (ptr < nl_pos)) {
      char buffer[MAX_INPUT_LENGTH + 64];

      sprintf(buffer, "Line too long.  Truncated to:\r\n%s\r\n", tmp);
      if (write_to_descriptor(t->descriptor, buffer) < 0)
        return (-1);
    }
    if (t->snoop_by) {
      SEND_TO_Q("% ", t->snoop_by);
      SEND_TO_Q(tmp, t->snoop_by);
      SEND_TO_Q("\r\n", t->snoop_by);
    }
    failed_subst = 0;

    if (*tmp == '!' && !(*(tmp + 1)))   /* Redo last command. */
      strcpy(tmp, t->last_input);
    else if (*tmp == '!' && *(tmp + 1)) {
      char *commandln = (tmp + 1);
      int starting_pos = t->history_pos,
          cnt = (t->history_pos == 0 ? HISTORY_SIZE - 1 : t->history_pos - 1);

      skip_spaces(&commandln);
      for (; cnt != starting_pos; cnt--) {
        if (t->history[cnt] && is_abbrev(commandln, t->history[cnt])) {
          strcpy(tmp, t->history[cnt]);
          strcpy(t->last_input, tmp);
          SEND_TO_Q(tmp, t); SEND_TO_Q("\r\n", t);
          break;
        }
        if (cnt == 0)   /* At top, loop to bottom. */
          cnt = HISTORY_SIZE;
      }
    } else if (*tmp == '^') {
      if (!(failed_subst = perform_subst(t, t->last_input, tmp)))
        strcpy(t->last_input, tmp);
    } else {
      strcpy(t->last_input, tmp);
      if (t->history[t->history_pos])
        /*free(t->history[t->history_pos]);*/       /* Clear the old line. */
      t->history[t->history_pos] = str_dup(tmp);        /* Save the new. */
      if (++t->history_pos >= HISTORY_SIZE)     /* Wrap to top. */
        t->history_pos = 0;
    }

    if (!failed_subst)
      write_to_q(tmp, &t->input, 0);

    /* find the end of this line */
    while (ISNEWL(*nl_pos))
      nl_pos++;

    /* see if there's another newline in the input buffer */
    read_point = ptr = nl_pos;
    for (nl_pos = NULL; *ptr && !nl_pos; ptr++)
      if (ISNEWL(*ptr))
        nl_pos = ptr;
  }

  /* now move the rest of the buffer up to the beginning for the next pass */
  write_point = t->inbuf;
  while (*read_point)
    *(write_point++) = *(read_point++);
  *write_point = '\0';

  return (1);
}


/* perform substitution for the '^..^' csh-esque syntax orig is the
 * orig string, i.e. the one being modified.  subst contains the
 * substition string, i.e. "^telm^tell"
 */
int perform_subst(struct descriptor_data *t, char *orig, char *subst)
{
  char newsub[MAX_INPUT_LENGTH + 5];
   
  char *first, *second, *strpos;

  /*
   * first is the position of the beginning of the first string (the one
   * to be replaced
   */
  first = subst + 1;

  /* now find the second '^' */
  if (!(second = strchr(first, '^'))) {
    SEND_TO_Q("Invalid substitution.\r\n", t);      
    return (1);
  }
  /* terminate "first" at the position of the '^' and make 'second' point
   * to the beginning of the second string */
  *(second++) = '\0';

  /* now, see if the contents of the first string appear in the original */
  if (!(strpos = strstr(orig, first))) {
    SEND_TO_Q("Invalid substitution.\r\n", t);    
    return (1);
  }
  /* now, we construct the new string for output. */

  /* first, everything in the original, up to the string to be replaced */
  strncpy(newsub, orig, (strpos - orig));
  newsub[(strpos - orig)] = '\0';

  /* now, the replacement string */
  strncat(newsub, second, (MAX_INPUT_LENGTH - strlen(newsub) - 1));

  /* now, if there's anything left in the original after the string to
   * replaced, copy that too. */
  if (((strpos - orig) + strlen(first)) < strlen(orig))
    strncat(newsub, strpos + strlen(first), (MAX_INPUT_LENGTH - strlen(newsub) - 1));

  /* terminate the string in case of an overflow from strncat */
  newsub[MAX_INPUT_LENGTH - 1] = '\0';
  strcpy(subst, newsub);  
     
  return (0);
}



void close_socket(struct descriptor_data *d)
{  
  char buf[128];  
  struct descriptor_data *temp;

  REMOVE_FROM_LIST(d, descriptor_list, next);
  CLOSE_SOCKET(d->descriptor);
  flush_queues(d);

  /* Forget snooping */
  if (d->snooping)
    d->snooping->snoop_by = NULL;

  if (d->snoop_by) {
    SEND_TO_Q("Your victim is no longer among us.\r\n", d->snoop_by);
    d->snoop_by->snooping = NULL;
  }

  /*. Kill any OLC stuff .*/
  switch (d->connected) {
    case CON_OEDIT:
    case CON_REDIT:
    case CON_ZEDIT:
    case CON_MEDIT:
    case CON_SEDIT:
    case CON_HEDIT:
    case CON_AEDIT:
    case CON_TRIGEDIT:
      cleanup_olc(d, CLEANUP_ALL);
    default:
      break;
  }

  if (d->character) {
    /* If we're switched, this resets the mobile taken. */
     d->character->desc = NULL;
    
    /*
     * Plug memory leak, from Eric Green.
     */
    if (!IS_NPC(d->character) && PLR_FLAGGED(d->character, PLR_MAILING) && d->str) {
      if (*(d->str))
        free(*(d->str));
      free(d->str);
    }
    if (STATE(d) == CON_PLAYING || STATE(d) == CON_DISCONNECT) {
      struct char_data *link_challenged = d->original ? d->original : d->character;	
      
      act("$n has lost $s link.", TRUE, link_challenged, 0, 0, TO_ROOM);
      if (!IS_NPC(link_challenged)) {
		  save_char(link_challenged, NOWHERE);
		  sprintf(buf, "Closing link to: %s.", GET_NAME(link_challenged));
		  mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(link_challenged)), TRUE);
		  link_challenged->player_specials->new_wait = GET_WAIT_STATE(d);
      }      
    } else {
      sprintf(buf, "Losing player: %s.",
              GET_NAME(d->character) ? GET_NAME(d->character) : "<null>");
      mudlog(buf, CMP, LVL_IMMORT, TRUE);
      free_char(d->character);
    }
  } else
    sprintf(buf, "Trying to entering on the mud failure: [%s]", d->host);
    mudlog(buf, CMP, LVL_IMMORT, TRUE);

  /* JE 2/22/95 -- part of my unending quest to make switch stable */
  if (d->original && d->original->desc)
    d->original->desc = NULL;

  /* Clear the command history. */
  if (d->history) {
    int cnt;
    for (cnt = 0; cnt < HISTORY_SIZE; cnt++)
      if (d->history[cnt])
        free(d->history[cnt]);
    free(d->history);
  }

  if (d->showstr_head)
    free(d->showstr_head);
  if (d->showstr_count)
    free(d->showstr_vector);
  if (d->storage)
    free(d->storage);
    
  free(d);
 
}



void check_idle_passwords(void)
{
  struct descriptor_data *d, *next_d;

  for (d = descriptor_list; d; d = next_d) {
    next_d = d->next;
    if (STATE(d) != CON_PASSWORD && STATE(d) != CON_GET_NAME && STATE(d) != CON_RMOTD 
    && STATE(d) != CON_MENU)
      continue;
    if (!d->idle_tics) {
      d->idle_tics++;
      continue;
    } else {
      echo_on(d);
      SEND_TO_Q("\r\nTimed out... The WarDome close your connection.\r\n", d);
      STATE(d) = CON_CLOSE;
    }
  }
}



/*
 * I tried to universally convert Circle over to POSIX compliance, but
 * alas, some systems are still straggling behind and don't have all the
 * appropriate defines.  In particular, NeXT 2.x defines O_NDELAY but not
 * O_NONBLOCK.  Krusty old NeXT machines!  (Thanks to Michael Jones for
 * this and various other NeXT fixes.)
 */

#if defined(CIRCLE_WINDOWS)

void nonblock(socket_t s)
{
  unsigned long val = 1;
  ioctlsocket(s, FIONBIO, &val);
}

#elif defined(CIRCLE_AMIGA)

void nonblock(socket_t s)
{
  long val = 1;
  IoctlSocket(s, FIONBIO, &val);
}

#elif defined(CIRCLE_ACORN)

void nonblock(socket_t s)
{
  int val = 1;
  socket_ioctl(s, FIONBIO, &val);
}

#elif defined(CIRCLE_UNIX) || defined(CIRCLE_OS2) || defined(CIRCLE_MACINTOSH) || defined(__APPLE__)

#ifndef O_NONBLOCK
#define O_NONBLOCK O_NDELAY
#endif

void nonblock(socket_t s)
{
  int flags;

  flags = fcntl(s, F_GETFL, 0);
  flags |= O_NONBLOCK;
  if (fcntl(s, F_SETFL, flags) < 0) {
    perror("SYSERR: Fatal error executing nonblock (comm.c)");
    exit(1);
  }
}

#endif  /* CIRCLE_UNIX || CIRCLE_OS2 || CIRCLE_MACINTOSH */


/* ******************************************************************
*  signal-handling functions (formerly signals.c).  UNIX only.      *
****************************************************************** */

#if defined(CIRCLE_UNIX) || defined(CIRCLE_MACINTOSH)

RETSIGTYPE reread_wizlists(int sig)
{
  mudlog("Signal received - rereading wizlists.", CMP, LVL_SUPGOD, TRUE);
  reboot_wizlists();
}


RETSIGTYPE unrestrict_game(int sig)
{
  mudlog("Received SIGUSR2 - completely unrestricting game (emergent)",
         BRF, LVL_IMMORT, TRUE);
  ban_list = NULL;
  circle_restrict = 0;
  num_invalid = 0;
}

#ifdef CIRCLE_UNIX

/* clean up our zombie kids to avoid defunct processes */
RETSIGTYPE reap(int sig)
{
  while (waitpid(-1, NULL, WNOHANG) > 0);

  my_signal(SIGCHLD, reap);
}

RETSIGTYPE checkpointing(int sig)
{
  if (!tics) {
    log("SYSERR: CHECKPOINT shutdown: tics not updated. (Infinite loop suspected)");
    abort();
  } else
    tics = 0;
}

RETSIGTYPE hupsig(int sig)
{
  log("SYSERR: Received SIGHUP, SIGINT, or SIGTERM.  Shutting down...");
  exit(1);
}

#endif	/* CIRCLE_UNIX */


/*
 * This is an implementation of signal() using sigaction() for portability.
 * (sigaction() is POSIX; signal() is not.)  Taken from Stevens' _Advanced
 * Programming in the UNIX Environment_.  We are specifying that all system
 * calls _not_ be automatically restarted for uniformity, because BSD systems
 * do not restart select(), even if SA_RESTART is used.
 *
 * Note that NeXT 2.x is not POSIX and does not have sigaction; therefore,
 * I just define it to be the old signal.  If your system doesn't have
 * sigaction either, you can use the same fix.
 *
 * SunOS Release 4.0.2 (sun386) needs this too, according to Tim Aldric.
 */

#ifndef POSIX
#define my_signal(signo, func) signal(signo, func)
#else
sigfunc *my_signal(int signo, sigfunc * func)
{
  struct sigaction act, oact;

  act.sa_handler = func;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
#ifdef SA_INTERRUPT
  act.sa_flags |= SA_INTERRUPT; /* SunOS */
#endif

  if (sigaction(signo, &act, &oact) < 0)
    return (SIG_ERR);

  return (oact.sa_handler);
}
#endif                          /* POSIX */


void signal_setup(void)
{
#ifndef CIRCLE_MACINTOSH
  struct itimerval itime;
  struct timeval interval;

  /* user signal 1: reread wizlists.  Used by autowiz system. */
  my_signal(SIGUSR1, reread_wizlists);

  /*
   * user signal 2: unrestrict game.  Used for emergencies if you lock
   * yourself out of the MUD somehow.  (Duh...)
   */
  my_signal(SIGUSR2, unrestrict_game);

  /*
   * set up the deadlock-protection so that the MUD aborts itself if it gets
   * caught in an infinite loop for more than 3 minutes.
   */
  interval.tv_sec = 180;
  interval.tv_usec = 0;
  itime.it_interval = interval;
  itime.it_value = interval;
  setitimer(ITIMER_VIRTUAL, &itime, NULL);
  my_signal(SIGVTALRM, checkpointing);

  /* just to be on the safe side: */
  my_signal(SIGHUP, hupsig);
  my_signal(SIGCHLD, reap);
#endif /* CIRCLE_MACINTOSH */
  my_signal(SIGINT, hupsig);
  my_signal(SIGTERM, hupsig);
  my_signal(SIGPIPE, SIG_IGN);
  my_signal(SIGALRM, SIG_IGN);
}

#endif	/* CIRCLE_UNIX || CIRCLE_MACINTOSH */

/* ****************************************************************
*       Public routines for system-to-player-communication        *
**************************************************************** */

void send_to_char(const char *messg, struct char_data *ch)
{
  if (ch->desc && messg)
    SEND_TO_Q(messg, ch->desc);
}


void send_to_all(const char *messg)
{
  struct descriptor_data *i;

  if (messg == NULL)
    return;

  for (i = descriptor_list; i; i = i->next)
    if (STATE(i) == CON_PLAYING)
      SEND_TO_Q(messg, i);
}


void send_to_outdoor(const char *messg)
{
  struct descriptor_data *i;

  if (!messg || !*messg)
    return;

  for (i = descriptor_list; i; i = i->next) {
    if (STATE(i) != CON_PLAYING || i->character == NULL)
      continue;
    if (!AWAKE(i->character) || !OUTSIDE(i->character))
      continue;
    SEND_TO_Q(messg, i);
  }
}



void send_to_room(const char *messg, int room)
{
  struct char_data *i;

  if (messg == NULL)
    return;

  for (i = world[room].people; i; i = i->next_in_room)
    if (i->desc)
      SEND_TO_Q(messg, i->desc);
}



const char *ACTNULL = "<NULL>";

#define CHECK_NULL(pointer, expression) \
  if ((pointer) == NULL) i = ACTNULL; else i = (expression);


/* higher-level communication: the act() function */
void perform_act(const char *orig, struct char_data *ch, struct obj_data *obj,
                const void *vict_obj, const struct char_data *to)
{
  const char *i = NULL;  
  char lbuf[MAX_STRING_LENGTH], *buf, *j; 
  const struct char_data *dg_victim = NULL;
  const struct obj_data *dg_target = NULL;
  const char *dg_arg = NULL;
  bool uppercasenext = FALSE;
  
  buf = lbuf;

  for (;;) {
    if (*orig == '$') {
      switch (*(++orig)) {
      case 'n':
        i = PERS(ch, to);
        break;
      case 'N':
        CHECK_NULL(vict_obj, PERS((const struct char_data *) vict_obj, to));
        dg_victim = (const struct char_data *) vict_obj;
        break;
      case 'm':
        i = HMHR(ch);
        break;
      case 'M':
        CHECK_NULL(vict_obj, HMHR((const struct char_data *) vict_obj));
        dg_victim = (const struct char_data *) vict_obj;
        break;
      case 's':
        i = HSHR(ch);
        break;
      case 'S':
        CHECK_NULL(vict_obj, HSHR((const struct char_data *) vict_obj));
        dg_victim = (const struct char_data *) vict_obj;
        break;
      case 'e':
        i = HSSH(ch);
        break;
      case 'E':
        CHECK_NULL(vict_obj, HSSH((const struct char_data *) vict_obj));
        dg_victim = (const struct char_data *) vict_obj;
        break;
      case 'o':
        CHECK_NULL(obj, OBJN(obj, to));
        break;
      case 'O':
        CHECK_NULL(vict_obj, OBJN((const struct obj_data *) vict_obj, to));
        dg_target = (const struct obj_data *) vict_obj;
        break;
      case 'p':
        CHECK_NULL(obj, OBJS(obj, to));
        break;
      case 'P':
        CHECK_NULL(vict_obj, OBJS((const struct obj_data *) vict_obj, to));
        dg_target = (const struct obj_data *) vict_obj;
        break;
      case 'a':
        CHECK_NULL(obj, SANA(obj));
        break;
      case 'A':
        CHECK_NULL(vict_obj, SANA((const struct obj_data *) vict_obj));
        dg_target = (const struct obj_data *) vict_obj;
        break;
      case 'T':
        CHECK_NULL(vict_obj, (const char *) vict_obj);
        break;
        dg_arg = (const char *) vict_obj;
      case 't':
	CHECK_NULL(obj, (char *) obj);
	break;
      case 'F':
        CHECK_NULL(vict_obj, fname((char *) vict_obj));
        break;
      /* uppercase previous word */
      case 'u':
        for (j=buf; j > lbuf && !isspace((int) *(j-1)); j--);
        if (j != buf)
          *j = UPPER(*j);
        i = "";
        break;
      /* uppercase next word */
      case 'U':
        uppercasenext = TRUE;
        i = "";
        break;
      case '$':
        i = "$";
        break;
      default:
        log("SYSERR: Illegal $-code to act(): %c", *orig);
        log("SYSERR: %s", orig);
        break;
      }
/*      while ((*buf = *(i++)))
        buf++;
      orig++;
    } else if (!(*(buf++) = *(orig++)))
      break;*/
      while ((*buf = *(i++)))
        {
        if (uppercasenext && !isspace((int) *buf))
          {
          *buf = UPPER(*buf);
          uppercasenext = FALSE;
          }
	buf++;
        }
      orig++;
    } else if (!(*(buf++) = *(orig++))) {
      break;
    } else if (uppercasenext && !isspace((int) *(buf-1))) {
      *(buf-1) = UPPER(*(buf-1));
      uppercasenext = FALSE;
    }
  }

  *(--buf) = '\r';
  *(++buf) = '\n';
  *(++buf) = '\0';

  if (to->desc)
    SEND_TO_Q(CAP(lbuf), to->desc);

  if ((IS_NPC(to) && dg_act_check) && (to != ch))
    act_mtrigger((struct char_data *)to, lbuf, ch, (struct char_data *)dg_victim, obj, (struct obj_data *)dg_target, (char *)dg_arg);
    
}

#ifndef SENDOK
#define SENDOK(ch)	((ch)->desc && (to_sleeping || AWAKE(ch)) && \
			(IS_NPC(ch) || !PLR_FLAGGED((ch), PLR_WRITING)))
#endif

void act(const char *str, int hide_invisible, struct char_data *ch,
         struct obj_data *obj, const void *vict_obj, int type)
{
  const struct char_data *to = NULL;
  int to_sleeping;
  struct descriptor_data *i;
  
  if (!str || !*str)
    return;

  if (!(dg_act_check = !(type & DG_NO_TRIG)))
    type &= ~DG_NO_TRIG;

  /*
   * Warning: the following TO_SLEEP code is a hack.
   * 
   * I wanted to be able to tell act to deliver a message regardless of sleep
   * without adding an additional argument.  TO_SLEEP is 128 (a single bit
   * high up).  It's ONLY legal to combine TO_SLEEP with one other TO_x
   * command.  It's not legal to combine TO_x's with each other otherwise.
   * TO_SLEEP only works because its value "happens to be" a single bit;
   * do not change it to something else.  In short, it is a hack.
   */

  /* check if TO_SLEEP is there, and remove it if it is. */
  if ((to_sleeping = (type & TO_SLEEP)))
    type &= ~TO_SLEEP;

  if (type == TO_CHAR) {
    if(ch)
      if(SENDOK(ch))
        perform_act(str, ch, obj, vict_obj, ch);
    return;
  }

  if (type == TO_VICT) {
    if ((to = (const struct char_data *) vict_obj) != NULL && SENDOK(to))
      perform_act(str, ch, obj, vict_obj, to);
    return;
  }
  /* ASSUMPTION: at this point we know type must be TO_NOTVICT or TO_ROOM */

  if (type == TO_GMOTE) {
    for (i = descriptor_list; i; i = i->next) {
      if (!i->connected && i->character &&
	!IS_NPC(i->character) &&
	!PRF_FLAGGED(i->character, PRF_NOGOSS) &&
	!PLR_FLAGGED(i->character, PLR_WRITING) &&
	!ROOM_FLAGGED(i->character->in_room, ROOM_SOUNDPROOF)) {
        perform_act(str, ch, obj, vict_obj, i->character);
      }
    }
    return;
  }


  if (ch) {
    if (ch->in_room != NOWHERE)
      to = world[ch->in_room].people;
  } else if (obj) {
    if (obj->in_room != NOWHERE)
      to = world[obj->in_room].people;
  } else {
    log("SYSERR: no valid target to act()!");  
    return;
  }
        
/* snip, taerin 4/12/99
   sometimes things die too rapidly
   sticking in separated pointer checks above
   this might fix the null pointer crashes
        
  if (ch && ch->in_room != NOWHERE)
    to = world[ch->in_room].people;
  else if (obj && obj->in_room != NOWHERE)
    to = world[obj->in_room].people;
  else {
    log("SYSERR: no valid target to act()!");
    return;
  } */  

  for (; to; to = to->next_in_room) {
    if (!SENDOK(to) || (to == ch))
      continue;
    if (hide_invisible && ch && !CAN_SEE(to, ch))
      continue;
    if (type != TO_ROOM && to == vict_obj)
      continue;
    perform_act(str, ch, obj, vict_obj, to);
  }
}

void write_mud_date_to_file(void)
{
   FILE *f;
   struct time_write date;

   f = fopen("date_record", "w");
   date.year = time_info.year;
   date.month = time_info.month;
   date.day   = time_info.day;
   fwrite(&date,sizeof(struct time_write),1,f);
   fclose(f);
}

/*
 * This function is called every 30 seconds from heartbeat().  It checks
 * the four global buffers in CircleMUD to ensure that no one has written
 * past their bounds.  If our check digit is not there (and the position
 * doesn't have a NUL which may result from snprintf) then we gripe that
 * someone has overwritten our buffer.  This could cause a false positive
 * if someone uses the buffer as a non-terminated character array but that
 * is not likely. -gg
 */
#define offset  (MAX_STRING_LENGTH - 1)
void sanity_check(void)
{
  int ok = TRUE;

  /*
   * If any line is false, 'ok' will become false also.
   */
  ok &= (buf[offset] == MAGIC_NUMBER || buf[offset] == '\0');
  ok &= (buf1[offset] == MAGIC_NUMBER || buf1[offset] == '\0');
  ok &= (buf2[offset] == MAGIC_NUMBER || buf2[offset] == '\0');
  ok &= (buf3[offset] == MAGIC_NUMBER || buf3[offset] == '\0');
  ok &= (arg[offset] == MAGIC_NUMBER || arg[offset] == '\0');

  /*
   * This isn't exactly the safest thing to do (referencing known bad memory)
   * but we're doomed to crash eventually, might as well try to get something
   * useful before we go down. -gg
   */
  if (!ok)
    log("SYSERR: *** Buffer overflow! ***\n"
        "buf: %s\nbuf1: %s\nbuf2: %s\nbuf3: %s\narg: %s", buf, buf1, buf2, buf3, arg);
    /* Initialize these to check for overruns later. */
    buf[MAX_STRING_LENGTH - 1] = buf1[MAX_STRING_LENGTH - 1] = MAGIC_NUMBER;
    buf2[MAX_STRING_LENGTH - 1] = arg[MAX_STRING_LENGTH - 1] = MAGIC_NUMBER;    
    buf3[MAX_STRING_LENGTH - 1] = MAGIC_NUMBER;
#if 0
  log("Statistics: buf=%d buf1=%d buf2=%d arg=%d",
        strlen(buf), strlen(buf1), strlen(buf2), strlen(arg));
#endif
}

/*
 * This may not be pretty but it keeps game_loop() neater than if it was inline.
 */
#if defined(CIRCLE_WINDOWS)

void circle_sleep(struct timeval *timeout)
{
  Sleep(timeout->tv_sec * 1000 + timeout->tv_usec / 1000);
}

#else

void circle_sleep(struct timeval *timeout)
{
  if (select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, timeout) < 0) {
    if (errno != EINTR) {
      perror("SYSERR: Select sleep");
      exit(1);
    }
  }
}

#endif /* CIRCLE_WINDOWS */

const char *sgod_wiznames[] =
{
  "Imm",
  "EId",
  "Lrd",
  "Dem",
  "God",
  "GrG",
  "SuG",
  "MjG",
  "Imp",
  "Own",
  "\n"
};

#define MAX_CORES		21

#define LETRA				0
#define CODIGOCOR			1

char *letra_cor[] =
{
	"n",
	"r",
	"g",
	"y",
	"b",
	"m",
	"c",
	"w",
	"R",
	"G",
	"Y",
	"B",
	"M",
	"C",
	"W",
        "f",
};

const char *codigo_cor[] =
{
	KNRM,
	KRED,
	KGRN,
	KYEL,
	KBLU,
	KMAG,
	KCYN,
	KWHT,
	KBRED,
	KBGRN,
	KBYEL,
	KBBLU,
	KBMAG,
	KBCYN,
	KBWHT,
	FLASH,
};
int conta_cores(const char *texto)
{
	short int cores;
	int pos, numero = 0;

	for(pos = 0; texto[pos]; pos++)
		if(texto[pos] == '&')
		{
			for(cores = 0; cores < MAX_CORES; cores++)
				if(texto[pos+1] == *letra_cor[cores])
					numero++;
		}

	return (numero);
}

char *colorir_cor(char *onde, const char *oque)
{
	short int cores;

	for(cores = 0; oque[cores]; cores++)
		*(onde++) = oque[cores];
	return (onde);
}

short int que_cor(char letra)
{
	short kor;

	for(kor = 0; kor < MAX_CORES; kor++)
		if(letra == *letra_cor[kor])
			return (kor);
	return (-1);
}

char *cores(const char *txt, struct descriptor_data *t)
{
	char *com_cor, *corbkp;
	short int pos, cor;
	int qtd, num_cores;

	num_cores = conta_cores(txt);

	if(!num_cores)
	{
		com_cor = strdup(txt);
		return (com_cor);
	}

	qtd = strlen(txt) + (num_cores * 5) + 1;

	corbkp = com_cor = malloc(qtd);

	for(pos = 0; txt[pos] != '\0'; pos++)
	{
		if(txt[pos] == '&' && txt[pos+1])
		{
			if((cor = que_cor(txt[pos+1])) != -1)
			{
					if(clr(t->character, C_CMP))
						corbkp = colorir_cor(corbkp, codigo_cor[cor]);
					pos++;
					continue;
			}
		}
		*(corbkp++) = txt[pos];
	}
	*corbkp = '\0';

	return (com_cor);
}
void tips(void) // da pra incrementar isso.. fazendo um file com varios tipos e pegando desse file. mas assim ta working
{
	  struct char_data *i, *next_char;

  for (i = character_list; i; i = next_char)
  {
    next_char = i->next;

    if (!(IS_NPC(i)))
    if (i && !(PRF2_FLAGGED(i, PRF2_NOTIPS)) && GET_POS(i)!=POS_FIGHTING)
	 switch (number(0, 9))	
	 {
	case 0:
		send_to_char("&r[&RDICA&r] &YPara ver um sumario do jogo digite summary.&n&n\r\n", i);
		break;
	case 1:
		send_to_char("&r[&RDICA&r] &YSe voce for newbie mate na newbie zone.&n\r\n", i);
		break;
		
	case 2:
		send_to_char("&r[&RDICA&r] &YNewbie zone outras zonas possuem seus paths no HELP GUIDE no seu inventory.&n\r\n", i);
		break;
			
	case 3:
		send_to_char("&r[&RDICA&r] &YPara ver seus stats digite SCORE.&n\r\n", i);
		break;
		
	case 4:
		send_to_char("&r[&RDICA&r] &YWardome Web-Page : http://www.geocities.com/wardome2004&n\r\n", i);
		break;
		
	case 5:
		send_to_char("&r[&RDICA&r] &YPara ser Player killer no level 10 digite PK.&n\r\n", i);
		break;
		
	case 6:
		send_to_char("&r[&RDICA&r] &YPara aperfeicoar suas spells e skills va no oracle a UP da fountain square.&n\r\n", i);
		break;	

	case 7:
		send_to_char("&r[&RDICA&r] &YEscreva TOGGLE para ligar comandos automaticos, como wimpy.&n\r\n", i);
		break;
	
	case 8:
		send_to_char("&r[&RDICA&r] &YCompre atributos como hp, mana, move no pitt bull's gym e no scientist.&n\r\n", i);
		break;
	
	case 9:
		send_to_char("&r[&RDICA&r] &YPara nao ver mais essas mensagens digite NOTIP.&n\r\n", i);
		break;
	
	case 10:
		send_to_char("&r[&RDICA&r] &YSe voce ja jogou aqui antes e ja sabe jogar nete mud escreva START.&n\r\n", i);
		break;
	case 11:
		send_to_char("&r[&RDICA&r] &YQuer jogar o mud com som? &CHELP MSP&n\r\n", i);
		break;
	case 12:
		send_to_char("&r[&RDICA&r] &YUse o comando &yNEWBIE &Y<&ymensagem&Y> para se comunicar se voce for iniciante&n\r\n", i);
		break;
	case 13:
		send_to_char("&r[&RDICA&r] &YEscolha sua religiao, HELP RELIGION&n\r\n", i);
		break;	
	default:
		send_to_char("&r[&RDICA&r] &YBUG REPORT.&n\r\n", i);
		break;
		}
    }
}

void brag(struct char_data *ch, struct char_data *vict)
{


struct descriptor_data *i;
char brag[256]={"       "};

   switch (number(0, 11)) {
  case 0:
   sprintf(brag, "&R$n &rbrags, '&R%s &rwas just too easy a kill!'&n", GET_NAME(vict));
  break;
  case 1:
   sprintf(brag, "&R$n &rbrags, '&R%s &rwas a tasty dinner, now who's for desert? Muhaha!'&n", GET_NAME(vict));
  break;
  case 2:
   sprintf(brag, "&R$n &rbrags, 'Bahaha! &R%s &rshould stick to Odif's !'&n",GET_NAME(vict));
  break;
  case 3:
   sprintf(brag, "&R$n &rbrags, '&R%s &ris now in need of some exp...Muhaha!'&n", GET_NAME(vict));
  break;
  case 4:
   sprintf(brag, "&R$n &rbrags, '&R%s &rneeds a hospital now. Muhaha!&n",GET_NAME(vict));
  break;
  case 5:
   sprintf(brag, "&R$n &rbrags, '&R%s's &rmother is a slut!  Muhaha!'&n", GET_NAME(vict));
  break;
  case 6:
   sprintf(brag, "&R$n &rbrags, '&R%s &ris a punk, hits like a swampfly. Bah.'&n", GET_NAME(vict));
  break;
  case 7:
   sprintf(brag, "&R$n &rbrags, '&R%s, &ryour life force has just run out...Muahaha!'&n", GET_NAME(vict));
  break;
  case 8:
   sprintf(brag, "&R$n &rbrags, 'Bah, &R%s &rshould stick to the newbie zone!'&n",GET_NAME(vict));
  break;
  case 9:
   sprintf(brag, "&R$n &rbrags, '&R%s, &rgive me your daughter's number and I might return your corpse.  Muhaha!'&n", GET_NAME(vict));
  break;
  case 10:
   sprintf(brag, "&R$n &rbrags, 'Hey &R%s!  &rCome back, you dropped your corpse! Muahaha'&n", GET_NAME(vict));
  break;
  case 11:
   sprintf(brag, "&R$n &rbrags, 'I think &R%s &rwears pink chainmail.  Fights like a girl!  Muhaha!&n", GET_NAME(vict));
  break;
  }

   for (i = descriptor_list; i; i = i->next)
     if (!i->connected && i != ch->desc && i->character &&
        !PRF_FLAGGED(i->character, PRF_NOGOSS) &&
        !PLR_FLAGGED(i->character, PLR_WRITING) &&
        !ROOM_FLAGGED(i->character->in_room, ROOM_SOUNDPROOF))  {

    // if (COLOR_LEV(i->character) >= C_NRM)
    //    send_to_char(CCRED(i->character, C_NRM), i->character);

        act(brag, FALSE, ch, 0, i->character, TO_VICT | TO_SLEEP);

    // if (COLOR_LEV(i->character) >= C_NRM)
      //  send_to_char(CCNRM(i->character, C_NRM), i->character);
   }
}	
