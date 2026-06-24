// infrastructure/db/PostgresUserRepo.cc

#include <pqxx/pqxx>
#include <string>

#include "PostgresUserRepo.h"
#include "adapters/repositories/IUserRepository.h"
#include "core/entities/User.h"

namespace infrastructure {
std::optional<domain::User> PostgresUserRepo::Create(const std::string &email) {
  try {
    pqxx::result res = txn_.exec("INSERT INTO users (email) VALUES ($1) "
                                 "ON CONFLICT (email) DO NOTHING "
                                 "RETURNING id, email, date",
                                 pqxx::params(email));

    if (res.empty()) {
      return std::nullopt;
    }

    return ParseRow(static_cast<pqxx::row>(res[0]));
  } catch (const std::exception &e) {
    throw;
  }
}

domain::User PostgresUserRepo::ParseRow(const pqxx::row &row) {
  return domain::User{row[0].as<int>(), row[1].as<std::string>(),
                      row[2].as<std::string>()};
}
} // namespace infrastructure