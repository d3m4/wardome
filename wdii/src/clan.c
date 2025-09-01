/***********************************************************************************
* File: Clan.c						Parte integrada do Wardome *
* Comandos Principais de Clan  		by: Zaaroth (Paulo Mello)		   *
*										   *
* Comandos de Guerra			By: Taerom (Thales Paim)		   *
*										   *
************************************************************************************/

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "clan.h"

//estruturas externas
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern struct char_data *character_list;
extern struct char_data *ch;

//estruturas internas
short int numero_clans = 0;

/* Global clans table (declared extern in clan.h) */
struct estrutura_clan *clans = NULL;

struct comando_clan {
   char *comando;
   void (*procedimento_comando) (char *argumento, struct char_data *ch);
};

//nunca mude a ordem dos comandos,
//a ordem deles interfere no salvar_clan.
const struct comando_clan comandos[] = {
	{ "convocar", aceitar_membro },
	{ "numposicoes", posicoes_clan },
	{ "alteranomepos", alterar_nomes_pos },
	{ "mudaposcomando", muda_pos_comando },
	{ "promover", promover_membro },
	{ "rebaixar", rebaixar_membro },
	{ "expulsar", expulsar_membro },
	{ "falar", falar_clan },
	{ "membros", membros_clan },
	{ "depositar", depositar_clan },
	{ "sacar", sacar_clan },
	{ "guerra", guerra_clan },
	{ "aceitar", aceitar_guerra },
	{ "recusar", recusar_guerra },
	{ "status", status_clan },
	{ "serender", serender_guerra },
	{ "custo", custo_guerra },
	{ "alianca", alianca_clan },
};

char *complete_char(char *string, int num);

void carregar_clans()
{
	FILE *arquivo;
	char num_clan[15], cam_arq[30];
	short int numero = 0;

	if(!(arquivo = fopen("clans/index", "r+")))
	{
		log("Lista de clans nao pode ser aberta.");
		return;
	}

	while(get_line(arquivo, num_clan))
		numero_clans++;

	rewind(arquivo);

	CREATE(clans, struct estrutura_clan, numero_clans + 1);

	while(get_line(arquivo, num_clan))
	{
		sprintf(cam_arq, "clans/%s.clan", num_clan);
		//colocar previamente os numeros dos clans
		clans[numero].clan_num = atoi(num_clan);
		abrir_clan(cam_arq, numero);
		numero++;
	}
	fclose(arquivo);
	log("Clans abertos com sucesso.");
}

void abrir_clan(char *clan_arq, short int numero)
{
	FILE *clan;
	char linha[600];
	struct char_data *tch;
	short int membros = 0, posicao = 0, nivel = 0;

	if(!(clan = fopen(clan_arq, "r")))
	{
		log("Arquivo de clan faltando. Favor verificar.");
		exit(1);
	}

	while(get_line(clan, linha))
	{
		if(!str_cmp(linha, "Nome:"))
		{
			get_line(clan, linha);
			strcpy(clans[numero].nome_clan, linha);
		}
		else if(!str_cmp(linha, "Posicoes:"))
		{
			while(get_line(clan, linha))
			{
				if(linha[0] == '~')
					break;

				strcpy(clans[numero].pos_nome[posicao], linha);
				posicao++;
			}
			clans[numero].numero_posicoes = posicao;
		}
		else if(!str_cmp(linha, "Membros:"))
		{
			for(membros = 0; membros < MAX_MEMBROS; membros++)
				clans[numero].membros[membros] = 0;

			membros = 0;
			while(get_line(clan, linha))
			{
				if(linha[0] == '~')
					break;

				clans[numero].membros[membros] = atoi(linha);
				membros++;
			}
			clans[numero].numero_membros = membros;
		}
		else if(!str_cmp(linha, "PosComandos:"))
		{
			for(nivel = 0; nivel < NUM_COMANDOS; nivel++)
				clans[numero].pos_comando[nivel] = clans[numero].numero_posicoes;

			nivel = 0;
			while(get_line(clan, linha))
			{
				if(linha[0] == '~')
					break;

				clans[numero].pos_comando[nivel] = atoi(linha);
				nivel++;
			}
		}
		else if(!str_cmp(linha, "Grana:"))
		{
			get_line(clan, linha);
			clans[numero].grana = atoi(linha);
		}
		else if(!str_cmp(linha, "Guerra:"))
		{
			get_line(clan, linha);
			clans[numero].em_guerra = atoi(linha);
		}
		else if(!str_cmp(linha, "Vitorias:"))
		{
			get_line(clan, linha);
			clans[numero].pkvit = atoi(linha);
		}
		else if(!str_cmp(linha, "Derrotas:"))
		{
			get_line(clan, linha);
			clans[numero].pkder = atoi(linha);
		}
		else if(!str_cmp(linha, "Rendicoes:"))
		{
			get_line(clan, linha); 
			clans[numero].rendicoes = atoi(linha);
		}
		else if(!str_cmp(linha, "Recusas:"))
		{
			get_line(clan, linha);
			clans[numero].recusas = atoi(linha);
		}
		else if(!str_cmp(linha, "Preco:"))
		{
			get_line(clan, linha);
			clans[numero].preco = atoi(linha);
		}
		else if(!str_cmp(linha, "Alianca:"))
		{
			get_line(clan, linha);
			clans[numero].alianca = atoi(linha);
		}
		else if(!str_cmp(linha, "Tempo:"))
		{
			get_line(clan, linha);
			clans[numero].tempo = atoi(linha);
		}
		else if(!str_cmp(linha, "Desafiando:"))
		{
			get_line(clan, linha);
			clans[numero].desafiando = atoi(linha);
		}
		else if(!str_cmp(linha, "Desafiado:"))
		{
			get_line(clan, linha);
			clans[numero].desafiado = atoi(linha);
		}
		else if(!str_cmp(linha, "Religion:"))
		{
			get_line(clan, linha);
			clans[numero].religion = atoi(linha);
		}
		else if(!str_cmp(linha, "Points:"))
		{
			get_line(clan, linha);
			clans[numero].points = atoi(linha);
		}
	}

	//atualizar poder
	for(membros = 0; membros < MAX_MEMBROS; membros++)
	{
		if(clans[numero].membros[membros] != 0)
		{
			CREATE(tch, struct char_data, 1);
			clear_char(tch);
			CREATE(tch->player_specials, struct player_special_data, 1);
			if (load_char(get_name_by_id(clans[numero].membros[membros]), tch) > -1)
				clans[numero].poder += qto_poder(tch);
			free_char(tch);
		}
	}

	fclose(clan);
}

void atualizar_index()
{
	FILE *index;
	short int clan = 0;

	if(!(index = fopen("clans/index", "w")))
		log("Index de clan faltando. Favor verificar.");

	for(; clan < numero_clans; clan++)
		fprintf(index, "%d\n", clans[clan].clan_num);

	fclose(index);
}

int posicao_clan(short int num_clan)
{
	short int pos;

	for(pos = 0; pos < numero_clans; pos++)
		if(clans[pos].clan_num == num_clan)
			return (pos);
	return (-1);
}

void salvar_clan(short int num_clan)
{
	FILE *clan;
	char clan_arq[30];
	short int membros, pos, posicao, nivel;

	pos = posicao_clan(num_clan);

	sprintf(clan_arq, "clans/%d.clan", num_clan);

	if(!(clan = fopen(clan_arq, "w")))
		log("Arquivo de clan faltando. Criando novo.");

	fprintf(clan, "Nome:\n%s\n", clans[pos].nome_clan);

	fprintf(clan, "Posicoes:\n");
	for(posicao = 0; posicao < clans[pos].numero_posicoes; posicao++)
		fprintf(clan, "%s\n", clans[pos].pos_nome[posicao]);
	fprintf(clan, "~\n");

	fprintf(clan, "Membros:\n");
	for(membros = 0; membros < MAX_MEMBROS; membros++)
		if(clans[pos].membros[membros] != 0)
			fprintf(clan, "%ld\n", clans[pos].membros[membros]);
	fprintf(clan, "~\n");

	fprintf(clan, "PosComandos:\n");
	for(nivel = 0; nivel < NUM_COMANDOS; nivel++)
		fprintf(clan, "%d\n", clans[pos].pos_comando[nivel]);
	fprintf(clan, "~\n");

	fprintf(clan, "Grana:\n%d\n", clans[pos].grana);
	fprintf(clan, "Poder:\n%d\n", clans[pos].poder);
	fprintf(clan, "Guerra:\n%d\n", clans[pos].em_guerra);
	fprintf(clan, "Vitorias:\n%d\n", clans[pos].pkvit);
	fprintf(clan, "Derrotas:\n%d\n", clans[pos].pkder);
	fprintf(clan, "Rendicoes:\n%d\n", clans[pos].rendicoes);
	fprintf(clan, "Recusas:\n%d\n", clans[pos].recusas);
	fprintf(clan, "Preco:\n%d\n", clans[pos].preco);
	fprintf(clan, "Alianca:\n%d\n", clans[pos].alianca);
	fprintf(clan, "Tempo:\n%d\n", clans[pos].tempo);
	fprintf(clan, "Desafiando:\n%d\n", clans[pos].desafiando);
	fprintf(clan, "Desafiado:\n%d\n", clans[pos].desafiado);
	fprintf(clan, "Religion:\n%d\n", clans[pos].religion);
	fprintf(clan, "Points:\n%d\n", clans[pos].points);
	fclose(clan);
}

