/************************************************************************
 * buffer.c - Advanced Buffer System				v1.8	*
 *									*
 * Designed for CircleMUD 3.0				May 30, 1998	*
 ************************************************************************/

#define __BUFFER_C__

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "buffer.h"
#include "utils.h"
#include "interpreter.h"

/* ------------------------------------------------------------------------ */

/*
 * ... BUF_NUKE ...
 * Set	: Use a memset() to clear each buffer after use.
 * Unset: Do a *buffer = '\0'
 * ... BUF_CHECK ...
 * Set	: Report how much of each buffer the functions used.
 * Unset: Don't report.
 * ... BUF_OVERBOOT ...
 * Set	: Reboot immediately on overflow. (recommended)
 * Unset: Try to keep running after an overflow.
 * ... BUF_DETAIL
 * Set  : Log details of what is going on.
 * Unset: Only report errors.
 * ... BUF_VERBOSE ...
 * Set	: Output extremely detailed information.
 * Unset: Log only error messages.
 */
#define BUF_NUKE	(1 << 0)
#define BUF_CHECK	(1 << 1)
#define BUF_OVERBOOT	(1 << 2)
#define BUF_DETAIL	(1 << 3)	/* VERBOSE overrides DETAIL. */
#define BUF_VERBOSE	(1 << 4)
/*
 * Example: ush_int buffer_opt = BUF_OVERBOOT | BUF_VERBOSE | BUF_CHECK;
 */
ush_int buffer_opt = BUF_OVERBOOT;
//ush_int buffer_opt = 0;

/*
 * 1 = Check every pointer in every buffer allocated on every new allocation.
 * 0 = Recommended, unless you are tracking memory corruption at the time.
 *
 * NOTE: This only works under Linux.  Do not try to use with anything else!
 */
#define BUFFER_EXTREME_PARANOIA	0

/*
 * 1 = Enable multithreaded support.  You _must_ have POSIX threads for this!
 * 0 = Use the standard heartbeat() method of freeing the buffers.
 */
#define BUFFER_THREADED	0

/*
 * MAGIC_NUMBER - Any arbitrary character unlikely to show up in a string.
 *	In this case, we use a control character. (0x6)
 * BUFFER_LIFE - How long can a buffer be unused before it is free()'d?
 * LEASE_LIFE - The amount of time a buffer can be used before it is
 *	considered forgotten and released automatically.
 */
#if !defined(MAGIC_NUMBER)
#define MAGIC_NUMBER	(0x6)	/* Also in bpl13. */
#endif
#define BUFFER_LIFE	(180 RL_SEC)	/* 5 minutes */
#define LEASE_LIFE	(60 RL_SEC)	/* 1 minute */

/*
 * MEMORY ARRAY OPTIONS
 *
 * If you change the maximum or minimum allocations, you'll probably also want
 * to change the hash number.  See the 'bufhash.c' program for determining a
 * decent one.  Think prime numbers.
 *
 * MAX_ALLOC - The largest buffer magnitude allowed without griping about it.
 * MIN_ALLOC - The lowest buffer magnitude ever handed out.  If 20 bytes are
 *	requested, we will allocate this size buffer anyway.
 * BUFFER_HASH - The direct size->array hash mapping number. This is _not_ an
	arbitrary number.
 * CACHE_SIZE - How many data->buffer mappings to maintain.
 */
#define MAX_ALLOC	(16)	/* 2**16 = 65536 bytes	*/
#define MIN_ALLOC	(6)	/* 2**6  = 64 bytes	*/
#define BUFFER_HASH	(11)	/* Not arbitrary! */
#define CACHE_SIZE	(3 + BUFFER_MEMORY * 22)	/* Cache is 10 in that case. */

/*
 * End of configurables section.
 * ----------------------------------------------------------------------------
 */

/*
 * Don't change these, they are derived from other settings above.
 *
 * B_FREELIFE - The buffer life in pulses.
 * B_GIVELIFE - The lease life in pulses.
 * BUFFER_MINSIZE - The smallest buffer actually allocated, in bytes.
 */
#define BUFFER_MINSIZE	(1 << MIN_ALLOC)	/* Minimum size, in bytes. */
#define BUFFER_MAXSIZE	(1 << MAX_ALLOC)	/* Maximum size, in bytes. */
#if BUFFER_THREADED
#define B_FREELIFE	(BUFFER_LIFE / PASSES_PER_SEC)
#define B_GIVELIFE	(BUFFER_LIFE / PASSES_PER_SEC)
#else
#define B_FREELIFE	(BUFFER_LIFE / PULSE_BUFFER)
#define B_GIVELIFE	(LEASE_LIFE / PULSE_BUFFER)	
#endif

#if BUFFER_THREADED
/*
 * Assorted lock types.
 */
#define LOCK_NONE               0
#define LOCK_ACQUIRE            1
#define LOCK_WILL_CLEAR         2
#define LOCK_WILL_FREE          4
#define LOCK_WILL_REMOVE        8
#endif

/*
 * NOTE: There are reasons for not making a 'mem_data' structure.  We would
 * have to modify all of the iterator functions to work with both structures,
 * possibly though void pointers, which is ugly.  Currently, the only change
 * required was to pick get_{memory,buffer}_head() based on what type of
 * variable is requested.  This does waste 4 bytes for every malloc() that we
 * could save with a separate mem_data, but the added complexity is not worth
 * it in my opinion.  Debugging isn't necessarily cheap. Beware of the union.
 */
struct buf_mem_data {
  struct buf_data **buf;
  struct buf_data **mem;
};

struct buf_data {
  byte magic;		/* Have we been trashed?		*/
#if BUFFER_THREADED
  byte locked;		/* Don't touch this item, we're locked.	*/
#endif
  byte type;		/* What type of buffer are we?		*/
  ush_int line;         /* What source code line is using this. */
  size_t req_size;	/* How much did the function request?	*/
  union {
    sh_long life;	/* An idle counter to free unused ones.	(B) */
    const char *var;	/* Name of variable allocated to.	(M) */
  } var_life;
  size_t size;          /* How large is this buffer?		*/
  const char *who;      /* Name of the function using this.     */
  char *data;           /* The buffer passed back to functions. */
  struct buf_data *next;	/* Next structure.		*/
};

/*
 * The union support.
 */
#define life	var_life.life
#define var	var_life.var

/* ------------------------------------------------------------------------ */

/*
 * Global variables.
 *  buffers - Head of the main buffer allocation linked list.
 *  buffer_cache - A list of the most recently acquired buffers.
 *  buffer_cache_stat - Holds how many buffer cache hits/misses we've had.
 *  buffer_array - The minimum and maximum array ranges for 'buffers'.
 *  buf_thread - Handle of the buffer thread.
 *  buflistmutex - Make sure the threads don't stomp on each other.
 */
