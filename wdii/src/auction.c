#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "screen.h"
#include "auction.h"

extern struct descriptor_data *descriptor_list;
extern struct room_data *world;

char *complete_char(char *string, int num);
int conta_cores(const char *texto);
char *item_condition(struct obj_data *obj);

void copia_auc(struct item_auction *donde, struct item_auction *pronde)
{
	pronde->obj = donde->obj;
	pronde->dono = donde->dono;
	pronde->comprador = donde->comprador;
	pronde->preco = donde->preco;
	pronde->tempo = donde->tempo;
}

void recriar_lista()
{
	struct item_auction *aucn;
	short int num, num2 = 0;

	CREATE(aucn, struct item_auction, 1);

	for(num = 0; num < num_aucs; num++)
		if(auc[num].tempo)
		{
			copia_auc(&auc[num], &aucn[num2++]);
			RECREATE(aucn, struct item_auction, num2 + 1);
		}

	free(auc);
	auc = aucn;
	num_aucs = num2;
}

void send_to_auction(const char *messg)
{
	struct descriptor_data *i;

	if (messg == NULL)
		return;

	for (i = descriptor_list; i; i = i->next)
		if (STATE(i) == CON_PLAYING)
			if (!PLR_FLAGGED(i->character, PLR_WRITING))
				if (!PRF_FLAGGED(i->character, PRF_NOAUCT))
					SEND_TO_Q(messg, i);
}

void adiciona_auction(struct char_data *ch, struct obj_data *tobj, int grana, short int tempu)
{
	if(!num_aucs)
		CREATE(auc, struct item_auction, num_aucs + 2);
	else
		RECREATE(auc, struct item_auction, num_aucs + 2);

	auc[num_aucs].obj = tobj;
	obj_from_char(auc[num_aucs].obj);

	auc[num_aucs].dono = ch;
	auc[num_aucs].comprador = NULL;
	auc[num_aucs].preco = grana;
	auc[num_aucs].ultimo_preco = grana;
	auc[num_aucs].tempo = tempu;
	num_aucs++;

	sprintf(buf, "&R[AUCTION] &c%s &Westa sendo leiloado.&n\r\n", tobj->short_description);
	send_to_auction(buf);
}

void passar_item(struct item_auction *auct)
{

	if(auct->dono)
	{
		sprintf(buf, "&WVoce vendeu %s&W.&n\r\n", auct->obj->short_description);
		send_to_char(buf, auct->dono);
		GET_GOLD(auct->dono) += auct->preco;
	}

	sprintf(buf, "&WVoce comprou %s&W.&n\r\n", auct->obj->short_description);
	send_to_char(buf, auct->comprador);
	GET_GOLD(auct->comprador) -= auct->preco;
	obj_to_char(auct->obj, auct->comprador);
	auct->tempo = 0;

	sprintf(buf, "&R[AUCTION] &c%s &Wfoi vendido por &Y%s &Wpara &C%s&W.&n\r\n",
				 auct->obj->short_description, add_points(auct->preco),
				 GET_NAME(auct->comprador));
	send_to_auction(buf);
}

void auction_update()
{
	short int num, atual = 0;

	if(!num_aucs)
		return;

	for(num = 0; num < num_aucs; num++)
	{
		if(auc[num].tempo > 1)
		{
			auc[num].tempo--;
			continue;
		}

		if(auc[num].comprador)
			passar_item(&auc[num]);
		else
		{
			if(auc[num].dono)
			{
				obj_to_char(auc[num].obj, auc[num].dono);
				send_to_char("&WVoce &Rnao &Wconseguiu vender o item.&n\r\n", auc[num].dono);
			}
			else
			{
				extract_obj(auc[num].obj);
				auc[num].obj = NULL;
			}

			auc[num].tempo = 0;
		}
		atual = 1;
	}

	if(atual)
		recriar_lista();

//	send_to_auction("&R[AUCTION] &WUm tick passou.\r\n");
}

void limpa_auctions(struct char_data *ch)
{
	short int num;

	for(num = 0; num < num_aucs; num++)
	{
		if(auc[num].dono == ch)
			auc[num].dono = NULL;
		if(auc[num].comprador == ch)
		{
			auc[num].comprador = NULL;
			auc[num].preco = auc[num].ultimo_preco;
		}
	}
}

