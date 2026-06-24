// infrastructure/db/PostgresUserRepo.h
#pragma once

#include <pqxx/pqxx>
#include <string>

#include "adapters/repositories/IUserRepository.h"
#include "core/entities/User.h"

namespace infrastructure {
class PostgresUserRepo : public domain::IUserRepository {
public:
  explicit PostgresUserRepo(pqxx::work &txn) : txn_(txn) {}

  // Inserts and returns user if it doesn't exist
  std::optional<domain::User> Create(const std::string &email) override;

private:
  static domain::User ParseRow(const pqxx::row &row);

  pqxx::work &txn_;
};

} // namespace infrastructure