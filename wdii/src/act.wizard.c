/* ************************************************************************
*   File: act.wizard.c                                  Part of CircleMUD *
*  Usage: Player-level god commands and other goodies                     *
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
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "house.h"
#include "screen.h"
#include "constants.h"
#include "olc.h"
#include "dg_scripts.h"
#include "teleport.h"
#include "clan.h"

/*   external vars  */
extern struct room_data *world;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct zone_data *zone_table;
extern struct attack_hit_type attack_hit_text[];
extern char *class_abbrevs[];
extern char *race_abbrevs[];
extern const char *dirs[];
extern time_t boot_time;
extern int top_of_zone_table;
extern int circle_shutdown, circle_reboot;
extern int circle_restrict;
extern int load_into_inventory;
extern int top_of_world;
extern int buf_switches, buf_largecount, buf_overflows;
extern int top_of_mobt;
extern int top_of_objt;
extern int top_of_p_table;
extern int free_rent;
extern struct player_index_element *player_table;
extern struct obj_data *obj_proto;
extern int top_of_objt;
extern struct char_data *mob_proto;
int find_name(char *name);
int _parse_name(char *arg, char *name);
SPECIAL(magic_user);
SPECIAL(ninja);
SPECIAL(thief);
SPECIAL(barbarian);

/* for chars */
extern const char *spells[];
extern const char *pc_class_types[];
extern const char *pc_race_types[];
extern const char *god_wiznames[];

#define pasta "/home/wardome/wdII/lib"
/* extern functions */
int level_exp(int remort, int level);
void Read_Invalid_List(void);
void show_shops(struct char_data * ch, char *value);
void hcontrol_list_houses(struct char_data *ch);
void do_start(struct char_data *ch);
void appear(struct char_data *ch);
void reset_zone(int zone);
void roll_real_abils(struct char_data *ch);
int parse_class(char arg);
int parse_race(char arg);
char *add_points(int value);
void actualize_splskl(struct char_data *ch);
int breath_gain(struct char_data * ch);
struct char_data *find_char(int n);
void Crash_rentsave(struct char_data * ch, int cost);
void display_classes(struct descriptor_data *d);
int allow_class(struct char_data *ch, int i);
void perform_remove(struct char_data * ch, int pos);
void check_autowiz(struct char_data * ch);
void die_follower(struct char_data * ch);
extern int allowed_classes(int x, int i);
int real_zone(int number);
int find_name(char *name);
void write_aliases(struct char_data *ch);
extern int top_of_p_table;
extern struct player_index_element *player_table;

/* local functions */
int perform_set(struct char_data *ch, struct char_data *vict, int mode, char *val_arg);
void perform_immort_invis(struct char_data *ch, int level);
ACMD(do_echo);
ACMD(do_send);
room_rnum find_target_room(struct char_data * ch, char *rawroomstr);
ACMD(do_at);
ACMD(do_goto);
ACMD(do_trans);
ACMD(do_teleport);
ACMD(do_vnum);
void do_stat_room(struct char_data * ch);
void do_stat_object(struct char_data * ch, struct obj_data * j);
void do_stat_character(struct char_data * ch, struct char_data * k);
ACMD(do_stat);
ACMD(do_shutdown);
void stop_snooping(struct char_data * ch);
ACMD(do_snoop);
ACMD(do_switch);
ACMD(do_return);
ACMD(do_load);
ACMD(do_vstat);
ACMD(do_purge);
ACMD(do_syslog);
ACMD(do_advance);
ACMD(do_restore);
void perform_immort_vis(struct char_data *ch);
ACMD(do_invis);
ACMD(do_gecho);
ACMD(do_poofset);
ACMD(do_dc);
ACMD(do_wizlock);
ACMD(do_date);
ACMD(do_last);
ACMD(do_force);
ACMD(do_wiznet);
ACMD(do_zreset);
ACMD(do_wizutil);
void print_zone_to_buf(char *bufptr, int zone);
ACMD(do_show);
ACMD(do_set);
ACMD(do_remort);
ACMD(do_chown);
ACMD(do_tedit);
void show_mreport(struct char_data *ch);
ACMD(do_playerlink);
ACMD(do_veq);
ACMD(do_peace);
ACMD(do_makeall);
ACMD(do_bkplayers);
ACMD(do_bkpobj);
ACMD(do_backup);

ACMD(do_echo)
{
  skip_spaces(&argument);

  if (IS_NPC(ch)) {
    send_to_char("Voce e' um mob. Voce nao pode fazer isso.\r\n", ch);
    return;
  }

  if (!*argument)
    send_to_char("Sim... Mas oque??\r\n", ch);
  else {
    if (subcmd == SCMD_EMOTE)
      sprintf(buf, "$n %s", argument);
    else
      strcpy(buf, argument);
    act(buf, FALSE, ch, 0, 0, TO_ROOM);
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else
      act(buf, FALSE, ch, 0, 0, TO_CHAR);
  }
}


ACMD(do_send)
{
  struct char_data *vict;

  half_chop(argument, arg, buf);

  if (!*arg) {
    send_to_char("Enviar oque onde??\r\n", ch);
    return;
  }
  if (!(vict = get_char_vis(ch, arg))) {
    send_to_char(NOPERSON, ch);
    return;
  }
  send_to_char(buf, vict);
  send_to_char("\r\n", vict);
  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char("Enviado.\r\n", ch);
  else {
    sprintf(buf2, "Voce enviou '%s' to %s.\r\n", buf, GET_NAME(vict));
    send_to_char(buf2, ch);
  }
}



/* take a string, and return an rnum.. used for goto, at, etc.  -je 4/6/93 */
room_rnum find_target_room(struct char_data * ch, char *rawroomstr)
{
  int tmp;
  sh_int location;
  struct char_data *target_mob;
  struct obj_data *target_obj;
  char roomstr[MAX_INPUT_LENGTH];

  one_argument(rawroomstr, roomstr);

  if (!*roomstr) {
    send_to_char("Voce deve preencher o numero ou nome da sala.\r\n", ch);
    return NOWHERE;
  }
  if (isdigit(*roomstr) && !strchr(roomstr, '.')) {
    tmp = atoi(roomstr);
    if ((location = real_room(tmp)) < 0) {
      send_to_char("Nenhuma sala com esse nome existe.\r\n", ch);
      return NOWHERE;
    }
  } else if ((target_mob = get_char_vis(ch, roomstr)))
    location = target_mob->in_room;
  else if ((target_obj = get_obj_vis(ch, roomstr))) {
    if (target_obj->in_room != NOWHERE)
      location = target_obj->in_room;
    else {
      send_to_char("Esse objeto nao esta acessivel.\r\n", ch);
      return NOWHERE;
    }
  } else {
    send_to_char("Nenhuma criatura ou objeto por perto.\r\n", ch);
    return NOWHERE;
  }

  /* a location has been found -- if you're < GRGOD, check restrictions. */
  if (GET_LEVEL(ch) < LVL_GRGOD) {
    if (ROOM_FLAGGED(location, ROOM_GODROOM)) {
      send_to_char("Voce nao e' deus o suficiente para usar esta sala!\r\n", ch);
      return NOWHERE;
    }
    if (ROOM_FLAGGED(location, ROOM_PRIVATE) &&
        world[location].people && world[location].people->next_in_room) {
      send_to_char("Esta' acontecendo uma conversa privada naquela sala.\r\n", ch);
      return NOWHERE;
    }
    if (ROOM_FLAGGED(location, ROOM_HOUSE) &&
        !House_can_enter(ch, GET_ROOM_VNUM(location))) {
      send_to_char("E' privado -- nao ultrapasse!\r\n", ch);
      return NOWHERE;
    }
  }
  return location;
}



ACMD(do_at)
{
  char command[MAX_INPUT_LENGTH];
  int location, original_loc;

  half_chop(argument, buf, command);
  if (!*buf) {
    send_to_char("Voce deve fornecer um numero ou nome para essa sala.\r\n", ch);
    return;
  }

  if (!*command) {
    send_to_char("Oque voce quer fazer e onde?\r\n", ch);
    return;
  }

  if ((location = find_target_room(ch, buf)) < 0)
    return;

  /* a location has been found. */
  original_loc = ch->in_room;
  char_from_room(ch);
  char_to_room(ch, location);
  command_interpreter(ch, command);

  /* check if the char is still there */
  if (ch->in_room == location) {
    char_from_room(ch);
    char_to_room(ch, original_loc);
  }
}


ACMD(do_goto)
{
  sh_int location;

  if ((location = find_target_room(ch, argument)) < 0)
    return;

  if (POOFOUT(ch))
    sprintf(buf, "$n %s", POOFOUT(ch));
  else
    strcpy(buf, "$n desaparece em meio de uma fumaca.");

  act(buf, TRUE, ch, 0, 0, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, location);

  if (POOFIN(ch))
    sprintf(buf, "$n %s", POOFIN(ch));
  else
    strcpy(buf, "$n aparece mais rapido do que seus olhos possam perceber.");

  act(buf, TRUE, ch, 0, 0, TO_ROOM);
  look_at_room(ch, 0);
}



ACMD(do_trans)
{
  struct descriptor_data *i;
  struct char_data *victim;

  one_argument(argument, buf);
  if (!*buf)
    send_to_char("Quem voce deseja transferir?\r\n", ch);
  else if (str_cmp("all", buf)) {
    if (!(victim = get_char_vis(ch, buf)))
      send_to_char(NOPERSON, ch);
    else if (victim == ch)
      send_to_char("Isso nao faz muito sentido, faz?\r\n", ch);
    else {
      if ((GET_LEVEL(ch) < GET_LEVEL(victim)) && !IS_NPC(victim)) {
        send_to_char("Va' transferir alguem do seu tamanho.\r\n", ch);
        return;
      }
      act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
      char_from_room(victim);
      char_to_room(victim, ch->in_room);
      act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
      act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);
      look_at_room(victim, 0);
    }
  } else {                      /* Trans All */
    if (GET_LEVEL(ch) < LVL_GRGOD) {
      send_to_char("Eu acho que nao...\r\n", ch);
      return;
    }

    for (i = descriptor_list; i; i = i->next)
      if (STATE(i) == CON_PLAYING && i->character && i->character != ch) {
        victim = i->character;
        if (GET_LEVEL(victim) >= GET_LEVEL(ch))
          continue;
        act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
        char_from_room(victim);
        char_to_room(victim, ch->in_room);
        act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
        act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);
        look_at_room(victim, 0);
      }
    send_to_char(OK, ch);
  }
}



ACMD(do_teleport)
{
  struct char_data *victim;
  sh_int target;

  two_arguments(argument, buf, buf2);

  if (!*buf)
    send_to_char("Quem voce deseja teleportar?\r\n", ch);
  else if (!(victim = get_char_vis(ch, buf)))
    send_to_char(NOPERSON, ch);
  else if (victim == ch)
    send_to_char("Use 'goto' para teleportar voce mesmo.\r\n", ch);
  else if (GET_LEVEL(victim) >= GET_LEVEL(ch))
    send_to_char("Talvez voce nao deva fazer isso.\r\n", ch);
  else if (!*buf2)
    send_to_char("Onde voce deseja mandar esta pessoa?\r\n", ch);
  else if ((target = find_target_room(ch, buf2)) >= 0) {
    send_to_char(OK, ch);
    act("$n disappears in a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
    char_from_room(victim);
    char_to_room(victim, target);
    act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
    act("$n has teleported you!", FALSE, ch, 0, (char *) victim, TO_VICT);
    look_at_room(victim, 0);
  }
}



ACMD(do_vnum)
{
  two_arguments(argument, buf, buf2);

  if (!*buf || !*buf2 || (!is_abbrev(buf, "mob") && !is_abbrev(buf, "obj"))) {
    send_to_char("Usage: vnum { obj| mob } <name>\r\n", ch);
    return;
  }
  if (is_abbrev(buf, "mob"))
    if (!vnum_mobile(buf2, ch))
      send_to_char("Nenhum mob com esse nome.\r\n", ch);

  if (is_abbrev(buf, "obj"))
    if (!vnum_object(buf2, ch))
      send_to_char("Nenhum objeto com esse nome.\r\n", ch);
}



void do_stat_room(struct char_data * ch)
{
  struct extra_descr_data *desc;
  struct room_data *rm = &world[ch->in_room];
  int i, found = 0;
  struct obj_data *j = 0;
  struct char_data *k = 0;

  sprintf(buf, "Nome da sala: %s%s%s\r\n", CCCYN(ch, C_NRM), rm->name,
          CCNRM(ch, C_NRM));
  send_to_char(buf, ch);

  sprinttype(rm->sector_type, sector_types, buf2);
  sprintf(buf, "Zona: [%3d], VNum: [%s%5d%s], RNum: [%5d], Tipo: %s\r\n",
          zone_table[rm->zone].number, CCGRN(ch, C_NRM), rm->number,
          CCNRM(ch, C_NRM), ch->in_room, buf2);
  send_to_char(buf, ch);

  sprintbit((long) rm->room_flags, room_bits, buf2);
  sprintf(buf, "SpecProc: %s, Flags: %s\r\n",
          (rm->func == NULL) ? "None" : "Saidas", buf2);
  send_to_char(buf, ch);

  sprintbit((long) rm->room_affections, room_affections, buf2);
  sprintf(buf, "Encantamentos da Sala: %s\r\n", buf2);
  send_to_char(buf, ch);

  send_to_char("Descricao:\r\n", ch);
  if (rm->description)
    send_to_char(rm->description, ch);
  else
    send_to_char("  None.\r\n", ch);

  if (rm->ex_description) {
    sprintf(buf, "Descr. Extras:%s", CCCYN(ch, C_NRM));
    for (desc = rm->ex_description; desc; desc = desc->next) {
      strcat(buf, " ");
      strcat(buf, desc->keyword);
    }
    strcat(buf, CCNRM(ch, C_NRM));
    send_to_char(strcat(buf, "\r\n"), ch);
  }
  sprintf(buf, "Apresent. Person:%s", CCYEL(ch, C_NRM));
  for (found = 0, k = rm->people; k; k = k->next_in_room) {
    if (!CAN_SEE(ch, k))
      continue;
    sprintf(buf2, "%s %s(%s)", found++ ? "," : "", GET_NAME(k),
            (!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB")));
    strcat(buf, buf2);
    if (strlen(buf) >= 62) {
      if (k->next_in_room)
        send_to_char(strcat(buf, ",\r\n"), ch);
      else
        send_to_char(strcat(buf, "\r\n"), ch);
      *buf = found = 0;
    }
  }

  if (*buf)
    send_to_char(strcat(buf, "\r\n"), ch);
  send_to_char(CCNRM(ch, C_NRM), ch);

  if (rm->contents) {
    sprintf(buf, "Conteudo:%s", CCGRN(ch, C_NRM));
    for (found = 0, j = rm->contents; j; j = j->next_content) {
      if (!CAN_SEE_OBJ(ch, j))
        continue;
      sprintf(buf2, "%s %s", found++ ? "," : "", j->short_description);
      strcat(buf, buf2);
      if (strlen(buf) >= 62) {
        if (j->next_content)
          send_to_char(strcat(buf, ",\r\n"), ch);
        else
          send_to_char(strcat(buf, "\r\n"), ch);
        *buf = found = 0;
      }
    }

    if (*buf)
      send_to_char(strcat(buf, "\r\n"), ch);
    send_to_char(CCNRM(ch, C_NRM), ch);
  }

  if (rm->tele != NULL) {
        sprintf(buf, "Teleporta a cada %d0 (durante %d0) segundos para %20s (Sala %d)\r\n",
                rm->tele->time, rm->tele->cnt,
                world[real_room(rm->tele->targ)].name,  rm->tele->targ);
        send_to_char(buf, ch);
        send_to_char("Teleport Flags   :", ch);
        sprintbit(rm->tele->mask, teleport_bits, buf);
        send_to_char(buf, ch);
        send_to_char("\r\n",ch);
        if (IS_SET(rm->tele->mask, TELE_OBJ) ||
            IS_SET(rm->tele->mask, TELE_NOOBJ)) {
          j = read_object(rm->tele->obj, VIRTUAL);
          sprintf(buf, "Teleport Object : %s\r\n", j->short_description);
          send_to_char(buf, ch);
          extract_obj(j);
        }
  }

  for (i = 0; i < NUM_OF_DIRS; i++) {
    if (rm->dir_option[i]) {
      if (rm->dir_option[i]->to_room == NOWHERE)
        sprintf(buf1, " %sNONE%s", CCCYN(ch, C_NRM), CCNRM(ch, C_NRM));
      else
        sprintf(buf1, "%s%5d%s", CCCYN(ch, C_NRM),
                GET_ROOM_VNUM(rm->dir_option[i]->to_room), CCNRM(ch, C_NRM));
      sprintbit(rm->dir_option[i]->exit_info, exit_bits, buf2);
      sprintf(buf, "Saida %s%-5s%s:  Para: [%s], Chave: [%5d], Palavra-chave: %s, Tipo: %s\r\n ",
              CCCYN(ch, C_NRM), dirs[i], CCNRM(ch, C_NRM), buf1, rm->dir_option[i]->key,
           rm->dir_option[i]->keyword ? rm->dir_option[i]->keyword : "None",
              buf2);
      send_to_char(buf, ch);
      if (rm->dir_option[i]->general_description)
        strcpy(buf, rm->dir_option[i]->general_description);
      else
        strcpy(buf, "  Sem descricao.\r\n");
      send_to_char(buf, ch);
    }
  }

  /* check the room for a script */
  do_sstat_room(ch);
}



void do_stat_object(struct char_data * ch, struct obj_data * j)
{
  int i, vnum, found;
  struct obj_data *j2;
  struct extra_descr_data *desc;
  int remort, level;

  vnum = GET_OBJ_VNUM(j);
  sprintf(buf, "Nome: '%s%s%s', Aliases: %s\r\n", CCYEL(ch, C_NRM),
          ((j->short_description) ? j->short_description : "<None>"),
          CCNRM(ch, C_NRM), j->name);
  send_to_char(buf, ch);
  sprinttype(GET_OBJ_TYPE(j), item_types, buf1);
  if (GET_OBJ_RNUM(j) >= 0)
    strcpy(buf2, (obj_index[GET_OBJ_RNUM(j)].func ? "Saidas" : "None"));
  else
    strcpy(buf2, "None");
  sprintf(buf, "VNum: [%s%5d%s], RNum: [%5d], Type: %s, SpecProc: %s\r\n",
   CCGRN(ch, C_NRM), vnum, CCNRM(ch, C_NRM), GET_OBJ_RNUM(j), buf1, buf2);
  send_to_char(buf, ch);
  sprintf(buf, "L-Des: %s\r\n", ((j->description) ? j->description : "None"));
  send_to_char(buf, ch);

  if (j->ex_description) {
    sprintf(buf, "Descr. Extras:%s", CCCYN(ch, C_NRM));
    for (desc = j->ex_description; desc; desc = desc->next) {
      strcat(buf, " ");
      strcat(buf, desc->keyword);
    }
    strcat(buf, CCNRM(ch, C_NRM));
    send_to_char(strcat(buf, "\r\n"), ch);
  }
  send_to_char("Pode-se vestir no(a): ", ch);
  sprintbit(j->obj_flags.wear_flags, wear_bits, buf);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  send_to_char("Set char bits : ", ch);
  sprintbit(j->obj_flags.bitvector, affected_bits, buf);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  send_to_char("Flags extras   : ", ch);
  sprintbit(GET_OBJ_EXTRA(j), extra_bits, buf);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);


  level = (GET_OBJ_LEVEL(j)-((LVL_IMMORT - 1)*(GET_OBJ_LEVEL(j)/(LVL_IMMORT - 1))));
    if ((level == 0) && (GET_OBJ_LEVEL(j) > 0)){
     level = (LVL_IMMORT - 1);
     remort = (GET_OBJ_LEVEL(j)/(LVL_IMMORT - 1)) - 1;
    }
    else{
       remort = (GET_OBJ_LEVEL(j)/(LVL_IMMORT - 1));
    }

  sprintf(buf, "Peso: %d, Valor: %s, Preco/iay: %d, Tempo: %d, Lvl_min: %d, Remorte: %d\r\n",
     GET_OBJ_WEIGHT(j), add_points(GET_OBJ_COST(j)), GET_OBJ_RENT(j), GET_OBJ_TIMER(j),
     level, remort);
  send_to_char(buf, ch);

  strcpy(buf, "Na sala: ");
  if (j->in_room == NOWHERE)
    strcat(buf, "Lugar algum");
  else {
    sprintf(buf2, "%d", GET_ROOM_VNUM(IN_ROOM(j)));
    strcat(buf, buf2);
  }
  /*
   * NOTE: In order to make it this far, we must already be able to see the
   *       character holding the object. Therefore, we do not need CAN_SEE().
   */
  strcat(buf, ", No Objeto: ");
  strcat(buf, j->in_obj ? j->in_obj->short_description : "Nada");
  strcat(buf, ", Carregado por: ");
  strcat(buf, j->carried_by ? GET_NAME(j->carried_by) : "Ninguem");
  strcat(buf, ", Vestido por: ");
  strcat(buf, j->worn_by ? GET_NAME(j->worn_by) : "Ninguem");
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  switch (GET_OBJ_TYPE(j)) {
  case ITEM_LIGHT:
    if (GET_OBJ_VAL(j, 2) == -1)
      strcpy(buf, "Horas faltando: Infinitas");
    else
      sprintf(buf, "Horas Restantes: [%d]", GET_OBJ_VAL(j, 2));
    break;
  case ITEM_ERB:
  case ITEM_SCROLL:
  case ITEM_POTION:
    sprintf(buf, "Magias: (Level %d) %s, %s, %s", GET_OBJ_VAL(j, 0),
            skill_name(GET_OBJ_VAL(j, 1)), skill_name(GET_OBJ_VAL(j, 2)),
            skill_name(GET_OBJ_VAL(j, 3)));
    break;
  case ITEM_SPELLBOOK:
    sprintf(buf, "Magias: (Learn %d%%) %s, %s, %s", GET_OBJ_VAL(j, 0),
            skill_name(GET_OBJ_VAL(j, 1)), skill_name(GET_OBJ_VAL(j, 2)),
            skill_name(GET_OBJ_VAL(j, 3)));
    break;
  
  case ITEM_WAND:
  case ITEM_STAFF:
    sprintf(buf, "Magias: %s at level %d, %d (of %d) cargas restantes",
            skill_name(GET_OBJ_VAL(j, 3)), GET_OBJ_VAL(j, 0),
            GET_OBJ_VAL(j, 2), GET_OBJ_VAL(j, 1));
    break;
  case ITEM_WEAPON:
    sprintf(buf, "Aodano: %dd%d (%.1f average), Mensagem tipo: %d",
            GET_OBJ_VAL(j, 1), GET_OBJ_VAL(j, 2),
            ((GET_OBJ_VAL(j, 2) + 1) / 2.0) * GET_OBJ_VAL(j, 1),
            GET_OBJ_VAL(j, 3));
    break;
  case ITEM_ARMOR:
    sprintf(buf, "Aplica no AC: [%d]", GET_OBJ_VAL(j, 0));
    break;
  case ITEM_TRAP:
    sprintf(buf, "Magia: %d, - Pontos de vida: %d",
            GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1));
    break;
  case ITEM_CONTAINER:
    sprintbit(GET_OBJ_VAL(j, 1), container_bits, buf2);
    sprintf(buf, "Capacidade de peso: %d, Tipo de tranca: %s, Key Num: %d, Corpo: %s",
            GET_OBJ_VAL(j, 0), buf2, GET_OBJ_VAL(j, 2),
            YESNO(GET_OBJ_VAL(j, 3)));
    break;
  case ITEM_DRINKCON:
  case ITEM_FOUNTAIN:
    sprinttype(GET_OBJ_VAL(j, 2), drinks, buf2);
    sprintf(buf, "Capacidade: %d, Contem: %d, Envenenado: %s, Liquido: %s",
            GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1), YESNO(GET_OBJ_VAL(j, 3)),
            buf2);
    break;
  case ITEM_NOTE:
    sprintf(buf, "Linguagem: %d", GET_OBJ_VAL(j, 0));
    break;
  case ITEM_KEY:
    strcpy(buf, "");
    break;
  case ITEM_FOOD:
    sprintf(buf, "Deixa cheio: %d, envenenado: %s", GET_OBJ_VAL(j, 0),
            YESNO(GET_OBJ_VAL(j, 3)));
    break;
  case ITEM_MONEY:
    sprintf(buf, "Modedas: %d", GET_OBJ_VAL(j, 0));
    break;
  default:
    sprintf(buf, "Valores 0-3: [%d] [%d] [%d] [%d]",
            GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1),
            GET_OBJ_VAL(j, 2), GET_OBJ_VAL(j, 3));
    break;
  }
  send_to_char(strcat(buf, "\r\n"), ch);

  /*
   * I deleted the "equipment status" code from here because it seemed
   * more or less useless and just takes up valuable screen space.
   */

  if (j->contains) {
    sprintf(buf, "\r\nContem:%s", CCGRN(ch, C_NRM));
    for (found = 0, j2 = j->contains; j2; j2 = j2->next_content) {
      sprintf(buf2, "%s %s", found++ ? "," : "", j2->short_description);
      strcat(buf, buf2);
      if (strlen(buf) >= 62) {
        if (j2->next_content)
          send_to_char(strcat(buf, ",\r\n"), ch);
        else
          send_to_char(strcat(buf, "\r\n"), ch);
        *buf = found = 0;
      }
    }

    if (*buf)
      send_to_char(strcat(buf, "\r\n"), ch);
    send_to_char(CCNRM(ch, C_NRM), ch);
  }
  found = 0;
  send_to_char("Encantamentos:", ch);
  for (i = 0; i < MAX_OBJ_AFFECT; i++)
    if (j->affected[i].modifier) {
      sprinttype(j->affected[i].location, apply_types, buf2);
      sprintf(buf, "%s %+d to %s", found++ ? "," : "",
              j->affected[i].modifier, buf2);
      send_to_char(buf, ch);
    }
  if (!found)
    send_to_char(" Nada", ch);

  send_to_char("\r\n", ch);

  /* check the object for a script */
  do_sstat_object(ch, j);
}


