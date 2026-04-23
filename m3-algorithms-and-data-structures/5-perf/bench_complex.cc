#if 0
#include <benchmark/benchmark.h>
#include <list>
#include <map>
#include <random>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define SET_ENABLED_ 1
#define MAP_ENABLED_ 1
#define SEQ_ENABLED_ 1

namespace {
[[maybe_unused]] auto GetMaxCacheSize() {
  using CPUInfo = benchmark::CPUInfo;
  static auto caches = CPUInfo::Get().caches;
  return std::ranges::max_element(caches, std::less{},
                                  &CPUInfo::CacheInfo::size)
      ->size;
}

class CacheFlusher {
public:
  CacheFlusher() : buffer_(GetMaxCacheSize(), 0) {}

  void Flush() {
    volatile char *data = buffer_.data();
    for (size_t i = 0; i < buffer_.size(); ++i) {
      data[i] = 1;
    }
  }

private:
  std::vector<char> buffer_;
};

[[maybe_unused]] void DoSetup([[maybe_unused]] const benchmark::State &state) {
  static CacheFlusher cache_flusher;
  cache_flusher.Flush();
}

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
std::vector<T> GenerateData(size_t size, T lo, T hi) {
  static std::mt19937 engine(std::random_device{}());
  std::uniform_int_distribution<T> distr{lo, hi};
  auto gen = [&distr] { return distr(engine); };

  std::vector<T> data(size);
  FillVector(data, gen);

  return data;
}

template <typename Cont, std::integral T = int>
Cont ConstructRandomContainerFast(size_t size) {
  static std::mt19937 engine(std::random_device{}());
  std::uniform_int_distribution<T> distr(std::numeric_limits<T>::min(),
                                         std::numeric_limits<T>::max());

  Cont container;

  if constexpr (requires { container.reserve(size); }) {
    container.reserve(size + 1000);
  }

  if constexpr (requires(Cont c, T num) { c.insert(num); } &&
                !std::is_same_v<Cont, std::vector<T>>) {
    for (size_t i = 0; i < size; ++i) {
      container.insert(distr(engine));
    }
  } else if constexpr (std::is_same_v<Cont, std::vector<T>>) {
    for (size_t i = 0; i < size; ++i) {
      container.push_back(distr(engine));
    }
  } else if constexpr (requires(Cont c, std::pair<T, T> p) { c.insert(p); }) {
    for (size_t i = 0; i < size; ++i) {
      T num = distr(engine);
      container.insert(std::pair{num, num});
    }
  } else if constexpr (requires(Cont c, T num) { c.push_back(num); }) {
    for (size_t i = 0; i < size; ++i) {
      container.push_back(distr(engine));
    }
  } else if constexpr (requires(Cont c, T num) { c.insert(num); }) {
    for (size_t i = 0; i < size; ++i) {
      container.insert(distr(engine));
    }
  } else {
    for (size_t i = 0; i < size; ++i) {
      T num = distr(engine);
      container.emplace(num);
    }
  }

  return container;
}
} // namespace

template <typename Cont>
[[maybe_unused]] static void BenchInsert(benchmark::State &state) {
  using T = Cont::value_type;

  const int kInserts = state.range(0);
  const auto data = ConstructRandomContainerFast<std::vector<T>>(kInserts);

  for (auto _ : state) {
    Cont container;

    for (const auto &val : data) {
      container.insert(val);
    }
    benchmark::DoNotOptimize(container);
  }
  state.SetComplexityN(kInserts);
  state.SetItemsProcessed(state.iterations() * kInserts);
}

template <typename Cont>
[[maybe_unused]] static void BenchFind(benchmark::State &state) {
  using T = Cont::value_type;
  const int kDataSize = state.range(0);

  auto container = ConstructRandomContainerFast<Cont>(kDataSize);
  auto key = RandomNumber<T>();
  benchmark::DoNotOptimize(key);

  for (auto _ : state) {
    benchmark::DoNotOptimize(container.find(key));
  }
  state.SetComplexityN(kDataSize);
  state.SetItemsProcessed(state.iterations());
}

