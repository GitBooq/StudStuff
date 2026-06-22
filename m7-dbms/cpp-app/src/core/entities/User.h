// core/entities/User.h
#pragma once

#include <string>
#include <utility>

namespace domain {
class User final {
public:
  // DBMS performs base validity checks
  User(int id, std::string email, std::string date)
      : id_(id), email_(std::move(email)), date_(std::move(date)) {}

  [[nodiscard]] int id() const { return id_; }
  [[nodiscard]] std::string email() const { return email_; }
  [[nodiscard]] std::string date() const { return date_; }

private:
  int id_;
  std::string email_;
  std::string date_;
};
} // namespace domain