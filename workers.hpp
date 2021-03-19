#include <iostream>
#include <mutex>
#include <thread>
#include <list>
#include <atomic>
#include <vector>
#include <chrono>
#include <functional>
#include <condition_variable>

using namespace std;
// g++ main_sleep.cpp -lpthread -o main_sleep
class Workers
{
    int thread_size;
    list<function<void()>> tasks;
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
    void post(function<void()> task)
    {
        {
            unique_lock<mutex> lock(worker_mutex);
            tasks.emplace_back(task);
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
        function<void()> task;
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
            task = *tasks.begin();
            tasks.pop_front();
        }
        if (task)
            task();
    }
}
