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

struct CPG_Model * CPG_ModelCreate(struct CPG_Limb * limbs, size_t size);

struct CPG_constants
{
    // Core oscillator parameters
    float frequency_adaptation_rate;   // α - how fast it learns from feedback
    
    // Phase coupling (replaces inhibition)
    float contralateral_coupling;      // K_contra - couples opposite limbs (typically ~1.0-2.0)
    float ipsilateral_coupling;        // K_ipsi - couples same-side limbs (typically ~0.5-1.0)
    
    // Sensory feedback
    float ground_contact_gain;         // how much ground contact speeds/slows oscillator
    float hip_angle_gain;              // how much hip extension affects frequency
    float load_feedback_gain;          // how much leg loading affects phase
    
    // Stability/smoothing
    float phase_coupling_strength;     // overall coupling multiplier
    float frequency_bounds[2];         // [min_freq, max_freq] - prevent runaway
};

struct CPG_constants default_CPG_constants();

struct CPG_Oscilator;
struct CPG_Segment;
struct CPG_Group;

struct CPG_Model
{
	double accumulator;
	struct CPG_constants settings;
	
	const int16_t noOscilators;
	const int16_t noSegments;
	const int16_t noGroups;
	const int16_t noNeurons;
	
	struct CPG_Oscilator *const oscilators;
	struct CPG_Segment   *const segments;
	struct CPG_Group     *const groups;
	
// feedback, per limb 
	float *const contact_force;
	float *const joint_cos;
};


struct CPG_Oscilator
{
// if true then use to set IK target, if false then we check against this to recalibrate speed 
// (if stance swings faster than we expect adjust swing phase etc)
// 0 is front, pi is back
	int8_t  cpg_state;
	int8_t  root_id;
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
	int16_t first_oscilator;
	int16_t last_oscilator;
};

// ipsilateral inhibition
struct CPG_Group
{
	int16_t * segment;
	int32_t length;
	
	bool  enabled;
	float group_target_hz;
	float group_phase_offset;
};

#ifdef __cplusplus
}
#endif

#endif

