/* Race file created by Archangel to WarDome MUD */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "buffer.h"
#include "interpreter.h"
#include "utils.h"
#include "db.h"

void    write_to_output(const char *txt, struct descriptor_data *d);
#define SEND_TO_Q(messg, desc)  write_to_output((messg), desc)
//int invalid_race(struct char_data *ch, struct obj_data *obj);

const char *race_abbrevs[] = {   
  "Hum",
  "Dro",
  "Dwa",
  "Elf",
  "Ogr",
  "Orc",
  "Tro",
  "Git",
  "Gno",
  "Hal",
  "Liz",
  "Sea",
  "Gor",
  "Dun",
  "Anc",
  "Nau",
  "Hig",
  "Hil",
  "Bug", 
  "Cav",
  "Lic",
  "Tin",
  "Dra",
  "Hse",
  "Arc",
  "\n"
};                               
                                 
const char *pc_race_types[] = {  
  "Human",
  "Drow",
  "Dwarf",
  "Elf",
  "Ogre",
  "Orc",
  "Troll",
  "Gith",
  "Gnome",
  "Lizardman",
  "Sea Elf",
  "Gorak",
  "Dunedain",
  "Ancient Drow",
  "Naugrim",
  "High Elf",
  "Hill Ogre",
  "BugBear",
  "Cave Troll",
  "Lich",
  "Tinker",
  "Draconian",
  "High Sea Elf",
  "Archons",
  "\n"                           
};

char *pc_race_help_types[] = {  
  "Human",
  "Drow",
  "Dwarf",
  "Elf",
  "Ogre",
  "Orc",
  "Troll",
  "Gith",
  "Gnome",
  "Lizardman",
  "Sea Elf",
  "Gorak",
  "Dunedain",
  "Ancient Drow",
  "Naugrim",
  "High Elf",
  "Hill Ogre",
  "BugBear",
  "Cave Troll",
  "Lich",
  "Tinker",
  "Draconian",
  "High Sea Elf",
  "Archons",
  "\n"                           
};                                    

/* The menu for choosing a race in interpreter.c: */ 
const char *race_menu =                              
"\r\n"
"Here you can select you race, a race defines your personality...\r\n"
"You will never change your race, so think before the selection.\r\n"
"\r\n"
"  [a] Human		""  [b] Drow		""  [c] Dwarf		\r\n"
"  [d] Elf		""  [e] Ogre		""  [f] Orc		\r\n"
"  [g] Troll		""  [h] Gith		""  [i] Gnome		\r\n"
"  [j] Lizardman		""  [l] Sea Elf		""  [m] Gorak		\r\n"
"-----------------------------------------------------------------------------  \r\n"
"  [R]Dunedain          ""  [R]Ancient Drow     ""  [R]Naugrim         \r\n" 
"  [R]High Elf          ""  [R]Hill Ogre        ""  [R]BugBear         \r\n"
"  [R]Cave Troll        ""  [R]Lich             ""  [R]Tinker          \r\n"
"  [R]Draconian                 ""  [R]High Sea Elf     ""  [R]Archons         \r\n"
" ** &b[&BR&b]&n - REMORT RACES, NOT AVAIBLE FOR NEW PLAYERS \r\r";

const char *pc_race_menu[] = {  
  "[A]    &WHuman&n        ",
  "[B]    &bDrow&n         ",
  "[C]    &GDwarf&n        ",
  "[D]    &gElf&n          ",
  "[E]    &BOgre&n         ",
  "[F]    &wOrc&n          ",
  "[G]    &cTroll&n        ",
  "[H]    &YGith&n         ",
  "[I]    &MGnome&n        ",
  "[J]    &yLizardman&n    ",
  "[L]    &CSea Elf&n      ",
  "[M]    &mGorak&n        ",
  "[R]    &WDunedain&n     ",     
  "[R]    &bAncient Drow&n ",    
  "[R]    &GNaugrim&n      ", 
  "[R]    &gHigh Elf&n     ",   
  "[R]    &BHill Ogre&n    ", 
  "[R]    &wBugBea&nr      ",
  "[R]    &cCave Troll&n   ",   
  "[R]    &YLich&n         ", 
  "[R]    &MTinker&n       ", 
  "[R]    &yDraconian&n    ",         
  "[R]    &CHigh Sea Elf&n ",  
  "[R]    &mArchons&n      ",
  "\n"                           
};

#define STR	0
#define DEX	1
#define CON	2
#define INW	3
#define WIS	4
#define CHA	5

