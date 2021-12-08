#ifndef INCLUDE_FITNESS_CU_
#define INCLUDE_FITNESS_CU_

#include "schedule.cpp"
#include <vector>
#include <algorithm>

#define CUDA_BLOCK_SIZE 256
#define MAX_ACADEMIC_BLOCKS 10
#define MAX_INSTS 5

//Penalties used to calculate fitness
struct Constants {
	int CUDA_SAME_DAY_PENALTY;
	int CUDA_INSTRUCTOR_COLLISION_PENALTY;
	int CUDA_CAPACITY_PENALTY;
	int CUDA_CLASSROOM_COLLISION_PENALTY;
	int CUDA_CONCURRENCY_COLLISION_PENALTY_MAX;
	int CUDA_ACADEMIC_BLOCK_COLLISION_PENALTY;
	int CUDA_INSTRUCTOR_TOO_MANY_CLASSES_PENALTY_MAX;
	int CUDA_INSTRUCTOR_TOO_MANY_CLASSES_PENALTY_MIN;
	int CUDA_CONSECUTIVE_DAYS_PENALTY;
	int CUDA_MAX_CONCURRENCY;
};

struct CudaCourseTime {
	int course_id;
	int group;
	int timeslot;
	int day;
	int room_capacity;
	int room_id;
	int course_capacity;
	int academic_blocks[MAX_ACADEMIC_BLOCKS];
	int instructors[MAX_INSTS];
	int academic_blocks_size;
	int instructors_size;
};



__global__ void calculateFitnessBulkCuda(CudaCourseTime* course_times, int* concurrency_matrix, int* result,
		int courses_size, int combined_size,
		int schedule_size, int number_of_schedules, Constants cons) {


	//Calculate indices

	int index = (blockIdx.x * blockDim.x) + threadIdx.x;

	if(index < 0 || index >= combined_size){
		return;
	}

	int schedule_index = index / schedule_size;

	if(schedule_index < 0 || schedule_index >= number_of_schedules){
		return;
	}

	int offset = schedule_index*schedule_size;


	//Calculate fitnesses
	unsigned int fitness = 0;


	CudaCourseTime* this_ct = &(course_times[index]);

	//Capacity check
	if(this_ct->room_capacity < this_ct->course_capacity){
		fitness += cons.CUDA_CAPACITY_PENALTY;
	}

	//All classes in same day check
	for(int i=index+1; i<offset+schedule_size && i<combined_size; i++){
		CudaCourseTime* other_ct = &(course_times[i]);

		if(other_ct->day != this_ct->day){
			break;
		}

		if(other_ct->course_id == this_ct->course_id && other_ct->group != this_ct->group){
			fitness += cons.CUDA_SAME_DAY_PENALTY;
		}

	}


	//Collision checks
	for(int i=index+1; i<offset + schedule_size && i < combined_size; i++){

		CudaCourseTime* other_ct = &(course_times[i]);

		if(this_ct->timeslot != other_ct->timeslot){
			//Since course times are sorted before this funtion begins
			//after this point all other coursetimes have non-colliding timeslots with this one.
			break;
		}

		//Room collision check
		if(this_ct->room_id == other_ct->room_id){
			fitness += cons.CUDA_CLASSROOM_COLLISION_PENALTY;
		}


		//Academic block collision check
		bool found = false;
		for(int j=0; !found && j < this_ct->academic_blocks_size && j < MAX_ACADEMIC_BLOCKS; j++){
			for(int k=0; !found && k< other_ct->academic_blocks_size && k < MAX_ACADEMIC_BLOCKS; k++){
				if(this_ct->academic_blocks[j] == other_ct->academic_blocks[k]){
					fitness += cons.CUDA_ACADEMIC_BLOCK_COLLISION_PENALTY;
					found = true;
				}
			}
		}


		//Instructor collision check
		found = false;
		for(int j=0; !found && j < this_ct->instructors_size && j < MAX_INSTS; j++){
			for(int k=0; !found &&  k< other_ct->instructors_size && k < MAX_INSTS; k++){
				if(this_ct->instructors[j] == other_ct->instructors[k]){
					fitness += cons.CUDA_INSTRUCTOR_COLLISION_PENALTY;
					found =  true;
				}
			}
		}

		//Concurrency collision check
		int concurrency_index = (this_ct->course_id * courses_size) + other_ct->course_id;
		int concurrent_penalty =  cons.CUDA_CONCURRENCY_COLLISION_PENALTY_MAX * ((float) concurrency_matrix[concurrency_index] / cons.CUDA_MAX_CONCURRENCY);
		if(concurrent_penalty > cons.CUDA_CONCURRENCY_COLLISION_PENALTY_MAX){
			concurrent_penalty = cons.CUDA_CONCURRENCY_COLLISION_PENALTY_MAX;
		}
		fitness += concurrent_penalty;


	}

	//Atomically write the results

	atomicAdd(&(result[schedule_index]), fitness);

}

