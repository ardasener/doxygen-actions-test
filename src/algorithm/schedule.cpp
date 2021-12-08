#ifndef INCLUDE_SCHEDULE_CPP_
#define INCLUDE_SCHEDULE_CPP_

#include "../model/timeslot.cpp"
#include "../model/coursetime.cpp"
#include "../model/concurrent.cpp"
#include "../util/config.cpp"
#include "../util/random.hpp"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <iostream>

using Random = effolkronium::random_thread_local;


/**
 * A representation of a weekly course schedule.
 * Also a chromosome in the terms of a genetic algorithm
 */
class Schedule {
	private:

		//! Number of points to be taken from the other schedule during crossovers
		int m_numberOfCrossoverPoints;

		//! Number of points to be randomly changed during mutations
		int m_mutationSize;

		//! Chance of a crossover to occur (between 0 and 100)
		int m_crossoverProbability;

		//! Chance of a mutation to occur (between 0 and 100)
		int m_mutationProbability;

		/**
		 * Fitness value of the chromosome calculated with above penalties.
		 * Lower is better (perhaps confusingly).
		 */
		int m_fitness;

		//! If true, mutation sizes are randomized between 0 and 2*m_mutationSize
		bool m_variableMutations;

		//! A vector of all time slots
		std::vector<TimeSlot> * m_slots;

		//! A vector of all courses
		std::vector<Course>* m_courses;

		//! A vector of all classrooms
		std::vector<Classroom>* m_classrooms;

		//! The schedule itself, a vector of course times
		std::vector<CourseTime> * m_courseTimes;

		//! An object for accessing concurrency information
		const Concurrent * m_concurrent;

		Config * m_config;


		bool isRecitation(Course * c){
			char last = c->getCode().back();
			return (last == 'R' || last == 'D' || last == 'L');

		}

		bool instructorCollision(Course * c1, Course * c2){
			if(isRecitation(c1) || isRecitation(c2)){
				return false;
			}

			auto inst1 = c1->getInstructors() ;
			auto inst2 = c2->getInstructors() ;



			for (auto pos = inst1->begin(); pos != inst1->end(); pos++) {
				if(inst2->count(*pos) != 0){
					return true;
				}
			}

			return false;
		}

		int blockCollisionCount(Course * c1, Course * c2){
			auto blocks1 = c1->getBlocks();
			auto blocks2 = c2->getBlocks();

			int count = 0;

			for(auto pos = blocks1->begin() ; pos != blocks1->end() ; pos++){
				if(blocks2->count(*pos) > 0){
					count++;
				}
			}

			return count;

		}



	public:

		Schedule(){

		}

		~Schedule(){
			delete m_courseTimes;
		}

		Schedule(Config & config){


			m_mutationSize = config.m_mutationSize;
			m_numberOfCrossoverPoints =config.m_numberOfCrossoverPoints;
			m_mutationProbability = config.m_mutationProbability;
			m_crossoverProbability = config.m_crossoverProbability;

			m_variableMutations = config.m_variableMutations;

			m_fitness = -1;

			m_config = &config;

			m_slots = config.m_slots;

			m_concurrent = config.m_concurrent;
			m_courses = config.m_courses;
			m_classrooms = config.m_classrooms;
		}

		Schedule(const Schedule & c){
			m_mutationSize = c.m_mutationSize;
			m_numberOfCrossoverPoints = c.m_numberOfCrossoverPoints;
			m_mutationProbability = c.m_mutationProbability;
			m_crossoverProbability = c.m_crossoverProbability;

			m_variableMutations = c.m_variableMutations;

			m_fitness = -1;

			m_config = c.m_config;

			//Copies just the pointers to these vectors
			//All these values are shared and not changed
			m_concurrent = c.m_concurrent;
			m_courses = c.m_courses;
			m_classrooms = c.m_classrooms;
			m_slots = c.m_slots;

			m_courseTimes = new std::vector<CourseTime>();
			m_courseTimes->assign(c.m_courseTimes->begin(), c.m_courseTimes->end());
		}

