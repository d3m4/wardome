/***************************************************************************
 *  OasisOLC - olc.c 		                                           *
 *    				                                           *
 *  Copyright 1996 Harvey Gilpin.                                          *
 ***************************************************************************/
 
#define _OASIS_OLC_
 
#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "buffer.h"
#include "interpreter.h"
#include "comm.h"
#include "utils.h"
#include "db.h"
#include "olc.h"
#include "dg_olc.h"
#include "screen.h"
#include "handler.h"
#include "quest.h"

/*
 * External data structures.
 */
extern struct obj_data *obj_proto;
extern struct char_data *mob_proto;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct room_data *world;
extern int top_of_zone_table;
extern struct social_messg *soc_mess_list;
extern int top_of_socialt;
extern int top_of_world;
extern int rev_dir[];
extern const char *dirs[];
extern struct zone_data *zone_table;
extern struct descriptor_data *descriptor_list;
  
/*
 * External functions.
 */
extern int zedit_setup(struct descriptor_data *d, int room_num);
extern int zedit_save_to_disk(int zone);
extern int zedit_new_zone(struct char_data *ch, int new_zone);
extern int medit_setup_new(struct descriptor_data *d);
extern int medit_setup_existing(struct descriptor_data *d, int rmob_num);
extern int medit_save_to_disk(int zone);
extern int redit_setup_new(struct descriptor_data *d);
extern int redit_setup_existing(struct descriptor_data *d, int rroom_num);
extern void redit_save_internally(struct descriptor_data *d);
extern int redit_save_to_disk(int zone);
extern int oedit_setup_new(struct descriptor_data *d);
extern int oedit_setup_existing(struct descriptor_data *d, int robj_num);
extern int oedit_save_to_disk(int zone);
extern int sedit_setup_new(struct descriptor_data *d);
extern int sedit_setup_existing(struct descriptor_data *d, int robj_num);
extern int sedit_save_to_disk(int zone);
extern int real_shop(int vnum);
extern int free_shop(struct shop_data *shop);
extern int free_room(struct room_data *room);
extern void medit_free_mobile(struct char_data *mob);
extern void trigedit_setup_new(struct descriptor_data *d);
extern void trigedit_setup_existing(struct descriptor_data *d, int rtrg_num);
extern int real_trigger(int vnum);
extern int hedit_save_to_disk(void);
extern int free_help(struct help_index_element *help);
int find_help_rnum(char *keyword);
extern int hedit_setup_new(struct descriptor_data *d, char *new_key);
extern int hedit_setup_existing(struct descriptor_data *d, int rnum);
extern int aedit_save_to_disk(struct descriptor_data *d);
extern int free_action(struct social_messg *action);
extern int find_action(int cmd);
int is_name(const char *str, const char *namelist);
extern void qedit_setup_new(struct descriptor_data *d);
extern void qedit_setup_existing(struct descriptor_data *d, int real_num);
extern void qedit_save_to_disk(int znum);
extern void free_quest(struct aq_data *quest);

/*
 * Internal function prototypes.
 */
int can_edit_zone(struct char_data *ch, int number);
int real_zone(int number);
void olc_saveinfo(struct char_data *ch);

/*
 * Global string constants.
 */
const char *save_info_msg[9] = {"Rooms", "Objects", "Zone info",
	"Mobiles", "Shops", "Triggers", "Help", "Actions", "Quests"}; 
 
/*
 * Internal data structures.
 */

struct olc_scmd_data {
  char *text;
  int con_type;
};
  
