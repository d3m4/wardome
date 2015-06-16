#ifndef _TELEPORT_H_
#define _TELEPORT_H_

#define PULSE_TELEPORT      (10 RL_SEC)
#define MIN_TELEPORT_FREQ   2		/* 20 seconds RealTime */
#define MAX_TELEPORT_FREQ   30		/* 5 minutes RealTime */

#define TELE_LOOK	    (1 << 0)
#define TELE_COUNT	    (1 << 1)
#define TELE_RANDOM         (1 << 2)
#define TELE_SPIN	    (1 << 3)
#define TELE_OBJ            (1 << 4)
#define TELE_NOOBJ          (1 << 5)
#define TELE_NOMSG          (1 << 6)
#define TELE_NOMOB          (1 << 7)
#define TELE_SKIPOBJ        (1 << 8)

#define NUM_TELEPORT		9


void TeleportPulseStuff();

#endif /* _TELEPORT_H_ */
