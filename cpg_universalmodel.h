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
	int group;
};

struct CPG_CreationCommand
{
	int * limb_id;
	int * limbs_in_group; // 0 for sentinal
	int * root_of_group; // gets an up/down oscilator
	int no_limbs;
	int no_groups;
};

struct CPG_Model * CPG_ModelCreate(struct CPG_Limb * limbs, size_t size);

struct CPG_constants
{
	float state_memory_half_life;		// Tr{0.0473};
	float fatigue_memory_half_life;		// Ta{0.6}
	
	float brain_signal_strength;		// s{1.71};
	
	float recurrent_inhibition;			// b{3.0};
	float contralateral_inhibition;		// a{-0.3}; 
	float ipsilateral_inhibition;		// B{-0.8}; 
	float sensory_inhibition;			// lambda {-2.0}; 
	
	float foot_feedback_constant;		// k2{0.08};
	float hip_feedback_constant;		// k1{3.0};	
};

struct CPG_constants default_CPG_constants();

struct CPG_Oscilator;
struct CPG_Segment;
struct CPG_Group;

struct CPG_Model
{
	double accumulator;
	CPG_constants settings;
	
	const int16_t noOscilators;
	const int16_t noSegments;
	const int16_t noGroups;
	const int16_t noNeurons;
	
	struct CPG_Oscilator *const root_oscilators;
	struct CPG_Oscilator *const oscilators;
	struct CPG_Segment   *const segments;
	struct CPG_Group     *const groups;
	
// output
	char *const state; // 0 = passive monitor input, 1 = control by CPG,
// feedback 
	float *const contact_force;
// per limb	
	float *const joint_cos;
// per group target hz
	float *const group_target_hz;
	
// neurons
// guarunteed 16 byte aligned.
// each oscilator has 2 neurons, so it maps kinda-directly
	void *const neurons;
};


struct CPG_Oscilator
{
// if true then use to set IK target, if false then we check against this to recalibrate speed 
// (if stance swings faster than we expect adjust swing phase etc)
// 0 is front, pi is back
	int8_t  cpg_state;
	int8_t  root_id;
	int16_t limb_id;
	float   speed; // rad/s
// for a leg we conceptualize this as a scotch yoke
	float   position; // [0, 2pi)
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
	int16_t first_segment;
	int16_t last_segment;
};

#ifdef __cplusplus
}
#endif

#endif

