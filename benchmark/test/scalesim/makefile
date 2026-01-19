# Project environment
# SIMULATOR_ROOT, defined by setup_env.sh
BENCHMARK_ROOT=$(SIMULATOR_ROOT)/benchmark/test/scalesim

# Compiler environment of C/C++
CC=g++
CFLAGS=-Wall -Werror -g -I$(SIMULATOR_ROOT)/interchiplet/includes
INTERCHIPLET_C_LIB=$(SIMULATOR_ROOT)/interchiplet/lib/libinterchiplet_c.a

# C/C++ Source file
C_SRCS=sniper.cpp
C_OBJS=obj/sniper.o
C_TARGET=bin/sniper_c

# NPU Source file
CUDA_SRCS=scalesim.cpp
CUDA_OBJS=cuobj/scalesim.o
CUDA_TARGET=bin/scalesim

all: bin_dir obj_dir cuobj_dir C_target CUDA_target

# C language target
C_target: $(C_OBJS)
	$(CC) $(C_OBJS) $(INTERCHIPLET_C_LIB) -o $(C_TARGET)

# CUDA language target
CUDA_target: $(CUDA_OBJS)
	$(CC) $(CUDA_OBJS) $(INTERCHIPLET_C_LIB) -o $(CUDA_TARGET)

# Rule for C object
obj/%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Rule for Cuda object
cuobj/%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Directory for binary files.
bin_dir:
	mkdir -p bin

# Directory for object files for C.
obj_dir:
	mkdir -p obj

# Directory for object files for CUDA.
cuobj_dir:
	mkdir -p cuobj

# Clean generated files.
clean:
	rm -rf bench.txt delayInfo.txt buffer* message_record.txt
	rm -rf proc_r*_t* *.log
	rm -rf obj cuobj bin