void do_stat_character(struct char_data * ch, struct char_data * k)
{
  int i, i2, found = 0;
  struct obj_data *j;
  struct follow_type *fol;
  struct affected_type *aff;
  

  switch (GET_SEX(k)) {
  case SEX_NEUTRAL:    strcpy(buf, "NEUTRAL-SEX");   break;
  case SEX_MALE:       strcpy(buf, "MALE");          break;
  case SEX_FEMALE:     strcpy(buf, "FEMALE");        break;
  default:             strcpy(buf, "ILLEGAL-SEX!!"); break;
  }

  sprintf(buf2, " %s '%s'  IDNum: [%s], Na sala [%5d] Casa [%d] Lang [%s]\r\n",
          (!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB")),
          GET_NAME(k), add_points(GET_IDNUM(k)), GET_ROOM_VNUM(IN_ROOM(k)), HOUSE(k),
	  (GET_LANGUAGE(k) ? "English" : "Portugues"));

  send_to_char(strcat(buf, buf2), ch);
  if (IS_MOB(k)) {
    sprintf(buf, "Alias: %s, VNum: [%5d], RNum: [%5d]\r\n",
            k->player.name, GET_MOB_VNUM(k), GET_MOB_RNUM(k));
    send_to_char(buf, ch);
  }
  sprintf(buf, "Titulo: &n%s&n\r\n", (k->player.title ? k->player.title : "<Nada>"));
  send_to_char(buf, ch);
  
   
  sprintf(buf, "L-Des: %s", (k->player.long_descr ? k->player.long_descr : "<Nada>\r\n"));
  send_to_char(buf, ch);

  if (!IS_NPC(k)) {      /* Use GET_CLASS() macro? */
    sprintf(buf, "A raca e' %s com a classe %s.\r\n", pc_race_types[(int) GET_RACE(k)], pc_class_types[(int) GET_CLASS(k)]);
  } else {
    sprintf(buf, "O monstro e' da raca %s com a classe %s.\r\n", race_types[(int)(GET_RACE(k) + 1)],  class_types[(int)(GET_CLASS(k) + 1)]);
  }
  send_to_char(buf, ch);

  sprintf(buf, "Lev: [%s%3d%s], XP: [%s%d%s], Align: [%s%d%s], Points: [%s%d%s], QP: [%s%d%s], R: [%s%d%s]\r\n",
          CCYEL(ch, C_NRM), GET_LEVEL(k), CCNRM(ch, C_NRM),
          CCYEL(ch, C_NRM), GET_EXP(k), CCNRM(ch, C_NRM),
          CCYEL(ch, C_NRM), GET_ALIGNMENT(k), CCNRM(ch, C_NRM),
          CCYEL(ch, C_NRM), GET_POINTS(k), CCNRM(ch, C_NRM),
          CCYEL(ch, C_NRM), GET_QP(k), CCNRM(ch, C_NRM),
          CCYEL(ch, C_NRM), GET_REMORT(k), CCNRM(ch, C_NRM));
  send_to_char(buf, ch);

  if (!IS_NPC(k)) {
    strcpy(buf1, (char *) asctime(localtime(&(k->player.time.birth))));
    strcpy(buf2, (char *) asctime(localtime(&(k->player.time.logon))));
    buf1[10] = buf2[10] = '\0';

    sprintf(buf, "Criado: [%s], Ult Logon: [%s], Jogado [%dh %dm], Idade [%d]\r\n",
            buf1, buf2, k->player.time.played / 3600,
            ((k->player.time.played % 3600) / 60), age(k)->year);
    send_to_char(buf, ch);

    sprintf(buf, "Casa: [%d], Falas: [%d/%d/%d], (STL[%d]/per[%d]/NSTL[%d])",
         k->player.hometown, GET_TALK(k, 0), GET_TALK(k, 1), GET_TALK(k, 2),
            GET_PRACTICES(k), int_app[GET_INT(k)].learn,
            wis_app[GET_WIS(k)].bonus);

    sprintf(buf+strlen(buf), ", OLC[%d]", GET_OLC_ZONE(k));
    strcat(buf, "\r\n");
    send_to_char(buf, ch);
  }
  sprintf(buf, "Str: [%s%d/%d%s]  Int: [%s%d%s]  Wis: [%s%d%s]  "
          "Dex: [%s%d%s]  Con: [%s%d%s]  Cha: [%s%d%s]  Luck:[%s%d%s]\r\n",
          CCCYN(ch, C_NRM), GET_STR(k), GET_ADD(k), CCNRM(ch, C_NRM),
          CCCYN(ch, C_NRM), GET_INT(k), CCNRM(ch, C_NRM),
          CCCYN(ch, C_NRM), GET_WIS(k), CCNRM(ch, C_NRM),
          CCCYN(ch, C_NRM), GET_DEX(k), CCNRM(ch, C_NRM),
          CCCYN(ch, C_NRM), GET_CON(k), CCNRM(ch, C_NRM),
          CCCYN(ch, C_NRM), GET_CHA(k), CCNRM(ch, C_NRM),
          CCCYN(ch, C_NRM), GET_LUK(k), CCNRM(ch, C_NRM));
  send_to_char(buf, ch);

  sprintf(buf, "Hit p.:[%s%d/%d+%d%s]  Mana p.:[%s%d/%d+%d%s]  Move p.:[%s%d/%d+%d%s]\r\nBreath: [%s%d/%d+%d%s]  Mental:[%s%d/%d%s]\r\n",
          CCGRN(ch, C_NRM), GET_HIT(k), GET_MAX_HIT(k), hit_gain(k), CCNRM(ch, C_NRM),
          CCGRN(ch, C_NRM), GET_MANA(k), GET_MAX_MANA(k), mana_gain(k), CCNRM(ch, C_NRM),
          CCGRN(ch, C_NRM), GET_MOVE(k), GET_MAX_MOVE(k), move_gain(k), CCNRM(ch, C_NRM),
          CCGRN(ch, C_NRM), GET_OXI(k), GET_MAX_OXI(k), breath_gain(k), CCNRM(ch, C_NRM),
          CCGRN(ch, C_NRM), GET_MENTAL(k), GET_MAX_MENTAL(k), CCNRM(ch, C_NRM));
  send_to_char(buf, ch);

  /* check mobiles for a script */
  if (IS_NPC(k)) {
    do_sstat_character(ch, k);
    if (SCRIPT_MEM(k)) {
      struct script_memory *mem = SCRIPT_MEM(k);
      send_to_char("Script memory:\r\n  Remember             Command\r\n", ch);
      while (mem) {
        struct char_data *mc = find_char(mem->id);
        if (!mc) send_to_char("  ** Corrupted!\r\n", ch);
        else {
          if (mem->cmd) sprintf(buf,"  %-20.20s%s\r\n",GET_NAME(mc),mem->cmd);
          else sprintf(buf,"  %-20.20s <default>\r\n",GET_NAME(mc));
          send_to_char(buf, ch);
        }
      mem = mem->next;
      }
    }
  }

  sprintf(buf, "Moedas: [%d], Banco: [%d] (Total: %s)\r\n",
          GET_GOLD(k), GET_BANK_GOLD(k), add_points(GET_GOLD(k) + GET_BANK_GOLD(k)));
  send_to_char(buf, ch);

  if(!IS_NPC(k)){
    sprintf(buf, "Clan: [%d], Rank: [%d]\r\n",
          GET_CLAN(k), GET_CLAN_POS(k));
    send_to_char(buf, ch);
  }
  sprintf(buf, "AC: [%d/10], Hitroll: [%4d], Damroll: [%4d], Saving throws: [%d/%d/%d/%d/%d]\r\n",
          GET_AC(k), k->points.hitroll, k->points.damroll, GET_SAVE(k, 0),
          GET_SAVE(k, 1), GET_SAVE(k, 2), GET_SAVE(k, 3), GET_SAVE(k, 4));
  send_to_char(buf, ch);

  if(!IS_NPC(k)){
    sprintf(buf, "Quest atual: [%5d]\r\n", GET_QUEST(k));
    send_to_char(buf, ch);
  }

  sprinttype(GET_POS(k), position_types, buf2);
  sprintf(buf, "Pos: %s, Lutando: %s", buf2,
          (FIGHTING(k) ? GET_NAME(FIGHTING(k)) : "Ninguem"));

  if (IS_NPC(k)) {
    strcat(buf, ", Tipo Ataque: ");
    strcat(buf, attack_hit_text[k->mob_specials.attack_type].singular);
  }
  if (k->desc) {
    sprinttype(STATE(k->desc), connected_types, buf2);
    strcat(buf, ", Conectado: ");
    strcat(buf, buf2);
  }
  send_to_char(strcat(buf, "\r\n"), ch);

  strcpy(buf, "Posicao inicial: ");
  sprinttype((k->mob_specials.default_pos), position_types, buf2);
  strcat(buf, buf2);

  sprintf(buf2, ", Tempo Ausente(in tics) [%d]\r\n", k->char_specials.timer);
  strcat(buf, buf2);
  send_to_char(buf, ch);

  if (IS_NPC(k)) {
    sprintbit(MOB_FLAGS(k), action_bits, buf2);
    sprintf(buf, "NPC flags: %s%s%s\r\n", CCCYN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
    send_to_char(buf, ch);
  } else {
    sprintbit(PLR_FLAGS(k), player_bits, buf2);
    sprintf(buf, "PLR: %s%s%s\r\n", CCCYN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
    send_to_char(buf, ch);
    sprintbit(PRF_FLAGS(k), preference_bits, buf2);
    sprintf(buf, "PRF: %s%s%s\r\n", CCGRN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
    send_to_char(buf, ch);
    sprintbit(PRF2_FLAGS(k), preference2_bits, buf2);
    sprintf(buf, "PRF2: %s%s%s\r\n", CCGRN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
    send_to_char(buf, ch);
  }

  if (IS_MOB(k)) {
    sprintf(buf, "Mob Spec-Proc: %s, NPC Dano por mao: %dd%d\r\n",
            (mob_index[GET_MOB_RNUM(k)].func ? "Saidas" : "Nada"),
            k->mob_specials.damnodice, k->mob_specials.damsizedice);
    send_to_char(buf, ch);
  }
  sprintf(buf, "Carregado: peso: %d, itens: %d; ",
          IS_CARRYING_W(k), IS_CARRYING_N(k));

  for (i = 0, j = k->carrying; j; j = j->next_content, i++);
  sprintf(buf + strlen(buf), "Itens no: inventario: %d, ", i);

  for (i = 0, i2 = 0; i < NUM_WEARS; i++)
    if (GET_EQ(k, i))
      i2++;
  sprintf(buf2, "eq: %d\r\n", i2);
  strcat(buf, buf2);
  send_to_char(buf, ch);

  if (!IS_NPC(k)) {
    sprintf(buf, "Fome: %d, Sede: %d, Bebida: %d\r\n",
          GET_COND(k, FULL), GET_COND(k, THIRST), GET_COND(k, DRUNK));
    send_to_char(buf, ch);
  }
  if (PLR_FLAGGED(k, PLR_SECPLAYER) && GET_SECOND(k) > 2){
  	
  	sprintf(buf, "First Char: %s\r\n", get_name_by_id(GET_SECOND(k)));
  	send_to_char(buf, ch);
	}
  sprintf(buf, "Master e': %s, Seguidores sao:",
          ((k->master) ? GET_NAME(k->master) : "<nada>"));

  for (fol = k->followers; fol; fol = fol->next) {
    sprintf(buf2, "%s %s", found++ ? "," : "", PERS(fol->follower, ch));
    strcat(buf, buf2);
    if (strlen(buf) >= 62) {
      if (fol->next)
        send_to_char(strcat(buf, ",\r\n"), ch);
      else
        send_to_char(strcat(buf, "\r\n"), ch);
      *buf = found = 0;
    }
  }

  if (*buf)
    send_to_char(strcat(buf, "\r\n"), ch);

  /* Showing the bitvector */
  sprintbit(AFF_FLAGS(k), affected_bits, buf2);
  sprintf(buf, "AFF: %s%s%s\r\n", CCYEL(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
  send_to_char(buf, ch);

  sprintbit(AFF2_FLAGS(k), affected2_bits, buf2);
  sprintf(buf, "AFF2: %s%s%s\r\n", CCYEL(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
  send_to_char(buf, ch);

  sprintbit(AFF3_FLAGS(k), affected3_bits, buf2);
  sprintf(buf, "AFF3: %s%s%s\r\n", CCYEL(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
  send_to_char(buf, ch);

  /* Routine to show what spells a char is affected by */
  if (k->affected) {
    for (aff = k->affected; aff; aff = aff->next) {
      *buf2 = '\0';
        sprintf(buf, "SPL: (%3dhr) %s%-21s%s ", aff->duration + 1,
              CCCYN(ch, C_NRM), spells[aff->type], CCNRM(ch, C_NRM));

      if (aff->modifier) {
        sprintf(buf2, "%+d to %s", aff->modifier, apply_types[(int) aff->location]);
        strcat(buf, buf2);
      }
      if (aff->bitvector) {
        if (*buf2)
          strcat(buf, ", sets ");
        else
          strcat(buf, "sets ");
        sprintbit(aff->bitvector, affected_bits, buf2);
        strcat(buf, buf2);
      }
      if (aff->bitvector2) {
        if (*buf2)
          strcat(buf, ", sets ");
        else
          strcat(buf, "sets ");
        sprintbit(aff->bitvector2, affected2_bits, buf2);
        strcat(buf, buf2);
      }
      if (aff->bitvector3) {
        if (*buf2)
          strcat(buf, ", sets ");
        else
          strcat(buf, "sets ");
        sprintbit(aff->bitvector3, affected3_bits, buf2);
        strcat(buf, buf2);
      }
      send_to_char(strcat(buf, "\r\n"), ch);
    }
  }
}


ACMD(do_stat)
{
  struct char_data *victim = 0;
  struct obj_data *object = 0;
  int tmp;

  half_chop(argument, buf1, buf2);

  if (!*buf1) {
    do_stat_character(ch, ch);
    return;
  } else if (is_abbrev(buf1, "sala")) {
    do_stat_room(ch);
  } else if (is_abbrev(buf1, "mob")) {
    if (!*buf2)
      send_to_char("Status em que player?\r\n", ch);
    else {
      if ((victim = get_char_vis(ch, buf2)))
        do_stat_character(ch, victim);
      else
        send_to_char("Nenhum mob por perto.\r\n", ch);
    }
  } else if (is_abbrev(buf1, "jogador")) {
    if (!*buf2) {
      send_to_char("Status em qual jogador?\r\n", ch);
    } else {
      if ((victim = get_player_vis(ch, buf2, 0)))
        do_stat_character(ch, victim);
      else
        send_to_char("Nenhum jogador por perto.\r\n", ch);
    }
  } else if (is_abbrev(buf1, "file")) {
    if (!*buf2) {
      send_to_char("Status em qual jogador?\r\n", ch);
    } else {
      CREATE(victim, struct char_data, 1);
      clear_char(victim);
      CREATE(victim->player_specials, struct player_special_data, 1);
      if (load_char(buf2, victim) > -1) {
        char_to_room(victim, 0);
        if (GET_LEVEL(victim) > GET_LEVEL(ch))
          send_to_char("Desculpe, voce nao pode fazer isso.\r\n", ch);
        else
          do_stat_character(ch, victim);
        extract_char(victim);
      } else {
        send_to_char("Nao tem nenhum jogador.\r\n", ch);
        free_char(victim);
      }
    }
  } else if (is_abbrev(buf1, "objeto")) {
    if (!*buf2)
      send_to_char("Status em qual objeto?\r\n", ch);
    else {
      if ((object = get_obj_vis(ch, buf2)))
        do_stat_object(ch, object);
      else
        send_to_char("Nenhum objeto por perto.\r\n", ch);
    }
  } else {
    if ((object = get_object_in_equip_vis(ch, buf1, ch->equipment, &tmp)))
      do_stat_object(ch, object);
    else if ((object = get_obj_in_list_vis(ch, buf1, ch->carrying)))
      do_stat_object(ch, object);
    else if ((victim = get_char_room_vis(ch, buf1)))
      do_stat_character(ch, victim);
    else if ((object = get_obj_in_list_vis(ch, buf1, world[ch->in_room].contents)))
      do_stat_object(ch, object);
    else if ((victim = get_char_vis(ch, buf1)))
      do_stat_character(ch, victim);
    else if ((object = get_obj_vis(ch, buf1)))
      do_stat_object(ch, object);
    else
      send_to_char("Nada por perto com este nome.\r\n", ch);
  }
}


ACMD(do_shutdown)
{
  if (subcmd != SCMD_SHUTDOWN) {
    send_to_char("Se voce quer desligar algo que o faca rapido entao!\r\n", ch);
    return;
  }
  one_argument(argument, arg);

  if (!*arg) {
    log("(GC) Desligado por %s.", GET_NAME(ch));
    send_to_all("Desligando.\r\n");
    circle_shutdown = 1;
  } else if (!str_cmp(arg, "reboot")) {
    log("(GC) Reiniciado por %s.", GET_NAME(ch));
    send_to_all("Reiniciando.. Volte em 1 minuto.\r\n");
    touch(FASTBOOT_FILE);
    circle_shutdown = circle_reboot = 1;
  } else if (!str_cmp(arg, "now")) {
    sprintf(buf, "(GC) Reiniciando agora por %s.", GET_NAME(ch));
    log(buf);
    send_to_all("Reiniciando.. volte em 1 minuto.\r\n");
    circle_shutdown = 1;
    circle_reboot = 2;
  } else if (!str_cmp(arg, "die")) {
    log("(GC) Desligado por %s.", GET_NAME(ch));
    send_to_all("Desligando para manutencao.\r\n");
    touch(KILLSCRIPT_FILE);
    circle_shutdown = 1;
  } else if (!str_cmp(arg, "pause")) {
    log("(GC) Desligado por %s.", GET_NAME(ch));
    send_to_all("Desligando para manutencao.\r\n");
    touch(PAUSE_FILE);
    circle_shutdown = 1;
  } else
    send_to_char("Opcao de desligamento desconhecida.\r\n", ch);
}


void stop_snooping(struct char_data * ch)
{
  if (!ch->desc->snooping)
    send_to_char("Voce nao esta espionando ninguem.\r\n", ch);
  else {
    send_to_char("Voce parou de espionar.\r\n", ch);
    ch->desc->snooping->snoop_by = NULL;
    ch->desc->snooping = NULL;
  }
}


ACMD(do_snoop)
{
  struct char_data *victim, *tch;

  if (!ch->desc)
    return;

  one_argument(argument, arg);

  if (!*arg)
    stop_snooping(ch);
  else if (!(victim = get_char_vis(ch, arg)))
    send_to_char("Nenhuma pessoa por perto.\r\n", ch);
  else if (!victim->desc)
    send_to_char("Nao existe conexao. Nao ha' nada para espionar.\r\n", ch);
  else if (victim == ch)
    stop_snooping(ch);
  else if (victim->desc->snoop_by)
    send_to_char("Ja esta' ocupado. \r\n", ch);
  else if (victim->desc->snooping == ch->desc)
    send_to_char("Nao seja estupido.\r\n", ch);
  else {
    if (victim->desc->original)
      tch = victim->desc->original;
    else
      tch = victim;

    if (GET_LEVEL(tch) >= GET_LEVEL(ch)) {
      send_to_char("Voce nao pode.\r\n", ch);
      return;
    }
    send_to_char(OK, ch);

    if (ch->desc->snooping)
      ch->desc->snooping->snoop_by = NULL;

    ch->desc->snooping = victim->desc;
    victim->desc->snoop_by = ch->desc;
  }
}



ACMD(do_switch)
{
  struct char_data *victim;

  one_argument(argument, arg);

  if (ch->desc->original)
    send_to_char("Voce ja esta' encarnado.\r\n", ch);
  else if (!*arg)
    send_to_char("Encarnar em quem?\r\n", ch);
  else if (!(victim = get_char_vis(ch, arg)))
    send_to_char("Nenhum jogador.\r\n", ch);
  else if (ch == victim)
    send_to_char("Hee hee... nos estamos engracados hoje e'?\r\n", ch);
  else if (victim->desc)
    send_to_char("Voce nao pode, este corpo ja' esta' em uso!\r\n", ch);
  else if ((GET_LEVEL(ch) < LVL_SUBIMPL) && !IS_NPC(victim))
    send_to_char("Voce nao eh bom o suficiente para encarnar em mortais.\r\n", ch);
  else {
    send_to_char(OK, ch);

    ch->desc->character = victim;
    ch->desc->original = ch;

    victim->desc = ch->desc;
    ch->desc = NULL;
  }
}


ACMD(do_return)
{
  if (ch->desc && ch->desc->original) {
    send_to_char("Voce retorna ao seu corpo original.\r\n", ch);

    /* JE 2/22/95 */
    /* if someone switched into your original body, disconnect them */
    if (ch->desc->original->desc)
      STATE(ch->desc->original->desc) = CON_DISCONNECT;

    ch->desc->character = ch->desc->original;
    ch->desc->original = NULL;

    ch->desc->character->desc = ch->desc;
    ch->desc = NULL;
  }
}

#define LOAD_USAGE "Usar: load { obj | mob } <vnum>\r\n"

ACMD(do_load)
{
  struct char_data *mob;
  struct obj_data *obj;
  int number, r_num;

  two_arguments(argument, buf, buf2);

  if (!*buf || !*buf2 /*|| !isdigit(*buf2)*/) {
    send_to_char(LOAD_USAGE, ch);
    return;
  }
  if ((number = atoi(buf2)) < 0) {
    send_to_char("Um numero negativo??\r\n", ch);
    return;
  }
  if (is_abbrev(buf, "mob")) {
    if (!isdigit(*buf2)) {
      send_to_char(LOAD_USAGE , ch);
      return;
    }
    if ((r_num = real_mobile(number)) < 0) {
      send_to_char("Nao tem nenhum monstro com este numero.\r\n", ch);
      return;
    }
    mob = read_mobile(r_num, REAL);
    char_to_room(mob, ch->in_room);

    act("$n makes a quaint, magical gesture with one hand.", TRUE, ch,
        0, 0, TO_ROOM);
    act("$n has created $N!", FALSE, ch, 0, mob, TO_ROOM);
    act("You create $N.", FALSE, ch, 0, mob, TO_CHAR);
    sprintf(buf, "(GC) %s loads mob \"%s\" (%d)", GET_NAME(ch), GET_NAME(mob), number);
    mudlog(buf, BRF, LVL_GOD, TRUE);
    load_mtrigger(mob);
  }
  else if (is_abbrev(buf, "obj")) {
    if (!isdigit(*buf2)) {
      send_to_char(LOAD_USAGE , ch);
      return;
    }
    if ((r_num = real_object(number)) < 0) {
      send_to_char("Nao tem objeto com este numero.\r\n", ch);
      return;
    }
    obj = read_object(r_num, REAL);
    if (load_into_inventory)
      obj_to_char(obj, ch);
    else
      obj_to_room(obj, ch->in_room);
    act("$n makes a strange magical gesture.", TRUE, ch, 0, 0, TO_ROOM);
    act("$n has created $p!", FALSE, ch, obj, 0, TO_ROOM);
    act("You create $p.", FALSE, ch, obj, 0, TO_CHAR);
    sprintf(buf, "(GC) %s loads obj \"%s\" (%d)", GET_NAME(ch), obj->short_description, number);
    mudlog(buf, BRF, LVL_GOD, TRUE);
    load_otrigger(obj);
  } else
    send_to_char("That'll have to be either 'obj' or 'mob'.\r\n", ch);
}



ACMD(do_vstat)
{
  struct char_data *mob;
  struct obj_data *obj;
  int number, r_num;

  two_arguments(argument, buf, buf2);

  if (!*buf || !*buf2 || !isdigit(*buf2)) {
    send_to_char("Usage: vstat { obj | mob } <number>\r\n", ch);
    return;
  }
  if ((number = atoi(buf2)) < 0) {
    send_to_char("Um numero negativo??\r\n", ch);
    return;
  }
  if (is_abbrev(buf, "mob")) {
    if ((r_num = real_mobile(number)) < 0) {
      send_to_char("Nao tem nenhum monstro com este numero.\r\n", ch);
      return;
    }
    mob = read_mobile(r_num, REAL);
    char_to_room(mob, 0);
    do_stat_character(ch, mob);
    extract_char(mob);
  } else if (is_abbrev(buf, "obj")) {
    if ((r_num = real_object(number)) < 0) {
      send_to_char("Nao existe nenhum objeto com este numero.\r\n", ch);
      return;
    }
    obj = read_object(r_num, REAL);
    do_stat_object(ch, obj);
    extract_obj(obj);
  } else
    send_to_char("Tem que ser 'obj' ou 'mob'.\r\n", ch);
}


 

/* clean a room of all mobiles and objects */
ACMD(do_purge)
{
  struct char_data *vict, *next_v;
  struct obj_data *obj, *next_o;

  one_argument(argument, buf);

  if (*buf) {                   /* argument supplied. destroy single object
                                 * or char */
    if ((vict = get_char_room_vis(ch, buf))) {
      if (!IS_NPC(vict) && (GET_LEVEL(ch) <= GET_LEVEL(vict))) {
        send_to_char("Fuuuuuuuuu!\r\n", ch);
        return;
      }
      act("$n desintegra $N.", FALSE, ch, 0, vict, TO_NOTVICT);

      if (!IS_NPC(vict)) {
        sprintf(buf, "(GC) %s has purged %s.", GET_NAME(ch), GET_NAME(vict));
        mudlog(buf, BRF, LVL_GOD, TRUE);
        if (vict->desc) {
          STATE(vict->desc) = CON_CLOSE;
          vict->desc->character = NULL;
          vict->desc = NULL;
        }
      }
      extract_char(vict);
    } else if ((obj = get_obj_in_list_vis(ch, buf, world[ch->in_room].contents))) {
      act("$n destruiu $p.", FALSE, ch, obj, 0, TO_ROOM);
      extract_obj(obj);
    } else {
      send_to_char("Nada aqui com esse nome.\r\n", ch);
      return;
    }

    send_to_char(OK, ch);
  } else {                      /* no argument. clean out the room */
    act("$n gestures... You are surrounded by scorching flames!",
        FALSE, ch, 0, 0, TO_ROOM);
    send_to_room("The world seems a little cleaner.\r\n", ch->in_room);

    for (vict = world[ch->in_room].people; vict; vict = next_v) {
      next_v = vict->next_in_room;
      if (IS_NPC(vict))
        extract_char(vict);
    }

    for (obj = world[ch->in_room].contents; obj; obj = next_o) {
      next_o = obj->next_content;
      extract_obj(obj);
    }
  }
}



const char *logtypes[] = {
  "off", "brief", "normal", "complete", "\n"
};

ACMD(do_syslog)
{
  int tp;

  one_argument(argument, arg);

  if (!*arg) {
    tp = ((PRF_FLAGGED(ch, PRF_LOG1) ? 1 : 0) +
          (PRF_FLAGGED(ch, PRF_LOG2) ? 2 : 0));
    sprintf(buf, "Your syslog is currently %s.\r\n", logtypes[tp]);
    send_to_char(buf, ch);
    return;
  }
  if (((tp = search_block(arg, logtypes, FALSE)) == -1)) {
    send_to_char("Usage: syslog { Off | Brief | Normal | Complete }\r\n", ch);
    return;
  }
  REMOVE_BIT(PRF_FLAGS(ch), PRF_LOG1 | PRF_LOG2);
  SET_BIT(PRF_FLAGS(ch), (PRF_LOG1 * (tp & 1)) | (PRF_LOG2 * (tp & 2) >> 1));

  sprintf(buf, "Your syslog is now %s.\r\n", logtypes[tp]);
  send_to_char(buf, ch);
}


ACMD(do_advance)
{
  struct char_data *victim;
  char *name = arg, *level = buf2;
  int newlevel, oldlevel;

  two_arguments(argument, name, level);

  if (*name) {
    if (!(victim = get_char_vis(ch, name))) {
      send_to_char("Aquele jogador nao esta aqui.\r\n", ch);
      return;
    }
  } else {
    send_to_char("Avancar quem?\r\n", ch);
    return;
  }

  if (GET_LEVEL(ch) <= GET_LEVEL(victim)) {
    send_to_char("Talvez esta nao seja uma grande ideia.\r\n", ch);
    return;
  }
  if (IS_NPC(victim)) {
    send_to_char("Nao em NPC's.\r\n", ch);
    return;
  }
  if (!*level || (newlevel = atoi(level)) <= 0) {
    send_to_char("Isso nao e' um level!\r\n", ch);
    return;
  }
  if (newlevel > LVL_IMPL) {
    sprintf(buf, "%d e' o level mais alto possivel.\r\n", LVL_IMPL);
    send_to_char(buf, ch);
    return;
  }
  if (newlevel > GET_LEVEL(ch)) {
    send_to_char("Sim, claro.\r\n", ch);
    return;
  }
  if (newlevel == GET_LEVEL(victim)) {
    send_to_char("Ele ja' esta' neste level.\r\n", ch);
    return;
  }
  oldlevel = GET_LEVEL(victim);

  if (newlevel < GET_LEVEL(victim)) {
    do_start(victim);
    GET_LEVEL(victim) = newlevel;
    send_to_char("Voce esta' momentaneamente envolto pela escuridao!\r\n"
		 "Voce se sente de alguma forma diminuido.\r\n", victim);
    GET_EXP(victim) = 0;
  } else {
    act("$n faz estranhos gestos.\r\n"
	"Voce sente um sentimento estranho chegando ate' voce,\r\n"
	"Como uma mao gigante que vem para baixo\r\n"
	"de cima agarrando o seu corpo, that\r\n"
	"comeca a piscar em cores diferentes\r\n"
	"em seu interior.\r\n\r\n"
	"Sua cabeca parece estar cheia de demonios\r\n"
	"assim como sua cabeca dissolve \r\n"
	"os elementos de tempo e espaco..\r\n"
	"Derrepente uma silenciosa explosao de luz\r\n"
	"faz voce voltar a realidade.\r\n\r\n"
	"Voce se sente um pouco diferente.", FALSE, ch, 0, victim, TO_VICT);
  }

  send_to_char(OK, ch);

  if (newlevel < oldlevel) {
    log("(GC) %s rebaixado %s do level %d para %d.",
		GET_NAME(ch), GET_NAME(victim), oldlevel, newlevel);
  } else {
    log("(GC) %s avancou %s para o level %d (do %d)",
		GET_NAME(ch), GET_NAME(victim), newlevel, oldlevel);
    gain_exp_regardless(victim, (newlevel - oldlevel));
  }
  save_char(victim, NOWHERE);
}


ACMD(do_restore)
{
  struct descriptor_data *pt;
  struct char_data *vict;
  int i;
  bool found = FALSE;

  one_argument(argument, buf);
  if (!*buf)
    send_to_char("Quem voce deseja restaurar?\r\n", ch);
  else if (!strcmp(buf, "all")) {
    send_to_char("Voce restaura", ch);
    for (pt = descriptor_list; pt; pt = pt->next)
      if (!pt->connected && pt->character && pt->character != ch) {
        vict = pt->character;
        GET_HIT(vict) = GET_MAX_HIT(vict);
        GET_MANA(vict) = GET_MAX_MANA(vict);
        GET_MOVE(vict) = GET_MAX_MOVE(vict);
	GET_MENTAL(vict) = GET_MAX_MENTAL(vict);
	GET_OXI(vict) = GET_MAX_OXI(vict);
        update_pos(vict);
        act("$N makes strange gestures and restores your life!", FALSE, vict, 0, ch, TO_CHAR);
        sprintf(buf, "%s %s", (found?",":""), GET_NAME(vict));
        if (!found)
          found = TRUE;
        send_to_char(buf, ch);
      }
    if (!found) send_to_char(" ninguem", ch);
    send_to_char(".\r\n", ch);
  }
  else if (!(vict = get_char_vis(ch, buf)))
    send_to_char(NOPERSON, ch);
  else {
    GET_HIT(vict) = GET_MAX_HIT(vict);
    GET_MANA(vict) = GET_MAX_MANA(vict);
    GET_MOVE(vict) = GET_MAX_MOVE(vict);
    GET_MENTAL(vict) = GET_MAX_MENTAL(vict);
    GET_OXI(vict) = GET_MAX_OXI(vict);
    if (!IS_NPC(vict) && (GET_LEVEL(ch) >= LVL_GRGOD) && (GET_LEVEL(vict) >= LVL_ELDER)) {
      for (i = 1; i <= MAX_SKILLS; i++)
        SET_SKILL(vict, i, 100);

      if (GET_LEVEL(vict) >= LVL_GRGOD) {
        vict->real_abils.str_add = 100;
        vict->real_abils.intel = 25;
        vict->real_abils.wis = 25;
        vict->real_abils.dex = 25;
        vict->real_abils.str = 25;
        vict->real_abils.con = 25;
        vict->real_abils.cha = 25;
      }
      vict->aff_abils = vict->real_abils;
    }
    update_pos(vict);
    send_to_char(OK, ch);
    act("Voce foi curado por $N!", FALSE, vict, 0, ch, TO_CHAR);
  }
}


void perform_immort_vis(struct char_data *ch)
{
  if (GET_INVIS_LEV(ch) == 0 && !AFF_FLAGGED(ch, AFF_HIDE | AFF_INVISIBLE)) {
    send_to_char("Voce ja esta' visivel.\r\n", ch);
    return;
  }

  GET_INVIS_LEV(ch) = 0;
  appear(ch);
  send_to_char("Voce esta' agora totalmente visivel.\r\n", ch);
}


void perform_immort_invis(struct char_data *ch, int level)
{
  struct char_data *tch;

  if (IS_NPC(ch))
    return;

  GET_INVIS_LEV(ch) = level;
  sprintf(buf, "Seu level de invisibilidade e' %d.\r\n", level);
  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
    if (tch == ch)
      continue;
    if (GET_LEVEL(tch) >= GET_INVIS_LEV(ch) && GET_LEVEL(tch) < level)
      act("Num piscar de olhos voce nota que $n se foi.", FALSE, ch, 0,
          tch, TO_VICT);
    if (GET_LEVEL(tch) < GET_INVIS_LEV(ch) && GET_LEVEL(tch) >= level)
      act("Voce nota que $n esta' de pe' em sua frente.", FALSE, ch, 0,
          tch, TO_VICT);
  }

  send_to_char(buf, ch);
}


ACMD(do_invis)
{
  int level;

  if (IS_NPC(ch)) {
    send_to_char("Voce nao pode fazer isso!\r\n", ch);
    return;
  }

  one_argument(argument, arg);
  if (!*arg) {
    if (GET_INVIS_LEV(ch) > 0)
      perform_immort_vis(ch);
    else
      perform_immort_invis(ch, GET_LEVEL(ch));
  } else {
    level = atoi(arg);
    if (level > GET_LEVEL(ch))
      send_to_char("Voce nao pode ficar ivnisivel acima de seu level.\r\n", ch);
    else if (level < 1)
      perform_immort_vis(ch);
    else
      perform_immort_invis(ch, level);
  }
}


ACMD(do_gecho)
{
  struct descriptor_data *pt;

  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (!*argument)
    send_to_char("Isso deve ser um engano...\r\n", ch);
  else {
    sprintf(buf, "%s\r\n", argument);
    for (pt = descriptor_list; pt; pt = pt->next)
      if (STATE(pt) == CON_PLAYING && pt->character && pt->character != ch)
        send_to_char(buf, pt->character);
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else
      send_to_char(buf, ch);
  }
}


ACMD(do_poofset)
{
  char **msg;

  switch (subcmd) {
  case SCMD_POOFIN:    msg = &(POOFIN(ch));    break;
  case SCMD_POOFOUT:   msg = &(POOFOUT(ch));   break;
  default:    return;
  }

  skip_spaces(&argument);

  if (*msg)
    free(*msg);

  if (!*argument)
    *msg = NULL;
  else
    *msg = str_dup(argument);

  send_to_char(OK, ch);
}



ACMD(do_dc)
{
  struct descriptor_data *d;
  int num_to_dc;

  one_argument(argument, arg);
  if (!(num_to_dc = atoi(arg))) {
    send_to_char("Usage: DC <user number> (type USERS for a list)\r\n", ch);
    return;
  }
  for (d = descriptor_list; d && d->desc_num != num_to_dc; d = d->next);

  if (!d) {
    send_to_char("No such connection.\r\n", ch);
    return;
  }
  if (d->character && GET_LEVEL(d->character) > GET_LEVEL(ch)) { 
    if (!CAN_SEE(ch, d->character))
      send_to_char("No such connection.\r\n", ch);
    else
      send_to_char("Umm.. Talvez nao seja uma boa ideia...\r\n", ch);
    return;
  }

  /* We used to just close the socket here using close_socket(), but
   * various people pointed out this could cause a crash if you're
   * closing the person below you on the descriptor list.  Just setting
   * to CON_CLOSE leaves things in a massively inconsistent state so I
   * had to add this new flag to the descriptor.
   *
   * It is a much more logical extension for a CON_DISCONNECT to be used
   * for in-game socket closes and CON_CLOSE for out of game closings.
   * This will retain the stability of the close_me hack while being
   * neater in appearance. -gg 12/1/97
   */
  STATE(d) = CON_DISCONNECT;
  sprintf(buf, "Conexao #%d fechada.\r\n", num_to_dc);
  send_to_char(buf, ch);
  log("(GC) Conexao fechada por %s.", GET_NAME(ch));
}



ACMD(do_wizlock)
{
  int value;
  const char *when;

  one_argument(argument, arg);
  if (*arg) {
    value = atoi(arg);
    if (value < 0 || value > GET_LEVEL(ch)) {
      send_to_char("Invalid wizlock value.\r\n", ch);
      return;
    }
    circle_restrict = value;
    when = "now";
  } else
    when = "currently";

  switch (circle_restrict) {
  case 0:
    sprintf(buf, "O jogo esta' %s completamente aberto.\r\n", when);
    break;
  case 1:
    sprintf(buf, "O jogo esta' %s fechado para novos jogadores.\r\n", when);
    break;
  default:
    sprintf(buf, "Apenas level %d acima podem entrar no jogo %s.\r\n",
            circle_restrict, when);
    break;
  }
  send_to_char(buf, ch);
}


ACMD(do_date)
{
  char *tmstr;
  time_t mytime;
  int d, h, m;

  if (subcmd == SCMD_DATE)
    mytime = time(0);
  else
    mytime = boot_time;

  tmstr = (char *) asctime(localtime(&mytime));
  *(tmstr + strlen(tmstr) - 1) = '\0';

  if (subcmd == SCMD_DATE)
    sprintf(buf, "Current machine time: %s\r\n", tmstr);
  else {
    mytime = time(0) - boot_time;
    d = mytime / 86400;
    h = (mytime / 3600) % 24;
    m = (mytime / 60) % 60;

    sprintf(buf, "Up since %s: %d day%s, %d:%02d\r\n", tmstr, d,
            ((d == 1) ? "" : "s"), h, m);
  }

  send_to_char(buf, ch);
}



ACMD(do_last)
{
  struct char_data *vict = NULL;

  one_argument(argument, arg);
  if (!*arg) {
    send_to_char("For whom do you wish to search?\r\n", ch);
    return;
  }
  CREATE(vict, struct char_data, 1);
  clear_char(vict);
  CREATE(vict->player_specials, struct player_special_data, 1);
  if (load_char(arg, vict) <  0) {
    send_to_char("There is no such player.\r\n", ch);
    free_char(vict);
    return;
  }
  if ((GET_LEVEL(vict) > GET_LEVEL(ch)) && (GET_LEVEL(ch) < LVL_IMPL)) {
    send_to_char("You are not sufficiently godly for that!\r\n", ch);
    return;
  }
  sprintf(buf, "[%5ld] [%2d %s] %-12s : %-18s : %-20s\r\n",
	GET_IDNUM(vict), (int) GET_LEVEL(vict),
	class_abbrevs[(int) GET_CLASS(vict)], GET_NAME(vict),
	vict->player_specials->host && *vict->player_specials->host
	? vict->player_specials->host : "(NOHOST)",
	ctime(&vict->player.time.logon));
   send_to_char(buf, ch);
   free_char(vict);
}


ACMD(do_force)
{
  struct descriptor_data *i, *next_desc;
  struct char_data *vict, *next_force;
  char to_force[MAX_INPUT_LENGTH + 2];

  half_chop(argument, arg, to_force);

  sprintf(buf1, "$n forcou voce a digitar '%s'.", to_force);

  if (!*arg || !*to_force)
    send_to_char("Quem voce gostaria de forcar?\r\n", ch);
  else if ((GET_LEVEL(ch) < LVL_GRGOD) || (str_cmp("all", arg) && str_cmp("room", arg))) {
    if (!(vict = get_char_vis(ch, arg)))
      send_to_char(NOPERSON, ch);
    else if (GET_LEVEL(ch) <= GET_LEVEL(vict))
      send_to_char("NAO NAO NAO!\r\n", ch);
    else {
      send_to_char(OK, ch);
      act(buf1, TRUE, ch, NULL, vict, TO_VICT);
      sprintf(buf, "(GC) %s forcou %s a %s", GET_NAME(ch), GET_NAME(vict), to_force);
      mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
      command_interpreter(vict, to_force);
    }
  } else if (!str_cmp("room", arg)) {
    send_to_char(OK, ch);
    sprintf(buf, "(GC) %s forcou sala %d a %s",
                GET_NAME(ch), GET_ROOM_VNUM(IN_ROOM(ch)), to_force);
    mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);

    for (vict = world[ch->in_room].people; vict; vict = next_force) {
      next_force = vict->next_in_room;
      if (GET_LEVEL(vict) >= GET_LEVEL(ch))
        continue;
      act(buf1, TRUE, ch, NULL, vict, TO_VICT);
      command_interpreter(vict, to_force);
    }
  } else { /* force all */
    send_to_char(OK, ch);
    sprintf(buf, "(GC) %s forcou todos a digitarem %s", GET_NAME(ch), to_force);
    mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);

    for (i = descriptor_list; i; i = next_desc) {
      next_desc = i->next;

      if (STATE(i) != CON_PLAYING || !(vict = i->character) || GET_LEVEL(vict) >= GET_LEVEL(ch))
        continue;
      act(buf1, TRUE, ch, NULL, vict, TO_VICT);
      command_interpreter(vict, to_force);
    }
  }
}



ACMD(do_wiznet)
{
  struct descriptor_data *d;
  char any = FALSE;
  int level = LVL_IMMORT;

  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (!*argument) {
    send_to_char("Usage: wiznet <text> | #<level> <text> | wiznet @\r\n "
                 "Digite HELP WIZNET para maiores informacoes.\r\n", ch);
    return;
  }
  switch (*argument) {
  case '#':
    one_argument(argument + 1, buf1);
    if (is_number(buf1)) {
      half_chop(argument+1, buf1, argument);
      level = MAX(atoi(buf1), LVL_IMMORT);
      if (level > GET_LEVEL(ch)) {
        send_to_char("You can't wizline above your own level.\r\n", ch);
        return;
      }
    }
    break;
  case '@':
    for (d = descriptor_list; d; d = d->next) {
      if (STATE(d) == CON_PLAYING && GET_LEVEL(d->character) >= LVL_IMMORT &&
          !PRF_FLAGGED(d->character, PRF_NOWIZ) &&
          (CAN_SEE(ch, d->character) || GET_LEVEL(ch) == LVL_IMPL)) {
        if (!any) {
          strcpy(buf1, "Gods online:\r\n");
          any = TRUE;
        }
        sprintf(buf1 + strlen(buf1), "  %s", GET_NAME(d->character));
        if (PLR_FLAGGED(d->character, PLR_WRITING))
          strcat(buf1, " (Writing)\r\n");
        else if (PLR_FLAGGED(d->character, PLR_MAILING))
          strcat(buf1, " (Writing mail)\r\n");
        else
          strcat(buf1, "\r\n");

      }
    }
    any = FALSE;
    for (d = descriptor_list; d; d = d->next) {
      if (STATE(d) == CON_PLAYING && GET_LEVEL(d->character) >= LVL_IMMORT &&
          PRF_FLAGGED(d->character, PRF_NOWIZ) &&
          CAN_SEE(ch, d->character)) {
        if (!any) {
          strcat(buf1, "Gods offline:\r\n");
          any = TRUE;
        }
        sprintf(buf1 + strlen(buf1), "  %s\r\n", GET_NAME(d->character));
      }
    }
    send_to_char(buf1, ch);
    return;
  case '\\':
    ++argument;
    break;
  default:
    break;
  }
  if (PRF_FLAGGED(ch, PRF_NOWIZ)) {
    send_to_char("You are offline!\r\n", ch);
    return;
  }
  skip_spaces(&argument);

  if (!*argument) {
    send_to_char("Don't bother the gods like that!\r\n", ch);
    return;
  }
  if (level > LVL_IMMORT) {
    sprintf(buf1, "[%d] %s: %s\r\n", level, GET_NAME(ch), argument);
    sprintf(buf2, "[%d] Someone: %s\r\n", level, argument);
  } else {
    sprintf(buf1, "%s: %s\r\n", GET_NAME(ch), argument);
    sprintf(buf2, "Someone: %s\r\n", argument);
  }

  for (d = descriptor_list; d; d = d->next) {
    if ((STATE(d) == CON_PLAYING) && (GET_LEVEL(d->character) >= level) &&
        (!PRF_FLAGGED(d->character, PRF_NOWIZ)) &&
        (!PLR_FLAGGED(d->character, PLR_WRITING | PLR_MAILING))
        && (d != ch->desc || !(PRF_FLAGGED(d->character, PRF_NOREPEAT)))) {
      send_to_char(CCBCYN(d->character, C_NRM), d->character);
      if (CAN_SEE(d->character, ch))
        send_to_char(buf1, d->character);
      else
        send_to_char(buf2, d->character);
      send_to_char(CCNRM(d->character, C_NRM), d->character);
    }
  }

  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char(OK, ch);
}



ACMD(do_zreset)
{
  int i, j;

  one_argument(argument, arg);
  if (!*arg) {
    send_to_char("Voce deve especificar a zona.\r\n", ch);
    return;
  }
  if (*arg == '*') {
    for (i = 0; i <= top_of_zone_table; i++)
      reset_zone(i);
    send_to_char("Reinicia o mundo.\r\n", ch);
    sprintf(buf, "(GC) %s reinicia o mundo inteiro.", GET_NAME(ch));
    mudlog(buf, NRM, MAX(LVL_GRGOD, GET_INVIS_LEV(ch)), TRUE);
    return;
  } else if (*arg == '.')
    i = world[ch->in_room].zone;
  else {
    j = atoi(arg);
    for (i = 0; i <= top_of_zone_table; i++)
      if (zone_table[i].number == j)
        break;
  }
  if (i >= 0 && i <= top_of_zone_table) {
    reset_zone(i);
    sprintf(buf, "Reinicia a zona %d (#%d): %s.\r\n", i, zone_table[i].number,
            zone_table[i].name);
    send_to_char(buf, ch);
    sprintf(buf, "(GC) %s reinicia a zona %d (%s)", GET_NAME(ch), i, zone_table[i].name);
    mudlog(buf, NRM, MAX(LVL_GRGOD, GET_INVIS_LEV(ch)), TRUE);
  } else
    send_to_char("Numero de zona invalido.\r\n", ch);
}


/*
 *  General fn for wizcommands of the sort: cmd <player>
 */

ACMD(do_wizutil)
{
  struct char_data *vict;
  long result;

  one_argument(argument, arg);

  if (!*arg)
    send_to_char("Sim, mas quem?\r\n", ch);
  else if (!(vict = get_char_vis(ch, arg)))
    send_to_char("Nao tem nenhum jogador.\r\n", ch);
  else if (IS_NPC(vict))
    send_to_char("Voce nao pode fazer isso com um Mob!\r\n", ch);
  else if (GET_LEVEL(vict) > GET_LEVEL(ch))
    send_to_char("Hmmm...melhor nao fazer isso.\r\n", ch);
  else {
    switch (subcmd) {
    case SCMD_REROLL:
      send_to_char("Redestribuido...\r\n", ch);
      roll_real_abils(vict);
      log("(GC) %s redestribuiu %s.", GET_NAME(ch), GET_NAME(vict));
      sprintf(buf, "Novo status: Str %d/%d, Int %d, Wis %d, Dex %d, Con %d, Cha %d\r\n",
              GET_STR(vict), GET_ADD(vict), GET_INT(vict), GET_WIS(vict),
              GET_DEX(vict), GET_CON(vict), GET_CHA(vict));
      send_to_char(buf, ch);
      break;
    case SCMD_PARDON:
      if (!PLR_FLAGGED(vict, PLR_THIEF | PLR_KILLER | PLR_DEAD)) {
	send_to_char("Sua vitima nao esta' flagada.\r\n", ch);
	return;
      }
      REMOVE_BIT(PLR_FLAGS(vict), PLR_THIEF | PLR_KILLER | PLR_DEAD);
      send_to_char("Perdoado.\r\n", ch);
      send_to_char("Voce foi perdoado pelos deuses!\r\n", vict);
      sprintf(buf, "(GC) %s perdoado por %s", GET_NAME(vict), GET_NAME(ch));
      mudlog(buf, BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
      break;
    case SCMD_NOTITLE:
      result = PLR_TOG_CHK(vict, PLR_NOTITLE);
      sprintf(buf, "(GC) Sem titulo em %s por %s by %s.", ONOFF(result),
              GET_NAME(vict), GET_NAME(ch));
      mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
      if (result)
        send_to_char("Voce abusou usando o titulo. Voce nao pode mais muda-lo.\r\n", vict);
      else
        send_to_char("Voce pode mudar seu titulo denovo.\r\n", vict);
      sprintf(buf, "Sem-titulo %s ativado por %s.\r\n", (result?"":"de"), GET_NAME(vict));
      send_to_char(buf, ch);
      break;
    case SCMD_SQUELCH:
      result = PLR_TOG_CHK(vict, PLR_NOSHOUT);
      sprintf(buf, "(GC) Squelch %s for %s by %s.", ONOFF(result),
              GET_NAME(vict), GET_NAME(ch));
      mudlog(buf, BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
      strcat(buf, "\r\n");
      send_to_char(buf, ch);
      break;
    case SCMD_FREEZE:
      if (ch == vict) {
        send_to_char("Ah sim, muito esperto...\r\n", ch);
        return;
      }
      if (PLR_FLAGGED(vict, PLR_FROZEN)) {
        send_to_char("Sua vitima ja' esta' bem gelada.\r\n", ch);
        return;
      }
      SET_BIT(PLR_FLAGS(vict), PLR_FROZEN);
      GET_FREEZE_LEV(vict) = GET_LEVEL(ch);
      send_to_char("A bitter wind suddenly rises and drains every erg of heat from your body!\r\nYou feel frozen!\r\n", vict);
      send_to_char("Frozen.\r\n", ch);
      act("A sudden cold wind conjured from nowhere freezes $n!", FALSE, vict, 0, 0, TO_ROOM);
      sprintf(buf, "(GC) %s frozen by %s.", GET_NAME(vict), GET_NAME(ch));
      mudlog(buf, BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
      break;
    case SCMD_THAW:
      if (!PLR_FLAGGED(vict, PLR_FROZEN)) {
        send_to_char("Sorry, your victim is not morbidly encased in ice at the moment.\r\n", ch);
        return;
      }
      if (GET_FREEZE_LEV(vict) > GET_LEVEL(ch)) {
        sprintf(buf, "Sorry, a level %d God froze %s... you can't unfreeze %s.\r\n",
           GET_FREEZE_LEV(vict), GET_NAME(vict), HMHR(vict));
        send_to_char(buf, ch);
        return;
      }
      sprintf(buf, "(GC) %s un-frozen by %s.", GET_NAME(vict), GET_NAME(ch));
      mudlog(buf, BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
      REMOVE_BIT(PLR_FLAGS(vict), PLR_FROZEN);
      send_to_char("A fireball suddenly explodes in front of you, melting the ice!\r\nYou feel thawed.\r\n", vict);
      send_to_char("Thawed.\r\n", ch);
      act("A sudden fireball conjured from nowhere thaws $n!", FALSE, vict, 0, 0, TO_ROOM);
      break;
    case SCMD_UNAFFECT:
      if (vict->affected) {
        while (vict->affected)
			affect_remove(vict, vict->affected);
        send_to_char("There is a brief flash of light!\r\n"
                     "You feel slightly different.\r\n", vict);
        send_to_char("All spells removed.\r\n", ch);
      } else {
        send_to_char("Your victim does not have any affections!\r\n", ch);
        return;
      }
      break;
    default:
      log("SYSERR: Unknown subcmd %d passed to do_wizutil (%s)", subcmd, __FILE__);
      break;
    }
    save_char(vict, NOWHERE);
  }
}


/* single zone printing fn used by "show zone" so it's not repeated in the
   code 3 times ... -je, 4/6/93 */

void print_zone_to_buf(char *bufptr, int zone)
{
   sprintf(bufptr, "%s%3d %-30.30s Status: %8s ; Top: %5d\r\n",
          bufptr, zone_table[zone].number, zone_table[zone].name,
          (ZONE_FLAGGED(zone, ZONE_OPEN)?"DONE" : "NOT DONE"),
          zone_table[zone].top);
}

ACMD(do_find)
{
  int i, l;
  struct obj_data *obj;
  int count = 0;
  char field[MAX_INPUT_LENGTH];
  char value[MAX_INPUT_LENGTH];
  FILE *fl;
  char fname[MAX_INPUT_LENGTH];
  char find[MAX_STRING_LENGTH];
  struct obj_file_elem object;
  int searching, achado;
  bool named;
  bool found = FALSE;
  struct rent_info rent;

  extern struct player_index_element *player_table;

  struct find_struct {
    const char *cmd;
    int level;
  } fields[] = {
/* 00 */ { "nothing",        0  },
/* 01 */ { "rent",           LVL_GRGOD },
/* 01 */ { "nolinkzone",     LVL_GRGOD },
/* 02 */ { "\n", 0 }
  };

  skip_spaces(&argument);

  if (!*argument) {
    sprintf(find, "As %s %s you can find:\r\n",
         strchr("aeiouyAEIOUY", *GET_GOD_NAME(ch))?"an":"a", GET_GOD_NAME(ch));
    send_to_char(find, ch);
    count=0;
    for (i=1; fields[i].cmd != "\n"; i++) {
      if (GET_LEVEL(ch) >= fields[i].level) {
        count=(count==4?1:count+1);
        sprintf(find, "[%3d] %-13s%s", fields[i].level,
                fields[i].cmd, (count==4?"\r\n":" "));
        send_to_char(find, ch);
      }
    }
    if (count != 4) send_to_char("\r\n", ch);
    return;
  }

  strcpy(arg, two_arguments(argument, field, value));

  for (l = 0; *(fields[l].cmd) != '\n'; l++)
    if (is_abbrev(field, fields[l].cmd))
      break;

  if (GET_LEVEL(ch) < fields[l].level) {
    send_to_char("You are not godly enough for that!\r\n", ch);
    return;
  }
  buf[0] = '\0';
  switch (l) {
    case 1: /* rent */
      if (!isdigit(*value)) {
         send_to_char("Try to use real numbers...\r\n", ch);
         return;
      }
      if ((searching = atoi(value)) < 0) {
         send_to_char("A NEGATIVE number??\r\n", ch);
         return;
      }
      if ((achado = real_object(searching)) < 0) {
        send_to_char("Invalid object vnum.\r\n", ch);
        return;
      }
      obj = read_object(achado, REAL);
      sprintf(find, "Searching for the object '%s' (vnum #%d) in all rent files...\r\n",
                   obj->short_description, searching);
      send_to_char(find, ch);
      *find = '\0';
      extract_obj(obj);
      for (i=0, named = FALSE; i<=top_of_p_table; i++, named = FALSE) {
        if (!get_filename(player_table[i].name, fname, CRASH_FILE))
          return;
        if ((fl = fopen(fname, "rb"))) {
          if (!feof(fl))
            fread(&rent, sizeof(struct rent_info), 1, fl);
          while (!feof(fl)) {
            fread(&object, sizeof(struct obj_file_elem), 1, fl);
            if (ferror(fl)) {
              fclose(fl);
              return;
            }
            if (!feof(fl))
              if (real_object(object.item_number) > -1) {
                if (object.item_number == searching) {
                  if (!named) {
                    named = TRUE;
                    sprintf(find, "%s%s:\r\n", find, player_table[i].name);
                  }
                  found = TRUE;
                  obj = read_object(object.item_number, VIRTUAL);
                  sprintf(find, "%s [%5d] (%9dau) %s\r\n",
                          find, object.item_number, GET_OBJ_COST(obj),
                          obj->short_description);
                  extract_obj(obj);
                }
              }
          }
          fclose(fl);
        }
      }
      if (!found)
        send_to_char("No one found.\r\n", ch);
      else
        page_string(ch->desc, find, 1);
      break;
    case 2: /* link obj */
      if (!isdigit(*value)) {
         send_to_char("Try to use real numbers...\r\n", ch);
         return;
      }
      if ((searching = atoi(value)) < 0) {
         send_to_char("A NEGATIVE number??\r\n", ch);
         return;
      }
      if ((achado = real_zone(searching)) < 0) {
        send_to_char("Invalid zone vnum.\r\n", ch);
        return;
      }
      sprintf(find, "Searching for non linked object '%d' in all rent files...\r\n",
                   searching);
      send_to_char(find, ch);
      *find = '\0';
      for (i=0, named = FALSE; i<=top_of_p_table; i++, named = FALSE) {
        if (!get_filename(player_table[i].name, fname, CRASH_FILE))
          return;
        if ((fl = fopen(fname, "rb"))) {
          if (!feof(fl))
            fread(&rent, sizeof(struct rent_info), 1, fl);
          while (!feof(fl)) {
            fread(&object, sizeof(struct obj_file_elem), 1, fl);
            if (ferror(fl)) {
              fclose(fl);
              return;
            }
            if (!feof(fl))
              if (real_object(object.item_number) > -1) {
                if (div(object.item_number,100).quot == searching) {
                  if (!named) {
                    named = TRUE;
                    sprintf(find, "%s%s:\r\n", find, player_table[i].name);
                  }
                  found = TRUE;
                  obj = read_object(object.item_number, VIRTUAL);
                  sprintf(find, "%s [%5d] (%9dau) %s\r\n",
                          find, object.item_number, GET_OBJ_COST(obj),
                          obj->short_description);
                  extract_obj(obj);
                }
              }
          }
          fclose(fl);
        }
      }
      if (!found)
        send_to_char("No one found.\r\n", ch);
      else
        page_string(ch->desc, find, 1);
      break;
  }
}

#define EXITN(room, door)               (world[room].dir_option[door])

ACMD(do_liblist)
{
  extern struct room_data *world;
  extern struct index_data *mob_index;
  extern struct char_data *mob_proto;
  extern struct index_data *obj_index;
  extern struct obj_data *obj_proto;
  extern struct zone_data *zone_table;
  extern int top_of_objt;
  extern int top_of_mobt;
  extern int top_of_world;

  int first, last, nr, found = 0;
  int door, temsaida, tementrada,tnr;

  two_arguments(argument, buf, buf2);

  if (!*buf || !*buf2) {
    switch (subcmd) {
      case SCMD_RLIST:
        send_to_char("Usage: rlist <begining number> <ending number>\r\n", ch);
        break;
      case SCMD_OLIST:
        send_to_char("Usage: olist <begining number> <ending number>\r\n", ch);
        break;
      case SCMD_MLIST:
        send_to_char("Usage: mlist <begining number> <ending number>\r\n", ch);
        break;
      case SCMD_ZLIST:
        send_to_char("Usage: zlist <begining number> <ending number>\r\n", ch);
        break;
      case SCMD_ELIST:
        send_to_char("Usage: elist <begining number> <ending number>\r\n", ch);
        break;
      case SCMD_DLIST:
        send_to_char("Usage: dlist <begining number> <ending number>\r\n", ch);
        break;
      case SCMD_LLIST:
        send_to_char("Usage: llist <begining number> <ending number>\r\n", ch);
        break;
      case SCMD_VLIST:
        send_to_char("Usage: vlist <begining number> <ending number>\r\n", ch);
        break;
      default:
        sprintf(buf, "SYSERR:: invalid SCMD passed to ACMDdo_build_list!");
        mudlog(buf, BRF, LVL_SUPGOD, TRUE);
        break;
    }
    return;
  }

  first = atoi(buf);
  last = atoi(buf2);

  if ((first < 0) || (first > 99999) || (last < 0) || (last > 99999)) {
    send_to_char("Values must be between 0 and 99999.\n\r", ch);
    return;
  }

  if (first >= last) {
    send_to_char("Second value must be greater than first.\n\r", ch);
    return;
  }

  if(last - first > 100){
    send_to_char("You can't pass the limit... (100 targets)\n\r", ch);
    return;
  }

  switch (subcmd) {
    case SCMD_ELIST:
      sprintf(buf, "Rooms Exits List From Vnum %d to %d\r\n", first, last);
      for (nr = 0; nr <= top_of_world && (world[nr].number <= last); nr++)
       if (world[nr].number >= first)
         {
           for (door = 0; door < NUM_OF_DIRS; door++)
            if (EXITN(nr, door) && EXITN(nr, door)->to_room != NOWHERE )
             {
               if (world[EXITN(nr, door)->to_room].zone !=
                   world[nr].zone)
                  {
                   sprintf(buf, "%s%5d. [%5d] to [%5d] (%3d)\r\n", buf, ++found,
                      world[nr].number, GET_ROOM_VNUM(EXITN(nr, door)->to_room), door);
                  }
             }
          }
      break;

    case SCMD_DLIST:
      sprintf(buf, "Rooms List From Vnum %d to %d\r\n", first, last);
      for (nr = 0; nr <= top_of_world && (world[nr].number <= last); nr++)
       if (world[nr].number >= first)
         {
           temsaida = 0;
           for (door = 0; door < NUM_OF_DIRS; door++)
            if (EXITN(nr, door) && EXITN(nr, door)->to_room != NOWHERE )
              {
                temsaida = 1;
              }
            if (temsaida == 0)
             sprintf(buf, "%s%5d. [%5d]\r\n", buf, ++found,
                      world[nr].number);
            temsaida = 0;
          }
      break;

    case SCMD_LLIST:
      sprintf(buf, "Rooms List From Vnum %d to %d\r\n", first, last);
      for (nr = 0; nr <= top_of_world && (world[nr].number <= last); nr++)
       if (world[nr].number >= first)
         {
           tementrada = 0;
           for (tnr = 0; tnr <= top_of_world && (world[tnr].number <= last); tnr++)
            if (world[tnr].number >= first)
            {
             for (door = 0; door < NUM_OF_DIRS; door++)
              if (EXITN(tnr, door) && EXITN(tnr, door)->to_room != NOWHERE )
               {
                 if (world[EXITN(tnr, door)->to_room].number ==
                   world[nr].number)
                    if ( world[tnr].number != world[nr].number)
                     tementrada = 1;
               }
            }
            if (tementrada == 0)
             sprintf(buf, "%s%5d. [%5d]\r\n", buf, ++found,
                      world[nr].number);
          }
      break;

    case SCMD_VLIST:
      sprintf(buf, "Rooms List From Vnum %d to %d\r\n", first, last);
      for (nr = 0; nr <= top_of_world && (world[nr].number <= last); nr++)
       if (world[nr].number >= first)
         {
           for (tnr = 0; tnr <= top_of_world; tnr++)
            {
             for (door = 0; door < NUM_OF_DIRS; door++)
              if (EXITN(tnr, door) && EXITN(tnr, door)->to_room != NOWHERE )
               {
                 if (world[EXITN(tnr, door)->to_room].number ==
                   world[nr].number)
                    if ( world[tnr].zone != world[nr].zone)
                     sprintf(buf, "%s%5d. [%5d] to [%5d] (%3d)\r\n", buf, ++found,
                      world[tnr].number, world[nr].number, door);
               }
            }

          }
      break;

    case SCMD_RLIST:
      sprintf(buf, "Room List From Vnum %d to %d\r\n", first, last);
      for (nr = 0; nr <= top_of_world && (world[nr].number <= last); nr++) {
        if (world[nr].number >= first) {
          sprintf(buf, "%s%5d. [%5d] (%3d) %s\r\n", buf, ++found,
                  world[nr].number, world[nr].zone,
                  world[nr].name);
        }
      }
      break;
    case SCMD_OLIST:
      sprintf(buf, "Object List From Vnum %d to %d\r\n", first, last);
      for (nr = 0; nr <= top_of_objt && (obj_index[nr].vnum <= last); nr++) {
        if (obj_index[nr].vnum >= first) {
          sprintf(buf, "%s%5d. [%5d] %s\r\n", buf, ++found,
                  obj_index[nr].vnum,
                  obj_proto[nr].short_description);
        }
      }
      break;
    case SCMD_MLIST:
      sprintf(buf, "Mob List From Vnum %d to %d\r\n", first, last);
      for (nr = 0; nr <= top_of_mobt && (mob_index[nr].vnum <= last); nr++) {
        if (mob_index[nr].vnum >= first) {
          sprintf(buf, "%s%5d. [%5d] %s\r\n", buf, ++found,
                  mob_index[nr].vnum,
                  mob_proto[nr].player.short_descr);
        }
      }
      break;
    case SCMD_ZLIST:
      sprintf(buf, "Zone List From Vnum %d to %d\r\n", first, last);
      for (nr = 0; nr <= top_of_zone_table && (zone_table[nr].number <= last); nr++) {
        if (zone_table[nr].number >= first) {
          sprintf(buf, "%s%5d. [%5d] (%3d) %s\r\n", buf, ++found,
                  zone_table[nr].number, zone_table[nr].lifespan,
                  zone_table[nr].name);
        }
      }
      break;
    default:
      sprintf(buf, "SYSERR:: invalid SCMD passed to ACMD(do_liblist)!");
      mudlog(buf, BRF, LVL_OLC_FULL, TRUE);
      return;
  }

  if (!found) {
    switch (subcmd) {
      case SCMD_RLIST:
        send_to_char("No rooms found within those parameters.\r\n", ch);
        break;
      case SCMD_OLIST:
        send_to_char("No objects found within those parameters.\r\n", ch);
        break;
      case SCMD_MLIST:
        send_to_char("No mobiles found within those parameters.\r\n", ch);
        break;
      case SCMD_ZLIST:
        send_to_char("No zones found within those parameters.\r\n", ch);
        break;
      case SCMD_ELIST:
        send_to_char("No rooms exits found within those parameters.\r\n", ch);
        break;
      case SCMD_DLIST:
        send_to_char("No rooms found within those parameters.\r\n", ch);
        break;
      case SCMD_LLIST:
        send_to_char("No rooms found within those parameters.\r\n", ch);
        break;
      case SCMD_VLIST:
        send_to_char("No rooms found within those parameters.\r\n", ch);
        break;
      default:
        sprintf(buf, "SYSERR:: invalid SCMD passed to do_build_list!");
        mudlog(buf, BRF, LVL_GOD, TRUE);
        break;
    }
    return;
  }

  page_string(ch->desc, buf, TRUE);
}


ACMD(do_show)
{
  int i, j, k, l, con;
  char self = 0;
  struct char_data *vict = NULL;
  struct obj_data *obj;
  struct descriptor_data *d;
  int count = 0;
  char field[MAX_INPUT_LENGTH], value[MAX_INPUT_LENGTH], birth[80];
  //bool found = FALSE;

  struct show_struct {
    const char *cmd;
    int level;
  } fields[] = {
/* 00 */ { "nothing",        0  },
/* 01 */ { "zones",          LVL_LORD },
/* 02 */ { "character",      LVL_DEMIGOD },
/* 03 */ { "rent",           LVL_GOD },
/* 04 */ { "stats",          LVL_GRGOD },
/* 05 */ { "errors",         LVL_SUPGOD },
/* 06 */ { "death",          LVL_GOD },
/* 07 */ { "godrooms",       LVL_GOD },
/* 08 */ { "shops",          LVL_LORD },
/* 09 */ { "houses",         LVL_SUPGOD },
/* 10 */ { "players",        LVL_MJGOD },
/* 11 */ { "rooms",          LVL_GOD },
/* 12 */ { "entrances",      LVL_DEMIGOD },
/* 13 */ { "snoop",	     LVL_GRGOD },			/* 10 */
/* 14 */ { "teleport",	     LVL_BUILDER },
/* 15 */ { "memory",	     LVL_SUPGOD },
         { "richest",        LVL_BUILDER },
         { "idnum",					LVL_BUILDER},
/* 17 */ { "\n", 0 }
  };

  skip_spaces(&argument);

  if (!*argument) {
    sprintf(buf, "As %s %s you can view:\r\n",
         strchr("aeiouyAEIOUY", *GET_GOD_NAME(ch))?"an":"a", GET_GOD_NAME(ch));
    send_to_char(buf, ch);
    count=0;
    for (i=1; fields[i].cmd != "\n"; i++) {
      if (GET_LEVEL(ch) >= fields[i].level) {
        count=(count==4?1:count+1);
        sprintf(buf, "[%3d] %-13s%s", fields[i].level,
                fields[i].cmd, (count==4?"\r\n":" "));
        send_to_char(buf, ch);
      }
    }
    if (count != 4) send_to_char("\r\n", ch);
    return;
  }

  strcpy(arg, two_arguments(argument, field, value));

  for (l = 0; *(fields[l].cmd) != '\n'; l++)
    //if (!strncmp(field, fields[l].cmd, strlen(field)))
    if (is_abbrev(field, fields[l].cmd))
      break;

  if (GET_LEVEL(ch) < fields[l].level) {
    send_to_char("You are not godly enough for that!\r\n", ch);
    return;
  }
  if (!strcmp(value, "."))
    self = 1;
  buf[0] = '\0';
  switch (l) {
  case 1: /* show zones */
    /* tightened up by JE 4/6/93 */
    if (self)
      print_zone_to_buf(buf, world[ch->in_room].zone);
    else if (*value && is_number(value)) {
      for (j = atoi(value), i = 0; zone_table[i].number != j && i <= top_of_zone_table; i++);
      if (i <= top_of_zone_table)
        print_zone_to_buf(buf, i);
      else {
        send_to_char("That is not a valid zone.\r\n", ch);
        return;
      }
    } else
      for (i = 0; i <= top_of_zone_table; i++)
        print_zone_to_buf(buf, i);
    page_string(ch->desc, buf, TRUE);
    break;
  case 2: /* show player */
    if (!*value) {
      send_to_char("A name would help.\r\n", ch);
      return;
    }

    CREATE(vict, struct char_data, 1);
    clear_char(vict);
    CREATE(vict->player_specials, struct player_special_data, 1);
    if (load_char(value, vict) < 0) {
      send_to_char("There is no such player.\r\n", ch);
      free_char(vict);
      return;
    }
    sprintf(buf, "Player: %-12s (%s) [%2d %s]\r\n", GET_NAME(vict),
      genders[(int) GET_SEX(vict)], GET_LEVEL(vict), class_abbrevs[(int)
	GET_CLASS(vict)]);
    sprintf(buf,
         "Au: %-8d  Bal: %-8d  Exp: %-8d  Align: %-5d  Lessons: %-3d\r\n",
	    GET_GOLD(vict), GET_BANK_GOLD(vict), GET_EXP(vict),
	    GET_ALIGNMENT(vict), GET_PRACTICES(vict));
    strcpy(birth, ctime(&vict->player.time.birth));
    sprintf(buf,
            "Started: %-20.16s  Last: %-20.16s  Played: %3dh %2dm\r\n",
	    birth, ctime(&vict->player.time.logon),
	    (int) (vict->player.time.played / 3600),
	    (int) (vict->player.time.played / 60 % 60));
    send_to_char(buf, ch);
    free_char(vict);
    break;
  case 3: /* show rent */
    Crash_listrent(ch, value);
    break;
  case 4: /* show stats */
    i = 0;
    j = 0;
    k = 0;
    con = 0;

    for (vict = character_list; vict; vict = vict->next) {
      if (IS_NPC(vict))
        j++;
      else if (CAN_SEE(ch, vict)) {
        i++;
        if (vict->desc)
          con++;
      }
    }
    for (obj = object_list; obj; obj = obj->next)
      k++;
    strcpy(buf, "Current stats:\r\n");
    sprintf(buf + strlen(buf), "  %5d players in game  %5d connected\r\n",
                i, con);
    sprintf(buf + strlen(buf), "  %5d registered\r\n",
                top_of_p_table + 1);
    sprintf(buf + strlen(buf), "  %5d mobiles          %5d prototypes\r\n",
                j, top_of_mobt + 1);
    sprintf(buf + strlen(buf), "  %5d objects          %5d prototypes\r\n",
                k, top_of_objt + 1);
    sprintf(buf + strlen(buf), "  %5d rooms            %5d zones\r\n",
                top_of_world + 1, top_of_zone_table + 1);
    sprintf(buf + strlen(buf), "  %5d large bufs\r\n",
                buf_largecount);
    sprintf(buf + strlen(buf), "  %5d buf switches     %5d overflows\r\n",
                buf_switches, buf_overflows);
    send_to_char(buf, ch);
    break;
  case 5: /* show errors */
    strcpy(buf, "Errant Rooms\r\n------------\r\n");
    for (i = 0, k = 0; i <= top_of_world; i++)
      for (j = 0; j < NUM_OF_DIRS; j++)
        if (world[i].dir_option[j] && world[i].dir_option[j]->to_room == 0)
          sprintf(buf + strlen(buf), "%2d: [%5d] %s\r\n", ++k, GET_ROOM_VNUM(i),
                  world[i].name);
    page_string(ch->desc, buf, TRUE);
    break;
  case 6: /* show death */
    strcpy(buf, "Death Traps\r\n-----------\r\n");
    for (i = 0, j = 0; i <= top_of_world; i++)
      if (ROOM_FLAGGED(i, ROOM_DEATH))
        sprintf(buf + strlen(buf), "%2d: [%5d] %s\r\n", ++j,
                GET_ROOM_VNUM(i), world[i].name);
    page_string(ch->desc, buf, TRUE);
    break;
  case 7: /* show godrooms */
    strcpy(buf, "Godrooms\r\n--------------------------\r\n");
    for (i = 0, j = 0; i < top_of_world; i++)
    if (ROOM_FLAGGED(i, ROOM_GODROOM))
      sprintf(buf + strlen(buf), "%2d: [%5d] %s\r\n",
                ++j, GET_ROOM_VNUM(i), world[i].name);
    page_string(ch->desc, buf, TRUE);
    break;
  case 8: /* show shops */
    show_shops(ch, value);
    break;
  case 9: /* show houses */
    hcontrol_list_houses(ch);
    break;
  case 10: /* show players */
    strcpy(buf, "\r\nDarkest WarDome MUD registered players:\r\n");
    for (j = 0; j <= top_of_p_table; j++){
	if(find_name((player_table + j)->name) >= 0)
          sprintf(buf + strlen(buf), "[%3d] %-19.19s%s", (player_table + j)->level, (player_table + j)->name, !(++count % 3) ? "\r\n" : "");
    }
    strcat(buf, (count % 3) ? "\r\n" : "");
    sprintf(buf + strlen(buf), "\r\nTotal active players: %d\r\n", count);
    page_string(ch->desc, buf, 1);
    break;
  case 11: /* show rooms <zone> */
    if (self) {
      for (i = 0; i <= top_of_world; i++)
        if ((int)(GET_ROOM_VNUM(i) / 100) == zone_table[world[ch->in_room].zone].number) {
          count++;
          if (count == 1 || !(count % 21 - 1)) sprintf(buf + strlen(buf), "      vnum   Room name\r\n");
          sprintf(buf + strlen(buf), "%3d. [%5d] %s\r\n", count, GET_ROOM_VNUM(i), world[i].name);
        }
    }
    else if (*value && is_number(value)) {
      for (j = atoi(value), i = 0; zone_table[i].number != j && i <= top_of_zone_table; i++);
        if (i <= top_of_zone_table) {
          for (i = 0; i <= top_of_world; i++)
            if ((int)(GET_ROOM_VNUM(i) / 100) == atoi(value)) {
              count++;
              if (count == 1 || !(count % 21 - 1)) sprintf(buf + strlen(buf), "      vnum   Room name\r\n");
              sprintf(buf + strlen(buf), "%3d. [%5d] %s\r\n", count, GET_ROOM_VNUM(i), world[i].name);
            }
        }
        else {
          send_to_char("Invalid zone number.\r\n", ch);
          return;
        }
    }
    else {
      send_to_char("Invalid zone number.\r\n", ch);
      return;
    }
    page_string(ch->desc, buf, TRUE);
    break;
  case 12: /* show entrances */
    if (self) {
      sprintf(buf, "Possible entrances to the room \"%s\" (%d):\r\n",
              world[ch->in_room].name, GET_ROOM_VNUM(ch->in_room));
      for (i = 0; i <= top_of_world; i++)
        for (j = 0; j < NUM_OF_DIRS; j++)
          if (world[i].dir_option[j] && GET_ROOM_VNUM(world[i].dir_option[j]->to_room) == GET_ROOM_VNUM(ch->in_room))
            sprintf(buf + strlen(buf), "%2d. [%5d] (%5s) %s\r\n", ++count, GET_ROOM_VNUM(i), dirs[j], world[i].name);
    }
    else {
      send_to_char("Sorry, only \"show entrances .\" is working.\r\n", ch);
      return;
      /*
      sprintf(buf, "Possible entrances to the room \"%s\" (%d):\r\n",
              world[ch->in_room].name, GET_ROOM_VNUM(ch->in_room));
      for (i = 0; i <= top_of_world; i++)
        for (j = 0; j < NUM_OF_DIRS; j++)
          if (world[i].dir_option[j] && GET_ROOM_VNUM(world[i].dir_option[j]->to_room) == GET_ROOM_VNUM(ch->in_room))
            sprintf(buf + strlen(buf), "[%5d] %s (%s)\r\n", GET_ROOM_VNUM(i), world[i].name, dirs[j]);
      */
    }
    page_string(ch->desc, buf, TRUE);
    break;
  case 13:
    *buf = '\0';
    send_to_char("People currently snooping:\r\n", ch);
    send_to_char("--------------------------\r\n", ch);
    for (d = descriptor_list; d; d = d->next) {
      if (d->snooping == NULL || d->character == NULL)
	continue;
      if (STATE(d) != CON_PLAYING || GET_LEVEL(ch) < GET_LEVEL(d->character))
	continue;
      if (!CAN_SEE(ch, d->character) || IN_ROOM(d->character) == NOWHERE)
	continue;
      sprintf(buf + strlen(buf), "%-10s - snooped by %s.\r\n",
               GET_NAME(d->snooping->character), GET_NAME(d->character));
    }
    send_to_char(*buf ? buf : "No one is currently snooping.\r\n", ch);
    break;
  case 14:      /* teleport */
    strcpy(buf, "Teleport Rooms\r\n--------------\r\n");
    for (i = 0, j = 0; i <= top_of_world; i++)
        if (world[i].tele != NULL)
          sprintf(buf, "%s%2d: [%5d] %-24.24s Targ: %5d\r\n", buf, ++j,
                  world[i].number, world[i].name, world[i].tele->targ);
    page_string(ch->desc, buf, 1);
    break;
  case 15: /* show memory */
    show_mreport(ch);
    break;
 case 16:
 {
  struct char_data *victim = NULL;
 strcpy(buf, "&wThe &YRichests &wPlayers of &BWardome &bMud:&n\r\n");
 for (j = 0; j <= top_of_p_table; j++){
    CREATE(victim, struct char_data, 1);
   	 clear_char(victim);
   	 CREATE(victim->player_specials, struct player_special_data, 1);
    if(load_char((player_table + j)->name, victim) > -1)  
 	
 	 
 	   if ((GET_LEVEL(victim) < 201) && GET_BANK_GOLD(victim) > 500000)
 	   sprintf(buf + strlen(buf), "&R[&r%3d&R] &WName: &w%-19.19s  &yGold:&Y%-10d    &mBank:&M%-10d \r\n", (player_table + j)->level, (player_table + j)->name, victim->points.gold, victim->points.bank_gold); 
	  
	    
    }
    page_string(ch->desc, buf, TRUE);
    free_char(victim);
    }
    break;
    case 17:
 {
  struct char_data *victim = NULL;
 strcpy(buf, "&wThe Clan Member:&n\r\n");
 for (j = 0; j <= top_of_p_table; j++){
    CREATE(victim, struct char_data, 1);
   	 clear_char(victim);
   	 CREATE(victim->player_specials, struct player_special_data, 1);
    if(load_char((player_table + j)->name, victim) > -1)  
 	
 	 
 	   if ((GET_LEVEL(victim) < 201) /*&& (GET_CLAN(victim) > 0)*/ && (GET_IDNUM(victim) > -1))
 	   sprintf(buf + strlen(buf), "&R[&r%3d&R] &WName: &w%-19.19s  &yClan: [%d]    &mIDnum:&M%-10d \r\n", (player_table + j)->level, (player_table + j)->name, GET_CLAN(victim), (player_table + j)->id);
	  
	    
    }
    page_string(ch->desc, buf, TRUE);
    free_char(victim);
    }
    break;
	
  default:
    send_to_char("Invalid SHOW option.\r\n", ch);
    break;
}
}

/***************** The do_set function ***********************************/

#define PC   1
#define NPC  2
#define BOTH 3

#define MISC    0
#define BINARY  1
#define NUMBER  2

#define SET_OR_REMOVE(flagset, flags) { \
        if (on) SET_BIT(flagset, flags); \
        else if (off) REMOVE_BIT(flagset, flags); }

#define RANGE(low, high) (value = MAX((low), MIN((high), (value))))


/* The set options available */
  struct set_struct {
    const char *cmd;
    int level;
    const char pcnpc;
    const char type;
  } set_fields[] = {
/* 00 */ { "brief",           LVL_GOD,        PC,     BINARY },
/* 01 */ { "invstart",        LVL_GOD,        PC,     BINARY },
/* 02 */ { "title",           LVL_GOD,        PC,     MISC   },
/* 03 */ { "nosummon",        LVL_GRGOD,      PC,     BINARY },
/* 04 */ { "maxhit",          LVL_GRGOD,      BOTH,   NUMBER },
/* 05 */ { "maxmana",         LVL_GRGOD,      BOTH,   NUMBER },
/* 06 */ { "maxmove",         LVL_GRGOD,      BOTH,   NUMBER },
/* 07 */ { "hit",             LVL_GOD,        BOTH,   NUMBER },
/* 08 */ { "mana",            LVL_GOD,        BOTH,   NUMBER },
/* 09 */ { "move",            LVL_GOD,        BOTH,   NUMBER },
/* 10 */ { "align",           LVL_GOD,        BOTH,   NUMBER },
/* 11 */ { "str",             LVL_GRGOD,      BOTH,   NUMBER },
/* 12 */ { "stradd",          LVL_GRGOD,      BOTH,   NUMBER },
/* 13 */ { "int",             LVL_GRGOD,      BOTH,   NUMBER },
/* 14 */ { "wis",             LVL_GRGOD,      BOTH,   NUMBER },
/* 15 */ { "dex",             LVL_GRGOD,      BOTH,   NUMBER },
/* 16 */ { "con",             LVL_GRGOD,      BOTH,   NUMBER },
/* 17 */ { "cha",             LVL_GRGOD,      BOTH,   NUMBER },
/* 18 */ { "ac",              LVL_GRGOD,      BOTH,   NUMBER },
/* 19 */ { "gold",            LVL_GRGOD,      BOTH,   NUMBER },
/* 20 */ { "bank",            LVL_GRGOD,      PC,     NUMBER },
/* 21 */ { "exp",             LVL_SUPGOD,     BOTH,   NUMBER },
/* 22 */ { "hitroll",         LVL_GRGOD,      BOTH,   NUMBER },
/* 23 */ { "damroll",         LVL_GRGOD,      BOTH,   NUMBER },
/* 24 */ { "invis",           LVL_IMPL,       PC,     NUMBER },
/* 25 */ { "nohassle",        LVL_GRGOD,      PC,     BINARY },
/* 26 */ { "frozen",          LVL_FREEZE,     PC,     BINARY },
/* 27 */ { "practices",       LVL_GRGOD,      PC,     NUMBER },
/* 28 */ { "lessons",         LVL_GRGOD,      PC,     NUMBER },
/* 29 */ { "drunk",           LVL_GRGOD,      BOTH,   MISC   },
/* 30 */ { "hunger",          LVL_GRGOD,      BOTH,   MISC   },
/* 31 */ { "thirst",          LVL_GRGOD,      BOTH,   MISC   },
/* 32 */ { "killer",          LVL_GOD,        PC,     BINARY },
/* 33 */ { "thief",           LVL_GOD,        PC,     BINARY },
/* 34 */ { "level",           LVL_MJGOD,     BOTH,   NUMBER },
/* 35 */ { "room",            LVL_SUPGOD,     BOTH,   NUMBER },
/* 36 */ { "siteok",          LVL_GRGOD,      PC,     BINARY },
/* 37 */ { "deleted",         LVL_SUPGOD,     PC,     BINARY },
/* 38 */ { "class",           LVL_GRGOD,      BOTH,   MISC   },
/* 39 */ { "nowizlist",       LVL_GOD,        PC,     BINARY },
/* 40 */ { "quest",           LVL_GOD,        PC,     BINARY },
/* 41 */ { "loadroom",        LVL_GRGOD,      PC,     MISC   },
/* 42 */ { "color",           LVL_GOD,        PC,     BINARY },
/* 43 */ { "idnum",           LVL_GOD,       PC,     NUMBER },
/* 44 */ { "passwd",          LVL_SUPGOD,     PC,     MISC   },
/* 45 */ { "nodelete",        LVL_GOD,        PC,     BINARY },
/* 46 */ { "sex",             LVL_GRGOD,      BOTH,   MISC   },
/* 47 */ { "age",             LVL_GRGOD,      BOTH,   NUMBER },
/* 48 */ { "olc",             LVL_SUPGOD,     PC,     NUMBER },
/* 49 */ { "crossdoors",      LVL_SUPGOD,     PC,     BINARY },
/* 50 */ { "name",            LVL_IMPL,       PC,     MISC   },
/* 51 */ { "points",          LVL_GRGOD,      PC,     NUMBER },
/* 52 */ { "email",           LVL_SUPGOD,     PC,     MISC   },
/* 53 */ { "race",            LVL_GRGOD,      PC,     MISC   },
/* 54 */ { "remort",          LVL_SUPGOD,     PC,     NUMBER },
/* 55 */ { "fbi",             LVL_SUPGOD,     PC,     NUMBER },
/* 56 */ { "nopk",	      LVL_SUPGOD,     PC,     BINARY },
/* 57 */ { "qnum",	      LVL_GOD,        PC,     NUMBER },
/* 58 */ { "maxmental",       LVL_SUPGOD,     BOTH,   NUMBER },
/* 59 */ { "qpoints",         205,            PC,     NUMBER },
/* 60 */ { "house",           LVL_GRGOD,      PC,     NUMBER },
/* 61 */ { "rpoints",	      LVL_GRGOD,      PC,     NUMBER },
/* 62 */ { "banned",	      LVL_GOD,        PC,     BINARY },
/* 63 */// { "cpoints",	      LVL_GOD,        PC,     NUMBER },
/* 64 */ { "secondplayer",    LVL_GOD,	      PC,     BINARY },
/* 65 */ { "firstplayer",     LVL_GOD,        PC,     NUMBER },
/* 66 */// { "icqnumber",       LVL_GOD,        PC,     NUMBER },
    { "\n", 0, BOTH, MISC }
  };


int perform_set(struct char_data *ch, struct char_data *vict, int mode,
                char *val_arg)
{
  int i, on = 0, off = 0, value = 0;
  char output[MAX_STRING_LENGTH];
  char out_to_log2[MAX_STRING_LENGTH];
  char out_to_log1[MAX_STRING_LENGTH];
//  char out_to_log0[MAX_STRING_LENGTH];

  /* Check to make sure all the levels are correct */
  if (GET_LEVEL(ch) != LVL_IMPL) {
    if (!IS_NPC(vict) && GET_LEVEL(ch) <= GET_LEVEL(vict) && vict != ch) {
      send_to_char("Maybe that's not such a great idea...\r\n", ch);
      return 0;
    }
  }
  if (GET_LEVEL(ch) < set_fields[mode].level) {
    send_to_char("You are not godly enough for that!\r\n", ch);
    return 0;
  }

  /* Make sure the PC/NPC is correct */
  if (IS_NPC(vict) && !(set_fields[mode].pcnpc & NPC)) {
    send_to_char("You can't do that to a beast!\r\n", ch);
    return 0;
  }
  else if (!IS_NPC(vict) && !(set_fields[mode].pcnpc & PC)) {
    send_to_char("That can only be done to a beast!\r\n", ch);
    return 0;
  }

  /* Find the value of the argument */
  if (set_fields[mode].type == BINARY) {
    if (!strcmp(val_arg, "on") || !strcmp(val_arg, "yes"))
      on = 1;
    else if (!strcmp(val_arg, "off") || !strcmp(val_arg, "no"))
      off = 1;
    if (!(on || off)) {
      send_to_char("Value must be 'on' or 'off'.\r\n", ch);
      return 0;
    }
    sprintf(output, "(set) %s sets %s %s for %s.", GET_NAME(ch), set_fields[mode].cmd, ONOFF(on),
            GET_NAME(vict));
    mudlog(output, CMP, MAX(GET_INVIS_LEV(ch), LVL_SUBIMPL), TRUE);
  }
  else if (set_fields[mode].type == NUMBER) {
    value = atoi(val_arg);
    sprintf(output, "(set) %s sets %s's %s set to %d.", GET_NAME(ch), GET_NAME(vict),
            set_fields[mode].cmd, value);
    mudlog(output, CMP, MAX(GET_INVIS_LEV(ch), LVL_SUBIMPL), TRUE);
  }
  else {
    strcpy(output, "Okay.");  /* can't use OK macro here 'cause of \r\n */
  }

  switch (mode) {
  case 0: /* brief */
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_BRIEF);
    break;
  case 1: /* invstart */
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_INVSTART);
    break;
  case 2: /* title */
    sprintf(out_to_log1, "%s sets %s's title from '%s' to '%s'",
            GET_NAME(ch), GET_NAME(vict), GET_TITLE(vict), val_arg);
    set_title(vict, val_arg);
    break;
  case 3: /* nosummon */
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_SUMMONABLE);
    break;
  case 4: /* maxhit */
    vict->points.max_hit = RANGE(1, 500000);
    affect_total(vict);
    break;
  case 5: /* maxmana */
    vict->points.max_mana = RANGE(1, 500000);
    affect_total(vict);
    break;
  case 6: /* maxmove */
    vict->points.max_move = RANGE(1, 500000);
    affect_total(vict);
    break;
  case 7: /* hit */
    vict->points.hit = RANGE(-9, vict->points.max_hit);
    affect_total(vict);
    break;
  case 8: /* mana */
    vict->points.mana = RANGE(0, vict->points.max_mana);
    affect_total(vict);
    break;
  case 9: /* move */
    vict->points.move = RANGE(0, vict->points.max_move);
    affect_total(vict);
    break;
  case 10: /* alignment */
    GET_ALIGNMENT(vict) = RANGE(-1000, 1000);
    affect_total(vict);
    break;
  case 11: /* str */
      RANGE(3, 100);
    vict->real_abils.str = value;
    vict->real_abils.str_add = 0;
    affect_total(vict);
    break;
  case 12: /* stradd */
    vict->real_abils.str_add = RANGE(0, 100);
    if (value > 0)
      vict->real_abils.str = 18;
    affect_total(vict);
    break;
  case 13: /* int */
      RANGE(3, 100);
    vict->real_abils.intel = value;
    affect_total(vict);
    break;
  case 14: /* wis */
    RANGE(3, 100);
    vict->real_abils.wis = value;
    sprintf(out_to_log1, out_to_log2, vict->real_abils.wis);
    affect_total(vict);
    break;
  case 15: /* dex */
    RANGE(3, 100);
    vict->real_abils.dex = value;
    affect_total(vict);
    break;
  case 16: /* con */
    RANGE(3, 100);
    vict->real_abils.con = value;
    affect_total(vict);
    break;
  case 17: /* cha */
    RANGE(3, 100);
    vict->real_abils.cha = value;
    affect_total(vict);
    break;
  case 18: /* ac */
    vict->points.armor = RANGE(-300, 300);
    affect_total(vict);
    break;
  case 19: /* gold */
    GET_GOLD(vict) = RANGE(0, 100000000);
    break;
  case 20: /* bank */
    GET_BANK_GOLD(vict) = RANGE(0, 100000000);
    break;
  case 21: /* exp */
//    vict->points.exp = RANGE(0, 18000000000);
    GET_EXP(vict) = RANGE(0, 2005000000);
    break;
  case 22: /* hitroll */
    vict->points.hitroll = RANGE(-1200, 1200);
    affect_total(vict);
    break;
  case 23: /* damroll */
    vict->points.damroll = RANGE(-1200, 1200);
    affect_total(vict);
    break;
  case 24: /* invis */
    if (GET_LEVEL(ch) < LVL_IMPL && ch != vict) {
      send_to_char("You aren't godly enough for that!\r\n", ch);
      return 0;
    }
    GET_INVIS_LEV(vict) = RANGE(0, GET_LEVEL(vict));
    break;
  case 25: /* nohassle */
    if (GET_LEVEL(ch) < LVL_IMPL && ch != vict) {
      send_to_char("You aren't godly enough for that!\r\n", ch);
      return 0;
    }
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_NOHASSLE);
    break;
  case 26: /* frozen */
    if (ch == vict) {
      send_to_char("Better not -- could be a long winter!\r\n", ch);
      return 0;
    }
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_FROZEN);
    break;
  case 27: /* practices */
  case 28: /* lessons */
    GET_PRACTICES(vict) = RANGE(0, 100);
    break;
  case 29: /* drunk */
    if (mode == 29) strcpy(out_to_log1, "DRUNK");
  case 30: /* hunger */
    if (mode == 30) strcpy(out_to_log1, "HUNGER");
  case 31: /* thirst */
    if (mode == 31) strcpy(out_to_log1, "THIRST");
    if (GET_COND(vict, (mode - 29)) > -1)
      sprintf(out_to_log1, out_to_log2, GET_COND(vict, (mode - 29)));
    if (!str_cmp(val_arg, "off")) {
      GET_COND(vict, (mode - 29)) = (char) -1; /* warning: magic number here */
      sprintf(out_to_log1, out_to_log2, "off");
    }
    else if (is_number(val_arg)) {
      value = atoi(val_arg);
      RANGE(0, 24);
      GET_COND(vict, (mode - 29)) = (char) value; /* and here too */
      sprintf(out_to_log1, out_to_log2, "%d");
      sprintf(out_to_log2, out_to_log1, GET_COND(vict, (mode - 29)));
      strcpy(out_to_log1, out_to_log2);
    }
    else {
      send_to_char("Must be 'off' or a value from 0 to 24.\r\n", ch);
      return 0;
    }
    break;
  case 32: /* killer */
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_KILLER);
    break;
  case 33: /* thief */
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_THIEF);
    break;
  case 34: /* level */
    if (value > GET_LEVEL(ch) || value > LVL_IMPL) {
      send_to_char("You can't do that.\r\n", ch);
      return 0;
    }
    RANGE(0, LVL_IMPL);
    vict->player.level = value;
    break;
  case 35: /* room */
    if ((i = real_room(value)) < 0) {
      send_to_char("No room exists with that number.\r\n", ch);
      return 0;
    }
    if (IN_ROOM(vict) != NOWHERE) {      /* Another Eric Green special. */
      char_from_room(vict);
    }
    char_to_room(vict, i);
    break;
  case 36: /* siteok */
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_SITEOK);
    break;
  case 37: /* deleted */
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_DELETED);
    break;
  case 38: /* class */
    if ((i = parse_class(*val_arg)) == CLASS_UNDEFINED) {
      send_to_char("That is not a class.\r\n", ch);
      return 0;
    }
    sprintf(out_to_log2, "%s changes %s's CLASS from '%s' to '%%s'.", GET_NAME(ch),
            GET_NAME(vict), pc_class_types[(int) GET_CLASS(vict)]);
    GET_CLASS(vict) = i;
    sprintf(out_to_log1, out_to_log2, pc_class_types[(int) GET_CLASS(vict)]);
    break;
  case 39: /* nowizlistable */
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_NOWIZLIST);
    break;
  case 40: /* quest */
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_QUEST);
    break;
  case 41: /* loadroom */
    if (!str_cmp(val_arg, "off")) {
      REMOVE_BIT(PLR_FLAGS(vict), PLR_LOADROOM);
    } else if (is_number(val_arg)) {
      value = atoi(val_arg);
      if (real_room(value) != NOWHERE) {
        SET_BIT(PLR_FLAGS(vict), PLR_LOADROOM);
        GET_LOADROOM(vict) = value;
        sprintf(output, "%s will enter at room #%d.", GET_NAME(vict),
                GET_LOADROOM(vict));
      } else {
        send_to_char("That room does not exist!\r\n", ch);
        return 0;
      }
    } else {
      send_to_char("Must be 'off' or a room's virtual number.\r\n", ch);
      return 0;
    }
    break;
  case 42: /* color */
    SET_OR_REMOVE(PRF_FLAGS(vict), (PRF_COLOR_1 | PRF_COLOR_2));
    break;
  case 43: /* idnum */
    GET_IDNUM(vict) = value;
    break;
  case 44: /* password */
    if (GET_LEVEL(vict) == LVL_IMPL && ch != vict) {
      send_to_char("You cannot change that.\r\n", ch);
      return 0;
    }
    strncpy(GET_PASSWD(vict), CRYPT(val_arg, GET_NAME(vict)), MAX_PWD_LENGTH);
    *(GET_PASSWD(vict) + MAX_PWD_LENGTH) = '\0';
    sprintf(output, "Password changed to '%s'.", val_arg);
    break;
  case 45: /* nodelete */
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_NODELETE);
    break;
  case 46: /* sex */
    if (!str_cmp(val_arg, "male"))
      vict->player.sex = SEX_MALE;
    else if (!str_cmp(val_arg, "female"))
      vict->player.sex = SEX_FEMALE;
    else if (!str_cmp(val_arg, "neutral"))
      vict->player.sex = SEX_NEUTRAL;
    else {
      send_to_char("Must be 'male', 'female', or 'neutral'.\r\n", ch);
      return 0;
    }
    break;
  case 47: /* age */
    if (value < 2 || value > 200) {     /* Arbitrary limits. */
      send_to_char("Ages 2 to 200 accepted.\r\n", ch);
      return 0;
    }
    /*
     * NOTE: May not display the exact age specified due to the integer
     * division used elsewhere in the code.  Seems to only happen for
     * some values below the starting age (17) anyway. -gg 5/27/98
     */
    ch->player.time.birth = time(0) - ((value - 17) * SECS_PER_MUD_YEAR);
    break;
  case 48: /* olc */
    GET_OLC_ZONE(vict) = value;
    break;

  case 49: /* crossdoors */
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_CROSSDOORS);
    break;

  case 50: /* name */
   /* strncpy(GET_NAME(vict), val_arg,  MAX_NAME_LENGTH);
    *(GET_NAME(vict) + MAX_NAME_LENGTH) = '\0';
    sprintf(output, "Name changed to '%s'.", val_arg);
    */
    
    break;

  case 51: /* points */
    GET_POINTS(vict) = value;
    break;

  case 52: /* email */
 // sprintf(buf, "%s has changed %s's email to: %s", GET_NAME(ch), GET_NAME(vict), val_arg);
 //   mudlog(buf, NRM, LVL_IMMORT, 0);
 //   set_email(vict, val_arg);
 //  GET_EMAIL(vict) = val_arg;
 //  save_char(vict, NOWHERE);
 //   break;

  case 53:
    if ((i = parse_race(*val_arg)) == RACE_UNDEFINED) {
      send_to_char("That is not a race.\r\n", ch);
      return 0;
    }
    GET_RACE(vict) = i;
    break;

  case 54:
    if ((value < 0) || (value > 99)) {
      send_to_char("Values between 0 and 99 are allowed.\r\n", ch);
      return 0;
    }
    GET_REMORT(vict) = value;
    break;

  case 55:
    if ((value < 0) || (value > 3)) {
      send_to_char("Values between 0 and 3 are allowed.\r\n", ch);
      return 0;
    }
    GET_FBI_POS(vict) = value;
    break;
  case 56:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_NOPK);
    break;
  case 57:
    GET_QUEST(vict) = value;
    break;
  case 58: /* maxmental */
    vict->points.max_mental = RANGE(1, 500000);
    affect_total(vict);
    break;
  case 59: /* qpoints */
    GET_QP(vict) = value;
	break;
  case 60: /* House vnum */
    HOUSE(vict) = value;
	break;
  case 61:
    GET_RP(vict) = RANGE(0, 10000);
	break;
  case 62:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_BANNED);
    break;
  case 63:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_SECPLAYER);
 	break;
  case 64:
    GET_SECOND(vict) = value;
    sprintf(buf, "%s has changed %s's first char name to: %s", GET_NAME(ch), GET_NAME(vict), get_name_by_id(value));
    mudlog(buf, NRM, LVL_IMMORT, 0);
    break;
 // case 65:
   // GET_ICQ(vict) = value;
    
    //break; 
  //case 63:
  //  clans[GET_CLAN(vict)].points = value;
  //  break;  	
  default:
    send_to_char("Can't set that!\r\n", ch);
    return 0;
  }
  return 1;
}