static struct buf_mem_data buffers;
static struct buf_data *buffer_cache[CACHE_SIZE];
int buffer_cache_stat[2] = { 0, 0 };	/* Used in act.wizard.c also. */
static int buffer_array[2] = { 0, 0 };
#define BUF_MAX	0	/* buffer_array */
#define BUF_MIN	1

/* External functions and variables. */
extern void send_to_char(char *, struct char_data *);
extern void send_to_all(char *);
extern int circle_shutdown;
extern int circle_reboot;

/* Private functions. */
static struct buf_data *new_buffer(size_t size, int type);
static struct buf_data *malloc_buffer(size_t size, int type);
static struct buf_data *in_cache(const char *data);
static void free_buffer(struct buf_data *f);
static void clear_buffer(struct buf_data *clearme);
static void remove_buffer(struct buf_data *removeme);
static int remove_from_cache(struct buf_data *removeme);
static void add_to_cache(struct buf_data *addme);
static int get_used(struct buf_data *buf);
static int get_magnitude(ush_long number);
#if BUFFER_THREADED
static void *buffer_thread(void *);
static void bufferlist_lock(const char *func, ush_int line);
static void bufferlist_unlock(const char *func, ush_int line);
static void buffer_lock(struct buf_data *buf, int type, const char *func, ush_int line);
static void buffer_unlock(struct buf_data *buf, const char *func, ush_int line);
#endif


#if BUFFER_THREADED
/*
 * Wrapper macros.
 */
#include <pthread.h>
static pthread_mutex_t buflistmutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t buf_thread = 0;
#define lock_buffers()		(bufferlist_lock(__FUNCTION__, __LINE__))
#define unlock_buffers()	(bufferlist_unlock(__FUNCTION__, __LINE__))
#define LOCK(buf, type)		(buffer_lock((buf), (type), __FUNCTION__, __LINE__))
#define UNLOCK(buf)		(buffer_unlock((buf), __FUNCTION__, __LINE__))
#define LOCKED(buf)		((buf)->locked)
#else
#define lock_buffers()		/* Not necessary. */
#define unlock_buffers()	/* Not necessary. */
#define LOCK(buf, type)		/* Not necessary. */
#define UNLOCK(buf)		/* Not necessary. */
#define LOCKED(buf)		(FALSE)
#endif

/*
 * Useful macros.
 */
#define buffer_hash(x)		((x) % BUFFER_HASH)
#define USED(x)			((x)->who)
#define get_buffer_head()	(buffers.buf)
#define set_buffer_head(x)	do { buffers.buf = (x); } while(0)
#define get_memory_head()	(buffers.mem)
#define set_memory_head(x)	do { buffers.mem = (x); } while(0)

/* ------------------------------------------------------------------------ */

/*
 * Private: count_bits(number to count)
 * Find out how many bits are in a number.  Used to see if it can be
 * rounded into a power of 2.
 */
static int count_bits(size_t y)
{
  int i, count = 0;

  for (i = 0; i < sizeof(size_t) * 8; i++)
    count += (y & (1 << i)) >> i;	/* Avoiding a jump to get 0 or 1. */
  return count;
}

/*
 * Private: round_to_pow2(number to round)
 * Converts a number to the next highest power of two.
 * 0000011000111010 (1594)
 *        to
 * 0000100000000000 (2048)
 */
#define bit(a,b)	((a) & (1 << (b)))
static void round_to_pow2(size_t *y)
{
  int i;

  if (count_bits(*y) <= 1)
    return;

  if (buffer_opt & BUF_VERBOSE)
    fprintf(stderr, "BUF: round_to_pow2: %d -> ", *y);

  i = sizeof(size_t) * 8 - 1;

  /* Check for MSB, otherwise scan for first set bit. */
  if (bit(*y, i) == 0) {
    do {
      if (bit(*y, i - 1) != 0)
        break;
    } while (--i > 0);

    /* No bits were set, exit. */
    if (i <= 0)
      return;

    /* Set the bit above the previous highest. */
    *y |= (1 << i);
  }

  /* Clear out the remaining bits. */
  while (i--)
    *y &= ~(1 << i);

  if (buffer_opt & BUF_VERBOSE) {
    fprintf(stderr, "%d\n", *y);
    fflush(stderr);
  }
} 

/*
 * Private: buffer_reboot(none)
 * Determine whether to reboot or continue after corruption.
 */
static void buffer_reboot(void)
{
   /* Recursing, just die now... */
  if (buffer_opt & BUF_OVERBOOT && circle_shutdown == 1 && circle_reboot == 1) {
    log("BUF: SYSERR: Clean shutdown not possible, dying...");
    exit(1);
  }
  
  init_buffers();
  
  if (buffer_opt & BUF_OVERBOOT) {
    send_to_all("Emergency reboot.. come back in a minute or two.\r\n");
    log("BUF: SYSERR: Emergency reboot, buffer corrupted!");
    circle_shutdown = circle_reboot = 1;
    return;
  }
  log("BUF: SYSERR: Buffer list cleared, crash imminent!");
}

#if BUFFER_EXTREME_PARANOIA == 0 || !defined(__linux__)	/* Doesn't support. */
#define valid_pointer(ptr)	(TRUE)

#else	/* only works on Linux due to pointer addresses. */
#define valid_pointer(ptr)	(((unsigned long)(ptr) & 0x48000000))

/*
 * Private: buffer_sanity_check(nothing)
 * Checks the entire buffer/memory list for overruns.
 */
static void buffer_sanity_check(struct buf_data **check)
{
  int magnitude, die = FALSE;

  if (check == NULL) {
    buffer_sanity_check(get_buffer_head());
    buffer_sanity_check(get_memory_head());
    return;
  }
  for (magnitude = 0; magnitude <= buffer_array[BUF_MAX]; magnitude++) {
    struct buf_data *chk;
    if (check[magnitude] && !valid_pointer(check[magnitude]) && (die = TRUE))
      log("BUF: SYSERR: %s/%d head corrupted.", check == get_buffer_head() ? "buffer" : "memory", magnitude);
    for (chk = check[magnitude]; chk; chk = chk->next) {
      if (!valid_pointer(chk->data) && (die = TRUE))
	log("BUF: SYSERR: %p->%p (data) corrupted.", chk, chk->data);
      if (chk->next && !valid_pointer(chk->next) && (die = TRUE))
	log("BUF: SYSERR: %p->%p (next) corrupted.", chk, chk->next);
      if (chk->type != BT_MALLOC && valid_pointer(chk->life) && (die = TRUE))
	log("BUF: SYSERR: %p->%p [%ld] (life) is valid pointer on non-malloc memory.", chk, chk->var, chk->life);
      if (chk->type == BT_MALLOC && !valid_pointer(chk->var) && (die = TRUE))
	log("BUF: SYSERR: %p->%p [%ld] (var) not a valid pointer for malloc memory.", chk, chk->var, chk->life);
      if (chk->magic != MAGIC_NUMBER && (die = TRUE))
	log("BUF: SYSERR: %p->%d (magic) corrupted.", chk, chk->magic);
      if (chk->data[chk->req_size] != MAGIC_NUMBER && (die = TRUE))
	log("BUF: SYSERR: %p (%s:%d) overflowed requested size (%d).", chk, chk->who, chk->line, chk->req_size);
      if (chk->data[chk->size] != MAGIC_NUMBER && (die = TRUE))
	log("BUF: SYSERR: %p (%s:%d) overflowed real size (%d).", chk, chk->who, chk->line, chk->size);
    }
  }
  if (die)
    buffer_reboot();
}
#endif

