#include "json.hpp"
#include <vector>
#include "../model/course.cpp"
#include "../model/classroom.cpp"
#include "../model/concurrent.cpp"
#include "../model/timeslot.cpp"
#include <string>
#include <fstream>
#include <iostream>

// for convenience
using json = nlohmann::json;

json file_to_json(std::string t_filename){
	std::ifstream input(t_filename);
	json js;
	input >> js;

	return js;
}


std::vector<Course>* load_courses(std::string t_filename){

	std::cout << "Loading courses..." << std::endl;

	std::vector<Course> * courses = new std::vector<Course>();

	auto js = file_to_json(t_filename);

	int id = 0;
	int inst_id = 0;

	std::unordered_map<std::string,int>* instructor_id_map = new std::unordered_map<std::string,int>();

	for(auto pos = js.begin(); pos < js.end(); pos++){
		auto curr = *pos;

		std::unordered_set<std::string> * instructors = new std::unordered_set<std::string>();
		std::unordered_set<int> * blocks = new std::unordered_set<int>();
		std::unordered_set<int> * instructor_ids = new std::unordered_set<int>();


		auto json_inst = curr["instructors"];
		for(unsigned int i=0; i<json_inst.size(); i++){
			std::string inst = json_inst[i];
			instructors->insert(inst);

			if(instructor_id_map->count(inst) == 0){
				instructor_id_map->insert(*(new std::pair<std::string,int>(inst,inst_id++)));
			}

			instructor_ids->insert(instructor_id_map->at(inst));
		}

		std::string code = curr["code"];
		std::string section = "";

		if(curr["section"] != nullptr)
			section = curr["section"];

		std::string full_code = code + " " + section;

		int duration1 = (int)curr["firstDuration"] ;
		int duration2 = (int)curr["secondDuration"];

		//std::cout << "Pushing: " << full_code << std::endl;

		Course * course = new Course(id++, code, curr["crn"],curr["capacity"], blocks, instructors,instructor_ids, duration1, duration2);
		courses->push_back(*course);
	}

	std::cout << "Finished loading courses." << std::endl;
	return courses;
}

std::vector<Classroom>* load_classroom(std::string t_filename){

	std::cout << "Loading classrooms..." << std::endl;

	std::vector<Classroom>* classrooms = new std::vector<Classroom>();

	auto js = file_to_json(t_filename);


	int id = 0;

	for(auto pos = js.begin(); pos < js.end(); pos++){
		auto curr = *pos;

		Classroom * classroom = new Classroom(++id,curr["code"],curr["capacity"]);
		classrooms->push_back(*classroom);
	}

	std::cout << "Finished loading classrooms." << std::endl;
	return classrooms;
}


Concurrent * load_concurrent(std::string t_filename){
	std::cout << "Loading concurrents..." << std::endl;

	Concurrent * concurrent = new Concurrent();

	auto js = file_to_json(t_filename);

	for(auto pos = js.begin(); pos < js.end(); pos++){
		auto curr = *pos;
		concurrent->insert(curr["courseCode1"],curr["courseCode2"],curr["studentCount"]);
	}

	std::cout << "Finished loading concurrents." << std::endl;
	return concurrent;

}

std::vector<TimeSlot> * generate_time_slots(){

	std::vector<TimeSlot> * timeslots = new std::vector<TimeSlot>();

	int id=0;

	for (int i=0; i <= TimeSlot::MAX_DAY; ++i) {
		for(int j=0; j <= TimeSlot::MAX_HOUR; ++j){
			TimeSlot * timeslot = new TimeSlot(++id,j,i);
			timeslots->push_back(*timeslot);
		}
	}

	return timeslots;

}
