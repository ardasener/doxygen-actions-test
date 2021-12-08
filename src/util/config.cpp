#ifndef INCLUDE_CONFIG_CPP_
#define INCLUDE_CONFIG_CPP_

#include <vector>
#include "../model/course.cpp"
#include "../model/classroom.cpp"
#include "../model/timeslot.cpp"
#include "../model/concurrent.cpp"
#include <fstream>
#include "./cpptoml.h"

class Config {

	public:
		int m_numberOfCrossoverPoints = 100;
		int m_mutationSize = 100;
		int m_crossoverProbability = 100;
		int m_mutationProbability = 60;

		int m_numberOfChromosomes = 100;
		int m_trackBest = 20;
		int m_replaceByGeneration = 60;

		bool m_variableMutations = false;

		std::vector<Course> * m_courses;
		std::vector<Classroom> * m_classrooms;
		std::vector<TimeSlot> * m_slots;
		Concurrent * m_concurrent;

		int SAME_DAY_PENALTY;
		int INSTRUCTOR_COLLISION_PENALTY;
		int CAPACITY_PENALTY;
		int CLASSROOM_COLLISION_PENALTY;
		int CONCURRENCY_COLLISION_PENALTY_MAX;
		int ACADEMIC_BLOCK_COLLISION_PENALTY;
		int INSTRUCTOR_TOO_MANY_CLASSES_PENALTY_MAX;
		int INSTRUCTOR_TOO_MANY_CLASSES_PENALTY_MIN;
		int CONSECUTIVE_DAYS_PENALTY;
		int MAX_CONCURRENCY;

		int num_iterations = 1;
		int num_threads = 1;
		int device_num = 0;
		int cuda_perc = 50;

		Config(){

		}

		Config(std::string config_file_name,std::vector<Course> * courses,
				std::vector<Classroom>* classrooms, std::vector<TimeSlot> * timeslots, Concurrent * concurrent){
		
			m_courses = courses;
			m_classrooms = classrooms;
			m_slots = timeslots;
			m_concurrent = concurrent;
			
			auto config = cpptoml::parse_file(config_file_name);

			std::cout << "Reading config file..." << std::endl;

			m_numberOfCrossoverPoints = config->get_qualified_as<int>("algorithm.crossover_size").value_or(-1);
			m_mutationSize = config->get_qualified_as<int>("algorithm.mutation_size").value_or(-1);
			m_crossoverProbability = config->get_qualified_as<int>("algorithm.crossover_prob").value_or(-1);
			m_mutationProbability = config->get_qualified_as<int>("algorithm.mutation_prob").value_or(-1);

			m_numberOfChromosomes = config->get_qualified_as<int>("algorithm.num_chromosomes").value_or(-1);

			m_trackBest = config->get_qualified_as<int>("algorithm.track_best").value_or(-1);
			m_replaceByGeneration = config->get_qualified_as<int>("algorithm.replace_generation").value_or(-1);


			m_variableMutations = config->get_qualified_as<bool>("algorithm.variable_mutations").value_or(false);



			SAME_DAY_PENALTY = config->get_qualified_as<int>("constants.same_day_penalty").value_or(-1);

			INSTRUCTOR_COLLISION_PENALTY = config->get_qualified_as<int>("constants.instructor_collision_penalty").value_or(-1);

			CAPACITY_PENALTY = config->get_qualified_as<int>("constants.capacity_penalty").value_or(-1);

			CLASSROOM_COLLISION_PENALTY = config->get_qualified_as<int>("constants.classroom_collision_penalty").value_or(-1);

			CONCURRENCY_COLLISION_PENALTY_MAX = config->get_qualified_as<int>("constants.concurrency_collision_penalty_max").value_or(-1);

			ACADEMIC_BLOCK_COLLISION_PENALTY = config->get_qualified_as<int>("constants.academic_block_collision_penalty").value_or(-1);

			INSTRUCTOR_TOO_MANY_CLASSES_PENALTY_MAX = config->get_qualified_as<int>("constants.instructor_too_many_classes_penalty_max").value_or(-1);

			INSTRUCTOR_TOO_MANY_CLASSES_PENALTY_MIN = config->get_qualified_as<int>("constants.instructor_too_many_classes_penalty_min").value_or(-1);

			CONSECUTIVE_DAYS_PENALTY = config->get_qualified_as<int>("constants.consecutive_days_penalty").value_or(-1);

			MAX_CONCURRENCY = config->get_qualified_as<int>("constants.max_concurrency").value_or(-1);

			num_threads = config->get_qualified_as<int>("parallel.num_threads").value_or(-1);
			device_num = config->get_qualified_as<int>("parallel.device_num").value_or(-1);

			cuda_perc = config->get_qualified_as<int>("parallel.cuda_perc").value_or(-1);

			num_iterations = config->get_qualified_as<int>("algorithm.num_iterations").value_or(-1);
			std::cout << "Finished reading config file." << std::endl;
		}

};

#endif