/*
 * Private: magic_check(buffer to check)
 * Makes sure a buffer hasn't been overwritten by something else.
 */
static void magic_check(struct buf_data *buf)
{
  if (buf->magic != MAGIC_NUMBER) {
    char trashed[16];
    strncpy(trashed, (char *)buf, 15);
    trashed[15] = '\0';
    log("BUF: SYSERR: Buffer %p was trashed! (%s)", buf, trashed);
    buffer_reboot();
  }
}

/*
 * Private: get_magnitude(number)
 * Simple function to avoid math library, and enforce maximum allocations.
 * If it is not enforced here, we will overrun the bounds of the array.
 */
static int get_magnitude(ush_long y)
{
  ush_int number = buffer_hash(y);

  if (number > buffer_array[BUF_MAX]) {
    log("BUF: SYSERR: Hash result %d out of range 0-%d.", number, buffer_array[BUF_MAX]);
    number = buffer_array[BUF_MAX];
  }

  if (buffer_opt & BUF_VERBOSE)
    log("BUF: get_magnitude: %ld bytes -> %d", y, number);

  return number;
}

/*
 * Private: add_to_cache(structure to add)
 * Does what it says.
 */
static void add_to_cache(struct buf_data *addme)
{
  /*
   * Move everybody right.
   */
  memmove(buffer_cache + 1, buffer_cache, sizeof(struct buf_data *) * (CACHE_SIZE - 1));

  /*
   * Add the structure to the front.
   */
  buffer_cache[0] = addme;
}

/*
 * Private: remove_from_cache(structure to remove)
 * Does what it says.
 */
static int remove_from_cache(struct buf_data *removeme)
{
  int i, ret = FALSE;

  for (i = 0; i < CACHE_SIZE; i++)
    if (buffer_cache[i] == removeme) {
      buffer_cache[i] = NULL;
      ret = TRUE;
    }
  return ret;
}

/*
 * Private: in_cache(data to search for)
 * Used internally since the cache expanded complexity.
 */
static struct buf_data *in_cache(const char *data)
{
  int i;
  for (i = 0; i < CACHE_SIZE; i++)
    if (buffer_cache[i] && buffer_cache[i]->data == data) {
      buffer_cache_stat[BUFFER_CACHE_HITS]++;
      if (buffer_opt & BUF_VERBOSE)
	log("BUF: in_cache: Cache hit %p->%p from %p.", buffer_cache[i], buffer_cache[i]->data, data);
#if BUFFER_THREADED
      if (LOCKED(buffer_cache[i]))
	log("BUF: in_cache: Giving out locked buffer reference %p.", buffer_cache[i]);
#endif
      return buffer_cache[i];
    }
  if (buffer_opt & BUF_VERBOSE)
    log("BUF: in_cache: Cache miss.");

  buffer_cache_stat[BUFFER_CACHE_MISSES]++;
  return NULL;
}

/*
 * Public: exit_buffers()
 * Called to clear out the buffer structures and log any BT_MALLOC's left.
 */
void exit_buffers(void)
{
  struct buf_data **head;
  ush_int magnitude;

  log("BUF: Shutting down.");
  
 /* buffer_opt = BUF_DETAIL;*/

#if BUFFER_THREADED
  pthread_join(buf_thread, NULL);
#endif

  log("BUF: Clearing buffer memory.");
  head = get_buffer_head();
  for (magnitude = 0; magnitude <= buffer_array[BUF_MAX]; magnitude++) {
    struct buf_data *b, *bn;
    for (b = head[magnitude]; b; b = bn) {
      bn = b->next;
      LOCK(b, LOCK_WILL_CLEAR);
      clear_buffer(b);
      LOCK(b, LOCK_WILL_REMOVE);
      remove_buffer(b);
      LOCK(b, LOCK_WILL_FREE);
      free_buffer(b);
    }
  }
  log("BUF: Clearing malloc()'d memory.");
  head = get_memory_head();
  for (magnitude = 0; magnitude <= buffer_array[BUF_MAX]; magnitude++)
    while (head[magnitude]) {
      LOCK(head[magnitude], LOCK_WILL_CLEAR);
      clear_buffer(head[magnitude]);
    }
  log("BUF: Done.");
}

/*
 * Private: find_hash(none)
 * Determine the size of the buffer array based on the hash size and the
 * allocation limits.
 */
static int find_hash(void)
{
  int maxhash = 0, result, i;
  
  if (buffer_opt & (BUF_DETAIL | BUF_VERBOSE))
    log("BUF: Calculating array size...");
    
  for (i = MIN_ALLOC; i <= MAX_ALLOC; i++) {
    result = buffer_hash(1 << i);
    if (result > maxhash)
      maxhash = result;
    if (buffer_opt & (BUF_DETAIL | BUF_VERBOSE))
      log("BUF: %7d -> %2d", 1 << i, result);
  }
  
  if (buffer_opt & (BUF_DETAIL | BUF_VERBOSE)) {
    log("BUF: ...done.");
    log("BUF: Array range is 0-%d.", maxhash);
  }
  
  buffer_array[BUF_MAX] = maxhash;
  return maxhash;
}

/*
 * Public: init_buffers(none)
 * This is called from main() to get everything started.
 */
