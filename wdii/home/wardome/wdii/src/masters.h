/* File: Masters.h				**
** Usado para os Comandos basicos de Discipulos **
** Implementado por Taerom e Zhantar            **
** Inicio do Projeto: 04/07/03                  */

void criacao(struct descriptor_data *d, char *arg);
void criacao_new(struct descriptor_data *d, char *arg);
void nome_maiusculo(char *saida, char *entrada);
void verifica(struct descriptor_data *d);
void ganha_rp(struct char_data *ch);
void list_skills(struct char_data *ch);

int prac_params[4][NUM_CLASSES];

extern char *pc_class_types[];
extern char *pc_race_types[];
extern int check_remort_sk;
extern struct int_app_type int_app[];
extern int spell_sort_info[MAX_SKILLS+1];
extern char *spells[];
extern struct spell_info_type spell_info[];


ACMD(do_master);
SPECIAL(master);


 #define LEARNED_LEVEL	        0 // % known which is considered "learned"/
 #define MAX_PER_PRAC		1 // max percent gain in skill per practice 
 #define MIN_PER_PRAC		2 // min percent gain in skill per practice 
 
 #define SPELL	0
 #define SKILL	1
 
 #define LEARNED(ch) (prac_params[LEARNED_LEVEL][(int)GET_CLASS(ch)])
 #define MINGAIN(ch) (prac_params[MIN_PER_PRAC][(int)GET_CLASS(ch)])
 #define MAXGAIN(ch) (prac_params[MAX_PER_PRAC][(int)GET_CLASS(ch)])
 
 #define MENU \
"&YW&yE&YL&yC&YO&YM&yE&Y &yT&YO&y &YW&yA&YR&yM&YA&yS&YT&yE&YR&y G&YU&yI&YL&yD&n\r\n" \
"\r\n" \
"&yHere you can learn new &Yskills &yor &Yspells.&n\r\n" \
"&yType &Ypractice &y<&Yskill&y/&Yspell&y> to learn.\r\n" \
"\r\n" \
"&yYou can learn the following &Yskill&y/&Yspell:&n\r\n" \
"&Y[&yCost&Y] &Y[&yName&Y]              &Y[&yCost&Y] &Y[&yName&Y]              &Y[&yCost&Y] &Y[&yName&Y]&n\r\n" \

int prac_params[4][NUM_CLASSES] = {

  { 10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10   },	/* learned level */
  { 15,  15,  15,  15,  15,  15,  15,  15,  15,  15,  15,  15	},	/* max per practice */
  { 25,	 25,  25,  25,	25,  25,  25,  25,  25,	 25,  25,  25   },	/* min per practice */
  {  1,   0,   1,   0,   1,   1,   0,   1,   1,   0,   1,   0   },	/* prac name */
};




