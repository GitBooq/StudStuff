// sender_client.cc
#include <net_logger/net_logger.h>

#include "sender/grpc_message_sender.h"
#include "common/log_message.h"

#include <chrono>
#include <format>
#include <fstream>
#include <iostream>
#include <string>

namespace {

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

}  // namespace

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cout << "Usage: sender [path_to_file]\n";
    return 1;
  }

  std::ifstream input(argv[1]);
  if (!input.is_open()) {
    std::cerr << "Can't open file\n";
    return 1;
  }

  std::stringstream buffer;
  buffer << input.rdbuf();

  try {
    auto filter = net::logger::CreateFilter(
        {{"type", "subnet", "value", "192.168.1.0/24"},
         {"type", "range", "value", "10.0.0.1-10.0.0.100"}});

    std::ostringstream payload;
    net::logger::ProcessStream(buffer, payload, filter);
    std::string payload_str = payload.str();

    auto payloads = SplitByLines(payload_str, 3);

    std::vector<application::LogMessage> log_messages =
        GetLogMessages(payloads, "ipv4_filter");

    std::string target =
        GetTargetServerString(GetServerNameOrLocalhost(), "50051");
    auto channel =
        grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
    datatransfer::GrpcMessageSender sender(channel);

    for (const auto& msg : log_messages) {
      sender.Send(msg);
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Unknown exception caught" << std::endl;
    return 1;
  }

  return 0;
}