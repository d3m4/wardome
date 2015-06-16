/*
	diskio.c	Fast file buffering

	(C) Copyright 1998 by Brian Boyle

	Version 1.3
*/

#include "conf.h"
#include "sysdep.h"

#include <sys/stat.h>

#include "diskio.h"


int fbgetline(FBFILE *fbfl, char *line)
{
  char *r = fbfl->ptr, *w = line;

  if(!fbfl || !line || !*fbfl->ptr)
    return FALSE;

  for(; *r && *r != '\n' && r <= fbfl->buf + fbfl->size; r++)
    *(w++) = *r;

  while(*r == '\r' || *r == '\n')
    r++;

  *w = '\0';

  if(r > fbfl->buf + fbfl->size)
    return FALSE;
  else {
    fbfl->ptr = r;
    return TRUE;
  }
}


int find_string_size(char *str)
{
  int i;
  char *p;

  if(!str || !*str || *str == '~')
    return 0;

  for(i = 1, p = str; *p; i++) {
    switch(*p) {
    case '\r':
      i++;
      if(*(++p) == '\n')
	p++;
      break;
    case '\n':
      i++;
      if(*(++p) == '\r') {
        *(p - 1) = '\r';
	*(p++) = '\n';
      } else
	p++;
      break;
    case '~':
      if(*(p - 1) == '\r' || *(p - 1) == '\n' ||
	*(p + 1) == '\r' || *(p + 1) == '\n' || *(p + 1) == '\0')
	return i;
      else
	p++;
      break;
    default:
      p++;
    }
  }
  return i;
}


char *fbgetstring(FBFILE *fl)
{
  int size;
  char *str, *r, *w;

  if(!fl || !*fl->ptr)
    return NULL;

  if(!(size = find_string_size(fl->ptr)))
    return NULL;

  str = (char *)malloc(size + 1);
  *str = '\0';
  r = fl->ptr;
  w = str;

  for( ; *r; r++, w++) {
    switch(*r) {
    case '\r':
      *(w++) = '\r';
      *w = '\n';
      if(*(r + 1) == '\n')
	r++;
      break;
    case '\n':
      *(w++) = '\r';
      *w = '\n';
      break;
    case '~':
      if(*(r - 1) == '\r' || *(r - 1) == '\n' ||
	*(r + 1) == '\r' || *(r + 1) == '\n' || *(r + 1) == '\0') {
	*w = '\0';
	for(r++; *r == '\r' || *r == '\n'; r++);
	fl->ptr = r;
	return str;
      } else
      *w = *r;
      break;
    case '\0':
      *w = '\0';
      fl->ptr = r;
      return str;
    default:
      *w = *r;
    }
  }
  fl->ptr = r;
  return str;
}


FBFILE *fbopen_for_read(char *fname)
{
  int err;
  FILE *fl;
  struct stat sb;
  FBFILE *fbfl;

  if(!(fbfl = (FBFILE *)malloc(sizeof(FBFILE))))
    return NULL;

  if(!(fl = fopen(fname, "r"))) {
    free(fbfl);
    return NULL;
  }

  err = fstat(fileno(fl), &sb);
  if(err < 0 || sb.st_size <= 0) {
    free(fbfl);
    fclose(fl);
    return NULL;
  }

  fbfl->size = sb.st_size;
  if(!(fbfl->buf = malloc(fbfl->size))) {
    free(fbfl);
    return NULL;
  }
  if(!(fbfl->name = malloc(strlen(fname) + 1))) {
    free(fbfl->buf);
    free(fbfl);
    return NULL;
  }
  fbfl->ptr = fbfl->buf;
  fbfl->flags = FB_READ;
  strcpy(fbfl->name, fname);
  fread(fbfl->buf, sizeof(char), fbfl->size, fl);
  fclose(fl);

  return fbfl;
}


FBFILE *fbopen_for_write(char *fname, int mode)
{
  FBFILE *fbfl;

  if(!(fbfl = (FBFILE *)malloc(sizeof(FBFILE))))
    return NULL;

  if(!(fbfl->buf = malloc(FB_STARTSIZE))) {
    free(fbfl);
    return NULL;
  }
  if(!(fbfl->name = malloc(strlen(fname) + 1))) {
    free(fbfl->buf);
    free(fbfl);
    return NULL;
  }
  strcpy(fbfl->name, fname);
  fbfl->ptr = fbfl->buf;
  fbfl->size = FB_STARTSIZE;
  fbfl->flags = mode;

  return fbfl;
}


