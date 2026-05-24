// thread_pool.cc

#include "thread_pool.h"

namespace hwmod5 {
ThreadPool::ThreadPool(std::size_t workers) {
  if (0 == workers) {
    throw std::invalid_argument("There can't be 0 work threads");
  }

  try {
    for (std::size_t i = 0; i < workers; ++i) {
      threads_.emplace_back(
          [this](std::stop_token stop) { WorkerThread(stop); },
          stop_source_.get_token());
    }
  } catch (...) {
    stop_source_.request_stop();
    throw;
  }
}

ThreadPool::~ThreadPool() noexcept { Shutdown(); }

void ThreadPool::Shutdown() noexcept {
  stop_source_.request_stop();
  JoinAllThreads();
}

void ThreadPool::JoinAllThreads() noexcept {
  for (auto &t : threads_) {
    if (t.joinable()) {
      t.join();
    }
  }
}

void ThreadPool::WorkerThread(std::stop_token stop) {
  decltype(work_queue_)::value_type task;

  for (;;) {
    {
      const auto lock = monitor_.MakeLockWithWait(
          stop, [this] { return !work_queue_.empty(); });
      if (stop.stop_requested() && work_queue_.empty()) {
        return;
      }
      task = std::move(work_queue_.front());
      work_queue_.pop();
    }

    task();
  }
}
} // namespace hwmod5