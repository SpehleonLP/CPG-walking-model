#include "cpg_walkingmodel.h"
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct KeyedTiming
{
	int8_t isRight;
	int8_t isFront;
	int8_t isStance;
	float  time;
};

struct KeyedGait
{
	int    loop_start;
	int    loop_end;
	float  loop_time;
	struct KeyedTiming * sequence;
};


enum { LEFT=0, RIGHT=1, BACK=0, FRONT=1, SWING=0, STANCE=1 };


#define TIME(hr, min, sec, frame) ((hr*60 + min)*60 + ( sec + (frame/30.0) ))

struct KeyedTiming Walk_Sequence[] =  {	
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

struct KeyedGait Walk = {
	
	.loop_start = 8,
	.loop_end   = (sizeof(Walk_Sequence) / sizeof(Walk_Sequence[0])) - 1,
	.loop_time  = TIME(00, 00, 9, 9) - TIME(00, 00, 01, 2),
	.sequence = Walk_Sequence
};



#if 0
struct KeyedTiming Trot[] = {
{ LEFT, BACK, SWING,  },
{-1, -1, -1, -1}	
	
};

struct KeyedTiming Canter[] = {
{ LEFT, BACK, SWING,  },
{-1, -1, -1, -1}	
	
};

struct KeyedTiming Gallop[] = {
{ LEFT, BACK, SWING,  },
{-1, -1, -1, -1}	
	
};
#endif
void CPG_Update(CPG_Model * model, float dt);

#ifndef M_PI
#define M_PI 3.14159265358
#endif
#define M_2PI (2*M_PI)

struct CPG_neuron
{
	float state;
	float fatigue;
	float output;
};


static float SolveAcceleration(float distance, float time, float maxVelocity)
{
	(void)maxVelocity;
	// distance = 0.5 * acceleration * time*time
	// distance * 2 / (time*time) = = acceleration 
	float acceleration = distance * 2 / (time*time);
	return acceleration;
}

static float SolveHalfAcceleration(float distance, float time, float maxVelocity)
{
	return SolveAcceleration(distance/2, time/2, maxVelocity);
}

struct CPG_Model * CPG_ModelCreate(int noSegments)
{
	int noLegs     = noSegments*2;
	int noNeurons  = noLegs*2;

	size_t size = sizeof(struct CPG_Model) 
				+ sizeof(CPG_neuron) * noNeurons
				+ sizeof(float) * noLegs
				+ sizeof(CPG_Segment) * noSegments
				+ sizeof(char) * noSegments
				+ 16;
	
	
	struct CPG_Model * r = calloc(size, 1);
		
//	r->settings.state_memory_constant		= 0.0473;	
//	r->settings.fatigue_memory_constant		= 0.6; 
//	r->settings.brain_signal_strength		= 1.71;
	
	r->settings.state_memory_half_life		= CPG_ComputeHalfLife(0.473, 0.5);		
	r->settings.fatigue_memory_half_life	= CPG_ComputeHalfLife(0.6, 0.5);	
	
	r->settings.brain_signal_strength		= 1.71;
	
	r->settings.recurrent_inhibition		= 3.0;	
	r->settings.contralateral_inhibition	= 3.0;	
	r->settings.ipsilateral_inhibition		= 0.8;		
	r->settings.sensory_inhibition			= 2.0;
	
	r->settings.foot_feedback_constant		= 7.0 * 0.08;	
	r->settings.hip_feedback_constant		= 3.0;	
	
	
	float swing_time = 0.3;
	float step_time  = 1.2;
	
	r->drivers.unit[PD_Swing][PD_Hip].target		= 1.0;	
	r->drivers.unit[PD_Swing][PD_Hip].maxVelocity  = 0;
	r->drivers.unit[PD_Swing][PD_Hip].acceleration  = SolveHalfAcceleration((1.0 + 0.438), swing_time, 1);
	r->drivers.unit[PD_Swing][PD_Hip].deceleration  = r->drivers.unit[PD_Swing][PD_Hip].acceleration;
	