void init_buffers(void)
{
#if BUFFER_THREADED
  int error = 0;
#endif
  static int yesyesiknow = FALSE;

  lock_buffers();
  
    /* Tag line. */
  if (!yesyesiknow) {
    log("BUF: Enhanced Buffer Allocation System v1.9");
    if (LEASE_LIFE * 2 < PULSE_BUFFER)
      log("BUF: SYSERR: Buffer lease life is too short.");
    if (BUFFER_LIFE * 2 < PULSE_BUFFER)
      log("BUF: SYSERR: Buffer unused life is too short.");
  }

  find_hash();

#if BUFFER_THREADED
  /*
   * If a buffer thread already exists, kill it.
   */
  if (buf_thread != 0)
    pthread_cancel(buf_thread);

  /*
   * Initialize pthread information and start the thread.
   */
  if ((error = pthread_create(&buf_thread, NULL, buffer_thread, NULL)) < 0) {
    perror("pthread_create");
    printf("%d\n", error);
    exit(1);
  }
#endif

  /*
   * Allocate room for the array of pointers.  We can't use the CREATE
   * macro here because that uses the array we're trying to make!
   */
  set_buffer_head(calloc(buffer_array[BUF_MAX] + 1, sizeof(struct buf_data *)));
  set_memory_head(calloc(buffer_array[BUF_MAX] + 1, sizeof(struct buf_data *)));

  if (!yesyesiknow) {
    log("BUF: Buffer cache: %d elements, %d bytes.", CACHE_SIZE, sizeof(struct buf_data *) * CACHE_SIZE);
    log("BUF: Allocations: %d-%d bytes (%d overhead)", 1 << MIN_ALLOC, 1 << MAX_ALLOC, sizeof(struct buf_data));
  }
  /*
   * Put any persistant buffers here.
   * Ex: new_buffer(8192, BT_PERSIST);
   */

  unlock_buffers();
  yesyesiknow = TRUE;
}

/*
 * Private: decrement_all_buffers(none)
 * Reduce the life on all buffers by 1.
 */
static void decrement_all_buffers(void)
{
  int magnitude;
  struct buf_data *clear, **head = get_buffer_head();

  for (magnitude = 0; magnitude <= buffer_array[BUF_MAX]; magnitude++)
    for (clear = head[magnitude]; clear; clear = clear->next) {
      if (clear->type == BT_MALLOC) {
	log("BUF: SYSERR: Ack, memory (%p, %s:%d, '%s') in buffer list!", clear, clear->who, clear->line, clear->var);
	continue;
      }
      if (clear->life < 0)
	log("BUF: SYSERR: %p from %s:%d has %ld life.", clear, clear->who, clear->line, clear->life);
      else if (clear->life != 0)	/* We don't want to go negative. */
	clear->life--;
    }
}

/*
 * Private: release_old_buffers()
 * Check for any used buffers that have no remaining life.
 */
static void release_old_buffers(void)
{
  int magnitude;
  struct buf_data *relbuf, **head = get_buffer_head();

  for (magnitude = 0; magnitude <= buffer_array[BUF_MAX]; magnitude++)
    for (relbuf = head[magnitude]; relbuf; relbuf = relbuf->next) {
      if (relbuf->type == BT_MALLOC) {
	log("BUF: SYSERR: Ack, memory (%p, %s:%d, '%s') in buffer list!", relbuf, relbuf->who, relbuf->line, relbuf->var);
	continue;
      }      
      if (!USED(relbuf))	/* Can't release, no one is using this. */
	continue;
      if (LOCKED(relbuf))	/* Someone has this locked already. */
	continue;
      if (relbuf->life > 0)	/* We only want expired buffers. */
	continue;
      LOCK(relbuf, LOCK_WILL_CLEAR);
      log("BUF: %s:%d forgot to release %d bytes in %p.", relbuf->who ? relbuf->who : "UNKNOWN", relbuf->line, relbuf->size, relbuf);
      clear_buffer(relbuf);
      UNLOCK(relbuf);
    }
}

/*
 * Private: free_old_buffers(void)
 * Get rid of any old unused buffers.
 */
static void free_old_buffers(void)
{
  int magnitude;
  struct buf_data *freebuf, *next_free, **head = get_buffer_head();

  lock_buffers();

  for (magnitude = 0; magnitude <= buffer_array[BUF_MAX]; magnitude++)
    for (freebuf = head[magnitude]; freebuf; freebuf = next_free) {
      next_free = freebuf->next;
      if (freebuf->type != BT_STACK)	/* We don't free persistent ones. */
	continue;
      if (USED(freebuf))	/* Needs to be cleared first if used. */
	continue;      
      if (LOCKED(freebuf))	/* Already locked, skip it. */
	continue;
      if (freebuf->life > 0)	/* Hasn't expired yet. */
	continue;

      LOCK(freebuf, LOCK_WILL_REMOVE);
      remove_buffer(freebuf);
      LOCK(freebuf, LOCK_WILL_FREE);
      free_buffer(freebuf);
    }

  unlock_buffers();
}

/*
 * Public: release_all_buffers()
 * Forcibly release all buffers currently allocated.  This is useful to
 * reclaim any forgotten buffers.
 * See structs.h for PULSE_BUFFER to change the release rate.
 */
void release_all_buffers(void)
{
#if BUFFER_THREADED
  /* We don't do anything, there's already a thread doing this. */
#else
  decrement_all_buffers();
  release_old_buffers();
  free_old_buffers();
#endif
}

/*
 * Private: remove_buffer(buffer to remove)
 * Removes a buffer from the list without freeing it.
 */
static void remove_buffer(struct buf_data *removeme)
{
  struct buf_data **head, *traverse, *prev = NULL;
  int magnitude;

  if (!removeme) {
    log("BUF: SYSERR: NULL buf_data given to remove_buffer.");
    return;
  }

#if BUFFER_THREADED
  if (LOCKED(removeme) != LOCK_WILL_REMOVE)
    log("BUF: SYSERR: remove_buffer: Lock bit not properly set on %p!", removeme);
#endif

  head = (removeme->type == BT_MALLOC ? get_memory_head() : get_buffer_head());
  magnitude = get_magnitude(removeme->size);

  for (traverse = head[magnitude]; traverse; traverse = traverse->next) {
    if (traverse == removeme) {
      if (traverse == head[magnitude])
	head[magnitude] = traverse->next;
      else if (prev)
	prev->next = traverse->next;
      else
	log("BUF: SYSERR: remove_buffer: Don't know what to do with %p.", removeme);
#if BUFFER_THREADED
      if (LOCKED(removeme) != LOCK_WILL_REMOVE)
	log("BUF: SYSERR: remove_buffer: Lock bit removed from %p during operation!", removeme);
#endif
      return;
    }
    prev = traverse;
  }
  log("BUF: SYSERR: remove_buffer: Couldn't find the buffer %p from %s:%d.", removeme, removeme->who, removeme->line);
}

/*
 * Private: clear_buffer(buffer to clear)
 * This is used to declare an allocated buffer unused.
 */
