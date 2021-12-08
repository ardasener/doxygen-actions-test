#include "schedule.cpp"
#include <iostream>
#include <vector>
#include <chrono>
#include <unistd.h>
#include <thread>
#include "../util/config.cpp"
#include "../util/output.cpp"
#include "../util/random.hpp"

#ifdef CUDA
#include "fitness.hpp"
#endif


using Random = effolkronium::random_thread_local;

//#include <omp.h>

class Algorithm {

	private:

		//! Size of the vector m_bestChromosomes currently (must be < m_trackBest)
		int m_currentBestSize;

		//! Number of unfit chromosomes to be replaced in the next generation
		int m_replaceByGeneration;

		//! ID of the current generation
		int m_currentGeneration;

		//! Total number of chromosomes
		int m_numberOfChromosomes;

		//! Number of fit chromosomes to be kept
		int m_trackBest;

		/**
		 * A vector that keeps the chromosomes with the best fitness scores. A pair
		 * is kept for each chromosome. A pair consists of a pointer to the chromosome
		 * and the index of it in the m_chromosomes.
		 */
		std::vector<std::pair<Schedule*, int>*> m_bestChromosomes;

		//! A vector that keeps the indices of the best chromosomes in m_chromosomes
		std::vector<int> m_bestIndices;

		/**
		 * For all chromsomes in m_chromosomes, keeps "true" in the same index
		 * if that chromsome is in m_bestChromosomes. Keeps "false" otherwise.
		 */
		std::vector<bool> m_bestFlags;

		//! A vector of all the chromosomes
		std::vector<Schedule *> m_chromosomes;

		//! A vector of all the courses
		std::vector<Course> *m_courses;

		//! A vector of all the classrooms
		std::vector<Classroom> *m_classrooms;

		//! A vector of all the time slots.
		std::vector<TimeSlot> *m_timeslots;

		//! An object for accessing concurrency information
		Concurrent *m_concurrent;

		//! A copy of the config to be given to schedule constructors.
		Config m_config;

		//! Refer to Schedule class
		int m_numberOfCrossoverPoints = 100;

		//! Refer to Schedule class
		int m_mutationSize = 100;

		//! Refer to Schedule class
		int m_crossoverProbability = 80;

		//! Refer to Schedule class
		int m_mutationProbability = 50;

		//! Returns true if the chromosome is in m_bestChromosomes
		bool isInBest(int t_index) const { return m_bestFlags[t_index]; }

		//! Clears the best chromosomes
		void clearBest() {
			for (unsigned int i = 0; i < m_bestFlags.size(); i++) {
				m_bestFlags[i] = false;
			}
		}

		/** Adds the chromosome in the given index to m_bestChromsomes if it has a
		 * better score than the last chromosome in m_bestChromosomes. Addition is
		 * done in a way that will preserve the order of the vector.
		 */
		void addToBest(int t_index) {

			Schedule *chromosome = m_chromosomes[t_index];

			if (m_bestChromosomes.size() < m_trackBest) {
				m_bestChromosomes.push_back(new std::pair<Schedule*,int>(chromosome,t_index));
				std::sort(m_bestChromosomes.begin(), m_bestChromosomes.end(),
						compareChromosome);
				m_bestFlags[t_index] = true;
				return;
			}

			if (chromosome->getFitness() < m_bestChromosomes.at(m_bestChromosomes.size() - 1)->first->getFitness()) {

				m_bestFlags[m_bestChromosomes.at(m_bestChromosomes.size()-1)->second] = false;
				m_bestChromosomes.at(m_bestChromosomes.size() - 1) = new std::pair<Schedule*,int>(chromosome,t_index);
				std::sort(m_bestChromosomes.begin(), m_bestChromosomes.end(),
						compareChromosome);

				m_bestFlags[t_index] = true;
				return;
			}

			m_bestFlags[t_index] = false;
		}

		//! Calls addToBest() on all elements of m_chromosomes
		void addAllToBest(){
			for (unsigned int i=0; i < m_chromosomes.size(); ++i) {
				addToBest(i);
			}
		}


		//! Compares to chromosomes based on their fitness. Lowest (best) first.
		static bool compareChromosome(const std::pair<Schedule*,int> *s1, const std::pair<Schedule*,int> *s2) {
			return s1->first->getFitness() < s2->first->getFitness();
		}

	public:
		Algorithm(Config & config) {

			if (config.m_numberOfChromosomes < 2) {
				throw "Number of chromosomes must be at least 2.";
			}

			if (config.m_trackBest < 1) {
				throw "At least 1 of the best must be tracked.";
			}

			if (config.m_replaceByGeneration < 1) {
				throw "At least 1 non-best chromosome must be replaced.";
			}

			if (config.m_numberOfChromosomes <= config.m_trackBest) {
				throw "There should be more chromosomes than the tracked best.";
			}

			if (config.m_replaceByGeneration > config.m_numberOfChromosomes - config.m_trackBest) {
				throw "Replaced chromosomes can not be more than the total of non-best chromosomes.";
			}

			m_numberOfChromosomes = config.m_numberOfChromosomes;
			m_trackBest = config.m_trackBest;
			m_replaceByGeneration = config.m_replaceByGeneration;

			m_currentBestSize = 0;
			m_currentGeneration = 0;
			m_courses = config.m_courses;
			m_classrooms = config.m_classrooms;
			m_timeslots = config.m_slots;
			m_concurrent = config.m_concurrent;
			m_config = config;


#ifdef CUDA
			//Initialize the matrix to be used in CUDA
			m_concurrent->generateCudaMatrix(m_courses);
#endif

			// Initialize the vectors
			// m_chromosomes.resize(m_numberOfChromosomes,nullptr);
			m_bestFlags.resize(m_numberOfChromosomes, false);
			// m_bestChromosomes.resize(m_trackBest,-1);
		}

