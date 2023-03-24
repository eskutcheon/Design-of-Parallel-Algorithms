
fluidomp: fluid.cc
	g++ -O3 -o fluidomp fluid.cc -fopenmp -lpthread

fluid: fluid.cc
	g++ -O3 -pg -g -o fluid fluid.cc 

fluidmpi: fluidmpi.cc
	mpiCC -O3 -o fluidmpi fluidmpi.cc -fopenmp -lpthread

clean:
	rm -f fluidomp fluid *~ fke.dat
