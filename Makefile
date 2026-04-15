CXXFLAGS := -std=gnu++17 -O2 

IVFFLAT  := cluster/ivfflat
IVFPQ    := cluster/ivfpq

INCLUDE  := -I$(IVFFLAT)/include -I$(IVFPQ)/include -Icluster/utils -Iutils/include

all: 
	g++ $(CXXFLAGS) -fopenmp $(INCLUDE)  $^ search.cpp -o search	

run_flat:
	./search -d input/input.dat -q input/query.dat \
	-kclusters 100 -nprobe 5 -o output/flat_out \
	-N 1 -R 10 -type mnist -range false -ivfflat -seed 1627

run_flat2:
	./search -q input/query.dat -d input/input.dat \
	-kclusters 100 -o output/flat_out -nprobe 5 \
	-seed 1627 -N 1 -R 10 -range false -type mnist -ivfflat 
run_pq:
	./search -d input/input.dat -q input/query.dat \
	-kclusters 100 -nprobe 10 -M 16 -o output/pq_out \
	-N 1 -R 10 -type mnist -nbits 8 -range false -ivfpq -seed 1 

clean:
	rm search
