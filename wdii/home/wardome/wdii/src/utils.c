/* ************************************************************************
*   File: utils.c                                       Part of CircleMUD *
*  Usage: various internal functions of a utility nature                  *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "buffer.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "screen.h"
#include "spells.h"
#include "handler.h"
#include <stdarg.h>

extern struct descriptor_data *descriptor_list;
extern struct time_data time_info;
extern struct room_data *world;
extern struct zone_data *zone_table;
extern int top_of_world;

/* local functions */
struct time_info_data *real_time_passed(time_t t2, time_t t1);
struct time_info_data *mud_time_passed(time_t t2, time_t t1);
void die_follower(struct char_data * ch);
void add_follower(struct char_data * ch, struct char_data * leader);
char *make_bar(int val, int max, int len, int cores);
char *stripcr(char *dest, const char *src);
void topten(struct char_data *topch) ;
void topten1(struct char_data *topch) ;
void topten2(struct char_data *topch) ;


void topten1(struct char_data *topch)
{
  int x ; int y ; int z ; int cont ;

  struct toptenlevel
 {
  char nome[30] ;
  int level ;
  long id ;
 } ;


 struct toptenlevel topvetorX[10] ;
 struct toptenlevel topvetor[10] ;
 struct toptenlevel aux ;

 FILE *fl;
 const char *filename = TOPTEN_FILE1 ;

 if(GET_LEVEL(topch) > 200)     // NAO MUDE ESSA LINHA NUNCA!!!!!!!!!!!!!
  return ;                      // imortals e gods nao fazem parte do topten

 if(GET_ARENA_DIED(topch) + GET_ARENA_KILLED(topch) == 0)
  return ;

 cont =  3* GET_ARENA_KILLED(topch) *
 GET_ARENA_KILLED(topch)/(GET_ARENA_DIED(topch)
+GET_ARENA_KILLED(topch)) * GET_ARENA_KILLED(topch)/(GET_ARENA_DIED(topch)
+GET_ARENA_KILLED(topch));


 if(cont == 0)
  return ;

/* A parte a seguir deve ser executada apenas 1 vez. Essa parte eh responsavel
por criar o arquivo inicial. Ela nao deve existir depois que esse
arquivo inicial for criado. Ou seja, apenas execute a parte a seguir
1 vez. Nao se esqueca de entrar no mud pelo menos com 1 player para
esse parte poder ser executada. Qualquer duvida entre em contato com
luigi(lpalma@dc.ufscar.br)

   if (!(fl = fopen(filename, "w")))
   {
    send_to_char("Could not open the file. Report to Luigi.\r\n", topch);
    return;
   }

   topvetorX[0].id = 0  ;
   topvetorX[0].level = 10  ;
   strcpy(topvetorX[0].nome, "Joao0");

   topvetorX[1].id = 0  ;
   topvetorX[1].level = 9  ;
   strcpy(topvetorX[1].nome, "Joao1");

   topvetorX[2].id = 0  ;
   topvetorX[2].level = 8  ;
   strcpy(topvetorX[2].nome, "Joao2");

   topvetorX[3].id = 0  ;
   topvetorX[3].level = 7  ;
   strcpy(topvetorX[3].nome, "Joao3");

   topvetorX[4].id = 0  ;
   topvetorX[4].level = 6  ;
   strcpy(topvetorX[4].nome, "Joao4");

   topvetorX[5].id = 0  ;
   topvetorX[5].level = 5  ;
   strcpy(topvetorX[5].nome, "Joao5");

   topvetorX[6].id = 0 ;
   topvetorX[6].level = 4  ;
   strcpy(topvetorX[6].nome, "Joao6");

   topvetorX[7].id = 0  ;
   topvetorX[7].level = 3  ;
   strcpy(topvetorX[7].nome, "Joao7");

   topvetorX[8].id = 0  ;
   topvetorX[8].level = 2  ;
   strcpy(topvetorX[8].nome, "Joao8");

   topvetorX[9].id = 0  ;
   topvetorX[9].level = 1  ;
   strcpy(topvetorX[9].nome, "Joao9");

 fwrite(topvetorX, sizeof(struct toptenlevel[10]), 1, fl);
 fclose(fl);
  */

  if (!(fl = fopen(filename, "r")))
  {
   send_to_char("Could not open the file. Report to Luigi.\r\n", topch);
   return;
  }

 fread(topvetor, sizeof(struct toptenlevel[10]),1, fl)   ;
 fclose(fl);

 for(x = 0 ; x < 10 ; x++)
 {
  topvetorX[x].id =  topvetor[x].id ;
  topvetorX[x].level =  topvetor[x].level   ;
  strcpy(topvetorX[x].nome,topvetor[x].nome)  ;
 }

 y = 0 ;

 for(x = 0 ; x<10 ; x++)
 {
  if(GET_IDNUM(topch) == topvetorX[x].id )
  {
   topvetorX[x].level = cont;
   y = 1 ;
  }
 }

 if(topvetorX[9].level >=  cont && (y == 0))
 {
  //send_to_char("Weak, too weak.\r\n", topch);
  return;
 }
 else
  //send_to_char("Your data has been refreshed on topten.\r\n", topch);

 if(y == 0)
 {
  topvetorX[9].id = GET_IDNUM(topch) ;
  topvetorX[9].level = cont;
  strcpy(topvetorX[9].nome,GET_NAME(topch));
 }

 z = 1   ;

  while(z == 1)       // ordenacao
 {
  z = 0 ;
  for(x = 0 ; x < 9 ; x++)
  {
   if(topvetorX[x].level < topvetorX[x+1].level)
   {
    aux.level = topvetorX[x].level;
    strcpy(aux.nome, topvetorX[x].nome) ;
    aux.id = topvetorX[x].id;

    topvetorX[x].level = topvetorX[x+1].level;
    strcpy(topvetorX[x].nome, topvetorX[x+1].nome);
    topvetorX[x].id = topvetorX[x+1].id;

    topvetorX[x+1].level = aux.level;
    strcpy(topvetorX[x+1].nome, aux.nome);
    topvetorX[x+1].id = aux.id ;
    z = 1;
   }
  }
 }


 if (!(fl = fopen(filename, "w")))
 {
  send_to_char("Could not open the file. Report to Luigi.\r\n", topch);
  return;
 }

 fwrite(topvetorX, sizeof(struct toptenlevel[10]), 1, fl) ;
 fclose(fl);

}



