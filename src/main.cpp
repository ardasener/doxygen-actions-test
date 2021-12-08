#include <fstream>
#include <iostream>
#include "load/load.cpp"
#include "model/course.cpp"
#include "load/json.hpp"
#include "model/classroom.cpp"
#include "model/concurrent.cpp"
#include "model/timeslot.cpp"
#include "algorithm/algorithm.cpp"
#include "util/config.cpp"
#include "util/args.hxx"
#include <omp.h>

// for convenience
using json = nlohmann::json;

bool file_exists(const std::string fileName)
{
	std::ifstream infile(fileName);
	return infile.good();
}

std::string getFileContents (std::ifstream& File)
{
	std::string Lines = "";        //All lines


	if (File)                      //Check if everything is good
	{
		while (File.good ())
		{
			std::string TempLine;                  //Temp line
			std::getline (File , TempLine);        //Get temp line
			TempLine += "\n";                      //Add newline character

			Lines += TempLine;                     //Add newline
		}
		return Lines;
	}
	else                           //Return error
	{
		return "ERROR File does not exist.";
	}
}



const std::string PATH_SEP =
#ifdef _WIN32
"\\";
#else
"/";
#endif

void printAscii(){
	std::string ascii_path = "." + PATH_SEP + "res" + PATH_SEP + "ascii.txt";

	std::ifstream file(ascii_path);
	std::cout << getFileContents(file) << std::endl;
}


int main(int argc, char **argv) {

	printAscii();

	args::ArgumentParser parser("A genetic algorithm based course scheduling program.");
	args::HelpFlag help(parser,"help","Display this menu", {"h","help"});

	args::ValueFlag<std::string> data_directory(parser, "path", "Path to a directory with the 3 required files in it (courses.json, classrooms.json, concurrent.json)", {"data-directory","dd"}, "");

	args::ValueFlag<std::string> config_file(parser, "path", "Configuration file", {"config-file", "cf"}, "");

	args::Flag gen_config(parser,"gen-config","Generates a default config", {"gen-config", "gc"});

	try{
		parser.ParseCLI(argc,argv);
	}
	catch (args::Help)
	{
		std::cout << parser;
		return 0;
	}
	catch (args::ParseError e)
	{
		std::cerr << e.what() << std::endl;
		std::cerr << parser;
		return 1;
	}
	catch (args::ValidationError e)
	{
		std::cerr << e.what() << std::endl;
		std::cerr << parser;
		return 1;
	}

	if(gen_config.Get()){

		std::string config_path = "." + PATH_SEP + "res" + PATH_SEP + "defaults.toml";

		if(file_exists(config_path)){
			std::ifstream input(config_path);
			std::ofstream output("config.toml");
			output << input.rdbuf();
		}

		std::cout << "Generated config.toml" << std::endl;
		return 1;
	}

	if(config_file.Get().empty()){
		std::cerr << "A config file must be provided with the --cf=<path> argument. Use --gc to generate the default one." << std::endl;
		return 1;
	}


	if(data_directory.Get().empty()){
		std::cerr << "You must enter a data directory with the --dd=<path> argument." << std::endl;
		return 1;
	}

	std::string course_file = data_directory.Get() + PATH_SEP + "courses.json";
	std::string classroom_file = data_directory.Get() + PATH_SEP + "classrooms.json";
	std::string concurrent_file = data_directory.Get() + PATH_SEP + "concurrent.json";

	if(!file_exists(course_file) || !file_exists(classroom_file) || !file_exists(concurrent_file)){
		std::cerr << "One (or more) of the required files (courses.json, classrooms.json, concurrent.json) is missing in the data directory." << std::endl;
		return 1;
	}


	std::vector<Course> * courses = load_courses(course_file);
	std::vector<Classroom> * classrooms = load_classroom(classroom_file);
	Concurrent * concurrent = load_concurrent(concurrent_file);
	std::vector<TimeSlot> * timeslots = generate_time_slots();


	Config config(config_file.Get(),courses,classrooms,timeslots,concurrent);

	omp_set_num_threads(config.num_threads);


	try{
		Algorithm * algorithm = new Algorithm(config);
		algorithm->run(config.num_iterations);

	} catch (const char *e){
		std::cout << "Error: " << e << std::endl;
	}
}