struct olc_scmd_data olc_scmd_info[9] =
{
  {"room", CON_REDIT},
  {"object", CON_OEDIT},
  {"room", CON_ZEDIT},
  {"mobile", CON_MEDIT},
  {"shop", CON_SEDIT},
  {"help", CON_HEDIT},
  {"action", CON_AEDIT},
  {"trigger", CON_TRIGEDIT},
  {"quest", CON_QEDIT}
};
  
 /*------------------------------------------------------------*/
 
 /*
  * Exported ACMD do_olc function.
  *
  * This function is the OLC interface.  It deals with all the 
  * generic OLC stuff, then passes control to the sub-olc sections.
  */
  
  ACMD(do_olc)
  {
   int number = -1, save = 0, real_num;
   struct descriptor_data *d;
 
   /*
    * No screwing around as a mobile.
    */
   if (IS_NPC(ch))
      return;
  
   if (subcmd == SCMD_OLC_SAVEINFO) {
     olc_saveinfo(ch);
      return;
    }
  
   /*
    * Parse any arguments.
    */
   two_arguments(argument, buf1, buf2);
   if (!*buf1) {		/* No argument given. */
     switch (subcmd) {
     case SCMD_OLC_ZEDIT:
     case SCMD_OLC_REDIT:
       number = world[IN_ROOM(ch)].number;
       break;
     case SCMD_OLC_TRIGEDIT:
     case SCMD_OLC_OEDIT:
     case SCMD_OLC_MEDIT:
     case SCMD_OLC_SEDIT:
     case SCMD_OLC_QEDIT:
       sprintf(buf, "Specify a %s VNUM to edit.\r\n", olc_scmd_info[subcmd].text);
       send_to_char(buf, ch);
       return;
     case SCMD_OLC_HEDIT:
       /*
        * Altered as a nitpick.  Why waste a sprintf() call when you know
        * that it's going to be a help entry?  TR 5-18-98
        */
       send_to_char("Specify a help entry to edit.\r\n", ch);
       return;
     case SCMD_OLC_AEDIT:
       send_to_char("Specify an action to edit.\r\n", ch);
       return;
     }
   } else if (!isdigit(*buf1)) {
     if (strn_cmp("save", buf1, 4) == 0) {
       if ((subcmd == SCMD_OLC_HEDIT) || (subcmd == SCMD_OLC_AEDIT)) {
         save = 1;
         number = 0;
       } else if (!*buf2) {
 	 send_to_char("Save which zone?\r\n", ch);
   	 return;
       } else {
 	 save = 1;
 	 number = atoi(buf2) * 100;
       }
     } else if ((subcmd == SCMD_OLC_HEDIT) || (subcmd == SCMD_OLC_AEDIT))
       number = 0;
     else if (subcmd == SCMD_OLC_ZEDIT && GET_LEVEL(ch) >= LVL_OLC_FULL) {
       if ((strn_cmp("new", buf1, 3) == 0) && *buf2)
 	zedit_new_zone(ch, atoi(buf2));
       else
 	send_to_char("Specify a new zone number.\r\n", ch);
       return;
     } else {
       send_to_char("Yikes!  Stop that, someone will get hurt!\r\n", ch);
       return;
     }
   }
   /*
    * If a numeric argument was given, get it.
    */
   if ((number == -1) && ((subcmd != SCMD_OLC_AEDIT) || (subcmd != SCMD_OLC_HEDIT)))
     number = atoi(buf1);
 
   /*
    * Check that whatever it is isn't already being edited.
    */
   for (d = descriptor_list; d; d = d->next)
     if (d->connected == olc_scmd_info[subcmd].con_type)
       if (d->olc && OLC_NUM(d) == number) {
	if (subcmd == SCMD_OLC_HEDIT)
	  sprintf(buf, "Help files are already being editted by %s.\r\n",
		  (CAN_SEE(ch, d->character) ? GET_NAME(d->character) : "someone"));
	else if (subcmd == SCMD_OLC_AEDIT)
          sprintf(buf, "Help files are already being editted by %s.\r\n",
		  (CAN_SEE(ch, d->character) ? GET_NAME(d->character) : "someone"));
        else
	  sprintf(buf, "That %s is currently being edited by %s.\r\n",
 		olc_scmd_info[subcmd].text, GET_NAME(d->character));
 	send_to_char(buf, ch);
 	return;
       }
   d = ch->desc;
  
   /*
    * Give descriptor an OLC struct.
    */
   CREATE(d->olc, struct olc_data, 1);
 
   /*
    * Find the zone (or help rnum).
    */
   if (subcmd != SCMD_OLC_AEDIT) {
     if (subcmd == SCMD_OLC_HEDIT && !save)
       OLC_ZNUM(d) = find_help_rnum(buf1);
     else if ((OLC_ZNUM(d) = real_zone(number)) == -1) {
       send_to_char("Sorry, there is no zone for that number!\r\n", ch);
       free(d->olc);
       return;
     }
   }
   /*
    * Everyone but IMPLs can only edit zones they have been assigned.
    */
  if (GET_LEVEL(ch) < LVL_GOD)
  {
      if (subcmd == SCMD_OLC_HEDIT && GET_OLC_ZONE(ch) != HEDIT_PERMISSION) 
     {
       send_to_char("You do not have permssion to edit help entries.\r\n", ch);
       free(d->olc);
       return;
     }
  }   
   if (GET_LEVEL(ch) < LVL_OLC_FULL)
  {
    
       if (subcmd == SCMD_OLC_AEDIT && GET_OLC_ZONE(ch) != AEDIT_PERMISSION) 
     {
       send_to_char("You do not have permission to edit actions.\r\n", ch);
       free(d->olc);
       return;
     } 
       else if (subcmd == SCMD_OLC_QEDIT && GET_OLC_ZONE(ch) != QEDIT_PERMISSION) 
     {
       send_to_char("You do not have permission to edit quests.\r\n", ch);
       free(d->olc);
       return;
     } 
       else if (subcmd == SCMD_OLC_TRIGEDIT && GET_OLC_ZONE(ch) != TRIGEDIT_PERMISSION) 
     {
       send_to_char("You do not have permission to edit triggers.\r\n", ch);
       free(d->olc);
       return;
     } else if (subcmd != SCMD_OLC_AEDIT && subcmd != SCMD_OLC_HEDIT &&
            subcmd != SCMD_OLC_QEDIT && subcmd != SCMD_OLC_TRIGEDIT &&
            !can_edit_zone(ch, OLC_ZNUM(d)))
     {
       send_to_char("You do not have permission to edit this zone.\r\n", ch);
       free(d->olc);
       return;
     }
  }

   if (save) {
     const char *type = NULL;
  
     switch (subcmd) {
     /*
      * Removed as a nitpick.  Why do something if it's unnecessary?
      * (cases for AEDIT and HEDIT removed)
      */
     case SCMD_OLC_AEDIT: type = "action"; break;
     case SCMD_OLC_HEDIT: type = "help"; break;
     case SCMD_OLC_REDIT: type = "room"; break;
     case SCMD_OLC_ZEDIT: type = "zone"; break;
     case SCMD_OLC_SEDIT: type = "shop"; break;
     case SCMD_OLC_MEDIT: type = "mobile"; break;
     case SCMD_OLC_OEDIT: type = "object"; break;
     case SCMD_OLC_QEDIT: type = "quest"; break;
     }
     if (!type) {
       send_to_char("Oops, I forgot what you wanted to save.\r\n", ch);
       return;
     }
     if (subcmd == SCMD_OLC_HEDIT) {
       send_to_char("Saving all help entries.\r\n", ch);
       sprintf(buf, "OLC: %s saves help entries.", GET_NAME(ch));
       mudlog(buf, NRM, MAX(LVL_BUILDER, GET_INVIS_LEV(ch)), TRUE);
     } else if (subcmd == SCMD_OLC_AEDIT) {
       send_to_char("Saving all actions.\r\n", ch);
       sprintf(buf, "OLC: %s saves all actions.", GET_NAME(ch));
       mudlog(buf, NRM, MAX(LVL_BUILDER, GET_INVIS_LEV(ch)), TRUE);
     } else {
       sprintf(buf, "Saving all %ss in zone %d.\r\n",
 		type, zone_table[OLC_ZNUM(d)].number);
       send_to_char(buf, ch);
       sprintf(buf, "OLC: %s saves %s info for zone %d.", GET_NAME(ch),  type,
 		zone_table[OLC_ZNUM(d)].number);
       mudlog(buf, CMP, MAX(LVL_BUILDER, GET_INVIS_LEV(ch)), TRUE);
     }
 
     switch (subcmd) {
     case SCMD_OLC_REDIT: redit_save_to_disk(OLC_ZNUM(d)); break;
     case SCMD_OLC_ZEDIT: zedit_save_to_disk(OLC_ZNUM(d)); break;
     case SCMD_OLC_OEDIT: oedit_save_to_disk(OLC_ZNUM(d)); break;
     case SCMD_OLC_MEDIT: medit_save_to_disk(OLC_ZNUM(d)); break;
     case SCMD_OLC_SEDIT: sedit_save_to_disk(OLC_ZNUM(d)); break;
     case SCMD_OLC_QEDIT: qedit_save_to_disk(OLC_ZNUM(d)); break;
     case SCMD_OLC_HEDIT: hedit_save_to_disk(); break;
     case SCMD_OLC_AEDIT: aedit_save_to_disk(d); break;
     }
     free(d->olc);
     return;
   }

   if (subcmd != SCMD_OLC_AEDIT) OLC_NUM(d) = number;
   else {
     OLC_NUM(d) = 0;
     OLC_STORAGE(d) = str_dup(buf1);
     for (OLC_ZNUM(d) = 0; (OLC_ZNUM(d) <= top_of_socialt); OLC_ZNUM(d)++)
       if (is_abbrev(OLC_STORAGE(d), soc_mess_list[OLC_ZNUM(d)].command))
         break;
     if (OLC_ZNUM(d) > top_of_socialt)  {
       if (find_command(OLC_STORAGE(d)) > NOTHING)  {
         cleanup_olc(d, CLEANUP_ALL);
         send_to_char("That command already exists.\r\n", ch);
         return;
       }
       sprintf(buf, "Do you wish to add the '%s' action? ", OLC_STORAGE(d));
       send_to_char(buf, ch);
       OLC_MODE(d) = AEDIT_CONFIRM_ADD;
     } else  {
       sprintf(buf, "Do you wish to edit the '%s' action? ", soc_mess_list[OLC_ZNUM(d)].command);
       send_to_char(buf, ch);
       OLC_MODE(d) = AEDIT_CONFIRM_EDIT;
     }
   }
 
   /*
    * Steal player's descriptor start up subcommands.
    */
   switch (subcmd) {
   case SCMD_OLC_TRIGEDIT:
     if ((real_num = real_trigger(number)) >= 0)
       trigedit_setup_existing(d, real_num);
     else
       trigedit_setup_new(d);
     STATE(d) = CON_TRIGEDIT;
     break;
   case SCMD_OLC_REDIT:
     if ((real_num = real_room(number)) >= 0)
       redit_setup_existing(d, real_num);
     else
       redit_setup_new(d);
     STATE(d) = CON_REDIT;
      break;
   case SCMD_OLC_ZEDIT:
     if ((real_num = real_room(number)) < 0) {
       send_to_char("That room does not exist.\r\n", ch);
       free(d->olc);
       return;
     }
     zedit_setup(d, real_num);
     STATE(d) = CON_ZEDIT;
      break;
   case SCMD_OLC_MEDIT:
     if ((real_num = real_mobile(number)) < 0)
       medit_setup_new(d);
     else
       medit_setup_existing(d, real_num);
     STATE(d) = CON_MEDIT;
      break;
   case SCMD_OLC_OEDIT:
     if ((real_num = real_object(number)) >= 0)
       oedit_setup_existing(d, real_num);
     else
       oedit_setup_new(d);
     STATE(d) = CON_OEDIT;
     break;
   case SCMD_OLC_SEDIT:
     if ((real_num = real_shop(number)) >= 0)
       sedit_setup_existing(d, real_num);
     else
       sedit_setup_new(d);
     STATE(d) = CON_SEDIT;
     break;
   case SCMD_OLC_HEDIT:
     if (OLC_ZNUM(d) < 0)
       hedit_setup_new(d, buf1);
     else
       hedit_setup_existing(d, OLC_ZNUM(d));
     STATE(d) = CON_HEDIT;
     break;
   case SCMD_OLC_AEDIT:
     STATE(d) = CON_AEDIT;
     break;
   case SCMD_OLC_QEDIT:
    real_num = real_quest(number);
    if (real_num >= 0)
      qedit_setup_existing(d, real_num);
    else
      qedit_setup_new(d);
    STATE(d) = CON_QEDIT;
    break;
   }
   act("$n starts using OLC.", TRUE, d->character, 0, 0, TO_ROOM);
   SET_BIT(PLR_FLAGS(ch), PLR_WRITING);
 }
  
 /*------------------------------------------------------------*\
  Internal utilities 
 \*------------------------------------------------------------*/
  
 void olc_saveinfo(struct char_data *ch)
 {
   struct olc_save_info *entry;
  
   if (olc_save_list)
     send_to_char("The following OLC components need saving:-\r\n", ch);
   else
     send_to_char("The database is up to date.\r\n", ch);
  
   for (entry = olc_save_list; entry; entry = entry->next) {
     if ((int)entry->type == OLC_SAVE_HELP)
       sprintf(buf, " - Help Entries.\r\n");
     else if ((int)entry->type == OLC_SAVE_ACTION)
       sprintf(buf, " - Actions.\r\n");
     else if ((int)entry->type == OLC_SAVE_QUEST)
       sprintf(buf, " - Quests for zone %d.\r\n", entry->zone);
     else
       sprintf(buf, " - %s for zone %d.\r\n",
 		save_info_msg[(int)entry->type], entry->zone);
     send_to_char(buf, ch);
    }
  }
  
 int real_zone(int number)
 {
   int counter;
 
   for (counter = 0; counter <= top_of_zone_table; counter++)
     if ((number >= (zone_table[counter].number * 100)) &&
 	(number <= (zone_table[counter].top)))
       return counter;
 
   return -1;
 }
  
 /*------------------------------------------------------------*\
  Exported utilities 
 \*------------------------------------------------------------*/
 
 /*
  * Add an entry to the 'to be saved' list.
  */
 
 void olc_add_to_save_list(int zone, byte type)
  {
   struct olc_save_info *new;
 
   /*
    * Return if it's already in the list.
    */
   for (new = olc_save_list; new; new = new->next)
     if ((new->zone == zone) && (new->type == type))
       return;
 
   CREATE(new, struct olc_save_info, 1);
   new->zone = zone;
   new->type = type;
   new->next = olc_save_list;
   olc_save_list = new;
  }
  
 /*
  * Remove an entry from the 'to be saved' list.
  */
  
 void olc_remove_from_save_list(int zone, byte type)
  {
   struct olc_save_info **entry;
   struct olc_save_info *temp;
  
   for (entry = &olc_save_list; *entry; entry = &(*entry)->next)
     if (((*entry)->zone == zone) && ((*entry)->type == type)) {
       temp = *entry;
       *entry = temp->next;
       free(temp);
       return;
      }
  }
  
 /*
  * Set the colour string pointers for that which this char will
  * see at color level NRM.  Changing the entries here will change 
  * the colour scheme throughout the OLC.
  */
  
 void get_char_cols(struct char_data *ch)
 {
   nrm = CCNRM(ch, C_NRM);
   grn = CCGRN(ch, C_NRM);
   cyn = CCCYN(ch, C_NRM);
   yel = CCYEL(ch, C_NRM);
 }
 
 /*
  * This procedure removes the '\r\n' from a string so that it may be
  * saved to a file.  Use it only on buffers, not on the original
  * strings.
  */
 void strip_string(char *buffer)
  {
   char *ptr, *str;
  
   ptr = buffer;
   str = ptr;
  
   while ((*str = *ptr)) {
     str++;
     ptr++;
     if (*ptr == '\r')
       ptr++;
    }
 }
  
 /*
  * This procdure frees up the strings and/or the structures
  * attatched to a descriptor, sets all flags back to how they
  * should be.
  */
  
 void cleanup_olc(struct descriptor_data *d, byte cleanup_type)
 {
   if (d->olc) {
     /*
      * Check for storage.
      */
     if (OLC_STORAGE(d))
       free(OLC_STORAGE(d));

     /*
      * Check for help.
      */
     if (OLC_HELP(d)) {
       switch (cleanup_type) {
       case CLEANUP_ALL:	free_help(OLC_HELP(d));	break;
       case CLEANUP_STRUCTS:  free(OLC_HELP(d));	break;
       default: /* The caller has screwed up. */	break;
       }
     }
     /*
      * Check for a room.
      */
     if (OLC_ROOM(d)) {
       /*
        * free_room doesn't perform sanity checks, must be careful here.
        */
       switch (cleanup_type) {
       case CLEANUP_ALL:	free_room(OLC_ROOM(d));	break;
       case CLEANUP_STRUCTS:  free(OLC_ROOM(d));	break;
       default: /* The caller has screwed up. */	break;
       }
     }
     /*
      * Check for an object.
      */
     if (OLC_OBJ(d))
       /*
        * free_obj() makes sure strings aern't part of the prototype.
        */
       free_obj(OLC_OBJ(d));
 
     /*
      * Check for a mob.
      */
     if (OLC_MOB(d))
       /*
        * medit_free_mobile() makes sure strings are not in the prototype.
        */
       medit_free_mobile(OLC_MOB(d));
 
     /*
      * Check for a zone.
      */
     if (OLC_ZONE(d)) {
       /*
        * cleanup_type is irrelevant here, free() everything.
        */
       free(OLC_ZONE(d)->name);
       free(OLC_ZONE(d)->cmd);
       free(OLC_ZONE(d));
     }
 
    /*
     * Check for a shop.
     */
    if (OLC_SHOP(d)) {
       /*
        * free_shop doesn't perform sanity checks, we must be careful here.
        */
        switch (cleanup_type) {
        case CLEANUP_ALL: free_shop(OLC_SHOP(d));	break;
        case CLEANUP_STRUCTS:  free(OLC_SHOP(d));	break;
        default: /* The caller has screwed up. */	break;
      }
    }

    /*. Check for aedit stuff -- M. Scott */
    if (OLC_ACTION(d))  {
      switch(cleanup_type)  {
      case CLEANUP_ALL:
        free_action(OLC_ACTION(d));
        break;
      case CLEANUP_STRUCTS:
        free(OLC_ACTION(d));
        break;
      default:
        /* Caller has screwed up */
        break;
      }
    }

    if (OLC_QUEST(d)) {		/*. free_quest performs no sanity checks, must be carefull here . */
      switch (cleanup_type) {
      case CLEANUP_ALL:
	free_quest(OLC_QUEST(d));
	break;
      case CLEANUP_STRUCTS:
	free(OLC_QUEST(d));
	break;
      default:
	/*. Caller has screwed up . */
	break;
      }
    }
	
    /*
     * Restore descriptor playing status.
     */
    if (d->character) {
      REMOVE_BIT(PLR_FLAGS(d->character), PLR_WRITING);
      STATE(d) = CON_PLAYING;
      act("$n stops using OLC.", TRUE, d->character, 0, 0, TO_ROOM);
    }
    free(d->olc);
  }
}

