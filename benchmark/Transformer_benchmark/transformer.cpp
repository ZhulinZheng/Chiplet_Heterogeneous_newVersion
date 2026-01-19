#include <fstream>
#include <iostream>
#include <stdio.h>
#include "apis_c.h"
#include <random>
#include <ctime>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>

using namespace std;

int idX, idY;
std::mutex mtx, mtxM1, mtxM2, mtxM3, mtx_send, mtx_read;
queue<float*> first_second, second_third, third_forth, answer;
std::condition_variable cv;
bool start_first = true;

queue<bool> read_first_done, read_sec_done, read_third_done, read_forth_done;
bool send_first_done = false;
bool send_sec_done = false;
bool send_third_done = false;
bool send_forth_done = false;

int batch = 2;



void send_first(int idX, int idY, float* M, size_t size) {
	for (int i = 0; i < batch; i++) {
		{
			std::unique_lock<std::mutex> lock(mtx);
			cv.wait(lock, []
				{ return !send_first_done && (send_sec_done || start_first); });
		}
        
		{
            std::unique_lock<std::mutex> lock(mtx_send);
            cout<<"向(0,1)发送数据"<<endl;
			InterChiplet::sendMessage(0, 1, idX, idY, M, size);
		}
		{
			std::unique_lock<std::mutex> lock(mtx);
			send_first_done = true;
		}
		cv.notify_all();
	}
}

void read_first(int idX, int idY, float* M_out_first, size_t size_out_first) {
    for (int i = 0; i < batch; i++) {
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, []
                { return send_first_done; });
        }
        
        
        {
            unique_lock<mutex> lock(mtx_read);
            cout<<"从(0,1)读取数据"<<endl;
            InterChiplet::receiveMessage(idX, idY, 0, 1, M_out_first, size_out_first);
        }
        
        {
            std::unique_lock<std::mutex> lock(mtxM1);
            first_second.push(M_out_first);
        }
        {
            std::lock_guard<std::mutex> lock(mtx);
            read_first_done.push(true);
            send_first_done = false;
            start_first = false;
        }
        cv.notify_all();
    }
}

void send_sec(int idX, int idY, float* M_out_first, size_t size_out_first) {
    for (int i = 0; i < batch; i++) {
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, []
                { return read_first_done.size() && !send_sec_done; });
            read_first_done.pop();
        }
        {
            M_out_first=first_second.front();
            std::unique_lock<std::mutex> lock(mtxM1);
            first_second.pop();
        }
        {
            cout<<"向(0,2)发送数据"<<endl;
            std::unique_lock<std::mutex> lock(mtx_send);
            InterChiplet::sendMessage(0, 2, idX, idY, M_out_first, size_out_first);
        }
        {
            std::unique_lock<std::mutex> lock(mtx);
            send_sec_done = true;
        }
        cv.notify_all();
    }
    
}

void read_sec(int idX, int idY, float* M_out_second, size_t size_out_second) {
    float *allow_receive=new float;
    for (int i = 0; i < batch; i++) {
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, []
                { return send_sec_done; });
        }
        {
            std::unique_lock<std::mutex> lock(mtx_read);
            cout<<"从(0,2)读取数据"<<endl;
            InterChiplet::receiveMessage(idX, idY, 0, 2, M_out_second, size_out_second);
        }
        
        {
            std::unique_lock<std::mutex> lock(mtxM2);
            second_third.push(M_out_second);
        }
        {
            std::lock_guard<std::mutex> lock(mtx);
            read_sec_done.push(true);
            send_sec_done = false;
        }
        cv.notify_all();
    }
    delete allow_receive;
    
}

void send_third(int idX, int idY, float* M_out_second, size_t size_out_second) {
    for (int i = 0; i < batch; i++) {
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, []
                { return read_sec_done.size() && !send_third_done; });
            read_sec_done.pop();
        }
        {
            std::unique_lock<std::mutex> lock(mtxM2);
            M_out_second=second_third.front();
            second_third.pop();
        }
        
        {
            std::unique_lock<std::mutex> lock(mtx_send);
            cout<<"向(0,3)发送数据"<<endl;
            InterChiplet::sendMessage(0, 3, idX, idY, M_out_second, size_out_second);
        }
        
        {
            std::unique_lock<std::mutex> lock(mtx);
            send_third_done = true;
            
        }
        cv.notify_all();
    }
}