short int tem_permissao(struct char_data *ch, short int comando)
{
	short int permissao = 0, clan;

	if(comando >= NUM_COMANDOS)
		return (permissao);

	if((clan = posicao_clan(GET_CLAN(ch))) == -1)
		return (permissao);

	if(clans[clan].pos_comando[comando] <= GET_CLAN_POS(ch))
		permissao = 1;

	return (permissao);
}

int qto_poder(struct char_data *ch)
{
	int poder = 0;
        if(!(PLR_FLAGGED(ch, PLR_DELETED)))
        {
        poder = (GET_REMORT(ch)*(LVL_IMMORT-1))+GET_LEVEL(ch);
	}

  return (poder);
        
}

short int membro_livre(short int clan)
{
	short int membro, klan;

	klan = posicao_clan(clan);
	for(membro = 0; membro < MAX_MEMBROS; membro++)
		if(clans[klan].membros[membro] == 0)
			return (membro);
	return (-1);
}

short int membro_pos(struct char_data *ch)
{
	short int membro, klan, id;

	klan = posicao_clan(GET_CLAN(ch));
	id = get_id_by_name(GET_NAME(ch));
	for(membro = 0; membro < MAX_MEMBROS; membro++)
		if(clans[klan].membros[membro] == id)
			return membro;
	return(-1);
}

short int remove_do_clan(struct char_data *ch)
{
	short int nclan = 0, pos;

	if((pos = membro_pos(ch)) == -1)
		return (0);

	nclan = posicao_clan(GET_CLAN(ch));

	clans[nclan].membros[pos] = 0;
	clans[nclan].numero_membros--;
	clans[nclan].poder -= qto_poder(ch);

	salvar_clan(GET_CLAN(ch));

	return (1);
}

void send_to_clan(struct char_data *ch, const char *messg)
{
  struct descriptor_data *i;

  if (messg == NULL)
    return;

  for (i = descriptor_list; i; i = i->next) {
    if (STATE(i) != CON_PLAYING)
      continue;
	if(PLR_FLAGGED(i->character, PLR_WRITING))
	  continue;
    if ((GET_CLAN(i->character) != GET_CLAN(ch)) || !GET_CLAN_POS(i->character))
      continue;

    SEND_TO_Q(messg, i);
  }
}

#define CLANMENSAGEM_INICIO \
"&WSeu clan foi criado com sucesso, porem, ainda faltam alguns ajustes.\r\n" \
"Use o comando &R'clan numposicoes' &We em seguida o comando &R'clan alteranomepos'&W\r\n" \
"para ajustar as posicoes do clan. Isso podera ser feito somente uma vez;\r\n" \
"portanto, nao erre. Os deuses nao tem culpa de sua incompetencia.&n\r\n"

//aqui comeca o sistema de clan
void cria_clan(char *nome, struct char_data *ch)
{
	struct char_data *lider;
	char nome_lider[50], clan_nome[500];
	short int num_mem, ult_num = 0, clan, nivel;

	if(!*nome)
	{
		send_to_char("Formato: clan criar <lider> <nome do clan>\r\n", ch);
		return;
	}

	half_chop(nome, nome_lider, clan_nome);

	if((lider = get_player_vis(ch, nome_lider, FIND_CHAR_ROOM)) == NULL)
	{
		send_to_char("O lider precisa estar no mesmo local que voce.\r\n", ch);
		return;
	}

	if(GET_CLAN(lider))
	{
		send_to_char("O lider ja possui clan.\r\n", ch);
		return;
	}

	if(!*clan_nome)
	{
		send_to_char("O clan precisa de um nome.\r\n", ch);
		return;
	}

	if(!numero_clans)
		CREATE(clans, struct estrutura_clan, 2);
	else
		RECREATE(clans, struct estrutura_clan, numero_clans + 2);

	strcpy(clans[numero_clans].nome_clan, clan_nome);

	for(clan = 0; clan < numero_clans; clan++)
		if(ult_num < clans[clan].clan_num)
			ult_num = clans[clan].clan_num;
	clans[numero_clans].clan_num = ult_num + 1;

	strcpy(clans[numero_clans].pos_nome[0], "Aprendiz");
	strcpy(clans[numero_clans].pos_nome[1], "Lider");
	clans[numero_clans].numero_posicoes = 2;

	clans[numero_clans].membros[0] = get_id_by_name(nome_lider);
	for(num_mem = 1; num_mem < MAX_MEMBROS; num_mem++)
		clans[numero_clans].membros[num_mem] = 0;
	clans[numero_clans].numero_membros = 1;

	for(nivel = 0; nivel < NUM_COMANDOS; nivel++)
		clans[numero_clans].pos_comando[nivel] = clans[numero_clans].numero_posicoes;

	clans[numero_clans].grana = 0;

	clans[numero_clans].poder = qto_poder(lider);
	clans[numero_clans].em_guerra = -1;
	clans[numero_clans].pkvit = 0;
	clans[numero_clans].pkder = 0;
	clans[numero_clans].recusas = 0;
	clans[numero_clans].rendicoes = 0;
	clans[numero_clans].preco = 0;
	clans[numero_clans].alianca = -1;
	clans[numero_clans].tempo = 390;
	clans[numero_clans].desafiando = -1;
	clans[numero_clans].desafiado = -1;
	clans[numero_clans].religion = GET_RELIGION(lider);
	clans[numero_clans].points = 0;
	//lider
	GET_CLAN(lider) = clans[numero_clans].clan_num;
	GET_CLAN_POS(lider) = clans[numero_clans].numero_posicoes;
	save_char(lider, lider->in_room);

	salvar_clan(clans[numero_clans++].clan_num);
	atualizar_index();

	sprintf(buf, "&WUma nova forca emerge, o clan %s &Wfoi fundado!&n\r\n", clan_nome);
	send_to_all(buf);

	send_to_char("Clan criado com sucesso!\r\n", ch);
	send_to_char(CLANMENSAGEM_INICIO, lider);
}
char *clan_religion(int number)
{
  static char buf[256];

  if (number == 1)
    strcpy(buf, "&CM&cuir&n");
  else if (number == 2)
    strcpy(buf, "&YI&ylma&wna&yteu&Yr&n");
  else if (number == 3)
    strcpy(buf, "&CS&ce&Cl&cu&Cn&ce&n");
  else if (number == 4)
    strcpy(buf, "&YH&Welm&n");
  else if (number == 5)
    strcpy(buf, "&YOghma&n");
  else if (number == 6)
    strcpy(buf, "&WT&wempus&n");
  else if (number == 7)
    strcpy(buf, "&WB&wa&Wn&we&n");
  else if (number == 8)
    strcpy(buf, "&MPa&mndo&Mra&n");
  else if (number == 9)
    strcpy(buf, "&RT&ra&wl&ro&Rs&n");    
  else
    strcpy(buf, "&w-&n");

  return (buf);
}
void lista_clans(char *clan, struct char_data *ch)
{
	struct char_data *tch;
	char buffer[MAX_INPUT_LENGTH], lider[1000], vice_lider[1000];
	short int nclan = 0, pos = 0, membros, temlider = 0, temvice = 0;

	if(!numero_clans)
	{
		send_to_char("&WNenhum clan domina &BW&bar&BD&bome &Wainda.&n\r\n", ch);
		return;
	}

	if(!*clan || !is_number(clan))
	{
		send_to_char("Formato: clan lista <numero>\r\n\r\n", ch);
		send_to_char("&WLista de &CClans&W:&n\r\n", ch);
		for(; nclan < numero_clans; nclan++)
		{
			sprintf(buffer, "      &W%2d. %s   &rPoder&W: &R%-8d&n\r\n", clans[nclan].clan_num,
				complete_char(clans[nclan].nome_clan, 30), clans[nclan].poder);
			send_to_char(buffer, ch);
		}
	}
	else
	{
		if((nclan = posicao_clan(atoi(clan))) == -1)
		{
			send_to_char("Numero de clan invalido.\r\n", ch);
			return;
		}

		sprintf(buffer, "&WListando &CClan&n %s&W.&n\r\n", clans[nclan].nome_clan);
		send_to_char(buffer, ch);

		sprintf(lider, "%s&W:&C", clans[nclan].pos_nome[clans[nclan].numero_posicoes - 1]);
		sprintf(vice_lider, "%s&W:&c", clans[nclan].pos_nome[clans[nclan].numero_posicoes - 2]);
		for(membros = 0; membros < MAX_MEMBROS; membros++)
		{
			if(clans[nclan].membros[membros] != 0)
			{
				CREATE(tch, struct char_data, 1);
				clear_char(tch);
				CREATE(tch->player_specials, struct player_special_data, 1);
				if (load_char(get_name_by_id(clans[nclan].membros[membros]), tch) > -1)
				{
					if(GET_CLAN_POS(tch) == clans[nclan].numero_posicoes)
					{
						temlider++;
						sprintf(lider + strlen(lider), " %s", GET_NAME(tch));
					}
					else if(GET_CLAN_POS(tch) == (clans[nclan].numero_posicoes - 1))
					{
						temvice++;
						sprintf(vice_lider + strlen(vice_lider), " %s", GET_NAME(tch));
					}
				}
				free_char(tch);
			}
		}

		if(!temlider)
			strcat(lider, " &cNenhum&n");

		if(!temvice)
			strcat(vice_lider, " &cNenhum&n");

		send_to_char("&CL&Wideres &We &CV&Wice-Lideres&W:&n\r\n", ch);
		sprintf(buffer, " &C%2d&n %s\r\n", temlider, lider);
		send_to_char(buffer, ch);
		sprintf(buffer, " &c%2d&n %s\r\n", temvice, vice_lider);
		send_to_char(buffer, ch);

		sprintf(buffer, "&CD&We &CP&Woder &R%d&W.&n\r\n", clans[nclan].poder);
		sprintf(buffer + strlen(buffer), "&CC&Wom &R%d &Cm&Wembros.&n\r\n", clans[nclan].numero_membros);
		strcat(buffer, "&CP&Wosicoes:&n ");
		send_to_char(buffer, ch);
		for(; pos < clans[nclan].numero_posicoes; pos++)
		{
			sprintf(buffer, "%s%s", clans[nclan].pos_nome[pos],
				(pos == (clans[nclan].numero_posicoes - 1) ? "&W.&n\r\n" : "&W,&n "));
			send_to_char(buffer, ch);
		}
		sprintf(buffer, "&YT&Wesouro &YA&Wproximado &y%d&W.&n\r\n", clans[nclan].grana);
		sprintf(buffer + strlen(buffer),"&CV&Witorias: &R%d &CD&Werrotas: &R%d &CRe&Wcusas: &R%d &CRe&Wndicoes: &R%d \r\n", clans[nclan].pkvit, clans[nclan].pkder, clans[nclan].recusas, clans[nclan].rendicoes);		
		sprintf(buffer + strlen(buffer), "&YC&Wusto de &YG&Wuerra: &y%d&n\r\n", clans[nclan].preco);
		send_to_char(buffer, ch);
		
		if(clans[nclan].em_guerra == -1)
		send_to_char("&CG&Wuerriando: &cNinguem&N.\r\n", ch); 
			 
		if(clans[nclan].em_guerra > 0)
		{
		sprintf(buf, "&CG&Wuerriando: %s&n\r\n", complete_char(clans[posicao_clan(clans[nclan].em_guerra)].nome_clan, 30));    	
		send_to_char(buf, ch);    	
		}
	
		if(clans[nclan].alianca == -1)
		send_to_char("&CA&Wliado: &cNenhum.\r\n", ch);
		
		if(clans[nclan].alianca > 0)
		{
		sprintf(buf, "&CA&Wliado: %s&n\r\n", complete_char(clans[posicao_clan(clans[nclan].alianca)].nome_clan, 30));    	
		send_to_char(buf, ch);    	
		}
		sprintf(buf, "&CR&Weligiao: %s \r\n", clan_religion(clans[nclan].religion));
		send_to_char(buf, ch);
		sprintf(buf, "&CC&Wlan &CP&Woints: &R%d&n \r\n", clans[nclan].points);
		send_to_char(buf, ch);		
		
	}
}
int aceita_ra_clan(int num1, int mode)
{
  
  switch (num1)
 {
      case  999:  // lawful good
        if(mode == 1) return 1 ;
        if(mode == 2) return 1 ;
        if(mode == 4) return 1 ;
        return 0 ;
      case  1000:
        if(mode == 1) return 1 ;
        if(mode == 2) return 1 ;
        if(mode == 3) return 1 ;
        return 0 ;
      case  994:
        if(mode == 2) return 1 ;
        if(mode == 3) return 1 ;
        if(mode == 6) return 1 ;
        return 0 ;
      case  3:
        if(mode == 1) return 1 ;
        if(mode == 4) return 1 ;
        if(mode == 7) return 1 ;
        return 0 ;
      case  5:
     if(mode == 4) return 1 ;
        if(mode == 5) return 1 ;
        if(mode == 6) return 1 ;
        return 0 ;
      case  7:
        if(mode == 3) return 1 ;
        if(mode == 6) return 1 ;
        if(mode == 9) return 1 ;
        return 0 ;
      case  -999:
       if(mode == 4) return 1 ;
        if(mode == 7) return 1 ;
        if(mode == 8) return 1 ;
        return 0 ;
      case  -1000:
        if(mode == 7) return 1 ;
        if(mode == 8) return 1 ;
        if(mode == 9) return 1 ;
        return 0 ;
      case  -994:
        if(mode == 6) return 1 ;
        if(mode == 8) return 1 ;
        if(mode == 9) return 1 ;
        return 0 ;
  }
 return 0 ;
}
void alistar_clan(char *clan, struct char_data *ch)
{
	short int nclan = 0;
	int mode;
	
	if(!*clan || !is_number(clan))
	{
		send_to_char("Formato: clan alistar <numero>\r\n", ch);
		return;
	}
	if(PLR_FLAGGED(ch, PLR_BANNED))
	{
		send_to_char("Voce esta irregular, comunique um God.\r\n", ch);
		return;
	}
	if(GET_CLAN(ch))
	{
		send_to_char("Voce ja possui um clan, nao seja traidor.\r\n", ch);
		return;
	}
	nclan = atoi(clan);
	mode = clans[posicao_clan(nclan)].religion;
	
	if(posicao_clan(nclan) == -1)
	{
		send_to_char("Numero de clan invalido.\r\n", ch);
		return;
	}
  	/*if(!aceita_ra_clan(GET_ALIGNMENT(ch), (mode)))
  	{
   	  send_to_char("Os Deuses desse Clan nao podem aceitar Voce!\r\n", ch);
    	  return;
  	}*/
	GET_CLAN(ch) = nclan;
	save_char(ch, ch->in_room);
	sprintf(buf, "&RCLAN INFO&W: &c%s &Wacaba de alistar no seu clan.&n\r\n", GET_NAME(ch));
	send_to_clan(ch, buf);
	sprintf(buf, "&WVoce se alistou no clan %s&W.&n\r\n", clans[posicao_clan(nclan)].nome_clan);
	send_to_char(buf, ch);
}

