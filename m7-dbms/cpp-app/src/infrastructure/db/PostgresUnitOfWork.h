// infrastructure/db/PostgresUnitOfWork.h
#pragma once

#include <memory>
#include <pqxx/pqxx>
#include <stdexcept>

#include "adapters/repositories/IUnitOfWork.h"
#include "infrastructure/db/PostgresCommentRepo.h"
#include "infrastructure/db/PostgresPostRepo.h"
#include "infrastructure/db/PostgresTagRepo.h"
#include "infrastructure/db/PostgresUserRepo.h"

namespace infrastructure {

class PostgresUnitOfWork : public domain::IUnitOfWork {
private:
  std::shared_ptr<pqxx::connection> conn_;
  std::unique_ptr<pqxx::work> txn_;

  std::unique_ptr<PostgresUserRepo> user_repo_;
  std::unique_ptr<PostgresPostRepo> post_repo_;
  std::unique_ptr<PostgresTagRepo> tag_repo_;
  std::unique_ptr<PostgresCommentRepo> comment_repo_;

  bool is_active_ = false;

public:
  explicit PostgresUnitOfWork(std::shared_ptr<pqxx::connection> conn)
      : conn_(std::move(conn)) {
    if (!conn_) {
      throw std::runtime_error("Database connection is null");
    }
  }

  PostgresUnitOfWork(const PostgresUnitOfWork &) = delete;
  PostgresUnitOfWork &operator=(const PostgresUnitOfWork &) = delete;
  PostgresUnitOfWork(PostgresUnitOfWork &&) = default;
  PostgresUnitOfWork &operator=(PostgresUnitOfWork &&) = default;

  void Begin() override;
  void Commit() override;
  void Rollback() override;

  [[nodiscard]] bool IsActive() const override { return is_active_; }

  domain::IUserRepository &Users() override;
  domain::IPostRepository &Posts() override;
  domain::ITagRepository &Tags() override;
  domain::ICommentRepository &Comments() override;

  ~PostgresUnitOfWork() override;

private:
  void Cleanup() noexcept;
};

class PostgresUnitOfWorkFactory : public domain::IUnitOfWorkFactory {
private:
  std::shared_ptr<pqxx::connection> conn_;

public:
  explicit PostgresUnitOfWorkFactory(const std::string &conn_str) {
    try {
      conn_ = std::make_shared<pqxx::connection>(conn_str);
      if (!conn_->is_open()) {
        throw std::runtime_error("Failed to connect to database");
      }
    } catch (const std::exception &e) {
      throw std::runtime_error(std::string("Failed to create connection: ") +
                               e.what());
    }
  }

  explicit PostgresUnitOfWorkFactory(std::shared_ptr<pqxx::connection> conn)
      : conn_(std::move(conn)) {
    if (!conn_ || !conn_->is_open()) {
      throw std::runtime_error("Invalid database connection");
    }
  }

  std::unique_ptr<domain::IUnitOfWork> Create() override {
    if (!conn_ || !conn_->is_open()) {
      throw std::runtime_error("Database connection is not available");
    }

    return std::make_unique<PostgresUnitOfWork>(conn_);
  }

  [[nodiscard]] std::shared_ptr<pqxx::connection> getConnection() const {
    return conn_;
  }
};

} // namespace infrastructure