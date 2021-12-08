#ifndef INCLUDE_CLASSROOM_CPP_
#define INCLUDE_CLASSROOM_CPP_

#include <string>

//! Class representing a classroom at Sabancı University
class Classroom
{
public:

	//! A unique integer for the classroom (assigned by load.cpp)
  int m_id;

	//! A unique string for the classroom (Example: FENS L089)
  std::string m_code;

	//! Maximum number of students that the classroom can hold
  int m_capacity;

  Classroom(int t_id, std::string t_code, int t_capacity){
	m_id = t_id;
	m_code = t_code;
	m_capacity = t_capacity;
  }


};



#endif