/* Everything below this line is part of the OLC+ package specifically. */

/* Can they edit a zone?  This takes a zone's rnum.   *
 * Any reason to add a REAL/VIRTUAL setup? TR 5-20-98 */
int can_edit_zone(struct char_data *ch, int number)
{
  if (GET_LEVEL(ch) >= LVL_OLC_FULL)
    return TRUE;

  if (GET_LEVEL(ch) < LVL_BUILDER)
    return FALSE;

  if(number == real_zone(GET_OLC_ZONE(ch)))
    return TRUE;
    
  return FALSE;
}

/* This handy little function will give the zone number *
 * of any object, room, or mob.      TR 5-18-98         */

#define ROOM	0
#define OBJECT	1
#define MOBILE	2

int zone_number(void *what, int type)
{
  struct char_data *character;
  struct obj_data *object;
  struct room_data *room;
  int return_value;

  switch (type) {
  case ROOM:
    room = (struct room_data *)what;
    return_value = zone_table[real_zone(room->number)].number;
    break;
  case OBJECT:
    object = (struct obj_data *)what;
    return_value = (GET_OBJ_VNUM(object) / 100);
    break;
  case MOBILE:
    character = (struct char_data *)what;
    if (IS_NPC(character))
      return_value = -1;
    /* This formula seems to work in all cases, although I   *
     * know it wouldn't in the case of rooms.  The documents *
     * state that the top_of_zone specifies the last ROOM,   *
     * but not object.  I think the default is (zone#*10)+99 */
    else
      /* Doesn't this look like it wouldn't work?  It seems  *
       * that C has decided to simply truncate integers in   *
       * lieu of rounding them...  Is this machine specific? *
       *                                   TR 5-20-98        */
      return_value = (GET_MOB_VNUM(character) / 100);
    break;
  default:
    return_value = -1;
    break;
  }

  return return_value;
}