ACMD(do_set)
{
  struct char_data *vict = NULL, *cbuf = NULL;
  char field[MAX_INPUT_LENGTH], name[MAX_INPUT_LENGTH],
        val_arg[MAX_INPUT_LENGTH];
  int mode = -1, len = 0, player_i = 0, retval;
  char is_file = 0, is_mob = 0, is_player = 0;
  int i = 0;
  int conta = 0;

  half_chop(argument, name, buf);

  if (!strcmp(name, "file")) {
    is_file = 1;
    half_chop(buf, name, buf);
  } else if (!str_cmp(name, "player")) {
    is_player = 1;
    half_chop(buf, name, buf);
  } else if (!str_cmp(name, "mob")) {
    is_mob = 1;
    half_chop(buf, name, buf);
  }
  half_chop(buf, field, buf);
  strcpy(val_arg, buf);

  if (!*name || !*field) {
    send_to_char("Usage: set <victim> <field> <value>\r\n", ch);
    sprintf(buf, "As %s %s you can set the following fields:\r\n",
                 strchr("aeiouyAEIOUY", *GET_GOD_NAME(ch))?"an":"a",
                 GET_GOD_NAME(ch));
    send_to_char(buf, ch);
    conta=0;
    for (i=0; set_fields[i].cmd != "\n"; i++) {
      if (GET_LEVEL(ch) >= set_fields[i].level) {
        conta=(conta==4?1:conta+1);
        sprintf(buf, "[%3d] %-13s%s", set_fields[i].level,
                set_fields[i].cmd, (conta==4?"\r\n":" "));
        send_to_char(buf, ch);
      }
    }
    if (conta != 4) send_to_char("\r\n", ch);
    return;
  }

  /* find the target */
  if (!is_file) {
    if (is_player) {
      if (!(vict = get_player_vis(ch, name, 0))) {
        send_to_char("There is no such player.\r\n", ch);
        return;
      }
    } else {
      if (!(vict = get_char_vis(ch, name))) {
        send_to_char("There is no such creature.\r\n", ch);
        return;
      }
    }
  }
  else if (is_file) {
    if (mode == 35 || mode == 51) {
      send_to_char("You can't set this in the playerfile.\r\n", ch);
      return;
    }
    /* try to load the player off disk */
    CREATE(cbuf, struct char_data, 1);
    clear_char(cbuf);
    CREATE(cbuf->player_specials, struct player_special_data, 1);
    if ((player_i = load_char(name, cbuf)) > -1) {
      if (GET_LEVEL(cbuf) >= GET_LEVEL(ch)) {
        free_char(cbuf);
        send_to_char("Sorry, you can't do that.\r\n", ch);
        return;
      }
      vict = cbuf;
    }
    else {
      free_char(cbuf);
      send_to_char("There is no such player.\r\n", ch);
      return;
    }
  }

  /* find the command in the list */
  len = strlen(field);
  for (mode = 0; *(set_fields[mode].cmd) != '\n'; mode++)
    if (!strncmp(field, set_fields[mode].cmd, len))
      break;

  /* perform the set */
  retval = perform_set(ch, vict, mode, val_arg);

  /* save the character if a change was made */
  if (retval) {
    if (!is_file && !IS_NPC(vict))
      save_char(vict, NOWHERE);
    if (is_file) {
      GET_PFILEPOS(cbuf) = player_i;
      save_char(cbuf, GET_LOADROOM(cbuf));
      send_to_char("Saved in file.\r\n", ch);
    }
  }

  /* free the memory if we allocated it earlier */
  if (is_file)
    free_char(cbuf);
}


