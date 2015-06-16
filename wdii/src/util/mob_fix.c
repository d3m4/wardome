/* *********************************************************************** *
 *   File:  mob_fix.c                                    Part of CircleMUD *
 *  Usage:  equalizing the mob files                                       *
 *  Writer: Angus Mezick, parts taken from db.c                            *
 * *********************************************************************** */

/*
 * here is some code for a mob file equalizer.   Since we spend a lot of
 * time porting in zones, we noticed that some zones were really powerful
 * and some were weak.  this mainly had to do with mobs.  This program
 * takes in a .mob file, and then equalizes some of the stats based on
 * level, writing out to another file.  A lot of this code is taken from
 * db.c, so you should recognize it :)  I just don't want anyone making
 * money with it :).
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>


int stats[][13]=
{
/*lev hp        exp thaco  ac   #d#+dam     gold          at2   at3     at4  hit*/
  {0,  10,       100,  20, 100,  1,3,0,       100,        0,     0,     0,   0}, 
  {1,  15,       100,  19, 100,  1,4,0,       200,        0,     0,     0,   0}, 
  {2,  25,       400,  18,  90,  1,4,0,       300,        0,     0,     0,   1}, 
  {3,  40,       500,  17,  80,  1,5,0,       400,        0,     0,     0,   1},
  {4,  65,       600,  16,  70,  1,6,0,       500,        0,     0,     0,   2}, 
  {5,  75,       700,  15,  60,  1,8,1,       600,        0,     0,     0,   2}, 
  {6,  85,       800,  14,  60,  1,8,1,       700,        0,     0,     0,   3},
  {7,  95,       900,  13,  50,  1,9,2,       800,        0,     0,     0,   3},
  {8,  130,     1000,  12,  40,  1,12,2,       900,       0,     0,     0,   3},
  {9,  140,     2000,  11,  20,  1,12,3,       1000,      0,     0,     0,   4},
  {10, 150,     3000,  10,  10,  1,15,3,       1100,      0,     0,     0,   5},
  {11, 170,     4000,   9,  10,  1,15,3,       1200,      0,     0,     0,   6},
  {12, 200,     5000,   8,  10,  1,16,4,       1300,      0,     0,     0,   6},
  {13, 250,     6000,   7,  10,  1,16,4,       1400,      0,     0,     0,   7},
  {14, 300,     7000,   6,  10,  1,17,4,       1500,      5,     0,     0,   8},
  {15, 400,     8000,   5,   5,  1,17,4,      1600,      5,     0,     0,   9},
  {16, 450,     9000,   4,   5,  1,18,4,      1700,      5,     0,     0,  11},
  {17, 460,    10000,   3,  -2,  1,18,4,      1800,     10,     0,     0,  12},
  {18, 470,    20000,   2,  -5,  1,18,6,      1900,     10,     0,     0,  14},
  {19, 480,    30000,   1,  -8,  1,20,6,      2000,     10,     0,     0,  16},
  {20, 500,    40000,   0, -10,  1,20,6,      5000,     15,     0,     0,  17},
  {21, 530,    50000,  -1, -20,  1,20,7,      10000,    15,     0,     0,  18},
  {22, 560,    60000,  -2, -30,  1,22,7,      15000,    15,     0,     0,  20},
  {23, 590,    70000,  -3, -40,  1,24,10,      20000,    20,     0,     0,  20},
  {24, 600,    80000,  -4, -50,  1,26,11,      25000,    20,     0,     0,  20},
  {25, 630,    90000,  -5, -60,  1,28,12,      30000,    20,     0,     0,  20},
  {26, 660,   100000,  -6, -70,  1,30,13,      35000,    25,     0,     0,  20},
  {27, 690,   110000,  -7, -80,  1,32,14,      40000,    25,     0,     0,  20},
  {28, 700,   120000,  -8, -80,  1,36,15,      45000,    25,     0,     0,  20},
  {29, 800,   150000,  -9, -90,  1,38,16,      50000,    30,     0,     0,  20},
  {30, 900,   200000, -10,-100,  1,40,17,      55000,    30,     0,     0,  20},
  {31, 1000,  250000, -11,-100,  1,50,18,      60000,    30,     0,     0,  22},
  {32, 1200,  300000, -12,-110,  1,50,19,      70000,    30,     0,     0,  22},
  {33, 1400,  350000, -13,-110,  1,50,19,      75000,    35,     0,     0,  22},
  {34, 1600,  400000, -14,-110,  1,60,20,      80000,    40,     0,     0,  23},
  {35, 1800,  450000, -15,-110,  1,60,20,      85000,    40,     0,     0,  24},
  {36, 2000,  500000, -16,-120,  1,60,21,      90000,    45,     0,     0,  24},
  {37, 2200,  550000, -17,-120,  1,70,22,      100000,   45,    30,     0,  26},
  {38, 2400,  600000, -18,-120,  1,70,22,      120000,   50,    30,     0,  26},
  {39, 2600,  650000, -19,-130,  1,70,23,      130000,   50,    35,     0,  26},
  {40, 2800,  700000, -20,-130,  1,70,24,      140000,   55,    35,     0,  28},
  {41, 3000,  710000, -20,-130,  1,80,24,      150000,   55,    40,     0,  28},
  {42, 3000,  720000, -20,-140,  1,80,25,      160000,   60,    45,     0,  28},
  {43, 3200,  730000, -20,-140,  1,80,26,      170000,   60,    50,     0,  30},
  {44, 3200,  740000, -20,-140,  1,80,26,      180000,   65,    55,     0,  30},
  {45, 3400,  750000, -20,-150,  1,90,27,      190000,   70,    60,     0,  30},
  {46, 3400,  760000, -20,-150,  1,90,28,      200000,   75,    60,     0,  32},
  {47, 3600,  770000, -20,-150,  1,90,28,      220000,   80,    65,     0,  32},
  {48, 3600,  780000, -20,-160,  1,90,29,      230000,   80,    70,     0,  32},
  {49, 3800,  790000, -20,-160,  1,100,30,     240000,   85,    70,     0,  34},
  {50, 3800,  800000, -20,-160,  1,100,30,     250000,   90,    75,    25,  34},
  {51, 4000,  810000, -20,-170,  1,100,31,     260000,   90,    80,    30,  34},
  {52, 4000,  820000, -20,-170,  1,100,32,     270000,   95,    85,    35,  36},
  {53, 4200,  830000, -20,-170,  1,100,32,     280000,  100,    90,    40,  36},
  {54, 4200,  840000, -20,-180,  1,100,33,     290000,  100,    95,    45,  36},
  {55, 4200,  850000, -20,-180,  1,100,34,     310000,  100,   100,    55,  38},
  {56, 4400,  870000, -20,-190,  1,100,34,     320000,  100,   100,    60,  38},
  {57, 4500,  880000, -20,-190,  1,100,35,     330000,  100,   100,    65,  40},
  {58, 4500,  890000, -20,-190,  1,100,36,     340000,  100,   100,    70,  40},
  {59, 4600,  900000, -20,-200,  1,100,36,     350000,  100,   100,    75,  40},
  {60, 5000,  940000, -20,-200,  1,200,42,     550000,  100,   100,    80,  42},
  {61, 5500,  960000, -20,-200,  1,200,44,     750000,  100,   100,    85,  44},
  {62, 6000,  980000, -20,-200,  1,200,46,     850000,  100,   100,    90,  46},
  {63, 15000,1000000, -20,-200,  1,200,48,     950000,  100,   100,    95,  48},
  {64, 30000,1100000, -20,-200,  1,200,50,    1000000,  100,   100,   100,  50},
  {65, 30000,1100000, -20,-200,  1,200,50,    1000000,  100,   100,   100,  50},
  {65, 30000,1100000, -20,-200,  1,200,50,    1000000,  100,   100,   100,  50}
};

