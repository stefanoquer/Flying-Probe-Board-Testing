# Flying-Probe-Board-Generator
Flying Probe Board Generator
From the paper

Andrea Calabrese, Stefano Quer, Giovanni Squillero
"Flying Probe Testing:  A Trajectory Planner and a Benchmark Suite"

this tool produces synthetic board descriptions with different layouts and testing sequences and represents them graphically.

The generators can be used togheter an optimizer for test sequences composed of two separate parts. The first one works at a high level, detecting tests that could be performed together and reordering them to obtain a more efficient probing sequence. The second one acts at a low level, modeling the movements of the flying probes as a multiple and collaborative planning problem, avoiding non-fly and no-touch areas on the board.
