/* ************************************************************************
*   File: spec_assign.c                                 Part of CircleMUD *
*  Usage: Functions to assign function pointers to objs/mobs/rooms        *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"
#include "winddragon.h"
#include "buffer.h"

extern int top_of_world;
extern int dts_are_dumps;
extern int mini_mud;
extern struct room_data *world;
extern struct index_data *mob_index;
extern struct index_data *obj_index;

SPECIAL(dump);
SPECIAL(pet_shops);
SPECIAL(archer);
SPECIAL(postmaster);
SPECIAL(cityguard);
SPECIAL(gym);
SPECIAL(temple);
SPECIAL(receptionist);
SPECIAL(cryogenicist);
SPECIAL(puff);
SPECIAL(fido);
SPECIAL(janitor);
SPECIAL(mayor);
SPECIAL(forger);
SPECIAL(snake);
SPECIAL(thief);
SPECIAL(magic_user);
SPECIAL(marbles);
SPECIAL(bank);
SPECIAL(winddragon);
SPECIAL(windroom);
SPECIAL(gen_board);
SPECIAL(priest_healer);
SPECIAL(priest);
SPECIAL(blacksmith);
SPECIAL(icewizard);
SPECIAL(preparation_room);
SPECIAL(questmaster);
SPECIAL(sund_earl);
SPECIAL(hangman);
SPECIAL(blinder);
SPECIAL(silktrader);
SPECIAL(butcher);
SPECIAL(idiot);
SPECIAL(athos);
SPECIAL(stu);
SPECIAL(wardome);
SPECIAL(questshop);
SPECIAL(black_dragon);
SPECIAL(black_monster);
SPECIAL(reborn);
SPECIAL(alinhamento);
SPECIAL(religiao) ;
SPECIAL(random_room);
SPECIAL(espada_fica_azul);
SPECIAL(tiamat2);
SPECIAL(tiamat_heads);
SPECIAL(oracle);
SPECIAL(clan_guard);
SPECIAL(destructor) ; 
SPECIAL(soundroom) ;
SPECIAL(soundroom2);
SPECIAL(anelrecall);

//SPECIAL(cassino);

void assign_kings_castle(void);

/* local functions */
void assign_mobiles(void);
void assign_objects(void);
void assign_rooms(void);
void ASSIGNROOM(int room, SPECIAL(fname));
void ASSIGNMOB(int mob, SPECIAL(fname));
void ASSIGNOBJ(int obj, SPECIAL(fname));

/* functions to perform assignments */

void ASSIGNMOB(int mob, SPECIAL(fname))
{
  int rnum;

  if ((rnum = real_mobile(mob)) >= 0)
    mob_index[rnum].func = fname;
  else if (!mini_mud)
    log("SYSERR: Attempt to assign spec to non-existant mob #%d", mob);
}

void ASSIGNOBJ(int obj, SPECIAL(fname))
{
  if (real_object(obj) >= 0)
    obj_index[real_object(obj)].func = fname;
  else if (!mini_mud)
    log("SYSERR: Attempt to assign spec to non-existant obj #%d", obj);
}

void ASSIGNROOM(int room, SPECIAL(fname))
{
  if (real_room(room) >= 0)
    world[real_room(room)].func = fname;
  else if (!mini_mud)
    log("SYSERR: Attempt to assign spec to non-existant room #%d", room);
}


/* ********************************************************************
*  Assignments                                                        *
******************************************************************** */