int find_skill_num(char *name);
extern struct spell_info_type spell_info[];

char *two_argumentss(char *argument, char *first_arg, char *second_arg)
{
  return one_word(one_word(argument, first_arg), second_arg); /* :-) */
}

int was_class(struct char_data *ch, int num)
{
  int x, c = -1;

     for(x = 1; x <= GET_REMORT(ch); x++)
        if(GET_CLASS_REMORT(ch, x) == num)
		c++;

  return (c);
}

int already_have(struct char_data *ch, int num)
{
  int w, x, r;

     r = (GET_REMORT(ch) > 0 ? GET_REMORT(ch) : 1);
     for(x = 0; x <= r; x++)
	  for(w = 0; w < MAX_RSKILLS; w++)
              if(GET_RSKILL(ch, x, w) == num)
	         return (num);

  return (0);
}

#define REMORT_TEXT \
"WarDome II - REMORT SYSTEM\r\n" \
"\r\n" \
"First of all, you can choose a maximum of 5 skills to keep after remorting:\r\n" \
"   remort skill <skill\\spell> -> the complete name\r\n" \
"\r\n" \
"After that, choose a class to remort with:\r\n" \
"   remort class <class letter>\r\n" \
"To see the letter of your future class, type: 'remort class'. It will show\r\n" \
"a list of classes and their letters.\r\n" \
"\r\n" \
"Every class you remort will be recorded. The same with your remort\r\n" \
"skills. Each skill you pick costs 1,000,000 and class 2,000,000. Choose\r\n" \
"wisely.\r\n" \
"\r\n" \
"&YCOST&n: A complete remort costs 7,000,000 coins.\r\n" \
"&RWARNING&n: Do not save in the remort process, you can lose gold.\r\n"

