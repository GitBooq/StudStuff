// core/entities/Comment.h
#pragma once

#include <optional>
#include <string>
#include <utility>

namespace domain {
class Comment final {
public:
  // DBMS performs base validity checks
  Comment(int id, std::string content, std::string date, int post_id,
          std::optional<int> author_id)
      : id_(id), content_(std::move(content)), date_(std::move(date)),
        post_id_(post_id), author_id_(author_id) {}

  [[nodiscard]] int id() const { return id_; }
  [[nodiscard]] std::string content() const { return content_; }
  [[nodiscard]] int post_id() const { return post_id_; }
  [[nodiscard]] std::optional<int> author_id() const { return author_id_; }

private:
  int id_;
  std::string content_;
  std::string date_;
  int post_id_;
  std::optional<int> author_id_;
};
} // namespace domain