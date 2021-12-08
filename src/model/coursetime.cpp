#ifndef INCLUDE_COURSETIME_CPP_
#define INCLUDE_COURSETIME_CPP_

#include "course.cpp"
#include "classroom.cpp"
#include "timeslot.cpp"

/**
 * A class representing 1 hour of class.
 * It is basicly a three-way association between a course, a classroom and 
 * a timeslot.
 */
class CourseTime
{

private:

	/**
	 * This flag is set to true only for the very first hour of a consecutive
	 * set of course times. For example if the course CS406 has a 2 hour class on
	 * a day, then only the first hour has m_root set to true. This is used to 
	 * preserve this structure of consecutive classes during mutations and 
	 * crossovers.
	 */
  bool m_root;

	/**
	 * Each course has 2 potential classes during a week. This integer is 1 or 2
	 * depending on which one of these groups it is in.
	 */
  int m_group;

public:

  Course* m_course;
  Classroom* m_classroom;
  TimeSlot* m_timeslot;
  
  CourseTime(Course * t_course, Classroom * t_classroom,
			 TimeSlot * t_timeslot, bool t_root, int t_group){
	m_course = t_course;
	m_classroom = t_classroom;
	m_timeslot = t_timeslot;
	m_root = t_root;
	m_group = t_group;
  }

	/**
	 * Compares course times by the timeslots they are assigned to.
	 * Check TimeSlot::compare for more info.
	 */ 
	static bool compareByTime(const CourseTime & t1, const CourseTime & t2){
	return TimeSlot::compare(*t1.m_timeslot ,*t2.m_timeslot); 
  }

	/**
	 * Compares course times by course ids.
	 * Check Course::compare for more info.
	 */
  static bool compareByCourse(const CourseTime & t1, const CourseTime & t2){
	return Course::compare(*t1.m_course ,*t2.m_course); 
  }

	/**
	 * Compares course times by their groups where group1 < group2
	 */
  static bool compareByGroup(const CourseTime & t1, const CourseTime & t2){
	return t1.getGroup() < t2.getGroup();
  }

  bool isRoot() const {
	return m_root;
  }

  int getGroup() const {
	return m_group;
  }
};


#endif
