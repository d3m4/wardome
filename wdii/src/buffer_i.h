#if !defined(__BUFFER_I_H__)
#define __BUFFER_I_H__

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

#endif
