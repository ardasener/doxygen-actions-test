#ifndef INCLUDE_OUTPUT_CPP_
#define INCLUDE_OUTPUT_CPP_

#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include "../model/coursetime.cpp"

std::string dayToString(int day){
  switch(day){
  case 0: return "Monday";
  case 1: return "Tuesday";
  case 2: return "Wednesday";
  case 3: return "Thursday";
  case 4: return "Friday";
  default: return "Christmas";
  }
}


void output(std::string filename, std::vector<CourseTime> * data){
  std::ofstream file;
  file.open(filename);

  std::sort(data->begin(), data->end(), CourseTime::compareByTime);

  TimeSlot * last_slot = data->at(0).m_timeslot;
  file << dayToString(last_slot->m_day) << " " << last_slot->m_hour << ": "; 

  for(unsigned int i=0; i < data->size(); i++){
	CourseTime ct = data->at(i);

	if(ct.m_timeslot != last_slot){
	  last_slot = ct.m_timeslot;
	  file << std::endl <<  dayToString(last_slot->m_day) << " " << last_slot->m_hour << ": "; 
	}

	file << ct.m_course->getCode() << " (" << ct.m_classroom->m_code << ")" << ", ";	
  }
}

#endif