template <typename Cont>
[[maybe_unused]] static void BenchErase(benchmark::State &state) {
  const int kDataSize = state.range(0);
  auto container = ConstructRandomContainerFast<Cont>(kDataSize);
  auto iter = container.begin();

  for (auto _ : state) {
    iter = container.erase(iter);
    benchmark::DoNotOptimize(container);
  }
  state.SetComplexityN(kDataSize);
  state.SetItemsProcessed(state.iterations());
}

template <typename Cont> static void BenchMapInsert(benchmark::State &state) {
  using K = Cont::key_type;
  using V = Cont::mapped_type;

  const int kDataSize = state.range(0);
  auto keys = ConstructRandomContainerFast<std::vector<int>>(kDataSize);
  auto values = ConstructRandomContainerFast<std::vector<int>>(kDataSize);

  Cont container;
  if constexpr (requires { container.reserve(kDataSize); }) {
    container.reserve(kDataSize);
  }
  for (auto i = 0; i < kDataSize; ++i) {
    container[keys[i]] = values[i];
  }

  for (auto _ : state) {
    auto obj_ins = std::make_pair(RandomNumber<K>(), RandomNumber<V>());
    benchmark::DoNotOptimize(obj_ins);
    benchmark::DoNotOptimize(container.insert(obj_ins));
  }
  state.SetComplexityN(kDataSize);
  state.SetItemsProcessed(state.iterations());
}

template <typename Cont> static void BenchMapFind(benchmark::State &state) {
  using K = Cont::key_type;

  const int kDataSize = state.range(0);
  auto container = ConstructRandomContainerFast<Cont>(kDataSize);
  auto key = RandomNumber<K>();
  benchmark::DoNotOptimize(key);

  for (auto _ : state) {
    benchmark::DoNotOptimize(container.find(key));
  }
  state.SetComplexityN(kDataSize);
  state.SetItemsProcessed(state.iterations());
}

template <typename Cont> static void BenchMapErase(benchmark::State &state) {
  // using K = Cont::key_type;

  const int kDataSize = state.range(0);
  auto container = ConstructRandomContainerFast<Cont>(kDataSize);
  auto iter = container.begin();

  for (auto _ : state) {
    iter = container.erase(iter);
    benchmark::DoNotOptimize(container);
  }
  state.SetComplexityN(kDataSize);
  state.SetItemsProcessed(state.iterations());
}

template <typename Cont> static void BenchInsertFront(benchmark::State &state) {
  const int kDataSize = state.range(0);
  auto container = ConstructRandomContainerFast<Cont>(kDataSize);

  for (auto _ : state) {
    benchmark::DoNotOptimize(container.insert(container.begin(), 1));
  }
  state.SetComplexityN(kDataSize);
  state.SetItemsProcessed(state.iterations());
}

template <typename Cont>
static void BenchInsertMiddle_PAUSE(benchmark::State &state) {
  const int kDataSize = state.range(0);
  auto data = ConstructRandomContainerFast<Cont>(kDataSize);

  for (auto _ : state) {
    state.PauseTiming();
    Cont container = data;
    auto iter = container.begin();
    std::advance(iter, kDataSize / 2);
    state.ResumeTiming();

    container.insert(iter, 1);

    benchmark::DoNotOptimize(container);
  }
  state.SetComplexityN(kDataSize);
  state.SetItemsProcessed(state.iterations());
}

template <typename Cont>
static void BenchInsertMiddle(benchmark::State &state) {
  const int kDataSize = state.range(0);
  auto container = ConstructRandomContainerFast<Cont>(kDataSize);

  auto iter = container.begin();
  std::advance(iter, kDataSize / 2);

  for (auto _ : state) {
    iter = container.insert(iter, 1);
    benchmark::DoNotOptimize(container);
  }
  state.SetComplexityN(kDataSize);
  state.SetItemsProcessed(state.iterations());
}

// For list and vector to perform erase from one container it is required to
//  restore it after erasing. push_back should not affect bigO as vector
//  generates with additional reserved space. This is all because
//  benchmark::State::PauseTimer() doesn't work as intended.
template <typename Cont> static void BenchEraseMiddle(benchmark::State &state) {
  const int kDataSize = state.range(0);
  auto container = ConstructRandomContainerFast<Cont>(kDataSize);
  auto iter = container.begin();
  std::advance(iter, kDataSize / 2);

  for (auto _ : state) {
    iter = container.erase(iter);
    benchmark::DoNotOptimize(iter);
    container.push_back(1);
  }
  state.SetComplexityN(kDataSize);
  state.SetItemsProcessed(state.iterations());
}