/* This little function has real potential.  Give it  *
 * a source room's rnum and a target room's rnum, and *
 * it will do a copy.  Use it in your own commands.   *
 *                                 TR 5-20-98         */
void copy_room(int rnum_src, int rnum_targ)
{
  if (world[rnum_src].name)
    world[rnum_targ].name = str_dup(world[rnum_src].name);
  if (world[rnum_src].description)
    world[rnum_targ].description = str_dup(world[rnum_src].description);
  world[rnum_targ].sector_type = world[rnum_src].sector_type;
  world[rnum_targ].room_flags = world[rnum_src].room_flags;
  /* Note:  ex_descriptions are not being      *
   * copied.  I think it will stay that way.   *
   *                       TR 5-20-98          */
  return;
}


/* Same as copy_room, but with objects.  No error checking.  *
 * Should this be made an integer so a check can be made for *
 * success?                             TR 2-20-98           */
void copy_object(int rnum_src, int rnum_targ)
{
  if (obj_proto[rnum_src].name)
    obj_proto[rnum_targ].name = str_dup(obj_proto[rnum_src].name);
  if (obj_proto[rnum_src].description)
    obj_proto[rnum_targ].description = str_dup(obj_proto[rnum_src].description);
  if (obj_proto[rnum_src].short_description)
    obj_proto[rnum_targ].short_description = str_dup(obj_proto[rnum_src].short_description);
  if (obj_proto[rnum_src].action_description)
    obj_proto[rnum_targ].action_description = str_dup(obj_proto[rnum_src].action_description);
  if (obj_proto[rnum_src].ex_description)
    obj_proto[rnum_targ].ex_description = obj_proto[rnum_src].ex_description;
  obj_proto[rnum_targ].obj_flags = obj_proto[rnum_src].obj_flags;
  obj_proto[rnum_targ].worn_on = obj_proto[rnum_src].worn_on;
  /* add more if you want... */

  return;
}