		Schedule *getBestChromosome() const { return m_bestChromosomes[0]->first; }


		/**
		 * Main function that starts the algorithm. It will run until the
		 * t_iterationCount is reached. The best chromosome left on the final
		 * iteration is exported into a file.
		 */
		void run(int t_iterationCount = 1000) {


			srand(time(0));

			auto start = std::chrono::steady_clock::now();

			m_chromosomes.resize(m_numberOfChromosomes,nullptr);

			// Randomly create the initial chromosomes
#pragma omp parallel for default(shared)
			for (int i = 0; i < m_numberOfChromosomes; i++) {
				//std::cout << "Initial Chromosome: " << i << std::endl;
				Schedule *schedule = Schedule::getRandomSchedule(m_config);

#ifndef CUDA
				schedule->calculateFitness();
#endif

				m_chromosomes[i] = schedule;
			}

#ifdef CUDA
#pragma omp parallel default(shared)
			{

				int split = m_chromosomes.size() * ((float)m_config.cuda_perc/100);


#pragma omp single nowait
				{

					try{
						std::vector<Schedule*> cuda_chromosomes(m_chromosomes.begin(),
								m_chromosomes.begin()+split);
						calculateFitnessBulk(&cuda_chromosomes, m_concurrent->getCudaMatrix(), m_courses->size(), m_config);
					}  catch (const char *e){
						std::cout << "Error: " << e << std::endl;
					}
				}

#pragma omp for
				for(int chr_index=split; chr_index < m_chromosomes.size(); chr_index++)
				{
					m_chromosomes[chr_index]->calculateFitness();
				}

			}

#endif

			//Adds all initial chromosomes to best (if they are indeed the best)
			addAllToBest();

			output("./schedule-output-gen-0.txt", getBestChromosome()->getCourseTimes());

			std::cout << std::endl;
			std::cout << " --- Initial: ---" << std::endl;
			std::cout << "Best Fitness: "	 << getBestChromosome()->getFitness() << std::endl;
			std::cout << std::endl;

			m_currentGeneration = 0;

			for (int i = 0; i < t_iterationCount; i++) {

				//Produce Offspring
				std::vector<Schedule *> offspring;
				offspring.resize( m_replaceByGeneration, nullptr);

#pragma omp parallel for default(shared)
				for (int j = 0; j < m_replaceByGeneration; j++) {

					int best_size = m_bestChromosomes.size();
					int p1_index = Random::get(0,best_size-1);
					int p2_index = Random::get(0,best_size-1);

					if(p1_index == p2_index){
						p2_index = (p2_index + 1) % m_bestChromosomes.size();
					}

					Schedule *p1 = m_bestChromosomes[p1_index]->first;
					Schedule *p2 = m_bestChromosomes[p2_index]->first;

					offspring[j] = p1->crossover(*p2);
					offspring[j]->mutation();

#ifndef CUDA
					offspring[j]->calculateFitness();
#endif

				}

#ifdef CUDA
#pragma omp parallel default(shared)
			{

				int split = offspring.size() * ((float)m_config.cuda_perc/100);


#pragma omp single nowait
				{

					try{
						std::vector<Schedule*> cuda_chromosomes(offspring.begin(),
								offspring.begin()+split);
						calculateFitnessBulk(&cuda_chromosomes, m_concurrent->getCudaMatrix(), m_courses->size(), m_config);
					}  catch (const char *e){
						std::cout << "Error: " << e << std::endl;
					}
				}

#pragma omp for
				for(int chr_index=split; chr_index < offspring.size(); chr_index++)
				{
					offspring[chr_index]->calculateFitness();
				}

			}


#endif


				//Replace old chromosome with offspring
				for (int j = 0; j < m_replaceByGeneration; j++) {

					int best_flags_size = m_bestFlags.size();
					int rand_index = Random::get(0,best_flags_size-1);

					while (isInBest(rand_index)) {
						rand_index = (rand_index + 1) % m_bestFlags.size();
					}

					delete m_chromosomes[rand_index];
					m_chromosomes[rand_index] = offspring[j];
					addToBest(rand_index);
				}

				m_currentGeneration++;

				if(m_currentGeneration % 10 == 0){
					auto end = std::chrono::steady_clock::now();

					std::cout << " --- Generation: " << m_currentGeneration << " ---" << std::endl;
					std::cout << "Elapsed Time: " << (float) std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000 << " seconds" << std::endl;
					std::cout << "Best Fitness: "	 << getBestChromosome()->getFitness() << std::endl;
					std::cout << std::endl;
				}

			}


			output("./schedule-output-gen-final.txt", getBestChromosome()->getCourseTimes());
		}
};