ACMD(do_remort)
{
  int x = 0, y = 0, i, w, c =  0, t = 0;
  int parse_class(char arg);
  char argum[MAX_INPUT_LENGTH], argum1[MAX_INPUT_LENGTH];
  ACMD(do_rskills);

  strcpy(argum1, one_argument(argument, argum));

  if (GET_LEVEL(ch) > LVL_GOREMORT) {
  	send_to_char("You are too experienced to remort.\r\n", ch);
        return;
  }

  if (GET_LEVEL(ch) < LVL_GOREMORT) {
	if(GET_REMORT(ch)) {
		do_rskills(ch, NULL, 0, 0);
		return;
	} else {
	  	send_to_char("You need more experience to remort.\r\n", ch);
        	return;
	}
  }

  if(!str_cmp(argum, "class")) {
    if(GET_GOLD(ch) < 200000) {
       	send_to_char("You need 200,000 gold in cash to set a class.\r\n", ch);
       	return;
    }
    if(!*argum1) {
          send_to_char("&WPick a class to remort:&n\r\n\r\n", ch);
          for(i = 0; i < NUM_CLASSES; i++)
   	   if(allowed_classes(GET_RACE(ch), i)) {
   	   	sprintf(buf,"		%s %s%s\r\n", pc_clsl_types[i], (was_class(ch, i) != -1 ? "!Remorted! " : ""), (GET_CLASS(ch) == i ? "*Current Class*" : ""));
              	send_to_char(buf, ch);
   	   }
	  return;
    }

    y = GET_CLASS(ch);
    if (((x = parse_class(argum1[1])) == CLASS_UNDEFINED) ||
      !allowed_classes(GET_RACE(ch), x)){
      send_to_char("That is not a class.\r\n", ch);
      return;
    }

    if (GET_CLASS(ch) == x) {
      send_to_char("You can't remort with your current class.\r\n", ch);
      return;
    }

      for (w = 0; w < NUM_WEARS; w++){
        if (GET_EQ(ch, w))
            obj_to_char(unequip_char(ch, w), ch);
      }

        GET_INVIS_LEV(ch) = 0;
    	GET_CLASS(ch) = x;
	GET_REMORT(ch) += 1;
	GET_LEVEL(ch) = 1;
	GET_MAX_HIT(ch) = MAX(1, GET_MAX_HIT(ch) * 0.02);
	GET_MAX_MANA(ch) = MAX(1, GET_MAX_MANA(ch) * 0.02);
	GET_MAX_MOVE(ch) = MAX(1, GET_MAX_MOVE(ch) * 0.02);
	GET_MAX_MENTAL(ch) = 1;
	GET_HIT(ch) = GET_MAX_HIT(ch);
	GET_MOVE(ch) = GET_MAX_MOVE(ch);
	GET_MANA(ch) = GET_MAX_MANA(ch);
	GET_MENTAL(ch) = GET_MAX_MENTAL(ch);
	GET_EXP(ch) = 0;
	GET_POINTS(ch) = 9+(GET_REMORT(ch)/2);
        GET_STYLE(ch) = 2 ; // pode ser qq estilo que nao seja pro(3) ;
	GET_GOLD(ch) -= 200000;
	GET_CLASS_REMORT(ch, (int)GET_REMORT(ch)) = y;

	if (PRF_FLAGGED(ch, PRF_NOHASSLE))
	  REMOVE_BIT(PRF_FLAGS(ch), PRF_NOHASSLE);
	if (PRF_FLAGGED(ch, PRF_HOLYLIGHT))
	  REMOVE_BIT(PRF_FLAGS(ch), PRF_HOLYLIGHT);
	if (PRF_FLAGGED(ch, PRF_LOG1))
	  REMOVE_BIT(PRF_FLAGS(ch), PRF_LOG1);
	if (PRF_FLAGGED(ch, PRF_LOG2))
	  REMOVE_BIT(PRF_FLAGS(ch), PRF_LOG2);
	if (PRF_FLAGGED(ch, PRF_CROSSDOORS))
	  REMOVE_BIT(PRF_FLAGS(ch), PRF_CROSSDOORS);
	if (!PLR_FLAGGED(ch, PLR_NODELETE))
          SET_BIT(PLR_FLAGS(ch), PLR_NODELETE);

	for (t = 1; t < MAX_SKILLS+1; t++)
		GET_SKILL_LS(ch, t) = 0;

        actualize_splskl(ch);
        Crash_crashsave(ch);
        save_char(ch, NOWHERE);
	check_autowiz(ch);

	if (ch->followers || ch->master)
	 die_follower(ch);

        sprintf(buf, "(REMORT) %s was %s, remorted with %s", GET_NAME(ch), pc_class_types[(int) y], pc_class_types[(int) x]);
        mudlog(buf, BRF, LVL_GRGOD, TRUE);

	send_to_char("\r\n"
		"You feel your blood bubbling in your heart.\r\n"
		"You can see the death angel waiting for you.\r\n"
		"The death angel takes your spirit and disappears...\r\n"
		"The death angel appears in front of you and recite some legendary words.\r\n"
		"You feel your spirit in a strange new body.\r\n"
		"You now begin a new life, with the power of the death angel!\r\n"
		"\r\n"
		"You are the next remort generation of WarDome II.\r\n", ch);
  } else if(!str_cmp(argum, "skill")) {
        if(GET_GOLD(ch) < 100000) {
           send_to_char("You need 100,000 gold in cash to set a skill.\r\n", ch);
           return;
        }

	if(!*argum1) {
	  send_to_char("&WPick a skill to remort with.&n\r\n", ch);
	  return;
	}

	c = GET_CLASS(ch);
	if(((y = find_skill_num(argum1)) == -1) ||
	    spell_info[y].min_level[c] > GET_LEVEL(ch)) {
	  send_to_char("&WYou have not that skill.&n\r\n", ch);
	  return;
	}
/* trava msm skill...
	if(already_have(ch, y)) {
	  send_to_char("&WYou already have this remort skill.&n\r\n", ch);
	  return;
	} */
	for(w = 0; w < MAX_RSKILLS && !x; w++) {
            if(!GET_RSKILL(ch, ((int)GET_REMORT(ch)+1), w)) {
	       x++;
	       GET_RSKILL(ch, ((int)GET_REMORT(ch)+1), w) = y;
	       GET_GOLD(ch) -= 100000;
	       sprintf(buf, "&R%d &Wability: &c%s&W.&n\r\n", (w+1), spells[GET_RSKILL(ch, (int)GET_REMORT(ch)+1, w)]);
	       send_to_char(buf, ch);
	    }
	}
	if(!x)
	   send_to_char("&WYou already have &R5&W remort &cskills&W.&n\r\n", ch);
  } else
     send_to_char(REMORT_TEXT, ch);
}

