// infrastructure/db/PostgresCommentRepo.h
#pragma once

#include <pqxx/pqxx>
#include <vector>

#include "adapters/repositories/ICommentRepository.h"
#include "core/entities/Comment.h"

namespace infrastructure {
// placeholder
class PostgresCommentRepo : public domain::ICommentRepository {
public:
  explicit PostgresCommentRepo(pqxx::work &txn) : txn_(txn) {}

  std::vector<domain::Comment>
  FindByUserId([[maybe_unused]] int userId) override;

private:
  [[maybe_unused]] pqxx::work &txn_;
};
} // namespace infrastructure