# CPG Universal Model

A flexible, modular Central Pattern Generator (CPG) implementation for multi-limbed locomotion based on phase oscillator principles.

## Overview

The Universal Model is a general-purpose CPG system designed to coordinate rhythmic movements across any number of limbs with arbitrary topology. Unlike the prototype implementation (see `prototpye/`), this uses **phase oscillators** with coupling rather than explicit neuron simulation, providing a more stable and configurable foundation for gait generation.

### Key Features

- **Arbitrary limb topology** - Support for any creature architecture (bipeds, quadrupeds, hexapods, etc.)
- **Phase-based oscillators** - Smooth, continuous phase tracking [0, 2π) instead of discrete neuron states
- **Hierarchical coupling** - Three levels of coordination:
  - **Segments**: Contralateral coupling (opposite limbs, e.g., left-right legs)
  - **Groups**: Ipsilateral coupling (same-side limbs, e.g., front-back coordination)
  - **Limbs**: Individual oscillator tracking
- **Adaptive frequency** - Oscillators adjust their speed based on sensory feedback
- **Sensory integration** - Ground contact, hip angle, and load feedback
- **State machine** - Per-limb state tracking (DISABLED, STANCE, SWING)

## Architecture

### Data Structures

```
CPG_Model
├── CPG_Oscilator[] - Individual limb oscillators
│   ├── phase [0, 2π)
│   ├── frequency (rad/s)
│   ├── target_frequency (adaptive)
│   └── phase_velocity (current rate of change)
│
├── CPG_Segment[] - Contralateral pairs
│   ├── target_frequency (base rhythm)
│   ├── contralateral_coupling (K_contra ~ 1.0-2.0)
│   └── desired_phase_offset (typically π for opposites)
│
└── CPG_Group[] - Ipsilateral chains
    ├── segment[] (array of segment indices)
    └── ipsilateral_coupling (K_ipsi ~ 0.5-1.0)
```

### Update Cycle

The model updates in three phases per frame:

1. **Frequency Adaptation** - Each oscillator computes target frequency from:
   - Segment base frequency
   - Ground contact feedback (speeds up/slows down based on timing)
   - Hip angle feedback (scotch yoke position)
   - Load feedback (weight bearing)

2. **Coupling Computation** - Phase coupling forces are calculated:
   - **Contralateral**: Couples opposite limbs within segments (e.g., left-right)
   - **Ipsilateral**: Couples same-side limbs across segments (e.g., front-back)
   - Uses Kuramoto-style coupling: `K * sin(desired_offset - actual_offset)`

3. **Phase Integration** - Oscillators advance:
   - Frequency smoothly adapts toward target
   - Phase velocity = 2π * frequency + coupling forces
   - Phase wraps to [0, 2π)
   - State updates: phase < π → STANCE, phase ≥ π → SWING

## Usage

### Creating a Model

```c
// Define limbs (example: quadruped)
// segments are 0=shoulders and 1=hips
struct CPG_Limb limbs[] = {
    {.limb = 0, .common_root = 0, .segment = 0},  // Front-left
    {.limb = 1, .common_root = 0, .segment = 0},  // Front-right
    {.limb = 2, .common_root = 1, .segment = 1},  // Back-left
    {.limb = 3, .common_root = 1, .segment = 1},  // Back-right
};

// Define ipsilateral groups (segment-segment coordination)
// Negative values (-1) act as separators between groups
int ipsi_groups[] = {
    0, 1, -1,  // couple front and back: segments 0 and 1
};

struct CPG_ConstructionCommand cmd = {
    .limbs = limbs,
    .limb_size = 4,
    .ipsilateral_inhibition_groups = ipsi_groups,
    .ipsilateral_inhibition_groups_size = 3,
};

struct CPG_Model *model = CPG_ModelCreate(&cmd);
```

### Updating the Model