void posicoes_clan(char *argumento, struct char_data *ch)
{
	short int pos = 0, clan, nivel;

	if(!GET_CLAN(ch))
	{
		send_to_char("Comando nao disponivel.\r\n", ch);
		return;
	}

	if(!tem_permissao(ch, NUMPOSICOES))
	{
		send_to_char("Voce nao tem posicao suficiente para usar esse comando.\r\n", ch);
		return;
	}

	if(!*argumento || !is_number(argumento))
	{
		send_to_char("Formato: clan numposicoes <numero>\r\n", ch);
		return;
	}

	clan = posicao_clan(GET_CLAN(ch));
	if(clans[clan].numero_posicoes <= pos )
	{
		send_to_char("Voce Pode Apenas Adicionar Posicoes.\r\n", ch);
		return;
	}

	pos = atoi(argumento);
	if(pos <= 2 || pos > MAX_POSICOES)
	{
		sprintf(buf, "Quantidade de posicoes invalida. (2 < posicao < %d)\r\n", MAX_POSICOES);
		send_to_char(buf, ch);
		return;
	}

	strcpy(clans[clan].pos_nome[pos-1], clans[clan].pos_nome[clans[clan].numero_posicoes-1]);

	clans[clan].numero_posicoes = pos;
	for(pos = 1; pos < (clans[clan].numero_posicoes - 1); pos++)
		strcpy(clans[clan].pos_nome[pos], "Membro");

	for(nivel = 0; nivel < NUM_COMANDOS; nivel++)
		clans[clan].pos_comando[nivel] = clans[clan].numero_posicoes;

	salvar_clan(GET_CLAN(ch));

	GET_CLAN_POS(ch) = clans[clan].numero_posicoes;
	save_char(ch, ch->in_room);
	sprintf(buf, "&WSeu clan possuira &R%d&W posicoes.&n\r\n"
				 "&WEsse numero agora e &Rfixo&W e &Rnao podera&W mais ser alterado.&n\r\n", clans[clan].numero_posicoes);
	send_to_char(buf, ch);
}

void alterar_nomes_pos(char *argumento, struct char_data *ch)
{
	char posicao[MAX_INPUT_LENGTH], nome[51];
	short int pos = 0, clan = 0;

	if(!GET_CLAN(ch))
	{
		send_to_char("Comando nao disponivel.\r\n", ch);
		return;
	}

	if(!tem_permissao(ch, ALTERANOMEPOS))
	{
		send_to_char("Voce nao tem posicao suficiente para usar esse comando.\r\n", ch);
		return;
	}

	half_chop(argumento, posicao, nome);

	if(!*posicao || !*nome || !is_number(posicao))
	{
		send_to_char("Formato: clan alteranomepos <posicao> <nome>\r\n", ch);
		return;
	}

	clan = posicao_clan(GET_CLAN(ch));
	pos = atoi(posicao);
	if(pos < 1 || pos > clans[clan].numero_posicoes)
	{
		sprintf(buf, "Posicao invalida. (1 < posicao < %d)\r\n", clans[clan].numero_posicoes);
		send_to_char(buf, ch);
		return;
	}

	if(strlen(nome) > 49)
	{
		send_to_char("Nome muito extenso, se limite a 49 caracteres.\r\n", ch);
		return;
	}

	sprintf(buf, "&RCLAN INFO&W: A posicao &c'%s&c' &Wmudou de nome para &c'%s&c'&W.&n\r\n",
			clans[clan].pos_nome[pos - 1], nome);
	send_to_clan(ch, buf);

	strcpy(clans[clan].pos_nome[pos - 1], nome);
	salvar_clan(GET_CLAN(ch));

	sprintf(buf, "&WVoce alterou o nome da posicao &c%d &Wpara &c%s&W.&n\r\n", pos, nome);
	send_to_char(buf, ch);
}

