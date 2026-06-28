# Nbody-sim

C and CUDA simulation exploring performance and numerical accuracy across implementation strategies.

## Overview

A physics engine developed in C and implemented in CUDA to explore the differences between CPU and GPU computing by simulating large numbers of celestial objects in real-time using Particle-to-particle algorithmic capabilities and Barnes-Hut approximation.

### Purpose

- Calculating CPU-GPU crossover threshold.
- Analyzing accuracy differences between floating-point and double precision, and assessing trade-offs of each.
- Calculating system accuracy using the law of conservation of energy.

### Results
> [!NOTE]  
> This project is in progress and lacks results.

## File Outline
> [!NOTE]
> This will be updated as files are added and created, as this is near the beginning of production there are not many files in the filetree.
  ### Main folder
  nbody.c : first iteration of the Nbody-sim, currently works for up to 2 bodies, currently lacks visualization.
## Implementation 
1. Brute-force C baseline O(N<sup>2</sup>): Utilizes AoS layout, Euler integration, softening factor ε; serves as an unoptimized starting point processed on the CPU. 
2. Array of Structs to Struct of Arrays: Restructuring the data from AoS to SoA helps with memory coalescing; the process where instead of requesting each data point individually, the hardware requests batches.
3. CUDA port: Porting the CPU bound C code into a CUDA kernel to be run on NVIDIA GPUs; allows for parallel processing marking a huge jump in performance.
5. Float vs double precision: FP32 vs FP64, each with their own performance effects and accuracy.
       <br> FP32, Float: Higher performance but lower accuracy.
      <br>  FP64, Double: Much lower performance but increased accuracy.
6. Barnes-Hut algorithmic octree approximation: Further increasing performance from O(N<sup>2</sup>) to O(N log N); with minor accuracy complications.

## Building And Running the Program

### Building the Program
Windows Subsystem for Linux
   #### WSL installation
   1. Open PowerShell as Administrator
   2. Run ```dism.exe /online /enable-feature /featurename:Microsoft-Windows-Subsystem-Linux /all /norestart```
   3. Then run ```dism.exe /online /enable-feature /featurename:VirtualMachinePlatform /all /norestart```
   4. Restart PC
   5. Run ```wsl --set-default-version 2```
   6. Then run ```WSL --install```
   7. Restart PC
### Running the Program
1. Open Command Prompt
2. Run ```gcc nbody.c -o nbody -lm```
3. Run ```./nbody```
   #### Using the Nbody-sim
   - Input starting position x,y,z for particle 1.
   - Input starting velocity vx,vy,vz for particle 1: vx meaning velocity on the x coordinate etc; negative is left positive is right.
   - Input mass for particle 1.
   - Do the same for particle 2.
  
 >[!NOTE]
  >This is subject to change as the program evolves.


## References
- [MarcVivas — N-body Simulation (GitHub)](https://github.com/MarcVivas/N-body/tree/main)
- [Distributed Parallel Barnes-Hut N-body Simulation — arXiv:2203.08966](https://arxiv.org/abs/2203.08966)
- [GPU Gems, Ch. 37 — A Toolkit for Computation on GPUs](https://developer.nvidia.com/gpugems/gpugems/part-vi-beyond-triangles/chapter-37-toolkit-computation-gpus)
- [maythaswang — N-body Simulator (GitHub)](https://github.com/maythaswang/N-body-simulator/tree/main)
- [Algorithm Archive — Verlet Integration](https://www.algorithm-archive.org/contents/verlet_integration/verlet_integration.html)
- [Nyland, Harris & Prins — GPU Gems 3, Ch. 31: Fast N-Body Simulation with CUDA](https://developer.nvidia.com/gpugems/gpugems3/part-v-physics-simulation/chapter-31-fast-n-body-simulation-cuda)
- [NVIDIA Blog — An Easy Introduction to CUDA C and C++](https://developer.nvidia.com/blog/easy-introduction-cuda-c-and-c/)
- [NVIDIA Blog — Six Ways to SAXPY](https://developer.nvidia.com/blog/six-ways-saxpy/)

