
struct item_auction {
	struct obj_data *obj;
	struct char_data *dono;
	struct char_data *comprador;
	int preco;
	int ultimo_preco;
	short int tempo;
};

struct item_auction *auc;
short int num_aucs = 0;

void auction_update(void);

