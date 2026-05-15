#pragma once
#include "function_wrapper.h"
#include "threadsafe_queue.h"
#include "work_stealing_queue.h"
#include <future>
#include <thread>
#include <vector>

class ThreadPool {
  using TaskType = FunctionWrapper;
  std::atomic_bool done_;
  ThreadsafeQueue<TaskType> pool_work_queue_;
  std::vector<std::unique_ptr<WorkStealingQueue>> queues_;
  std::vector<std::jthread> threads_;
  static inline thread_local WorkStealingQueue *local_work_queue_;
  static inline thread_local unsigned my_index_;

  void WorkerThread(unsigned my_index) {
    my_index_ = my_index;
    local_work_queue_ = queues_[my_index_].get();
    while (!done_) {
      run_pending_task();
    }
  }
  bool PopTaskFromLocalQueue(TaskType &task) {
    return local_work_queue_ && local_work_queue_->TryPop(task);
  }
  bool PopTaskFromPoolQueue(TaskType &task) {
    return pool_work_queue_.TryPop(task);
  }
  bool PopTaskFromOtherThreadQueue(TaskType &task) {
    for (unsigned i = 0; i < queues_.size(); ++i) {
      unsigned const index = (my_index_ + i + 1) % queues_.size();
      if (queues_[index]->TrySteal(task)) {
        return true;
      }
    }
    return false;
  }

public:
  ThreadPool() : done_(false) {
    unsigned const thread_count = std::thread::hardware_concurrency();
    try {
      for (unsigned i = 0; i < thread_count; ++i) {
        queues_.push_back(
            std::unique_ptr<WorkStealingQueue>(new WorkStealingQueue));
      }
      for (unsigned i = 0; i < thread_count; ++i) {
        threads_.push_back(std::jthread(&ThreadPool::WorkerThread, this, i));
      }
    } catch (...) {
      done_ = true;
      throw;
    }
  }
  ~ThreadPool() { done_ = true; }
  template <typename FunctionType>
  std::future<std::invoke_result_t<FunctionType>> Enqueue(FunctionType f) {
    using ResultType = std::invoke_result_t<FunctionType>;
    std::packaged_task<ResultType()> task(f);
    std::future<ResultType> res(task.get_future());
    if (local_work_queue_) {
      local_work_queue_->Push(std::move(task));
    } else {
      pool_work_queue_.Push(std::move(task));
    }
    return res;
  }
  void run_pending_task() {
    TaskType task;
    if (PopTaskFromLocalQueue(task) || PopTaskFromPoolQueue(task) ||
        PopTaskFromOtherThreadQueue(task)) {
      task();
    } else {
      std::this_thread::yield();
    }
  }
};