int level;

#define MAX_STRING_LENGTH 8192
#define READ_LINE  get_line(input_file,output_file,line)
#define HP(lev)  stats[(lev)][1]
#define EXP(lev) stats[(lev)][2]
#define THACO(lev) stats[(lev)][3]
#define AC(lev) stats[(lev)][4]
#define NUM_DICE(lev) stats[(lev)][5]
#define TYPE_DICE(lev) stats[(lev)][6]
#define DAMAGE(lev) stats[(lev)][7]
#define GOLD(lev) stats[(lev)][8]
#define CREATE(result, type, number)  do {\
        if (!((result) = (type *) calloc ((number), sizeof(type))))\
	 { perror("malloc failure");abort();}}while(0)
#define LOWER(c) (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))


int get_line(FILE *input_file, FILE *output_file, char *buf)
{
  char temp[256];
  int lines = 0;
  do
  {
    lines++;
    fgets(temp, 256, input_file);
    if(*temp=='*')
      fprintf(output_file,temp);
    else if (*temp)
      temp[strlen(temp) - 1] = '\0';
    else if(!*temp)
      fprintf(output_file,temp);
  }
  while (!feof(input_file)&&((*temp=='*')||(!*temp)));
  
  if (feof(input_file))
    return 0;
  else
  {
    strcpy(buf, temp);
    return lines;
  }
}