ACMD(do_auction)
{
	char prim[MAX_INPUT_LENGTH], sec[MAX_INPUT_LENGTH], terc[MAX_INPUT_LENGTH];
	struct obj_data *obj;
	short int numauc, tempo;
	int gold;

	if (IS_NPC(ch) || (ch->master && AFF_FLAGGED(ch, AFF_CHARM)))
		return;
	else if (PRF_FLAGGED(ch, PRF_NOAUCT))
	{
		send_to_char("&WSem o canal de &Rauction &Wligado isso nao sera possivel&W.&n\r\n", ch);
		return;
	}
	else if (ROOM_FLAGGED(ch->in_room, ROOM_ARENA))
	{
		send_to_char("Impossivel leiloar enquanto esta na &RWAR&n.\r\n", ch);
		return;
	}

	skip_spaces(&argument);
	half_chop(argument, prim, buf);
	half_chop(buf, sec, terc);

	if((!*prim || !*sec) && !*terc)
	{
		send_to_char("Formato: auction <tempo> <quantia> <obj>\r\n", ch);
		if(num_aucs)
		{
			send_to_char("         auction info <num>\r\n\r\n"
						 "&WNum. &cObjeto                          &CDono       &CComprador        &YPreco &WTempo\r\n", ch);
			for(numauc = 0; numauc < num_aucs; numauc++)
			{
				sprintf(buf, "&R%3d&W# &c%s &C%15s %15s &y%12d &W%5d&n\r\n", numauc + 1,
							complete_char(reprimir(auc[numauc].obj->short_description,
							20+(2*conta_cores(auc[numauc].obj->short_description))), 20),
							(auc[numauc].dono ? GET_NAME(auc[numauc].dono) : "Nao disponivel"),
							(auc[numauc].comprador ? GET_NAME(auc[numauc].comprador) : "Nenhum"),
							auc[numauc].preco, auc[numauc].tempo);
				send_to_char(buf, ch);
			}
		}
	}
	else if(!*terc && !strcmp(prim, "info") && is_number(sec))
	{
		numauc = atoi(sec) - 1;

		if(numauc < 0 || numauc > (num_aucs - 1))
		{
			send_to_char("Ver informacoes de qual objeto?\r\n", ch);
			return;
		}

		strcpy(buf, "&R[AUCTION] &WInfo&n\r\n");
		sprintf(buf + strlen(buf), "&WObjeto&n: %s&n\r\n", auc[numauc].obj->short_description);
		sprintf(buf + strlen(buf), "&cRemort&n/&CLevel&n: &c%d&n/&C%d&n\r\n",
					(GET_OBJ_LEVEL(auc[numauc].obj) - (GET_OBJ_LEVEL(auc[numauc].obj) %
					(LVL_IMMORT - 1))) / (LVL_IMMORT - 1), (GET_OBJ_LEVEL(auc[numauc].obj)
					% (LVL_IMMORT - 1)));
		sprintf(buf + strlen(buf), "&YPreco &Wno leilao&n: &y%14s      &YRende&n: %s%d&W%%&n\r\n",
					add_points(auc[numauc].preco), (GET_OBJ_COST(auc[numauc].obj) >
					auc[numauc].preco ? "&R" : "&Y"), ((auc[numauc].preco*100) /
					GET_OBJ_COST(auc[numauc].obj)) - 100);
		sprintf(buf + strlen(buf), "&WCondicao:%s&n\r\n",
					item_condition(auc[numauc].obj));
		send_to_char(buf, ch);
	}
	else
	{
		if(!is_number(prim) || !is_number(sec))
		{
			send_to_char("Tempo e/ou quantia tem de ser valor numerico.\r\n", ch);
			return;
		}

		if ((obj = get_obj_in_list_vis(ch, terc, ch->carrying)) == NULL)
		{
			send_to_char("Que objeto?\r\n", ch);
			return;
		}

		gold = atoi(sec);
		tempo = atoi(prim);

		if (IS_OBJ_STAT(obj, ITEM_NORENT))
		{
			send_to_char("Impossivel leiloar itens que nao sao salvos.\n\r", ch);
			return;
		}
		else if ((GET_OBJ_TYPE(obj) == ITEM_CONTAINER) && (GET_OBJ_VAL(obj, 3)))
		{
			send_to_char("Impossivel leiloar corpos.\n\r", ch);
			return;
		}
		else if (((GET_OBJ_TYPE(obj)) == ITEM_POTION) ||
			((GET_OBJ_TYPE(obj)) == ITEM_OTHER) || ((GET_OBJ_TYPE(obj)) == ITEM_TRASH) ||
			(((GET_OBJ_TYPE(obj)) == ITEM_NOTE) || ((GET_OBJ_TYPE(obj)) == ITEM_DRINKCON) ||
			((GET_OBJ_TYPE(obj)) == ITEM_KEY) || ((GET_OBJ_TYPE(obj)) == ITEM_FOOD) ||
			((GET_OBJ_TYPE(obj)) == ITEM_PEN) || ((GET_OBJ_TYPE(obj)) == ITEM_BOAT) ||
			((GET_OBJ_TYPE(obj)) == ITEM_SCROLL)))
		{
			send_to_char("Use o canal para verdadeiros leiloes somente.\n\r", ch);
			return;
		}
		else if ((GET_OBJ_TYPE(obj) == ITEM_CONTAINER) && (obj->contains))
		{
			send_to_char("You can auction only empty containers.\n\r", ch);
			return;
		}
		else if (gold <= (GET_OBJ_COST(obj)*0.5))
		{
			send_to_char("Impossivel leiloar por menos de 50% do valor do item.\r\n", ch);
			return;
		}
		else if(gold > (GET_OBJ_COST(obj)*15))
		{
			send_to_char("Impossivel leiloar por mais que 15x o preco do objeto.\r\n", ch);
			return;
		}
		else if (tempo < 3 || tempo > 20)
		{
			send_to_char("Tempo fora dos limites. (3 <= tempo <= 20)\r\n", ch);
			return;
		}

		adiciona_auction(ch, obj, gold, tempo);
	}
}