/* assign special procedures to mobiles */
void assign_mobiles(void)
{
  assign_kings_castle();

  
  ASSIGNMOB(1, puff);
  ASSIGNMOB(16, destructor) ; 
  //ASSIGNMOB(499, darkside_guard);
  ASSIGNMOB(10198, clan_guard); // 
  ASSIGNMOB(30201, clan_guard); // 
  ASSIGNMOB(9400, clan_guard); //
  ASSIGNMOB(30401, clan_guard); // 
  ASSIGNMOB(10100, clan_guard); //
  ASSIGNMOB(1900, clan_guard); // warlords
  //ASSIGNMOB(2400, clan_guard); // kingdom steel
 
  ASSIGNMOB(500, preparation_room);
//  ASSIGNMOB(503, mayor);
  ASSIGNMOB(505, questmaster);
  ASSIGNMOB(506, questshop);
  ASSIGNMOB(513, oracle);
  ASSIGNMOB(575, gym);
  ASSIGNMOB(576, temple);
  ASSIGNMOB(508, wardome);
  ASSIGNMOB(580, alinhamento);
  ASSIGNMOB(598, religiao) ;
  //ASSIGNMOB(514, cassino);

  /* Immortal Zone */
  ASSIGNMOB(1200, receptionist);
  ASSIGNMOB(1201, postmaster);
  ASSIGNMOB(1202, janitor);

  /* Midgaard */
 // ASSIGNMOB(3005, receptionist);
  ASSIGNMOB(3010, postmaster);
 // ASSIGNMOB(3059, cityguard);
//  ASSIGNMOB(3060, cityguard);
//  ASSIGNMOB(3061, janitor);
//  ASSIGNMOB(3062, fido);
//  ASSIGNMOB(3066, fido);
//  ASSIGNMOB(3067, cityguard);
//  ASSIGNMOB(3068, janitor);
  ASSIGNMOB(3095, cryogenicist);
//  ASSIGNMOB(3105, mayor);
  ASSIGNMOB(3080, priest);
  ASSIGNMOB(3030, forger);
  ASSIGNMOB(3097, black_dragon);
  ASSIGNMOB(3098, black_monster);
  ASSIGNMOB(567, cityguard);
  ASSIGNMOB(502, archer);

  /* MORIA */
  ASSIGNMOB(4000, snake);
  ASSIGNMOB(4001, snake);
  ASSIGNMOB(4053, snake);
  ASSIGNMOB(4100, magic_user);
  ASSIGNMOB(4102, snake);
  ASSIGNMOB(4103, thief);

  /* Redferne's */
  ASSIGNMOB(7900, cityguard);

  /* PYRAMID */
  ASSIGNMOB(5300, snake);
  ASSIGNMOB(5301, snake);
  ASSIGNMOB(5304, thief);
  ASSIGNMOB(5305, thief);
  ASSIGNMOB(5309, magic_user); /* should breath fire */
  ASSIGNMOB(5311, magic_user);
  ASSIGNMOB(5313, magic_user); /* should be a cleric */
  ASSIGNMOB(5314, magic_user); /* should be a cleric */
  ASSIGNMOB(5315, magic_user); /* should be a cleric */
  ASSIGNMOB(5316, magic_user); /* should be a cleric */
  ASSIGNMOB(5317, magic_user);

  /* High Tower Of Sorcery */
  ASSIGNMOB(2501, magic_user); /* should likely be cleric */
  ASSIGNMOB(2504, magic_user);
  ASSIGNMOB(2507, magic_user);
  ASSIGNMOB(2508, magic_user);
  ASSIGNMOB(2510, magic_user);
  ASSIGNMOB(2511, thief);
  ASSIGNMOB(2514, magic_user);
  ASSIGNMOB(2515, magic_user);
  ASSIGNMOB(2516, magic_user);
  ASSIGNMOB(2517, magic_user);
  ASSIGNMOB(2518, magic_user);
  ASSIGNMOB(2520, magic_user);
  ASSIGNMOB(2521, magic_user);
  ASSIGNMOB(2522, magic_user);
  ASSIGNMOB(2523, magic_user);
  ASSIGNMOB(2524, magic_user);
  ASSIGNMOB(2525, magic_user);
  ASSIGNMOB(2526, magic_user);
  ASSIGNMOB(2527, magic_user);
  ASSIGNMOB(2528, magic_user);
  ASSIGNMOB(2529, magic_user);
  ASSIGNMOB(2530, magic_user);
  ASSIGNMOB(2531, magic_user);
  ASSIGNMOB(2532, magic_user);
  ASSIGNMOB(2533, magic_user);
  ASSIGNMOB(2534, magic_user);
  ASSIGNMOB(2536, magic_user);
  ASSIGNMOB(2537, magic_user);
  ASSIGNMOB(2538, magic_user);
  ASSIGNMOB(2540, magic_user);
  ASSIGNMOB(2541, magic_user);
  ASSIGNMOB(2548, magic_user);
  ASSIGNMOB(2549, magic_user);
  ASSIGNMOB(2552, magic_user);
  ASSIGNMOB(2553, magic_user);
  ASSIGNMOB(2554, magic_user);
  ASSIGNMOB(2556, magic_user);
  ASSIGNMOB(2557, magic_user);
  ASSIGNMOB(2559, magic_user);
  ASSIGNMOB(2560, magic_user);
  ASSIGNMOB(2562, magic_user);
  ASSIGNMOB(2564, magic_user);

  /* SEWERS */
  ASSIGNMOB(7006, snake);
  ASSIGNMOB(7009, magic_user);
  ASSIGNMOB(7200, magic_user);
  ASSIGNMOB(7201, magic_user);
  ASSIGNMOB(7202, magic_user);

  /* FOREST */
  ASSIGNMOB(6112, magic_user);
  ASSIGNMOB(6113, snake);
  ASSIGNMOB(6114, magic_user);
  ASSIGNMOB(6115, magic_user);
  ASSIGNMOB(6116, magic_user); /* should be a cleric */
  ASSIGNMOB(6117, magic_user);

  /* ARACHNOS */
  ASSIGNMOB(6302, magic_user);
  ASSIGNMOB(6309, magic_user);
  ASSIGNMOB(6312, magic_user);
  ASSIGNMOB(6314, magic_user);
  ASSIGNMOB(6315, magic_user);

  /* Desert */
  ASSIGNMOB(5004, magic_user);
  ASSIGNMOB(5010, magic_user);
  ASSIGNMOB(5014, magic_user);

  /* Drow City */
  ASSIGNMOB(5103, magic_user);
  ASSIGNMOB(5104, magic_user);
  ASSIGNMOB(5107, magic_user);
  ASSIGNMOB(5108, magic_user);

  /* Old Thalos */
  ASSIGNMOB(5200, magic_user);
  ASSIGNMOB(5201, magic_user);
  ASSIGNMOB(5209, magic_user);

  /* New Thalos */
/* 5481 - Cleric (or Mage... but he IS a high priest... *shrug*) */
  ASSIGNMOB(5404, receptionist);
  ASSIGNMOB(5421, magic_user);
  ASSIGNMOB(5422, magic_user);
  ASSIGNMOB(5423, magic_user);
  ASSIGNMOB(5424, magic_user);
  ASSIGNMOB(5425, magic_user);
  ASSIGNMOB(5426, magic_user);
  ASSIGNMOB(5427, magic_user);
  ASSIGNMOB(5428, magic_user);
  ASSIGNMOB(5434, cityguard);
  ASSIGNMOB(5440, magic_user);
  ASSIGNMOB(5455, magic_user);
  ASSIGNMOB(5461, cityguard);
  ASSIGNMOB(5462, cityguard);
  ASSIGNMOB(5463, cityguard);
  ASSIGNMOB(5482, cityguard);

  /* WASTELAND */
  ASSIGNMOB(9002, snake);
  ASSIGNMOB(9012, magic_user);
  ASSIGNMOB(9011, icewizard);

  /* ROME */
  ASSIGNMOB(12009, magic_user);
  ASSIGNMOB(12018, cityguard);
  ASSIGNMOB(12020, magic_user);
  ASSIGNMOB(12021, cityguard);
  ASSIGNMOB(12025, magic_user);
  ASSIGNMOB(12030, magic_user);
  ASSIGNMOB(12031, magic_user);
  ASSIGNMOB(12032, magic_user);

  /* King Welmar's Castle (not covered in castle.c) */
  ASSIGNMOB(15015, thief);      /* Ergan... have a better idea? */
  ASSIGNMOB(15032, magic_user); /* Pit Fiend, have something better?  Use it */
  ASSIGNMOB(15001, wardome);

  /* DWARVEN KINGDOM */
  ASSIGNMOB(6500, cityguard);
  ASSIGNMOB(6502, magic_user);
  ASSIGNMOB(6509, magic_user);
  ASSIGNMOB(6516, magic_user);

  /* SUNDHEAVEN */
  ASSIGNMOB(6700, sund_earl);        /* Earl of Sundhaven */
  ASSIGNMOB(6701, cityguard);
 // ASSIGNMOB(6702, hangman);
  ASSIGNMOB(6764, postmaster);
  ASSIGNMOB(6766, stu);
  ASSIGNMOB(6706, fido);             /* Smoke rat */
  ASSIGNMOB(6759, cityguard);
  ASSIGNMOB(6760, cityguard);
  ASSIGNMOB(6707, thief);
  ASSIGNMOB(6748, butcher);
  ASSIGNMOB(6761, blinder);
  ASSIGNMOB(6737, silktrader);
  ASSIGNMOB(6715, idiot);
  ASSIGNMOB(6753, athos);
  ASSIGNMOB(6704, priest);

  ASSIGNMOB(2244, tiamat2);

  ASSIGNMOB(2216, tiamat_heads);
  ASSIGNMOB(2217, tiamat_heads);
  ASSIGNMOB(2218, tiamat_heads);
  ASSIGNMOB(2219, tiamat_heads);
  ASSIGNMOB(2220, tiamat_heads);

}



