#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "buffer.h"
#include "utils.h"

void half_chop(char *string, char *arg1, char *arg2) {}
int is_abbrev(const char *arg1, const char *arg2) {}
void send_to_char(const char *x, struct char_data *y) {}
void send_to_all(const char *x) {}
int circle_shutdown, circle_reboot;
void skip_spaces(char **x) {}
#define logfile stderr
void basic_mud_log(const char *format, ...)
{va_list args;time_t ct = time(0);char *time_s = asctime(localtime(&ct));
time_s[strlen(time_s) - 1] = '\0';if (logfile == NULL)abort();
fprintf(logfile, "%-15.15s :: ", time_s + 4);va_start(args, format);
vfprintf(logfile, format, args);va_end(args);fprintf(logfile, "\n");
fflush(logfile);}
extern int buffer_opt;

int main(void)
{
  buffer *foo;
  buffer *bar;

  init_buffers();
  buffer_opt = 20;

  foo = get_buffer(32);
  bar = get_buffer(32);

  bprintf(foo, "This is a test of the emergency buffer system.");
  s2b_cat(foo, "This is only a test.");

  s2b_cpy(bar, "1234567890123456789012345678901234567890");
  s2b_cat(bar, "1234567890123456789012345678901234567890");
  printf("[%s|%s]\n", sz(foo), sz(bar));
  release_buffer(bar);
  release_buffer(foo);

  return 0;
}