ACMD(do_chown)
{
  struct char_data *victim;
  struct obj_data *obj;
  char buf2[80];
  char buf3[80];
  int i, k = 0;

  two_arguments(argument, buf2, buf3);

  if (!*buf2)
    send_to_char("Syntax: chown <object> <character>.\r\n", ch);
  else if (!(victim = get_char_vis(ch, buf3)))
    send_to_char("No one by that name here.\r\n", ch);
  else if (victim == ch)
    send_to_char("Are you sure you're feeling ok?\r\n", ch);
  else if (GET_LEVEL(victim) >= GET_LEVEL(ch))
    send_to_char("That's really not such a good idea.\r\n", ch);
  else if (!*buf3)
    send_to_char("Syntax: chown <object> <character>.\r\n", ch);
  else {
    for (i = 0; i < NUM_WEARS; i++) {
      if (GET_EQ(victim, i) && CAN_SEE_OBJ(ch, GET_EQ(victim, i)) &&
         isname(buf2, GET_EQ(victim, i)->name)) {
        obj_to_char(unequip_char(victim, i), victim);
        k = 1;
      }
    }

  if (!(obj = get_obj_in_list_vis(victim, buf2, victim->carrying))) {
    if (!k && !(obj = get_obj_in_list_vis(victim, buf2, victim->carrying))) {
      sprintf(buf, "%s does not appear to have the %s.\r\n", GET_NAME(victim), buf2);
      send_to_char(buf, ch);
      return;
    }
  }

  act("$n makes a magical gesture and $p flies from $N to $m.",FALSE,ch,obj,
       victim,TO_NOTVICT);
  act("$n makes a magical gesture and $p flies away from you to $m.",FALSE,ch,
       obj,victim,TO_VICT);
  act("You make a magical gesture and $p flies away from $N to you.",FALSE,ch,
       obj, victim,TO_CHAR);

  obj_from_char(obj);
  obj_to_char(obj, ch);
  save_char(ch, NOWHERE);
  save_char(victim, NOWHERE);
  }

}

