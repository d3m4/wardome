#if !defined(__BUFFER_H__)
#define __BUFFER_H__

/*
 * CONFIGURABLES (aka, The place to shoot your own foot.) :)
 * ---------------------------------------------------------
 */

/*
 * 1 = use buffer system for all memory allocations using CREATE().
 * 0 = use standard calloc/realloc in the CREATE() macro.
 *
 * The advantage to using the buffer system is that it will keep track of
 * all your allocations and warn if one of the malloc buffers is overflowed.
 * You can also view every allocation, what file and line it was allocated
 * from, and how large it was.  This would be useful to detect a memory leak.
 * Using this option, stock CircleMUD bpl12 takes 1.8 seconds to boot on my
 * Pentium 133, and 1.6 seconds to boot without it.
 */
#define BUFFER_MEMORY	1

/*
 * 1 = Include original CircleMUD buffers too.
 * 0 = Use only new buffer system.
 *
 * This will helpfully point out all your existing global buffer uses if you
 * decide to convert to all buffer system.
 */
#define USE_CIRCLE_BUFFERS      1

/*
 * -----------------------------------------------------
 * No tweakables below! See 'buffer.c' for more options.
 * -----------------------------------------------------
 */

/*
 * Handle GCC-isms.
 */
#if !defined(__GNUC__)
#define __attribute__(x)
#define __FUNCTION__	__FILE__
#endif

/*
 * These macros neatly pass the required information without breaking the
 * rest of the code.  The 'get_buffer()' code should be used for a temporary
 * memory chunk such as the current CircleMUD 'buf,' 'buf1,' and 'buf2'
 * variables.  Remember to use 'release_buffer()' to give up the requested
 * buffer when finished with it.  'release_my_buffers()' may be used in
 * functions with a lot of return statements but it is _not_ encouraged.
 * 'get_memory()' and 'release_memory()' should only be used for memory that
 * you always want handled here regardless of BUFFER_MEMORY.
 */
#define get_buffer(a)		acquire_buffer((a), BT_STACK, NULL, __FUNCTION__, __LINE__)
#define get_memory(a)		acquire_buffer((a), BT_MALLOC, NULL, __FUNCTION__, __LINE__)
#define release_buffer(a)	do { detach_buffer((a), BT_STACK, __FUNCTION__, __LINE__); (a) = NULL; } while(0)
#define release_memory(a)	do { detach_buffer((a), BT_MALLOC, __FUNCTION__, __LINE__); (a) = NULL; } while(0)
#define release_my_buffers()	detach_my_buffers(__FUNCTION__, __LINE__)

/*
 * Types for the memory to allocate.  It should never be necessary to use
 * these definitions directly.
 */
#define BT_STACK	0	/* Stack type memory.			*/
#define BT_PERSIST	1	/* A buffer that doesn't time out.	*/
#define BT_MALLOC	2	/* A malloc() memory tracker.		*/

/*
 * Check for released and timed out buffers every 5 seconds.  This can be
 * be arbitrarily changed.
 */
#define PULSE_BUFFER	(5 RL_SEC)

/*
 * Public functions for outside use.
 */
#if 0 /* BUFFER_SNPRINTF */
buffer *str_cpy(buffer *d, buffer*s);
int bprintf(buffer *buf, const char *format, ...);
#endif
#if BUFFER_MEMORY
void *debug_calloc(size_t number, size_t size, const char *var, const char *func, int line);
void *debug_realloc(void *ptr, size_t size, const char *var, const char *func, int line);
void debug_free(void *ptr, const char *func, ush_int line);
char *debug_str_dup(const char *txt, const char *var, const char *func, ush_int line);
void really_free(void *ptr);
#endif
void init_buffers(void);
void exit_buffers(void);
void release_all_buffers(void);
struct buf_data *detach_buffer(char *data, byte type, const char *func, const int line_n);
void detach_my_buffers(const char *func, const int line_n);
char *acquire_buffer(size_t size, int type, const char *var, const char *who, ush_int line);
void show_buffers(struct char_data *ch, int buffer_type, int display_type);
extern int buffer_cache_stat[];
#define BUFFER_CACHE_HITS	0
#define BUFFER_CACHE_MISSES	1

#endif