static void clear_buffer(struct buf_data *clearme)
{
  if (clearme == NULL) {
    log("BUF: SYSERR: clear_buffer: NULL argument.");
    return;
  }

#if BUFFER_THREADED
  if (LOCKED(clearme) != LOCK_WILL_CLEAR)
    log("BUF: SYSERR: clear_buffer: Lock not properly set on %p!", clearme);
#endif

  magic_check(clearme);

  /*
   * If the magic number we set is not there then we have a suspected
   * buffer overflow.
   */
  if (clearme->data[clearme->req_size] != MAGIC_NUMBER) {
    log("BUF: SYSERR: Overflow in %p (%s) from %s:%d.",
		clearme, clearme->type == BT_MALLOC ? clearme->var : "a buffer", clearme->who ? clearme->who : "UNKNOWN", clearme->line);
    log("BUF: SYSERR: ... ticketed for doing %d in a %d (%d) zone.",
		strlen(clearme->data) + 1, clearme->req_size, clearme->size);
    if (clearme->data[clearme->size] == MAGIC_NUMBER) {
      log("BUF: SYSERR: ... overflow did not compromise memory.");
      clearme->data[clearme->req_size - 1] = '\0';
      clearme->data[clearme->req_size] = MAGIC_NUMBER;
    } else
      buffer_reboot();
  } else if (clearme->type == BT_MALLOC) {
    lock_buffers();
    LOCK(clearme, LOCK_WILL_REMOVE);
    remove_buffer(clearme);
    LOCK(clearme, LOCK_WILL_FREE);
    free_buffer(clearme);
    unlock_buffers();
  } else {
    if (buffer_opt & BUF_CHECK) {
      /*
       * If nothing in clearme->data return 0, else if paranoid_buffer, return
       * the result of get_used(), otherwise just strlen() the buffer.
       */
      log("BUF: %s:%d used %d/%d bytes in %p.",
		clearme->who, clearme->line, (clearme->data && *clearme->data ?
		((buffer_opt & BUF_NUKE) ? get_used(clearme) :
		strlen(clearme->data)) : 0), clearme->req_size, clearme);
    }
    if (buffer_opt & BUF_NUKE)
      memset(clearme->data, '\0', clearme->size);
    else
      *clearme->data = '\0';
    clearme->who = NULL;
    clearme->line = 0;
    clearme->req_size = clearme->size;	/* For exit_buffers() check. */
    clearme->life = B_FREELIFE;
#if BUFFER_THREADED
    if (LOCKED(clearme) != LOCK_WILL_CLEAR)
      log("BUF: SYSERR: clear_buffer: Someone cleared lock bit on %p.", clearme);
#endif
  }
}

/*
 * Private: find_this_buffer(buffer data pointer)
 * Used by detach_buffer() to locate the correct buffer.
 */
static struct buf_data *find_this_buffer(char *given, int type, int *searches)
{
  struct buf_data *clear, **b_head;
  int magnitude, scanned;
  
  if (searches)
    *searches = -1;

  /*
   * We cache the last allocated buffer to speed up cases where we
   * allocate a buffer and then free it shortly afterwards.
   */
  if ((clear = in_cache(given)) != NULL)    
    return clear;  

  b_head = (type == BT_MALLOC ? get_memory_head() : get_buffer_head());

  for (magnitude = 0, scanned = 0; magnitude <= buffer_array[BUF_MAX]; magnitude++)
    for (clear = b_head[magnitude]; clear; clear = clear->next, scanned++)
      if ((clear) && (clear->data)) 
        if (clear->data == given)
          goto got_it;	/* Cleaner than attempting two 'break;' commands. */

got_it:

  if (buffer_opt & BUF_VERBOSE)
    log("BUF: find_this_buffer: Scanning for %p, found %p->%p (%d scans).\n", given, clear, clear ? clear->data : NULL, scanned);

  if (searches)
    *searches = scanned;
  return clear;
}

/*
 * Public: as release_buffer(buffer data pointer)
 * Used throughout the code to finish their use of the buffer.
 */
struct buf_data *detach_buffer(char *data, byte type, const char *func, const int line_n)
{
  struct buf_data *clear;
  int scanned;

  if (data == NULL || func == NULL) {
    log("BUF: SYSERR: detach_buffer: Invalid information passed from %s:%d.", func, line_n);
    return FALSE;
  }

  clear = find_this_buffer(data, type, &scanned);

  if (clear == NULL) {
    log("BUF: SYSERR: detach_buffer: No buffer->data %p found for %s:%d.", data, func, line_n);
#if BUFFER_THREADED
  } else if (LOCKED(clear)) {
    if (LOCKED(clear) == LOCK_WILL_FREE)
      log("BUF: detach_buffer: Buffer %p, requested by %s:%d, already slated to be freed.", clear, func, line_n);
    else if (LOCKED(clear) == LOCK_WILL_CLEAR)
      log("BUF: detach_buffer: Buffer %p, requested by %s:%d, already being detached.", clear, func, line_n);
    else if (LOCKED(clear) == LOCK_WILL_REMOVE)
      log("BUF: detach_buffer: Buffer %p, requested by %s:%d, already being removed from list.", clear, func, line_n);
    else
      log("BUF: detach_buffer: Buffer %p, requested by %s:%d, already locked.", clear, func, line_n);
#endif
  } else if (clear->who == NULL)
    log("BUF: SYSERR: detach_buffer: Buffer %p, requested by %s:%d, already released. Locked: %d", clear, func, line_n, LOCKED(clear));
  else {
    LOCK(clear, LOCK_WILL_CLEAR);
    if (buffer_opt & (BUF_VERBOSE | BUF_DETAIL))
      log("BUF: %s:%d released %d bytes in '%s' (%p, %d scans) from %s:%d.", func, line_n,
		clear->size, clear->type == BT_MALLOC ? clear->var : "a buffer", clear, scanned, clear->who ? clear->who : "UNKNOWN", clear->line);
    clear_buffer(clear);
    UNLOCK(clear);
    return clear;
  }
  return NULL;
}
  
/*
 * Private: free_buffer(buffer)
 * Internal function for getting rid of buffers.
 */
static void free_buffer(struct buf_data *f)
{
  if (f == NULL) {
    log("BUF: SYSERR: free_buffer: NULL pointer.");
    return;
  } else if (f->type == BT_PERSIST)
    log("BUF: SYSERR: free_buffer: Freeing %d byte persistant buffer %p.", f->size, f);
  else if (f->type == BT_MALLOC) {
    if (buffer_opt & (BUF_VERBOSE | BUF_DETAIL))
      log("BUF: free_buffer: Freeing %d bytes in '%s' (%p) from %s:%d.", f->size, f->var, f, f->who, f->line);
  }/* else
    log("BUF: free_buffer: Freeing %d bytes in expired buffer %p.", f->size, f);*/

#if BUFFER_THREADED
  if (LOCKED(f) != LOCK_WILL_FREE)
    log("BUF: SYSERR: free_buffer: Lock not properly set on %p!", f);
#endif

  /*
   * Would be very bad to reuse this buffer after it is free()'d.
   */
  remove_from_cache(f);

  if (f->data)
    really_free(f->data);
  else
    log("BUF: SYSERR: free_buffer: Hey, no data in %p?", f);
  really_free(f);
}

/*
 * Private: new_buffer(size of buffer, type flag)
 * Finds where it should place the new buffer it's trying to malloc.
 */
static struct buf_data *new_buffer(size_t size, int type)
{
  struct buf_data **buflist, *potential;
  int magnitude;

