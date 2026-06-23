// core/entities/Tag.h
#pragma once

#include <string>
#include <utility>
#include <stdexcept>

namespace domain {
class Tag final {
 public:
  Tag(int tag_id, std::string name) : id_(tag_id), name_(std::move(name)) {
    Validate();
  }

  [[nodiscard]] int id() const { return id_; }
  [[nodiscard]] std::string name() const { return name_; }

 private:
  static constexpr std::size_t kMaxLength = 250;
  void Validate() const;

  int id_;
  std::string name_;
};

inline void Tag::Validate() const {
  if (id_ < 0) {
    throw std::invalid_argument("Id must be positive.");
  }
  if (name_.length() > kMaxLength) {
    throw std::invalid_argument("Name too long.");
  }
  if (name_.length() == 0) {
    throw std::invalid_argument("Name cannot be empty");
  }
}
}  // namespace domain