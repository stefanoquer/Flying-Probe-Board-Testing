# Flying-Probe-Board-Generator

Flying-Probe-Board-Generator

This tool generates synthetic descriptions of circuit boards, featuring various layouts and corresponding test sequences, which are also represented graphically.

The output of this generator can be utilized with a specialized optimizer designed for test sequences. The optimizer operates in two distinct stages:

1. A high-level optimization stage detects tests that can be performed concurrently and reorders the sequence for improved probing efficiency.

2. A low-level optimization stage models the movement of the flying probes as a collaborative, multi-probe motion planning problem. This stage ensures that the probes avoid predefined no-fly zones and no-touch areas on the board.

To complement these tools, a verification utility is included to ensure the correctness of the generated testing sequences.

If you incorporate these benchmarks or any associated tools into your research, we kindly request that you cite the following publication:

@article{
  FpBench2025,
  author = {Calabrese, Andrea and Quer, Stefano and Squillero, Giovanni},
  year = {2025},
  title = {Flying-Probe Testing: A Trajectory Planner and a Benchmark Suite},
  journal = {IEEE Transactions on Computer-Aided Design of Integrated Circuits and Systems},
  publisher = {Institute of Electrical and Electronics Engineers (IEEE)},
  ISSN = {1937-4151},
  DOI = {10.1109/tcad.2025.3567012},
  url = {https://ieeexplore.ieee.org/document/10985887},
}
