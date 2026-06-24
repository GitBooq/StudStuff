// core/entities/Comment.h
#pragma once

#include <optional>
#include <string>
#include <utility>
#include <stdexcept>

namespace domain {
class Comment final {
 public:
  Comment(int id, std::string content, std::string date, int post_id,
          std::optional<int> author_id)
      : id_(id),
        content_(std::move(content)),
        date_(std::move(date)),
        post_id_(post_id),
        author_id_(author_id) {
    Validate();
  }

  [[nodiscard]] int id() const { return id_; }
  [[nodiscard]] std::string content() const { return content_; }
  [[nodiscard]] int post_id() const { return post_id_; }
  [[nodiscard]] std::optional<int> author_id() const { return author_id_; }

 private:
  static constexpr std::size_t kMaxLength = 250;
  void Validate() const;

  int id_;
  std::string content_;
  std::string date_;
  int post_id_;
  std::optional<int> author_id_;
};

inline void Comment::Validate() const {
  if (id_ < 0) {
    throw std::invalid_argument("Id must be positive.");
  }
  if (content_.length() > kMaxLength) {
    throw std::invalid_argument("Content too long.");
  }
  if (content_.length() == 0) {
    throw std::invalid_argument("Content cannot be empty");
  }
  if (date_.length() > kMaxLength) {
    throw std::invalid_argument("Date too long.");
  }
  if (post_id_ < 0) {
    throw std::invalid_argument("post_id must be positive.");
  }
  if (author_id_.has_value() && author_id_.value() < 0) {
     throw std::invalid_argument("author_id must be positive.");
  }
}
}  // namespace domain