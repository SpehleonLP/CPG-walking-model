#include "debug_keyed_timings.h"

enum { LEFT=0, RIGHT=1, BACK=0, FRONT=1, SWING=0, STANCE=1 };
#define TIME(hr, min, sec, frame) ((hr*60 + min)*60 + ( sec + (frame/30.0) ))

static struct CPG_KeyedTiming Walk_Sequence[] =  {	
{ RIGHT, BACK, STANCE, TIME(00, 00, 00, 0) },
{ LEFT, BACK, STANCE, TIME(00, 00, 00, 0) },
{ LEFT, FRONT, STANCE, TIME(00, 00, 00, 0) },
{ RIGHT, FRONT, SWING, TIME(00, 00, 00, 0) },

{ RIGHT, BACK, SWING, TIME(00, 00, 00, 6) },
{ LEFT, FRONT, STANCE, TIME(00, 00, 00, 9) },
{ RIGHT, BACK, STANCE, TIME(00, 00, 00, 15) },
{ RIGHT, FRONT, SWING, TIME(00, 00, 00, 24) },

{ LEFT, BACK, SWING, TIME(00, 00, 01, 2) },
{ RIGHT, FRONT, STANCE, TIME(00, 00, 01, 04) },
{ LEFT, BACK, STANCE, TIME(00, 00, 01, 11) },
{ LEFT, FRONT, SWING, TIME(00, 00, 01, 20) },

{ RIGHT, BACK, SWING, TIME(00, 00, 01, 28) },
{ LEFT, FRONT, STANCE, TIME(00, 00, 01, 29) },
{ RIGHT, BACK, STANCE, TIME(00, 00, 02, 06) },
{ RIGHT, FRONT, SWING, TIME(00, 00, 02, 15) },

{ LEFT, BACK, SWING, TIME(00, 00, 02, 23) },
{ RIGHT, FRONT, STANCE, TIME(00, 00, 02, 24) },
{ LEFT, BACK, STANCE, TIME(00, 00, 03, 02) },
{ LEFT, FRONT, SWING, TIME(00, 00, 03, 10) },

{ RIGHT, BACK, SWING, TIME(00, 00, 03, 18) },
{ LEFT, FRONT, STANCE, TIME(00, 00, 03, 21) },
{ RIGHT, BACK, STANCE, TIME(00, 00, 03, 28) },
{ RIGHT, FRONT, SWING, TIME(00, 00, 04, 05) },

{ LEFT, BACK, SWING, TIME(00, 00, 04, 14) },
{ RIGHT, FRONT, STANCE, TIME(00, 00, 04, 16) },
{ LEFT, BACK, STANCE, TIME(00, 00, 04, 23) },
{ LEFT, FRONT, SWING, TIME(00, 00, 05, 02) },

{ RIGHT, BACK, SWING, TIME(00, 00, 05, 9) },
{ LEFT, FRONT, STANCE, TIME(00, 00, 05, 11) },
{ RIGHT, BACK, STANCE, TIME(00, 00, 05, 18) },
{ RIGHT, FRONT, SWING, TIME(00, 00, 05, 27) },

{ LEFT, BACK, SWING, TIME(00, 00, 06, 05) },
{ RIGHT, FRONT, STANCE, TIME(00, 00, 06, 06) },
{ LEFT, BACK, STANCE, TIME(00, 00, 06, 15) },
{ LEFT, FRONT, SWING, TIME(00, 00, 06, 22) },

{ RIGHT, BACK, SWING, TIME(00, 00, 07, 00) },
{ LEFT, FRONT, STANCE, TIME(00, 00, 07, 01) },
{ RIGHT, BACK, STANCE, TIME(00, 00, 07, 10) },
{ RIGHT, FRONT, SWING, TIME(00, 00, 07, 17) },

{ LEFT, BACK, SWING, TIME(00, 00, 07, 25) },
{ RIGHT, FRONT, STANCE, TIME(00, 00, 07, 27) },
{ LEFT, BACK, STANCE, TIME(00, 00,  8, 05) },
{ LEFT, FRONT, SWING, TIME(00, 00,  8, 12) },

{ RIGHT, BACK, SWING, TIME(00, 00, 8, 21) },
{ LEFT, FRONT, STANCE, TIME(00, 00, 8, 23) },
{ RIGHT, BACK, STANCE, TIME(00, 00, 9, 00) },
{ RIGHT, FRONT, SWING, TIME(00, 00, 9, 9) },

{-1, -1, -1, -1}	
};

struct CPG_KeyedGait CPG_Debug_Walk = {
	