void topten2(struct char_data *topch)
{
  int x ; int y ; int z ; int cont ;

  struct toptenlevel
 {
  char nome[30] ;
  int level ;
  long id ;
 } ;


 struct toptenlevel topvetorX[10] ;
 struct toptenlevel topvetor[10] ;
 struct toptenlevel aux ;

 FILE *fl;
 const char *filename = TOPTEN_FILE2 ;

 if(GET_LEVEL(topch) > 200)     // NAO MUDE ESSA LINHA NUNCA!!!!!!!!!!!!!
  return ;                      // imortals e gods nao fazem parte do topten

 cont = GET_LEVEL(topch) + GET_REMORT(topch)*200 ;

 if(cont == 0)
  return ;

/* A parte a seguir deve ser executada apenas 1 vez. Essa parte eh responsavel
por criar o arquivo inicial. Ela nao deve existir depois que esse
arquivo inicial for criado. Ou seja, apenas execute a parte a seguir
1 vez. Nao se esqueca de entrar no mud pelo menos com 1 player para
esse parte poder ser executada. Qualquer duvida entre em contato com
luigi(lpalma@dc.ufscar.br)

   if (!(fl = fopen(filename, "w")))
   {
    send_to_char("Could not open the file. Report to Luigi.\r\n", topch);
    return;
   }

   topvetorX[0].id = 0  ;
   topvetorX[0].level = 10  ;
   strcpy(topvetorX[0].nome, "Joao0");

   topvetorX[1].id = 0  ;
   topvetorX[1].level = 9  ;
   strcpy(topvetorX[1].nome, "Joao1");

   topvetorX[2].id = 0  ;
   topvetorX[2].level = 8  ;
   strcpy(topvetorX[2].nome, "Joao2");

   topvetorX[3].id = 0  ;
   topvetorX[3].level = 7  ;
   strcpy(topvetorX[3].nome, "Joao3");

   topvetorX[4].id = 0  ;
   topvetorX[4].level = 6  ;
   strcpy(topvetorX[4].nome, "Joao4");

   topvetorX[5].id = 0  ;
   topvetorX[5].level = 5  ;
   strcpy(topvetorX[5].nome, "Joao5");

   topvetorX[6].id = 0 ;
   topvetorX[6].level = 4  ;
   strcpy(topvetorX[6].nome, "Joao6");

   topvetorX[7].id = 0  ;
   topvetorX[7].level = 3  ;
   strcpy(topvetorX[7].nome, "Joao7");

   topvetorX[8].id = 0  ;
   topvetorX[8].level = 2  ;
   strcpy(topvetorX[8].nome, "Joao8");

   topvetorX[9].id = 0  ;
   topvetorX[9].level = 1  ;
   strcpy(topvetorX[9].nome, "Joao9");

 fwrite(topvetorX, sizeof(struct toptenlevel[10]), 1, fl);
 fclose(fl);
  */

  if (!(fl = fopen(filename, "r")))
  {
   send_to_char("Could not open the file. Report to Luigi.\r\n", topch);
   return;
  }

 fread(topvetor, sizeof(struct toptenlevel[10]),1, fl)   ;
 fclose(fl);

 for(x = 0 ; x < 10 ; x++)
 {
  topvetorX[x].id =  topvetor[x].id ;
  topvetorX[x].level =  topvetor[x].level   ;
  strcpy(topvetorX[x].nome,topvetor[x].nome)  ;
 }

 y = 0 ;

 for(x = 0 ; x<10 ; x++)
 {
  if(GET_IDNUM(topch) == topvetorX[x].id )
  {
   topvetorX[x].level = cont;
   y = 1 ;
  }
 }

 if(topvetorX[9].level >=  cont && (y == 0))
 {
  //send_to_char("Weak, too weak.\r\n", topch);
  return;
 }
 else
  //send_to_char("Your data has been refreshed on topten.\r\n", topch);

 if(y == 0)
 {
  topvetorX[9].id = GET_IDNUM(topch) ;
  topvetorX[9].level = cont;
  strcpy(topvetorX[9].nome,GET_NAME(topch));
 }

 z = 1   ;

  while(z == 1)       // ordenacao
 {
  z = 0 ;
  for(x = 0 ; x < 9 ; x++)
  {
   if(topvetorX[x].level < topvetorX[x+1].level)
   {
    aux.level = topvetorX[x].level;
    strcpy(aux.nome, topvetorX[x].nome) ;
    aux.id = topvetorX[x].id;

    topvetorX[x].level = topvetorX[x+1].level;
    strcpy(topvetorX[x].nome, topvetorX[x+1].nome);
    topvetorX[x].id = topvetorX[x+1].id;

    topvetorX[x+1].level = aux.level;
    strcpy(topvetorX[x+1].nome, aux.nome);
    topvetorX[x+1].id = aux.id ;
    z = 1;
   }
  }
 }


 if (!(fl = fopen(filename, "w")))
 {
  send_to_char("Could not open the file. Report to Luigi.\r\n", topch);
  return;
 }

 fwrite(topvetorX, sizeof(struct toptenlevel[10]), 1, fl) ;
 fclose(fl);

}





