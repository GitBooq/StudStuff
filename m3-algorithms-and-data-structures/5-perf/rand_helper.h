#pragma once

#include <algorithm>
#include <random>
#include <vector>

namespace rng {
template <std::integral T = int> T RandomNumber() {
  static std::mt19937 engine(std::random_device{}());
  static std::uniform_int_distribution<T> distr{std::numeric_limits<T>::min(),
                                                std::numeric_limits<T>::max()};
  return distr(engine);
}

template <std::integral T>
auto FillVector(std::vector<T> &vec, const auto &generator) {
  std::ranges::generate(vec, generator);
}

template <std::integral T = int>
std::vector<T> GenerateVector(size_t size, T lo = std::numeric_limits<T>::min(),
                              T hi = std::numeric_limits<T>::max()) {
  static std::mt19937 engine(std::random_device{}());
  std::uniform_int_distribution<T> distr{lo, hi};
  auto gen = [&distr] { return distr(engine); };

  std::vector<T> data(size);
  FillVector(data, gen);

  return data;
}

template <std::integral T = int>
std::vector<T> GenerateVector(size_t size, size_t seed,
                              T lo = std::numeric_limits<T>::min(),
                              T hi = std::numeric_limits<T>::max()) {
  static std::mt19937 engine(seed);
  std::uniform_int_distribution<T> distr{lo, hi};
  auto gen = [&distr] { return distr(engine); };

  std::vector<T> data(size);
  FillVector(data, gen);

  return data;
}
} // namespace rng