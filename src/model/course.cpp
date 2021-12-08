#ifndef INCLUDE_COURSE_CPP_
#define INCLUDE_COURSE_CPP_


#include <string>
#include <unordered_map>
#include <unordered_set>
#include <iostream>

/**
 * This class is a representation of a course in Sabancı University
 */
class Course
{
	private:
		//! An integer unique to this course (assigned automatically by load.cpp)
		int m_id;

		//! Duration for the first class of the week
		int m_duration1;

		//! Duration for the second class of the week
		int m_duration2;

		/**
		 * A unique string for the course.
		 * Obtained by concatenating course code (example: CS 406)
		 * and section (example: A)
		 */
		std::string m_code;

		//! Another unique string that is currently not used
		std::string m_crn;

		//! Maximum number of students that can take this code in one term
		int m_capacity;

		//! Set of academic block ids (explained in more detail in the data section)
		std::unordered_set<int>* m_academicBlocks;

		//! Names of the instructors that are teaching this course
		std::unordered_set<std::string>* m_instructors;

		//! ID's for the instructors mentioned above (assigned by load.cpp)
		std::unordered_set<int>* m_instructorIds;

	public:
		Course(int t_id, std::string t_code, std::string t_crn, int t_capacity,
				std::unordered_set<int>* t_block, std::unordered_set<std::string>* t_instructors,
				std::unordered_set<int>* t_instructor_ids, int t_duration1, int t_duration2){
			m_id = t_id;
			m_code = t_code;
			m_crn = t_crn;
			m_capacity = t_capacity;
			m_academicBlocks = t_block;
			m_instructors = t_instructors;
			m_instructorIds = t_instructor_ids;
			m_duration1 = t_duration1;
			m_duration2 = t_duration2;
		}


		/**
		 * Compares the two given courses by their m_id member variables.
		 * It is assumed that these ids are unique to each course.
		 * And should be assigned as such in load.cpp
		 */
		static bool compare(const Course & c1, const Course & c2){
			return c1.m_id < c2.m_id;
		}

		std::unordered_set<std::string>* getInstructors() {
			return m_instructors;
		}

		std::unordered_set<int>* getInstructorIds() {
			return m_instructorIds;
		}


		std::string getCode() const {
			return m_code;
		}

		int getCapacity() const {
			return m_capacity;
		}

		int getDuration1() const {
			return m_duration1;
		}

		int getDuration2() const {
			return m_duration2;
		}

		int getId() const {
			return m_id;
		}

		std::unordered_set<int> * getBlocks(){
			return m_academicBlocks;
		}
};


#endif
