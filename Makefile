CXXFLAGS := -std=gnu++17 -O2 

IVFFLAT  := cluster/ivfflat
IVFPQ    := cluster/ivfpq

INCLUDE  := -I$(IVFFLAT)/include -I$(IVFPQ)/include -Icluster/utils -Iutils/include
OUT_DIR  := output

all: 
	g++ $(CXXFLAGS) -fopenmp $(INCLUDE)  $^ search.cpp -o search	

run_flat: out_dir
	./search -d input/input.dat -q input/query.dat \
	-kclusters 100 -nprobe 5 -o $(OUT_DIR)/flat_out \
	-N 1 -R 10 -type mnist -range false -ivfflat -seed 1627

run_pq: out_dir
	./search -d input/input.dat -q input/query.dat \
	-kclusters 100 -nprobe 10 -M 16 -o $(OUT_DIR)/pq_out \
	-N 1 -R 10 -type mnist -nbits 8 -range false -ivfpq -seed 1 

out_dir:
	mkdir -p $(OUT_DIR)

clean:
	rm search
