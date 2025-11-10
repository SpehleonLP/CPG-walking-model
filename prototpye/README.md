# CPG Walking Model Prototype

A C implementation of a Central Pattern Generator (CPG) based quadrupedal locomotion controller, based on the neuro-musculo-skeletal model described in [Owaki et al. (2015)](https://www.nature.com/articles/srep08169).

## Overview

This prototype implements:
- CPG neural network with state/fatigue dynamics
- PD controllers for hip and knee joints
- Support for multiple gait patterns (walk, trot, canter, gallop)
- Configurable inhibition between contralateral, ipsilateral, and extensor-flexor neurons

## Status: Non-Functional

**This implementation does not produce stable locomotion.** Multiple attempts to reproduce the paper's results were unsuccessful, despite significant parameter tuning and debug efforts.

## Technical Issues Identified

### 1. Sign Convention Ambiguity
The paper specifies negative values for inhibition constants (α=-0.3, β=-0.8, γ=-2), but the equations subtract inhibition terms. This creates ambiguity: should inhibition be added (treating negatives as intended) or should the constants be positive? The current implementation uses positive values (cpg_walkingmodel.c:52-55).

### 2. Underspecified Timestep
The model exhibits extreme sensitivity to simulation timestep (dt), but the paper does not specify the timestep used in the original experiments. Various values from 0.001 to 0.1 seconds were tested without success.

### 3. PD Controller Issues
A freelance robotics engineer declined to implement this in the original simulation environment, citing problems with the PID equations. The current implementation replaces the paper's PD controller with a custom acceleration-based driver (see `struct Accel_Driver` in cpg_walkingmodel.h:92).

### 4. Parameter Sensitivity
The model appears highly sensitive to initial conditions and parameter values, making it difficult to achieve stable periodic gaits without extensive tuning or potentially missing information from the paper.

## Implementation Notes

- CPG neurons compute in serial order (not parallel) - the algorithm depends on neuron `i` being computed before `i+1`
- Half-life calculations are used for memory constants instead of direct time constants
- Gait timing can be configured via debug sequences (see `CPG_SetDebugSequence`)
- Joint limits are enforced to prevent unrealistic poses

## Files

- `cpg_walkingmodel.h` - Core data structures and API
- `cpg_walkingmodel.c` - CPG update logic and PD controllers
- `debug_keyed_timings.h` - Gait sequence debugging utilities

## References

Owaki, D., Kano, T., Nagasawa, K. et al. Simple robot suggests physical interlimb communication is essential for quadruped walking. *J. R. Soc. Interface* 10: 20120669 (2013).

[Supplementary Material (PDF)](https://static-content.springer.com/esm/art%3A10.1038%2Fsrep08169/MediaObjects/41598_2015_BFsrep08169_MOESM3_ESM.pdf)

## Contact

If you have successfully implemented this paper or identified the issues preventing stable locomotion, contributions and insights are welcome.
