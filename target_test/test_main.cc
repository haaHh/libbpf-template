#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <time.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <pthread.h>
#include <stdlib.h>
// #include <sys/sdt.h>

#define CPU_NUM_MASK 20

std::atomic<uint64_t> total_calls(0);
std::mutex cout_mutex;

// void add_func() {

//     DTRACE_PROBE1(test_app, add_func_entry, 0);
//     struct timespec now;
//     clock_gettime(CLOCK_MONOTONIC, &now);
//     int a = 1;
//     int b = 2;
//     int c = a + b;
//     DTRACE_PROBE1(test_app, add_func_exit, 0);
//     struct timespec end;
//     clock_gettime(CLOCK_MONOTONIC, &end);
//     uint64_t elapsed_ns = (end.tv_sec - now.tv_sec) * 1000000000 + (end.tv_nsec - now.tv_nsec);
//     // std::cout << "add_func time: " << elapsed_ns << " ns" << std::endl;
// }

void add_func() {
    int a = 1;
    int b = 2;
    int c = a + b;

    // char *p = (char *)malloc(1024*1024*10);
    // free(p);
}

void set_thread_affinity(int cpu_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);
    
    pthread_t current_thread = pthread_self();
    int result = pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
    
    if (result != 0) {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cerr << "Failed to bind thread to CPU " << cpu_id << ", error: " << result << std::endl;
    } else {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "Successfully bound thread to CPU " << cpu_id << std::endl;
    }
}

void worker_thread(int thread_id, int iterations) {
    set_thread_affinity((CPU_NUM_MASK+thread_id) % sysconf(_SC_NPROCESSORS_ONLN));
    
    for(int i = 0; i < iterations; i++) {
        add_func();
    }

    // while(true) {
    //     for(int i = 0; i < 1000000; i++) {
    //         add_func();
    //     }
    //     // printf("thread %d, total calls: %d\n", thread_id, total_calls.load());
    //     sleep(1);
    // }

    total_calls.fetch_add(iterations);
}

void user_defined_hook_func() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    
    std::cout << "Number of system CPU cores: " << sysconf(_SC_NPROCESSORS_ONLN) << std::endl;

    const int NUM_THREADS = 4;               //4 threads
    const int ITERATIONS_PER_THREAD = 1000000;  //every thread run 1000000 times
    std::vector<std::thread> threads;
    
    for(int i = 0; i < NUM_THREADS; i++) {
        threads.emplace_back(worker_thread, i, ITERATIONS_PER_THREAD);
    }
    
    for(auto& t : threads) {
        if(t.joinable()) {
            t.join();
        }
    }

    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);
    uint64_t elapsed_ns = (end.tv_sec - now.tv_sec) * 1000000000 + (end.tv_nsec - now.tv_nsec);
    
    std::cout << "total calls: " << total_calls.load() << std::endl;
    std::cout << "total time: " << elapsed_ns << " ns" << std::endl;
    std::cout << "average time: " << (double)elapsed_ns / total_calls.load() << " ns" << std::endl;
}

int main() {
    // std::cout << "program start, waiting for 1 seconds to attach eBPF probe..." << std::endl;
    // sleep(1);

    std::cout << "start test..." << std::endl;
    user_defined_hook_func();
    
    return 0;
}

