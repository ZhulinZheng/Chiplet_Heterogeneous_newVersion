# Project environment
# SIMULATOR_ROOT, defined by setup_env.sh
BENCHMARK_ROOT=$(SIMULATOR_ROOT)/benchmark/transformer

# Compiler environment of C/C++
CC=g++
CFLAGS=-Wall -Werror -std=c++11 -g -I$(SIMULATOR_ROOT)/interchiplet/includes 
INTERCHIPLET_C_LIB=$(SIMULATOR_ROOT)/interchiplet/lib/libinterchiplet_c.a

# C/C++ Source file
C_SRCS=transformer.cpp
C_OBJS=obj/transformer.o
C_TARGET=bin/transformer_c

MNSIM_SRCS=mnsim.cpp
MNSIM_OBJS=obj/mnsim.o
MNSIM_TARGET=bin/mnsim

# Compiler environment of CUDA
NVCC=nvcc
CUFLAGS=--compiler-options -Wall -g -G -I$(SIMULATOR_ROOT)/interchiplet/includes -I /home/zzl/json/include

# CUDA Source file
CUDA_SRCS=transformer.cu
CUDA_OBJS=cuobj/transformer.o
CUDA_TARGET=bin/transformer_cu

all: bin_dir obj_dir cuobj_dir C_target CUDA_target MNSIM_target

# C language target
C_target: $(C_OBJS)
	$(CC) $(C_OBJS) $(INTERCHIPLET_C_LIB) -o $(C_TARGET) -lpthread

# MNSIM language target
MNSIM_target: $(MNSIM_OBJS)
	$(CC) $(MNSIM_OBJS) $(INTERCHIPLET_C_LIB) -o $(MNSIM_TARGET)

# CUDA language target
CUDA_target: $(CUDA_OBJS)
	$(NVCC) -g -G -L$(SIMULATOR_ROOT)/gpgpu-sim/lib/$(GPGPUSIM_CONFIG) --cudart shared $(CUDA_OBJS) -o $(CUDA_TARGET)

# Rule for C object
obj/%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Rule for Cuda object
cuobj/%.o: %.cu
	$(NVCC) $(CUFLAGS) -c $< -o $@

# Directory for binary files.
bin_dir:
	mkdir -p bin

# Directory for object files for C.
obj_dir:
	mkdir -p obj

# Directory for object files for CUDA.
cuobj_dir:
	mkdir -p cuobj

run:
	./run.sh
# Clean generated files.
clean:
	./clean.sh
	rm -rf bench.txt delayInfo.txt buffer* message_record.txt
	rm -rf proc_r*_t* *.log
	rm -rf  libcaffe* *.txt

clean_all: clean
	rm -rf *.json *.log
	rm -rf _cuobj* obj cuobj bin

kill:
	pkill -9 python
	pkill -9 transformer
	pkill -9 sniper
