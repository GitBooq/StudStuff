// core/use_cases/GetUserTagsFromComments.cc

#include "GetUserTagsFromComments.h"
#include "adapters/repositories/ITagRepository.h"
#include "adapters/repositories/IUnitOfWork.h"
#include "core/entities/UserTagsResult.h"

#include <memory>
#include <vector>

namespace application {

GetUserTagsFromComments::Response
GetUserTagsFromComments::Execute([[maybe_unused]] const Request &request) {
  try {
    auto uow = uow_factory_->Create();

    uow->Begin();

    std::vector<domain::UserTagsResult> results =
        uow->Tags().GetTagsForAllUsers();

    uow->Commit();

    return Response::Success(results);

  } catch (const std::exception &e) {
    return Response::Error(std::string("Database error: ") + e.what());
  }
}

} // namespace application