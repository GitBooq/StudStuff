# Thread Pool (`std::mutex` + `std::condition_variable`)

## Features

- Graceful shutdown
- Result obtainable with `std::future`
- Support any callables with variadic args
- Lock-based threadsafety
- **Non-movable and non-copyable**

## API
```cpp
class ThreadPool final {
public:
  explicit ThreadPool(std::size_t workers);

  template <typename F, typename... Args>
    requires std::invocable<F, Args...>
  auto Enqueue(F &&f, Args &&...args)
      -> std::future<std::invoke_result_t<F, Args...>>;

  void Shutdown() noexcept;
};
```

## Example
```cpp
#include ...

#include "thread_pool.h"

namespace {
auto RandomMilliseconds(int min_ms, int max_ms) {
  static thread_local std::random_device rd;
  static thread_local std::mt19937 gen(rd());
  std::uniform_int_distribution<> dist(min_ms, max_ms);
  return std::chrono::milliseconds(dist(gen));
}

void PrettyPrint(
    std::string_view message,
    const std::source_location &loc = std::source_location::current()) {
  std::osyncstream(std::cout)
      << std::format(" [Thread {}] {}: ", std::this_thread::get_id(),
                     loc.function_name())
      << message << std::endl;
}

void TaskA() {
  PrettyPrint("started");
  std::this_thread::sleep_for(RandomMilliseconds(100, 500));
  PrettyPrint("finished");
}
int TaskB(int val) {
  PrettyPrint("started");
  std::this_thread::sleep_for(RandomMilliseconds(100, 500));
  PrettyPrint("finished");
  return val;
}
void TaskC(int val1, std::string val2, double val3) {
  PrettyPrint("started");
  std::this_thread::sleep_for(RandomMilliseconds(100, 500));
  auto msg = std::format("{}, {}, {}", val1, val2, val3);
  PrettyPrint(msg);
  PrettyPrint("finished");
}
void TaskThrow() {
  std::this_thread::sleep_for(RandomMilliseconds(100, 500));
  PrettyPrint("throw!");
  throw std::runtime_error("test");
}
} // namespace

int main() {
  std::vector<long long> nums;
  for (auto i = 1LL; i <= 1'000'000; ++i) {
    nums.push_back(i);
  }
  // (1 + 1'000'000) * 1'000'000 / 2 = 500'000'500'000
  constexpr auto kSn = 500'000'500'000;
  long long sn_res{};

  auto threads = std::thread::hardware_concurrency();
  threads = threads != 0 ? threads : 2;

  std::cout << "Creating thread pool with " << threads << " threads"
            << std::endl;
  {
    // Create threadpool
    hwmod5::ThreadPool pool(threads);

    // Enqueue tasks
    for (int i = 0; i < 100; ++i) {
      pool.Enqueue(TaskA);
    }

    try {
      pool.Enqueue(TaskThrow);
    } catch (const std::runtime_error &e) {
      std::cout << e.what() << std::endl;
    }

    pool.Enqueue(TaskA);
    auto taskB_future = pool.Enqueue(TaskB, 42);
    pool.Enqueue(TaskC, 42, "string", .7);

    assert(taskB_future.get() == 42);

    sn_res = ParallelAccumulate(pool, nums.begin(), nums.end(), 0LL);

    // Shutting down the pool
    // All tasks that are already enqueued will be completed
    pool.Shutdown();
    try {
      // Trying to enqueue one more task leads to runtime_error
      pool.Enqueue(TaskA);
    } catch (const std::runtime_error &e) {
      std::cout << e.what() << std::endl;
    }
  }
  std::cout << "Thread pool destroyed" << std::endl;
  std::cout << std::format(
                   "ParallelAccumulate Result:\n\tExpected: {} Actual: {}", kSn,
                   sn_res)
            << std::endl;
}
```

## Build & Run
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/Main
```