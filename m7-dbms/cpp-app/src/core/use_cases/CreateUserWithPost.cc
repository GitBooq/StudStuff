// core/use_cases/CreateUserWithPost.h

#include "CreateUserWithPost.h"
#include "adapters/repositories/IPostRepository.h"
#include "adapters/repositories/IUnitOfWork.h"
#include "adapters/repositories/IUserRepository.h"
#include "core/entities/Post.h"
#include "core/entities/User.h"

#include <memory>
#include <optional>
#include <string>

namespace application {

CreateUserWithPost::Response
CreateUserWithPost::Execute(const Request &request) {
  if (request.email.empty()) {
    return Response::Error("Email cannot be empty");
  }

  if (request.title.empty()) {
    return Response::Error("Post title cannot be empty");
  }

  auto uow = uow_factory_->Create();

  try {
    uow->Begin();

    auto new_user = uow->Users().Create(request.email);
    if (!new_user.has_value()) {
      uow->Rollback();
      return Response::UserExists();
    }

    domain::Post new_post = uow->Posts().Create(
        request.title, request.description, request.status, new_user->id());

    uow->Commit();

    return Response::Success(*new_user, new_post);
  } catch (const std::exception &e) {
    try {
      uow->Rollback();
    } catch (...) {
      // suppress rollback exceptions
    }

    return Response::Error(std::string("Database error: ") + e.what());
  }
}

} // namespace application