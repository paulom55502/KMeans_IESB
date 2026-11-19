CXX = g++
MPICXX = mpic++
CXXFLAGS = -O2 -std=c++17 -Wall -Wextra
OMPFLAGS = -fopenmp

all: sequencial openmp mpi

sequencial:
	$(CXX) $(CXXFLAGS) sequencial/kmeans.cpp -o sequencial/kmeans

openmp:
	$(CXX) $(CXXFLAGS) $(OMPFLAGS) openmp/kmeans_openmp.cpp -o openmp/kmeans_openmp

mpi:
	$(MPICXX) $(CXXFLAGS) mpi/kmeans_mpi.cpp -o mpi/kmeans_mpi

clean:
	rm -f sequencial/kmeans openmp/kmeans_openmp mpi/kmeans_mpi

.PHONY: all clean sequencial openmp mpi