void topten(struct char_data *topch)
{
  int x ; int y ; int z ;

  struct toptenlevel
 {
  char nome[30] ;
  int level ;
  long id ;
 } ;


 struct toptenlevel topvetorX[10] ;
 struct toptenlevel topvetor[10] ;
 struct toptenlevel aux ;

 FILE *fl;
 const char *filename = TOPTEN_FILE ;

 if(GET_LEVEL(topch) > 200)     // NAO MUDE ESSA LINHA NUNCA!!!!!!!!!!!!!
  return ;                      // imortals e gods nao fazem parte do
                                //topten

 sprintf(buf, "%s verificado na lista de top ten", GET_NAME(topch));
   mudlog(buf, NRM, LVL_IMMORT, 0);
/* A parte a seguir deve ser executada apenas 1 vez. Essa parte eh
responsavel
por criar o arquivo inicial. Ela nao deve existir depois que esse
arquivo inicial for criado. Ou seja, apenas execute a parte a seguir
1 vez. Nao se esqueca de entrar no mud pelo menos com 1 player para
esse parte poder ser executada. Qualquer duvida entre em contato com
luigi(lpalma@dc.ufscar.br)

   if (!(fl = fopen(filename, "w")))
   {
    send_to_char("Could not open the file. Report to Luigi.\r\n", topch);
    return;
   }

   topvetorX[0].id = 0  ;
   topvetorX[0].level = 10  ;
   strcpy(topvetorX[0].nome, "Joao0");
   topvetorX[1].id = 0  ;
   topvetorX[1].level = 9  ;
   strcpy(topvetorX[1].nome, "Joao1");
   topvetorX[2].id = 0  ;
   topvetorX[2].level = 8  ;
   strcpy(topvetorX[2].nome, "Joao2");

   topvetorX[3].id = 0  ;
   topvetorX[3].level = 7  ;
   strcpy(topvetorX[3].nome, "Joao3");

   topvetorX[4].id = 0  ;
   topvetorX[4].level = 6  ;
   strcpy(topvetorX[4].nome, "Joao4");

   topvetorX[5].id = 0  ;
   topvetorX[5].level = 5  ;
   strcpy(topvetorX[5].nome, "Joao5");

   topvetorX[6].id = 0 ;
   topvetorX[6].level = 4  ;
   strcpy(topvetorX[6].nome, "Joao6");
   topvetorX[7].id = 0  ;
   topvetorX[7].level = 3  ;
   strcpy(topvetorX[7].nome, "Joao7");
   topvetorX[8].id = 0  ;
   topvetorX[8].level = 2  ;
   strcpy(topvetorX[8].nome, "Joao8");
   topvetorX[9].id = 0  ;
   topvetorX[9].level = 1  ;
   strcpy(topvetorX[9].nome, "Joao9");
 fwrite(topvetorX, sizeof(struct toptenlevel[10]), 1, fl);

 fclose(fl);

  */

  if (!(fl = fopen(filename, "r")))
  {
   send_to_char("Could not open the file. Report to Luigi.\r\n", topch);
   return;
  }

 fread(topvetor, sizeof(struct toptenlevel[10]),1, fl)   ;
 fclose(fl);
 for(x = 0 ; x < 10 ; x++)
 {
  topvetorX[x].id =  topvetor[x].id ;
  topvetorX[x].level =  topvetor[x].level   ;
  strcpy(topvetorX[x].nome,topvetor[x].nome)  ;
//  sprintf(buf, "Faz parte do top ten '%d'", topvetorX[x].level);
//  mudlog(buf, NRM, LVL_IMMORT, 0);
//  sprintf(buf, "Faz parte do top ten '%s'", topvetorX[x].nome);
//  mudlog(buf, NRM, LVL_IMMORT, 0);
//  sprintf(buf, "Faz parte do top ten '%5ld'", xx);
//  mudlog(buf, NRM, LVL_IMMORT, 0);
 }



// if(topvetorX[9].level >=  GET_QP(topch))
//  return ;

 y = 0 ;
 for(x = 0 ; x<10 ; x++)
 {
  if(GET_IDNUM(topch) == topvetorX[x].id )
  {
   topvetorX[x].level = GET_QP(topch);
   y = 1 ;
  }
 }

 if(topvetorX[9].level >=  GET_QP(topch) && (y == 0))
 {
  //send_to_char("Weak, too weak.\r\n", topch);
  return;
 }
 else
  //send_to_char("Your data has been refreshed on topten.\r\n", topch);
 if(y == 0)
 {
  topvetorX[9].id = GET_IDNUM(topch) ;
  topvetorX[9].level = GET_QP(topch);
  strcpy(topvetorX[9].nome,GET_NAME(topch));
 }
 z = 1   ;
  while(z == 1)       // ordenacao
 {
  z = 0 ;
  for(x = 0 ; x < 9 ; x++)
  {
   if(topvetorX[x].level < topvetorX[x+1].level)
   {
    aux.level = topvetorX[x].level;
    strcpy(aux.nome, topvetorX[x].nome) ;
    aux.id = topvetorX[x].id;

    topvetorX[x].level = topvetorX[x+1].level;
    strcpy(topvetorX[x].nome, topvetorX[x+1].nome);
    topvetorX[x].id = topvetorX[x+1].id;

    topvetorX[x+1].level = aux.level;
    strcpy(topvetorX[x+1].nome, aux.nome);
    topvetorX[x+1].id = aux.id ;
    z = 1;
   }
  }
 }


 if (!(fl = fopen(filename, "w")))
 {
  send_to_char("Could not open the file. Report to Luigi.\r\n", topch);
  return;
 }

 fwrite(topvetorX, sizeof(struct toptenlevel[10]), 1, fl) ;
 fclose(fl);
// for(x = 0 ; x < 10 ; x++)
// {
//  sprintf(buf, "\r\nFaz parte do top ten '%d' ", topvetorX[x].level);
//  send_to_char(buf, topch);
//  sprintf(buf, "\r\nFaz parte do top ten '%s' ", topvetorX[x].nome);
//  send_to_char(buf, topch);
// }
}