```c
// Set segment parameters
model->segments[0].enabled = true;
model->segments[0].target_frequency = 2.0f;  // 2 Hz
model->segments[0].contralateral_coupling = 1.5f;
model->segments[0].desired_phase_offset = M_PI;  // 180° out of phase

// Provide sensory feedback (per frame)
model->contact_force[0] = 0.8f;  // Leg 0 has 80% weight
model->joint_cos[0] = -0.5f;     // Hip position (scotch yoke interpretation)

// Update
float dt = 1.0f / 60.0f;  // 60 Hz simulation
CPG_ModelUpdate(model, dt);

// Read outputs
for (int i = 0; i < model->noOscilators; i++) {
    enum CPG_State state = model->cpg_state[i];
    float phase = model->oscilators[i].phase;
    // Use state and phase for IK target generation
}
```

### Configuration Parameters

```c
struct CPG_constants settings = {
    .frequency_adaptation_rate = 0.5,   // How fast oscillators adapt (cycles)

    // Sensory gains
    .ground_contact_gain = 0.3,         // Ground contact influence
    .hip_angle_gain = 0.5,              // Joint angle influence
    .load_feedback_gain = 0.2,          // Weight bearing influence

    // Stability
    .phase_coupling_strength = 1.0,     // Overall coupling multiplier
    .frequency_bounds = {0.5, 5.0},     // [min, max] as multipliers
};

model->settings = settings;
```

## Key Concepts

### Phase Oscillators

Unlike the prototype's discrete neuron approach, this model uses continuous phase oscillators. Each limb has a phase φ ∈ [0, 2π):
- **STANCE phase**: [0, π) - leg on ground, passive
- **SWING phase**: [π, 2π) - leg in air, actively moving

### Kuramoto Coupling

Oscillators synchronize using Kuramoto-style coupling:
```
dφ/dt = 2πf + Σ K_ij * sin(φ_desired - φ_actual)
```

This creates stable, emergent gait patterns without explicit state machines.

### Sensory Feedback

The model adapts in real-time to sensory input:
- **Ground contact**: If a leg touches ground during swing, slow down the cycle
- **Hip angle**: Extended hip suggests end of stance, speed up transition
- **Load**: More weight on a leg affects its rhythm

### Adaptive Frequency

Each oscillator's frequency smoothly adapts toward a target influenced by feedback:
```
f(t+dt) = f(t) + α * (f_target - f(t)) * dt
```

This allows the model to naturally adjust to terrain and load conditions.

## Comparison to Prototype

| Feature | Prototype | Universal Model |
|---------|-----------|-----------------|
| **Architecture** | Discrete neurons | Phase oscillators |
| **Stability** | Unstable, sensitive | Smooth, robust |
| **Flexibility** | Quadruped only | Arbitrary topology |
| **State** | Binary on/off | Continuous phase |
| **Coupling** | Inhibition terms | Kuramoto coupling |
| **Feedback** | Hard to tune | Adaptive frequency |
| **Implementation** | Paper-specific | General-purpose |

## Files

- `cpg_universalmodel.h` - Public API and data structures
- `cpg_universalmodel.c` - Implementation (330 lines)

## Design Notes

### Memory Layout

The model uses a single `calloc()` allocation with careful pointer arithmetic to ensure cache-friendly memory layout:
```
[CPG_Model][Oscillators...][Segments...][Groups...][contact_force...][joint_cos...][group_indices...][cpg_state...]
```

All arrays are const pointers to prevent accidental reallocation.

### Construction System

The `CPG_ConstructionCommand` provides a flexible way to specify arbitrary limb topologies:
- `limbs[]` defines individual oscillators and their segment membership
- `ipsilateral_inhibition_groups[]` defines inter-segment coupling chains
- Use `-1` as separator between groups

### Thread Safety

The model is **not thread-safe** by design. Each `CPG_Model` instance should be updated by a single thread. Use `CPG_ModelCopy()` to create independent instances for parallel evaluation.

## Future Work

- [ ] Add phase offset configuration for ipsilateral coupling (currently hardcoded)
- [ ] Implement inter-group coupling for multi-body creatures
- [ ] Add phase reset on ground contact for more reactive stepping
- [ ] Optimize coupling computation with spatial indexing for large limb counts
- [ ] Add visualization/debugging utilities

## References

This implementation draws inspiration from:
- Ijspeert, A.J. "Central pattern generators for locomotion control in animals and robots" (2008)
- Kuramoto, Y. "Self-entrainment of a population of coupled non-linear oscillators" (1975)
- Owaki et al. (2013) - see `prototpye/` for specific paper implementation attempt

## License

See repository root for license information.
