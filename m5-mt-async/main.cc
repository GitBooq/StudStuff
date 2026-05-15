#include "parallel_accumulate.h"
#include "thread_pool.h"
#include <cassert>
#include <format>
#include <iostream>
#include <random>
#include <source_location>
#include <string_view>

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
  std::cout << std::format(" [Thread {}] {}: ", std::this_thread::get_id(),
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
    hwmod5::ThreadPool pool(threads);

    for (int i = 0; i < 100; ++i) {
      pool.Enqueue(TaskA);
    }

    try {
      pool.Enqueue(TaskThrow);
    } catch (const std::runtime_error &e) {
      std::cout << e.what() << std::endl;
    }

    pool.Enqueue(TaskA);
    auto taskBFuture = pool.Enqueue(TaskB, 42);
    pool.Enqueue(TaskC, 42, "string", .7);

    assert(taskBFuture.get() == 42);

    sn_res = ParallelAccumulate(pool, nums.begin(), nums.end(), 0LL);
  }
  std::cout << "Thread pool destroyed" << std::endl;
  std::cout << std::format("ParallelAccumulate Result:\n\tExpected: {} Actual: {}", kSn, sn_res) << std::endl;
}
