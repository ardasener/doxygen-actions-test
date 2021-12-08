std:
	g++ src/main.cpp -fopenmp -O3 -std=c++11 

std-gdb:
	g++ src/main.cpp -fopenmp -O3 -std=c++11 -g

std-valgrind:
	g++ src/main.cpp -fopenmp -O3 -std=c++11 -pg

cuda: 
	nvcc src/main.cpp src/algorithm/fitness.cu -Xcompiler -fopenmp -Xcompiler -O3 -Xcompiler -DCUDA -std=c++11 

cuda-gdb: 
	nvcc src/main.cpp src/algorithm/fitness.cu -Xcompiler -fopenmp -Xcompiler -O3 -Xcompiler -DCUDA -Xcompiler -std=c++11 -g -G
