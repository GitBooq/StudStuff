// core/entities/User.h
#pragma once

#include <string>
#include <utility>
#include <stdexcept>

namespace domain {
class User final {
 public:
  User(int id, std::string email, std::string date)
      : id_(id), email_(std::move(email)), date_(std::move(date)) {
    Validate();
  }

  [[nodiscard]] int id() const { return id_; }
  [[nodiscard]] std::string email() const { return email_; }
  [[nodiscard]] std::string date() const { return date_; }

 private:
  static constexpr std::size_t kMaxLength = 250;
  void Validate() const;

  int id_;
  std::string email_;
  std::string date_;
};

inline void User::Validate() const {
  if (id_ < 0) {
    throw std::invalid_argument("Id must be positive.");
  }
  if (email_.length() > kMaxLength || email_.length() == 0) {
    throw std::invalid_argument("Incorrect email.");
  }
  if (date_.length() > kMaxLength) {
    throw std::invalid_argument("Date too long.");
  }
}
}  // namespace domain