/* assign special procedures to objects */
void assign_objects(void)
{
  //ASSIGNOBJ(30401, gen_board); /**/
  //ASSIGNOBJ(500, gen_board);   /* mortal board */
  //ASSIGNOBJ(507, gen_board);   /* idea board */
  //ASSIGNOBJ(509, gen_board);   /* mortal board */
  //ASSIGNOBJ(512, gen_board);   /* mortal board */
  //ASSIGNOBJ(3077, gen_board);   /*    bug board */
  ASSIGNOBJ(3096, gen_board);   /* social board */
  //ASSIGNOBJ(3097, gen_board);   /*  */
  //ASSIGNOBJ(3098, gen_board);   /* immortal board */
  /*ASSIGNOBJ(3099, gen_board);   mortal board */
  //ASSIGNOBJ(31001, gen_board);   /* Kingdom of Steel Clan */
  //ASSIGNOBJ(30100, gen_board);   /*  */
  //ASSIGNOBJ(97, gen_board);
  //ASSIGNOBJ(98, gen_board);
  //ASSIGNOBJ(1300, gen_board);  /*questbook board*/
  //ASSIGNOBJ(9403, gen_board);  /* Viking clan */
  //ASSIGNOBJ(3034, bank);        /* atm */
  //ASSIGNOBJ(3036, bank);        /* cashcard */
  //ASSIGNOBJ(6747, marbles);        /* marbles */
  //ASSIGNOBJ(10113, espada_fica_azul);
  //ASSIGNOBJ(1901, gen_board); //Warlords Clan
  //ASSIGNOBJ(2407, gen_board); //Dark Heresy Clan
  //ASSIGNOBJ(595, anelrecall); //
}



