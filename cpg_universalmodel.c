#include "cpg_universalmodel.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

struct CPG_constants default_CPG_constants()
{
	struct CPG_constants defaults = {
	//	.base_frequency = 2.0,              // ~2 Hz stepping
		.frequency_adaptation_rate = 0.5,   // moderate adaptation speed
		
		//.contralateral_coupling = 1.5,      // strong coupling to opposite limb
		//.ipsilateral_coupling = 0.7,        // weaker same-side coupling
		//.desired_phase_offset = PI,         // 180° for opposite limbs
		
		.ground_contact_gain = 0.3,
		.hip_angle_gain = 0.5,
		.load_feedback_gain = 0.2,
		
		.phase_coupling_strength = 1.0,
		.frequency_bounds = {0.5, 5.0}      // don't get too slow or fast
	};
	
	return defaults;
}

static struct CPG_Model * CPG_ModelAllocate(int no_limbs, int no_segments, int no_groups, int total_group_items, uint8_t ** group_indices_begin);

struct CPG_Model * CPG_ModelCreate(struct CPG_ConstructionCommand const* cmd)
{
	int no_groups = 1;
	
	for(uint32_t i = 0u; i < cmd->ipsilateral_inhibition_groups_size; ++i)
	{
		if(cmd->ipsilateral_inhibition_groups[i] == -1) 
			++no_groups;
	}
	
	uint8_t * group_indices = 0;
	struct CPG_Model * model = CPG_ModelAllocate(
		cmd->limb_size,
		cmd->limbs[cmd->limb_size-1].segment, 
		no_groups,
		cmd->ipsilateral_inhibition_groups_size - (no_groups-1),
		&group_indices);
		
// set up segments	
	for(uint32_t i = 0u; i < cmd->limb_size; ++i)
	{
		model->oscilators[i].limb_id = cmd->limbs[i].limb;
		model->oscilators[i].root_id = cmd->limbs[i].common_root;
	
		if(i > 0 && cmd->limbs[i].segment != cmd->limbs[i-1].segment)
		{
			assert(cmd->limbs[i].segment > cmd->limbs[i-1].segment);
			
			model->segments[cmd->limbs[i-1].segment].last_oscilator = i;
			model->segments[cmd->limbs[i].segment].first_oscilator = i;		
		}
	}
	
	model->segments[model->noSegments-1].last_oscilator = model->noOscilators;
		
// set up groups	
	uint8_t * begin = group_indices;
	int group_idx = 0;
	for(uint32_t i = 0u; i < cmd->ipsilateral_inhibition_groups_size; ++i)
	{
		if(cmd->ipsilateral_inhibition_groups[i] != -1)
			*(group_indices++) = cmd->ipsilateral_inhibition_groups[i];
		else
		{
			model->groups[group_idx].segment = begin;
			model->groups[group_idx].length = group_indices - begin;
			begin = group_indices;
			group_idx += 1;
		}
	}
	
	model->groups[group_idx].segment = begin;
	model->groups[group_idx].length = group_indices - begin;
	
	return model;
}

struct CPG_Model * CPG_ModelCopy(struct CPG_Model const* cmd)
{
	assert(cmd != 0L);
	
	uint8_t * group_indices = 0;
	struct CPG_Model * model = CPG_ModelAllocate(
		cmd->noOscilators,
		cmd->noSegments, 
		cmd->noGroups,
		cmd->noGroupIndices,
		&group_indices);
		
// set up segments	
	for(uint32_t i = 0u; i < cmd->noOscilators; ++i)
	{
		model->oscilators[i].limb_id = cmd->oscilators[i].limb_id;
		model->oscilators[i].root_id = cmd->oscilators[i].root_id;
	}
	
	for(uint32_t i = 0u; i < cmd->noSegments; ++i)
	{
		model->segments[i].first_oscilator = cmd->segments[i].first_oscilator;
		model->segments[i].last_oscilator = cmd->segments[i].last_oscilator;
	}
	
// set up groups	
	uint8_t * begin = cmd->groups[0].segment;
	memcpy(group_indices, begin, cmd->noGroupIndices);
	for(uint32_t i = 0u; i < cmd->noGroups; ++i)
	{
		model->groups[i].segment = group_indices + (cmd->groups[i].segment - begin);
		model->groups[i].length = cmd->groups[i].length;
	}
	
	return model;

}

static struct CPG_Model * CPG_ModelAllocate(int no_limbs, int no_segments, int no_groups, int total_group_items, uint8_t ** group_indices_begin)
{
	size_t no_bytes =
		  sizeof(struct CPG_Model)
		+ sizeof(struct CPG_Oscilator)*no_limbs
		+ sizeof(struct CPG_Segment)  *no_segments
		+ sizeof(struct CPG_Group)    *no_groups
		
		+ (sizeof(float)*2) * no_limbs	
		+ (sizeof(int8_t)) * no_limbs	
		+ sizeof(**group_indices_begin) * total_group_items
		+ 16;
		
	
	struct CPG_Model * model = (struct CPG_Model*)calloc(1, no_bytes);
	model->accumulator = 0;
	model->settings = default_CPG_constants();

	*(int16_t*)(&model->noOscilators) = no_limbs;
	*(int16_t*)(&model->noSegments) = no_limbs;
	*(int16_t*)(&model->noGroups) = no_groups;
	*(int16_t*)(&model->noGroupIndices) = total_group_items;
	
	*(struct CPG_Oscilator**)(&model->oscilators) = (struct CPG_Oscilator*)(model+1);
	*(struct CPG_Segment**)(&model->segments) = (struct CPG_Segment*)(model->oscilators+no_limbs);
	*(struct CPG_Group**)(&model->groups) = (struct CPG_Group*)(model->segments+no_segments);
	
	intptr_t aligned = (intptr_t)(model->groups+no_groups);
	aligned = (aligned+15) & ~16llu;
	
	*(float**)(&model->contact_force) = (float*)(aligned); 
	*(float**)(&model->joint_cos) = (model->contact_force + no_limbs); 
	*group_indices_begin = (uint8_t*)(model->joint_cos+no_limbs);
	
	*(int8_t**)(&model->cpg_state) = (int8_t*)(group_indices_begin + total_group_items);
	
	return model;
}