using set_int = std::set<int>;
using unordered_set_int = std::unordered_set<int>;
using map_int_int = std::map<int, int>;
using unordered_map_int_int = std::unordered_map<int, int>;
using vector_int = std::vector<int>;
using list_int = std::list<int>;

// set vs unordered_set
#define SET_BENCHES                                                            \
  BENCHMARK(BenchInsert<set_int>)->Apply(ConfigureForSet)->MinWarmUpTime(1);   \
  BENCHMARK(BenchInsert<unordered_set_int>)                                    \
      ->Apply(ConfigureForSet)                                                 \
      ->MinWarmUpTime(1);                                                      \
  BENCHMARK(BenchFind<set_int>)->Apply(ConfigureForSet)->MinWarmUpTime(1);     \
  BENCHMARK(BenchFind<unordered_set_int>)                                      \
      ->Apply(ConfigureForSet)                                                 \
      ->MinWarmUpTime(1);                                                      \
  BENCHMARK(BenchErase<set_int>)->Apply(ConfigureForSet)->Iterations(256);     \
  BENCHMARK(BenchErase<unordered_set_int>)                                     \
      ->Apply(ConfigureForSet)                                                 \
      ->Iterations(256);

// map vs unordered_map
#define MAP_BENCHES                                                            \
  BENCHMARK(BenchMapInsert<map_int_int>)                                       \
      ->Apply(ConfigureForMap)                                                 \
      ->MinWarmUpTime(1);                                                      \
  BENCHMARK(BenchMapInsert<unordered_map_int_int>)                             \
      ->Apply(ConfigureForMap)                                                 \
      ->MinWarmUpTime(1);                                                      \
  BENCHMARK(BenchMapFind<map_int_int>)                                         \
      ->Apply(ConfigureForMap)                                                 \
      ->MinWarmUpTime(1);                                                      \
  BENCHMARK(BenchMapFind<unordered_map_int_int>)                               \
      ->Apply(ConfigureForMap)                                                 \
      ->MinWarmUpTime(1);                                                      \
  BENCHMARK(BenchMapErase<map_int_int>)                                        \
      ->Apply(ConfigureForMap)                                                 \
      ->Iterations(256);                                                       \
  BENCHMARK(BenchMapErase<unordered_map_int_int>)                              \
      ->Apply(ConfigureForMap)                                                 \
      ->Iterations(256);

// vector vs list
#define SEQ_BENCHES                                                            \
  BENCHMARK(BenchInsertFront<vector_int>)                                      \
      ->Apply(ConfigureForSequential)                                          \
      ->MinWarmUpTime(1);                                                      \
  BENCHMARK(BenchInsertFront<list_int>)                                        \
      ->Apply(ConfigureForSequential)                                          \
      ->MinWarmUpTime(1);                                                      \
  BENCHMARK(BenchInsertMiddle<vector_int>)                                     \
      ->Apply(ConfigureForSequential)                                          \
      ->MinWarmUpTime(1);                                                      \
  BENCHMARK(BenchInsertMiddle<list_int>)                                       \
      ->Apply(ConfigureForSequential)                                          \
      ->MinWarmUpTime(1);                                                      \
  BENCHMARK(BenchEraseMiddle<vector_int>)                                      \
      ->Apply(ConfigureForSequential)                                          \
      ->Iterations(256);                                                       \
  BENCHMARK(BenchEraseMiddle<list_int>)                                        \
      ->Apply(ConfigureForSequential)                                          \
      ->Iterations(256);

[[maybe_unused]] static void ConfigureForSet(benchmark::Benchmark *b) {
  b->Range(1 << 8, 1 << 20)->Complexity();
}

[[maybe_unused]] static void ConfigureForMap(benchmark::Benchmark *b) {
  b->Range(1 << 8, 1 << 20)->Complexity();
}

[[maybe_unused]] static void ConfigureForSequential(benchmark::Benchmark *b) {
  b->Range(1 << 8, 1 << 20)->Complexity();
}

#if SET_ENABLED_
SET_BENCHES
#endif

#if MAP_ENABLED_
MAP_BENCHES
#endif

#if SEQ_ENABLED_
SEQ_BENCHES
#endif

BENCHMARK_MAIN();
#endif
