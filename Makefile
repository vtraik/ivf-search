CXXFLAGS := -std=gnu++17 -O2 

IVFFLAT  := cluster/ivfflat
IVFPQ    := cluster/ivfpq

INCLUDE  := -I$(IVFFLAT)/include -I$(IVFPQ)/include -Icluster/utils -Iutils/include

all: 
	g++ $(CXXFLAGS) -fopenmp $(INCLUDE)  $^ search.cpp -o search	

run_flat:
	./search -d ~/SDAP/input/input.dat -q ~/SDAP/input/query.dat \
	-kclusters 1024 -nprobe 16 -o output/flat_out \
	-N 50 -R 500 -type mnist -range false -ivfflat -seed 1 

run_pq:
	./search -d ~/SDAP/input/input.dat -q ~/SDAP/input/query.dat \
	-kclusters 50 -nprobe 5 -M 16 -o output/pq_out \
	-N 1 -R 500 -type mnist -nbits 8 -range false -ivfpq -seed 1 

clean:
	rm search
