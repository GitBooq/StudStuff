#include "rand_helper.h"
#include <benchmark/benchmark.h>
#include <list>
#include <random>
#include <ranges>
#include <set>
#include <unordered_set>

namespace {
constexpr auto kSeed = 1337;

template <typename Container>
[[maybe_unused]] void InsertSet(benchmark::State &state) {
  using T = Container::value_type;
  const int kDataSz = state.range(0);
  std::vector<T> data = rng::GenerateVector(kDataSz, kSeed); // fixed seed

  for (auto _ : state) {
    Container cont;
    for (int num : data) {
      cont.insert(num);
    }
    benchmark::DoNotOptimize(cont);
  }

  state.SetItemsProcessed(state.iterations() * kDataSz);
}

template <typename Container>
[[maybe_unused]] void FindSet(benchmark::State &state) {
  using T = Container::value_type;
  const int kDataSz = state.range(0);
  std::vector<T> data = rng::GenerateVector(kDataSz, kSeed); // fixed seed
  Container cont{data.begin(), data.end()};
  std::ranges::shuffle(data, std::random_device{});

  for (auto _ : state) {
    for (int num : data) {
      benchmark::DoNotOptimize(cont.find(num));
    }
    benchmark::DoNotOptimize(cont);
  }

  state.SetItemsProcessed(state.iterations() * kDataSz);
}

template <typename Container>
[[maybe_unused]] void EraseSet(benchmark::State &state) {
  using T = Container::value_type;
  const int kDataSz = state.range(0);
  std::vector<T> data = rng::GenerateVector(kDataSz, kSeed); // fixed seed
  Container cont{data.begin(), data.end()};
  auto iter = cont.begin();

  for (auto _ : state) {
    for (auto i = 0UZ; i < cont.size(); ++i) {
      iter = cont.erase(iter);
    }
    benchmark::DoNotOptimize(iter);
  }

  state.SetItemsProcessed(state.iterations() * kDataSz);
}

template <typename Container>
[[maybe_unused]] void InsertMap(benchmark::State &state) {
  using K = Container::key_type;
  using V = Container::mapped_type;
  const int kDataSz = state.range(0);
  std::vector<K> keys = rng::GenerateVector(kDataSz, kSeed); // fixed seed
  std::vector<V> vals = rng::GenerateVector(kDataSz, kSeed); // fixed seed

  for (auto _ : state) {
    Container cont;
    for (auto pair : std::views::zip(keys, vals)) {
      cont.insert(pair);
    }
    benchmark::DoNotOptimize(cont);
  }

  state.SetItemsProcessed(state.iterations() * kDataSz);
}

template <typename Container>
[[maybe_unused]] void FindMap(benchmark::State &state) {
  using K = Container::key_type;
  using V = Container::mapped_type;
  const int kDataSz = state.range(0);
  std::vector<K> keys = rng::GenerateVector(kDataSz, kSeed); // fixed seed
  std::vector<V> vals = rng::GenerateVector(kDataSz, kSeed); // fixed seed
  Container cont;
  for (auto pair : std::views::zip(keys, vals)) {
    cont.insert(pair);
  }

  for (auto _ : state) {
    for (auto key : keys) {
      benchmark::DoNotOptimize(cont.find(key));
    }
    benchmark::DoNotOptimize(cont);
  }

  state.SetItemsProcessed(state.iterations() * kDataSz);
}

template <typename Container>
[[maybe_unused]] void EraseMap(benchmark::State &state) {
  using K = Container::key_type;
  using V = Container::mapped_type;
  const int kDataSz = state.range(0);
  std::vector<K> keys = rng::GenerateVector(kDataSz, kSeed); // fixed seed
  std::vector<V> vals = rng::GenerateVector(kDataSz, kSeed); // fixed seed
  Container cont;
  for (auto pair : std::views::zip(keys, vals)) {
    cont.insert(pair);
  }

  auto iter = cont.begin();

  for (auto _ : state) {
    for (auto i = 0UZ; i < cont.size(); ++i) {
      iter = cont.erase(iter);
    }
    benchmark::DoNotOptimize(iter);
  }

  state.SetItemsProcessed(state.iterations() * kDataSz);
}

template <typename Container>
[[maybe_unused]] void InsertMidSeq(benchmark::State &state) {
  using T = Container::value_type;
  const int kDataSz = state.range(0);
  std::vector<T> data = rng::GenerateVector(kDataSz, kSeed); // fixed seed

  Container cont{data.begin(), data.end()};

  auto mid = cont.begin();
  std::advance(mid, kDataSz / 2);

  for (auto _ : state) {
    mid = cont.insert(mid, T());
    benchmark::DoNotOptimize(cont);
  }

  state.SetItemsProcessed(state.iterations());
}

template <typename Container>
[[maybe_unused]] void EraseEndSeq(benchmark::State &state) {
  using T = Container::value_type;
  const int kDataSz = state.range(0);
  std::vector<T> data = rng::GenerateVector(kDataSz, kSeed); // fixed seed

  Container cont{data.begin(), data.end()};

  for (auto _ : state) {
    cont.pop_back();
    benchmark::DoNotOptimize(cont);
  }

  state.SetItemsProcessed(state.iterations());
}

template <typename Container>
[[maybe_unused]] void EraseFrontSeq(benchmark::State &state) {
  using T = Container::value_type;
  const int kDataSz = state.range(0);
  std::vector<T> data = rng::GenerateVector(kDataSz, kSeed); // fixed seed

  Container cont{data.begin(), data.end()};

  for (auto _ : state) {
    cont.erase(cont.begin());
    benchmark::DoNotOptimize(cont);
  }

  state.SetItemsProcessed(state.iterations());
}

[[maybe_unused]] void FixedConfig(benchmark::Benchmark *b) { b->Arg(1 << 20); }
[[maybe_unused]] void FixedConfigOneIteration(benchmark::Benchmark *b) {
  b->Apply(FixedConfig)->Iterations(1);
}

using SetInt = std::set<int>;
using UnSetInt = std::unordered_set<int>;
using MapIntInt = std::map<int, int>;
using UnMapIntInt = std::unordered_map<int, int>;
using VecInt = std::vector<int>;
using ListInt = std::list<int>;

BENCHMARK(InsertSet<SetInt>)->Apply(FixedConfig);
BENCHMARK(InsertSet<UnSetInt>)->Apply(FixedConfig);
BENCHMARK(FindSet<SetInt>)->Apply(FixedConfig);
BENCHMARK(FindSet<UnSetInt>)->Apply(FixedConfig);
BENCHMARK(EraseSet<SetInt>)->Apply(FixedConfigOneIteration);
BENCHMARK(EraseSet<UnSetInt>)->Apply(FixedConfigOneIteration);

BENCHMARK(InsertMap<MapIntInt>)->Apply(FixedConfig);
BENCHMARK(InsertMap<UnMapIntInt>)->Apply(FixedConfig);
BENCHMARK(FindMap<MapIntInt>)->Apply(FixedConfig);
BENCHMARK(FindMap<UnMapIntInt>)->Apply(FixedConfig);
BENCHMARK(EraseMap<MapIntInt>)->Apply(FixedConfigOneIteration);
BENCHMARK(EraseMap<UnMapIntInt>)->Apply(FixedConfigOneIteration);

BENCHMARK(InsertMidSeq<VecInt>)->Apply(FixedConfig);
BENCHMARK(InsertMidSeq<ListInt>)->Apply(FixedConfig);
BENCHMARK(EraseEndSeq<VecInt>)->Apply(FixedConfigOneIteration);
BENCHMARK(EraseEndSeq<ListInt>)->Apply(FixedConfigOneIteration);
BENCHMARK(EraseFrontSeq<VecInt>)->Apply(FixedConfigOneIteration);
BENCHMARK(EraseFrontSeq<ListInt>)->Apply(FixedConfigOneIteration);
} // namespace

BENCHMARK_MAIN();
