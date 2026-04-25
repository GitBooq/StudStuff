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
    print_score("Alice", aliceScore.value());
  }

  if (!scores.empty()) {
    scores.Remove("Bob");
  }

  scores.Rehash(2);
  std::cout << "Table size: " << scores.size() << std::endl;

  scores.StatisticPrint();

  return 0;
}