void aceitar_membro(char *argumento, struct char_data *ch)
{
	struct char_data *jogador;
	short int clan = 0, livre;

	if(!GET_CLAN(ch))
	{
		send_to_char("Comando nao disponivel.\r\n", ch);
		return;
	}

	if(!tem_permissao(ch, CONVOCAR))
	{
		send_to_char("Voce nao tem posicao suficiente para usar esse comando.\r\n", ch);
		return;
	}

	if(!*argumento)
	{
		send_to_char("Formato: clan convocar <nome>\r\n", ch);
		return;
	}

	if((livre = membro_livre(GET_CLAN(ch))) == -1)
	{
		sprintf(buf, "Um clan pode ter no maximo %d membros.\r\n", MAX_MEMBROS);
		send_to_char(buf, ch);
		return;
	}

	if((jogador = get_player_vis(ch, argumento, FIND_CHAR_ROOM)) == NULL)
	{
		send_to_char("O jogador precisa estar no mesmo local que voce para ser convocado.\r\n", ch);
		return;
	}

	if(!GET_CLAN(jogador))
	{
		send_to_char("Este jogador nao tem interesse em nenhum clan no momento.\r\n", ch);
		return;
	}
	if(PLR_FLAGGED(jogador, PLR_BANNED))
	{
		send_to_char("Este Jogador esta Banido, Contacte um God.\r\n", ch);
		return;
	}

	if(GET_CLAN(jogador) != GET_CLAN(ch))
	{
		send_to_char("Este jogador ja se alistou em outro clan.\r\n", ch);
		return;
	}

	if(GET_CLAN_POS(jogador) > 0)
	{
		send_to_char("Este jogador ja faz parte do seu clan.\r\n", ch);
		return;
	}
	clan = posicao_clan(GET_CLAN(ch));

	clans[clan].numero_membros++;
	clans[clan].membros[livre] = get_id_by_name(argumento);
	clans[clan].poder += qto_poder(jogador);

	sprintf(buf, "&RCLAN INFO&W: &c%s &Wfoi convocado para o clan.&n\r\n", GET_NAME(jogador));
	send_to_clan(ch, buf);
	salvar_clan(GET_CLAN(jogador));

	//jogador
	GET_CLAN_POS(jogador) = 1;
	save_char(jogador, jogador->in_room);
	sprintf(buf, "Voce foi convocado para o clan %s.\r\n", clans[clan].nome_clan);
	send_to_char(buf, jogador);
}

void abandonar_clan(struct char_data *ch)
{
	if(!GET_CLAN(ch))
	{
		send_to_char("Voce nao possui clan, nao tem o que abandonar.\r\n", ch);
		return;
	}

	if(GET_CLAN_POS(ch))
	{
		remove_do_clan(ch);

		sprintf(buf, "&RCLAN INFO&W: &c%s &Wacaba de abandonar seu clan.&n\r\n", GET_NAME(ch));
		send_to_clan(ch, buf);
	}

	GET_CLAN(ch) = 0;
	GET_CLAN_POS(ch) = 0;
	SET_BIT(PLR_FLAGS(ch), PLR_BANNED);
	send_to_char("&WVoce abandonou seu clan, traidor.&n\r\n", ch);
}

void muda_pos_comando(char *argumento, struct char_data *ch)
{
	char comando[30], posicao[10];
	short int pos = 0, com, clan;

	if(!GET_CLAN(ch))
	{
		send_to_char("Comando nao disponivel.\r\n", ch);
		return;
	}

	if(!tem_permissao(ch, MUDAPOSCOMANDO))
	{
		send_to_char("Voce nao tem posicao suficiente para usar esse comando.\r\n", ch);
		return;
	}

	half_chop(argumento, comando, posicao);

	if(!*comando || !*posicao || !is_number(posicao))
	{
		send_to_char("Formato: clan mudaposcomando <comando> <posicao>\r\n", ch);
		return;
	}

	pos = atoi(posicao);
	clan = posicao_clan(GET_CLAN(ch));
	if(pos < 1 || pos > clans[clan].numero_posicoes)
	{
		sprintf(buf, "Posicao invalida. (1 < posicao < %d)\r\n", clans[clan].numero_posicoes);
		send_to_char(buf, ch);
		return;
	}

	//comando
	for(com = 0; com < NUM_COMANDOS; com++)
	{
		if(!str_cmp(comando, comandos[com].comando))
		{
			sprintf(buf, "&RCLAN INFO&W: O comando de clan &c'%s&c' &Wmudou de posicao.&n\r\n"
							 "&RCLAN INFO&W: Passou de &c%s &Wpara &c%s&W.&n\r\n",
					comandos[com].comando, clans[clan].pos_nome[clans[clan].pos_comando[com]-1],
					clans[clan].pos_nome[pos-1]);
			send_to_clan(ch, buf);
			clans[clan].pos_comando[com] = pos;
			salvar_clan(GET_CLAN(ch));
			sprintf(buf, "&WVoce alterou a posicao do comando &c%s&W.&n\r\n", comandos[com].comando);
			send_to_char(buf, ch);
			return;
		}
	}
	send_to_char("Comando de clan nao encontrado.\r\n", ch);
}

void promover_membro(char *argumento, struct char_data *ch)
{
	struct char_data *jogador;
	short int clan;

	if(!GET_CLAN(ch))
	{
		send_to_char("Comando nao disponivel.\r\n", ch);
		return;
	}

	if(!tem_permissao(ch, PROMOVER))
	{
		send_to_char("Voce nao tem posicao suficiente para usar esse comando.\r\n", ch);
		return;
	}

	if(!*argumento)
	{
		send_to_char("Formato: clan promover <nome>\r\n", ch);
		return;
	}

	if((jogador = get_player_vis(ch, argumento, FIND_CHAR_ROOM)) == NULL)
	{
		send_to_char("O jogador precisa estar no mesmo local que voce para ser promovido.\r\n", ch);
		return;
	}

	if((GET_CLAN(ch) != GET_CLAN(jogador)) || !GET_CLAN_POS(jogador))
	{
		send_to_char("Ele nao faz parte do seu clan.\r\n", ch);
		return;
	}

	if(GET_CLAN_POS(jogador) >= GET_CLAN_POS(ch))
	{
		send_to_char("Somente um superior pode fazer isso.\r\n", ch);
		return;
	}

	GET_CLAN_POS(jogador)++;
	save_char(jogador, jogador->in_room);

	clan = posicao_clan(GET_CLAN(jogador));
	sprintf(buf, "&WVoce foi promovido a &c%s&W.&n\r\n", clans[clan].pos_nome[GET_CLAN_POS(jogador)-1]);
	send_to_char(buf, jogador);
	sprintf(buf, "&RCLAN INFO&W: &c%s &Wfoi promovido a &c%s&W.&n\r\n", GET_NAME(jogador), clans[clan].pos_nome[GET_CLAN_POS(jogador)-1]);
	send_to_clan(jogador, buf);
}

void rebaixar_membro(char *argumento, struct char_data *ch)
{
	struct char_data *jogador;
	short int clan;

	if(!GET_CLAN(ch))
	{
		send_to_char("Comando nao disponivel.\r\n", ch);
		return;
	}

	if(!tem_permissao(ch, REBAIXAR))
	{
		send_to_char("Voce nao tem posicao suficiente para usar esse comando.\r\n", ch);
		return;
	}

	if(!*argumento)
	{
		send_to_char("Formato: clan rebaixar <nome>\r\n", ch);
		return;
	}

	if((jogador = get_player_vis(ch, argumento, FIND_CHAR_ROOM)) == NULL)
	{
		send_to_char("O jogador precisa estar no mesmo local que voce para ser rebaixado.\r\n", ch);
		return;
	}

	if((GET_CLAN(ch) != GET_CLAN(jogador)) || !GET_CLAN_POS(jogador))
	{
		send_to_char("Ele nao faz parte do seu clan.\r\n", ch);
		return;
	}

	if(GET_CLAN_POS(jogador) >= GET_CLAN_POS(ch))
	{
		send_to_char("Somente um superior pode fazer isso.\r\n", ch);
		return;
	}

	if((GET_CLAN_POS(jogador)-1) < 1)
	{
		send_to_char("Ele ja esta no nivel mais inferior do clan.\r\n", ch);
		return;
	}

	GET_CLAN_POS(jogador)--;
	save_char(jogador, jogador->in_room);

	clan = posicao_clan(GET_CLAN(jogador));
	sprintf(buf, "&WVoce foi rebaixado a &c%s&W.&n\r\n", clans[clan].pos_nome[GET_CLAN_POS(jogador)-1]);
	send_to_char(buf, jogador);
	sprintf(buf, "&RCLAN INFO&W: &c%s &Wfoi rebaixado a &c%s&W.&n\r\n", GET_NAME(jogador), clans[clan].pos_nome[GET_CLAN_POS(jogador)-1]);
	send_to_clan(jogador, buf);
}

