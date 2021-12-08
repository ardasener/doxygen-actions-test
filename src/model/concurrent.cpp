#ifndef INCLUDE_CONCURRENT_CPP_
#define INCLUDE_CONCURRENT_CPP_

#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include "course.cpp"

/**
 * A class that holds all the information regarding course concurrency.
 * Two courses are considered concurrent if they have been taken simultaniously
 * in past terms. And the number of students that took them together is the
 * concurrency value of these two courses.
 */
class Concurrent {

	private:

		/**
		 * Map that holds concatination of course codes of the two courses as keys.
		 * And the number of students that took them concurrently as values.
		 */
		std::unordered_map<std::string, int> * m_map;

		/**
		 * An alternative to the m_map member variable. This pointer holds a 2D matrix
		 * that is implemented in 1D. So given two courses, the concurrency value of
		 * them is in m_cuda_matrix[courseId1 * size + courseId2] (order does not
		 * matter). This is used specifically for CUDA.
		 */
		int * m_cuda_matrix;

		//! Is true if the m_cuda_matrix has been generated.
		bool m_matrix_generated;

	public:

		Concurrent(){
			m_map = new std::unordered_map<std::string,int>();
			m_matrix_generated = false;
		}

		/**
		 * Inserts the t_count for the given two course codes.
		 * The order of these codes do not matter.
		 */
		void insert(std::string t_code1, std::string t_code2, int t_count){
			if(t_code1 > t_code2){
				std::swap(t_code1,t_code2);
			}

			std::string key = t_code1 + "#" + t_code2;

			std::pair<std::string,int> * pair = new std::pair<std::string,int>(key,t_count);
			m_map->insert(*pair);
		}

		int get(std::string t_code1, std::string t_code2) const{

			if(t_code1 > t_code2){
				std::swap(t_code1, t_code2);
			}

			std::string key = t_code1 + "#" + t_code2;

			if(m_map->count(key) != 0){
				return m_map->at(key);
			}

			return 0;
		}

		//! Generates the m_cuda_matrix variable.
		void generateCudaMatrix(std::vector<Course> * courses){


			if(m_matrix_generated){
				return;
			}

			std::vector<Course> * courses_copy(courses);

			//Sort the courses by id
			std::sort(courses_copy->begin(), courses_copy->end(), Course::compare);


			int size = courses_copy->size();
			m_cuda_matrix = new int[size*size];

			for(int i=0; i<size; i++){
				for(int j=0; j<size; j++){
					int index = (i*size) + j;

					if(i == j){
						m_cuda_matrix[index] = 0;
					} else {
						m_cuda_matrix[index] = get(courses_copy->at(i).getCode(), courses_copy->at(j).getCode());
					}
				}
			}

			m_matrix_generated = true;

		}

		//! generateCudaMatrix() should be called once before calling this function
		int * getCudaMatrix(){
			if(!m_matrix_generated){
				throw "Matrix not initialized";
			}

			return m_cuda_matrix;
		}

};

#endif