/*
 * Command interface for:
 * Copying a room or object to another.
 * Attempt at ending the spaghetti:  5-20-98
 */

#define COPY_FORMAT	"Usage:  copy { room | obj } <source> <target>\r\n"

ACMD(do_copy)
{
  char src_num[256], targ_num[256], type[256];
  int vnum_targ = 0, rnum_targ = 0, vnum_src = 0, rnum_src = 0;
  int save_zone = 0, room_or_obj = -1;

  argument = two_arguments(argument, type, src_num);
  one_argument(argument, targ_num);

  /* Here are reasons to give up.  I think they're all right here. */
  if (!*type || !*src_num) {
    send_to_char(COPY_FORMAT, ch);
    return;
  } else if (!*targ_num && (room_or_obj == OBJECT)) {
    send_to_char("You must specify a target when copying objects.\r\n", ch);
    return;
  }

  if (is_abbrev(type, "room") && is_number(src_num)) {
    room_or_obj = ROOM;
    vnum_src = atoi(src_num);
    rnum_src = real_room(vnum_src);
    if (!*targ_num) {
      vnum_targ = world[IN_ROOM(ch)].number;
      rnum_targ = IN_ROOM(ch);
    } else {
      if (!is_number(targ_num)) {
        send_to_char(COPY_FORMAT, ch);
        return;
      }
      vnum_targ = atoi(targ_num);
      rnum_targ = real_room(vnum_targ);
    }
    save_zone = zone_number(&world[rnum_targ], ROOM);
  } else if (is_abbrev(type, "obj") && *targ_num && is_number(src_num) && is_number(targ_num)) {
    room_or_obj = OBJECT;
    vnum_src = atoi(src_num);
    rnum_src = real_object(vnum_src);
    vnum_targ = atoi(targ_num);
    rnum_targ = real_object(vnum_targ);
    save_zone = zone_number(&obj_index[rnum_targ], OBJECT);
  } else {
    send_to_char(COPY_FORMAT, ch);
    return;
  }

  if ((rnum_src < 0) || (rnum_targ < 0)) {
    sprintf(buf, "The source and target %ss must both currently exist.\r\n", (room_or_obj == OBJECT ? "object" : "room"));
    send_to_char(buf, ch);
    return;
  } else if (!can_edit_zone(ch, real_zone(save_zone))) {
    send_to_char("You cannot edit that zone.\r\n", ch);
    return;
  }

  /* We should now be ready to go.  All errors have been trapped (?) *
   * and we know what to do.                      TR 5-21-98         */
  switch (room_or_obj) {
  case ROOM:
    copy_room(rnum_src, rnum_targ);
    break;
  case OBJECT:
    copy_object(rnum_src, rnum_targ);
    break;
  default:
    mudlog("SYSERR: OLC: Reached default case in do_copy!", NRM, MAX(LVL_BUILDER, GET_INVIS_LEV(ch)), TRUE);
    send_to_char("There was an error in your copy.  Please report to an "
		"administrator.\r\n", ch);
    return;
  }

  /* I cheated right here a little bit.  By coincidence, *
   * ROOM == OLC_SAVE_ROOM, and OBJECT = OLC_SAVE_OBJ.   *
   * I think this is a Good Thing(tm), although if you   *
   * change the OLC_SAVE_x defines, it will blow up.  :P *
   *                                    TR 5-21-98       */
  sprintf(buf, "You copy %s %d to %d.\r\n", (room_or_obj == ROOM ? "room" : "object"), vnum_src, vnum_targ);
  send_to_char(buf, ch);
  olc_add_to_save_list(save_zone, room_or_obj);
}


