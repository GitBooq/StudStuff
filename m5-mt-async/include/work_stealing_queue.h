#pragma once
#include <deque>
#include <mutex>
#include <utility>

#include "function_wrapper.h"

class WorkStealingQueue {
private:
  typedef FunctionWrapper data_type;
  std::deque<data_type> the_queue;
  mutable std::mutex the_mutex;

public:
  WorkStealingQueue() {}
  WorkStealingQueue(const WorkStealingQueue &other) = delete;
  WorkStealingQueue &operator=(const WorkStealingQueue &other) = delete;
  void Push(data_type data) {
    std::lock_guard<std::mutex> lock(the_mutex);
    the_queue.push_front(std::move(data));
  }
  bool Empty() const {
    std::lock_guard<std::mutex> lock(the_mutex);
    return the_queue.empty();
  }
  bool TryPop(data_type &res) {
    std::lock_guard<std::mutex> lock(the_mutex);
    if (the_queue.empty()) {
      return false;
    }
    res = std::move(the_queue.front());
    the_queue.pop_front();
    return true;
  }
  bool TrySteal(data_type &res) {
    std::lock_guard<std::mutex> lock(the_mutex);
    if (the_queue.empty()) {
      return false;
    }
    res = std::move(the_queue.back());
    the_queue.pop_back();
    return true;
  }
};