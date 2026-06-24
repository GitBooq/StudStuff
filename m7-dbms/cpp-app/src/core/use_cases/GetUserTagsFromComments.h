// core/use_cases/GetUserTagsFromComments.h
#pragma once

#include "adapters/repositories/IUnitOfWork.h"
#include "core/entities/UserTagsResult.h"

#include <memory>
#include <stdexcept>
#include <vector>

namespace application {

/**
 * Get a set of tags from posts for each user to which he left a comment.
 */
class GetUserTagsFromComments {
public:
  struct Request {
    static constexpr int kLimit = 10;
    int limit;

    Request() : limit(kLimit) {}
    Request(int limit) : limit(limit) {}
  };

  struct Response {
    bool success;
    std::string message;
    std::vector<domain::UserTagsResult> results;

    static Response
    Success(const std::vector<domain::UserTagsResult> &results) {
      return Response{.success = true,
                      .message = "Tags retrieved successfully",
                      .results = results};
    }

    static Response Error(const std::string &error) {
      return Response{.success = false, .message = error, .results = {}};
    }
  };

  explicit GetUserTagsFromComments(
      std::shared_ptr<domain::IUnitOfWorkFactory> uow_factory)
      : uow_factory_(std::move(uow_factory)) {
    if (!uow_factory_) {
      throw std::runtime_error("UnitOfWorkFactory is null");
    }
  }

  Response Execute([[maybe_unused]] const Request &request = Request());

private:
  std::shared_ptr<domain::IUnitOfWorkFactory> uow_factory_;
};

} // namespace application