char *asciiflag_conv(char *flag)
{
  unsigned long flags = 0;
  int is_number = 1;
  char *p='\0';
  char flag_list[70];
  int i;
  char temp[40];
  char hold='\0';

  bzero(flag_list,70);
  bzero(temp,40);
  
  for (p = flag; *p; p++) {
    if (islower(*p))
      flags |= 1 << (*p - 'a');
    else if (isupper(*p))
    {
      flags |= 1 << (26 + (*p - 'A'));
      }
    if (!isdigit(*p))
      is_number = 0;
  }
  
  if (is_number)
    flags = atol(flag);
  
  if(flags>0)
  {
    for(i=31;i>=0;i--)
    {
      if(i>25)
      {
	if(flags>=(1<<i))
	{
	  hold = 'A'+(i-26);
	  sprintf(temp,"%c",hold);
	  strcat(flag_list,temp);
	  flags = flags-(1<<i);
	}
      }
      else if(flags>=(1<<i))
      {
	hold = 'a'+(i);
	sprintf(temp,"%c",hold);
	strcat(flag_list,temp);
	flags = flags-(1<<i);
      }
    }
    flag = flag_list;
  }
  return flag;
}

char fread_letter(FILE *input_file)
{
  char c;
  do
  {
    c=getc(input_file);
  }
  while(isspace(c));
  return c;
}


/* read and allocate space for a '~'-terminated string from a given file */
void  fread_string(FILE *fl,FILE *output_file, char *error)
{
  char buf[MAX_STRING_LENGTH], tmp[500];
  register char        *point;
  int  flag;
  
  bzero(buf, MAX_STRING_LENGTH);
  
  do {
    if (!fgets(tmp, MAX_STRING_LENGTH, fl)) {
      fprintf(stderr, "fread_string: format error at or near %s\n", error);
      exit(0);
    }
    
    fprintf(output_file,tmp);
    
    for (point = tmp + strlen(tmp) - 2; point >= tmp && isspace(*point);
	 point--)
      ;
    flag = (*point == '~');
  } while (!flag);
  
}

char    *fname(char *namelist)
{
  static char  holder[30];
  register char        *point;
  
  for (point = holder; isalpha(*namelist); namelist++, point++)
    *point = *namelist;
  
  *point = '\0';
  
  return(holder);
}

void parse_simple_mob(FILE *input_file, FILE *output_file, int nr)
{
  int t[10];
  char line[256];
  int thaco, maxhit, armor, numdice, damdice;
  time_t ti;
  int temp;
  

  READ_LINE;
  if(sscanf(line,"%d %d %d %dd%d+%d %dd%d+%d\n",
	    t,t+1,t+2,t+3,t+4,t+5,t+6,t+7,t+8)!=9)
   {
     printf("Format error in mob #%d, first line after S flag\n ... expecting line of form '# # # #d#+# #d#+#'\n",nr);
     exit(1);
   }
  level=t[0];
  thaco = 20 - stats[level][12];
  srand((unsigned)time(&ti));
  temp = (stats[level+1][1] - stats[level][1]);
  if (temp <= 0) { temp = 1; }
  maxhit = stats[level][1] + rand() % temp;
  armor = stats[level][4] / 10; 
  temp = ((stats[level][6]) / 4);
  if (temp <= 0) { temp = 1; }
  numdice = rand() % temp;
  if (numdice <= 0) { numdice = 1; }
  damdice = stats[level][6] / numdice;
  
  fprintf(output_file,"%d %d %d %dd%d+%d %dd%d+%d\n",
	  t[0],thaco, armor,1,1,  maxhit,
	  numdice, damdice, stats[level][7]);
  READ_LINE;
  sscanf(line," %d %d ",t,t+1);
  fprintf(output_file,"%d %d\n",stats[level][8],((stats[level][2])/2));
  READ_LINE;
  if (sscanf(line," %d %d %d ", t, t+1,t+2)!=3) 
  {
    printf("Format error in mob #%d, second line after S flag\n"
	   "...expecting line of form '# # #'\n",nr);
    exit(1);
  }
  fprintf(output_file,"%s\n",line);
}