/* adiciona separador de milhar a numeros */
/*
char *add_points(int z_number)
{
  int unidade, milhar, milhao, bilhao, trab;
  char sunidade[3], smilhar[3], smilhao[3];
  char z_temp[MAX_STRING_LENGTH];
  bool neg = FALSE;

  *sunidade = '\0';
  *smilhar = '\0';
  *smilhao = '\0';
  *z_temp = '\0';
  *buf3 = '\0';

  if (z_number < 0) {
    trab = - z_number;
    neg = TRUE;
  }
  else
    trab = z_number;

  if (trab >= 1000000000) {
    bilhao  = trab / 1000000000;
    milhao  = (trab - bilhao * 1000000000) / 1000000;
    milhar  = (trab - bilhao * 1000000000 - milhao * 1000000) / 1000;
    unidade = trab - bilhao * 1000000000 - milhao * 1000000 - milhar * 1000;
    if (milhao < 100) {
      strcpy(smilhao, "0");
      if (milhao < 10) {
        strcpy(smilhao, "00");
      }
    }
    if (milhar < 100) {
      strcpy(smilhar, "0");
      if (milhar < 10) {
        strcpy(smilhar, "00");
      }
    }
    if (unidade < 100) {
      strcpy(sunidade , "0");
      if (unidade < 10) {
        strcpy(sunidade, "00");
      }
    }
    sprintf(z_temp, "%d,%s%d,%s%d,%s%d", bilhao, smilhao, milhao, smilhar, milhar, sunidade, unidade);
  }
  else if (trab >= 1000000) {
    milhao  = trab / 1000000;
    milhar  = (trab - milhao * 1000000) / 1000;
    unidade = trab - milhao * 1000000 - milhar * 1000;
    if (milhar < 100) { strcpy(smilhar, "0"); if (milhar < 10)
      { strcpy(smilhar, "00"); } }
    if (unidade < 100) { strcpy(sunidade , "0"); if (unidade < 10)
      { strcpy(sunidade, "00"); } }
    sprintf(z_temp, "%d,%s%d,%s%d", milhao, smilhar, milhar, sunidade, unidade);
  }
  else if (trab >= 1000) {
    milhar  = trab / 1000;
    unidade = trab - milhar * 1000;
    if (unidade < 100) {
      strcpy(sunidade , "0");
      if (unidade < 10) {
        strcpy(sunidade, "00");
      }
    }
    sprintf(z_temp, "%d,%s%d", milhar, sunidade, unidade);
  }
  else
    sprintf(z_temp, "%d", trab);

  sprintf(buf3, "%s%s", (neg?"-":""), z_temp);
  return (buf3);
}
*/

// by samis
/*
char *add_points(int z_number)
{
 	int x, i = 0;
	bool negativo = 0;
	char buffer[200];

	if(z_number < 0) {
		z_number *= -1;
		negativo = 1;
	}

	sprintf(buffer, "%d", z_number);

 	x = strlen(buffer);

	while((x-=3) > 0)
	{
		i++;
		memmove(buffer + x + 1, buffer + x, i * 3 + i);
		*(buffer + x) = ',';
 	}

	sprintf(buf3, "%s%s", (negativo ? "-" : ""), buffer);

	return(buf3);
}
*/

/* by zaaroth, sem memmove que processa d+ e sem sequencia
demasiada de calculos */
char *add_points(int z_number)
{
	unsigned short int x, z = 0, y = 0;
	char buffer[20];

	sprintf(buffer, "%d", z_number);

	x = strlen(buffer);

	while(z < x)
	{
		*(buf3+z+y) = buffer[z];
		z++;
		if(!((x-z) % 3) && *(buf3+z+y-1) != '-')
		{
			*(buf3+z+y) = ',';
			y++;
		}
	}
	*(buf3+x+y-1) = '\0';

	return(buf3);
}

const char *unidade[] = {
  "",
  "K",
  "M",
  "G",
  "\n"
};

char *convert_num(double var)
{
   int x = 0, y = 0;

   *buf3 = '\0';

   if(var < 0) {
	var *= -1;
	y = TRUE;
   }

   if(var >= 1000000000) {
   	var /= 1000000000;
	x = 3;
   } else if(var >= 1000000) {
   	var /= 1000000;
	x = 2;
   } else if(var >= 1000) {
   	var /= 1000;
	x = 1;
   }

   if(x)
     sprintf(buf2, "%s%.2f%s", (y?"-":""), var, unidade[x]);
   else
     sprintf(buf2, "%s%.0f%s", (y?"-":""), var, unidade[x]);

   return (buf2);
}

/* creates a random number in interval [from;to] */
int number(int from, int to)
{
  /* error checking in case people call number() incorrectly */
  if (from > to) {
    int tmp = from;
    from = to;
    to = tmp;
    log("SYSERR: number() should be called with lowest, then highest. number(%d, %d), not number(%d, %d).", from, to, to, from);
  }

  return ((circle_random() % (to - from + 1)) + from);
}


/* simulates dice roll */
int dice(int number, int size)
{
  int sum = 0;

  if (size <= 0 || number <= 0)
    return 0;

  while (number-- > 0)
    sum += ((circle_random() % size) + 1);

  return sum;
}


int MIN(int a, int b)
{
  return a < b ? a : b;
}


int MAX(int a, int b)
{
  return a > b ? a : b;
}

//#if BUFFER_MEMORY == FALSE
/* Create a duplicate of a string */
/*char *str_dup(const char *source)
{
  char *new_z;

  CREATE(new_z, char, strlen(source) + 1);
  return (strcpy(new_z, source));
}*/
//#endif

int str_cmp(const char *arg1, const char *arg2)
{
  int chk, i;

  if (arg1 == NULL || arg2 == NULL) {
    log("SYSERR: str_cmp() passed a NULL pointer, %p or %p.", arg1, arg2);
    return (0);
  }

  for (i = 0; arg1[i] || arg2[i]; i++)
    if ((chk = LOWER(arg1[i]) - LOWER(arg2[i])) != 0)
      return (chk);	/* not equal */

  return (0);
}

