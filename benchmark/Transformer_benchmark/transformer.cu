#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <vector>
#include <math.h>
#include <stdio.h>
#include <sys/time.h>
#include<cstdio>
#include <string>
#include <thread>  // For sleep_for
#include <chrono>  // For milliseconds

#include "apis_cu.h"
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

// 读取数据函数
std::vector<float> read_data(const std::string &filename){
    // 检查文件是否成功打开
    while (true) {
        std::ifstream file(filename);
        if (file) {
            std::cout << "文件存在: " << filename << std::endl;
            break;
        } 
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));  // 每次等待1秒
    }
    std::ifstream inFile(filename);
    std::vector<float> numbers; // 用于存储读取的数字
    std::string line;
    float number;

    // 逐行读取文件
    while (inFile >> number){
        numbers.push_back(number);
    }

    // 关闭文件
    inFile.close();
    return numbers;
}

// 写入文件的函数
void write_data(const std::string &filename, const float *numbers, int size){
    std::ofstream outFile(filename); // 创建并打开一个文件用于写入

    // 检查文件是否成功打开
    if (!outFile.is_open()) {
        std::cout << "无法打开文件用于写入" << std::endl;
    }

    // 遍历 vector，并将每个元素写入到文件的一行中
    for (int i=0; i<size;i++) {
        outFile << numbers[i] << std::endl;
    }

    outFile.close(); // 关闭文件
}

void delete_file(std::string file_path){
// 检查文件是否存在，然后删除
    if (std::remove(file_path.c_str()) == 0) {
        std::cout << "文件 " << file_path << " 已被删除。" << std::endl;
    } else {
        std::cout << "文件 " << file_path << " 不存在。" << std::endl;
    }
}

// CUDA调用的主函数
void process_module(int idY, float *M, float *M_out, int width, int height, int channels) {
    std::string input_filename, output_filename, python_script;

    // 根据idY选择对应的Python脚本和txt文件
    if (idY == 1) {
        python_script = "attention_module.py";
        input_filename = "../input_attention.txt";
        output_filename = "../output_attention.txt";

    }
    else if (idY == 2) {
        python_script = "droppath_module.py";
        input_filename = "../input_droppath.txt";
        output_filename = "../output_droppath.txt";

    }
    else if (idY == 3) {
        python_script = "normlayer_module.py";
        input_filename = "../input_norm.txt";
        output_filename = "../output_norm.txt";
    }
    else if (idY == 4) {
        python_script = "mlp_module.py";
        input_filename = "../input_mlp.txt";
        output_filename = "../output_mlp.txt";

    } else {
        std::cout << "Invalid idY value!" << std::endl;
        return;
    }
    int size= width*height*channels;
    // 准备输入数据
    write_data(input_filename,M,size);
    std::cout<<"#############################################################写数据完成#############################################################"<<std::endl;
    // 读取输出结果
    std::vector<float> output_vector = read_data(output_filename);
    std::copy(output_vector.begin(), output_vector.end(), M_out);
    std::cout<<"#############################################################读数据完成#############################################################"<<std::endl;
    delete_file(output_filename);
}

int main(int argc, char** argv) {
    int idX = atoi(argv[1]);
    int idY = atoi(argv[2]);
    int batch = 2;
    int embed_dim = 128;
    int batch_size = 1;
    int n_dim = 50;

    int width = n_dim;
    int height = batch_size;
    int channels = embed_dim;
    int out_width = width;
    int out_height = height;
    int out_channels = channels;
    int size = width*height*channels;

    // 分配内存
    float *M, *M_out;
    float *M_host=new float [size];
    float *M_out_host = new float [size];
    cudaMalloc((void **)&M, sizeof(float) * width * height * channels);
    cudaMalloc((void **)&M_out, sizeof(float) * out_width * out_height * out_channels);

    for (int i = 0; i < batch; i++) {
        // 使用CUDA接口接收消息
        receiveMessage(idX, idY, 0, 0, M, width * height * channels * sizeof(float));
        cudaMemcpy(M_host, M, width*height*channels * sizeof(float), cudaMemcpyDeviceToHost);
        std::cout<<M_host[width * height * channels-1]<<std::endl;
        std::cout<<"#############################################################test_1#############################################################"<<std::endl;
        
        // 调用处理模块进行推理
        process_module(idY, M_host, M_out_host, width, height, channels);
        cudaMemcpy(M_out, M_out_host, width*height*channels * sizeof(float), cudaMemcpyHostToDevice);

        // 使用CUDA接口发送处理后的消息
        sendMessage(0, 0, idX, idY, M_out, out_width * out_height * out_channels * sizeof(float));
    }

    // 释放内存
    cudaFree(M);
    cudaFree(M_out);

    return 0;
}