void parse_enhanced_mob(FILE *input_file, FILE *output_file, int nr)
{
  char line[256];
  char temp[256];
  parse_simple_mob(input_file, output_file, nr);
  while(READ_LINE)
  {
    if(!strcmp(line,"E"))/*end of enhanced section*/
    {
      if (stats[level][9] != 0)
       fprintf(output_file,"Att2: %d\n",stats[level][9]);
      if (stats[level][10] != 0) 
       fprintf(output_file,"Att3: %d\n",stats[level][10]);
      if (stats[level][11] != 0)
       fprintf(output_file,"Att4: %d\n",stats[level][11]);
      fprintf(output_file,"E\n");
      return;
    }
    else if (*line =='#')
    {
      printf("Unterminated E section in mob #%d\n",nr);
      exit(1);
    }
    else
     {
      strncpy(temp,line,3);
      temp[3] = '\0';
      if(strcmp(temp,"Att")!=0)  
      {
       strcat(line,"\n");  
       fprintf(output_file,line);
      } 
     } 
  }
  printf("Unexpected end of file reached after mob #%d\n",nr);
  exit(1);
}

void parse_mob(FILE *input_file, FILE *output_file, int nr)
{
  char line[256];
  char letter;
  int t[10];
  static int i = 0;
  char f1[128],f2[128];
  char buf2[MAX_STRING_LENGTH];
  char *f1p, *f2p;
  long temp;
  
  /** string data **/
  /*get the name*/
  fread_string(input_file,output_file,buf2);
  
  /*do the short desc*/
  fread_string(input_file,output_file,buf2);
    
  /*long_desc*/
  fread_string(input_file,output_file,buf2);
    
  /*description*/
  fread_string(input_file,output_file,buf2);
  
  /** numeric data **/
  READ_LINE;
  sscanf(line,"%s %s %d %c",f1,f2,t+2,&letter);
  //f1p=asciiflag_conv(f1);
  //strcpy(f1,f1p);
  //f2p=asciiflag_conv(f2);
  //strcpy(f2,f2p);
  temp = atol(f2);
  temp=temp & 2143289343;
  sprintf(line,"%s %d %d %c\n",f1,temp,t[2],letter);
  fprintf(output_file,line);
  
  switch(letter)
  {
   case 'S':
     parse_simple_mob(input_file, output_file,nr);
     break;
   case 'E':
     parse_enhanced_mob(input_file,output_file,nr);
     break;
   default:
     printf("Unsupported mob type '%c' in mob #%d\n",letter,nr);
     exit(1);
     break;
  }
  
  letter=fread_letter(input_file);
  if(letter=='>')
  {
    ungetc(letter,input_file);
    do
    {
      READ_LINE;
      fprintf(output_file,line);
    }
    while(*line!='|');
    fprintf(output_file,"|\n");
  }
  else
    ungetc(letter,input_file);
  
  i++;
}




void main(int arg_count, char **arg_vector)
{
  FILE *input_file=0;/*input file*/
  FILE *output_file=0;/*the changed file*/
  int nr=-1;
  int last=0;
  char line[256];
  int end=1;
  char file_name[256];
  
  if(arg_count!=3)
  {
    printf("USAGE:  %s file_to_be_changed.mob new_file.mob\n",arg_vector[0]);
    exit(0);
  }
  
  sprintf(file_name,"./%s",arg_vector[1]);
  if(!(input_file=fopen(file_name,"r")))
  {
    printf("%s is not an existing file.\n",arg_vector[1]);
    exit(0);
  }
  
  sprintf(file_name,"./%s",arg_vector[2]);
  if(!(output_file=fopen(file_name,"w")))
  {
    printf("Could not open the output file: %s\n",arg_vector[2]);
    exit(0);
  }
  
  while(end)
  {
    if(!READ_LINE)
    {
      printf("Format error after mob #%d.(get_line)\n",nr);
      exit(1);
    }
    
    if(*line=='$')
    {
      fprintf(output_file,"$~\n");
      end=0;
      continue;
    }
    
    if(*line=='#')
    {
      last=nr;
      if(sscanf(line,"#%d",&nr)!=1)
      {
	printf("Format error after mob #%d (no number after #)\n",last);
	exit(1);
      }
      fprintf(output_file,"#%d\n",nr);
      if(nr>=99999)
      {
	fprintf(output_file,"$~\n");
	end=0;
	continue;
      }
      else
    	parse_mob(input_file,output_file,nr);
    }
    else
    { if ((*line=='T') || (*line=='Q')) //trigger and quest
       { 
       	strcat(line,"\n");  
        fprintf(output_file,line);   
       }
      else
       {
         printf("Format error in mob near #%d. \n",nr);
         printf("Offending line: '%s'\n",line);
       }  
    }
  }
  fclose(input_file);
  fclose(output_file);
}