#define MAX_ATTRIB	6
/*
int race_attrib[][MAX_ATTRIB] = {
//  STR DEX CON INT WIS CHA
    { 0,  0,  0,  0,  0,  0},//HUM  *
    { 0,  0,  0,  0,  2, -2},//DRO *
    { 0,  0,  2, -1, -1, -1},//DWA *
    { 0,  1, -1,  0,  0,  0},//ELF *
    { 4,  0,  1, -2, -2, -1},//OGR *
    { 2,  0,  1, -2, -2, -1},//ORC *
    { 0,  0,  2, -1, -1,  0},//TRO *
    {-1,  0, -2,  3,  0,  0},//GIT *
    {-1,  0,  0,  1,  0,  0},//GNO *
    { 2,  2,  0, -2, -2,  0},//LIZ
    { 0, -1, -1,  0,  2,  0},//SEA *
    { 2,  0,  0,  1,  1,  0} //GOR
};
*/

int race_attrib[][MAX_ATTRIB] = {
//  STR DEX CON INT WIS CHA
    { 9,  9,  9,  9,  9, 10},//HUM
    { 8,  13, 8, 10, 13,  8},//DRO
    { 9,  7, 11,  9,  9, 10},//DWA
    { 9, 13,  8,  10,10, 12},//ELF
    {12,  9,  9,  6,  9, 10},//OGR
    {13, 11, 13,  6,  9,  8},//ORC
    {11, 11, 13,  7,  7,  8},//TRO
    { 9,  9,  7, 12,  9,  8},//GIT
    { 9,  9,  9, 12,  9, 10},//GNO
    {12, 12, 10,  7,  8,  8},//LIZ
    { 9,  9,  8, 11, 12, 14},//SEA
    { 13, 9,  9,  8, 8,  10 }, // GORAK
    { 11,  9, 9,  9, 11, 10},//dunedain
    { 8,  15, 8, 10, 15,  8},//ancient drow
    { 11, 7, 13,  9,  9, 10},//naugrim
    { 9, 14,  8,  10,12, 14},//high elf
    {14, 10, 10,  6,  9, 10},//hill ogre
    {14, 13, 14,  6,  9,  8},//bugbear
    {12, 12, 15,  7,  7,  8},//cave troll
    { 9,  9,  9, 14,  9,  8},//lich
    { 9,  9, 11, 12, 11, 10},//tinker
    {14, 12, 10,  7,  8,  12},//dracon
    { 11, 9,  8, 13, 12, 14},//high sea elf
    { 13, 11, 9, 10, 8,  10 } // archon
};

char *atrib_line(int race)
{
   int j;

	*buf3 = '\0';
	for(j = 0; j < MAX_ATTRIB; j++)
		sprintf(buf3+strlen(buf3), "   %3d", 
race_attrib[race][j]);

   return (buf3);
}

void display_races(struct descriptor_data *d)
{
  int x;
  char tes[65536];
  
	strcpy(tes, "\r\n  &C[&cOption&C]  &WRACE&n 		  STR   DEX   CON   INT   WIS   CHA\r\n");
	for(x = 0; x < NUM_RACES; x++)
		sprintf(tes+strlen(tes), "     %s        %s\r\n", pc_race_menu[x], atrib_line(x));

	SEND_TO_Q(tes, d);
}

/*
 * The code to interpret a race letter (used in interpreter.c when a 
 * new character is selecting a race).                               
 */                                                                  
int parse_race(char arg)                                             
{                                                                    
  arg = LOWER(arg);                                                  
                                                                     
  switch (arg) {                                                     
  case 'A':
  case 'a':
    return RACE_VAMPIRE;
    break;
  case 'B':
  case 'b':
    return RACE_DROW;
    break;
  case 'C':
  case 'c':
    return RACE_DWARF;
    break;
  case 'D':
  case 'd':
    return RACE_ELF;
    break;
  case 'E':
  case 'e':
    return RACE_OGRE;
    break;
  case 'F':
  case 'f':
    return RACE_ORC;
    break;
  case 'G':
  case 'g':
    return RACE_TROLL;
    break;
  case 'H':
  case 'h':
    return RACE_GITH;
    break;
  case 'I':
  case 'i':
    return RACE_GNOME;
    break;
  case 'J':
  case 'j':
    return RACE_LIZARDMAN;
    break;
  case 'L':
  case 'l':
    return RACE_SEA_ELF;
    break;
  case 'M':
  case 'm':
    return RACE_GORAK;
    break;
  case 'N':
  case 'n':
    return RACE_DUNEDAIN;
    break;
  case 'O':
  case 'o':
    return RACE_ANCI_DROW;
    break;
  case 'P':
  case 'p':
    return RACE_NAUGRIM;
    break;
  case 'Q':
  case 'q':
    return RACE_HIGH_ELF;
    break;
  case 'R':
  case 'r':
    return RACE_HILL_OGRE;
    break;
  case 'S':
  case 's':
    return RACE_BUGBEAR;
    break;
  case 'T':
  case 't':
    return RACE_CAVE_TROLL;
    break;
  case 'U':
  case 'u':
    return RACE_LICH;
    break;
  case 'V':
  case 'v':
    return RACE_TINKER;
    break;
  case 'X':
  case 'x':
    return RACE_DRACONIAN;
    break;
  case 'Y':
  case 'y':
    return RACE_H_SEA_ELF;
    break;
  case 'Z':
  case 'z':
    return RACE_ARCHONS;
    break;
    
  default:                                                           
    return RACE_UNDEFINED;                                           
    break;                                                           
  }                                                                  
}                                                                    

