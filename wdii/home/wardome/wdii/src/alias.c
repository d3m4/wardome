/***************************************************************************
*  File: alias.c                                 an addition to CircleMUD  *
*  Usage: saving player's aliases                                          *
*                                                                          *
*  a drop-in replacement for alias.c                                       *
*  written by Edward Almasy (almasy@axis.com)                              *
*                                                                          *
*  (original alias.c by Jeremy Hess and Chad Thompson)                     *
*                                                                          *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University  *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.                *
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "interpreter.h"
#include "db.h"
#include "buffer.h"

void write_aliases(struct char_data *ch)
{
  FILE         *ptFHndl;
  char          pcFileName[50];
  struct alias *pstAliasRec;
  char         *pcRepStart;
  
  get_filename(GET_NAME(ch), pcFileName, ALIAS_FILE);
  
  unlink(pcFileName);
  
  if (GET_ALIASES(ch) == NULL)
    return;
  
  ptFHndl = fopen(pcFileName,"wt");
  
  pstAliasRec = GET_ALIASES(ch);
  while (pstAliasRec != NULL) {
    fprintf(ptFHndl, "%d\n", strlen(pstAliasRec->alias)); 
    fprintf(ptFHndl, "%s\n", pstAliasRec->alias);
   
    pcRepStart = pstAliasRec->replacement; 
    while (*pcRepStart == ' ') pcRepStart++;
    
    fprintf(ptFHndl, "%d\n", strlen(pcRepStart)); 
    fprintf(ptFHndl, "%s\n", pcRepStart);
    fprintf(ptFHndl, "%d\n", pstAliasRec->type);
   
    pstAliasRec = pstAliasRec->next;
  }
  
  fclose(ptFHndl);
}


void read_aliases(struct char_data *ch)
{
    char          pcFileName[127];
    FILE         *ptFHndl;
    struct alias *pstAliasRec;
    int           iLen;
    
    get_filename(GET_NAME(ch), pcFileName, ALIAS_FILE);
    
    ptFHndl = fopen(pcFileName, "r");
    
    if (ptFHndl == NULL)
        return;
    
    CREATE(pstAliasRec, struct alias, 1);
    GET_ALIASES(ch) = pstAliasRec;

    while (!feof(ptFHndl))
    {
        fscanf(ptFHndl, "%d\n", &iLen);

        CREATE(pstAliasRec->alias, char, (iLen + 2));

        fgets(pstAliasRec->alias, (iLen+1), ptFHndl);

        fscanf(ptFHndl, "%d\n", &iLen);

        CREATE(pstAliasRec->replacement, char, (iLen + 3));
        *pstAliasRec->replacement = ' ';;

        fgets((pstAliasRec->replacement + 1), (iLen+1), ptFHndl);

        fscanf(ptFHndl, "%d\n", &pstAliasRec->type);

        if (!feof(ptFHndl))
        {
            CREATE(pstAliasRec->next, struct alias, 1);
            pstAliasRec = pstAliasRec->next;
            pstAliasRec->next = NULL;
        }
    }

    fclose(ptFHndl);
} 
