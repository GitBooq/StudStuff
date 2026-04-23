#include <algorithm>
#include <format>
#include <iostream>
#include <list>
#include <ranges>
#include <vector>

/**
 * @brief Hash table (chaining)
 *
 * @tparam K key
 * @tparam V value
 */
template <typename K, typename V> class HashTable {
private:
  static constexpr double kLoadFactor = 0.75;
  static constexpr size_t kDefaultCapacity = 17;

  struct Entry {
    K key;
    V value;
    Entry(const K &key, const V &val) : key(key), value(val) {}
  };

  using Bucket = std::list<Entry>;
  using iterator = typename Bucket::iterator;
  using const_iterator = typename Bucket::const_iterator;

  std::vector<Bucket> table_;
  size_t size_;
  size_t capacity_;

  size_t HashFunction(const K &key) const { return std::hash<K>{}(key); }
  size_t Index(const K &key) const { return HashFunction(key) % capacity_; }

  /// Return [bucket_index, iter, bFound] of entry with specified key
  auto FindEntry(const K &key) -> std::tuple<size_t, iterator, bool>;
  auto FindEntry(const K &key) const
      -> std::tuple<size_t, const_iterator, bool>;

  /// Return [load factor, max bucket length]
  auto GetStatistic() const -> std::pair<double, size_t>;

public:
  explicit HashTable(size_t cap = kDefaultCapacity) : size_(0), capacity_(cap) {
    table_.resize(capacity_);
  }

  /**
   * @brief Insert new object into the hash table if it is not there otherwise
   * update existing object's value with specified by %value
   * @note May lead to table rehashing
   * @param key
   * @param value
   */
  void Insert(const K &key, const V &value);

  /// Return true if oject with specified %key exist, false otherwise
  bool Find(const K &key, V &value) const;

  /// Remove object with specified %key
  bool Remove(const K &key);

  /// Changes buckets number to value not less than %count, then rehashes the
  /// table
  void Rehash(size_t count);

  /// Get size of hash table
  size_t size() const { return size_; }
  /// Return true if hash table is empty, false otherwise
  bool empty() const { return size_ == 0; }

  /// Prints load factor and max bucket length
  void StatisticPrint(std::ostream &ostream = std::cout) const;
};

template <typename K, typename V>
void HashTable<K, V>::Insert(const K &key, const V &value) {
  auto [index, iter, found] = FindEntry(key);
  if (found) {
    iter->value = value;
  } else {
    table_[index].emplace_back(key, value);
    ++size_;

    if (size_ > capacity_ * kLoadFactor) {
      Rehash(capacity_ * 2);
    }
  }
}

template <typename K, typename V>
bool HashTable<K, V>::Find(const K &key, V &value) const {
  auto [_, iter, found] = FindEntry(key);
  if (found) {
    value = iter->value;
  }

  return found;
}

template <typename K, typename V> bool HashTable<K, V>::Remove(const K &key) {
  auto [index, iter, found] = FindEntry(key);
  if (found) {
    table_[index].erase(iter);
    --size_;
  }

  return found; // erased if found
}

template <typename K, typename V> void HashTable<K, V>::Rehash(size_t count) {
  /// @todo not prime -- collisions
  size_t min_buckets = (size() + 1) / kLoadFactor; // ceil
  size_t new_capacity = std::max(count, min_buckets);

  std::vector<std::list<Entry>> newTable(new_capacity);

  for (const auto &bucket : table_) {
    for (const auto &entry : bucket) {
      size_t newIndex = HashFunction(entry.key) % new_capacity;
      newTable[newIndex].push_back(entry);
    }
  }

  table_ = std::move(newTable);
  capacity_ = new_capacity;
}

template <typename K, typename V>
auto HashTable<K, V>::FindEntry(const K &key)
    -> std::tuple<size_t, iterator, bool> {
  size_t index = Index(key);
  auto &bucket = table_[index];
  auto iter = std::ranges::find(bucket, key, &Entry::key);
  bool found = iter != bucket.end();

  return {index, iter, found};
}

template <typename K, typename V>
auto HashTable<K, V>::FindEntry(const K &key) const
    -> std::tuple<size_t, const_iterator, bool> {
  size_t index = Index(key);
  const auto &bucket = table_[index];
  auto const_iter = std::ranges::find(bucket, key, &Entry::key);
  bool found = const_iter != bucket.end();

  return {index, const_iter, found};
}

template <typename K, typename V>
auto HashTable<K, V>::GetStatistic() const -> std::pair<double, size_t> {
  if (table_.empty()) {
    return {.0, 0};
  }

  auto load_factor = static_cast<double>(size_) / capacity_;
  auto max_bucket_length =
      std::ranges::max(table_ | std::views::transform(&Bucket::size));

  return {load_factor, max_bucket_length};
}

template <typename K, typename V>
void HashTable<K, V>::StatisticPrint(std::ostream &ostream) const {
  auto [load_factor, max_bucket_length] = GetStatistic();

  try {
    std::format_to(std::ostreambuf_iterator<char>(ostream),
                   "Load factor: {:.2f}\n"
                   "Max bucket length: {}\n",
                   load_factor, max_bucket_length);
  } catch (const std::exception &e) {
    std::cerr << "HashTable::StatisticPrint error: " << e.what() << '\n';
    throw;
  } catch (...) {
    std::cerr << "HashTable::StatisticPrint error: unknown\n";
    throw;
  }
}