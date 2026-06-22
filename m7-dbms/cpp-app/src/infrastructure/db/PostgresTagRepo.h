// infrastructure/db/PostgresTagRepo.h
#pragma once

#include <pqxx/pqxx>
#include <string>

#include "adapters/repositories/ITagRepository.h"
#include "core/entities/UserTagsResult.h"

namespace infrastructure {
class PostgresTagRepo : public domain::ITagRepository {
public:
  explicit PostgresTagRepo(pqxx::work &txn) : txn_(txn) {}

  std::vector<domain::UserTagsResult> GetTagsForAllUsers() override;

private:
  static std::set<std::string> ParseTags(const pqxx::field &field);

  pqxx::work &txn_;
};
} // namespace infrastructure