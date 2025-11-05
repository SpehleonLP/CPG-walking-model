#include "cpg_universalmodel.h"
#include <stdlib.h>
#include <stddef.h>

struct CPG_constants default_CPG_constants()
{
	struct CPG_constants s;
	
	s. state_memory_half_life		= 0.0473; // Tr{0.0473}
	s. fatigue_memory_half_life		= 0.6; // Ta{0.6}
	
	s. brain_signal_strength		= 1.71; // s{1.71}
	
	s. recurrent_inhibition			= 3.0; // b{3.0}
	s. contralateral_inhibition		= 0.3; // a{-0.3} 
	s. ipsilateral_inhibition		= 0.8; // B{-0.8} 
	s. sensory_inhibition			= 2.0; // lambda {-2.0} 
	
	s. foot_feedback_constant		= 0.03; // k2{0.08}
	s. hip_feedback_constant		= 3.0; // k1{3.0}	
	
	return s;
};

struct CPG_Model * CPG_ModelAllocate(int no_limbs, int no_segments, int no_groups);

struct CPG_Model * CPG_ModelCreate(CPG_CreationCommand const* cmd)
{
	int no_groups = 1;
	int no_segments = 0;
	
	for(auto i = 0u; i < cmd->no_groups; ++i)
	{
		if(cmd->limbs_in_group[i] != 0) 
			++no_segments;
		if(cmd->limbs_in_group[i] == 0) 
			++no_groups;
	}
	
	CPG_Model * model = CPG_ModelAllocate(cmd->no_limbs, no_segments, no_groups);
	
	no_groups = 0;
	no_segments = 0;
	int cur_segment = 0;
	int no_limbs = 0;
	
	for(auto i = 0u; i < cmd->no_groups; ++i)
	{
		if(cmd->limbs_in_group[i] != 0)
		{ 
			model->segments[no_segments].first_oscilator = no_limbs;
			model->segments[no_segments].last_oscilator = (no_limbs += cmd->limbs_in_group[i]);
			no_segments += 1;
		}
		if(cmd->limbs_in_group[i] == 0) 
		{
			model->groups[no_groups].first_segment = cur_segment;
			model->groups[no_groups].last_segment = no_segments;
			cur_segment = no_segments;
			no_groups += 1;
		}
	}

	model->groups[no_groups].first_segment = cur_segment;
	model->groups[no_groups].last_segment = no_segments;
	
	return model;
};

struct CPG_Model * CPG_ModelAllocate(int no_limbs, int no_segments, int no_groups)
{
	size_t no_bytes =
		  sizeof(struct CPG_Model)
		+ sizeof(struct CPG_Oscilator)*no_segments
		+ sizeof(struct CPG_Oscilator)*no_limbs
		+ sizeof(struct CPG_Segment)  *no_segments
		+ sizeof(struct CPG_Group)    *no_groups
		+ sizeof(struct CPG_Oscilator)*no_limbs
		
		+ (sizeof(char)) * no_limbs
		+ (sizeof(float)*5) * no_limbs	
		+ (sizeof(float)) * no_groups	
		+ 16;
	
	CPG_Model * model = (CPG_Model*)calloc(1, no_bytes);
	model->accumulator = 0;
	model->settings = default_CPG_constants();

	*(int16_t*)(&model->noOscilators) = no_limbs;
	*(int16_t*)(&model->noSegments) = no_limbs;
	*(int16_t*)(&model->noGroups) = no_groups;
	*(int16_t*)(&model->noNeurons) = no_limbs*2;
	
	*(CPG_Oscilator**)(&model->root_oscilators) = (CPG_Oscilator*)(model+1);
	*(CPG_Oscilator**)(&model->oscilators) = (CPG_Oscilator*)(model->root_oscilators+no_segments);
	*(CPG_Segment**)(&model->segments) = (CPG_Segment*)(model->oscilators+no_limbs);
	*(CPG_Group**)(&model->groups) = (CPG_Group*)(model->segments+no_segments);
	
	intptr_t aligned = (intptr_t)(model->groups+no_groups);
	aligned = (aligned+15) & ~16llu;
	
	*(void**)model->neurons = (void*)aligned; 
	*(float**)model->contact_force = ((float*)model->neurons) + no_limbs*3; 
	*(float**)model->joint_cos = (float*)(model->contact_force + no_limbs); 
	*(float**)model->group_target_hz = (float*)(model->joint_cos + no_groups); 
	*(char**)model->state = (char*)(model->group_target_hz + no_limbs); 
	
	return model;
}
