// adapters/repositories/IUnitOfWork.h
#pragma once

#include <memory>

namespace domain {

class IUserRepository;
class ICommentRepository;
class IPostRepository;
class ITagRepository;

class IUnitOfWork {
public:
  virtual ~IUnitOfWork() = default;

  virtual void Begin() = 0;
  virtual void Commit() = 0;
  virtual void Rollback() = 0;

  [[nodiscard]] virtual bool IsActive() const = 0;

  virtual IUserRepository &Users() = 0;
  virtual ICommentRepository &Comments() = 0;
  virtual IPostRepository &Posts() = 0;
  virtual ITagRepository &Tags() = 0;
};

class IUnitOfWorkFactory {
public:
  virtual ~IUnitOfWorkFactory() = default;
  virtual std::unique_ptr<IUnitOfWork> Create() = 0;
};

} // namespace domain