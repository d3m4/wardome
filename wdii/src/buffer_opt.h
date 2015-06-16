#if !defined(__BUFFER_OPT_H__)
#define __BUFFER_OPT_H__

/*
 * CONFIGURABLES (aka, The place to shoot your own foot.) :)
 * ---------------------------------------------------------
 */

/*
 * Check for released and timed out buffers every 5 seconds.  This can be
 * be arbitrarily changed, but be careful of the lease life warning in buffer.c
 */
#define PULSE_BUFFER    (5 RL_SEC)

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
 *
 * NOTE: If you use this option with BUFFER_SNPRINTF, you'll have to use
 *	the buffer copy/cat routines for CREATE() memory also...
 */
#define BUFFER_MEMORY		1

/*
 * 1 = Include original CircleMUD buffers too.
 * 0 = Use only new buffer system.
 *
 * This will blatantly point out all your existing global buffer uses if you
 * decide to convert to all buffer system.
 */
#define USE_CIRCLE_BUFFERS	0

/*
 * 1 = Return the buffer structure to the functions.
 * 0 = Return a 'char *' just like malloc() does.
 *
 * Useful for automatic length checking through snprintf().
 *
 * NOTE: This option has even more ramifications with BUFFER_MEMORY, see above.
 */
#define BUFFER_SNPRINTF		0

/*
 * 1 = Enable multithreaded support.  You _must_ have POSIX threads for this!
 *     (Also please note that since CircleMUD currently isn't thread-safe,
 *      this hasn't been very well tested except via the thread.c program.)
 * 0 = Use the standard heartbeat() method of freeing the buffers.
 */
#define BUFFER_THREADED         0

/*
 * --------------------------------
 * See 'buffer.c' for more options.
 * --------------------------------
 */

/*
 * We need these for structs.h, but can't include it in buffer.h because that
 * file needs the 'byte' from structs.h...sigh.
 *
 * buffer.h needs 'byte' from structs.h which would need 'buffer' from buffer.h
 */
#if BUFFER_SNPRINTF == 0
typedef char    buffer;
#else
struct buf_data;
typedef struct buf_data       buffer;
#endif

#endif