	.loop_start = 8,
	.loop_end   = (sizeof(Walk_Sequence) / sizeof(Walk_Sequence[0])) - 1,
	.loop_time  = TIME(00, 00, 9, 9) - TIME(00, 00, 01, 2),
	.start_time = TIME(00, 00, 9, 9),
	.sequence = Walk_Sequence
};

static struct CPG_KeyedTiming Trot_Sequence[] =  {	
{ RIGHT, BACK,  STANCE, TIME(00, 00, 20, 0) },
{ LEFT,  FRONT, STANCE, TIME(00, 00, 20, 0) },
{ LEFT,  BACK,  SWING,  TIME(00, 00, 20, 0) },
{ RIGHT, FRONT, SWING,  TIME(00, 00, 20, 0) },

{ LEFT,  BACK,  STANCE, TIME(00, 00, 20, 4) },
{ RIGHT, FRONT, STANCE, TIME(00, 00, 20, 4) },
{ LEFT,  FRONT, SWING,  TIME(00, 00, 20, 8) },
{ RIGHT, BACK,  SWING,  TIME(00, 00, 20, 8) },

{ LEFT,  FRONT, STANCE, TIME(00, 00, 20, 17) },
{ RIGHT, BACK,  STANCE, TIME(00, 00, 20, 17) },
{ LEFT,  BACK,  SWING,  TIME(00, 00, 20, 21) },
{ RIGHT, FRONT, SWING,  TIME(00, 00, 20, 21) },

{ LEFT,  BACK,  STANCE, TIME(00, 00, 21, 0) },
{ RIGHT, FRONT, STANCE, TIME(00, 00, 21, 0) },
{ LEFT,  FRONT, SWING,  TIME(00, 00, 21, 4) },
{ RIGHT, BACK,  SWING,  TIME(00, 00, 21, 4) },

{ LEFT,  FRONT, STANCE, TIME(00, 00, 21, 12) },
{ RIGHT, BACK,  STANCE, TIME(00, 00, 21, 12) },
{ LEFT,  BACK,  SWING,  TIME(00, 00, 21, 16) },
{ RIGHT, FRONT, SWING,  TIME(00, 00, 21, 16) },

{ LEFT,  BACK,  STANCE, TIME(00, 00, 21, 26) },
{ RIGHT, FRONT, STANCE, TIME(00, 00, 21, 26) },
{ LEFT,  FRONT, SWING,  TIME(00, 00, 21, 29) },
{ RIGHT, BACK,  SWING,  TIME(00, 00, 21, 29) },

{ LEFT,  FRONT, STANCE, TIME(00, 00, 22,  8) },
{ RIGHT, BACK,  STANCE, TIME(00, 00, 22,  8) },
{ LEFT,  BACK,  SWING,  TIME(00, 00, 22, 12) },
{ RIGHT, FRONT, SWING,  TIME(00, 00, 22, 12) },

{ LEFT,  BACK,  STANCE, TIME(00, 00, 22, 21) },
{ RIGHT, FRONT, STANCE, TIME(00, 00, 22, 21) },
{ LEFT,  FRONT, SWING,  TIME(00, 00, 22, 24) },
{ RIGHT, BACK,  SWING,  TIME(00, 00, 22, 24) },

{ LEFT,  FRONT, STANCE, TIME(00, 00, 23, 04) },
{ RIGHT, BACK,  STANCE, TIME(00, 00, 23, 04) },
{ LEFT,  BACK,  SWING,  TIME(00, 00, 23,  8) },
{ RIGHT, FRONT, SWING,  TIME(00, 00, 23,  8) },

{ LEFT,  BACK,  STANCE, TIME(00, 00, 23, 16) },
{ RIGHT, FRONT, STANCE, TIME(00, 00, 23, 16) },
{ LEFT,  FRONT, SWING,  TIME(00, 00, 23, 20) },
{ RIGHT, BACK,  SWING,  TIME(00, 00, 23, 20) },

{ LEFT,  FRONT, STANCE, TIME(00, 00, 23, 29) },
{ RIGHT, BACK,  STANCE, TIME(00, 00, 23, 29) },
{ LEFT,  BACK,  SWING,  TIME(00, 00, 24, 03) },
{ RIGHT, FRONT, SWING,  TIME(00, 00, 24, 03) },

{-1, -1, -1, -1}	
};

struct CPG_KeyedGait CPG_Debug_Trot = {
	