  if (size == 0) {
    log("BUF: SYSERR: new_buffer: 0 byte buffer requested.");
    return NULL;
  }

  /*
   * This caused a severe bug when it was in malloc_buffer().  Basically the
   * buffer would be placed according to the non-rounded size and then later
   * rounded.  This would cause a lookup to fail later when the hash returns
   * a different value based on the new size.
   */
  if (type != BT_MALLOC)
    round_to_pow2(&size);

  /*
   * We separate the malloc and buffer lists for efficiency.
   */
  buflist = (type == BT_MALLOC ? get_memory_head() : get_buffer_head());
  magnitude = get_magnitude(size);

  /*
   * The new method of prepending and rounding is extremely fast and clean.
   */
   potential = NULL;
   potential = malloc_buffer(size, type);
   if (potential) {
    lock_buffers();
    potential->next = buflist[magnitude];
    buflist[magnitude] = potential;
    unlock_buffers();
   } 
   return potential;
    
}

/*
 * Private: malloc_buffer(size of buffer, type flag)
 * Creates a new buffer for use.
 */
static struct buf_data *malloc_buffer(size_t size, int type)
{
  struct buf_data *new_buf;

  if (!(new_buf = (struct buf_data *)malloc(sizeof(struct buf_data)))) {
    log("BUF: SYSERR: malloc_buffer: Failed %d byte 'buf_data' malloc().", sizeof(struct buf_data));
    /*perror("malloc_buffer()");*/
    return NULL;
  }

  memset(new_buf, 0, sizeof(struct buf_data));

  LOCK(new_buf, LOCK_ACQUIRE);

  if ((new_buf->data = (char *)malloc(size + 1)) == NULL){
    log("BUF: SYSERR: malloc_buffer: Failed %d byte 'data' malloc().", size);
    really_free(new_buf);
    /*perror("malloc_buffer()");*/
    return NULL;
  }
  
  new_buf->magic = MAGIC_NUMBER;
  new_buf->who = NULL;
  new_buf->line = 0;
  new_buf->type = type;
  new_buf->next = NULL;
  new_buf->size = size;
  new_buf->req_size = size;	/* do_buffer fools overflow code otherwise. */
  if (type != BT_MALLOC)
    new_buf->life = B_FREELIFE;
  else
    new_buf->var = NULL;

  if (buffer_opt & (BUF_VERBOSE | BUF_DETAIL))
    log("BUF: malloc_buffer: Allocated %d byte buffer %p, %d byte overhead.",
		new_buf->size, new_buf, sizeof(struct buf_data));

  /* We emulate calloc(), remember? */
  if (type == BT_MALLOC || buffer_opt & BUF_NUKE)
    memset(new_buf->data, '\0', new_buf->size);
  else
    *new_buf->data = '\0';

  /* Implant number here. */
  new_buf->data[new_buf->size] = MAGIC_NUMBER;

  return new_buf;
}

/*
 * We no longer search for buffers outside the specified power of 2 since
 * the next highest array index may actually be smaller.
 */
struct buf_data *find_free_buffer(size_t size)
{
  struct buf_data *search, **head = get_buffer_head();
  int magnitude = get_magnitude(size), scans = 0;

  lock_buffers();

  for (search = head[magnitude]; search; search = search->next) {
    scans++;
    if (search->type == BT_MALLOC) {
      log("BUF: SYSERR: Ack, memory (%p, %s:%d, '%s') in buffer list!", search, search->who, search->line, search->var);
      continue;
    }
    if (LOCKED(search))
      continue;
    if (USED(search))
      continue;
    if (search->size < size)
      continue;
    LOCK(search, LOCK_ACQUIRE);
    break;
  }
  if (buffer_opt & (BUF_VERBOSE | BUF_DETAIL))
    log("BUF: find_free_buffer: %d scans for %d bytes, found %p.", scans, size, search);
  unlock_buffers();
  return search;
}

/*
 * Public: as get_buffer(size of buffer)
 * Requests a buffer from the free pool.  If a buffer of the desired size
 * is not available, one is created.
 */
char *acquire_buffer(size_t size, int type, const char *varname, const char *who, ush_int line)
{
  struct buf_data *give;

#if BUFFER_EXTREME_PARANOIA
  buffer_sanity_check(NULL);
#endif

  /* Could happen... */
  if (buffer_array[BUF_MAX] == 0) {
    log("BUF: SYSERR: System has not been initialized before request from %s:%d.", who, line);
    log("BUF: SYSERR: ... You need to call init_buffers() first.");
    abort();		/* Could call it here, but why hide the bug? */
  }

  if (size > BUFFER_MAXSIZE)
    log("BUF: %s:%d requested %d bytes for '%s'.", who, line, size, varname);

  if (size == 0) {
    log("BUF: SYSERR: %s:%d requested 0 bytes.", who, line);
    return NULL;
  } else if (type == BT_MALLOC)
    give = new_buffer(size, type);
  else if ((give = find_free_buffer(size)) == NULL) {
    /*
     * Minimum buffer size, since small ones have high overhead.
     */
    ush_long allocate_size;
    if ((allocate_size = size) < BUFFER_MINSIZE && type != BT_MALLOC)
      allocate_size = BUFFER_MINSIZE;

    if (buffer_opt & (BUF_VERBOSE | BUF_DETAIL))
      log("BUF: acquire_buffer: Making a new %ld byte buffer for %s:%d.", allocate_size, who, line);

    /*
     * If we don't have a valid pointer by now, we're out of memory so just
     * return NULL and hope things don't crash too soon.
     */
    if ((give = new_buffer(allocate_size, type)) == NULL)
      return NULL;
  }
  
  if (give == NULL)
   return NULL;

  magic_check(give);

#if BUFFER_THREADED
  if (LOCKED(give) != LOCK_ACQUIRE) {
    log("BUF: SYSERR: acquire_buffer: Someone stole my buffer.");
    abort();
  }
#endif

  give->who = who;
  give->line = line;
  give->req_size = size;

  if (type != BT_MALLOC)
    give->life = B_GIVELIFE;
  else
    give->var = varname;

  if (buffer_opt & (BUF_VERBOSE | BUF_DETAIL))
    log("BUF: %s:%d requested %d bytes for '%s', received %d in %p.", who, line, size, varname ? varname : "a buffer", give->size, give);

  /*
   * Plant a magic number to see if someone overruns the buffer. If the first
   * character of the buffer is not NUL then somehow our memory was
   * overwritten...most likely by someone doing a release_buffer() and
   * keeping a secondary pointer to the buffer.
   */
  give->data[give->req_size] = MAGIC_NUMBER;
  if (*give->data != '\0') {
    log("BUF: SYSERR: acquire_buffer: Buffer %p is not empty as it ought to be!", give);
    *give->data = '\0';
  }

#if 0
  /* This will only work if the buf_data is allocated by valloc(). */
  if (type == BT_MALLOC)
    if (mprotect(give, sizeof(struct buf_data), PROT_READ) < 0)
      abort();
#endif

  UNLOCK(give);

  add_to_cache(give);	/* Cache this entry. */
  
  if (give->type == BT_STACK && give->life <= 1) {
    log("BUF: SYSERR: acquire_buffer: give->life <= 1 (Invalid lease life?!)");
    give->life = 10;
  }

  return give->data;
}

