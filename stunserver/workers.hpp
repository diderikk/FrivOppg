#include <iostream>
#include <mutex>
#include <thread>
#include <list>
#include <atomic>
#include <vector>
#include <chrono>
#include <functional>
#include <condition_variable>
#pragma once

using namespace std;
// g++ main_sleep.cpp -lpthread -o main_sleep

// Container for function arguments
struct function_info{
    function<void(char *, sockaddr_in, int)> task;
    int n; 
    struct sockaddr_in client_addr;
    char *buffer;
};

class Workers
{
    int thread_size;
    list<function_info> tasks;
    vector<thread> threads;
    mutex worker_mutex;
    condition_variable worker_cv;
    atomic<bool> stop_var{false};
    // Begins all threads
    void create_threads(int)
    {
        for (int i = 0; i < thread_size; i++)
        {
            threads.emplace_back([this] {
                thread_task();
            });
        }
    };
    void thread_task();

public:
    // Constructor
    Workers(int n) : thread_size(n){};

    // Starts threads
    void start()
    {
        create_threads(thread_size);
    };
    // Posts tasks
    void post(function<void(char *, sockaddr_in, int)> task, int n, struct sockaddr_in client_addr, char *buffer)
    {
        function_info func{task, n, client_addr, buffer};
        {
            unique_lock<mutex> lock(worker_mutex);
            tasks.emplace_back(func);
        }
        worker_cv.notify_one();
    };
    //Stops all threads and joins them
    void stop()
    {
        stop_var.exchange(true);
        worker_cv.notify_all();
        for (auto &thread : threads)
        {
            thread.join();
        }
    }
};

void Workers::thread_task()
{
    while (true)
    {
        function_info task_info;
        {
            unique_lock<mutex> lock(worker_mutex);
            while (tasks.empty())
            {
                //Dersom tasks er tom og trådene skal avslutte returnerer vi
                if (stop_var)
                    return;
                //Tråden settes på ventelisten
                worker_cv.wait(lock);
            }
            task_info = *tasks.begin();
            tasks.pop_front();
        }
        if (task_info.task)
            task_info.task(task_info.buffer,task_info.client_addr, task_info.n);
    }
}
