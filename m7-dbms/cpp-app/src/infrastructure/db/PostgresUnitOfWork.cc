// infrastructure/db/PostgresUnitOfWork.cc

#include <memory>
#include <pqxx/pqxx>
#include <stdexcept>

#include "infrastructure/db/PostgresPostRepo.h"
#include "infrastructure/db/PostgresTagRepo.h"
#include "infrastructure/db/PostgresUserRepo.h"

#include "PostgresUnitOfWork.h"

namespace infrastructure {

void PostgresUnitOfWork::Begin() {
  if (is_active_) {
    throw std::runtime_error("Transaction is already active");
  }

  try {
    txn_ = std::make_unique<pqxx::work>(*conn_);
    is_active_ = true;

    user_repo_ = std::make_unique<PostgresUserRepo>(*txn_);
    post_repo_ = std::make_unique<PostgresPostRepo>(*txn_);
    tag_repo_ = std::make_unique<PostgresTagRepo>(*txn_);

  } catch (const std::exception &e) {
    is_active_ = false;
    throw std::runtime_error(std::string("Failed to begin transaction: ") +
                             e.what());
  }
}

void PostgresUnitOfWork::Commit() {
  if (!is_active_ || !txn_) {
    throw std::runtime_error("No active transaction to commit");
  }

  try {
    txn_->commit();
    Cleanup();
  } catch (const std::exception &e) {
    Cleanup();
    throw std::runtime_error(std::string("Failed to commit transaction: ") +
                             e.what());
  }
}

void PostgresUnitOfWork::Rollback() {
  if (!is_active_ || !txn_) {
    Cleanup();
    return;
  }

  try {
    txn_->abort();
    Cleanup();
  } catch (const std::exception &e) {
    Cleanup();
    throw std::runtime_error(std::string("Failed to rollback transaction: ") +
                             e.what());
  }
}

domain::IUserRepository &PostgresUnitOfWork::Users() {
  if (!is_active_ || !user_repo_) {
    throw std::runtime_error("Unit of work not started.");
  }
  return *user_repo_;
}

domain::IPostRepository &PostgresUnitOfWork::Posts() {
  if (!is_active_ || !post_repo_) {
    throw std::runtime_error("Unit of work not started.");
  }
  return *post_repo_;
}

domain::ITagRepository &PostgresUnitOfWork::Tags() {
  if (!is_active_ || !tag_repo_) {
    throw std::runtime_error("Unit of work not started.");
  }
  return *tag_repo_;
}

domain::ICommentRepository &PostgresUnitOfWork::Comments() {
  if (!is_active_ || !comment_repo_) {
    throw std::runtime_error("Unit of work not started.");
  }
  return *comment_repo_;
}

PostgresUnitOfWork::~PostgresUnitOfWork() {
  if (is_active_ && txn_) {
    try {
      txn_->abort();
    } catch (...) {
    }
    Cleanup();
  }
}

void PostgresUnitOfWork::Cleanup() noexcept {
  is_active_ = false;
  user_repo_.reset();
  post_repo_.reset();
  tag_repo_.reset();
  txn_.reset();
}

} // namespace infrastructure