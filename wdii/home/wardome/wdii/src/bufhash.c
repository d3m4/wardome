/*
 * Copyright (C), 1997, 1998 George Greer
 * License: public domain
 * Warranty: none at all
 *
 * To use:
 *   gcc -o hash bufhash.c
 *   ./hash <hash_size> <MIN_ALLOC> <MAX_ALLOC>
 *
 * i.e.: hash 11 6 16
 */
#include <stdio.h>
#include <stdlib.h>

int pow2(int n)
{
  return (--n, n == -1 ? 1 : pow2(n) * 2);
}

int hash(int i, int w)
{
  return (i % w);
}

int main(int argc, char **argv)
{
  int i, hv, min, max;

  if (argc != 4) {
    printf("Usage: %s <hash> <min order> <max order>\n", argv[0]);
    return 1;
  }

  hv = atoi(argv[1]);
  min = atoi(argv[2]);
  max = atoi(argv[3]);

  for (i = min; i <= max; i++) {
    printf("%2d: %5d -> %2d\n", i, pow2(i), hash(pow2(i), hv));
  }
  return 0;
}
