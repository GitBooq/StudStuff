// core/entities/Post.h
#pragma once

#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

namespace domain {
class Post final {
 public:
  enum class Status { DRAFT, PUBLISHED, ARCHIVED };

  static std::string StatusToString(Status status) {
    switch (status) {
      case Post::Status::DRAFT:
        return "draft";
      case Post::Status::PUBLISHED:
        return "published";
      case Post::Status::ARCHIVED:
        return "archived";
    }

    throw std::runtime_error("Unknown Post status value: " +
                             std::to_string(static_cast<int>(status)));
  }

  static Status StringToStatus(const std::string& status_str) {
    if (status_str == "published") {
      return Post::Status::PUBLISHED;
    }
    if (status_str == "draft") {
      return Post::Status::DRAFT;
    }
    if (status_str == "archived") {
      return Post::Status::ARCHIVED;
    }

    throw std::runtime_error("Unknown Post status string: " + status_str);
  }

  Post(int id, std::string title, std::string date, Status status,
       std::optional<int> author_id, std::optional<std::string> description)
      : id_(id),
        title_(std::move(title)),
        date_(std::move(date)),
        status_(status),
        author_id_(author_id),
        description_(std::move(description)) {
    Validate();
  }

  [[nodiscard]] int id() const { return id_; }
  [[nodiscard]] std::string title() const { return title_; }
  [[nodiscard]] Status status() const { return status_; }
  [[nodiscard]] std::optional<int> author_id() const { return author_id_; }
  [[nodiscard]] std::optional<std::string> description() const {
    return description_;
  }

 private:
  static constexpr std::size_t kMaxLength = 250;
  void Validate() const;

  int id_;
  std::string title_;
  std::string date_;
  Status status_;
  std::optional<int> author_id_;
  std::optional<std::string> description_;
};

inline void Post::Validate() const {
  if (id_ < 0) {
    throw std::invalid_argument("Id must be positive.");
  }
  if (title_.length() > kMaxLength) {
    throw std::invalid_argument("Content too long.");
  }
  if (title_.length() == 0) {
    throw std::invalid_argument("Title cannot be empty");
  }
  if (date_.length() > kMaxLength) {
    throw std::invalid_argument("Date too long.");
  }
  if (author_id_.has_value() && author_id_.value() < 0) {
    throw std::invalid_argument("author_id must be positive.");
  }
  if (description_.has_value() && description_.value().length() > kMaxLength) {
    throw std::invalid_argument("Description too long.");
  }
}
}  // namespace domain