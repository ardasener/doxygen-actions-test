#ifndef INCLUDE_FITNESS_CU_
#define INCLUDE_FITNESS_CU_

#include "schedule.cpp"
#include <vector>


//Penalties used to calculate fitness
__constant__ __device__ int CUDA_INSTRUCTOR_COLLISION_PENALTY = 1000;
__constant__ __device__ int CUDA_CAPACITY_PENALTY = 900;
__constant__ __device__ int CUDA_CLASSROOM_COLLISION_PENALTY = 850;
__constant__ __device__ int CUDA_CONCURRENCY_COLLISION_PENALTY_MAX = 700;
__constant__ __device__ int CUDA_ACADEMIC_BLOCK_COLLISION_PENALTY = 500;
__constant__ __device__ int CUDA_INSTRUCTOR_TOO_MANY_CLASSES_PENALTY_MAX = 600;
__constant__ __device__ int CUDA_INSTRUCTOR_TOO_MANY_CLASSES_PENALTY_MIN = 200;
__constant__ __device__ int CUDA_CONSECUTIVE_DAYS_PENALTY = 200;

/*

class CudaConcurrent {
  
};

class CudaTimeSlot {
public:
  int m_day;
  int m_hour;
  int m_id;

  CudaTimeSlot(){
	
  }

 __host__ CudaTimeSlot(TimeSlot* t_t){
	m_day = t_t->m_day;
	m_hour = t_t->m_hour;
	m_id = t_t->m_id;
  }
};

class CudaClassroom {
public:
  int m_capacity;

  CudaClassroom(){
	
  }

 __host__ CudaClassroom(Classroom *t_c){
	m_capacity = t_c->m_capacity;
  }
};

class CudaCourse {
public:
  int m_capacity;

  CudaCourse(){
	
  }

 __host__ CudaCourse(Course *t_c){
	m_capacity = t_c->getCapacity();
  }
};

class CudaCourseTime {
public:
  CudaCourse m_course; 
  CudaClassroom m_classroom;
  CudaTimeSlot m_timeslot;

  CudaCourseTime(){
	
  }

  __host__ CudaCourseTime(CourseTime *t_ct){
	m_course = *(new CudaCourse(t_ct->m_course));
	m_classroom = *(new CudaClassroom(t_ct->m_classroom));
	m_timeslot = *(new CudaTimeSlot(t_ct->m_timeslot));
  }
};


class CudaSchedule {
public:
  CudaCourseTime * m_coursetimes;
  int m_size;

  CudaSchedule(){
	
  }

  __host__ CudaSchedule(std::vector<CourseTime> * t_coursetimes){
	m_size = t_coursetimes->size();
	//std::cout << "Size:" << m_size << std::endl;
	CudaCourseTime * coursetimes = new CudaCourseTime[t_coursetimes->size()];
	cudaMalloc((void**) m_coursetimes, m_size*sizeof(CourseTime));

	for(int i=0; i < t_coursetimes->size(); i++){
	  coursetimes[0] = *(new CudaCourseTime(&(t_coursetimes->at(i))));
	}
	
	cudaMemcpy(m_coursetimes, coursetimes, m_size*sizeof(CourseTime), cudaMemcpyHostToDevice);  	
  }

  int getSize(){
	return m_size;
  }
};
*/

__global__ void calculateFitnessBulkCuda(CudaSchedule* schedules, int* result, int size) {

  int index = blockIdx.x * blockDim.x + threadIdx.x;

  if(index < 0 || index >= size){
	return;
  }

  int fitness = 0;

  CudaSchedule * schedule = &schedules[index];
  int schedule_size = schedule->m_size;

  for(int i=0; i<schedule_size; i++){
	CudaCourseTime * ct = &(schedule->m_coursetimes[i]);

	if(ct->m_course.m_capacity > ct->m_classroom.m_capacity){
	  fitness += CUDA_CAPACITY_PENALTY; 
	}
  }


  result[index] = schedule->m_coursetimes[0].m_course.m_capacity;
}


__host__ std::vector<int>* calculateFitnessBulk(std::vector<Schedule*>* schedules){

  int size = schedules->size();
  
  int* result_array;
  CudaSchedule* schedule_array = new CudaSchedule[size];

  for(int i=0; i<size; i++){
	std::vector<CourseTime> * coursetimes = schedules->at(i)->getCourseTimes();

	schedule_array[i] = *(new CudaSchedule(coursetimes));	
  }

  CudaSchedule* device_array;
  
  cudaMalloc((void**)&device_array, size*sizeof(CudaSchedule));
  cudaMalloc((void**)&result_array, size*sizeof(int));
  cudaMemcpy(device_array, schedule_array, size*sizeof(CudaSchedule), cudaMemcpyHostToDevice);  
  cudaMemset(result_array, 0, size*sizeof(int));
  
  calculateFitnessBulkCuda<<<1,256>>>(device_array, result_array, size);


  int * host_result = new int[size];
  cudaMemcpy(host_result, result_array, size*sizeof(int), cudaMemcpyDeviceToHost);
  std::vector<int> * result_vector = new std::vector<int>(host_result,host_result + size - 1);
	
  return result_vector;
}

#endif