#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>

class ThreadPool {
public:
    ThreadPool();
    ~ThreadPool();
    void start(int number_of_threads);
    void queueJob(const std::function<void()>& job);
    void stop();
    bool busy();

private:
    void threadLoop();

    bool should_terminate = false;           // Tells threads to stop looking for jobs
    std::mutex queue_mutex;                  // Prevents data races to the job queue
    std::condition_variable mutex_condition; // Allows threads to wait on new jobs or termination 
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> jobs;
};

#endif // THREADPOOL_HPP