# Performance Evaluations

## Execution Times

- We are including execution times from two different machines.
- In both cases `-O3` argument is used during compilation.

### Nebula

- Nebula is a machine designed for HPC (High Performance Computing)
- The CPU has 16 cores with hyper threading to provide 32 threads, 
- The GPU has 3584 CUDA cores.
- OS: Ubuntu 16.04.6 LTS (GNU/Linux 4.4.0-159-generic x86_64)

![](nebula-exe-times.png)

### Personal Computer

- This is a typical desktop computer.
- The CPU has 6 cores.
- The GPU has 640 CUDA cores.
- OS: KDE neon 5.18 (Ubuntu 18.04 based) (GNU/Linux 5.3.0-51-generic x86_64)

![](personal-exe-times.png)
