#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <cassert>
#include <cstdio>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <stdexcept>
#include "apis_c.h"
#include "../../interchiplet/includes/pipe_comm.h"

using namespace std;
#define CIM_ROW  16
#define CIM_COL  4
#define MAX_NUM  64

InterChiplet::PipeComm global_pipe_comm;
/**
 * compugte the output of cim model by chiplet idX and idY
 */

void cim_model(
    int idX,
    int idY,
    const int8_t *input,  //input: 512*TEST_NUM
    const int8_t *weight, //weight: 256*512
    int8_t  *output, //output: 256*TEST_NUM
    uint8_t shift_num 
){
    int num = idX * 2 + idY;
    assert(num<MAX_NUM);
    int32_t golden_out_data_full[256*MAX_NUM];
    for (int  row = 0; row < 16*CIM_ROW; row++) {
        uint32_t index = num * 16 * CIM_ROW + row;
        golden_out_data_full[index] = 0;  
        for ( int i = 0; i < CIM_COL; i++) {
            for ( int j = 0; j < 128; j ++) {
                golden_out_data_full[index] += weight[row * 128 * CIM_COL + 128*i + j] * input[num*CIM_COL*128 + i * 128 + j]; 
            }
        }
    }

    int32_t golden_out_data_shift[256*MAX_NUM];
    for (int row = 0; row < 16*CIM_ROW; row++) {
        uint32_t index = num * 16 * CIM_ROW + row;
        golden_out_data_shift[index] = golden_out_data_full[index] >> shift_num;// 
        if(golden_out_data_shift[index] > 127) {
            output[index]  = 127;
        }else if(golden_out_data_shift[index] < -128){
            output[index]  = -128;
        }else {
            output[index]  = golden_out_data_shift[index] ;
        }
    }
}

double sum_csv_column(const string& filename, int column_index) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("can't open file: " + filename);
    }

    string line;
    double sum = 0.0;
    bool is_first_line = true;

    while (getline(file, line)) {
        // jump empty line
        if (line.empty()) continue;

        stringstream ss(line);
        string cell;
        vector<string> cells;
        
        while (getline(ss, cell, ',')) {
            cells.push_back(cell);
        }

        // jump firs line
        if (is_first_line) {
            is_first_line = false;
            continue;
        }


        try {
            sum += stod(cells[column_index]);
        } catch (const invalid_argument& e) {
            cerr << "warnning: can't translate '" << cells[column_index] << "' to number" << endl;
        }
    }

    return sum;
}

int main(int argc, char** argv) {
    // get chiplet id

    int idX = atoi(argv[1]);
    int idY = atoi(argv[2]);
    uint8_t test_num = 4;
    uint8_t shift_num = 10; 

    int8_t *input  = (int8_t*)malloc(512*test_num);
    int8_t *weight = (int8_t*)malloc(256*512);
    int8_t *output = (int8_t*)malloc(256*test_num);

    memset(output, 0, sizeof(int8_t) * 256*test_num);

    long long unsigned int timeNow = 1;
    std::string fileName = InterChiplet::receiveSync(1, 2, idX, idY);
    global_pipe_comm.read_data(fileName.c_str(), input, sizeof(int8_t) * 512*test_num);
    long long int time_end = InterChiplet::readSync(timeNow, 1, 2, idX, idY, sizeof(int8_t) * 512*test_num, 0);

    fileName = InterChiplet::receiveSync(1, 2, idX, idY);
    global_pipe_comm.read_data(fileName.c_str(), weight, sizeof(int8_t) * 256*512);
    time_end = InterChiplet::readSync(time_end, 1, 2, idX, idY, sizeof(int8_t) * 256*512, 0);

    // calculate
    cim_model(
        idX,
        idY,
        input,  
        weight, 
        output, 
        shift_num         
    );
    //system("pip3 install -r /home/zzl/ws2/master_v2/Chiplet_Heterogeneous_newVersion/scale-sim-v2/requirements.txt");
    system("cd /home/zzl/ws2/master_v2/Chiplet_Heterogeneous_newVersion/scale-sim-v2/scalesim;python3 scale.py -c ../configs/scale.cfg -t ../topologies/conv_nets/test.csv");

    time_end += sum_csv_column("/home/zzl/ws2/master_v2/Chiplet_Heterogeneous_newVersion/scale-sim-v2/test_runs/scale_example_run_32x32_os/COMPUTE_REPORT.csv", 1);
    fileName = InterChiplet::sendSync(idX, idY, 1, 2);
    global_pipe_comm.write_data(fileName.c_str(), output, sizeof(int8_t) * 256*test_num);
    std::cout << "simulator power of cost " << ":" << sizeof(int8_t) * 256*test_num*8*0.024 << "pJ" << std::endl;

    InterChiplet::writeSync(time_end + 512*test_num/512, idX, idY, 1, 2, sizeof(int8_t) * 256*test_num, 0);
    
    free(input);
    free(weight);
    free(output);
    return 0;
}