		static Schedule* getRandomSchedule(Config & config){
			Schedule * schedule = new Schedule(config);

			schedule->m_courseTimes = new std::vector<CourseTime>();

			for(auto pos = config.m_courses->begin(); pos != config.m_courses->end(); pos++){

				//Allah kerim
				Course * course = &*pos;

				int classroom_size = config.m_classrooms->size();
				int timeslot_size = config.m_slots->size();
				int classroom_index = Random::get(0, classroom_size-1);
				int timeslot_index1 = Random::get(0, timeslot_size-1);
				int timeslot_index2 = Random::get(0, timeslot_size-1);

				while(config.m_slots->at(timeslot_index1).m_hour > TimeSlot::MAX_HOUR - course->getDuration1() + 1){
					timeslot_index1--;
				}

				while(config.m_slots->at(timeslot_index2).m_hour > TimeSlot::MAX_HOUR - course->getDuration2() + 1){
					timeslot_index2--;
				}



				Classroom * classroom = &(config.m_classrooms->at(classroom_index));

				bool root = true;
				for(int i=0; i<course->getDuration1(); i++){
					TimeSlot* slot = &config.m_slots->at(timeslot_index1);
					CourseTime * course_time = new CourseTime(course,classroom,slot,root,1);
					schedule->m_courseTimes->push_back(*course_time);
					timeslot_index1++;
					root = false;
				}


				root = true;
				for(int i=0; i<course->getDuration2(); i++){
					TimeSlot* slot = &config.m_slots->at(timeslot_index2);
					CourseTime * course_time = new CourseTime(course,classroom,slot,root,2);
					schedule->m_courseTimes->push_back(*course_time);
					timeslot_index2++;
					root = false;
				}

			}

			return schedule;
		}