	r->drivers.unit[PD_Stance][PD_Hip].target		= -0.438;	
	r->drivers.unit[PD_Stance][PD_Hip].maxVelocity  = 0;
	r->drivers.unit[PD_Stance][PD_Hip].acceleration  = SolveHalfAcceleration((1.0 + 0.438), step_time, 1);
	r->drivers.unit[PD_Stance][PD_Hip].deceleration  = r->drivers.unit[PD_Stance][PD_Hip].acceleration;
	
	r->drivers.unit[PD_Swing][PD_Knee].target		= 0.7;	
	r->drivers.unit[PD_Swing][PD_Knee].maxVelocity  = 0;
	r->drivers.unit[PD_Swing][PD_Knee].acceleration  = 50;
	r->drivers.unit[PD_Swing][PD_Knee].deceleration  = 50;
	
	r->drivers.unit[PD_Stance][PD_Knee].target		= 1.1;	
	r->drivers.unit[PD_Stance][PD_Knee].maxVelocity  = 0;
	r->drivers.unit[PD_Stance][PD_Knee].acceleration  = 20;
	r->drivers.unit[PD_Stance][PD_Knee].deceleration  = 1;
	

	/*
	r->drivers.unit[PD_Swing][PD_Hip].target		= 1.312;	
	r->drivers.unit[PD_Swing][PD_Hip].proportional  = 8.13;
	r->drivers.unit[PD_Swing][PD_Hip].derivative	= 1;
	
	r->drivers.unit[PD_Stance][PD_Hip].target		= -0.438;	
	r->drivers.unit[PD_Stance][PD_Hip].proportional = 8.13;
	r->drivers.unit[PD_Stance][PD_Hip].derivative	= 1;
	
	r->drivers.unit[PD_Swing][PD_Knee].target		= 0.5;	
	r->drivers.unit[PD_Swing][PD_Knee].proportional = 1.860;
	r->drivers.unit[PD_Swing][PD_Knee].derivative	= 0.040;
	
	r->drivers.unit[PD_Stance][PD_Knee].target		= 1.00;	
	r->drivers.unit[PD_Stance][PD_Knee].proportional= 1.970;
	r->drivers.unit[PD_Stance][PD_Knee].derivative	= 0.040;*/
	
	*(int*)&r->noSegments = noSegments;	
	*(int*)&r->noNeurons = noNeurons;
	*(int*)&r->noLegs = noLegs;
	
	*(void**)&r->neurons = r+1;	
	*(void**)&r->contact_force = r->neurons + noNeurons;
	*(void**)&r->segments = r->contact_force + noLegs;
	*(void**)&r->legState = r->segments + noSegments;
	
	
	r->debug_time = 0;	
	r->debug_frame = 0;
	r->debug_gait = &Walk;
	
/*	struct CPG_Leg * legs = &r->segments->leg[0];
	for(int i = 0; i < noLegs; ++i)
	{
		legs[i].knee.pos = r->drivers.unit[PD_Stance][PD_Knee].target;
	}
	
	*/
	assert((uint8_t*)r->legState + (noLegs) < (uint8_t*)r + size);
	
// set everything to stance	
	memset(r->legState, PD_Stance, noLegs);
// set knees to half extension
	for(int i = 0; i < noLegs; ++i)
	{
		r->segments[i/2].leg[i&1].hip.pos = 0.262;
		r->segments[i/2].leg[i&1].knee.pos = r->drivers.unit[PD_Stance][PD_Knee].target;
	}
	
	//fclose(fopen("cpg_test_data.txt", "w"));
	
