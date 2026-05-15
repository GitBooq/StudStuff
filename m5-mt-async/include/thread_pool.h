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

// Реализация Thread Pool на C++ (std::mutex + std::condition_variable)
/*
  Задача: реализовать ThreadPool, с корректным завершением потоков и безопасной
   передачей задач через очередь.

  Функциональные требования:
    1. Пул создаёт `N` worker-потоков при инициализации.
    2. Задачи хранятся в общей очереди (`std::queue<std::function<void()>>` или
  эквивалент).
    3. Worker-потоки ожидают задачи через `std::condition_variable`.
    4. После `Shutdown()`:
      - новые задачи не принимаются;
      - все уже добавленные задачи выполняются;
      - потоки корректно завершаются (`join`).
    5. `Enqueue` после остановки пула должен выбрасывать исключение
  (`std::runtime_error`).
*/

namespace hwmod5 {
// API
class ThreadPool final {
public:
  explicit ThreadPool(std::size_t workers);
  ~ThreadPool() noexcept;

  ThreadPool(const ThreadPool &) = delete;
  ThreadPool &operator=(const ThreadPool &) = delete;

  ThreadPool(ThreadPool &&) noexcept = delete;
  ThreadPool &operator=(ThreadPool &&) noexcept = delete;

  template <typename F, typename... Args>
  auto Enqueue(F &&f, Args &&...args)
      -> std::future<std::invoke_result_t<F, Args...>>;

  void Shutdown() noexcept;

private:
  std::stop_source stop_source_;
  std::queue<std::packaged_task<void()>> work_queue_;
  mutable std::mutex queue_mutex_;
  std::condition_variable_any cv_;
  std::vector<std::jthread> threads_;

  void worker_thread(std::stop_token st);
};

template <class F, class... Args>
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
    std::scoped_lock lock(queue_mutex_);
    work_queue_.push(std::packaged_task<void()>(
        [task = std::move(task)] mutable { task(); }));
  }

  cv_.notify_one();
  return result;
}
} // namespace hwmod5