		/**
		 * Calculates the fitness of this schedule by applying penalties to certain
		 * situations. This is explained in more detail in Technical Details
		 * section.
		 */
		void calculateFitness(){
			//TODO: Use the formula here

			std::sort(m_courseTimes->begin(), m_courseTimes->end(), CourseTime::compareByTime);
			int fitness = 0;

			//For each instructor keeps a vector of number of classes per day
			std::unordered_map<std::string, std::vector<int>> inst_map;

			//For each course hold a vector of the days it is in
			std::unordered_map<std::string, std::unordered_set<int>> course_map;

			for(auto pos = m_courseTimes->begin() ; pos != m_courseTimes->end() ; pos++){


				CourseTime ct1 = *pos;
				Course * course1 = ct1.m_course;
				TimeSlot * timeslot1 = ct1.m_timeslot;
				Classroom * classroom1 = ct1.m_classroom;


				//Iterate over instructors for the course and add them to the map
				//Penalty is applied at the very end of this function
				for(auto inst_pos = course1->getInstructors()->begin(); inst_pos != course1->getInstructors()->end(); inst_pos++){

					std::string inst = *inst_pos;
					if(inst_map.count(inst) == 0){
						std::vector<int> days;
						days.resize(5,0);
						inst_map.insert(std::pair<std::string,std::vector<int>>(inst,days));
					}

					inst_map[inst].at(timeslot1->m_day)++;
				}

				//Add this day to the course_map
				//Penalty is applied at the very end of this function
				if(course_map.count(course1->getCode()) == 0){
					std::unordered_set<int> days;
					course_map.insert(std::pair<std::string,std::unordered_set<int>>(course1->getCode(),days));
				}
				course_map[course1->getCode()].insert(timeslot1->m_day);


				//Check for capacity
				if(course1->getCapacity() > classroom1->m_capacity){
					//TODO: make this scale
					fitness += m_config->CAPACITY_PENALTY;
				}


				//All classes in same day check
				for(auto pos2 = pos+1; pos2 < m_courseTimes->end(); pos2++){
					CourseTime ct2 = *pos2;
					TimeSlot * timeslot2 = ct2.m_timeslot;
					Course * course2 = ct2.m_course;

					if(timeslot2->m_day != timeslot1->m_day){
						break;
					}

					if(course2->getId() == course1->getId() && ct2.getGroup() != ct1.getGroup()){
						fitness += m_config->SAME_DAY_PENALTY;
					}

				}
				//Check for collisions
				for(auto pos2 = pos+1; pos2 < m_courseTimes->end(); pos2++){

					CourseTime ct2 = *pos2;
					TimeSlot * timeslot2 = ct2.m_timeslot;
					Course * course2 = ct2.m_course;
					Classroom * classroom2 = ct2.m_classroom;


					if(timeslot1 != timeslot2){
						break;
					}

					if (instructorCollision(course1, course2)) {
						fitness += m_config->INSTRUCTOR_COLLISION_PENALTY;
					}

					if(classroom1 == classroom2){
						fitness += m_config->CLASSROOM_COLLISION_PENALTY;
					}

					//Calculate concurrency penalty
					int concurrent_penalty = m_config->CONCURRENCY_COLLISION_PENALTY_MAX * 
						((float) m_concurrent->get(course1->getCode() , course2->getCode()) / m_config->MAX_CONCURRENCY) ;

					if(concurrent_penalty > m_config->CONCURRENCY_COLLISION_PENALTY_MAX){
						concurrent_penalty = m_config->CONCURRENCY_COLLISION_PENALTY_MAX;
					}
					fitness += concurrent_penalty;


					//Calculate block penalty
					int block_penalty = m_config->ACADEMIC_BLOCK_COLLISION_PENALTY * blockCollisionCount(course1,course2);
					fitness += block_penalty;
				}
			}

			//Instructor too many classes penalty
			for(auto pos = inst_map.begin(); pos != inst_map.end(); pos++){
				for(auto vec_pos = pos->second.begin(); vec_pos != pos->second.end(); vec_pos++){
					int count = *vec_pos;

					if(count > 2){
						int inst_penalty = (m_config->INSTRUCTOR_TOO_MANY_CLASSES_PENALTY_MAX * (((float) count) / 10)) + m_config->INSTRUCTOR_TOO_MANY_CLASSES_PENALTY_MIN;
						//std::cout << "Inst Penalty: " << count << ", " << inst_penalty << std::endl;
						fitness += inst_penalty;
					}
				}
			}

			//Course consecutive days penalty
			for(auto pos = course_map.begin(); pos != course_map.end(); pos++){
				//
				for(int i=0; i<4; i++){
					if(pos->second.count(i) != 0 && pos->second.count(i+1) != 0){
						//std::cout << "Found consecutive days" << std::endl;
						fitness += m_config->CONSECUTIVE_DAYS_PENALTY;
					}
				}
			}


			m_fitness = fitness;
		}

		int getFitness() const {
			return m_fitness;
		}

		void setFitness(int t_fitness){
			m_fitness = t_fitness;
		}

		/**
		 * Mutates this schedule in place by changing a random selection of classes
		 * to random classrooms and time slots. Number of points mutated is defined
		 * in the m_mutationSize variable. If variable mutations is enabled, then
		 * a random value between 0 and m_mutationSize is used instead.
		 */
		void mutation() {

			if(Random::get(0,100) >= m_mutationProbability){
				return;
			}

			std::sort(m_courseTimes->begin(), m_courseTimes->end(), CourseTime::compareByGroup);
			std::stable_sort(m_courseTimes->begin(), m_courseTimes->end(), CourseTime::compareByTime);
			std::stable_sort(m_courseTimes->begin(), m_courseTimes->end(), CourseTime::compareByCourse);

			int classes_size = m_courseTimes->size();
			int classroom_count = m_classrooms->size();
			int timeslots_size = m_slots->size();

			int mutation_size = m_mutationSize;

			if(m_variableMutations){
				mutation_size = Random::get(0,2*m_mutationSize);
			}

			for(int mutation_count=0; mutation_count < mutation_size; mutation_count++){

				int random_index = Random::get(0,classes_size-1);
				int random_classroom = Random::get(0,classroom_count-1);
				int random_timeslot = Random::get(0,timeslots_size-1);

				while(!m_courseTimes->at(random_index).isRoot()){
					random_index--;

					if(random_index < 0){
						std::cerr << "Failed to find root, this should not happen!" << std::endl;
						random_index = Random::get(0,classes_size-1);
					}
				}


				CourseTime* ct = &(m_courseTimes->at(random_index));
				int duration = ct->m_course->getDuration1();

				if(ct->getGroup() == 2){
					duration = ct->m_course->getDuration2();
				}

				while(m_slots->at(random_timeslot).m_hour > TimeSlot::MAX_HOUR - duration + 1){
					random_timeslot--;
				}

				for(int i=0; i<duration; i++){
					m_courseTimes->at(random_index).m_classroom = &m_classrooms->at(random_classroom);
					m_courseTimes->at(random_index).m_timeslot  = &m_slots->at(random_timeslot);
					random_index++;
				}


			}

		}