ACMD(do_bid)
{
	char prim[MAX_INPUT_LENGTH], sec[MAX_INPUT_LENGTH];
	short int qual;
	int grana;

	skip_spaces(&argument);
	half_chop(argument, prim, sec);

	if (PRF_FLAGGED(ch, PRF_NOAUCT))
	{
		send_to_char("&WSem o canal de &Rauction &Wligado isso nao sera possivel&W.&n\r\n", ch);
		return;
	}
	else if(GET_REMORT(ch) == 0 && GET_LEVEL(ch) < 2)
	{
		send_to_char("Sem level pra dar um lance.\r\n", ch);
		return;
	}
	else if (ROOM_FLAGGED(ch->in_room, ROOM_ARENA))
	{
		send_to_char("Impossivel dar um lance enquanto se esta na &RWAR&n.\r\n", ch);
		return;
	}

	if(!*prim || !*sec || !is_number(prim) || !is_number(sec))
	{
		send_to_char("Formato: bid <numero> <grana>\r\n", ch);
		return;
	}

	qual = atoi(prim) - 1;
	grana = atoi(sec);

	if(qual < 0 || qual > (num_aucs - 1))
	{
		send_to_char("Dar lance por qual produto?\r\n", ch);
		return;
	}
	else if(auc[qual].dono == ch)
	{
		send_to_char("Dar lance no proprio produto? Estranho.\r\n", ch);
		return;
	}
	else if(auc[qual].comprador == ch)
	{
		send_to_char("Cobrir o proprio lance? Estranho.\r\n", ch);
		return;
	}
	else if(grana < (auc[qual].preco == auc[qual].ultimo_preco ? auc[qual].preco : (auc[qual].preco*1.05)))
	{
		send_to_char("Ofereca no minimo 5% a mais que o preco atual.\r\n", ch);
		return;
	}
	else if(GET_GOLD(ch) < grana)
	{
		send_to_char("Voce nao tem gold para isso.\r\n", ch);
		return;
	}

	auc[qual].comprador = ch;
	auc[qual].preco = grana;

	sprintf(buf, "&R[AUCTION] &WNovo lance para &c%s&W.&n\r\n", auc[qual].obj->short_description);
	send_to_auction(buf);
}
