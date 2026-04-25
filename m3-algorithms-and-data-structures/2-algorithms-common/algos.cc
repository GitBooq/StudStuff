#include <algorithm>
#include <functional>
#include <iostream>
#include <random>
#include <ranges>
#include <vector>

namespace ranges = std::ranges;

void FillVector(std::vector<int> &vec, const auto &generator) {
  ranges::generate(vec, generator);
}

bool IsPrime(int num) {
  if (num < 2) {
    return false;
  }
  for (auto j = 2; j * j <= num; ++j) {
    if ((num % j) == 0) {
      return false;
    }
  }
  return true;
}

int main() {
  // 1. Generate vector of 100 random integers in range [1,1000]
  constexpr int size = 100;
  constexpr int lo = 1;
  constexpr int hi = 1000;
  static std::mt19937 engine(std::random_device{}());
  std::uniform_int_distribution<int> distr{lo, hi};
  auto gen = [&distr] { return distr(engine); };

  std::vector<int> vec(size);
  FillVector(vec, gen);

  // 2. Sort in non increasing order
  ranges::sort(vec, ranges::greater{});

  // 3. Filter primes
  [[maybe_unused]] auto primes = vec | std::views::filter(IsPrime);
#if 0
    std::ostream_iterator<int> os{std::cout, " "};
    std::ranges::copy(primes, os);
#endif

  // 4. Remove duplicates from sorted vector
  auto last = std::unique(vec.begin(), vec.end());
  vec.erase(last, vec.end());

  // 5. Calc sum of squares
  int sum_of_squares =
      std::transform_reduce(vec.begin(), vec.end(), 0, std::plus{},
                            [](int num) { return num * num; });
  std::cout << "Sum of squares: " << sum_of_squares << std::endl;
}