void read_third(int idX, int idY, float* M_out_third, size_t size_out_third) {
    for (int i = 0; i < batch; i++) {
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, []
                { return send_third_done; });
        }
        
        {
            std::unique_lock<std::mutex> lock(mtx_read);
            cout<<"从(0,3)读取数据"<<endl;
            InterChiplet::receiveMessage(idX, idY, 0, 3, M_out_third, size_out_third);
        }
        
        {
            std::unique_lock<std::mutex> lock(mtxM3);
            third_forth.push(M_out_third);
        }
        
        {
            std::unique_lock<std::mutex> lock(mtx);
            send_third_done = false;
            read_third_done.push(true);
        }
        cv.notify_all();
    } 
}

void send_forth(int idX, int idY, float* M_out_third, size_t size_out_third) {
    for (int i = 0; i < batch; i++) {
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, []
                { return read_third_done.size() && !send_forth_done; });
            read_third_done.pop();
        }
        {
            std::unique_lock<std::mutex> lock(mtxM3);
            M_out_third=third_forth.front();
            third_forth.pop();
            cout<<"###############"<<endl;
        }
        
        {
            std::unique_lock<std::mutex> lock(mtx_send);
            cout<<"向(0,4)发送数据"<<endl;
            InterChiplet::sendMessage(0, 4, idX, idY, M_out_third, size_out_third);
        }
        {
            std::unique_lock<std::mutex> lock(mtx);
            send_forth_done = true;
        }
        cv.notify_all();
    }
}

void random_init(int64_t * data, int size){
    int i;
    for (i = 0; i < size; i++){
        //data[i] = rand();
        data[i] = 128;
    }
}

void read_forth(int idX, int idY, float* M_out_forth, size_t size_out_forth) {
    for (int i = 0; i < batch; i++) {
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, []
                { return send_forth_done; });
        }
        {
            unique_lock<mutex> lock(mtx_read);
            cout << "从(0,4)读取数据" << endl;
            InterChiplet::receiveMessage(idX, idY, 0, 4, M_out_forth, size_out_forth);
            // answer.push(M_out_forth);
        }
        {
            std::unique_lock<std::mutex> lock(mtx);
            send_forth_done = false;
            // read_forth_done.push(true);
        }
        cv.notify_all();
    } 
}

int main(int argc, char** argv) {
    idX = atoi(argv[1]);
    idY = atoi(argv[2]);
    // batch = atoi(argv[3]);

    int embed_dim = 128;
    int batch_size = 1;
    int n_dim = 50;
    int size=embed_dim*batch_size*n_dim;

    float* M = new float[size];
    float* M_out_first = new float[size];
    float* M_send_second = new float[size];
    float* M_out_second = new float[size];
    float* M_send_third = new float[size];
    float* M_out_third = new float[size];
    float* M_send_forth = new float[size];
    float* M_out_forth = new float[size];
    for (int i = 0; i < size; i++) {
        M[i] = (rand() % 255);
    }

    int64_t *test = new int64_t[112*112*64];
    int64_t *test_ans = new int64_t[112*112*128];
    random_init(test, 112*112*64);
    InterChiplet::sendMessage(0, 5, idX, idY, test, 112*112*64*sizeof(int64_t));
    InterChiplet::receiveMessage(idX, idY, 0, 5, test_ans, 112*112*128*sizeof(int64_t));
    delete[] test;
    delete[] test_ans;
    std::cout<<"-------------------------------------mnsim over-------------------------------------"<<std::endl;
    std::thread send_first_thread(send_first, idX, idY, M, size* sizeof(float));
    std::thread read_first_thread(read_first, idX, idY, M_out_first, size* sizeof(float));
    std::thread send_sec_thread(send_sec, idX, idY, M_send_second, size* sizeof(float));
    std::thread read_sec_thread(read_sec, idX, idY, M_out_second, size* sizeof(float));
    std::thread send_third_thread(send_third, idX, idY, M_send_third, size* sizeof(float));
    std::thread read_third_thread(read_third, idX, idY, M_out_third, size* sizeof(float));
    std::thread send_forth_thread(send_forth, idX, idY, M_send_forth, size* sizeof(float));
    std::thread read_forth_thread(read_forth, idX, idY, M_out_forth, size* sizeof(float));

    // 等待所有线程执行完毕
    send_first_thread.join();
    read_first_thread.join();
    send_sec_thread.join();
    read_sec_thread.join();
    send_third_thread.join();
    read_third_thread.join();
    send_forth_thread.join();
    read_forth_thread.join();

    delete[] M;
    delete[] M_out_first;
    delete[] M_send_second;
    delete[] M_out_second;
    delete[] M_send_third;
    delete[] M_out_third;
    delete[] M_send_forth;
    delete[] M_out_forth;

    return 0;
}