void expulsar_membro(char *argumento, struct char_data *ch)
{
	struct char_data *jogador;

	if(!GET_CLAN(ch))
	{
		send_to_char("Comando nao disponivel.\r\n", ch);
		return;
	}

	if(!tem_permissao(ch, EXPULSAR))
	{
		send_to_char("Voce nao tem posicao suficiente para usar esse comando.\r\n", ch);
		return;
	}

	if(!*argumento)
	{
		send_to_char("Formato: clan expulsar <nome>\r\n", ch);
		return;
	}

	if((jogador = get_player_vis(ch, argumento, FIND_CHAR_ROOM)) == NULL)
	{
		send_to_char("O jogador precisa estar no mesmo local que voce para ser rebaixado.\r\n", ch);
		return;
	}

	if((GET_CLAN(ch) != GET_CLAN(jogador)) || !GET_CLAN_POS(jogador))
	{
		send_to_char("Ele nao faz parte do seu clan.\r\n", ch);
		return;
	}

	if(GET_CLAN_POS(jogador) >= GET_CLAN_POS(ch))
	{
		send_to_char("Somente um superior pode fazer isso.\r\n", ch);
		return;
	}

	remove_do_clan(jogador);

	sprintf(buf, "&RCLAN INFO&W: &c%s &Wacaba de ser expulso do clan.&n\r\n", GET_NAME(jogador));
	send_to_clan(jogador, buf);

	GET_CLAN(jogador) = 0;
	GET_CLAN_POS(jogador) = 0;
	SET_BIT(PLR_FLAGS(jogador), PLR_BANNED);
	send_to_char("&WVoce foi expulso do clan.&n\r\n", jogador);
}

void send_to_membros_clan(struct char_data *ch, short int posicao, const char *messg)
{
  struct descriptor_data *i;

  if (messg == NULL)
    return;

  for (i = descriptor_list; i; i = i->next)
  {
    if (STATE(i) != CON_PLAYING)
      continue;
	if(ch == i->character)
	  continue;
	if(PLR_FLAGGED(i->character, PLR_WRITING) ||
		ROOM_FLAGGED(i->character->in_room, ROOM_SOUNDPROOF))
	  continue;
	if (PRF2_FLAGGED(i->character, PRF2_CLAN_SEMFALAR))
	  continue;
    if ((GET_CLAN(i->character) != GET_CLAN(ch)) || !GET_CLAN_POS(i->character))
      continue;
	if(GET_CLAN_POS(i->character) < posicao)
	  continue;

    SEND_TO_Q(messg, i);
  }
}

void falar_clan(char *argumento, struct char_data *ch)
{
	char posicao[MAX_INPUT_LENGTH], fala[MAX_INPUT_LENGTH];
	short int pos = 0, clan = 0;

	if(!GET_CLAN(ch))
	{
		send_to_char("Comando nao disponivel.\r\n", ch);
		return;
	}

	if(!tem_permissao(ch, FALAR))
	{
		send_to_char("Voce nao tem posicao suficiente para usar esse comando.\r\n", ch);
		return;
	}

	half_chop(argumento, posicao, fala);

	if(!*posicao && !*fala)
	{
		if (PRF2_FLAGGED(ch, PRF2_CLAN_SEMFALAR))
		{
			REMOVE_BIT(PRF2_FLAGS(ch), PRF2_CLAN_SEMFALAR);
			send_to_char("&WVoce &Cabriu &Wo canal de clan.\r\n", ch);
			return;
		}
		else
		{
			SET_BIT(PRF2_FLAGS(ch), PRF2_CLAN_SEMFALAR);
			send_to_char("&WVoce &Rfechou &Wo canal de clan.\r\n", ch);
			return;
		}
	}

	if(!*posicao || !*fala || !is_number(posicao))
	{
		send_to_char("Formato: clan falar <posicao> <texto>\r\n", ch);
		return;
	}

	clan = posicao_clan(GET_CLAN(ch));
	pos = atoi(posicao);
	if(pos < 0 || pos > clans[clan].numero_posicoes)
	{
		sprintf(buf, "Posicao invalida. (0 < posicao < %d)\r\n", clans[clan].numero_posicoes);
		send_to_char(buf, ch);
		return;
	}

	sprintf(buf2, " (%s%s&W)", (pos && pos < clans[clan].numero_posicoes ? "+" : ""),
			(pos ? clans[clan].pos_nome[pos - 1] : "todos"));

	sprintf(buf, "&C%s &Wfala para o clan%s, '%s&W'.\r\n", GET_NAME(ch), buf2, fala);
	send_to_membros_clan(ch, pos, buf);

	sprintf(buf, "&CVoce &Wfala para o clan%s, '%s&W'.\r\n", buf2, fala);
	send_to_char(buf, ch);
}

void nome_maiusculo(char *saida, char *entrada)
{
	strcpy(saida, entrada);
	saida[0] = UPPER(saida[0]);
}

void membros_clan(char *argumento, struct char_data *ch)
{
	char nome[30];
	short int membros, clan, num = 0;

	/*if(GET_CLAN(ch)) //tirar ! qnd der problema no clan membros
	{
		send_to_char("Comando nao disponivel.\r\n", ch);
		return;
	}*/

	if(!tem_permissao(ch, MEMBROS) /*|| tem_permissao(ch, MEMBROS)*/)
	{ sprintf(buf, "(GC) %s digitou clan membros", GET_NAME(ch));
      mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
		send_to_char("Voce nao tem posicao suficiente para usar esse comando.\r\n", ch);
		return;
	}

	clan = posicao_clan(GET_CLAN(ch));
	strcpy(buf, "\r\n&WLista de &Cmembros&W:\r\n&c");
	for(membros = 0; membros < MAX_MEMBROS; membros++)
		if(clans[clan].membros[membros] != 0)
		{
		  if(!(PLR_FLAGGED(ch, PLR_DELETED)))
        		{	
			num++;
			nome_maiusculo(nome, get_name_by_id(clans[clan].membros[membros]));
			sprintf(buf + strlen(buf), "%17s%s", nome, (!(num % 4) ? "\r\n" : " "));
		        }
		}
	if(num % 4)
		strcat(buf, "\r\n");
	send_to_char(buf, ch);
	//send_to_char("Comando Desativado\r\n", ch);
}

void clan_jogando(struct char_data *ch)
{
	struct descriptor_data *i;
	char buffer[MAX_INPUT_LENGTH];
	short int clan, contar = 0;

	clan = posicao_clan(GET_CLAN(ch));

	sprintf(buffer, "&WMembros do %s &cconectados&W:&n\r\n\r\n", clans[clan].nome_clan);
	send_to_char(buffer, ch);

	for (i = descriptor_list; i; i = i->next)
	{
		if (STATE(i) != CON_PLAYING)
			continue;
		if ((GET_CLAN(i->character) != GET_CLAN(ch)) || !GET_CLAN_POS(i->character))
			continue;
		if(!CAN_SEE(ch, i->character))
			continue;
		sprintf(buffer, "&y%20s&W : %s &W[&cLevel&W: &C%d &mRemort&W: &M%d&W]&n\r\n",
			GET_NAME(i->character),	clans[clan].pos_nome[GET_CLAN_POS(i->character) - 1],
			GET_LEVEL(i->character), GET_REMORT(i->character));
		send_to_char(buffer, ch);
		contar++;
	}
	sprintf(buffer, "\r\n&WVoce localizou &R%d &Wmembros do clan.&n\r\n", contar);
	send_to_char(buffer, ch);
}

void depositar_clan(char *argumento, struct char_data *ch)
{
	short int clan;
	int grana;

	if (!GET_CLAN(ch))
	{
		send_to_char("Comando nao disponivel.\r\n", ch);
		return;
	}

	if (!tem_permissao(ch, DEPOSITAR))
	{
		send_to_char("Voce nao tem posicao suficiente para usar esse comando.\r\n", ch);
		return;
	}

	if (!*argumento)
	{
		send_to_char("Formato: clan depositar <grana/mineral>\r\n", ch);
		return;
	}
	
         
	grana = atoi(argumento);

	if (grana < 1)
	{
		send_to_char("Que numero hein? Nao procure bugs!\r\n", ch);
		return;
	}

	if (GET_GOLD(ch) < grana)
	{
		send_to_char("Voce nao tem essa quantia.\r\n", ch);
		return;
	}

	clan = posicao_clan(GET_CLAN(ch));

	GET_GOLD(ch) -= grana;
	save_char(ch, ch->in_room);
	sprintf(buf, "Voce depositou %s no clan.\r\n", add_points(grana));
	send_to_char(buf, ch);

	clans[clan].grana += grana;
	sprintf(buf, "&RCLAN INFO&W: &c%s &Wdepositou %s no clan.&n\r\n",
				 GET_NAME(ch), add_points(grana));
	send_to_clan(ch, buf);
	salvar_clan(GET_CLAN(ch));
}

void sacar_clan(char *argumento, struct char_data *ch)
{
	short int clan;
	int grana;

	if(!GET_CLAN(ch))
	{
		send_to_char("Comando nao disponivel.\r\n", ch);
		return;
	}

	if(!tem_permissao(ch, SACAR))
	{
		send_to_char("Voce nao tem posicao suficiente para usar esse comando.\r\n", ch);
		return;
	}

	if(!*argumento || !is_number(argumento))
	{
		send_to_char("Formato: clan sacar <grana>\r\n", ch);
		return;
	}

	grana = atoi(argumento);
	clan = posicao_clan(GET_CLAN(ch));

	if(grana < 1)
	{
		send_to_char("Que numero hein? Nao procure bugs!\r\n", ch);
		return;
	}

	if(clans[clan].grana < grana)
	{
		send_to_char("O clan nao tem essa quantia.\r\n", ch);
		return;
	}

	clans[clan].grana -= grana;
	salvar_clan(GET_CLAN(ch));
	sprintf(buf, "&RCLAN INFO&W: &c%s &Wsacou %s do clan.&n\r\n",
				 GET_NAME(ch), add_points(grana));
	send_to_clan(ch, buf);

	GET_GOLD(ch) += grana;
	save_char(ch, ch->in_room);
	sprintf(buf, "Voce sacou %s no clan.\r\n", add_points(grana));
	send_to_char(buf, ch);
}