ACMD(do_xname)
{
   char tempname[MAX_INPUT_LENGTH];
   int i = 0;
   FILE *fp;
   *buf = '\0';

   one_argument(argument, buf);

   if(!*buf)
      send_to_char("Xname which name?\r\n", ch);
   if(!(fp = fopen(XNAME_FILE, "a"))) {
      perror("Problems opening xname file for do_xname");
      return;
   }
   strcpy(tempname, buf);
   for (i = 0; tempname[i]; i++)
      tempname[i] = LOWER(tempname[i]);
   fprintf(fp, "%s\n", tempname);
   fclose(fp);
   sprintf(buf1, "%s has been xnamed!\r\n", tempname);
   send_to_char(buf1, ch);
   Read_Invalid_List();
}

ACMD(do_seelog)
{
  FILE *req_file;
  int cur_line = 0,
      num_lines = 0,
      req_lines = 0,
      i;
  int l, count = 0;
  char field[MAX_INPUT_LENGTH],
       value[MAX_INPUT_LENGTH];

  struct file_struct {
    char *cmd;
    int level;
    char *file;
  } fields[] = {
    { "none",           LVL_IMPL,    "Does Nothing" },
    { "syslog",         LVL_IMPL,    "../syslog" },
    { "crash",          LVL_MJGOD,  "../syslog.CRASH" },
    { "levels",         LVL_GRGOD,   "../log/levels" },
    { "rip",            LVL_GRGOD,   "../log/rip" },
    { "new players",    LVL_GRGOD,   "../log/newplayers" },
    { "rentgone",       LVL_GRGOD,   "../log/rentgone" },
    { "godcmds",        LVL_SUBIMPL,    "../log/godcmds" },
    { "sets",           LVL_SUBIMPL,    "../log/sets" },
    { "remort",         LVL_GRGOD,   "../log/remort" },
    { "pkilling",       LVL_GRGOD,   "../log/pkilling" },
    { "errors",         LVL_MJGOD,  "../log/errors" },
    { "dts",            LVL_GRGOD,   "../log/dts" },
    { "restarts",       LVL_SUBIMPL,  "../log/restarts" },
    { "badpws",         LVL_GRGOD,  "../log/badpws" },
    { "delete",         LVL_SUPGOD,  "../log/delete" },
    { "olc",		LVL_OLC_FULL,  "../log/olc"},
    { "help",		LVL_SUPGOD,  "../log/help"},
    { "bug",	        LVL_MJGOD,   "../lib/misc/bugs"},
    { "typo",		LVL_SUPGOD,   "../lib/misc/typos"},
    { "ideas",		LVL_MJGOD,   "../lib/misc/ideas"},
    { "xnames",		LVL_SUPGOD,  "../lib/misc/xnames"},
    { "quests",		LVL_OLC_FULL,  "../log/quest"},
    { "revive",		LVL_MJGOD,  "../log/revive"},
    { "\n", 0, "\n" }
  };

  skip_spaces(&argument);

  if (!*argument) {
    sprintf(buf, "As %s %s you can access this logs:\r\n",
         strchr("aeiouyAEIOUY", *GET_GOD_NAME(ch))?"an":"a", GET_GOD_NAME(ch));
    send_to_char(buf, ch);
    count=0;
    for (i=1; fields[i].cmd != "\n"; i++) {
      if (GET_LEVEL(ch) >= fields[i].level) {
        count=(count==4?1:count+1);
        sprintf(buf, "[%3d] %-13s%s", fields[i].level,
                fields[i].cmd, (count==4?"\r\n":" "));
        send_to_char(buf, ch);
      }
    }
    if (count != 4) send_to_char("\r\n", ch);
    return;
  }

  strcpy(arg, two_arguments(argument, field, value));

  for (l = 0; *(fields[l].cmd) != '\n'; l++)
    if (!strncmp(field, fields[l].cmd, strlen(field)))
      break;

  if(*(fields[l].cmd) == '\n')
  {
    send_to_char("That is not a valid option!\r\n", ch);
    return;
  }

  if (GET_LEVEL(ch) < fields[l].level)
  {
    send_to_char("You are not godly enough to view that file!\r\n", ch);
    return;
  }

  if(!*value)
     req_lines = 60;
  else
     req_lines = atoi(value);

  if (!(req_file=fopen(fields[l].file,"r")))
  {
     sprintf(buf2, "SYSERR: Error opening file %s using 'seelog' command.",
             fields[l].file);
     mudlog(buf2, BRF, LVL_IMPL, TRUE);
     return;
  }

  get_line(req_file,buf);
  while (!feof(req_file))
  {
     num_lines++;
     get_line(req_file,buf);
  }
  fclose(req_file);


  if(req_lines > num_lines) req_lines = num_lines;
  if(req_lines > 15000) req_lines = 15000;


  if (!(req_file=fopen(fields[l].file,"r")))
  {
     sprintf(buf2, "SYSERR: Error opening file %s using 'seelog' command.",
             fields[l].file);
     mudlog(buf2, BRF, LVL_IMPL, TRUE);
     return;
  }

  buf2[0] = '\0';

  get_line(req_file,buf);
  while (!feof(req_file))
  {
     cur_line++;
     if(cur_line > (num_lines - req_lines))
     {
        sprintf(buf2,"%s%s\r\n",buf2, buf);
     }
     get_line(req_file,buf);
   }
   page_string(ch->desc, buf2, 1);


   fclose(req_file);
}

ACMD(do_tedit)
{
   int l, i;
   char field[MAX_INPUT_LENGTH];
   extern char *credits;
   extern char *news;
   extern char *motd;
   extern char *imotd;
   extern char *help;
   extern char *info;
   extern char *background;
   extern char *handbook;
   extern char *policies;

   struct editor_struct {
      char *cmd;
      int level;
      char *buffer;
      int  size;
      char *filename;
   } fields[] = {
      /* edit the lvls to your own needs */
	{ "credits",	LVL_IMPL,	credits,	2400,	CREDITS_FILE},
	{ "news",	LVL_MJGOD,	news,		8192,	NEWS_FILE},
	{ "motd",	LVL_GRGOD,	motd,		2400,	MOTD_FILE},
	{ "imotd",	LVL_GRGOD,	imotd,		2400,	IMOTD_FILE},
	{ "phelp",      LVL_SUPGOD,	help,		2400,	HELP_PAGE_FILE},
	{ "info",	LVL_SUBIMPL,	info,		8192,	INFO_FILE},
	{ "qotw",	LVL_GOD,	background,	8192,	BACKGROUND_FILE},
	{ "handbook",   LVL_IMPL,	handbook,	8192,   HANDBOOK_FILE},
	{ "policies",	LVL_MJGOD,	policies,	8192,	POLICIES_FILE},
	{ "\n",		0,		NULL,		0,	NULL }
   };

   if (ch->desc == NULL) {
      send_to_char("Get outta here you linkdead head!\r\n", ch);
      return;
   }

   half_chop(argument, field, buf);


   if (!*field) {
      sprintf(buf, "As %s %s you can edit:\r\n",
           strchr("aeiouyAEIOUY", *GET_GOD_NAME(ch))?"an":"a", GET_GOD_NAME(ch));
      i = 1;
      for (l = 0; *fields[l].cmd != '\n'; l++) {
	 if (GET_LEVEL(ch) >= fields[l].level) {
	    sprintf(buf, "%s[%3d] %-13.13s%s", buf, fields[l].level, fields[l].cmd, (i == 4 ? "\r\n" : " "));
	    i++;
	 }
      }
      if (--i % 4) strcat(buf, "\r\n");
      if (i == 0) strcat(buf, "None.\r\n");
      send_to_char(buf, ch);
      return;
   }

   for (l = 0; *(fields[l].cmd) != '\n'; l++)
     if (!strncmp(field, fields[l].cmd, strlen(field)))
     break;

   if (*fields[l].cmd == '\n') {
      send_to_char("Invalid text editor option.\r\n", ch);
      return;
   }

   if (GET_LEVEL(ch) < fields[l].level) {
      send_to_char("You are not godly enough for that!\r\n", ch);
      return;
   }

   switch (l) {
    case 0: ch->desc->str = &credits; break;
    case 1: ch->desc->str = &news; break;
    case 2: ch->desc->str = &motd; break;
    case 3: ch->desc->str = &imotd; break;
    case 4: ch->desc->str = &help; break;
    case 5: ch->desc->str = &info; break;
    case 6: ch->desc->str = &background; break;
    case 7: ch->desc->str = &handbook; break;
    case 8: ch->desc->str = &policies; break;
    default:
      send_to_char("Invalid text editor option.\r\n", ch);
      return;
   }

   /* set up editor stats */
   send_to_char("\x1B[J", ch);
   send_to_char("Now, edit the file: (/s saves /h for help)\r\n", ch);
   ch->desc->backstr = NULL;
   if (fields[l].buffer) {
      send_to_char(fields[l].buffer, ch);
      ch->desc->backstr = str_dup(fields[l].buffer);
   }
   ch->desc->max_str = fields[l].size;
   ch->desc->mail_to = 0;
   ch->desc->storage = str_dup(fields[l].filename);
   act("$n starts using OLC.", TRUE, ch, 0, 0, TO_ROOM);
   STATE(ch->desc) = CON_TEXTED;
   SET_BIT(PLR_FLAGS(ch), PLR_WRITING);

}

