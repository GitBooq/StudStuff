#include "common/helpers.h"

namespace helpers {

std::string GetCurrentUTC() {
  using namespace std::chrono;
  auto now = floor<seconds>(utc_clock::now());

  return std::format("{:%FT%T}Z", now);
}

std::string GetTargetServerString(const std::string& host,
                                  const std::string& port) {
  return host + std::string(":") + port;
}

std::vector<std::string> SplitByLines(const std::string& text,
                                      std::size_t lines_per_chunk) {
  std::vector<std::string> result;

  std::istringstream ss(text);

  std::string line;
  std::string chunk;

  std::size_t count{0};

  while (std::getline(ss, line)) {
    chunk += line + '\n';
    ++count;

    if (count == lines_per_chunk) {
      result.push_back(std::move(chunk));
      chunk.clear();
      count = 0;
    }
  }

  if (!chunk.empty()) {
    result.push_back(std::move(chunk));
  }

  return result;
}

std::vector<application::LogMessage> GetLogMessages(
    const std::vector<std::string>& payloads, const std::string& log_source) {
  std::vector<application::LogMessage> messages;
  messages.reserve(payloads.size());

  for (const auto& payload : payloads) {
    messages.emplace_back(log_source, GetCurrentUTC(), payload);
  }

  return messages;
}

std::string GetServerNameOrLocalhost() {
  const auto* const chars = std::getenv("SERVER_HOST");
  return (nullptr != chars) ? chars : "localhost";
}

std::string GetServerPortOr50051() {
  const auto* const chars = std::getenv("SERVER_PORT");
  return (nullptr != chars) ? chars : "50051";
}

}  // namespace helpers