	.loop_start = 4,
	.loop_end   = (sizeof(Trot_Sequence) / sizeof(Trot_Sequence[0])) - 1,
	.loop_time  = TIME(00, 00, 24, 03) - TIME(00, 00, 20, 0) + TIME(00, 00, 00, 4),
	.start_time = TIME(00, 00, 20, 0),
	.sequence = Trot_Sequence
};

static struct CPG_KeyedTiming Canter_Sequence[] =  {	
{ RIGHT, FRONT, STANCE, TIME(00, 00, 30, 0) },
{ LEFT,  BACK,  STANCE, TIME(00, 00, 30, 0) },	
{ RIGHT, BACK,  SWING,  TIME(00, 00, 30, 0) },
{ LEFT,  FRONT, SWING,  TIME(00, 00, 30, 0) },

{ RIGHT, FRONT, SWING,  TIME(00, 00, 30, 3) },
{ RIGHT, BACK,  STANCE, TIME(00, 00, 30, 3) },
{ LEFT,  FRONT, STANCE, TIME(00, 00, 30, 3) },
{ LEFT,  BACK,  SWING,  TIME(00, 00, 30, 8) },

{ RIGHT, FRONT, STANCE, TIME(00, 00, 30, 8) },
{ RIGHT, BACK,  SWING,  TIME(00, 00, 30, 12) },
{ LEFT,  FRONT, SWING,  TIME(00, 00, 30, 12) },
{ LEFT,  BACK,  STANCE, TIME(00, 00, 30, 12) },

{ RIGHT, FRONT, SWING,  TIME(00, 00, 30, 18) },
{ RIGHT, BACK,  STANCE, TIME(00, 00, 30, 18) },
{ LEFT,  FRONT, STANCE, TIME(00, 00, 30, 18) },
{ LEFT,  BACK,  SWING,  TIME(00, 00, 30, 22) },

{ RIGHT, FRONT, STANCE, TIME(00, 00, 30, 22) },
{ RIGHT, BACK,  SWING,  TIME(00, 00, 30, 28) },
{ LEFT,  FRONT, SWING,  TIME(00, 00, 30, 28) },
{ LEFT,  BACK,  STANCE, TIME(00, 00, 30, 29) },

{ RIGHT, FRONT, SWING,  TIME(00, 00, 31, 03) },
{ RIGHT, BACK,  STANCE, TIME(00, 00, 31, 03) },
{ LEFT,  FRONT, STANCE, TIME(00, 00, 31, 03) },
{ LEFT,  BACK,  SWING,  TIME(00, 00, 31,  8) },

{ RIGHT, FRONT, STANCE, TIME(00, 00, 31,  9) },
{ RIGHT, BACK,  SWING,  TIME(00, 00, 31, 14) },
{ LEFT,  FRONT, SWING,  TIME(00, 00, 31, 14) },
{ LEFT,  BACK,  STANCE, TIME(00, 00, 31, 14) },

{ RIGHT, FRONT, SWING,  TIME(00, 00, 31, 18) },
{ RIGHT, BACK,  STANCE, TIME(00, 00, 31, 18) },
{ LEFT,  FRONT, STANCE, TIME(00, 00, 31, 18) },
{ LEFT,  BACK,  SWING,  TIME(00, 00, 31, 23) },

{ RIGHT, FRONT, STANCE, TIME(00, 00, 31, 24) },
{ RIGHT, BACK,  SWING,  TIME(00, 00, 31, 28) },
{ LEFT,  FRONT, SWING,  TIME(00, 00, 31, 29) },
{ LEFT,  BACK,  STANCE, TIME(00, 00, 31, 29) },

{ RIGHT, FRONT, SWING,  TIME(00, 00, 32, 02) },
{ RIGHT, BACK,  STANCE, TIME(00, 00, 32, 02) },
{ LEFT,  FRONT, STANCE, TIME(00, 00, 32, 04) },
{ LEFT,  BACK,  SWING,  TIME(00, 00, 32,  9) },

{ RIGHT, FRONT, STANCE, TIME(00, 00, 32,  9) },
{ RIGHT, BACK,  SWING,  TIME(00, 00, 32, 14) },
{ LEFT,  FRONT, SWING,  TIME(00, 00, 32, 14) },
{ LEFT,  BACK,  STANCE, TIME(00, 00, 32, 14) },

{-1, -1, -1, -1}		
};


struct CPG_KeyedGait CPG_Debug_Canter = {
	
	.loop_start = 4,
	.loop_end   = (sizeof(Canter_Sequence) / sizeof(Canter_Sequence[0])) - 1,
	.loop_time  = TIME(00, 00, 32, 14) - TIME(00, 00, 30, 0) + TIME(00, 00, 00, 3),
	.start_time = TIME(00, 00, 30, 0),
	.sequence = Canter_Sequence
};

