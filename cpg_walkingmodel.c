#include "cpg_walkingmodel.h"
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

void CPG_Update(CPG_Model * model, float dt);

struct CPG_neuron
{
	float state;
	float fatigue;
	float output;
};

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
	
	r->settings.state_memory_half_life		= CPG_ComputeHalfLife(0.0473, 0.1);		
	r->settings.fatigue_memory_half_life	= CPG_ComputeHalfLife(0.6, 0.1);	
	
	r->settings.brain_signal_strength		= 1.71;
	
	r->settings.recurrent_inhibition		= 3.0;	
	r->settings.contralateral_inhibition	= 0.3;	
	r->settings.ipsilateral_inhibition		= 0.8;		
	r->settings.sensory_inhibition			= 2.0;
	
	r->settings.foot_feedback_constant		= 7.0 * 0.08;	
	r->settings.hip_feedback_constant		= 3.0;	
	
	r->drivers.unit[PD_Swing][PD_Hip].target		= 1.312;	
	r->drivers.unit[PD_Swing][PD_Hip].proportional  = 8.13;
	r->drivers.unit[PD_Swing][PD_Hip].derivative	= 1;
	
	r->drivers.unit[PD_Stance][PD_Hip].target		= -0.438;	
	r->drivers.unit[PD_Stance][PD_Hip].proportional = 8.13;
	r->drivers.unit[PD_Stance][PD_Hip].derivative	= 1;
	
	r->drivers.unit[PD_Swing][PD_Knee].target		= 0.57;	
	r->drivers.unit[PD_Swing][PD_Knee].proportional = 1.860;
	r->drivers.unit[PD_Swing][PD_Knee].derivative	= 0.40;
	
	r->drivers.unit[PD_Stance][PD_Knee].target		= 1.30;	
	r->drivers.unit[PD_Stance][PD_Knee].proportional= 1.970;
	r->drivers.unit[PD_Stance][PD_Knee].derivative	= 0.40;
	
	*(int*)&r->noSegments = noSegments;	
	*(int*)&r->noNeurons = noNeurons;
	*(int*)&r->noLegs = noLegs;
	
	*(void**)&r->neurons = r+1;	
	*(void**)&r->contact_force = r->neurons + noNeurons;
	*(void**)&r->segments = r->contact_force + noLegs;
	*(void**)&r->legState = r->segments + noSegments;
	
	
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

void CPG_ModelUpdate(CPG_Model * model, float dt)
{		
	dt*=2;
	int noLegs =  model->noSegments*2;
	struct CPG_Leg * legs = &model->segments->leg[0];

	float tick_rate = 0.1;
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
#if 0
		FILE *fp = fopen("cpg_test_data.txt", "a");
		
		for(int i = 0; i < noLegs; ++i)
		{
			fprintf(fp, "%c ", model->legState[i]? 'x' : ' ');
		}
		
		fprintf(fp, "\n");
		fclose(fp);
#endif
	}
#else
	
	CPG_Update(&model->settings, model->neurons, model->noNeurons, model->senses, model->noSenses, dt);
	
#endif

	
	float invDt = dt? 1.f / dt : 1.f;
	float halfDt = dt*0.5;
	
//FILE *fp = fopen("cpg_test_data.txt", "a");
//	
//	fprintf(fp, "%f\t", total_time);

	
	for(int i = 0; i < noLegs; ++i)
	{
		int s = model->legState[i];		
		
		float hip_error = model->drivers.unit[s][PD_Hip].target	  - legs[i].hip.pos;
		float knee_error = model->drivers.unit[s][PD_Knee].target - legs[i].knee.pos;
		
		float p_hip_error = model->drivers.unit[s][PD_Hip].target	- legs[i].hip.p_pos;
		float p_knee_error = model->drivers.unit[s][PD_Knee].target - legs[i].knee.p_pos;
		
		float d_hip_error  = (hip_error - p_hip_error) * invDt;		
		float d_knee_error = (knee_error - p_knee_error) * invDt;
		
		float hip_pd = model->drivers.unit[s][PD_Hip].proportional   * (hip_error  + model->drivers.unit[s][PD_Hip].derivative * d_hip_error);		
		float knee_pd = model->drivers.unit[s][PD_Knee].proportional * (knee_error + model->drivers.unit[s][PD_Knee].derivative * d_knee_error);
		
		legs[i].hip.p_pos = legs[i].hip.pos;		
		legs[i].knee.p_pos = legs[i].knee.pos;
		
		legs[i].hip.velocity += hip_pd * halfDt;		
		legs[i].knee.velocity += knee_pd * halfDt;	
		
		legs[i].hip.pos  += legs[i].hip.velocity * dt;
		legs[i].knee.pos += legs[i].knee.velocity * dt;	
		
		legs[i].hip.velocity += hip_pd * halfDt;		
		legs[i].knee.velocity += knee_pd * halfDt;	
		
	//	if(i == 1)
	//	fprintf(fp, "%f\t%f\t%f\t%f\t%f", legs[i].hip.pos, legs[i].hip.velocity, hip_error, d_hip_error, hip_pd);
		
		assert((void*)&legs[i] < (void*)&model->legState[0]);
	}
	