/*
 * Public: as 'show buffers'
 * This is really only useful to see who has lingering buffers around
 * or if you are curious.  It can't be called in the middle of a
 * command run by a player so it'll usually show the same thing.
 * You can call this with a NULL parameter to have it logged at any
 * time though.
 */
/*
 * XXX: This code works but is ugly and misleading.
 */
void show_buffers(struct char_data *ch, int buffer_type, int display_type)
{
  struct buf_data *disp, **head = NULL;
  char *buf;
  long i, size;
  char *buf_type[] = { "Stack", "Persist", "Malloc" };

  if (display_type == 1) {
    head = get_buffer_head();
    log("BUF: --- Buffer list --- (Inaccurate if not perfect hash.)");
  } else if (display_type == 2) {
    head = get_memory_head();
    log("BUF: --- Memory list --- (Byte categories are inaccurate.)");
  }
  if (display_type == 1 || display_type == 2) {
    for (size = MIN_ALLOC; size <= MAX_ALLOC; size++) {
      long bytes;
      int magnitude = get_magnitude(1 << size);
      for (i = 0, bytes = 0, disp = head[magnitude]; disp; disp = disp->next, i++)
	bytes += disp->size;
      log("%5d bytes (%2d): %5ld items, %6ld bytes, %5ld overhead.", (1 << size), magnitude, i, bytes, sizeof(struct buf_data) * i);
    }
    return;
  }

  buf = get_buffer(MAX_STRING_LENGTH);
  head = get_buffer_head();

  for (size = 0; size <= buffer_array[BUF_MAX]; size++)
    for (i = 0, disp = head[size]; disp; disp = disp->next) {
      if (buffer_type != -1 && buffer_type != disp->type) /* -1 == all */
	continue;
      sprintf(buf, "%1ld #%2ld %5d bytes, Life: %5ld, Type: %7s, Allocated: %s/%d.%s",
		size, ++i, disp->size, disp->life,
		buf_type[(int)disp->type], disp->who ? disp->who : "unused",
		disp->line ? disp->line : 0, ch ? "\r\n" : "");
      if (ch)
	send_to_char(buf, ch);
      else
	log(buf);
    }

  release_buffer(buf);
}

/*
 * Tests the overflow code.  Do not use. :)
 */
ACMD(do_overflow)
{
  /*
   * Write 256 bytes into the 130 byte buffer. Tweak to suit.
   */
  int write = 256, bufsize = 130;
  char *buf;

  buf = get_buffer(bufsize);
  while (--write)
    buf[write] = write;
  release_buffer(buf);

  if (ch)
    send_to_char("Ok!\r\n", ch);
}

char *BUFFER_FORMAT =
"buffer (add | delete) size (persistant | temporary)\r\n"
"buffer verbose - toggle verbose mode.\r\n"
"buffer detailed - toggle detailed mode.\r\n"
"buffer paranoid - toggle between memset() or *buf = NUL.\r\n"
"buffer check - toggle buffer usage checking.\r\n"
"buffer overflow - toggle immediate reboot on overflow.\r\n";

ACMD(do_buffer)
{
  char *arg1, *arg2, *arg3;
  long size;
  int persistant = FALSE;

  /* This looks nifty. */
  half_chop(argument, (arg1 = get_buffer(MAX_INPUT_LENGTH)), argument);
  half_chop(argument, (arg2 = get_buffer(MAX_INPUT_LENGTH)), argument);
  half_chop(argument, (arg3 = get_buffer(MAX_INPUT_LENGTH)), argument);

  if (!*arg1)
    size = -1;
  else if (!*arg2 || !*arg3)
    size = -2;
  else if ((size = atoi(arg2)) == 0)
    size = -1;
  else if (is_abbrev(arg3, "persistant"))
    persistant = TRUE;
  else if (is_abbrev(arg3, "temporary"))
    persistant = FALSE;
  else
    persistant = FALSE;

  /* Don't need these now. */
  release_buffer(arg2);
  release_buffer(arg3);

  if (size == -1)	/* Oops, error. */
    send_to_char(BUFFER_FORMAT, ch);
  else if (size == -2) {	/* -2 means a toggle command. */
    if (is_abbrev(arg1, "verbose")) {
      buffer_opt ^= BUF_VERBOSE;
      send_to_char((buffer_opt & BUF_VERBOSE) ? "Verbose On.\r\n" : "Verbose Off.\r\n", ch);
    } else if (is_abbrev(arg1, "detailed")) {
      buffer_opt ^= BUF_DETAIL;
      send_to_char((buffer_opt & BUF_DETAIL) ? "Detailed On.\r\n" : "Detailed Off.\r\n", ch);
    } else if (is_abbrev(arg1, "paranoid")) {
      buffer_opt ^= BUF_NUKE;
      send_to_char((buffer_opt & BUF_NUKE) ? "Now paranoid.\r\n" : "No longer paranoid.\r\n", ch);
    } else if (is_abbrev(arg1, "check")) {
      buffer_opt ^= BUF_CHECK;
      send_to_char((buffer_opt & BUF_CHECK) ? "Checking on.\r\n" : "Not checking.\r\n", ch);
    } else if (is_abbrev(arg1, "overflow")) {
      buffer_opt ^= BUF_OVERBOOT;
      send_to_char((buffer_opt & BUF_OVERBOOT) ? "Reboot on overflow.\r\n" : "Will try to keep going.\r\n", ch);
    } else
      send_to_char(BUFFER_FORMAT, ch);
  } else if (is_abbrev(arg1, "delete")) {
    struct buf_data *toy, **b_head = get_buffer_head();
    for (toy = b_head[get_magnitude(size)]; toy; toy = toy->next) {
      if (USED(toy))
	continue;
      if (toy->size != size)
	continue;
      if (persistant != toy->type)
	continue;
      if (LOCKED(toy))
	continue;
      LOCK(toy, LOCK_WILL_REMOVE);
      remove_buffer(toy);
      LOCK(toy, LOCK_WILL_FREE);
      free_buffer(toy);
      break;
    }
    if (!toy)
      send_to_char("Not found.\r\n", ch);
  } else if (is_abbrev(arg1, "add"))
    new_buffer(size, persistant); /* So easy. :) */
  else
    send_to_char(BUFFER_FORMAT, ch);

  release_buffer(arg1);
}

