#pragma once

#include <concepts>
#include <functional>
#include <future>
#include <iterator>
#include <numeric>
#include <utility>
#include <vector>

template <typename Iterator, typename T> struct accumulate_block {
  T operator()(Iterator first, Iterator last) {
    return std::accumulate(first, last, T());
  }
};

template <typename P>
concept ThreadPoolConc = requires(P &pool, std::function<void()> task) {
  { pool.Enqueue(std::move(task)) } -> std::same_as<std::future<void>>;
};

template <typename Iterator, typename T, ThreadPoolConc TP>
T ParallelAccumulate(TP &pool, Iterator first, Iterator last, T init) {
  unsigned long const length = std::distance(first, last);
  if (!length)
    return init;
  unsigned long const block_size = 25;
  unsigned long const num_blocks = (length + block_size - 1) / block_size;
  std::vector<std::future<T>> futures(num_blocks - 1);
  Iterator block_start = first;
  for (unsigned long i = 0; i < (num_blocks - 1); ++i) {
    Iterator block_end = block_start;
    std::advance(block_end, block_size);
    futures[i] = pool.Enqueue([=] {
      return accumulate_block<Iterator, T>()(block_start, block_end);
    });
    block_start = block_end;
  }
  T last_result = accumulate_block<Iterator, T>()(block_start, last);
  T result = init;
  for (unsigned long i = 0; i < (num_blocks - 1); ++i) {
    result += futures[i].get();
  }
  result += last_result;
  return result;
}