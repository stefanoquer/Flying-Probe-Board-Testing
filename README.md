# Flying-Probe-Board-Generator

Flying-Probe-Board-Generator

This tool generates synthetic descriptions of circuit boards, featuring various layouts and corresponding test sequences, which are also represented graphically.

The output of this generator can be utilized with a specialized optimizer designed for test sequences. The optimizer operates in two distinct stages:

1. A high-level optimization stage detects tests that can be performed concurrently and reorders the sequence for improved probing efficiency.

2. A low-level optimization stage models the movement of the flying probes as a collaborative, multi-probe motion planning problem. This stage ensures that the probes avoid predefined no-fly zones and no-touch areas on the board.

To complement these tools, a verification utility is included to ensure the correctness of the generated testing sequences.

If you incorporate these benchmarks or any associated tools into your research, we kindly request that you cite the following publication:

A. Calabrese, S. Quer and G. Squillero, "Flying-Probe Testing: A Trajectory Planner and a Benchmark Suite," in IEEE Transactions on Computer-Aided Design of Integrated Circuits and Systems, doi: 10.1109/TCAD.2025.3567012, url: https://ieeexplore.ieee.org/document/10985887
