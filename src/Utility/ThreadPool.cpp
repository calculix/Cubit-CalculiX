#include "ThreadPool.hpp"

ThreadPool::ThreadPool()
{}

ThreadPool::~ThreadPool()
{}

void ThreadPool::start(int number_of_threads) {
  for (size_t i = 0; i < number_of_threads; ++i) 
  {
    threads.emplace_back(std::thread(&ThreadPool::threadLoop,this));
  }
}

void ThreadPool::threadLoop() {
  while (true) {
    std::function<void()> job;
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      mutex_condition.wait(lock, [this] {
          return !jobs.empty() || should_terminate;
      });
      if (should_terminate) {
          return;
      }
      job = jobs.front();
      jobs.pop();
    }
    job();
  }
}

void ThreadPool::queueJob(const std::function<void()>& job) 
{
  {
      std::unique_lock<std::mutex> lock(queue_mutex);
      jobs.push(job);
  }
  mutex_condition.notify_one();
}

bool ThreadPool::busy() 
{
  bool poolbusy;
  {
    std::unique_lock<std::mutex> lock(queue_mutex);
    poolbusy = !jobs.empty();
  }
  return poolbusy;
}

void ThreadPool::stop() 
{
  {
    std::unique_lock<std::mutex> lock(queue_mutex);
    should_terminate = true;
  }
  mutex_condition.notify_all();
  for (std::thread& active_thread : threads) 
  {
    active_thread.join();
  }
  threads.clear();
}