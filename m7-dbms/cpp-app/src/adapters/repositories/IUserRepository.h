// adapters/repositories/IUserRepository.h
#pragma once

#include <optional>
#include <string>

namespace domain {
class User;

class IUserRepository {
public:
  virtual ~IUserRepository() = default;
  virtual std::optional<User> Create(const std::string &email) = 0;
};
} // namespace domain