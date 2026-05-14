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
} // namespace

int main() {
  auto threads = std::thread::hardware_concurrency();
  threads = threads != 0 ? threads : 2;

  std::cout << "Creating thread pool with " << threads << " threads"
            << std::endl;
  {
    hwmod5::ThreadPool tp(threads);

    auto taskA = [] {
      PrettyPrint("started");
      std::this_thread::sleep_for(RandomMilliseconds(100, 500));
      PrettyPrint("finished");
    };
    auto taskB = [](int val) {
      PrettyPrint("started");
      std::this_thread::sleep_for(RandomMilliseconds(100, 500));
      PrettyPrint("finished");
      return val;
    };
    auto taskC = [](int val1, std::string val2, double val3) {
      PrettyPrint("started");
      std::this_thread::sleep_for(RandomMilliseconds(100, 500));
      auto msg = std::format("{}, {}, {}", val1, val2, val3);
      PrettyPrint(msg);
      PrettyPrint("finished");
    };

    for (int i = 0; i < 100; ++i) {
      tp.Enqueue(taskA);
    }

    tp.Enqueue(taskA);
    auto taskBFuture = tp.Enqueue(taskB, 42);
    tp.Enqueue(taskC, 42, "string", .7);

    assert(taskBFuture.get() == 42);
  }
  std::cout << "Thread pool destroyed" << std::endl;
}