/* strn_cmp: a case-insensitive version of strncmp */
/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different, end of both, or n reached     */
int strn_cmp(const char *arg1, const char *arg2, int n)
{
  int chk, i;

  if (arg1 == NULL || arg2 == NULL) {
    log("SYSERR: strn_cmp() passed a NULL pointer, %p or %p.", arg1, arg2);
    return (0);
  }

  for (i = 0; (arg1[i] || arg2[i]) && (n > 0); i++, n--)
    if ((chk = LOWER(arg1[i]) - LOWER(arg2[i])) != 0)
      return (chk);	/* not equal */

  return (0);
}


/* log a death trap hit */
void log_death_trap(struct char_data * ch)
{
  char buf[150];

  sprintf(buf, "%s hit death trap #%d (%s)", GET_NAME(ch),
          GET_ROOM_VNUM(IN_ROOM(ch)), world[ch->in_room].name);
  mudlog(buf, BRF, LVL_IMMORT, TRUE);
}

/*
 * New variable argument log() function.  Works the same as the old for
 * previously written code but is very nice for new code.
 */
void basic_mud_log(const char *format, ...)
{
  va_list args;
  time_t ct = time(0);
  char *time_s = asctime(localtime(&ct));

  if (logfile == NULL) {
    puts("SYSERR: Using log() before stream was initialized!");
    return;
  }

  if (format == NULL)
    format = "SYSERR: log() received a NULL format.";

  time_s[strlen(time_s) - 1] = '\0';

  fprintf(logfile, "%-15.15s :: ", time_s + 4);

  va_start(args, format);
  vfprintf(logfile, format, args);
  va_end(args);

  fprintf(logfile, "\n");
  fflush(logfile);
}


/* the "touch" command, essentially. */
int touch(const char *path)
{
  FILE *fl;

  if (!(fl = fopen(path, "a"))) {
    log("SYSERR: %s: %s", path, strerror(errno));
    return (-1);
  } else {
    fclose(fl);
    return (0);
  }
}


/*
 * mudlog -- log mud messages to a file & to online imm's syslogs
 * based on syslog by Fen Jul 3, 1992
 */
void mudlog(const char *str, int type, int level, int file)
{
  char buf[MAX_STRING_LENGTH], tp;
  struct descriptor_data *i;

  if (str == NULL)
    return;	/* eh, oh well. */
  if (file)
    log("%s", str);
  if (level < 0)
    return;

  sprintf(buf, "[ %s ]\r\n", str);

  for (i = descriptor_list; i; i = i->next) {
    if (STATE(i) != CON_PLAYING || IS_NPC(i->character)) /* switch */
      continue;
    if (GET_LEVEL(i->character) < level)
      continue;
    if (PLR_FLAGGED(i->character, PLR_WRITING))
      continue;
    tp = ((PRF_FLAGGED(i->character, PRF_LOG1) ? 1 : 0) +
	  (PRF_FLAGGED(i->character, PRF_LOG2) ? 2 : 0));
    if (tp < type)
      continue;

    send_to_char(CCGRN(i->character, C_NRM), i->character);
    send_to_char(buf, i->character);
    send_to_char(CCNRM(i->character, C_NRM), i->character);
  }
}


void sprintbit(long bitvector, const char *names[], char *result)
{
  long nr;

  *result = '\0';

  if (bitvector < 0) {
    strcpy(result, "<INVALID BITVECTOR>");
    return;
  }
  for (nr = 0; bitvector; bitvector >>= 1) {
    if (IS_SET(bitvector, 1)) {
      if (*names[nr] != '\n') {
        strcat(result, names[nr]);
        strcat(result, " ");
      } else
        strcat(result, "UNDEFINED ");
    }
    if (*names[nr] != '\n')
      nr++;
  }

  if (!*result)
    strcpy(result, "NOBITS ");
}



void sprinttype(int type, const char *names[], char *result)
{
  int nr = 0;

  while (type && *names[nr] != '\n') {
    type--;
    nr++;
  }

  if (*names[nr] != '\n')
    strcpy(result, names[nr]);
  else
    strcpy(result, "UNDEFINED");
}


/* Calculate the REAL time passed over the last t2-t1 centuries (secs) */
struct time_info_data *real_time_passed(time_t t2, time_t t1)
{
  long secs;
  static struct time_info_data now;

  secs = (long) (t2 - t1);

  now.hours = (secs / SECS_PER_REAL_HOUR) % 24; /* 0..23 hours */
  secs -= SECS_PER_REAL_HOUR * now.hours;

  now.day = (secs / SECS_PER_REAL_DAY); /* 0..34 days  */
  secs -= SECS_PER_REAL_DAY * now.day;

  now.month = -1;
  now.year = -1;

  return &now;
}



/* Calculate the MUD time passed over the last t2-t1 centuries (secs) */
struct time_info_data *mud_time_passed(time_t t2, time_t t1)
{
  long secs;
  static struct time_info_data now;

  secs = (long) (t2 - t1);

  now.hours = (secs / SECS_PER_MUD_HOUR) % 24;  /* 0..23 hours */
  secs -= SECS_PER_MUD_HOUR * now.hours;

  now.day = (secs / SECS_PER_MUD_DAY) % 35;     /* 0..34 days  */
  secs -= SECS_PER_MUD_DAY * now.day;

  now.month = (secs / SECS_PER_MUD_MONTH) % 17; /* 0..16 months */
  secs -= SECS_PER_MUD_MONTH * now.month;

  now.year = (secs / SECS_PER_MUD_YEAR);        /* 0..XX? years */

  return &now;
}



struct time_info_data *age(struct char_data * ch)
{
  static struct time_info_data player_age;
  struct affected_type af;
  
  player_age = *mud_time_passed(time(0), ch->player.time.birth);

