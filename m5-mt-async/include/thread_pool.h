// thread_pool.h

#pragma once

#include <condition_variable>
#include <cstddef>
#include <future>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <stop_token>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace hwmod5 {
class Monitor {
public:
  struct UnlockAndNotify {
    std::mutex mutex_;
    std::condition_variable_any condition_;

    void lock() { mutex_.lock(); }
    void unlock() {
      mutex_.unlock();
      condition_.notify_one();
    }
  };

private:
  UnlockAndNotify combined_;

public:
  [[nodiscard]] std::unique_lock<UnlockAndNotify> MakeLockWithNotify() {
    return std::unique_lock{combined_};
  }

  template <std::predicate Pred>
  [[nodiscard]] std::unique_lock<std::mutex>
  MakeLockWithWait(Pred &&waitForCondition) {
    std::unique_lock lock{combined_.mutex_};
    combined_.condition_.wait(lock, std::forward<Pred>(waitForCondition));
    return lock;
  }

  template <std::predicate Pred>
  [[nodiscard]] std::unique_lock<std::mutex>
  MakeLockWithWait(std::stop_token stop, Pred &&waitForCondition) {
    std::unique_lock lock{combined_.mutex_};
    combined_.condition_.wait(lock, stop, std::forward<Pred>(waitForCondition));
    return lock;
  }
};

class ThreadPool final {
public:
  /**
   * @brief Construct a new Thread Pool object
   *
   * @param workers number of threads, should be > 0
   * @throw std::invalid_arguments if workers == 0
   */
  explicit ThreadPool(std::size_t workers);
  ~ThreadPool() noexcept;

  ThreadPool(const ThreadPool &) = delete;
  ThreadPool &operator=(const ThreadPool &) = delete;

  ThreadPool(ThreadPool &&) noexcept = delete;
  ThreadPool &operator=(ThreadPool &&) noexcept = delete;

  /**
   * @brief Puts task into threadpool queue
   *
   * @tparam F callable type
   * @tparam Args
   * @param f callable
   * @param args
   * @return std::future<std::invoke_result_t<F, Args...>>
   * @throw std::runtime_error if pool is already stopped
   */
  template <typename F, typename... Args>
    requires std::invocable<F, Args...>
  auto Enqueue(F &&f, Args &&...args)
      -> std::future<std::invoke_result_t<F, Args...>>;

  /**
   * @brief Stops the threadpool. Already enqueued tasks will be completed.
   *
   */
  void Shutdown() noexcept;

private:
  Monitor monitor_;
  std::stop_source stop_source_;
  std::queue<std::packaged_task<void()>> work_queue_;
  std::vector<std::jthread> threads_;

  void WorkerThread(std::stop_token stop);
};

template <class F, class... Args>
  requires std::invocable<F, Args...>
auto ThreadPool::Enqueue(F &&f, Args &&...args)
    -> std::future<std::invoke_result_t<F, Args...>> {
  if (stop_source_.stop_requested()) {
    throw std::runtime_error("Enqueue on stopped ThreadPool");
  }

  using ReturnType = std::invoke_result_t<F, Args...>;
  std::packaged_task<ReturnType()> task{
      [f = std::forward<F>(f), ... args = std::forward<Args>(args)] mutable {
        return f(args...);
      }};
  std::future<ReturnType> result = task.get_future();

  {
    const auto lock = monitor_.MakeLockWithNotify();
    work_queue_.push(std::packaged_task<void()>(
        [task = std::move(task)] mutable { task(); }));
  }

  return result;
}
} // namespace hwmod5