		/**
		 * Crosses over this schedule with another scheduling and returns
		 * a pointer to the new schedule. During the crossover this schedule is
		 * copied and then a number of points defined in m_numberOfCrossoverPoints
		 * is taken from the other course given as parameter to this function.
		 */
		Schedule * crossover(Schedule & parent2) const {

			Schedule * new_schedule = new Schedule(*this);

			if(Random::get(0,100) >= m_crossoverProbability){
				return new_schedule;
			}

			Schedule * new_parent2 = new Schedule(parent2);

			//Sort course times by course ids
			//We sort twice one being stable, so that
			//equivalent courses are sorted by time

			std::sort(new_schedule->m_courseTimes->begin(), new_schedule->m_courseTimes->end(), CourseTime::compareByGroup);
			std::stable_sort(new_schedule->m_courseTimes->begin(), new_schedule->m_courseTimes->end(), CourseTime::compareByTime);
			std::stable_sort(new_schedule->m_courseTimes->begin(), new_schedule->m_courseTimes->end(), CourseTime::compareByCourse);

			std::sort(new_parent2->m_courseTimes->begin(), new_parent2->m_courseTimes->end(), CourseTime::compareByGroup);
			std::stable_sort(new_parent2->m_courseTimes->begin(), new_parent2->m_courseTimes->end(), CourseTime::compareByTime);
			std::stable_sort(new_parent2->m_courseTimes->begin(), new_parent2->m_courseTimes->end(), CourseTime::compareByCourse);

			for(int i=0; i<m_numberOfCrossoverPoints; i++){

				int course_times_size = new_schedule->m_courseTimes->size();
				int rand_index = Random::get(0,course_times_size-1);

				while(!new_schedule->m_courseTimes->at(rand_index).isRoot()){
					rand_index--;


					if(rand_index < 0){
						std::cerr << "Failed to find root, this should not happen!" << std::endl;
						rand_index = Random::get(0,course_times_size-1);
					}
				}


				if(rand_index >= m_courseTimes->size()){
					std::cout << "rand_index out of bounds crossover: " << rand_index << std::endl;
					throw "out of bounds";
				}

				CourseTime * ct1 = &(new_schedule->m_courseTimes->at(rand_index));
				// CourseTime * ct2 = &(new_parent2->m_courseTimes->at(rand_index));

				int duration = ct1->m_course->getDuration1();

				if(ct1->getGroup() == 2){
					duration = ct1->m_course->getDuration2();
				}

				for(int i=0; i<duration; i++){
					new_schedule->m_courseTimes->at(rand_index).m_classroom =  new_parent2->m_courseTimes->at(rand_index).m_classroom;
					new_schedule->m_courseTimes->at(rand_index).m_timeslot =  new_parent2->m_courseTimes->at(rand_index).m_timeslot;
					rand_index++;
				}

			}

			delete new_parent2;
			return new_schedule;

		}

		std::vector<CourseTime> * getCourseTimes(){
			return m_courseTimes;
		}

		int getSize(){
			return m_courseTimes->size();
		}


};

#endif