long find_race_bitvector(char arg)                                   
{                                                                    
 arg = LOWER(arg);                                                  
                                                                     
 switch (arg) {                                                     

  case 'A':
  case 'a':
    return ( 1 << RACE_VAMPIRE);
    break;
  case 'B':
  case 'b':
    return ( 1 << RACE_DROW);
    break;
  case 'C':
  case 'c':
    return ( 1 << RACE_DWARF);
    break;
  case 'D':
  case 'd':
    return ( 1 << RACE_ELF);
    break;
  case 'E':
  case 'e':
    return ( 1 << RACE_OGRE);
    break;
  case 'F':
  case 'f':
    return ( 1 << RACE_ORC);
    break;
  case 'G':
  case 'g':
    return ( 1 << RACE_TROLL);
    break;
  case 'H':
  case 'h':
    return ( 1 << RACE_GITH);
    break;
  case 'I':
  case 'i':
    return ( 1 << RACE_GNOME);
    break;
  case 'J':
  case 'j':
    return ( 1 << RACE_LIZARDMAN);
    break;
  case 'L':
  case 'l':
    return ( 1 << RACE_SEA_ELF);
    break;
  case 'M':
  case 'm':
    return ( 1 << RACE_GORAK);
    break;
    case 'N':
  case 'n':
    return ( 1 << RACE_DUNEDAIN);
    break;
  case 'O':
  case 'o':
    return ( 1 << RACE_ANCI_DROW);
    break;
  case 'P':
  case 'p':
    return ( 1 << RACE_NAUGRIM);
    break;
  case 'Q':
  case 'q':
    return ( 1 << RACE_HIGH_ELF);
    break;
  case 'R':
  case 'r':
    return ( 1 << RACE_HILL_OGRE);
    break;
  case 'S':
  case 's':
    return ( 1 << RACE_BUGBEAR);
    break;
  case 'T':
  case 't':
    return ( 1 << RACE_CAVE_TROLL);
    break;
  case 'U':
  case 'u':
    return ( 1 << RACE_LICH);
    break;
  case 'V':
  case 'v':
    return ( 1 << RACE_TINKER);
    break;
  case 'X':
  case 'x':
    return ( 1 << RACE_DRACONIAN);
    break;
  case 'Y':
  case 'y':
    return ( 1 << RACE_H_SEA_ELF);
    break;
  case 'Z':
  case 'z':
    return ( 1 << RACE_ARCHONS);
    break;  
          
  default:                                                         
    return RACE_UNDEFINED;                                                      
    break;                                                         
 }                                                                  
}
/*
int invalid_race(struct char_data *ch, struct obj_data *obj) {
  if (GET_LEVEL(ch) >= LVL_GOD ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_VAMPIRE)  && IS_VAMPIRE(ch) )  ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_DROW)      && IS_DROW(ch)     )  ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_DWARF) && IS_DWARF(ch))  ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_ELF)     && IS_ELF(ch)    )  ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_OGRE)     && IS_OGRE(ch)    )  ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_ORC)       && IS_ORC(ch)      )  ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_TROLL)       && IS_TROLL(ch)      )  ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_GITH)      && IS_GITH(ch)     )  ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_GNOME)     && IS_GNOME(ch)    )  ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_LIZARDMAN)     && IS_LIZARDMAN(ch)    )  ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_SEA_ELF)    && IS_SEA_ELF(ch)   )  ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_GORAK)   && IS_GORAK(ch)) )

        return 1;
  else
        return 0;
}
*/
