#ifndef DEBUG_KEYED_TIMINGS_H
#define DEBUG_KEYED_TIMINGS_H
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct CPG_KeyedTiming
{
	int8_t isRight;
	int8_t isFront;
	int8_t isStance;
	float  time;
};

struct CPG_KeyedGait
{
	int    loop_start;
	int    loop_end;
	float  loop_time;
	struct CPG_KeyedTiming * sequence;
};

extern struct CPG_KeyedGait CPG_Debug_Walk;
extern struct CPG_KeyedGait CPG_Debug_Trot;
extern struct CPG_KeyedGait CPG_Debug_Canter;
extern struct CPG_KeyedGait CPG_Debug_Gallop;


#ifdef __cplusplus
}
#endif


#endif // DEBUG_KEYED_TIMINGS_H
