#ifndef CPG_WALKINGMODEL_H
#define CPG_WALKINGMODEL_H

/*

Research paper:
https://static-content.springer.com/esm/art%3A10.1038%2Fsrep08169/MediaObjects/41598_2015_BFsrep08169_MOESM3_ESM.pdf
  
	Parameter	Value		
	Tr			0.0473				state_memory_constant
	Ta			0.6					fatigue_memory_constant
	s			1.71				brainstem_propagation_time
	b			3					recurrent_inhibition
	α			-0.3				contralateral_inhibition
	β			-0.8				ipsilateral_inhibition
	γ			-2					extensor_inhibition
	θ0			-0.262	rad			hip_default_rotation
	k1			3					hip_feedback_constant
	k2			0.08				foot_feedback_constant
	
	θ	di·sw	1.312	rad	Hip		target joint angle during swing phase
	Kτ	pi·sw	7.6		Nm/rad		Hip proportional gain during swing phase
	Kτ	vi·sw	1		Nms/rad		Hip derivative gain during swing phase
	
	θ	di·st	-0.438	rad	Hip		target joint angle during stance phase
	Kτ	pi·st	8.13	Nm/rad		Hip proportional gain during stance phase
	Kτ	vi·st	1		Nms/rad		Hip derivative gain during stance phase
	
	l	di·sw	57		mm			Elbow target length during swing phase
	KF	pi·sw	1.86	kN/m		Elbow proportional gain during swing phase
	KF	vi·sw	40		Ns/m		Elbow derivative gain during swing phase
	
	l	di·st	130		mm			Elbow target length during stance phase
	KF	pi·st	1.97	kN/m		Elbow proportional gain during stance phase
	KF	vi·st	40		Ns/m		Elbow derivative gain during stance phase
	
	constants that change motion rate:		
	s			brainstem_propagation_time
	Tr			state_memory_constant
	θ	d·st	target joint angle during stance phase
	Kτ	pi·st	Hip proportional gain during stance phase
	KF	pi·st	Elbow proportional gain during stance phase
	l	d·sw	Elbow target joint angle during swing phase
	KF	pi·sw	Elbow proportional gain during swing phase
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CPG_neuron		CPG_neuron;
typedef struct CPG_constants	CPG_constants;
typedef struct PD_Model			PD_Model;
typedef struct CPG_Segment		CPG_Segment;
typedef struct CPG_Model		CPG_Model;

// returns a malloced pointer that must be free-d
struct CPG_Model * CPG_ModelCreate(int noSegments);
void CPG_ModelUpdate(CPG_Model * model, float dt);

// catmul rom spline src must contain 4 elements.
void PD_ModelSpline(PD_Model * dst, PD_Model const* src, float t);
void PD_ModelLerp(PD_Model * dst, PD_Model const* src0, PD_Model const* src1, float t);

// helper functions to compute weight on each foot 
// returns total mass
float CPG_GetCenterOfGravity(float *__restrict dst_xyz, float const*__restrict xyz, float const*__restrict mass, int size);
/// xyz is interpreted as an array of bytes with points at offset 
/// float[3] * point = xyz + stride*point_no
float CPG_GetCenterOfGravity_Stride(float *__restrict dst_xyz, char const*__restrict xyz, int stride, float const*__restrict mass, int size);

float CPG_ComputeHalfLife(float percent_remaining, float time);

/// the moment of inertia matrix is symetrical
/// meaning row major/column major is irrelevant. 
/// each point is considered a vertex; e.g. for a solid sphere premultiply mass array by 2/3
float CPG_ComputeMomentOfInertiaMat3(float *__restrict mat3x3_out, float const*__restrict center_of_graivty_xyz, float const*__restrict xyz, float const*__restrict mass, int size);

/// xyz is interpreted as an array of bytes with points at offset 
/// float[3] * point = xyz + stride*point_no
float CPG_ComputeMomentOfInertiaMat3_Stride(float *__restrict mat3x3_out, float const*__restrict center_of_graivty_xyz, char const*__restrict xyz, int stride, float const*__restrict mass, int size);


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

struct PD_Driver
{
	float target;
	float proportional;
	float derivative;
};

enum PD_UNIT
{
	PD_Hip,
	PD_Knee,
	PD_JointTotal,
	
	PD_Swing = 0,
	PD_Stance,
	PD_StateTotal,
};

struct PD_Model
{
	struct PD_Driver unit[PD_StateTotal][PD_JointTotal];
};


// used in internal model...
struct CPG_Joint
{	
	float pos;
	float velocity;
	float p_pos;
};

struct CPG_Leg { struct CPG_Joint hip, knee; };
struct CPG_Segment { struct CPG_Leg leg[2]; };

struct CPG_Model
{		
	CPG_constants settings;
	PD_Model	  drivers;	
	
	const int	  noSegments;	
	const int	  noNeurons;
	const int	  noLegs;
	float		  biotick; // neurons don't use dt so only tick when this passes a threshold
	
	CPG_neuron	   *const neurons;	
	CPG_Segment	   *const segments; // hip knee, hip knee, one segment at a time. 
	float		   *const contact_force;
	
	char * legState; // 0 = swing, 1 = stance;
};



#ifdef __cplusplus
}
#endif



#endif // CPG_WALKINGMODEL_H
