// core/entities/Tag.h
#pragma once

#include <string>
#include <utility>

namespace domain {
class Tag final {
public:
  // DBMS performs base validity checks
  Tag(int tag_id, std::string name) : id_(tag_id), name_(std::move(name)) {}

  [[nodiscard]] int id() const { return id_; }
  [[nodiscard]] std::string name() const { return name_; }

private:
  int id_;
  std::string name_;
};
} // namespace domain