short int ativa_comando(char *komando, char *buffer, struct char_data *ch)
{
	short int com;

	for(com = 0; com < NUM_COMANDOS; com++)
		if(!str_cmp(komando, comandos[com].comando))
		{
			(*comandos[com].procedimento_comando)(buffer, ch);
			return (1);
		}
	return (0);
}

#define COMANDOS_CLAN \
"&cConvencionais&W:&n\r\n" \
"&c lista          alistar          abandonar         npc&n\r\n"
#define COMANDOS_SOH_CLAN \
"&c jogando                   &n\r\n\r\n"

#define COMANDOS_GOD \
"&cPara deuses&C: &Rcriar&n\r\n"

void lista_comandos(struct char_data *ch)
{
	char buffer[MAX_INPUT_LENGTH], texto[1000];
	short int komando, klanp = 0;

	strcpy(buffer, "&WLista de comandos de Clan:&n\r\n\r\n");
	strcat(buffer, COMANDOS_CLAN);
	send_to_char(buffer, ch);

	if(GET_CLAN(ch) && GET_CLAN_POS(ch))
	{
		send_to_char(COMANDOS_SOH_CLAN, ch);

		klanp = posicao_clan(GET_CLAN(ch));

		send_to_char("&CEspeciais&W:&n\r\n", ch);
		for(komando = 0; komando < NUM_COMANDOS; komando++)
		{
			sprintf(texto, "&C%s&W(%s&W)", comandos[komando].comando,
					clans[klanp].pos_nome[clans[klanp].pos_comando[komando] - 1]);
			sprintf(buffer, " %s%s", complete_char(texto, 25),
						((komando+1) % 3 ? "" : "\r\n"));
			send_to_char(buffer, ch);
		}
		if(komando % 3)
			send_to_char("\r\n", ch);
	}

	if(GET_LEVEL(ch) >= CLAN_GOD)
		send_to_char(COMANDOS_GOD, ch);
}

ACMD(do_clan)
{
	char buffer[MAX_INPUT_LENGTH], buffer2[MAX_INPUT_LENGTH];

	half_chop(argument, buffer, buffer2);

	if (!*buffer)
		lista_comandos(ch);
	else if (!str_cmp(buffer, "lista"))
		lista_clans(buffer2, ch);
	else if (!str_cmp(buffer, "alistar"))
		alistar_clan(buffer2, ch);
	else if (!str_cmp(buffer, "abandonar"))
		abandonar_clan(ch);
	else if (!str_cmp(buffer, "jogando") && GET_CLAN_POS(ch))
		clan_jogando(ch);
	else if (!str_cmp(buffer, "criar") && GET_LEVEL(ch) >= CLAN_GOD)
		cria_clan(buffer2, ch);
	else if (ativa_comando(buffer, buffer2, ch))
		return;
	else
		send_to_char("Comando nao encontrado.\r\n", ch);
}

void guerra_clan (char *argumento, struct char_data *ch)
{

      short int clan;

	if (!GET_CLAN(ch))
	{
		send_to_char("Comando nao disponivel.\r\n", ch);
		return;
	}

	if (!tem_permissao(ch, GUERRA))
	{
		send_to_char("Voce nao tem posicao suficiente para usar esse comando.\r\n", ch);
		return;
	}

	if (!*argumento || !is_number(argumento))
	{
		send_to_char("Formato: clan guerra <clan>\r\n", ch);
		return;
	}

                
        clan = atoi(argumento);

	if (posicao_clan(clan) == -1)
	{
		send_to_char("Numero de clan invalido.\r\n", ch);
		return;
	}
	if (clan == GET_CLAN(ch))
	{
		send_to_char("Seu proprio Clan?\r\n", ch);
		return;
	}
	
	if (clans[posicao_clan(GET_CLAN(ch))].em_guerra == -2)
	{
		send_to_char("Voce ja enviou o intimato de Guerra. Aguarde", ch);
		return;
	}	
	if (clans[posicao_clan(clan)].em_guerra == -3)
	{
		send_to_char("Esse CLan Recebeu um Intimato de outro Clan. Aguarde", ch);
		return;
	}

clans[posicao_clan(GET_CLAN(ch))].em_guerra = -2;
clans[posicao_clan(GET_CLAN(ch))].desafiando = clan;
clans[posicao_clan(clan)].desafiado = GET_CLAN(ch);
clans[posicao_clan(clan)].em_guerra = -3;
clans[posicao_clan(clan)].tempo = 120;
sprintf(buf, "&RCLAN INFO&w: &c%s&n &ndeclara guerra a &c%s&n!\r\n", clans[posicao_clan(GET_CLAN(ch))].nome_clan, clans[posicao_clan(clan)].nome_clan);
send_to_all(buf);
salvar_clan(GET_CLAN(ch));
salvar_clan(clan);
return;
}

