// src/main.cc

#include <iostream>
#include <memory>
#include <pqxx/pqxx>

#include "core/use_cases/CreateUserWithPost.h"
#include "core/use_cases/GetUserTagsFromComments.h"
#include "infrastructure/db/PostgresUnitOfWork.h"

namespace {

struct PgSettings final {
  std::string host;
  std::string port;
  std::string dbname;
  std::string user;
  std::string password;

  PgSettings() {
    const char *host_ = std::getenv("POSTGRES_HOST");
    const char *port_ = std::getenv("POSTGRES_PORT");
    const char *dbname_ = std::getenv("POSTGRES_DB");
    const char *user_ = std::getenv("POSTGRES_USER");
    const char *password_ = std::getenv("POSTGRES_PASSWORD");

    host = (host_ != nullptr) ? host_ : "localhost";
    port = (port_ != nullptr) ? port_ : "5432";
    dbname = (dbname_ != nullptr) ? dbname_ : "mydb";
    user = (user_ != nullptr) ? user_ : "admin";
    password = (password_ != nullptr) ? password_ : "admin";
  }
};

std::string MakeConnectionString(const PgSettings &env) {
  return std::string("host=" + env.host + " port=" + env.port +
                     " dbname=" + env.dbname + " user=" + env.user +
                     " password=" + env.password);
}
} // namespace

int main() {
  try {
    PgSettings env;
    std::string conn_str = MakeConnectionString(env);
    std::cout << "Connecting to: " << env.host << ":" << env.port << std::endl;
    auto conn = std::make_shared<pqxx::connection>(conn_str);
    std::cout << "Connected to: " << conn->dbname() << std::endl;

    auto factory =
        std::make_shared<infrastructure::PostgresUnitOfWorkFactory>(conn);

    auto tags_use_case = application::GetUserTagsFromComments(factory);
    auto tags_response = tags_use_case.Execute();
    if (tags_response.success) {
      const int kLimit = 50;
      int count = 0;
      for (const auto &[user_id, email, tags] : tags_response.results) {
        if (count >= kLimit) {
          break;
        }
        ++count;

        std::cout << "User ID: " << user_id << " | Email: " << email
                  << " | Tags: ";

        if (tags.empty()) {
          std::cout << "(none)";
        } else {
          for (const auto &tag : tags) {
            std::cout << tag << " ";
          }
        }
        std::cout << "\n";
      }
    } else {
      std::cout << tags_response.message << std::endl;
    }

    auto create_user_post_use_case = application::CreateUserWithPost(factory);
    auto create_user_post_response = create_user_post_use_case.Execute(
        {.title = "transaction test",
         .email = "d34db33f@example.gav",
         .description = "test",
         .status = domain::Post::Status::PUBLISHED});
    std::cout << create_user_post_response.message << std::endl;
    if (create_user_post_response.success) {
      std::cout << std::format(
          "Added: id {} | email {} | title {} | desc {}\n",
          create_user_post_response.user->id(),
          create_user_post_response.user->email(),
          create_user_post_response.post->title(),
          create_user_post_response.post->description().has_value()
              ? *create_user_post_response.post->description()
              : "NULL");
    }

    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}