#include "cpg_universalmodel.h"
#include <stdlib.h>
#include <stddef.h>

struct CPG_constants default_CPG_constants()
{
	struct CPG_constants defaults = {
	//	.base_frequency = 2.0,              // ~2 Hz stepping
		.frequency_adaptation_rate = 0.5,   // moderate adaptation speed
		
		.contralateral_coupling = 1.5,      // strong coupling to opposite limb
		.ipsilateral_coupling = 0.7,        // weaker same-side coupling
		//.desired_phase_offset = PI,         // 180° for opposite limbs
		
		.ground_contact_gain = 0.3,
		.hip_angle_gain = 0.5,
		.load_feedback_gain = 0.2,
		
		.phase_coupling_strength = 1.0,
		.frequency_bounds = {0.5, 5.0}      // don't get too slow or fast
	};
	
	return defaults;
};

struct CPG_Model * CPG_ModelAllocate(int no_limbs, int no_segments, int no_groups);

struct CPG_Model * CPG_ModelCreate(struct CPG_Limb * limbs, size_t size)
{
	int no_groups = 1;
	int no_segments = 0;
	
	for(uint32_t i = 0u; i < size; ++i)
	{
		if(cmd->limbs_in_group[i] != 0) 
			++no_segments;
		if(cmd->limbs_in_group[i] == 0) 
			++no_groups;
	}
	
	struct CPG_Model * model = CPG_ModelAllocate(cmd->no_limbs, no_segments, no_groups);
	
	no_groups = 0;
	no_segments = 0;
	int cur_segment = 0;
	int no_limbs = 0;
	uint32_t segment_start = 0;
	
	for(uint32_t i = 0u; i < size; ++i)
	{
		if(limbs[i].segment != limbs[segment_start].segment)
		{ 
			model->segments[no_segments].first_oscilator = no_limbs;
			model->segments[no_segments].last_oscilator = (no_limbs += (i - segment_start));
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
		+ sizeof(struct CPG_Oscilator)*no_limbs
		+ sizeof(struct CPG_Segment)  *no_segments
		+ sizeof(struct CPG_Group)    *no_groups
		
		+ (sizeof(float)*2) * no_limbs	
		+ 16;
	
	struct CPG_Model * model = (struct CPG_Model*)calloc(1, no_bytes);
	model->accumulator = 0;
	model->settings = default_CPG_constants();

	*(int16_t*)(&model->noOscilators) = no_limbs;
	*(int16_t*)(&model->noSegments) = no_limbs;
	*(int16_t*)(&model->noGroups) = no_groups;
	
	*(struct CPG_Oscilator**)(&model->oscilators) = (struct CPG_Oscilator*)(model+1);
	*(struct CPG_Segment**)(&model->segments) = (struct CPG_Segment*)(model->oscilators+no_limbs);
	*(struct CPG_Group**)(&model->groups) = (struct CPG_Group*)(model->segments+no_segments);
	
	intptr_t aligned = (intptr_t)(model->groups+no_groups);
	aligned = (aligned+15) & ~16llu;
	
	*(float**)model->contact_force = (float*)(aligned); 
	*(float**)model->joint_cos = (float*)(model->contact_force + no_limbs); 
	
	return model;
}
