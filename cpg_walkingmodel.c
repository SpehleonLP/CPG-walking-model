#include "cpg_walkingmodel.h"
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

void CPG_Update(CPG_constants const* constants, CPG_neuron * neurons, int size, CPG_senses const* senses, int senses_size);

struct CPG_neuron
{
	float state;
	float fatigue;
	float output;
};

struct CPG_Model * CPG_ModelCreate(int noSegments)
{
	int noSenses     = noSegments*2;
	int noNeurons    = noSenses*2;

	size_t size = sizeof(struct CPG_Model) 
				+ sizeof(CPG_neuron) * noNeurons
				+ sizeof(CPG_senses) * noSenses
				+ sizeof(CPG_Segment) * noSegments
				+ sizeof(char) * noSegments
				+ 16;
	
	
	struct CPG_Model * r = calloc(size, 1);
		
	r->settings.state_memory_constant		= 0.0473;	
	r->settings.fatigue_memory_constant		= 0.6; 
	r->settings.brain_signal_strength		= 1.71;
	
	r->settings.recurrent_inhibition		= 3.0;	
	r->settings.contralateral_inhibition	= 0.3;	
	r->settings.ipsilateral_inhibition		= 0.8;		
	r->settings.sensory_inhibition			= 2.0;
	
	r->settings.foot_feedback_constant		= 0.08;	
	r->settings.hip_feedback_constant		= 3.0;	
	
	r->drivers.unit[PD_Swing][PD_Hip].target		= 1.312;	
	r->drivers.unit[PD_Swing][PD_Hip].proportional  = 8.13;
	r->drivers.unit[PD_Swing][PD_Hip].derivative	= 1;
	
	r->drivers.unit[PD_Stance][PD_Hip].target		= -0.438;	
	r->drivers.unit[PD_Stance][PD_Hip].proportional = 8.13;
	r->drivers.unit[PD_Stance][PD_Hip].derivative	= 1;
	
	r->drivers.unit[PD_Swing][PD_Knee].target		= 0.57;	
	r->drivers.unit[PD_Swing][PD_Knee].proportional = 1.86;
	r->drivers.unit[PD_Swing][PD_Knee].derivative	= 2;
	
	r->drivers.unit[PD_Stance][PD_Knee].target		= 1.30;	
	r->drivers.unit[PD_Stance][PD_Knee].proportional= 1.97;
	r->drivers.unit[PD_Stance][PD_Knee].derivative	= 2;
	
	*(int*)&r->noSegments = noSegments;	
	*(int*)&r->noNeurons = noNeurons;
	*(int*)&r->noSenses = noSenses;
	
	*(void**)&r->neurons = r+1;	
	*(void**)&r->senses = r->neurons + noNeurons;
	*(void**)&r->segments = r->senses + noSenses;
	*(void**)&r->legState = r->segments + noSegments;
	
	assert(r->legState + (noSenses) < (uint8_t*)r + size);
	
	fclose(fopen("cpg_test_data.txt", "w"));
	
	return r;
}

