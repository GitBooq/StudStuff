// core/use_cases/CreateUserWithPost.h
#pragma once

#include "adapters/repositories/IUnitOfWork.h"
#include "adapters/repositories/IUserRepository.h"
#include "core/entities/Post.h"
#include "core/entities/User.h"

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

namespace application {

/**
 * Perform transaction-based insertion of new user and post
 */
class CreateUserWithPost {
public:
  struct Request {
    std::string title;
    std::string email;
    std::optional<std::string> description;
    domain::Post::Status status = domain::Post::Status::PUBLISHED;
  };

  struct Response {
    bool success;
    std::string message;
    std::optional<domain::User> user;
    std::optional<domain::Post> post;

    static Response Success(const domain::User &user,
                            const domain::Post &post) {
      return Response{.success = true,
                      .message = "User and post created successfully",
                      .user = user,
                      .post = post};
    }

    static Response Error(const std::string &error) {
      return Response{.success = false,
                      .message = error,
                      .user = std::nullopt,
                      .post = std::nullopt};
    }

    static Response UserExists() {
      return Response{.success = false,
                      .message = "User with this email already exists",
                      .user = std::nullopt,
                      .post = std::nullopt};
    }
  };

  explicit CreateUserWithPost(
      std::shared_ptr<domain::IUnitOfWorkFactory> uow_factory)
      : uow_factory_(std::move(uow_factory)) {
    if (!uow_factory_) {
      throw std::runtime_error("UnitOfWorkFactory is null");
    }
  }

  Response Execute(const Request &request);

private:
  std::shared_ptr<domain::IUnitOfWorkFactory> uow_factory_;
};

} // namespace application