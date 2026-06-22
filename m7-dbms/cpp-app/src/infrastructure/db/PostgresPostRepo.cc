// infrastructure/db/PostgresPostRepo.cc

#include <optional>
#include <pqxx/pqxx>
#include <string>

#include "PostgresPostRepo.h"
#include "core/entities/Post.h"

namespace infrastructure {
domain::Post PostgresPostRepo::Create(
    const std::string &title, const std::optional<std::string> &description,
    domain::Post::Status status, std::optional<int> author_id) {

  try {
    std::string status_str = domain::Post::StatusToString(status);

    pqxx::result res = txn_.exec(
        "INSERT INTO posts (title, description, status, author_id) "
        "VALUES ($1, $2, $3, $4) "
        "RETURNING id, title, description, date, status, author_id",
        pqxx::params(title,
                     description.has_value() ? description.value()
                                             : std::optional<std::string>{},
                     status_str, author_id));

    enum Query { ID = 0, TITLE, DESCRIPTION, DATE, STATUS, AUTHOR_ID };

    auto row = res[0];

    int ret_id = row[Query::ID].as<int>();
    auto ret_title = row[Query::TITLE].as<std::string>();
    std::optional<std::string> ret_desc;
    if (!row[Query::DESCRIPTION].is_null()) {
      ret_desc = row[Query::DESCRIPTION].as<std::string>();
    }
    auto ret_date = row[Query::DATE].as<std::string>();
    auto ret_status_str = row[Query::STATUS].as<std::string>();
    std::optional<int> ret_author_id;
    if (!row[Query::AUTHOR_ID].is_null()) {
      ret_author_id = row[Query::AUTHOR_ID].as<int>();
    }

    domain::Post::Status ret_post_status =
        domain::Post::StringToStatus(ret_status_str);

    return domain::Post{ret_id,          ret_title,     ret_date,
                        ret_post_status, ret_author_id, ret_desc};

  } catch (const std::exception &e) {

    throw;
  }
}
} // namespace infrastructure