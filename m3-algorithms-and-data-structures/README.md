# 1. Student List (std::map)

## Features

- Reading a list of students from a stream (name, grade)
- Storage in `std::map` (automatic sorting by name)
- Finding the student with the maximum grade
- Calculating the average grade
- Outputting all students in sorted order

## API

| Method | Description |
|--------|-------------|
| `Read(is)` | Reads data from a stream (format: `name grade`) |
| `Output(os)` | Outputs all students in sorted order |
| `GetMaxGradeStudent()` | Returns an iterator to the student with the maximum grade |
| `GetAvgGrade()` | Returns the average grade (0.0 if there are no students) |
| `End()` | Returns a constant iterator to the end |

## Example

```cpp
#include "student_list.h"
#include <iostream>
#include <sstream>

int main() {
    std::istringstream data("Alice 95\nBob 87\nCharlie 92");
    
    StudentList list;
    list.Read(data);
    
    std::cout << "Students:\n";
    list.Output(std::cout);
    
    std::cout << "Average grade: " << list.GetAvgGrade() << std::endl;
    
    auto it = list.GetMaxGradeStudent();
    if (it != list.End()) {
        std::cout << "Best: " << it->first << " (" << it->second << ")" << std::endl;
    }
    
    return 0;
}
```

# 2. STL Algorithms

STL algorithms demonstration.

## Summary

1. Generate 100 random integers in range [1, 1000]
2. Sort in descending order
3. Find all prime numbers
4. Remove duplicates
5. Calculate sum of squares of elements

# 3. Red-Black Tree

Self-balancing binary search tree with guaranteed `O(log n)` complexity for insert, find, and erase operations.

## Features

- Self-balancing binary search tree
- Guaranteed `O(log n)` complexity for basic operations
- Duplicate keys support (multiset behavior)
- Bidirectional iterators
- Red-black invariants validation

## API

| Method | Description |
|--------|-------------|
| `Insert(key)` | Insert an element |
| `Find(key)` | Find an element |
| `Remove(key)` | Remove all elements with given key |
| `Remove(pos)` | Remove element pointed by iterator |
| `Print(ostream)` | Print elements inorder |
| `size()` / `empty()` | Current container size |
| `Clear()` | Clear the tree |

## Iterators

- `begin()` / `end()` — forward iteration in ascending order
- `cbegin()` / `cend()` — const iterators

## Example

```cpp
#include "red_black_tree.h"
#include <iostream>

int main() {
    RBTree::RedBlackTree<int> tree;

    tree.Insert(5);
    tree.Insert(3);
    tree.Insert(7);
    tree.Insert(1);
    tree.Insert(9);

    std::cout << "Size: " << tree.size() << std::endl;  // Size: 5

    if (auto it = tree.Find(3); it != tree.end()) {
        std::cout << "Found: " << *it << std::endl;     // Found: 3
    }

    tree.Remove(3);
    std::cout << "After removal: " << tree.size() << std::endl;  // After removal: 4

    std::cout << "Inorder: ";
    tree.Print();  // Inorder: 1 5 7 9
    std::cout << std::endl;

    return 0;
}
```

# 4. Hash Table (Chaining)

Hash table with chaining collision resolution using `std::list`.

## Features

- Collision resolution via `std::list` (chaining)
- Automatic rehashing when `load_factor > 0.75`
- Manual rehashing with `Rehash()`
- Statistics output (load factor, max bucket length)

## API

| Method | Description |
|--------|-------------|
| `Insert(key, value)` | Insert or update element |
| `Find(key)` | Find value by key |
| `Remove(key)` | Remove element by key |
| `Rehash(count)` | Force rehash (≥ `count` buckets) |
| `size()` / `empty()` | Current size |
| `StatisticPrint()` | Print load factor and max bucket length |

## Example

```cpp
#include "hash_table.h"
#include <iostream>

int main() {
    HashTable<std::string, int> scores;

    scores.Insert("Alice", 100);
    scores.Insert("Bob", 85);
    scores.Insert("Charlie", 92);


    auto print_score = [&](std::string_view name, int score) {
        std::cout << std::format("{}: {}", name, score) << std::endl;
    };

    if (auto aliceScore = scores.Find("Alice")) {
        print_score("Alice", aliceScore.value()); // Alice: 100
    }

    if (!scores.empty()) {
        scores.Remove("Bob");
    }

    scores.Rehash(2);
    std::cout << "Table size: " << scores.size() << std::endl;  // Table size: 2

    scores.StatisticPrint();
    // Load factor: 0.12
    // Max bucket length: 1

    return 0;
}
```

# 5. Performance Benchmarks

Benchmarks comparing performance of standard C++ containers.

## Containers Compared

| Category | Containers |
|----------|------------|
| **Associative (Set)** | `std::set` vs `std::unordered_set` |
| **Associative (Map)** | `std::map` vs `std::unordered_map` |
| **Sequential** | `std::vector` vs `std::list` |

## Operations Measured

| Container Type | Operations |
|----------------|------------|
| Set / Map | Insert, Find, Erase |
| Vector / List | Insert (middle), Erase (front), Erase (back) |

## Build
###  Options

| Option | Default | Description | Target name |
|--------|---------|-------------|-------------|
| `BUILD_TASK1` | ON | Containers common (StudentList) | StudentListTest |
| `BUILD_TASK2` | ON | Algorithms common | Algos |
| `BUILD_TASK3` | ON | Red-Black Tree | RBTreeTest |
| `BUILD_TASK4` | ON | Hash Table (chaining) | HashTable |
| `BUILD_TASK5` | ON | Performance benchmarks | BenchCompare

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release
# Build all tasks
cmake --build build
# Build only benchmark
cmake --build build --target BenchCompare
```

## Run Tests
```bash
ctest --test-dir ./build/ --output-on-failure
```
## Run Benchmark
```bash
./build/5-perf/BenchCompare
```