FBFILE *fbopen(char *fname, int mode)
{
  if(!fname || !*fname || !mode)
    return NULL;

  if(IS_SET(mode, FB_READ))
    return fbopen_for_read(fname);
  else if(IS_SET(mode, FB_WRITE) || IS_SET(mode, FB_APPEND))
    return fbopen_for_write(fname, mode);
  else
    return NULL;
}


int fbclose_for_read(FBFILE *fbfl)
{
  if(!fbfl)
    return 0;

  if(fbfl->buf)
    free(fbfl->buf);
  if(fbfl->name)
    free(fbfl->name);
  free(fbfl);
  return 1;
}


int fbclose_for_write(FBFILE *fbfl)
{
  char *arg, *tname;
  int len, bytes_written;
  FILE *fl;

  if(!fbfl || !fbfl->name || !fbfl->buf || fbfl->ptr == fbfl->buf)
    return 0;

  if(IS_SET(fbfl->flags, FB_APPEND))
    arg = "wa";
  else
    arg = "w";

  if(!(tname = malloc(strlen(fbfl->name) + 6)))
    return 0;

  len = strlen(fbfl->buf);
  if(!len)
    return 0;
  sprintf(tname, "%s.tmp", fbfl->name);

  if(!(fl = fopen(tname, arg))) {
    free(tname);
    return 0;
  }

  if((bytes_written = fwrite(fbfl->buf, sizeof(char), len, fl)) < len) {
    fclose(fl);
    remove(tname);
    free(tname);
    return 0;
  }

  fclose(fl);
  remove(fbfl->name);
  rename(tname, fbfl->name);
  free(tname);
  free(fbfl->name);
  free(fbfl->buf);
  free(fbfl);
  return bytes_written;
}


int fbclose(FBFILE *fbfl)
{
  if(!fbfl)
    return 0;

  if(IS_SET(fbfl->flags, FB_READ))
    return fbclose_for_read(fbfl);
  else if(IS_SET(fbfl->flags, FB_WRITE | FB_APPEND))
    return fbclose_for_write(fbfl);
  else
    return 0;
}


int fbprintf(FBFILE *fbfl, const char *format, ...)
{
  int bytes_written = 0, length = 0;
  va_list args;

  if(fbfl->ptr - fbfl->buf > (FB_STARTSIZE * 3) / 4) {
    length = fbfl->ptr - fbfl->buf;
    if(!(fbfl->buf = realloc(fbfl->buf, fbfl->size + FB_STARTSIZE)))
      return 0;
    fbfl->ptr = fbfl->buf + length;
    fbfl->size += FB_STARTSIZE;
  }

  va_start(args, format);
  bytes_written = vsprintf(fbfl->ptr, format, args);
  va_end(args);

  fbfl->ptr += bytes_written;
  return bytes_written;
}


void fbrewind(FBFILE *fbfl)
{
  fbfl->ptr = fbfl->buf;
}


int fbcat(char *fromfilename, FBFILE *tofile)
{
  struct stat sb;
  FILE *fromfile;
  char *in_buf = 0;
  int errnum = 0, length = 0;

  if(!fromfilename || !*fromfilename || !tofile)
    return 0;

  if(!(fromfile = fopen(fromfilename, "r+b")))
    return 0;

  errnum = fstat(fileno(fromfile), &sb);
  if(errnum < 0 || sb.st_size <= 0)
    return 0;

  length = tofile->ptr - tofile->buf;
  tofile->buf = realloc(tofile->buf, tofile->size + sb.st_size);
  tofile->ptr = tofile->buf + length;
  tofile->size += sb.st_size;
  in_buf = malloc(sb.st_size + 1);
  in_buf[0] = 0;
  errnum = fread(in_buf, sb.st_size, 1, fromfile);
  fbprintf(tofile, "%s", in_buf);
  fclose(fromfile);
  free(in_buf);
  return 1;
}