/* Supporting functions for RLINK.  A few of these are right out of *
 * OBuild, but I can't remember which.  :P  Credits to Samedi, and  *
 * Daniel Burke, I believe.                   TR 5-21-98            */

/* Create an exit in a room (rnum) in this direction.  (No target) */
int create_dir(int room, int dir)
{
   if ((room > top_of_world) || (room < 0)) {
      log("create_dir(): tried to create invalid door");
      return FALSE;
   }
   if (world[room].dir_option[dir])
     return FALSE;

   CREATE(world[room].dir_option[dir], struct room_direction_data, 1);
   world[room].dir_option[dir]->to_room = NOWHERE;
   world[room].dir_option[dir]->exit_info = 0;
   world[room].dir_option[dir]->general_description = str_dup("You see nothing special.\r\n");
   world[room].dir_option[dir]->keyword = NULL;
   world[room].dir_option[dir]->key = -1;

   return TRUE;

}


/* Remove an exit from a room (rnum). */
int free_dir(int room, int dir)
{
   if ((room > top_of_world) || (room < 0)) {
      log("free_dir(): tried to free invalid door");
      return FALSE;   
   }
   if ((dir < 0) || (dir >= NUM_OF_DIRS)) {
      log("free_dir(): tried to free invalid door");
      return FALSE;
   }

   if (!world[room].dir_option[dir])
     return FALSE;

   world[room].dir_option[dir]->to_room = NOWHERE;
   world[room].dir_option[dir]->exit_info = 0;
   if (world[room].dir_option[dir]->general_description)
     free(world[room].dir_option[dir]->general_description);
   if (world[room].dir_option[dir]->keyword)
     free(world[room].dir_option[dir]->keyword);
   world[room].dir_option[dir]->key = -1;
   free(world[room].dir_option[dir]);
   world[room].dir_option[dir] = NULL;

   return TRUE;

}