void aceitar_guerra(char *argumento, struct char_data *ch)
{
	short int clan;
	
	if (!GET_CLAN(ch))
	{
		send_to_char("Comando nao disponivel.\r\n", ch);
		return;
	}

	if (!tem_permissao(ch, ACEITAR))
	{
		send_to_char("Voce nao tem posicao suficiente para usar esse comando.\r\n", ch);
		return;
	}

	if (!*argumento || !is_number(argumento))
	{
		send_to_char("Formato: clan guerra <clan>\r\n", ch);
		return;
	}
	
	clan = atoi(argumento);
	if (clans[posicao_clan(GET_CLAN(ch))].em_guerra == -2)
	{
		send_to_char("Aceitar Seu proprio Pedido?!\r\n",ch);
		return;
	}		
	if (clans[posicao_clan(clan)].em_guerra > 0)
	{
		send_to_char("Esta Clan ja esta em guerra.\r\n", ch);
		return;
	}
	
	if (clans[posicao_clan(clan)].em_guerra == -1)
	{
		send_to_char("Esta Clan nao esta em guerra.\r\n", ch);
		return;
	}
	if (posicao_clan(clan) == -1)
	{
		send_to_char("Numero de clan invalido.\r\n", ch);
		return;
	}
	
	if (clan == GET_CLAN(ch))
	{
		send_to_char("Guerra Interna no Seu Clan??.\r\n", ch);
		return;
	}

	clans[posicao_clan(GET_CLAN(ch))].em_guerra = clan;
	clans[posicao_clan(clans[posicao_clan(GET_CLAN(ch))].alianca)].em_guerra = clan;
	clans[posicao_clan(clan)].em_guerra = GET_CLAN(ch);
	clans[posicao_clan(clans[posicao_clan(clan)].alianca)].em_guerra = GET_CLAN(ch); 
	clans[posicao_clan(GET_CLAN(ch))].desafiando = -1;
	clans[posicao_clan(GET_CLAN(ch))].desafiado = -1;
	clans[posicao_clan(clan)].desafiado = -1;
	clans[posicao_clan(clan)].desafiando = -1;
	clans[posicao_clan(GET_CLAN(ch))].tempo = 390;
	sprintf(buf, "&RCLAN INFO&w: &c%s &naceita guerriar com &c%s&n!\r\n", clans[posicao_clan(GET_CLAN(ch))].nome_clan, clans[posicao_clan(clan)].nome_clan);
	send_to_all(buf);
	salvar_clan(GET_CLAN(ch));
	salvar_clan(clan);

}
void recusar_guerra(char *argumento, struct char_data *ch)
{	
	short int clan;
		
	if (!GET_CLAN(ch))
	{
		send_to_char("Comando nao disponivel.\r\n", ch);
		return;
	}

	if (!tem_permissao(ch, RECUSAR))
	{
		send_to_char("Voce nao tem posicao suficiente para usar esse comando.\r\n", ch);
		return;
	}

	if (!*argumento || !is_number(argumento))
	{
		send_to_char("Formato: clan recusar <clan>\r\n", ch);
		return;
	}
	
	clan = atoi(argumento);
	if (clans[posicao_clan(GET_CLAN(ch))].em_guerra == -2)
	{
		send_to_char("Recusar Seu proprio Pedido?!\r\n",ch);
		return;
	}

	/*if (clans[posicao_clan(clan)].em_guerra != -1)
	{
		send_to_char("Esta Clan ja esta em guerra.\r\n", ch);
		return;
	}*/
	
	if (clans[posicao_clan(clan)].em_guerra == -1)
	{
		send_to_char("Esta Clan nao esta em guerra.\r\n", ch);
		return;
	}
	if (posicao_clan(clan) == -1)
	{
		send_to_char("Numero de clan invalido.\r\n", ch);
		return;
	}
			
	if (clan == GET_CLAN(ch))
	{
		send_to_char("Guerra Interna no Seu Clan??.\r\n", ch);
		return;
	}

	clans[posicao_clan(GET_CLAN(ch))].em_guerra = -1;
	clans[posicao_clan(clan)].em_guerra = -1;
	clans[posicao_clan(GET_CLAN(ch))].desafiando = -1;
	clans[posicao_clan(GET_CLAN(ch))].desafiado = -1;
	clans[posicao_clan(clan)].desafiado = -1;
	clans[posicao_clan(clan)].desafiando = -1;
	clans[posicao_clan(GET_CLAN(ch))].recusas++;
	clans[posicao_clan(GET_CLAN(ch))].tempo = 390;
	sprintf(buf, "&RCLAN INFO&w: &c%s &nrecusa o pedido de guerra de &c%s&n!\r\n", clans[posicao_clan(GET_CLAN(ch))].nome_clan, clans[posicao_clan(clan)].nome_clan);
	send_to_all(buf);
	salvar_clan(GET_CLAN(ch));
	salvar_clan(clan);

}
void status_clan(char *argumento, struct char_data *ch)
{
   short int clan, nclan;
   int tmp;

   
  tmp = atoi(argumento);
   
   if (*argumento)
	{
		send_to_char("Formato: clan status\r\n", ch);
		return;
	}
	
   if (!GET_CLAN(ch))
   {
   	send_to_char("Comando Nao disponivel",ch);
   	return;
   }
   
   clan = clans[posicao_clan(GET_CLAN(ch))].em_guerra;
   nclan = posicao_clan(GET_CLAN(ch));
        
   sprintf(buf, "\r\n");
   sprintf(buf + strlen(buf),"&cClan: &n%-30s\r\n", complete_char(clans[nclan].nome_clan, 30));	
   if (clan == -3)
   {
   sprintf(buf + strlen(buf),"&cStatus: &nDesafiado por &c%s&n. Seu Clan tem &g%d&n horas para responder\r\n", complete_char(clans[posicao_clan(clans[nclan].desafiado)].nome_clan, 12), clans[nclan].tempo);
   send_to_char(buf, ch);
   return;
   }
   if (clan == -2)
   {
   sprintf(buf + strlen(buf), "&cStatus: &nDesafiou &c%s&n. Eles tem &g%d&n horas para responder \r\n", complete_char(clans[posicao_clan(clans[nclan].desafiando)].nome_clan, 12), clans[posicao_clan(clans[nclan].desafiando)].tempo);  
   send_to_char(buf, ch);
   return;
   }
      if (clan > 0)
   {
   sprintf(buf + strlen(buf),"&cStatus: &rEm Guerra.\r\n");	
   sprintf(buf + strlen(buf),"&cVitorias: &r%d &cDerrotas: &r%d &cRecusas: &r%d &cRendicoes: &r%d \r\n", clans[nclan].pkvit, clans[nclan].pkder, clans[nclan].recusas, clans[nclan].rendicoes);	
   if (clans[nclan].alianca > 0)
   {
   sprintf(buf + strlen(buf),"&cAliado:&n%s \r\n", complete_char(clans[posicao_clan(clans[nclan].alianca)].nome_clan, 30));
   }
   else 
   {
   sprintf(buf + strlen(buf),"&cAliados: Nenhum\r\n");
   }
   sprintf(buf + strlen(buf),"&cCusto da Guerra: &Y%d\r\n", clans[nclan].preco);
   sprintf(buf + strlen(buf),"\r\n&CAdversario:&n %-10s\r\n", complete_char(clans[posicao_clan(clan)].nome_clan, 30));	
   if (clans[posicao_clan(clan)].alianca > 0)
   {
   sprintf(buf + strlen(buf),"&CAliado:&n %s\r\n", complete_char(clans[posicao_clan(clans[posicao_clan(clan)].alianca)].nome_clan, 30));
   }
   else 
   {
   sprintf(buf + strlen(buf),"&CAliados: Nenhum\r\n");
   }
   sprintf(buf + strlen(buf),"&CVitorias: &r%d &cDerrotas: &r%d &cRecusas: &r%d &cRendicoes: &r%d \r\n", clans[posicao_clan(clan)].pkvit, clans[posicao_clan(clan)].pkder, clans[posicao_clan(clan)].recusas, clans[posicao_clan(clan)].rendicoes );	
   sprintf(buf + strlen(buf),"&CCusto da Guerra: &Y%d\r\n", clans[posicao_clan(clan)].preco);
   }

   else
   sprintf(buf + strlen(buf),"&cStatus: &rEm Paz\r\n"); 
   send_to_char(buf, ch); 
   return;
}	
void serender_guerra(char *argumento, struct char_data *ch)
{
	short int clan;
	int custo;
	
	if (!GET_CLAN(ch))
	{
		send_to_char("Comando nao disponivel.\r\n", ch);
		return;
	}

	if (!tem_permissao(ch, SERENDER))
	{
		send_to_char("Voce nao tem posicao suficiente para usar esse comando.\r\n", ch);
		return;
	}

	if (!*argumento || !is_number(argumento))
	{
		send_to_char("Formato: clan serender <clan>\r\n", ch);
		return;
	}
	
	clan = atoi(argumento);
	custo = clans[posicao_clan(clan)].preco;
	
	if (clans[posicao_clan(GET_CLAN(ch))].em_guerra == -2)
	{
		send_to_char("Render-se? Comece uma Guerra e entao se renda!\r\n",ch);
		return;
	}
	if (clans[posicao_clan(GET_CLAN(ch))].em_guerra == -3)
	{
		send_to_char("Render-se? Comece uma Guerra e entao se renda!\r\n",ch);
		return;
	}
	if (clans[posicao_clan(clan)].em_guerra == -1)
	{
		send_to_char("Esta Clan nao esta em guerra.\r\n", ch);
		return;
	}
	
	if (clan == GET_CLAN(ch))
	{
		send_to_char("Esse e o seu proprio Clan\r\n", ch);
		return;
	}	
	
	if (posicao_clan(clan) == -1)
	{
		send_to_char("Numero de clan invalido.\r\n", ch);
		return;
	}
	if(clans[posicao_clan(GET_CLAN(ch))].grana < clans[posicao_clan(clan)].preco)
	{
		send_to_char("Voce nao tem Fundos para isso.\r\n", ch);
		return;
	}
	
	//clan rendido
	clans[posicao_clan(GET_CLAN(ch))].em_guerra = -1;
	clans[posicao_clan(GET_CLAN(ch))].pkder += 3;
	clans[posicao_clan(GET_CLAN(ch))].grana -= custo;
	clans[posicao_clan(GET_CLAN(ch))].rendicoes++;
	//aliado do clan rendido
	clans[posicao_clan(clans[posicao_clan(GET_CLAN(ch))].alianca)].em_guerra = -1;
	clans[posicao_clan(clans[posicao_clan(GET_CLAN(ch))].alianca)].pkder += 3;
	clans[posicao_clan(clans[posicao_clan(GET_CLAN(ch))].alianca)].grana -= custo;
	clans[posicao_clan(clans[posicao_clan(GET_CLAN(ch))].alianca)].rendicoes++;
	
	//clan vitorioso
	clans[posicao_clan(clan)].em_guerra = -1;
	clans[posicao_clan(clan)].pkvit += 3;
	clans[posicao_clan(clan)].grana += custo;
	//aliado clan vitorioso
	clans[posicao_clan(clans[posicao_clan(clan)].alianca)].em_guerra = -1;
	clans[posicao_clan(clans[posicao_clan(clan)].alianca)].pkvit += 3;
	clans[posicao_clan(clans[posicao_clan(clan)].alianca)].grana += custo;	
		
	sprintf(buf, "&RCLAN INFO&w: &c%s&n &nse rende. &c%s&n tem o poder em Wardome&n!\r\n", clans[posicao_clan(GET_CLAN(ch))].nome_clan, clans[posicao_clan(clan)].nome_clan);
	send_to_all(buf);
	clans[posicao_clan(GET_CLAN(ch))].alianca = -1;
	clans[posicao_clan(clans[posicao_clan(GET_CLAN(ch))].alianca)].alianca = -1;
	clans[posicao_clan(clan)].alianca = -1;
	clans[posicao_clan(clans[posicao_clan(clan)].alianca)].alianca = -1;
	salvar_clan(GET_CLAN(ch));
	salvar_clan(clan);
}

