// adapters/repositories/IPostRepository.h
#pragma once

#include "core/entities/Post.h"
#include <optional>
#include <string>

namespace domain {
class IPostRepository {
public:
  virtual ~IPostRepository() = default;
  virtual Post Create(const std::string &title,
                      const std::optional<std::string> &description,
                      Post::Status status, std::optional<int> author_id) = 0;
};
} // namespace domain