//header

void carregar_clans(void);
void abrir_clan(char *clan_arq, short int numero);
void salvar_clan(short int num_clan);
void aceitar_membro(char *argumento, struct char_data *ch);
void posicoes_clan(char *argumento, struct char_data *ch);
void alterar_nomes_pos(char *argumento, struct char_data *ch);
void muda_pos_comando(char *argumento, struct char_data *ch);
void promover_membro(char *argumento, struct char_data *ch);
void rebaixar_membro(char *argumento, struct char_data *ch);
void expulsar_membro(char *argumento, struct char_data *ch);
void falar_clan(char *argumento, struct char_data *ch);
void membros_clan(char *argumento, struct char_data *ch);
void depositar_clan(char *argumento,struct char_data *ch);
void sacar_clan(char *argumento, struct char_data *ch);
void guerra_clan (char *argumento, struct char_data *ch);
void status_clan(char *argumento, struct char_data *ch);
void aceitar_guerra(char *argumento, struct char_data *ch);
void recusar_guerra(char *argumento, struct char_data *ch);
void serender_guerra(char *argumento, struct char_data *ch);
void custo_guerra(char *argumento, struct char_data *ch);
void custo_clan(void);
void alianca_clan(char *argumento, struct char_data *ch);
void claim_zone(struct char_data *ch, struct obj_data *obj);

int posicao_clan(short int num_clan);
int qto_poder(struct char_data *ch);
short int tem_permissao(struct char_data *ch, short int comando);
short int remove_do_clan(struct char_data *ch);


#define CLAN_GOD		LVL_IMPL

#define MAX_MEMBROS		20
#define MAX_POSICOES		10

//comandos
#define CONVOCAR		0
#define NUMPOSICOES		1
#define ALTERANOMEPOS		2
#define MUDAPOSCOMANDO		3
#define PROMOVER		4
#define REBAIXAR		5
#define EXPULSAR		6
#define FALAR			7
#define MEMBROS			8
#define DEPOSITAR		9
#define SACAR			10
#define GUERRA			11	
#define STATUS			12
#define ACEITAR			13
#define RECUSAR			14
#define SERENDER		15
#define CUSTO			16
#define ALIANCA			17

#define NUM_COMANDOS		18

struct estrutura_clan {
	short int clan_num;
	short int numero_clans;
	short int numero_posicoes;
	short int numero_membros;
	short int pos_comando[NUM_COMANDOS];
	char nome_clan[500];
	char pos_nome[MAX_POSICOES][50];
	long membros[MAX_MEMBROS];
	int grana;
	int poder;
	int em_guerra;
	int alianca;
	int pkvit;
	int pkder;
 	int preco;
 	int rendicoes;
 	int recusas;
 	int tempo;
 	int raided;
 	int seen;
 	int desafiando;
 	int desafiado;
 	int religion;
 	int points;
 	int resources;
 	
};

struct estrutura_clan *clans;