/*
 * Private: get_used(buffer to search)
 * Used mainly in when releasing a buffer and check size is set.
 * Does a backwards search for the first non-NUL character.
 * Useful for when a function uses a large buffer and then uses
 * it later for a smaller string, this will always return the
 * most used value.
 */
static int get_used(struct buf_data *buf)
{
  int cnt;
  for (cnt = buf->req_size - 1; cnt > 0 && buf->data[cnt] == '\0'; cnt--);
  return cnt;
}

/* ------------------------------------------------------------------------ */

#if BUFFER_MEMORY

char *debug_str_dup(const char *txt, const char *varname, const char *func, ush_int line)
{
  if (buffer_opt & BUF_VERBOSE)
    log("BUF: debug_str_dup: %d bytes from %s:%d for '%s'.", strlen(txt) + 1, func, line, varname);
  return strcpy(acquire_buffer(strlen(txt) + 1, BT_MALLOC, varname, func, line), txt);
}

void *debug_calloc(size_t number, size_t size, const char *varname, const char *func, int line)
{
  if (buffer_opt & BUF_VERBOSE)
    log("BUF: debug_calloc: %d*%d bytes from %s:%d for '%s'.", number, size, func, line, varname);
  return acquire_buffer(number * size, BT_MALLOC, varname, func, line);
}

void debug_free(void *ptr, const char *func, ush_int line)
{
  if (buffer_opt & BUF_VERBOSE)
    log("BUF: debug_free: %p from %s:%d.", ptr, func, line);
  /* BT_MALLOC's are free()'d */
  detach_buffer(ptr, BT_MALLOC, func, line);
}

void *debug_realloc(void *ptr, size_t size, const char *varname, const char *func, int line)
{
  void *xtra;
  if (buffer_opt & BUF_VERBOSE)
    log("BUF: debug_realloc: '%s' (%p) resized to %d bytes for %s:%d.", varname, ptr, size, func, line);
  xtra = acquire_buffer(size, BT_MALLOC, varname, func, line);
  memmove(xtra, ptr, size);
  detach_buffer(ptr, BT_MALLOC, func, line);
  return xtra;
}
#endif

/* ------------------------------------------------------------------------ */

#if BUFFER_THREADED

static void bufferlist_lock(const char *func, ush_int line)
{
  if (buffer_opt & BUF_VERBOSE)
    log("BUF: List locked by %s:%d.", func, line);
  pthread_mutex_lock(&buflistmutex);
}

static void bufferlist_unlock(const char *func, ush_int line)
{
  if (buffer_opt & BUF_VERBOSE)
    log("BUF: List unlocked by %s:%d.", func, line);
  pthread_mutex_unlock(&buflistmutex);
}

static void buffer_lock(struct buf_data *buf, int type, const char *func, ush_int line)
{
  if (buf == NULL)
    log("BUF: SYSERR: buffer_lock: buf == NULL from %s:%d", func, line);
  else if (LOCKED(buf) != LOCK_NONE && LOCKED(buf) != LOCK_WILL_REMOVE && type != LOCK_WILL_FREE && LOCKED(buf) != LOCK_WILL_CLEAR && type != LOCK_WILL_REMOVE)
    log("BUF: SYSERR: buffer_lock: Trying to lock a buffer %p already locked, from %d to %d at %s:%d.", buf, LOCKED(buf), type, func, line);
  else
    buf->locked = type;
}

static void buffer_unlock(struct buf_data *buf, const char *func, ush_int line)
{
  if (buf == NULL)
    log("BUF: SYSERR: buffer_unlock: buf == NULL from %s:%d.", func, line);
  else if (LOCKED(buf) == LOCK_NONE)
    log("BUF: SYSERR: buffer_unlock: Buffer %p isn't locked, from %s:%d.", buf, func, line);
  else
    buf->locked = LOCK_NONE;
}

/*
 * Public: buffer_thread()
 * If we ever have a multithreaded base, this would be a very nice
 * application for it.  I plan to add a timer on how long someone
 * has used a buffer.
 *
 * Work in progress.
 */
void *buffer_thread(void *nothing)
{
  struct timeval tv;

  tv.tv_sec = 0;
  tv.tv_usec = OPT_USEC;
  log("BUF: Started buffer thread.");

  while (!circle_shutdown) {
    /* This is a generic function to reduce the life on all buffers by 1. */
    decrement_all_buffers();

    /* This checks for any buffers which were never released. */
    release_old_buffers();

    /* Here we free() any buffer which has not been used in a while. */
    free_old_buffers();

    /* Sleep the same amount of time the main loop does. */
    select(0, NULL, NULL, NULL, &tv);
  
    /* See if we should exit. */
    pthread_testcancel();
  }

  log("BUF: Buffer thread exited.");
  return NULL;
}
#endif

/* ------------------------------------------------------------------------ */

#if 0 /* BUFFER_SNPRINTF */
/*
 * Buffer using sprintf() with bounds checking via snprintf()
 */
int bprintf(buffer *str, const char *format, ...)
{
  va_list args;
  int chars, warned = FALSE;

  for (;;) {
    va_start(args, format);
    chars = vsnprintf(str->data, str->req_size, format, args);
    va_end(args);

    /* Wrote ok. */
    if (chars < str->req_size)
      break;

    if (!warned)
      log("BUF: SYSERR: bprintf: Overflow attempt in buffer %p from %s:%d with format '%s'.", str, str->who, str->line, format);

    /* Don't try again if it doesn't matter. */
    if (str->req_size == str->size)
      break;

    /* Don't double warn. */
    warned = TRUE;
    str->req_size = str->size;
  }
  return chars;
}

/*
 * Buffer wrapper.
 * b2b = from buffer to buffer.
 * s2b = from string to buffer.
 * b2s = from buffer to string.
 * s2s = doesn't exist, think about it.
 *
 * Another options would be to make b2b_cpy the default strcpy with #define.
 */
char *b2b_cpy(buffer *str1, const buffer *str2) { strcpy(str1->data, str2->data); }
char *s2b_cpy(buffer *str1, const char *str2) { strcpy(str1->data, str2); }
char *b2s_cpy(char *str1, const buffer *str2) { strcpy(str1, str2->data); )
char *b2s_cat(char *str1, const buffer *str2) { strcat(str1, str2->data); }
char *s2b_cat(buffer *str1, const char *str2) { strcat(str1->data, str2); }
char *b2b_cat(buffer *str1, const buffer *str2) { strcat(str1->data, str2->data); }
#endif

/* ------------------------------------------------------------------------ */

/*
 * At the bottom because we need to undefine the macro to get correct
 * results.
 */
/*#if BUFFER_MEMORY
#undef free*/
void really_free(void *ptr)
{
  if (!ptr)	/* Your OS may already do this, but it's insignificant. */
    return;
  free(ptr);
}
/*#endif*/