	return r;
}

void CPG_ModelUpdate_PID(CPG_Model * model, float dt);


void CPG_ModelUpdate(CPG_Model * model, float dt)
{		
	if(model->debug_gait)
	{
	//	dt *= 1.5f;
		model->debug_time += dt;
		
		for(;;)
		{
			for(; model->debug_frame < model->debug_gait->loop_end; ++model->debug_frame)
			{
				struct KeyedTiming * key = &model->debug_gait->sequence[model->debug_frame];
				
				if(key->time > model->debug_time)
					goto exit_loop;
				
				if(key->isRight < 0)
					break;
				
				int i = key->isFront*2 + key->isRight;
				model->legState[i] = key->isStance;
			}
		
			model->debug_frame = model->debug_gait->loop_start;
			model->debug_time -= model->debug_gait->loop_time;
		}
		
	exit_loop:	
		CPG_ModelUpdate_PID(model, dt);
		return;
	}
	
	
	
#if 0
	dt*=2;
	int noLegs =  model->noSegments*2;
	struct CPG_Leg * legs = &model->segments->leg[0];

	float tick_rate = 0.5;
#if 1
	if((model->biotick += dt) > tick_rate)
	{
	//	do
	//	{
			CPG_Update(model, tick_rate);
	//	} while((model->biotick -= tick_rate) > tick_rate); 	
		
			model->biotick = 0;
		memset(model->contact_force, 0, sizeof(float)*model->noLegs);
#if 1
		for(int i = 0; i < noLegs; ++i)
		{
			float extensor = model->neurons[i*2 + 0].output;
			float flexor   = model->neurons[i*2 + 1].output;
						
			if(extensor != flexor)
			{
				model->legState[i] = extensor > flexor;
			}
		}
#endif
	}
	
#endif
#endif
	
	CPG_ModelUpdate_PID(model, dt);
	
}

void CPG_ModelUpdate_PID(CPG_Model * model, float dt)
{		
//FILE *fp = fopen("cpg_test_data.txt", "a");
//	
//	fprintf(fp, "%f\t", total_time);
	
	float halfDt = dt*0.5f;
	
	int noJoints = model->noSegments*4;
	struct CPG_Joint * joints = &model->segments->leg[0].hip;
	
	struct min_max { float min, max; };
	struct min_max limit[2];
	
	for(int i = 0; i < 2; ++i)
	{	
		limit[i].min = model->drivers.unit[PD_Swing][i].target;
		limit[i].max = model->drivers.unit[PD_Stance][i].target;
		
		if(limit[i].min > limit[i].max)
		{
			float eax = limit[i].min;
			limit[i].min = limit[i].max;
			limit[i].max = eax;
		}
	}
	
	for(int i = 0; i < noJoints; ++i)
	{
		struct Accel_Driver * driver = &model->drivers.unit[(int)model->legState[i/2]][i % 2];
		
		if(joints[i].pos == driver->target)
			continue;
		
	//	if((driver->target - joints[i].pos) * joints[i].velocity < 0)
	//		joints[i].velocity = 0;
	 
		float acceleration = 0.f;
		
		int direction = joints[i].velocity > 0? -1 : 1;
		float time_0_velo = fabs(joints[i].velocity) / driver->deceleration;
		float pos_0_velo = joints[i].pos + time_0_velo * (joints[i].velocity + 0.5 * driver->deceleration * time_0_velo * direction);
		
		float ratio = (pos_0_velo - joints[i].pos) / (driver->target - joints[i].pos);
			
		if(ratio >= 1.f)
		{
// p' = p + vt + .5at^2
// p' - p - vt = .5at^2
// 2*(p' - p - vt)/t^2 = a
			acceleration = 2 * (driver->target - joints[i].pos - joints[i].velocity * time_0_velo) / (time_0_velo * time_0_velo);			
	//		acceleration = driver->deceleration * direction * ratio* ratio; 
		}
		else
			acceleration = (joints[i].pos < driver->target? 1 : -1) * driver->acceleration;
		
		acceleration *= halfDt;
		joints[i].velocity += acceleration;
		joints[i].pos += joints[i].velocity * dt;
		joints[i].velocity += acceleration;
			
		if(driver->maxVelocity && fabs(joints[i].velocity) > driver->maxVelocity)
			joints[i].velocity = (joints[i].velocity < 0? -1 : 1) * driver->maxVelocity;
			
		joints[i].velocity *= (joints[i].pos != driver->target);
		
		if(joints[i].pos < limit[i%2].min)
		{
			joints[i].pos = limit[i%2].min;
			joints[i].velocity = 0;
		}	
		
		if(joints[i].pos > limit[i%2].max)
		{
			joints[i].pos = limit[i%2].max;
			joints[i].velocity = 0;
		}	
		
		
#if 0
		float hip_error = model->drivers.unit[s][PD_Hip].target	  - legs[i].hip.pos;
		float knee_error = model->drivers.unit[s][PD_Knee].target - legs[i].knee.pos;
		
		float p_hip_error = model->drivers.unit[s][PD_Hip].target	- legs[i].hip.p_pos;
		float p_knee_error = model->drivers.unit[s][PD_Knee].target - legs[i].knee.p_pos;
		
		float d_hip_error  = (hip_error - p_hip_error);		
		float d_knee_error = (knee_error - p_knee_error);
		
		float hip_pd = model->drivers.unit[s][PD_Hip].proportional   * (hip_error  + model->drivers.unit[s][PD_Hip].derivative * d_hip_error);		
		float knee_pd = model->drivers.unit[s][PD_Knee].proportional * (knee_error + model->drivers.unit[s][PD_Knee].derivative * d_knee_error);	
			
		legs[i].hip.p_pos = legs[i].hip.pos;		
		legs[i].knee.p_pos = legs[i].knee.pos;

	//	float hip_acceleration = (hip_pd - legs[i].hip.velocity) * halfDt;
	//	float knee_acceleration = (knee_pd - legs[i].knee.velocity) * halfDt;
		
		legs[i].hip.velocity = hip_pd;		
		legs[i].knee.velocity = knee_pd;	
		
		legs[i].hip.pos  += hip_pd * dt;
		legs[i].knee.pos += knee_pd * dt;	
		
	//	legs[i].hip.velocity += hip_acceleration;		
		//legs[i].knee.velocity += knee_acceleration;	
		
		float n_hip_error = model->drivers.unit[s][PD_Hip].target	- legs[i].hip.pos;
		float n_knee_error = model->drivers.unit[s][PD_Knee].target - legs[i].knee.pos;
		
		if(n_hip_error * hip_error <= 0)
		{
			legs[i].hip.pos = model->drivers.unit[s][PD_Hip].target;
			legs[i].hip.velocity = 0;
		}
		
		if(n_knee_error * knee_error <= 0)
		{
			legs[i].knee.pos = model->drivers.unit[s][PD_Knee].target;
			legs[i].knee.velocity = 0;
		}
		
	//	if(i == 1)
	//	fprintf(fp, "%f\t%f\t%f\t%f\t%f", legs[i].hip.pos, legs[i].hip.velocity, hip_error, d_hip_error, hip_pd);
		
		assert((void*)&legs[i] < (void*)&model->legState[0]);
#endif
	}	
}

void PD_ModelSpline(PD_Model * dst, PD_Model const* src, float t)
{
	enum 
	{
		NoElements = sizeof(*src)/sizeof(float)
	};
	
	float * r = (float*)dst;
	
	float const* p0 = (float*)src;
	float const* p1 = (float*)(src+1);
	float const* p2 = (float*)(src+2);
	float const* p3 = (float*)(src+3);	
	
	float t3 = t*t*t;
	float invt_t_3 = 3 * (1 - t) * t;
	float invT3 = (1 - t) * (1 - t) * (1 - t);
	
	for(int i = 0; i < NoElements; ++i)
	{
		r[i] = invT3 * p0[i]
			 + invt_t_3 * ((1 - t) * p1[i] + t * p2[i])
			 + t3 * p3[i];
	}
}

void PD_ModelLerp(PD_Model * dst, PD_Model const* src0, PD_Model const* src1, float t)
{
	enum 
	{
		NoElements = sizeof(*src0)/sizeof(float)
	};
	
	float * r = (float*)dst;
	
	float const* p0 = (float*)src0;
	float const* p1 = (float*)src1;
		
	for(int i = 0; i < NoElements; ++i)
	{
		r[i] = p0[i] * (1 - t) + p1[i] * t;
	}
}

/// If optimized further than this the algorithm no longer works
/// its not like game of life where everything computes at once
/// the algorithm relies on i being computed before i+1
/// so SIMD just breaks it.

#define MULTILEG 1

void CPG_Update(CPG_Model * model, float dt)
{	
	CPG_constants const* constants = &model->settings;
	CPG_neuron * neurons = model->neurons;
	float * weight_on_foot = model->contact_force;
	
	struct CPG_Leg * legs = &model->segments->leg[0];
	
	static float total_time = 0;
	total_time += dt;
	
	float state_memory_constant = 1.f - pow(0.5, dt / constants->state_memory_half_life);	
	float fatigue_memory_constant = 1.f- pow( 0.5,  dt / constants->fatigue_memory_half_life);

	
// FIRST LOOP IS EXTENSOR; if output enter stance
// extensor 1.0 + flexor 0.0 = PD_Stance
// extensor 0.0 + flexor 1.0 = PD_Swing
	
//	float state_memory_constant = constants->state_memory_half_life;	
//	float fatigue_memory_constant = constants->fatigue_memory_half_life;
	
//	fprintf(stdout, "\n");
	
	for(int i = 0; i < model->noNeurons; i += 2)
	{
		int segment = i / 4;
		int side    = (i & 0x02);
		int opposite_neuron = side? i - 2 : i + 2;
		
		float inhibition = 0;
		
		// extension and flexation inhibit each other within the same CPG unit
		inhibition = constants->sensory_inhibition * neurons[i+1].output;		
		
		// flexation laterally inhibits flexation		

		inhibition += constants->contralateral_inhibition * neurons[opposite_neuron].output;
#if MULTILEG		 
		if(segment != 0)
		{
			inhibition += constants->ipsilateral_inhibition * neurons[i-4].output;
		}
		if(segment < model->noSegments)
		{
			inhibition += constants->ipsilateral_inhibition * neurons[i+4].output;
		}
#endif
		
		inhibition += neurons[i].fatigue;
		
		float error = fabs(legs[i/2].hip.pos - model->drivers.unit[PD_Swing][PD_Hip].target);
	//	inhibition += error * constants->hip_feedback_constant;
		
//		excitation += legs[i/2].hip.pos * constants->hip_feedback_constant;
		float state = state_memory_constant * neurons[i].state + (1.0 - inhibition);
		
		
		neurons[i].state 	= state;
		neurons[i].output   = (state > 0);
		neurons[i].fatigue  = fatigue_memory_constant * neurons[i].fatigue + neurons[i].output * constants->recurrent_inhibition;		
	//	fprintf(stdout, "%f\t%f\t%f\n", state, neurons[i].output,neurons[i].fatigue);
	}
	
	
// FIRST LOOP IS FLEXOR	 if output enter swing
	for(int i = 1; i < model->noNeurons; i += 2)
	{
		int segment = i / 4;
		int side    = (i & 0x02);
		int opposite_neuron = side? i - 2 : i + 2;
		
		float inhibition = 0;
		
		// extension and flexation inhibit each other within the same CPG unit
		inhibition = constants->sensory_inhibition * neurons[i-1].output;		
		
		// flexation laterally inhibits flexation		
		inhibition += constants->contralateral_inhibition * neurons[opposite_neuron].output;
		
#if MULTILEG
		if(segment != 0)
		{
			inhibition += constants->ipsilateral_inhibition * neurons[i-4].output;
		}
		if(segment < model->noSegments)
		{
			inhibition += constants->ipsilateral_inhibition * neurons[i+4].output;
		}
#endif
		
		inhibition += neurons[i].fatigue;
		
		float error = fabs(legs[i/2].hip.pos - model->drivers.unit[PD_Stance][PD_Hip].target);
		
	//	inhibition += error * constants->hip_feedback_constant;
		
		float state = state_memory_constant * neurons[i].state + (1.0 - inhibition);
		
		neurons[i].state 	= state;
		neurons[i].output   = (state > 0);
		neurons[i].fatigue  = fatigue_memory_constant * neurons[i].fatigue + neurons[i].output * constants->recurrent_inhibition;		
	//	fprintf(stdout, "%f\t%f\t%f\n", state, neurons[i].output,neurons[i].fatigue);
	}
	
	
#if 0
	FILE *fp = fopen("cpg_test_data.txt", "a");
	
	fprintf(fp, "%f", total_time);
	
	for(int i = 0; i < size; i += 2)
	{
		fprintf(fp, "\t%f", neurons[i].output - neurons[i+1].output);
	}
	
	fprintf(fp, "\t");
	fclose(fp);
#endif
	/*
	for(int i = 0; i < size; ++i)
	{		
		// extension and flexation inhibit each other within the same CPG unit
		__m128 swizzle0 = _mm_set_ps(neurons[i].output[1], neurons[i].output[0], neurons[i].output[3], neurons[i].output[2]);	
		
		// extension laterally inhibits extension
		// flexation laterally inhibits flexation
		__m128 swizzle1 = _mm_set_ps(neurons[i].output[2], neurons[i].output[3], neurons[i].output[0], neurons[i].output[1]);
		
		state = constants->sensory_inhibition * swizzle0;		
		state += constants->contralateral_inhibition * swizzle1;
		 
		if(i != 0)
		{
			state += constants->ipsilateral_inhibition * neurons[i-1].output;
		}
		if(i+1 < size)
		{
			state += constants->ipsilateral_inhibition * neurons[i+1].output;
		}
		
		state += constants->recurrent_inhibition * neurons[i].fatigue;
		state = constants->brain_signal_strength - state;
		
		state[0] += senses[i*2+0].hip_theta * constants->hip_feedback_constant;
		state[1] += senses[i*2+0].hip_theta *-constants->hip_feedback_constant + senses[i*2+0].weight_on_foot * constants->foot_feedback_constant;
		state[2] += senses[i*2+1].hip_theta * constants->hip_feedback_constant;
		state[3] += senses[i*2+1].hip_theta *-constants->hip_feedback_constant + senses[i*2+1].weight_on_foot * constants->foot_feedback_constant;
		
		neurons[i].state 	= state - constants->state_memory_constant * neurons[i].state;
		neurons[i].fatigue  = neurons[i].output - constants->fatigue_memory_constant * neurons[i].fatigue;
		
		__v4si eax = -(neurons[i].state > 0);
		neurons[i].output   = neurons[i].state * _mm_set_ps(eax[0], eax[1], eax[2], eax[3]);
	};	*/
}


float CPG_GetCenterOfGravity(float *__restrict dst, float const*__restrict xyz, float const*__restrict mass, int size)
{
	float accumulator[3] = {0, 0, 0};
	float total_mass = 0;
	
	for(int i = 0; i < size; ++i)
	{
		accumulator[0] += xyz[i*3+0] * mass[i];
		accumulator[1] += xyz[i*3+1] * mass[i];
		accumulator[2] += xyz[i*3+2] * mass[i];
		total_mass += mass[i];
	}
	
	float invMass = total_mass? 1.f / total_mass : 1.f;
	
	dst[0] = accumulator[0] * invMass;	
	dst[1] = accumulator[1] * invMass;	
	dst[2] = accumulator[2] * invMass;
		
	return total_mass;
}

float CPG_GetCenterOfGravity_Stride(float *__restrict dst, char const*__restrict xyz, int stride, float const*__restrict mass, int size)
{
	float accumulator[3] = {0, 0, 0};
	float total_mass = 0;
	
	for(int i = 0; i < size; ++i)
	{
		float * p = (float*)(xyz + stride*i);
		
		accumulator[0] += p[0] * mass[i];
		accumulator[1] += p[1] * mass[i];
		accumulator[2] += p[2] * mass[i];
		total_mass += mass[i];
	}
	
	float invMass = total_mass? 1.f / total_mass : 1.f;
	
	dst[0] = accumulator[0] * invMass;	
	dst[1] = accumulator[1] * invMass;	
	dst[2] = accumulator[2] * invMass;
		
	return total_mass;
}

float CPG_ComputeHalfLife(float percent_remaining, float time)
{
	return time / log2(percent_remaining) / -1.f;
}


float CPG_ComputeMomentOfInertiaMat3(float *__restrict mat3_out, float const*__restrict center_of_graivty_xyz, float const*__restrict xyz, float const*__restrict mass, int size)
{	
	float mat3[3][3];
	float r = 0;
	
	if(center_of_graivty_xyz == NULL || xyz == NULL || mass == NULL)
	{
		if(mat3_out)
			memset(mat3_out, 0, sizeof(mat3));
		
		return 0.f;
	}
	
	if(mat3_out == NULL)
	{
		for(int i = 0; i < size; ++i)
		{
			float d[3] = {
				xyz[i*3+0] - center_of_graivty_xyz[0], 
				xyz[i*3+1] - center_of_graivty_xyz[1], 
				xyz[i*3+2] - center_of_graivty_xyz[2]
			};
			
			float length2 = d[0]*d[0] + d[1]*d[1] + d[2]*d[2];
			r += length2 * mass[i];			
		}
	}
	else
	{
		memset(mat3, 0, sizeof(mat3));
		
		for(int i = 0; i < size; ++i)
		{
			float d[3] = {
				xyz[i*3+0] - center_of_graivty_xyz[0], 
				xyz[i*3+1] - center_of_graivty_xyz[1], 
				xyz[i*3+2] - center_of_graivty_xyz[2]
			};
			
			float length2 = d[0]*d[0] + d[1]*d[1] + d[2]*d[2];
			r += length2 * mass[i];		
			
			// unroll loop for better optimization
			d[0] = fabs(d[0]);			
			d[1] = fabs(d[1]);			
			d[2] = fabs(d[2]);

// unroll loop for better optimization
			mat3[0][0] += (d[1]*d[1] + d[2]*d[2]) * mass[i];
			mat3[0][1] -= d[0]*d[1] * mass[i];
			mat3[0][2] -= d[0]*d[2] * mass[i];
			mat3[1][0] -= d[1]*d[0] * mass[i];
			mat3[1][1] += (d[0]*d[0] + d[2]*d[2]) * mass[i];
			mat3[1][2] -= d[1]*d[2] * mass[i];
			mat3[2][0] -= d[2]*d[0] * mass[i];
			mat3[2][1] -= d[2]*d[1] * mass[i];
			mat3[2][2] += (d[0]*d[0] + d[1]*d[1]) * mass[i];
		}
	}

	
	if(mat3_out)
		memcpy(mat3_out, &mat3[0][0], 9*sizeof(float));	

	return r;
}

float CPG_ComputeMomentOfInertiaMat3_Stride(float *__restrict mat3_out, float const*__restrict center_of_graivty_xyz, char const*__restrict xyz, int stride, float const*__restrict mass, int size)
{
	float mat3[3][3];
	float r = 0;
	
	if(center_of_graivty_xyz == NULL || xyz == NULL || mass == NULL)
	{
		if(mat3_out)
			memset(mat3_out, 0, sizeof(mat3));
		
		return 0.f;
	}
	
	if(mat3_out == NULL)
	{
		for(int i = 0; i < size; ++i)
		{
			float * p = (float*)(xyz + stride*i);
			
			float d[3] = {
				p[0] - center_of_graivty_xyz[0], 
				p[1] - center_of_graivty_xyz[1], 
				p[2] - center_of_graivty_xyz[2]
			};
			
			float length2 = d[0]*d[0] + d[1]*d[1] + d[2]*d[2];
			r += length2 * mass[i];			
		}
	}
	else
	{
		memset(mat3, 0, sizeof(mat3));
		
		for(int i = 0; i < size; ++i)
		{
			float * p = (float*)(xyz + stride*i);
			
			float d[3] = {
				p[0] - center_of_graivty_xyz[0], 
				p[1] - center_of_graivty_xyz[1], 
				p[2] - center_of_graivty_xyz[2]
			};
			
			float length2 = d[0]*d[0] + d[1]*d[1] + d[2]*d[2];
			r += length2 * mass[i];		
			
			d[0] = fabs(d[0]);			
			d[1] = fabs(d[1]);			
			d[2] = fabs(d[2]);

// unroll loop for better optimization
			mat3[0][0] += (d[1]*d[1] + d[2]*d[2]) * mass[i];
			mat3[0][1] -= d[0]*d[1] * mass[i];
			mat3[0][2] -= d[0]*d[2] * mass[i];
			mat3[1][0] -= d[1]*d[0] * mass[i];
			mat3[1][1] += (d[0]*d[0] + d[2]*d[2]) * mass[i];
			mat3[1][2] -= d[1]*d[2] * mass[i];
			mat3[2][0] -= d[2]*d[0] * mass[i];
			mat3[2][1] -= d[2]*d[1] * mass[i];
			mat3[2][2] += (d[0]*d[0] + d[1]*d[1]) * mass[i];
		}
	}

	
	if(mat3_out)
		memcpy(mat3_out, &mat3[0][0], 9*sizeof(float));	

	return r;	
}
