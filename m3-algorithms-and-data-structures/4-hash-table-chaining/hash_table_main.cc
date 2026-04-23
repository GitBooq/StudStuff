#include "hash_table.h"
#include <iostream>

int main() {
  HashTable<std::string, int> scores;

  scores.Insert("Alice", 100);
  scores.Insert("Bob", 85);
  scores.Insert("Charlie", 92);

  int aliceScore;
  if (scores.Find("Alice", aliceScore)) {
    std::cout << "Alice: " << aliceScore << std::endl;
  }

  if (!scores.empty()) {
    scores.Remove("Bob");
  }

  scores.Rehash(2);
  std::cout << "Table size: " << scores.size() << std::endl;

  scores.StatisticPrint();

  return 0;
}