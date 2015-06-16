/*
 * thread.c - Testing stub for buffer.c
 * Copyright 1997, 1998, George Greer
 * License: public domain
 * Warranty: none at all
 */

#include "conf.h"
#include "sysdep.h"
#include <pthread.h>
#include "structs.h"
#include "buffer.h"
#include "utils.h"

void _thr_main() {}

/* Stubs! */
long random();
void send_to_all(char *t) { printf("%s", t); }
void send_to_char(char *t, struct char_data *ch) { printf("%s", t); }
int MIN(int a, int b) { return a < b ? a : b; }
int MAX(int a, int b) { return a > b ? a : b; }
int number(int from, int to) { return ((random() % (to - from + 1)) + from); }

int circle_shutdown = 0;
int circle_reboot = 0;

void log(const char *str, ...)
{
  time_t ct;
  char *tmstr;

  ct = time(0);
  tmstr = asctime(localtime(&ct));
  *(tmstr + strlen(tmstr) - 1) = '\0';
  fprintf(stdout, "%-15.15s :: %s\n", tmstr + 4, str);
  fflush(stdout);
}

void *thread(void *hi)
{
  FILE *dn;
  int i;
  char *tbuf, backup[128];;

  sleep(1);
  for (i = 0; i < INT_MAX; i++) {
    /* Grab a buffer. */
    tbuf = get_buffer(number(21,3000));

    /* Dump some data in it. */
    strcpy(tbuf, "12345678901234567890");
    sprintf(tbuf, "%d ", i);
    strcpy(backup, tbuf);

    /* Delay */
    dn = fopen("/dev/null", "w");
    fprintf(dn, "%s", tbuf);
    fclose(dn);

    /* Check */
    if (strcmp(backup, tbuf)) { /* different */
      sprintf(backup, "thread on %d: Someone changed my buffer!", i);
      log(backup);
    }

    /* Get rid of the buffer. */
    release_buffer(tbuf);
    if (circle_shutdown)
      return NULL;
  }
  log("thread: done.");
  return NULL;
}

#define NUM_THREADS	60

int main(void)
{
  int i;
  char *mbuf;
  pthread_t thread_id[NUM_THREADS];
  extern ush_int buffer_opt;

  buffer_opt |= (1 << 2);
//  buffer_opt |= (1 << 4);

  log("main: init_buffers();");
  init_buffers();
  sleep(1);

  log("main: starting threads.");
  for (i = 0; i < NUM_THREADS; i++) {
    printf("%d ", i);
    if (pthread_create(&thread_id[i], NULL, thread, NULL) < 0)
      log("thread: Failed thread creation.", i);
  }
  printf("\n");

  sleep(5);

  log("main: waiting...");
  for (i = 0; i < NUM_THREADS; i++)
    pthread_join(thread_id[i], NULL);
  circle_shutdown = 1;
  log("main: done.");

  sleep(5);

  log("main: exit_buffers()");
  exit_buffers();
  return 0;
}
