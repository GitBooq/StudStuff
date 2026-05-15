#pragma once
#include <condition_variable>
#include <memory>
#include <mutex>

// head == tail means empty list
template <typename T> class ThreadsafeQueue final {
private:
  struct node {
    std::shared_ptr<T> data;
    std::unique_ptr<node> next;
  };
  std::mutex head_mutex_;
  std::unique_ptr<node> head_;
  std::mutex tail_mutex_;
  node *tail_;
  std::condition_variable data_cond_;

  node *GetTail();
  std::unique_ptr<node> PopHead();
  std::unique_lock<std::mutex> WaitForData();
  std::unique_ptr<node> WaitPopHead();
  std::unique_ptr<node> WaitPopHead(T &value);

  std::unique_ptr<node> TryPopHead();
  std::unique_ptr<node> TryPopHead(T &value);

public:
  ThreadsafeQueue() : head_(new node), tail_(head_.get()) {}
  ~ThreadsafeQueue() = default;
  ThreadsafeQueue(const ThreadsafeQueue &) = delete;
  ThreadsafeQueue &operator=(const ThreadsafeQueue &) = delete;
  ThreadsafeQueue(const ThreadsafeQueue &&) = delete;
  ThreadsafeQueue &operator=(const ThreadsafeQueue &&) = delete;

  std::shared_ptr<T> TryPop();
  bool TryPop(T &value);
  std::shared_ptr<T> WaitAndPop();
  void WaitAndPop(T &value);
  void Push(T new_value);
  bool Empty();
};

template <typename T>
typename ThreadsafeQueue<T>::node *ThreadsafeQueue<T>::GetTail() {
  std::lock_guard<std::mutex> tail_lock(tail_mutex_);
  return tail_;
}

template <typename T>
std::unique_ptr<typename ThreadsafeQueue<T>::node>
ThreadsafeQueue<T>::PopHead() {
  std::unique_ptr<node> old_head = std::move(head_);
  head_ = std::move(old_head->next);
  return old_head;
}

template <typename T>
std::unique_lock<std::mutex> ThreadsafeQueue<T>::WaitForData() {
  std::unique_lock<std::mutex> head_lock(head_mutex_);
  data_cond_.wait(head_lock, [&] { return head_.get() != GetTail(); });
  return head_lock;
}

template <typename T>
std::unique_ptr<typename ThreadsafeQueue<T>::node>
ThreadsafeQueue<T>::WaitPopHead() {
  std::unique_lock<std::mutex> head_lock(WaitForData());
  return PopHead();
}

template <typename T>
std::unique_ptr<typename ThreadsafeQueue<T>::node>
ThreadsafeQueue<T>::WaitPopHead(T &value) {
  std::unique_lock<std::mutex> head_lock(WaitForData());
  value = std::move(*head_->data);
  return PopHead();
}

template <typename T> void ThreadsafeQueue<T>::Push(T new_value) {
  std::shared_ptr<T> new_data(std::make_shared<T>(std::move(new_value)));
  std::unique_ptr<node> p(new node);
  {
    std::lock_guard<std::mutex> tail_lock(tail_mutex_);
    tail_->data = new_data;
    node *const new_tail = p.get();
    tail_->next = std::move(p);
    tail_ = new_tail;
  }
  data_cond_.notify_one();
}

template <typename T> std::shared_ptr<T> ThreadsafeQueue<T>::WaitAndPop() {
  std::unique_ptr<node> const old_head = WaitPopHead();
  return old_head->data;
}

template <typename T> void ThreadsafeQueue<T>::WaitAndPop(T &value) {
  std::unique_ptr<node> const old_head = WaitPopHead(value);
}

template <typename T>
std::unique_ptr<typename ThreadsafeQueue<T>::node>
ThreadsafeQueue<T>::TryPopHead() {
  std::lock_guard<std::mutex> head_lock(head_mutex_);
  if (head_.get() == GetTail()) {
    return std::unique_ptr<node>();
  }
  return PopHead();
}

template <typename T>
std::unique_ptr<typename ThreadsafeQueue<T>::node>
ThreadsafeQueue<T>::TryPopHead(T &value) {
  std::lock_guard<std::mutex> head_lock(head_mutex_);
  if (head_.get() == GetTail()) {
    return std::unique_ptr<node>();
  }
  value = std::move(*head_->data);
  return PopHead();
}

template <typename T> std::shared_ptr<T> ThreadsafeQueue<T>::TryPop() {
  std::unique_ptr<node> old_head = TryPopHead();
  return old_head ? old_head->data : std::shared_ptr<T>();
}

template <typename T> bool ThreadsafeQueue<T>::TryPop(T &value) {
  std::unique_ptr<node> const old_head = TryPopHead(value);
  return old_head != nullptr;
}

template <typename T> bool ThreadsafeQueue<T>::Empty() {
  std::lock_guard<std::mutex> head_lock(head_mutex_);
  return (head_.get() == GetTail());
}