void custo_guerra(char *argumento, struct char_data *ch)
{
	int custo;
	
	if(!GET_CLAN(ch))
	{
		send_to_char("Comando nao disponivel.\r\n", ch);
		return;
	}

	if(!tem_permissao(ch, CUSTO))
	{
		send_to_char("Voce nao tem posicao suficiente para usar esse comando.\r\n", ch);
		return;
	}

	if(!*argumento || !is_number(argumento))
	{
		send_to_char("Formato: clan custo <valor>\r\n", ch);
		return;
	}
	
	if(clans[posicao_clan(GET_CLAN(ch))].preco > 0)
	{
		send_to_char("O custo de guerra so pode ser alterado uma vez.\r\n", ch);
		return;
	}
	custo = atoi(argumento);
	clans[posicao_clan(GET_CLAN(ch))].preco = custo;
		
		
	if(custo < 0)
	{
		send_to_char("Custo de Guerra 0? Isso nao Existe\r\n", ch);
		return;
	}	
	
	if(custo > 20000000)
	{
		send_to_char("Valor Maximo 20Megas.\r\n", ch);
		return;
	}
	
	sprintf(buf, "&RCLAN INFO&W: &c%s &Wmudou o custo de guerra para &c%d&n.\r\n", GET_NAME(ch), custo);
	send_to_clan(ch, buf);
	salvar_clan(GET_CLAN(ch));
}
void custo_clan(void) // adicionado ao HeartBeat(comm.c), CLan Custo recolhido de hora em hora Real.
{ 
  short int clan = 0;
  struct char_data *i, *next_char;
  
      
    for(; clan < numero_clans; clan++)
    {
       if (clans[posicao_clan(clan)].em_guerra == -3)
	  { 
	   sprintf(buf, "&RCLAN INFO&W: &c%s&n tem &g%d&n horas para Aceitar/Recusar o intimato de Guerra\r\n",clans[posicao_clan(clan)].nome_clan, clans[posicao_clan(clan)].tempo);
	   send_to_all(buf);
	   clans[posicao_clan(clan)].tempo--;
	   salvar_clan(clan);
	  } 
	  if (clans[posicao_clan(clan)].tempo == 0 && 
	     (clans[posicao_clan(clan)].em_guerra == -3 ||
	      clans[posicao_clan(clan)].em_guerra == -2))
	  { 
	   sprintf(buf, "&RCLAN INFO&W: %s nao enviou uma resposta de Guerra. Rendicao Automatica\r\n",clans[posicao_clan(clan)].nome_clan);
	   send_to_all(buf);
	   
           clans[posicao_clan(clans[posicao_clan(clan)].desafiado)].em_guerra = -1;
           clans[posicao_clan(clans[posicao_clan(clan)].desafiado)].desafiando = -1;
	   salvar_clan(clans[posicao_clan(clans[posicao_clan(clan)].desafiado)].clan_num);
	   clans[posicao_clan(clans[posicao_clan(clan)].desafiado)].desafiado= -1;
	   
	   clans[posicao_clan(clan)].tempo = 390;
	   clans[posicao_clan(clan)].grana -= 100000;
	   clans[posicao_clan(clan)].rendicoes++;
	   clans[posicao_clan(clan)].em_guerra = -1;
	   clans[posicao_clan(clan)].desafiado= -1;
	   clans[posicao_clan(clan)].desafiando = -1;
	   salvar_clan(clan);
	   return;
	  } 
	if (clans[posicao_clan(clan)].clan_num > 0)
	{
	 if (clans[posicao_clan(clan)].clan_num < 30)
	 {	 
	  if (clans[posicao_clan(clan)].em_guerra > 0)
	  {
	   if (clans[posicao_clan(clan)].grana <= 0) 
	    { 
 
      	       	 sprintf(buf, "&RCLAN INFO&W: &c%s&n, por falta de recurso monetario, perde a guerra\r\n", clans[posicao_clan(clan)].nome_clan); 
		 send_to_all(buf);
		 
	 	 // Clan Aliado Vitorioso
	 	 clans[posicao_clan(clans[posicao_clan(clans[posicao_clan(clan)].em_guerra)].alianca)].em_guerra = -1;
	 	 clans[posicao_clan(clans[posicao_clan(clans[posicao_clan(clan)].em_guerra)].alianca)].pkvit += 3;
	 	 salvar_clan(clans[posicao_clan(clans[posicao_clan(clans[posicao_clan(clan)].em_guerra)].alianca)].clan_num);
	 	 // Clan Vitorioso
	 	 clans[posicao_clan(clans[posicao_clan(clan)].em_guerra)].em_guerra = -1;
	 	 clans[posicao_clan(clans[posicao_clan(clan)].em_guerra)].pkvit += 3;
	 	 clans[posicao_clan(clans[posicao_clan(clan)].em_guerra)].grana += clans[posicao_clan(clan)].preco;
	 	 salvar_clan(clans[posicao_clan(clans[posicao_clan(clan)].em_guerra)].clan_num);
	 	 //Clan Aliado Derrotado
	 	 clans[posicao_clan(clans[posicao_clan(clan)].alianca)].em_guerra = -1;
	 	 clans[posicao_clan(clans[posicao_clan(clan)].alianca)].pkder += 3;
	 	 clans[posicao_clan(clans[posicao_clan(clan)].alianca)].alianca = -1;
	 	 salvar_clan(clans[posicao_clan(clans[posicao_clan(clan)].alianca)].clan_num);
	 	 //Clan Derrotado
	 	 clans[posicao_clan(clan)].em_guerra = -1;
	 	 clans[posicao_clan(clan)].pkder += 3;
	 	 clans[posicao_clan(clan)].alianca = -1;
	 	 salvar_clan(clan);
	 	 return;
      	    }
	   }
	  } 
	 }
	   if (clans[posicao_clan(clan)].em_guerra > 0)
	   {
	    clans[posicao_clan(clan)].grana -= 10000;
	    salvar_clan(clan);
	   } 

	}
    for (i = character_list; i; i = next_char)
             {
            	    next_char = i->next;
            if (!i) return;
            if (!IS_NPC(i) && !i->desc) return; 
            sprintf(buf, "&RCLAN INFO&W: Informacoes Atualizadas.\r\n");
            send_to_all(buf);
            return;
	} 
}
void alianca_clan(char *argumento, struct char_data *ch)
{

      //send_to_char("Comando Inoperante\r\n",ch);
      short int clan;
      int amount;

	
	if(!GET_CLAN(ch))
	{
		send_to_char("Comando nao disponivel.\r\n", ch);
		return;
	}

	if(!tem_permissao(ch, ALIANCA))
	{
		send_to_char("Voce nao tem posicao suficiente para usar esse comando.\r\n", ch);
		return;
	}

	if(!*argumento || !is_number(argumento))
	{
		send_to_char("Formato: clan alianca <clan>\r\n", ch);
		return;
	}

        clan = atoi(argumento);

	if(posicao_clan(clan) == -1)
	{
		send_to_char("Numero de clan invalido.\r\n", ch);
		return;
	}
	if(clans[posicao_clan(GET_CLAN(ch))].grana < clans[posicao_clan(clan)].preco)
	{
		send_to_char("Voce Precisa Pagar o Custo de Guerra para se Aliar.\r\n", ch);
		return;
	}
	if(clan == GET_CLAN(ch))
	{
		send_to_char("Seu proprio Clan?\r\n", ch);
		return;
	}
	
	if(clan == clans[posicao_clan(GET_CLAN(ch))].em_guerra )
	{
		send_to_char("Voce esta em guerra com este clan.", ch);
		return;
	}
	if(clans[posicao_clan(GET_CLAN(ch))].alianca) 
	{
   	 if(clans[posicao_clan(GET_CLAN(ch))].alianca == clan) 
   	 { 
      		sprintf(buf,"&RCLAN INFO&w: &c%s&n quebrou a Alianca com &c%s&n.\r\n", clans[posicao_clan(GET_CLAN(ch))].nome_clan, clans[posicao_clan(clans[posicao_clan(clan)].alianca)].nome_clan);
     		send_to_char(buf, ch);
     		clans[posicao_clan(GET_CLAN(ch))].alianca = -1;
     		clans[posicao_clan(clan)].alianca = -1;
     		salvar_clan(GET_CLAN(ch));
		salvar_clan(clan);
      		return;
    	 }
   	}
	if(clans[posicao_clan(GET_CLAN(ch))].alianca > 0) 
	{
		send_to_char("Este Clan Ja possui um Aliado\r\n",ch);
		return;
	}
		
amount = clans[posicao_clan(clan)].preco;
clans[posicao_clan(GET_CLAN(ch))].alianca = clan;
clans[posicao_clan(clan)].alianca = GET_CLAN(ch);
clans[posicao_clan(GET_CLAN(ch))].grana -= amount;
clans[posicao_clan(clan)].grana += amount;
clans[posicao_clan(GET_CLAN(ch))].em_guerra = clans[posicao_clan(clan)].em_guerra;
sprintf(buf, "&RCLAN INFO&w: &c%s &nse aliou a &c%s&n!\r\n", clans[posicao_clan(GET_CLAN(ch))].nome_clan, clans[posicao_clan(clan)].nome_clan);
send_to_all(buf);
salvar_clan(GET_CLAN(ch));
salvar_clan(clan);

return;

}

void claim_zone(struct char_data *ch, struct obj_data *obj)
{
	short int clan;
	if((GET_OBJ_TYPE(obj) == ITEM_NOTE) && (ROOM_FLAGGED(IN_ROOM(ch), ROOM_CHALLENGE)))
	{
		if(GET_CLAN(ch))
		{
		  if (clans[posicao_clan(GET_CLAN(ch))].em_guerra > 0)
		    { 	
		     if (clans[posicao_clan(GET_CLAN(ch))].em_guerra == GET_OBJ_VAL(obj, 0))
		      {
			if(GET_OBJ_VAL(obj, 0) != GET_CLAN(ch))
			{
				sprintf(buf, "&RCLAN INFO&w: &c%s &ncapturou &C%s&n para &c%s&n!\r\n\r\n&n&RCLAN INFO&w: &nA &CGuerra &nacabou, todos os &cClans&n estao em Paz.\r\n", GET_NAME(ch), 
				obj->short_description, clans[posicao_clan(GET_CLAN(ch))].nome_clan);

clan =  clans[posicao_clan(GET_CLAN(ch))].em_guerra;
//clan derrotado
clans[posicao_clan(GET_CLAN(ch))].em_guerra = -1;
clans[posicao_clan(GET_CLAN(ch))].pkder += 5;
clans[posicao_clan(GET_CLAN(ch))].grana -= 100000;
//aliado do clan derrotado
clans[posicao_clan(clans[posicao_clan(GET_CLAN(ch))].alianca)].em_guerra = -1;
clans[posicao_clan(clans[posicao_clan(GET_CLAN(ch))].alianca)].pkder += 5;
clans[posicao_clan(clans[posicao_clan(GET_CLAN(ch))].alianca)].grana -= 100000;
//clan vitorioso
clans[posicao_clan(clan)].em_guerra = -1;
clans[posicao_clan(clan)].pkvit += 5;
clans[posicao_clan(clan)].grana += 100000;
//aliado clan vitorioso
clans[posicao_clan(clans[posicao_clan(clan)].alianca)].em_guerra = -1;
clans[posicao_clan(clans[posicao_clan(clan)].alianca)].pkvit += 5;
clans[posicao_clan(clans[posicao_clan(clan)].alianca)].grana += 100000;	
				
send_to_all(buf);
salvar_clan(GET_CLAN(ch));
salvar_clan(clan);
		     }
		    }
		  }
		}
	      }
}