void CPG_ModelUpdate(CPG_Model * model, float dt)
{	
	int noLegs =  model->noSegments*2;
	struct CPG_Leg * legs = &model->segments->leg[0];

	if((model->biotick += dt) > 0.1)
	{
		for(int i = 0; i < noLegs; ++i)
			model->senses[i].hip_theta = legs[i].hip.pos;	
		
		do
		{
			CPG_Update(&model->settings, model->neurons, model->noNeurons, model->senses, model->noSenses);
		} while((model->biotick -= 0.1) > 0.1); 
		
		for(int i = 0; i < noLegs; ++i)
		{
			float extensor = model->neurons[i*2 + 0].output;
			float flexor   = model->neurons[i*2 + 1].output;
			
			int p = model->legState[i];
			
			if(extensor != flexor)
				model->legState[i] = extensor > flexor;
			
			if(p != model->legState[i] && i == 0)
			{
				fprintf(stderr, "switch!");	
			}
		}
	}
	
	dt *= 0.5;
	
	for(int i = 0; i < noLegs; ++i)
	{
		int s = model->legState[i];		
		
		float torque = model->drivers.unit[s][PD_Hip].proportional * (model->drivers.unit[s][PD_Hip].target - legs[i].hip.pos) - model->drivers.unit[s][PD_Hip].derivative * legs[i].hip.velo;		
		float accel = model->drivers.unit[s][PD_Knee].proportional * (model->drivers.unit[s][PD_Knee].target - legs[i].knee.pos) - model->drivers.unit[s][PD_Knee].derivative * legs[i].knee.velo;
		
		legs[i].hip.velo += torque * dt;		
		legs[i].hip.pos += legs[i].hip.velo * dt;		
		legs[i].hip.velo += torque * dt;		
		
		legs[i].knee.velo += accel * dt;		
		legs[i].knee.pos  += legs[i].knee.velo * dt;		
		legs[i].knee.velo += accel * dt;	
		
		assert(&legs[i] < &model->legState[0]);
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

void CPG_Update(CPG_constants const* constants, CPG_neuron * neurons, int size, CPG_senses const* senses, int senses_size)
{
	assert(size % 4 == 0);
// operate on pairs of legs each pair has 2 neurons
	assert(senses_size = size / 2);
	
	int noSegments = size / 4;
	
	for(int i = 0; i < size; i += 2)
	{
		int segment = i / 4;
		int side    = (i & 0x02);
		int opposite_neuron = side? i - 2 : i + 2;
		
		float		
		// extension and flexation inhibit each other within the same CPG unit
		state = constants->sensory_inhibition * neurons[i+1].output;		
		
		// flexation laterally inhibits flexation		
		state += constants->contralateral_inhibition * neurons[opposite_neuron].output;
		 
		if(segment != 0)
		{
			state += constants->ipsilateral_inhibition * neurons[i-4].output;
		}
		if(segment < noSegments)
		{
			state += constants->ipsilateral_inhibition * neurons[i+4].output;
		}
		
		state += constants->recurrent_inhibition * neurons[i].fatigue;
		state = constants->brain_signal_strength - state;
		
		state += senses[i/2].hip_theta * constants->hip_feedback_constant;
		state = state + constants->state_memory_constant * neurons[i].state;
		
		neurons[i].state 	= state;
		neurons[i].fatigue  = neurons[i].output + constants->fatigue_memory_constant * neurons[i].fatigue;
		neurons[i].output   = state * (state > 0);
	}
	
	for(int i = 1; i < size; i += 2)
	{
		int segment = i / 4;
		int side    = (i & 0x02);
		int opposite_neuron = side? i - 2 : i + 2;
		
		float		
		// extension and flexation inhibit each other within the same CPG unit
		state = constants->sensory_inhibition * neurons[i-1].output;		
		
		// flexation laterally inhibits flexation		
		state += constants->contralateral_inhibition * neurons[opposite_neuron].output;
		 
		if(segment != 0)
		{
			state += constants->ipsilateral_inhibition * neurons[i-4].output;
		}
		if(segment < noSegments)
		{
			state += constants->ipsilateral_inhibition * neurons[i+4].output;
		}
		
		state += constants->recurrent_inhibition * neurons[i].fatigue;
		state = constants->brain_signal_strength - state;
		
		state += senses[i/2].hip_theta *-constants->hip_feedback_constant + senses[i/2].weight_on_foot * constants->foot_feedback_constant;
		state = state + constants->state_memory_constant * neurons[i].state;
		
		neurons[i].state 	= state;
		neurons[i].fatigue  = neurons[i].output + constants->fatigue_memory_constant * neurons[i].fatigue;
		neurons[i].output   = state * (state > 0);
	}
	
	FILE *fp = fopen("cpg_test_data.txt", "a");
	
	for(int i = 0; i < size; ++i)
	{
		fprintf(fp, "%f\t", neurons[i].output);
	}
	
	fprintf(fp, "\n");
	fclose(fp);
	
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

