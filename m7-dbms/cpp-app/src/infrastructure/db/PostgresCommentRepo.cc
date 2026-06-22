// infrastructure/db/PostgresCommentRepo.cc

#include <pqxx/pqxx>
#include <vector>

#include "PostgresCommentRepo.h"
#include "core/entities/Comment.h"

namespace infrastructure {
// placeholder
std::vector<domain::Comment>
PostgresCommentRepo::FindByUserId([[maybe_unused]] int userId)  {
  return std::vector<domain::Comment>{};
};

} // namespace infrastructure