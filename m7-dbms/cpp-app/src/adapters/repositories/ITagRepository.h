// adapters/repositories/ITagRepository.h
#pragma once

#include "core/entities/UserTagsResult.h"
#include <vector>

namespace domain {
class ITagRepository {
public:
  virtual ~ITagRepository() = default;
  virtual std::vector<UserTagsResult> GetTagsForAllUsers() = 0;
};
} // namespace domain