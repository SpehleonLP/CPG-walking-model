#ifndef CPG_UNIVERSAL_MODEL_H
#define CPG_UNIVERSAL_MODEL_H
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


enum CPG_State {
  	CPG_DISABLED,   // Limb is passively monitoring it's input cosine.
  	CPG_STANCE,     // Phase [0, π) - passive, ground contact
  	CPG_SWING,      // Phase [π, 2π) - active, CPG drives
};

struct CPG_Limb
{
	int limb;
	int common_root;
	int segment;
};
	
struct	CPG_ConstructionCommand 
{
	struct CPG_Limb * limbs;
	size_t limb_size;
	int * ipsilateral_inhibition_groups;
	size_t ipsilateral_inhibition_groups_size; 
};
	
// does one malloc so can just destroy with free
struct CPG_Model * CPG_ModelCreate(const struct CPG_ConstructionCommand *cmd);
// doesn't copy internal state only structure of model. 
struct CPG_Model * CPG_ModelCopy(const struct CPG_Model *cmd);
void CPG_ModelUpdate(struct CPG_Model *model, float dt) ;


struct CPG_constants
{
    // Core oscillator parameters
    float frequency_adaptation_rate;   // α - how fast it learns from feedback (in number of cycles)
        
    // Sensory feedback
    float ground_contact_gain;         // how much ground contact speeds/slows oscillator (note: we may just kick our legs uselessly in the air!)
    // (probably not going to be actual angle, b/c that means getting atan2 for each limb each frame)
    // more likely: its being interpreted as a scotch yoke and we're getting 0-1 depending on where we are in the yoke.
    float hip_angle_gain;              // how much hip extension affects frequency 
    float load_feedback_gain;          // how much leg loading affects phase
    
    // Stability/smoothing
    float phase_coupling_strength;     // overall coupling multiplier
    float frequency_bounds[2];         // [min_freq, max_freq] - prevent runaway (multipliers for target of group)
};

struct CPG_constants default_CPG_constants();

struct CPG_Oscilator;
struct CPG_Segment;
struct CPG_Group;

struct CPG_Model
{
	double accumulator;
	struct CPG_constants settings;
	
	const uint16_t noOscilators;
	const uint16_t noSegments;
	const uint16_t noGroups;
	const uint16_t noGroupIndices;
	
	struct CPG_Oscilator *const oscilators;
	struct CPG_Segment   *const segments;
	struct CPG_Group     *const groups;
	
// feedback, per limb 
// if true then use to set IK target, if false then we check against this to recalibrate speed 
// (if stance swings faster than we expect adjust swing phase etc)
// 0 is front, pi is back
	int8_t *const cpg_state;
	// by percentage of weight. 
	// basically how close the center of mass is to us by barrycentric coordinates.
	float *const contact_force;
	float *const joint_cos;
};


struct CPG_Oscilator
{
	int16_t root_id;
	int16_t limb_id;
	
    float   phase;           // renamed from position - [0, 2π)
    float   frequency;       // renamed from speed - rad/s
    
    float   phase_last_frame;
    float   cycles_this_frame;
    
    // Optional but helpful:
    float   target_frequency;  // what frequency feedback is pushing toward
    float   phase_velocity;    // for smoothing phase updates (optional)
};

// contralateral inhibition
struct CPG_Segment
{
	uint8_t first_oscilator;
	uint8_t last_oscilator;

	bool enabled;
		
	// K_contra - couples opposite limbs (typically ~1.0-2.0)
	float target_frequency;
	float desired_phase_offset;
    float contralateral_coupling; 
};

// ipsilateral inhibition
struct CPG_Group
{
	uint8_t * segment;
	int32_t length;
	
	bool  enabled;	
    float ipsilateral_coupling;  // K_ipsi - couples same-side limbs (typically ~0.5-1.0)
};

#ifdef __cplusplus
}
#endif

#endif