static struct CPG_KeyedTiming Gallop_Sequence[] =  {	
{ LEFT,  BACK,  STANCE, TIME(00, 00, 50, 0) },	
{ RIGHT, BACK,  SWING,  TIME(00, 00, 50, 0) },
{ LEFT,  FRONT, SWING,  TIME(00, 00, 50, 0) },
{ RIGHT, FRONT, SWING,  TIME(00, 00, 50, 0) },

{ RIGHT, BACK,  STANCE, TIME(00, 00, 50, 1) },
{ RIGHT, FRONT, SWING,  TIME(00, 00, 50, 3) },
{ LEFT,  FRONT, STANCE, TIME(00, 00, 50, 3.5) },
{ LEFT,  BACK,  SWING,  TIME(00, 00, 50, 4) },	

{ RIGHT, FRONT, STANCE, TIME(00, 00, 50, 9) },
{ RIGHT, BACK,  SWING,  TIME(00, 00, 50, 11) },
{ LEFT,  FRONT, SWING,  TIME(00, 00, 50, 14) },
{ LEFT,  BACK,  STANCE, TIME(00, 00, 50, 14) },

{ RIGHT, BACK,  STANCE, TIME(00, 00, 50, 17) },
{ RIGHT, FRONT, SWING,  TIME(00, 00, 50, 18) },
{ LEFT,  FRONT, STANCE, TIME(00, 00, 50, 20) },
{ LEFT,  BACK,  SWING,  TIME(00, 00, 50, 22) },	

{ RIGHT, FRONT, STANCE, TIME(00, 00, 50, 24) },
{ RIGHT, BACK,  SWING,  TIME(00, 00, 50, 26) },
{ LEFT,  FRONT, SWING,  TIME(00, 00, 50, 29) },
{ LEFT,  BACK,  STANCE, TIME(00, 00, 50, 29) },

{ RIGHT, BACK,  STANCE, TIME(00, 00, 51, 02) },
{ RIGHT, FRONT, SWING,  TIME(00, 00, 51, 03) },
{ LEFT,  FRONT, STANCE, TIME(00, 00, 51, 04) },
{ LEFT,  BACK,  SWING,  TIME(00, 00, 51,  8) },	

{ RIGHT, FRONT, STANCE, TIME(00, 00, 51,  9) },
{ RIGHT, BACK,  SWING,  TIME(00, 00, 51, 11) },
{ LEFT,  FRONT, SWING,  TIME(00, 00, 51, 14) },
{ LEFT,  BACK,  STANCE, TIME(00, 00, 51, 14) },

{ RIGHT, BACK,  STANCE, TIME(00, 00, 51, 14) },
{ RIGHT, FRONT, SWING,  TIME(00, 00, 51, 18) },
{ LEFT,  FRONT, STANCE, TIME(00, 00, 51, 20) },
{ LEFT,  BACK,  SWING,  TIME(00, 00, 51, 23) },	

{ RIGHT, FRONT, STANCE, TIME(00, 00, 51, 23) },
{ RIGHT, BACK,  SWING,  TIME(00, 00, 51, 27) },
{ LEFT,  FRONT, SWING,  TIME(00, 00, 51, 29) },
{ LEFT,  BACK,  STANCE, TIME(00, 00, 51, 29) },

{ RIGHT, BACK,  STANCE, TIME(00, 00, 52, 03) },
{ RIGHT, FRONT, SWING,  TIME(00, 00, 52, 03) },
{ LEFT,  FRONT, STANCE, TIME(00, 00, 52, 05) },
{ LEFT,  BACK,  SWING,  TIME(00, 00, 52,  9) },	

{ RIGHT, FRONT, STANCE, TIME(00, 00, 52,  9) },
{ RIGHT, BACK,  SWING,  TIME(00, 00, 52, 11) },
{ LEFT,  FRONT, SWING,  TIME(00, 00, 52, 15) },
{ LEFT,  BACK,  STANCE, TIME(00, 00, 52, 15) },

{-1, -1, -1, -1}	
};

struct CPG_KeyedGait CPG_Debug_Gallop = {
	
	.loop_start = 4,
	.loop_end   = (sizeof(Gallop_Sequence) / sizeof(Gallop_Sequence[0])) - 1,
	.loop_time  = TIME(00, 00, 52, 15) - TIME(00, 00, 49, 28) + TIME(00, 00, 00, 4),
	.start_time = TIME(00, 00, 49, 28),
	.sequence = Gallop_Sequence
};
