// core/entities/UserTagsResult.h
#pragma once

#include <set>
#include <string>

namespace domain {
struct UserTagsResult {
  int user_id;
  std::string email;
  std::set<std::string> tags;
};
} // namespace domain