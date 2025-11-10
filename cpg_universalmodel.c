#include "cpg_universalmodel.h"
#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#ifndef M_PI
#define M_PI 3.14159265358
#endif

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

// Helper to wrap phase to [0, 2π)
static inline float wrap_phase(float phase) {
    const float TWO_PI = 2.0f * M_PI;
    while (phase >= TWO_PI) phase -= TWO_PI;
    while (phase < 0.0f) phase += TWO_PI;
    return phase;
}

// Helper to compute phase difference (shortest path on circle)
static inline float phase_difference(float target, float current) {
    float diff = target - current;
    const float PI = M_PI;
    if (diff > PI) diff -= 2.0f * PI;
    if (diff < -PI) diff += 2.0f * PI;
    return diff;
}

void CPG_ModelUpdate(struct CPG_Model *model, float dt) {
    const float TWO_PI = 2.0f * M_PI;
    const float PI = M_PI;
    
    // === PHASE 1: Compute target frequencies from sensory feedback ===
    for (uint16_t i = 0; i < model->noOscilators; i++) {
        struct CPG_Oscilator *osc = &model->oscilators[i];
        
        // Start with segment's base target frequency
        int16_t seg_id = osc->root_id;
        if (seg_id >= 0 && seg_id < model->noSegments) {
            osc->target_frequency = model->segments[seg_id].target_frequency;
        }
        
        // Adjust based on sensory feedback
        float feedback_adjustment = 0.0f;
        
        // Ground contact: if we're touching ground, adjust frequency
        // (speeds up or slows down based on whether we're ahead/behind expected)
        if (model->contact_force[i] > 0.1f) {
            feedback_adjustment += model->settings.ground_contact_gain * 
                                   model->contact_force[i];
        }
        
        // Hip angle feedback: joint_cos is like a scotch yoke position
        // If leg is more extended (joint_cos near -1), we might want to speed up swing
        feedback_adjustment += model->settings.hip_angle_gain * 
                               (model->joint_cos[i] + 1.0f) * 0.5f;
        
        // Load feedback: affects frequency based on weight bearing
        feedback_adjustment += model->settings.load_feedback_gain * 
                               model->contact_force[i];
        
        osc->target_frequency += feedback_adjustment;
        
        // Clamp to bounds
        if (seg_id >= 0 && seg_id < model->noSegments) {
            float base_freq = model->segments[seg_id].target_frequency;
            osc->target_frequency = fmaxf(osc->target_frequency, 
                                         base_freq * model->settings.frequency_bounds[0]);
            osc->target_frequency = fminf(osc->target_frequency, 
                                         base_freq * model->settings.frequency_bounds[1]);
        }
    }
    
    // === PHASE 2: Compute coupling forces ===
    float *phase_coupling = calloc(model->noOscilators, sizeof(float));
    
    // Contralateral coupling (segment level - opposite limbs)
    for (uint16_t s = 0; s < model->noSegments; s++) {
        struct CPG_Segment *seg = &model->segments[s];
        if (!seg->enabled) continue;
        
        // Find pairs within this segment and couple them
        for (uint8_t i = seg->first_oscilator; i <= seg->last_oscilator; i++) {
            for (uint8_t j = i + 1; j <= seg->last_oscilator; j++) {
                struct CPG_Oscilator *osc_i = &model->oscilators[i];
                struct CPG_Oscilator *osc_j = &model->oscilators[j];
                
                // Desired phase relationship (typically π for contralateral)
                float desired_diff = seg->desired_phase_offset;
                float actual_diff = phase_difference(osc_j->phase, osc_i->phase);
                float coupling_force = seg->contralateral_coupling * 
                                      sinf(desired_diff - actual_diff);
                
                phase_coupling[i] += coupling_force * model->settings.phase_coupling_strength;
                phase_coupling[j] -= coupling_force * model->settings.phase_coupling_strength;
            }
        }
    }
    
    // Ipsilateral coupling (group level - same-side limbs)
    for (uint16_t g = 0; g < model->noGroups; g++) {
        struct CPG_Group *group = &model->groups[g];
        if (!group->enabled) continue;
        
        // Couple segments within this group
        for (int32_t si = 0; si < group->length; si++) {
            for (int32_t sj = si + 1; sj < group->length; sj++) {
                uint8_t seg_i = group->segment[si];
                uint8_t seg_j = group->segment[sj];
                
                if (seg_i >= model->noSegments || seg_j >= model->noSegments) continue;
                
                struct CPG_Segment *s_i = &model->segments[seg_i];
                struct CPG_Segment *s_j = &model->segments[seg_j];
                
                // Couple representative oscillators from each segment
                if (s_i->first_oscilator < model->noOscilators && 
                    s_j->first_oscilator < model->noOscilators) {
                    
                    uint8_t osc_i_idx = s_i->first_oscilator;
                    uint8_t osc_j_idx = s_j->first_oscilator;
                    
                    struct CPG_Oscilator *osc_i = &model->oscilators[osc_i_idx];
                    struct CPG_Oscilator *osc_j = &model->oscilators[osc_j_idx];
                    
                    // Ipsilateral limbs typically have phase offset (like 0 or π/2)
                    float actual_diff = phase_difference(osc_j->phase, osc_i->phase);
                    float coupling_force = group->ipsilateral_coupling * sinf(-actual_diff);
                    
                    phase_coupling[osc_i_idx] += coupling_force * model->settings.phase_coupling_strength;
                    phase_coupling[osc_j_idx] -= coupling_force * model->settings.phase_coupling_strength;
                }
            }
        }
    }
    
    // === PHASE 3: Integrate oscillators ===
    for (uint16_t i = 0; i < model->noOscilators; i++) {
        struct CPG_Oscilator *osc = &model->oscilators[i];
        
        // Store last frame phase for cycle detection
        osc->phase_last_frame = osc->phase;
        
        // Adaptive frequency (exponential smoothing toward target)
        float freq_delta = (osc->target_frequency - osc->frequency) * 
                          model->settings.frequency_adaptation_rate * dt;
        osc->frequency += freq_delta;
        
        // Phase velocity includes base frequency + coupling
        osc->phase_velocity = TWO_PI * osc->frequency + phase_coupling[i];
        
        // Integrate phase
        osc->phase += osc->phase_velocity * dt;
        osc->phase = wrap_phase(osc->phase);
        
        // Detect cycle completion
        if (osc->phase < osc->phase_last_frame) {
            osc->cycles_this_frame = 1.0f;
        } else {
            osc->cycles_this_frame = 0.0f;
        }
        
        // Update state based on phase
        if (model->cpg_state[i] != CPG_DISABLED) {
            if (osc->phase < PI) {
                model->cpg_state[i] = CPG_STANCE;
            } else {
                model->cpg_state[i] = CPG_SWING;
            }
        }
    }
    
    free(phase_coupling);
    
    // Update global accumulator
    model->accumulator += dt;
}
