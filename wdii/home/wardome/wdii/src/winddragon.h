#define NUM_DRAGONS 2
#define NUM_ROOMS   3
#define DRAGON_VNUM 11001
#define DRAGONROOM_INI 11001
#define DRAGONROOM_FALL 11003 

extern const sh_int to_fly_rooms [NUM_ROOMS + 1];

sh_int vet_dragons [NUM_DRAGONS + 1];

struct pont_dragon {
   struct char_data *player;
   int room;
  }; 

struct pont_dragon buf_dragon_indo[NUM_DRAGONS + 1];
struct pont_dragon buf_dragon_vindo[NUM_DRAGONS + 1];