//	fprintf(fp, "\n");
	//fclose(fp);
	
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

void CPG_Update(CPG_Model * model, float dt)
{	
	CPG_constants const* constants = &model->settings;
	CPG_neuron * neurons = model->neurons;
	float * weight_on_foot = model->contact_force;
	
	struct CPG_Leg * legs = &model->segments->leg[0];
	
	static float total_time = 0;
	total_time += dt;
	
	float state_memory_constant = pow(0.5, dt / constants->state_memory_half_life);	
	float fatigue_memory_constant = pow( 0.5,  dt / constants->fatigue_memory_half_life);
	
//	float state_memory_constant = constants->state_memory_half_life;	
//	float fatigue_memory_constant = constants->fatigue_memory_half_life;
	
	for(int i = 0; i < model->noNeurons; i += 2)
	{
		int segment = i / 4;
		int side    = (i & 0x02);
		int opposite_neuron = side? i - 2 : i + 2;
		
		float inhibition = 0;
		float excitation = 0; 
		
		// extension and flexation inhibit each other within the same CPG unit
		inhibition = constants->sensory_inhibition * neurons[i+1].output;		
		
		// flexation laterally inhibits flexation		
		inhibition += constants->contralateral_inhibition * neurons[opposite_neuron].output;
		 
		if(segment != 0)
		{
			inhibition += constants->ipsilateral_inhibition * neurons[i-4].output;
		}
		if(segment < model->noSegments)
		{
			inhibition += constants->ipsilateral_inhibition * neurons[i+4].output;
		}
		
		inhibition += constants->recurrent_inhibition * neurons[i].fatigue;
		
		excitation = constants->brain_signal_strength;
		
		excitation += legs[i/2].hip.pos * constants->hip_feedback_constant;
		float state = state_memory_constant * neurons[i].state + (excitation - inhibition);
		
		neurons[i].state 	= state;
		neurons[i].fatigue  = fatigue_memory_constant * neurons[i].fatigue + neurons[i].output;
		neurons[i].output   = state * (state > 0);
	}
	
	for(int i = 1; i < model->noNeurons; i += 2)
	{
		int segment = i / 4;
		int side    = (i & 0x02);
		int opposite_neuron = side? i - 2 : i + 2;
		
		float inhibition = 0;
		float excitation = 0; 
		
		// extension and flexation inhibit each other within the same CPG unit
		inhibition = constants->sensory_inhibition * neurons[i-1].output;		
		
		// flexation laterally inhibits flexation		
		inhibition += constants->contralateral_inhibition * neurons[opposite_neuron].output;
		 
		if(segment != 0)
		{
			inhibition += constants->ipsilateral_inhibition * neurons[i-4].output;
		}
		if(segment < model->noSegments)
		{
			inhibition += constants->ipsilateral_inhibition * neurons[i+4].output;
		}
		
		inhibition += constants->recurrent_inhibition * neurons[i].fatigue;
		excitation = constants->brain_signal_strength;
		
		inhibition += legs[i/2].hip.pos *-constants->hip_feedback_constant;
		excitation += weight_on_foot[i/2] * constants->foot_feedback_constant;
		
		float state = state_memory_constant * neurons[i].state + (excitation - inhibition);
		
		neurons[i].state 	= state;
		neurons[i].fatigue  = fatigue_memory_constant * neurons[i].fatigue + neurons[i].output;
		neurons[i].output   = state * (state > 0);
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
