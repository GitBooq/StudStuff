// adapters/repositories/ICommentRepository.h
#pragma once

#include <vector>

#include "core/entities/Comment.h"

namespace domain {
class ICommentRepository {
public:
  virtual ~ICommentRepository() = default;
  virtual std::vector<Comment> FindByUserId(int userId) = 0;
};
} // namespace domain