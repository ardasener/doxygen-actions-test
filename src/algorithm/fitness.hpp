#ifndef INCLUDE_FITNESS_HPP_
#define INCLUDE_FITNESS_HPP_

#include "schedule.cpp"

/**
 * Calculates the fitness of multiple schedules given in a vector at once using
 * CUDA GPU parallelism.
 */
void calculateFitnessBulk(std::vector<Schedule *>* schedules, int* concurrency_matrix, int courses_size, Config& config);

#endif