/* These defines were harvested from....  zedit.c ?      */
#define ZCMD (zone_table[zone].cmd[cmd_no])
#define W_EXIT(room, num) (world[(room)].dir_option[(num)])
/* ***************************************************** */
#define RLINK_FORMAT	"Usage:  rlink <dir> <connect|disconnect> <1|2> [target]\r\n"

/* The big baby. */
ACMD(do_rlink)
{
  char direction[10], command[20], type[10], target[10];
  int vnum_base = 0, vnum_targ = 0, rnum_base = 0, rnum_targ = 0;
  int dir = 0, k = 0, type_int = 0, top_room = 0;
  int save_zone_1 = 0, save_zone_2 = 0, create_new_room = FALSE;

  argument = two_arguments(argument, direction, command);
  two_arguments(argument, type, target);

  if (!*direction || !*command || !*type) {
    send_to_char(RLINK_FORMAT, ch); 
    return;
  } else if (!is_number(type)) {
    send_to_char(RLINK_FORMAT, ch);
    return;
  }

  type_int = atoi(type);

  if (type_int != 1 && type_int != 2) {
    send_to_char(RLINK_FORMAT, ch);
    return;
  }

  vnum_base = world[IN_ROOM(ch)].number;
  rnum_base = IN_ROOM(ch);

  if (!*target && !is_abbrev(command, "disconnect")) {
    create_new_room = TRUE; 
  } else {
    if (!is_number(target)) {
      send_to_char(RLINK_FORMAT, ch);
      return;
    }
    vnum_targ = atoi(target);
    rnum_targ = real_room(vnum_targ);
  }

  if (rnum_targ < 0) {
    send_to_char(RLINK_FORMAT, ch);
    return;
  }

  save_zone_1 = zone_number(&world[IN_ROOM(ch)], ROOM);

  if (!can_edit_zone(ch, real_zone(save_zone_1))) {
    send_to_char("You cannot create exits in this zone.\r\n", ch);
    return;
  }

  if (!create_new_room) {
    if (rnum_targ < 0) {
      send_to_char(RLINK_FORMAT, ch);
      return;
    }
    save_zone_2 = zone_number(&world[rnum_targ], ROOM);
  } else {
    top_room = (zone_table[real_zone(world[IN_ROOM(ch)].number)].top);

    for (k = (save_zone_1 * 100); k <= top_room; k++) {
      if (k > top_room) {
        send_to_char("Cannot create a new room in this zone!\r\n", ch);
        return;
      }
      if (real_room(k) < 0) {
        CREATE(ch->desc->olc, struct olc_data, 1);
        CREATE(OLC_ROOM(ch->desc), struct room_data, 1);

        OLC_ZNUM(ch->desc) = world[ch->in_room].zone;
        OLC_NUM(ch->desc) = k;
        OLC_ROOM(ch->desc)->number = k;
        OLC_ROOM(ch->desc)->zone = world[ch->in_room].zone;
        OLC_ROOM(ch->desc)->name = str_dup("An unfinished room");
        OLC_ROOM(ch->desc)->description = str_dup("You are in an unfinished room.\r\n");
        OLC_ITEM_TYPE(ch->desc) = WLD_TRIGGER;
        OLC_VAL(ch->desc) = 0;
        vnum_targ = k;
    
        redit_save_internally(ch->desc);
        cleanup_olc(ch->desc, CLEANUP_STRUCTS);
        rnum_targ = real_room(vnum_targ);
        save_zone_2 = save_zone_1;
        sprintf(buf, "You have created new room #%i.\r\n", vnum_targ);
        send_to_char(buf, ch);
        break;
      }
    }
  }

  /* save_zone_2 has definitely been established.  If it's a two *
   * way exit, don't let them do the linkage.   TR 5-21-98       */

  if (!can_edit_zone(ch, real_zone(save_zone_2)) && type_int == 2) {
    send_to_char("You cannot create exits in the target zone.\r\n", ch);
    return;
  }

  switch (*direction) {
    case 'n':
    case 'N':
      dir = NORTH;
      break;
    case 'e':
    case 'E':
      dir = EAST;
      break;
    case 's':
    case 'S':
      dir = SOUTH;
      break;
    case 'w':
    case 'W':
      dir = WEST;
      break;
    case 'u':
    case 'U':
      dir = UP;
      break;
    case 'd':
    case 'D':
      dir = DOWN;
      break;
    default:
      send_to_char("No such direction!\r\n", ch);
      return;
  }

  if (is_abbrev(command, "connect")) {
    if (!world[ch->in_room].dir_option[dir]) {
      create_dir(IN_ROOM(ch), dir);
      world[IN_ROOM(ch)].dir_option[dir]->to_room = rnum_targ;
    } else
      world[IN_ROOM(ch)].dir_option[dir]->to_room = rnum_targ;

    if (type_int == 2) {
      if (!world[rnum_targ].dir_option[rev_dir[dir]])
        create_dir(rnum_targ, rev_dir[dir]);
      world[rnum_targ].dir_option[rev_dir[dir]]->to_room = IN_ROOM(ch);
      if (!save_zone_2) save_zone_2 = zone_number(&world[rnum_targ], ROOM);
    }
  } else if (is_abbrev(command, "disconnect")) {
    if (type_int == 2) {
      if (world[IN_ROOM(ch)].dir_option[dir]->to_room) {
        free_dir(world[IN_ROOM(ch)].dir_option[dir]->to_room, rev_dir[dir]);
        if (world[IN_ROOM(ch)].dir_option[dir])
          free_dir(IN_ROOM(ch), dir);
        else {
          send_to_char("No such exit!\r\n", ch);
          return;
        }
        save_zone_2 = zone_number(&world[rnum_targ], ROOM);
      } else {
        send_to_char("There is no reciprocol exit to remove.\r\n", ch);
        if (world[IN_ROOM(ch)].dir_option[dir]->to_room) {
          free_dir(IN_ROOM(ch), dir);
        } else {
          send_to_char("No such exit!\r\n", ch);
          return;
        }

      }
    } else if (type_int == 1) {
      if (!world[IN_ROOM(ch)].dir_option[dir]->to_room) {
        send_to_char("No such exit!\r\n", ch); 
        return;
      } else {
        free_dir(IN_ROOM(ch), dir);
      }
    } else {
      send_to_char("Invalid disconnect type.\r\n", ch);
      return;
    }
  } else {
    send_to_char("Invalid command type.  Valid choices are connect and disconnect.\r\n", ch);
    return;
  }

  if (is_abbrev(command, "connect")) {
    sprintf(buf, "You make an exit %s to room %d.\r\n", dirs[dir], vnum_targ);
    send_to_char(buf, ch);
  } else
    send_to_char("Exit deleted.\r\n", ch);

  olc_add_to_save_list(save_zone_1, OLC_SAVE_ROOM);
  if (save_zone_2)
    olc_add_to_save_list(save_zone_2, OLC_SAVE_ROOM);
  return;
}
