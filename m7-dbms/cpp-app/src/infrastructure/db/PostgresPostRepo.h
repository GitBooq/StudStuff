// infrastructure/db/PostgresPostRepo.h
#pragma once

#include <optional>
#include <pqxx/pqxx>
#include <string>

#include "adapters/repositories/IPostRepository.h"
#include "core/entities/Post.h"

namespace infrastructure {
class PostgresPostRepo : public domain::IPostRepository {
public:
  explicit PostgresPostRepo(pqxx::work &txn) : txn_(txn) {}

  domain::Post Create(const std::string &title,
                      const std::optional<std::string> &description,
                      domain::Post::Status status,
                      std::optional<int> author_id) override;

private:
  pqxx::work &txn_;
};
} // namespace infrastructure