// infrastructure/db/PostgresTagRepo.cc

#include <pqxx/pqxx>
#include <string>

#include "PostgresTagRepo.h"
#include "core/entities/UserTagsResult.h"

namespace infrastructure {
std::vector<domain::UserTagsResult> PostgresTagRepo::GetTagsForAllUsers() {
  try {
    auto raw_query_res = txn_.exec(
        "SELECT U.id AS user_id, U.email, ARRAY_AGG(DISTINCT T.name ORDER BY "
        "T.name) "
        "AS tags FROM users U LEFT JOIN comments C ON C.author_id = U.id "
        "LEFT JOIN post_tags PT ON PT.post_id = C.post_id "
        "LEFT JOIN tags T ON T.id = PT.tag_id "
        "GROUP BY U.id, U.email "
        "ORDER BY U.id ");

    std::vector<domain::UserTagsResult> query_res;
    for (auto row : raw_query_res) {
      int user_id = row[0].as<int>();
      auto email = row[1].as<std::string>();
      auto tags = ParseTags(static_cast<pqxx::field>(row[2]));

      query_res.emplace_back(user_id, std::move(email), std::move(tags));
    }

    return query_res;
  } catch (const std::exception &e) {
    throw;
  }
}

std::set<std::string> PostgresTagRepo::ParseTags(const pqxx::field &field) {
  std::set<std::string> tags;
  if (field.is_null()) {
    return tags;
  }

  // input: "{tag1,tag2,tag3}"
  auto arr = field.as<std::string>();
  // remove {}: "tag1,tag2,tag3"
  if (arr.size() >= 2) {
    arr = arr.substr(1, arr.size() - 2);
  }
  // tokenize by ","
  std::stringstream ss(arr);
  std::string tag;
  while (std::getline(ss, tag, ',')) {
    if (!tag.empty()) {
      tags.insert(tag);
    }
  }
  return tags;
}
} // namespace infrastructure