__host__ void calculateFitnessBulk(std::vector<Schedule*>* schedules, int* concurrency_matrix_host, int courses_size, Config &config){

	cudaSetDevice(config.device_num);

	Constants cons;

	cons.CUDA_SAME_DAY_PENALTY = config.SAME_DAY_PENALTY;
	cons.CUDA_INSTRUCTOR_COLLISION_PENALTY = config.INSTRUCTOR_COLLISION_PENALTY;
	cons.CUDA_CAPACITY_PENALTY = config.CAPACITY_PENALTY;
	cons.CUDA_CLASSROOM_COLLISION_PENALTY = config.CLASSROOM_COLLISION_PENALTY;
	cons.CUDA_CONCURRENCY_COLLISION_PENALTY_MAX = config.CONCURRENCY_COLLISION_PENALTY_MAX;
	cons.CUDA_ACADEMIC_BLOCK_COLLISION_PENALTY = config.ACADEMIC_BLOCK_COLLISION_PENALTY;
	cons.CUDA_INSTRUCTOR_TOO_MANY_CLASSES_PENALTY_MAX = config.INSTRUCTOR_TOO_MANY_CLASSES_PENALTY_MAX;
	cons.CUDA_INSTRUCTOR_TOO_MANY_CLASSES_PENALTY_MIN = config.INSTRUCTOR_TOO_MANY_CLASSES_PENALTY_MIN;
	cons.CUDA_CONSECUTIVE_DAYS_PENALTY = config.CONSECUTIVE_DAYS_PENALTY;
	cons.CUDA_MAX_CONCURRENCY = config.MAX_CONCURRENCY;





	//SORT THE SCHEDULES (NECESSARY)
	//TODO: Try to perform this on the gpu also
	for(auto pos = schedules->begin(); pos != schedules->end(); pos++){
		Schedule * schedule = (*pos);
		std::sort(schedule->getCourseTimes()->begin(), schedule->getCourseTimes()->end(), CourseTime::compareByTime);
	}

	//CREATE HOST VARIABLES

	int schedule_size = schedules->at(0)->getSize();
	//schedule_size = 2000;

	int number_of_schedules = schedules->size();

	int combined_array_size = number_of_schedules*schedule_size;

	int* result_array_host = new int[number_of_schedules];

	CudaCourseTime* course_times_host = new CudaCourseTime[combined_array_size];

	for(int i=0; i< number_of_schedules; i++){
		for(int j=0; j<schedule_size; j++){
			int index = (i*schedule_size) + j;
			CourseTime * ct = &(schedules->at(i)->getCourseTimes()->at(j));

			//std::cout << "New Course Time" << std::endl;

			CudaCourseTime cuda_ct;

			cuda_ct.course_id = ct->m_course->getId();
			cuda_ct.timeslot = ct->m_timeslot->m_id;
			cuda_ct.room_capacity = ct->m_classroom->m_capacity;
			cuda_ct.room_id = ct->m_classroom->m_id;
			cuda_ct.course_capacity = ct->m_course->getCapacity();
			cuda_ct.academic_blocks_size = ct->m_course->getBlocks()->size();
			cuda_ct.instructors_size = ct->m_course->getInstructorIds()->size();
			cuda_ct.group = ct->getGroup();
			cuda_ct.day = ct->m_timeslot->m_day;

			int k = 0;
			for(auto pos = ct->m_course->getBlocks()->begin(); pos != ct->m_course->getBlocks()->end(); pos++){
				if(k >= MAX_ACADEMIC_BLOCKS){
					break;
				}
				cuda_ct.academic_blocks[k] = *pos;
				//std::cout << "Academic Block: " << *pos << std::endl;
			}

			k = 0;
			for(auto pos = ct->m_course->getInstructorIds()->begin(); pos != ct->m_course->getInstructorIds()->end(); pos++){
				if(k >= MAX_INSTS){
					break;
				}
				cuda_ct.instructors[k] = *pos;
			}

			course_times_host[index] = cuda_ct;

		}
	}


	//CALCULATE THREAD / BLOCK SIZE
	int block_size = CUDA_BLOCK_SIZE;
	int num_blocks = (combined_array_size / block_size) + 1;

	/*
		 std::cout << "Number of schedules: " << number_of_schedules << ", Schedule Size: " << schedule_size << ", Combined Size: " << combined_array_size << std::endl;
		 std::cout << "Block Size: " << block_size << ", Num. Blocks:" << num_blocks << std::endl;
	 */

	//CREATE DEVICE VARIABLES

	CudaCourseTime* course_times_device;
	int* concurrency_matrix_device;
	int* result_array_device;



	cudaMalloc((void **) &concurrency_matrix_device, sizeof(int)*courses_size*courses_size);
	cudaMalloc((void **) &course_times_device, sizeof(CudaCourseTime)*combined_array_size);
	cudaMalloc((void **) &result_array_device, sizeof(int)*number_of_schedules);


	//SET DEVICE VARIABLES

	cudaMemset(result_array_device,0,sizeof(int)*number_of_schedules);


	//COPY HOST VARIABLES TO DEVICE

	cudaMemcpy(course_times_device,course_times_host,sizeof(CudaCourseTime)*combined_array_size, cudaMemcpyHostToDevice);
	cudaMemcpy(concurrency_matrix_device, concurrency_matrix_host, sizeof(int)*courses_size*courses_size, cudaMemcpyHostToDevice);

	//RUN THE KERNEL
	calculateFitnessBulkCuda<<<num_blocks,block_size>>>(course_times_device, concurrency_matrix_device , result_array_device, courses_size, combined_array_size, schedule_size, number_of_schedules,cons);


	//COPY DEVICE VARIABLES TO HOST
	cudaDeviceSynchronize();
	cudaMemcpy(result_array_host, result_array_device, sizeof(int)*number_of_schedules, cudaMemcpyDeviceToHost);
	cudaDeviceSynchronize();

	//std::cout << "Cuda Fitnesses: ";
	for(int i=0; i<number_of_schedules; i++){
		schedules->at(i)->setFitness(result_array_host[i]);
		//std::cout << result_array_host[i] << ", ";
	}
	//std::cout << std::endl;



	//DELETE (FREE) HOST MEMORY

	delete[] course_times_host;
	delete[] result_array_host;


	//DELETE (FREE) DEVICE MEMORY

	cudaFree(course_times_device);
	cudaFree(result_array_device);
	cudaFree(concurrency_matrix_device);

}

#endif