/* assign special procedures to rooms */
void assign_rooms(void)
{
  int i;

  ASSIGNROOM(3030, dump);      /* The Dump            */
  ASSIGNROOM(3031, pet_shops); /* The Pet Shop Store  */
  ASSIGNROOM(534, pet_shops);  /* The Wardome City Pet Shop Store  */
  ASSIGNROOM(3067, gym);       /* The Midgaard's Gym  */
  ASSIGNROOM(3005, temple);    /* The Temple Square   */
  ASSIGNROOM(DRAGONROOM_INI, winddragon);/* sanctuary   */
  ASSIGNROOM(500, reborn);
//  ASSIGNROOM(599, alinhamento);
  ASSIGNROOM(to_fly_rooms[0], windroom);/* flying   */
  ASSIGNROOM(to_fly_rooms[1], windroom);/* flying   */
  ASSIGNROOM(to_fly_rooms[2], windroom);/* flying   */
  ASSIGNROOM(2701, random_room);
  ASSIGNROOM(2702, random_room);
  ASSIGNROOM(2703, random_room);
  ASSIGNROOM(2705, random_room);
  ASSIGNROOM(2707, random_room);
  ASSIGNROOM(2708, random_room);
  ASSIGNROOM(2710, random_room);
  
 //SALAS COM SOM 
  
  ASSIGNROOM(543, soundroom); //som no meta 1
  ASSIGNROOM(556, soundroom2); //som do scientist
 //---------------------termina aqui---------------------------- 
  if (dts_are_dumps)
    for (i = 0; i < top_of_world; i++)
      if (ROOM_FLAGGED(i, ROOM_DEATH))
        world[i].func = dump;
 
 


}
