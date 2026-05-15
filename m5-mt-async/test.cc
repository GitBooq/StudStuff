#include "parallel_accumulate.h"
#include "thread_pool2.h"
#include <cassert>
#include <iostream>
#include <vector>

int main() {
  std::vector<long long> nums;
  for (auto i = 1LL; i <= 1'000'000; ++i) {
    nums.push_back(i);
  }
  // (1 + 1'000'000) * 1'000'000 / 2 = 500'000'500'000
  constexpr auto Sn = 500'000'500'000;

  ThreadPool pool;
  auto res = ParallelAccumulate(pool, nums.begin(), nums.end(), 0LL);

  std::cout << res << std::endl;
  assert(Sn == res);
}