extern int top_of_objt;
extern int top_of_mobt;
extern int top_of_world;
extern int top_of_helpt;
extern int top_of_socialt;
extern int top_of_aquestt;
extern int top_shop;
extern int top_of_zone_table;
extern int top_of_p_file;

void show_mreport(struct char_data *ch)
{
  int i, x = 0, j = 0;
  struct char_data *vict;

  for (vict = character_list; vict; vict = vict->next)
    if (vict->desc)
      j++;

  for (i = 0; i <= top_of_zone_table; i++)
      x++;

  strcpy(buf, "Memory Report\r\n");
  strcat(buf, "-------------\r\n");
  sprintf(buf, "%s          %7s %4s %10s\r\n", buf, "Number", "Byte", "Total Used");
  sprintf(buf, "%sPlayers  :%7d %4d %10d\r\n", buf, j, sizeof(struct char_data) + sizeof(struct descriptor_data), (j) * (sizeof(struct char_data) + sizeof(struct descriptor_data)));
  sprintf(buf, "%sObjects  :%7d %4d %10d\r\n", buf, top_of_objt + 1, sizeof(struct obj_data), (top_of_objt + 1) * sizeof(struct obj_data));
  sprintf(buf, "%sMobProto :%7d %4d %10d\r\n", buf, top_of_mobt + 1, sizeof(struct char_data), (top_of_mobt + 1) * sizeof(struct char_data));
  sprintf(buf, "%sRooms    :%7d %4d %10d\r\n", buf, top_of_world + 1, sizeof(struct room_data), (top_of_world + 1) * sizeof(struct room_data));
  sprintf(buf, "%sHelp     :%7d %4d %10d\r\n", buf, top_of_helpt + 1, sizeof(struct help_index_element), (top_of_helpt + 1) * sizeof(struct help_index_element));
  sprintf(buf, "%sSocials  :%7d %4d %10d\r\n", buf, top_of_socialt + 1, sizeof(struct command_info), (top_of_socialt + 1) * sizeof(struct command_info));
  sprintf(buf, "%sQuests   :%7d \r\n", buf, top_of_aquestt + 1);
  sprintf(buf, "%sShops    :%7d \r\n", buf, top_shop + 1);
  sprintf(buf, "%sZones    :%7d %4d %10d\r\n", buf, top_of_zone_table + 1, sizeof(struct zone_data), (top_of_zone_table + 1) * sizeof(struct zone_data));
  send_to_char(buf, ch);
}

ACMD(do_playerlink)
{
  struct char_data *victim = NULL;
  char *action = arg, *vict = buf2;

  two_arguments(argument, action, vict);

  if (!*action || !*vict) {
    send_to_char("Usage: playerlink [ load | remove ] <player>\r\n", ch);
    return;
  }
  if (is_abbrev(action, "load")) {
    if (get_player_vis(ch, vict, 0)) {
      send_to_char("That player is already connected/loaded!\r\n", ch);
      return;
    }
    CREATE(victim, struct char_data, 1);
    clear_char(victim);
    CREATE(victim->player_specials, struct player_special_data, 1);
    if (load_char(vict, victim) > -1) {
      if (GET_LEVEL(victim) < GET_LEVEL(ch)) {
	Crash_load(victim);
	victim->next = character_list;
	character_list = victim;
	victim->desc = NULL;
	char_to_room(victim, ch->in_room);
	act("You gesture and a statue of $N forms from nothingness.",
	    FALSE, ch, 0, victim, TO_CHAR);
	act("$n gestures and a statue of $N appears from nowhere.", FALSE,
	    ch, 0, victim, TO_NOTVICT);
	/* read_aliases(victim); */
	sprintf(buf, "(GC) %s has loaded %s.", GET_NAME(ch),
		GET_NAME(victim));
	mudlog(buf, BRF, MAX(LVL_IMPL, GET_INVIS_LEV(ch)), TRUE);
      } else {
	send_to_char("Sorry, you aren't high enough level to load that char.\r\n", ch);
	free_char(victim);
      }
    } else {
      send_to_char("No such player.\r\n", ch);
      free(victim);
    }
  } else if (is_abbrev(action, "remove")) {
    if (!(victim = get_player_vis(ch, vict, 0))) {
      send_to_char("That player isn't in the game right now.\r\n", ch);
      return;
    } else if (victim->desc) {
      send_to_char("That player isn't linkdead.\r\n", ch);
      return;
    }
    act("You gesture and $N dissolves into nothingness.", FALSE, ch, 0,
	victim, TO_CHAR);
    act("$n gestures and $N dissolves into nothingness.", FALSE, ch, 0,
	victim, TO_NOTVICT);
    sprintf(buf, "(GC) %s has unloaded %s.", GET_NAME(ch), GET_NAME(victim));
   if (free_rent)
      Crash_rentsave(victim, 0);
      extract_char(victim);           /* Char is saved in extract char */
    mudlog(buf, BRF, MAX(LVL_IMPL, GET_INVIS_LEV(ch)), TRUE);
  } else {
  send_to_char("That isn't an option.\r\n", ch);
  return;
  }
}

extern int mother_desc, port;
void Crash_rentsave(struct char_data * ch, int cost);

#define EXE_FILE "bin/circle" /* maybe use argv[0] but it's not reliable */

/* (c) 1996-97 Erwin S. Andreasen <erwin@andreasen.org> */
ACMD(do_copyover)
{
	FILE *fp;
	struct descriptor_data *d, *d_next;
	char buf [100], buf2[100];

	fp = fopen (COPYOVER_FILE, "w");

	if (!fp)
	{
		send_to_char ("Copyover file not writeable, aborted.\n\r",ch);
		return;
	}

	/* Consider changing all saved areas here, if you use OLC */
	sprintf (buf, "\n\r*** WARDOME BOOT SYSTEM - Please remain seated! ***\n\r");

	/* For each playing descriptor, save its state */
	for (d = descriptor_list; d ; d = d_next)
	{
		struct char_data * och = d->character;
		d_next = d->next; /* We delete from the list , so need to save this */

		if (!d->character || d->connected > CON_PLAYING) /* drop those logging on */
		{
            write_to_descriptor (d->descriptor, "\n\rSorry, we are rebooting. Come back in a few minutes.\n\r");
			close_socket (d); /* throw'em out */
		}
		else
		{
			fprintf (fp, "%d %s %s\n", d->descriptor, GET_NAME(och), d->host);

        	    /* save och */
			write_aliases(och);
			save_char(och, och->in_room);
			Crash_crashsave(och);
			write_to_descriptor (d->descriptor, buf);
		}
	}

	fprintf (fp, "-1\n");
	fclose (fp);

	/* exec - descriptors are inherited */

	sprintf (buf, "%d", port);
    sprintf (buf2, "-C%d", mother_desc);

    /* Ugh, seems it is expected we are 1 step above lib - this may be dangerous! */
    chdir ("..");

	execl (EXE_FILE, "circle", buf2, buf, (char *) NULL);

	/* Failed - sucessful exec will not return */

	perror ("do_copyover: execl");
	send_to_char ("Copyover FAILED!\n\r",ch);

    exit (1); /* too much trouble to try to recover! */
}

ACMD(do_veq)
{
  char field[MAX_INPUT_LENGTH], value[MAX_INPUT_LENGTH];
  int i, j, l;

void vwear_object(int wearpos, struct char_data * ch)
{

  int nr, found = 0;

  *buf = '\0';
  for (nr = 0; nr <= top_of_objt; nr++) {
     if(GET_OBJ_TYPE(&obj_proto[nr]) == ITEM_KEY)
           continue;
     else if(wearpos == ITEM_WEAR_HOLD && (GET_OBJ_TYPE(&obj_proto[nr]) == ITEM_LIGHT ||
            GET_OBJ_TYPE(&obj_proto[nr]) == ITEM_WEAPON))
	   continue;
     else if(wearpos == ITEM_WEAR_TAKE && GET_OBJ_TYPE(&obj_proto[nr]) != ITEM_LIGHT)
	   continue;

     if (CAN_WEAR(&obj_proto[nr], wearpos))
        sprintf(buf+strlen(buf), "%3d. [%5d] %s\r\n", ++found,
          obj_index[nr].vnum, obj_proto[nr].short_description);

  }
  page_string(ch->desc, buf, 1);

}

struct listeq_struct {
    char *cmd;
    int level;
}
/* Change the LVL_GOD1 to your appropriate god levels */
  fields[] = {
    { "nothing", LVL_ELDER },
    { "finger", LVL_ELDER },
    { "neck", LVL_ELDER },
    { "body", LVL_ELDER },
    { "head", LVL_ELDER },
    { "legs", LVL_ELDER },
    { "feet", LVL_ELDER },
    { "hands", LVL_ELDER },
    { "shield", LVL_ELDER },
    { "arms", LVL_ELDER },
    { "about", LVL_ELDER },
    { "waist", LVL_ELDER },
    { "wrist", LVL_ELDER },
    { "wield", LVL_ELDER },
    { "hold", LVL_ELDER },
    { "ear", LVL_ELDER },
    { "face", LVL_ELDER },
    { "float", LVL_ELDER },
    { "light", LVL_ELDER },
/* Add any extra eq types here */
    { "\n", 0 }
  };

  skip_spaces(&argument);

  if (!*argument) {
    strcpy(buf, "List eq options:\r\n");
    for (j = 0, i = 1; fields[i].level; i++)
      if (fields[i].level <= GET_LEVEL(ch))
        sprintf(buf, "%s%-15s%s", buf, fields[i].cmd, (!(++j % 5) ? "\r\n" : ""));
    strcat(buf, "\r\n");
    send_to_char(buf, ch);
    return;
  }
  strcpy(arg, two_arguments(argument, field, value));

  for (l = 0; *(fields[l].cmd) != '\n'; l++)
    if (!strncmp(field, fields[l].cmd, strlen(field)))
      break;

  if (GET_LEVEL(ch) < fields[l].level) {
    send_to_char("You are not godly enough for that!\r\n", ch);
    return;
  }
  buf[0] = '\0';
  switch (l) {
/* The order is based on the above list.  Try and keep the same order */
/* otherwise you will get what you DON'T want */
    case 1:                     /* Finger eq */
      vwear_object(ITEM_WEAR_FINGER, ch);
      break;
    case 2:                     /* Neck eq */
      vwear_object(ITEM_WEAR_NECK, ch);
      break;
    case 3:                     /* Body eq */
      vwear_object(ITEM_WEAR_BODY, ch);
      break;
    case 4:                     /* Head eq */
      vwear_object(ITEM_WEAR_HEAD, ch);
      break;
    case 5:                     /* Leg eq */
      vwear_object(ITEM_WEAR_LEGS, ch);
      break;
    case 6:                     /* Foot eq */
      vwear_object(ITEM_WEAR_FEET, ch);
      break;
    case 7:                     /* Hand eq */
      vwear_object(ITEM_WEAR_HANDS, ch);
      break;
    case 8:                       /* Shield eq */
      vwear_object(ITEM_WEAR_SHIELD, ch);
      break;
    case 9:                       /* Arm eq */
      vwear_object(ITEM_WEAR_ARMS, ch);
      break;
    case 10:                       /* Worn about body eq */
      vwear_object(ITEM_WEAR_ABOUT, ch);
      break;
    case 11:                       /* Waist eq */
      vwear_object(ITEM_WEAR_WAIST, ch);
      break;
    case 12:                       /* Wrist eq */
      vwear_object(ITEM_WEAR_WRIST, ch);
      break;
    case 13:                       /* Wielded objects */
      vwear_object(ITEM_WEAR_WIELD, ch);
      break;
    case 14:                       /* Held eq */
      vwear_object(ITEM_WEAR_HOLD, ch);
      break;
    case 15:                       /* Ear eq */
      vwear_object(ITEM_WEAR_EAR, ch);
      break;
    case 16:                       /* Face eq */
      vwear_object(ITEM_WEAR_FACE, ch);
      break;
    case 17:                       /* Float eq */
      vwear_object(ITEM_WEAR_FLOAT, ch);
      break;
    case 18:                       /* Float eq */
      vwear_object(ITEM_WEAR_TAKE, ch);
      break;
/* You add any other eq types here */
    default:
      send_to_char("Come again?\r\n", ch);
      break;
  }
}

int zona_do_mob(int vnum)
{
	short int zona = 0;

	zona = (vnum - (vnum % 100)) / 100;

	return (zona);
}

ACMD(do_mobfix)
{
	int mob_num, zone, mobs = 0, remorts;
	char zona[20], remort[20];

	half_chop(argument, zona, remort);

	if(!*zona)
	{
		send_to_char("Formato: mobfix <zona> <remort>.\r\n", ch);
		return;
	}

	if(!*remort)
		remorts = 0;
	else
	{
		remorts = atoi(remort);
		if(remorts < 0)
		{
			send_to_char("Nao existe mob com remort negativo.\r\n", ch);
			return;
		}
	}
	zone = atoi(zona);

	for (mob_num = 0; mob_num <= top_of_mobt; mob_num++)
	{
		if(zona_do_mob(mob_index[mob_num].vnum) == zone)
		{
			GET_REMORT(mob_proto + mob_num) = remorts;
			mob_proto[mob_num].points.hit = 1;
			mob_proto[mob_num].points.mana = 1;
			if(GET_LEVEL(mob_proto + mob_num) < 70 && GET_REMORT(mob_proto + mob_num) == 0)
				mob_proto[mob_num].points.move = ((((7+(GET_REMORT(mob_proto + mob_num)*2))*GET_LEVEL(mob_proto + mob_num))/3)*12)*0.75;
			else
				mob_proto[mob_num].points.move = (((7+(GET_REMORT(mob_proto + mob_num)*2))*GET_LEVEL(mob_proto + mob_num))/3)*12;
			GET_EXP(mob_proto + mob_num) = (level_exp(GET_REMORT(mob_proto + mob_num), GET_LEVEL(mob_proto + mob_num))/17.5);
			GET_GOLD(mob_proto + mob_num) = (GET_LEVEL(mob_proto + mob_num)*15);
			mob_proto[mob_num].points.damroll = GET_REMORT(mob_proto + mob_num)*10 + (GET_LEVEL(mob_proto + mob_num)/5)*2;
			mob_proto[mob_num].points.hitroll = GET_REMORT(mob_proto + mob_num)*2 + (GET_LEVEL(mob_proto + mob_num)/2);
			mob_proto[mob_num].mob_specials.damnodice = 5;
			mob_proto[mob_num].mob_specials.damsizedice = (GET_LEVEL(mob_proto + mob_num)/10 * ((100 + (GET_REMORT(mob_proto + mob_num) * 20))/100)) + 1;
			mob_proto[mob_num].points.armor = (100 + (GET_REMORT(mob_proto + mob_num)*(-30)) + ((GET_LEVEL(mob_proto + mob_num)/2)*(-3)));

			switch (mob_proto[mob_num].player.chclass) {
			case CLASS_MAGIC_USER:
			case CLASS_WARLOCK:
				GET_EXP(mob_proto + mob_num) = GET_EXP(mob_proto + mob_num)*1.1;
				mob_proto[mob_num].points.max_mana = (GET_LEVEL(mob_proto + mob_num)*25);
				if (mob_index[GET_MOB_RNUM(mob_proto + mob_num)].func == NULL)
					mob_index[GET_MOB_RNUM(mob_proto + mob_num)].func = magic_user;
			break;
			case CLASS_NINJA:
				if (mob_index[GET_MOB_RNUM(mob_proto + mob_num)].func == NULL)
					mob_index[GET_MOB_RNUM(mob_proto + mob_num)].func = ninja;
			break;
			case CLASS_THIEF:
				if (mob_index[GET_MOB_RNUM(mob_proto + mob_num)].func == NULL)
					mob_index[GET_MOB_RNUM(mob_proto + mob_num)].func = thief;
			break;
			case CLASS_BARBARIAN:
				if (mob_index[GET_MOB_RNUM(mob_proto + mob_num)].func == NULL)
					mob_index[GET_MOB_RNUM(mob_proto + mob_num)].func = barbarian;
			break;
			case CLASS_SORCERER:
			case CLASS_CLERIC:
			case CLASS_NECROMANCER:
			case CLASS_PSIONICIST:
				GET_EXP(mob_proto + mob_num) = GET_EXP(mob_proto + mob_num)*1.1;
				mob_proto[mob_num].points.max_mana = (GET_LEVEL(mob_proto + mob_num)*21);
			break;
			default:
			break;
			}
			mobs++;
		}
	}
	if(!mobs)
	{
		send_to_char("No mobs were changed.\r\n", ch);
		return;
	}
	else
	{
		sprintf(buf, "&WMobs changed: &R%dR%d&W.&n\r\n&WDo not &Rforget &Wto save zone info. &cType &COLC&c.&n\r\n", mobs, remorts);
		send_to_char(buf, ch);
		olc_add_to_save_list(zone, OLC_SAVE_MOB);
	}
}

ACMD(do_peace)
{
        struct char_data *vict, *next_v;
        act ("$n decides that everyone should just be friends.",
                FALSE,ch,0,0,TO_ROOM);
        send_to_room("Everything is quite peaceful now.\r\n",ch->in_room);
        for(vict=world[ch->in_room].people;vict;vict=next_v)
        {
                next_v=vict->next_in_room;
              if (FIGHTING(vict))
                {
                if(FIGHTING(FIGHTING(vict))==vict)
                        stop_fighting(FIGHTING(vict));
                stop_fighting(vict); 

                }
        }
}
ACMD(do_makeall)
{
	send_to_char("Compilando arquivos...\r\n", ch);
	send_to_char("Aguarde 5 minutos ate' rebootar o mud...\r\n", ch);
 	 send_to_char("\r\nExecutando \'Make all'\r\n", ch);
	sprintf(buf, "&g(GC) %s compilou o Mud.&n", GET_NAME(ch));
 	 log(buf);
 	 send_to_char(buf, ch);
	
	 
 	 system("cd /home/wardome/wdII/src ; make all"); //faz o make all
 	  
}

ACMD(do_backup)
{

	argument = one_argument(argument, arg);;
	
if (!*arg)
  {
    send_to_char("Backup: <pfiles> ou <plrobj>\r\n", ch);
    return;
  }

	if (!str_cmp(arg, "pfiles")) { 
  send_to_char("Tentanto fazer backup do Pfile..\r\n", ch); 


  sprintf(buf, "(GC) %s fez backup do Pfile.", GET_NAME(ch));
  log(buf);
 	send_to_char("\r\nExecutando \"tar zcf pfiles.tar pfiles\"\r\n", ch);

  system("tar zcf /home/wardome/wdII/lib/pfiles.tar.gz /home/wardome/wdII/lib/pfiles/*"); 
  send_to_char("O comando foi executado mas nao se sabe se  "
               "obteve exito.\r\n", ch);

  sprintf(buf, "Movendo o arquivo \"pfiles.tar.gz\" para pasta: %s\r\n",
          pasta);
  send_to_char(buf, ch);
  //sprintf(buf, "mv /home/wardome/wdII/lib/pfile.tar %s", pasta);
 // system(buf); 

 // send_to_char("O comando \"mv\" foi executado mas nao se sabe se "
       //        "\"pfile.tar\" foi movido.\r\n", ch);

				
	} else if (!str_cmp(arg, "plrobj")) { // else if ao inves de if, -ips
  send_to_char("Tentanto fazer backup do plrobjs..\r\n", ch); 


  sprintf(buf, "(GC) %s fez backup do Plrobjs.", GET_NAME(ch));
  log(buf);
 	send_to_char("\r\nExecutando \"tar zcf plrobjs.tar pfiles\"\r\n", ch);

  system("tar zcf /home/wardome/wdII/lib/plrobjs.tar.gz /home/wardome/wdII/lib/plrobjs/*"); 
  send_to_char("O comando foi executado mas nao se sabe se  "
               "obteve exito.\r\n", ch);

  sprintf(buf, "Movendo o arquivo \"plrobjs.tar.gz\" para pasta: %s\r\n",
          pasta);
  send_to_char(buf, ch);
	}
}