  player_age.year += 17;        /* All players start at 17 */

	//send_to_char("&WParabens! &wHoje e' seu aniversario.&n\r\n",ch);
	/*GET_HIT(ch) = GET_MAX_HIT(ch);
  	GET_MANA(ch) = GET_MAX_MANA(ch);
  	GET_MOVE(ch) = GET_MAX_MOVE(ch);
  	    af.type = SPELL_BLESS;
	    af.location = APPLY_HITROLL;
	    af.duration = 10;
	    af.modifier = 20 + (GET_LEVEL(ch) / 30);
	    af.bitvector = AFF_HASTE;
	    af.bitvector2 = AFF_SANCTUARY;
	    af.bitvector3 = APPLY_NONE;
	    affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
*/
  return &player_age;
}


/* Check if making CH follow VICTIM will create an illegal */
/* Follow "Loop/circle"                                    */
bool circle_follow(struct char_data * ch, struct char_data * victim)
{
  struct char_data *k;

  for (k = victim; k; k = k->master) {
    if (k == ch)
      return TRUE;
  }

  return FALSE;
}



/* Called when stop following persons, or stopping charm */
/* This will NOT do if a character quits/dies!!          */
void stop_follower(struct char_data * ch)
{
  struct follow_type *j, *k;

  if (ch->master == NULL) {
    core_dump();
    return;
  }

  if (AFF_FLAGGED(ch, AFF_CHARM)) {
    act("You realize that $N is a jerk!", FALSE, ch, 0, ch->master, TO_CHAR);
    act("$n realizes that $N is a jerk!", FALSE, ch, 0, ch->master, TO_NOTVICT);
    act("$n hates your guts!", FALSE, ch, 0, ch->master, TO_VICT);
    if (affected_by_spell(ch, SPELL_CHARM))
      affect_from_char(ch, SPELL_CHARM);
  } else {
    act("You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
    act("$n stops following $N.", TRUE, ch, 0, ch->master, TO_NOTVICT);
    act("$n stops following you.", TRUE, ch, 0, ch->master, TO_VICT);
  }

  if (ch->master->followers->follower == ch) {  /* Head of follower-list? */
    k = ch->master->followers;
    ch->master->followers = k->next;
    free(k);
  } else {                      /* locate follower who is not head of list */
    for (k = ch->master->followers; k->next->follower != ch; k = k->next);

    j = k->next;
    k->next = j->next;
    free(j);
  }

  ch->master = NULL;
  REMOVE_BIT(AFF_FLAGS(ch), AFF_CHARM | AFF_GROUP);
}



/* Called when a character that follows/is followed dies */
void die_follower(struct char_data * ch)
{
  struct follow_type *j, *k;

  if (ch->master)
    stop_follower(ch);

  for (k = ch->followers; k; k = j) {
    j = k->next;
    stop_follower(k->follower);
  }
}



/* Do NOT call this before having checked if a circle of followers */
/* will arise. CH will follow leader                               */
void add_follower(struct char_data * ch, struct char_data * leader)
{
  struct follow_type *k;

  if (ch->master) {
    core_dump();
    return;
  }

  ch->master = leader;

  CREATE(k, struct follow_type, 1);

  k->follower = ch;
  k->next = leader->followers;
  leader->followers = k;

  act("You now follow $N.", FALSE, ch, 0, leader, TO_CHAR);
  if (CAN_SEE(leader, ch))
    act("$n starts following you.", TRUE, ch, 0, leader, TO_VICT);
  act("$n starts to follow $N.", TRUE, ch, 0, leader, TO_NOTVICT);
}

/*
 * get_line reads the next non-blank line off of the input stream.
 * The newline character is removed from the input.  Lines which begin
 * with '*' are considered to be comments.
 *
 * Returns the number of lines advanced in the file.
 */
int get_line(FILE *fl, char *buf)
{
  char temp[256];
  int lines = 0;
  int sl;

  do {
    if (!fgets(temp, 256, fl))
      return (0);
    lines++;
  } while (*temp == '*' || *temp == '\n');

  /* Last line of file doesn't always have a \n, but it should. */
  sl = strlen(temp);
  if (sl > 0 && temp[sl - 1] == '\n')
    temp[sl - 1] = '\0';

  strcpy(buf, temp);
  return (lines);
}

int get_filename(char *orig_name, char *filename, int mode)
{
  const char *prefix, *middle, *suffix;
  char name[64], *ptr;

  switch (mode) {
  case CRASH_FILE:
    prefix = LIB_PLROBJS;
    suffix = SUF_OBJS;
    break;
  case ETEXT_FILE:
    prefix = LIB_PLRTEXT;
    suffix = SUF_TEXT;
    break;
  case ALIAS_FILE:
    prefix = LIB_ALIAS;
    suffix = SUF_ALIAS;
    break;
  default:
    return 0;
  }

  if (!*orig_name)
    return 0;

  strcpy(name, orig_name);
  for (ptr = name; *ptr; ptr++)
    *ptr = LOWER(*ptr);

  switch (LOWER(*name)) {
  case 'a':  case 'b':  case 'c':  case 'd':  case 'e':
    middle = "A-E";
    break;
  case 'f':  case 'g':  case 'h':  case 'i':  case 'j':
    middle = "F-J";
    break;
  case 'k':  case 'l':  case 'm':  case 'n':  case 'o':
    middle = "K-O";
    break;
  case 'p':  case 'q':  case 'r':  case 's':  case 't':
    middle = "P-T";
    break;
  case 'u':  case 'v':  case 'w':  case 'x':  case 'y':  case 'z':
    middle = "U-Z";
    break;
  default:
    middle = "ZZZ";
    break;
  }

  sprintf(filename, "%s%s"SLASH"%s.%s", prefix, middle, name, suffix);
  return 1;
}


/* string manipulation fucntion originally by Darren Wilson */
/* (wilson@shark.cc.cc.ca.us) improved and bug fixed by Chris (zero@cnw.com) */
/* completely re-written again by M. Scott 10/15/96 (scottm@workcommn.net), */
/* substitute appearances of 'pattern' with 'replacement' in string */
/* and return the # of replacements */
int replace_str(char **string, char *pattern, char *replacement, int rep_all,
                int max_size) {
   char *replace_buffer = NULL;
   char *flow, *jetsam, temp;
   int len, i;

   if ((strlen(*string) - strlen(pattern)) + strlen(replacement) > max_size)
     return -1;

   CREATE(replace_buffer, char, max_size);
   i = 0;
   jetsam = *string;
   flow = *string;
   *replace_buffer = '\0';
   if (rep_all) {
      while ((flow = (char *)strstr(flow, pattern)) != NULL) {
         i++;
         temp = *flow;
         *flow = '\0';
         if ((strlen(replace_buffer) + strlen(jetsam) + strlen(replacement)) > max_size) {
            i = -1;
            break;
         }
         strcat(replace_buffer, jetsam);
         strcat(replace_buffer, replacement);
         *flow = temp;
         flow += strlen(pattern);
         jetsam = flow;
      }
      strcat(replace_buffer, jetsam);
   }
   else {
      if ((flow = (char *)strstr(*string, pattern)) != NULL) {
         i++;
         flow += strlen(pattern);
         len = ((char *)flow - (char *)*string) - strlen(pattern);

         strncpy(replace_buffer, *string, len);
         strcat(replace_buffer, replacement);
         strcat(replace_buffer, flow);
      }
   }
   if (i == 0) return 0;
   if (i > 0) {
      RECREATE(*string, char, strlen(replace_buffer) + 3);
      strcpy(*string, replace_buffer);
   }
   free(replace_buffer);
   return i;
}


/* re-formats message type formatted char * */
/* (for strings edited with d->str) (mostly olc and mail)     */
void format_text(char **ptr_string, int mode, struct descriptor_data *d, int maxlen) {
   int total_chars, cap_next = TRUE, cap_next_next = FALSE;
   char *flow, *start = NULL, temp;
   /* warning: do not edit messages with max_str's of over this value */
   char formated[MAX_STRING_LENGTH];

   flow   = *ptr_string;
   if (!flow) return;

   if (IS_SET(mode, FORMAT_INDENT)) {
      strcpy(formated, "   ");
      total_chars = 3;
   }
   else {
      *formated = '\0';
      total_chars = 0;
   }

   while (*flow != '\0') {
      while ((*flow == '\n') ||
             (*flow == '\r') ||
             (*flow == '\f') ||
             (*flow == '\t') ||
             (*flow == '\v') ||
             (*flow == ' ')) flow++;

      if (*flow != '\0') {

         start = flow++;
         while ((*flow != '\0') &&
                (*flow != '\n') &&
                (*flow != '\r') &&
                (*flow != '\f') &&
                (*flow != '\t') &&
                (*flow != '\v') &&
                (*flow != ' ') &&
                (*flow != '.') &&
                (*flow != '?') &&
                (*flow != '!')) flow++;

         if (cap_next_next) {
            cap_next_next = FALSE;
            cap_next = TRUE;
         }

         /* this is so that if we stopped on a sentance .. we move off the sentance delim. */
         while ((*flow == '.') || (*flow == '!') || (*flow == '?')) {
            cap_next_next = TRUE;
            flow++;
         }

         temp = *flow;
         *flow = '\0';

         if ((total_chars + strlen(start) + 1) > 79) {
            strcat(formated, "\r\n");
            total_chars = 0;
         }

         if (!cap_next) {
            if (total_chars > 0) {
               strcat(formated, " ");
               total_chars++;
            }
         }
         else {
            cap_next = FALSE;
            *start = UPPER(*start);
         }

         total_chars += strlen(start);
         strcat(formated, start);

         *flow = temp;
      }

      if (cap_next_next) {
         if ((total_chars + 3) > 79) {
            strcat(formated, "\r\n");
            total_chars = 0;
         }
         else {
            strcat(formated, "  ");
            total_chars += 2;
         }
      }
   }
   strcat(formated, "\r\n");

   if (strlen(formated) > maxlen) formated[maxlen] = '\0';
   RECREATE(*ptr_string, char, MIN(maxlen, strlen(formated)+3));
   strcpy(*ptr_string, formated);
}
int num_pc_in_room(struct room_data *room)
{
  int i = 0;
  struct char_data *ch;

  for (ch = room->people; ch != NULL; ch = ch->next_in_room)
    if (!IS_NPC(ch))
      i++;

  return i;
}

/*
char *make_bar(int val, int max, int len, int cores)
{
	short int i;

	*buf2 = '[';
	*(buf2+len+1) = ']';
	*(buf2+len+2) = '\0';

	if (max <= 0)
	 max = 1;

	i = ((val*len)/max);

	for(; len > 0; len--)
		*(buf2+len) = (i >= len ? '>' : ' ');

	return(buf2);
}
*/

char *insere_cor(char *onde, char *oque, short int cores)
{
	char cor[10];

	if(!cores)
		return(onde);

	strcpy(cor, oque);

	oque = cor;

	for(; *oque; oque++)
		*(onde++) = *oque;

	return(onde);
}

char *corbarra[] = {
	KBBLU,
	KBCYN,
	KBGRN,
	KBYEL,
	KBRED,
	KNRM
};

char *make_bar(int val, int max, int len, int cores)
{
	char *buffer;
	int lenbkp, i;

	buffer = buf2;

	if (max <= 0)
		max = 1;

	i = ((val*len)/max);

	buffer = insere_cor(buffer, KBWHT, cores);
	*(buffer++) = '[';

	for(lenbkp = 0; lenbkp < len; lenbkp++)
	{
		if(i >= lenbkp)
		{
			if(!((5*(len-lenbkp)) % len))
				buffer = insere_cor(buffer, corbarra[((5*(len-lenbkp))/len)-1], cores);
			*(buffer++) = '>';
		}
		else
			*(buffer++) = ' ';
	}
	buffer = insere_cor(buffer, KBWHT, cores);
	*(buffer++) = ']';
	buffer = insere_cor(buffer, KNRM, cores);

	*buffer = '\0';

	return(buf2);
}

char *make_mbar(int min, int val, int max, int len, int cores)
{
  unsigned int i, n;
  char barra[MAX_INPUT_LENGTH];

    strcpy(barra, "&W[&r");

    i = len-(((max-val) * len) / (max+(min < 0 ? -min : min)));
    for (n = 0; n <= len; n++){
      if(n == len/2)
		strcat(barra, "&w:&b");
      else {
	if(n == i){
	  if(i > len/2)
		strcat(barra, "&B|&b");
	  else if(i < len/2)
		strcat(barra, "&R|&r");
	} else
		strcat(barra, "-");
      }
    }
    strcat(barra, "&W]&n");
    strcpy(buf2, barra);
    return (buf2);
}
/*
char *reprimir(char *palavra, char *vemde, int num)
{
    char *bkp;
    sh_int limit = 0;

    if(strlen(vemde) <= num)
      bkp = vemde;
    else {
      bkp = palavra;
	*palavra++ = *vemde;
	while(*vemde++ && limit++ < (num-4))
	     *(palavra++) = *vemde;
	*palavra++ = '.';
	*palavra++ = '.';
	*palavra++ = '.';
      *palavra = 0;
    }
    return(bkp);
}
*/

// samis
char *reprimir(char *vemde, int num)
{
    strcpy(buf2, vemde);

    if(strlen(buf2) > num)
    {

    	*(buf2 + num - 3) = '.';
    	*(buf2 + num - 2) = '.';
    	*(buf2 + num - 1) = '.';
    	*(buf2 + num) = '\0';
    }

    return (buf2);
}

/* strips \r's from line */
char *stripcr(char *dest, const char *src) {
   int i, length;
   char *temp;

   if (!dest || !src) return NULL;
   temp = &dest[0];
   length = strlen(src);
   for (i = 0; *src && (i < length); i++, src++)
     if (*src != '\r') *(temp++) = *src;
   *temp = '\0';
   return dest;
}

/* Check if the character doesn't have too many followers already */
int allow_follower(struct char_data *ch, int max_fol)
{
  struct follow_type *f;
  int fol = 1;
  max_fol = 2;

 if(GET_CHA(ch) >= 18) max_fol++;
 if(GET_RACE(ch) == RACE_H_SEA_ELF) max_fol++ ;

  for (f = ch->followers; f; f = f->next)
    if(IS_NPC(f->follower))
      fol++;

  return (fol > max_fol ? 0 : 1);
}

void manda_som(char *som, struct char_data *ch)
{
  	if (!PRF2_FLAGGED(ch, PRF2_WDPROTOCOL))
	   return;

	sprintf(buf2, "\x1B[play:%s]", som);
	send_to_char(buf2, ch);

}
/*
int GET_CLASS_REMORT(struct char_data *ch, int i)
{
	if(ch->player_specials)
		return (ch->player_specials->saved.rskills[i][0])
	return(0);
}

int GET_RSKILL(struct char_data *ch, int i, int num)
{
	num++;
	if(ch->player_specials)
		return (ch->player_specials->saved.rskills[i][num])
	return(0);
}
*/

/*
 * This function (derived from basic fork(); abort(); idea by Erwin S.
 * Andreasen) causes your MUD to dump core (assuming you can) but
 * continue running.  The core dump will allow post-mortem debugging
 * that is less severe than assert();  Don't call this directly as
 * core_dump_unix() but as simply 'core_dump()' so that it will be
 * excluded from systems not supporting them. (e.g. Windows '95).
 *
 * XXX: Wonder if flushing streams includes sockets?
 */


void core_dump_real(const char *who, ush_int line)
{
  log("SYSERR: Assertion failed at %s:%d!", who, line);
/*
#if defined(CIRCLE_UNIX)
  // These would be duplicated otherwise...
  fflush(stdout);
  fflush(stderr);
  fflush(logfile);


   // Kill the child so the debugger or script doesn't think the MUD
   // crashed.  The 'autorun' script would otherwise run it again.

  if (fork() == 0)
    abort();
#endif*/
}
void play_sound(struct char_data *ch, char *sound, int type)
 {
  char buf[MAX_STRING_LENGTH];
  struct char_data *vch, *next_vch;
  struct descriptor_data *d;
 
  if( (sound == NULL)  || (ch == NULL))
   return;
  sprintf( buf, "!!SOUND(%s)\n\r", sound);
  switch( type)
  {
   case SND_CHAR:
    if( HAS_SOUND(ch))
     send_to_char(buf, ch);
    break;
   case SND_ROOM:
    for (vch = world[ch->in_room].people; vch; vch = next_vch)
    {
     next_vch = vch->next_in_room;
 
     if( IS_NPC(vch))
      continue;
     if( HAS_SOUND(vch))
      send_to_char(buf, vch);
    }
    break;
   case SND_AREA:
    for (d = descriptor_list; d; d = d->next)
    {
     if( d->connected != CON_PLAYING)
      continue;
     if (d->character == NULL)
      continue;
     vch = d->character;
     if (world[IN_ROOM(ch)].zone != world[IN_ROOM(vch)].zone)
      continue;
     if (HAS_SOUND(vch))
      send_to_char(buf, vch);
    }
    break;
  case SND_WORLD:
   for( d = descriptor_list; d; d = d->next)
   {
    if( d->connected != CON_PLAYING)
     continue;
    if( d->character == NULL)
     continue;
    vch = d->character;
    if( HAS_SOUND(vch))
     send_to_char(buf, vch);
   }
   break;
   default:
   break; 
  }
  return;
 }
