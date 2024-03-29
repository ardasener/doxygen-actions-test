# Motivation

## Motivation for the Program in General

The course selection and scheduling system in Sabancı University is quite complex. Firstly, all the courses are categorized as required, core, area and free according to the specifications of every major. When the semester starts, every student must determine their own schedule by choosing the courses from these pools. The scheduling process should be done separately for each academic semester. Furthermore, some courses are prerequisites of other courses which means that students can not take the up-level courses before passing the prerequisite courses. However, there are some conflicts during the course scheduling process. These problems can be categorized as time and classroom conflicts. Some of the possible problems which cause  time conflicts can be listed as:
- Scheduling more than one course at the same time slot that was taken simultaneously in the previous years.
- Assigning instructors to different courses at the same time slot. 
        
		
On the other hand, there are some other problems which cause classroom conflicts: 
- Scheduling two courses in the same classroom at the same time. 
- Scheduling the course to the classroom which does not have enough capacity.
- Scheduling the course to the classroom which does not have enough technical capability such as projection, smart tables for the needs of the course.
  
When the course timetables and classes are determined, some other issues must also be considered such as the right sequence of the lectures and recitations, priority of prerequisite courses, scheduling the courses of part-time lecturers according to their off-campus work times, special requests of lecturers like off-days. 
  
The team is currently in the possession of a naive, greedy algorithm that covers the most basic cases. However, we believe we can achieve better results with higher efficiency using OpenMP and CUDA with C++. We also are planning to implement a genetic algorithm as they are pleasingly parallel and are often suggested for scheduling algorithms (S. Abdullah and H. Turabieh, 2008), to replace the greedy algorithm. 
	
To sum up, we aim to find a solution to this course scheduling problem in this project by designing and implementing a more efficient and comprehensive scheduling algorithm. 
”Scheduling problems play an important role in many production environments and occur in a broad range of application areas. Although some types of scheduling problems can be solved easily and efficiently, most of the existing and practically arising scheduling problems are NP-hard and difficult to solve in practice, particularly, when provably optimal solutions are desired.“(Holger H. Hoos & Thomas Stützle, 2005)


## Motivation for a Genetic Algorithm

The genetic algorithm is a method for solving both constrained and unconstrained optimization problems that is based on natural selection, the process that drives biological evolution. The genetic algorithm repeatedly modifies a population of individual solutions. At each step, the genetic algorithm selects individuals at random from the current population to be parents and uses them to produce the children for the next generation. Over successive generations, the population "evolves" toward an optimal solution. (MathWorks, 2020)

There is quite a lot of literary work suggesting the suitability of genetic algorithms in scheduling problems. Some excerpts are included below.

“From the tests it can be concluded that the automatic scheduling system is well made. Processing speed is also good, averaging less than 1 minute. In addition, the automatically generated class schedules are also correct and in accordance with the expectations of users.” (Budhi, Gunadi, & Wibowo, 2015)

“This paper has introduced a genetic algorithm and local search. A new repair function is also presented that is totally able to change infeasible timetable to feasible one.” (S. Abdullah and H. Turabieh, 2008)


## Motivation for Parallelism in General

Algorithm is not too slow to the point that use of parallelism is required to get decent results. However due to the nature of genetic algorithms, longer waits usually result in more optimal results. Therefore any speedups to we can introduce to the algorithm, will result in more optimal results in the same timeframe.

There is also precedent that genetic algorithms perform rather well in parallel. Here is an excerpt suggesting as such:

“This survey has presented numerous examples that show that parallel GAs are capable of combining speed and efficacy, and that we are reaching a better understand-ing which should allow us to utilize them better in the future.” (Cantú-Paz E., 1998)


## Motivation for GPU Computation (CUDA)

Simply using CPU parallelization with OpenMP, resulted in decent speedups. However it was quite obvious that calculating the fitness of chromosomes was a decent bottleneck. The current data we have ends up introducing around 2000 class hours per chromosome. Even when using a conservative number of chromosomes, there is quite a lot to compute here. We decided that fitnesses for all chromosomes in a generation can be computed in bulk in the GPU making use of the high number of cores.

The use of CUDA specifically was out of convenience as that was the framework we were familier with. It is quite possible to replicate these results with another GPU programming framework/library like OpenCL or Vulkan Compute Shaders.


## References

- S. Abdullah and H. Turabieh, (2008). Generating University Course Timetable Using Genetic Algorithms and Local Search, Third International Conference on Convergence and Hybrid Information Technology, Busan, pp. 254-260.
- Budhi, G. S., Gunadi, K., & Wibowo, D. A. (2015). Genetic Algorithm for Scheduling Courses. Communications in Computer and Information Science Intelligence in the Era of Big Data, 51–63. doi: 10.1007/978-3-662-46742-8_5
- Cantú-Paz, E. (1998). A survey of parallel genetic algorithms. Calculateurs paralleles, reseaux et systems repartis, 10(2), 141-171.
- Holger H. Hoos, H., & Thomas Stützle, T. (2005). Stochastic local search. Retrieved October 19, 2019 from www.sciencedirect.com 
