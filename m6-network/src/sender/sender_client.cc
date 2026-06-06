// sender_client.cc
#include <net_logger/net_logger.h>

#include "sender/grpc_message_sender.h"

#include <chrono>
#include <format>
#include <fstream>
#include <iostream>

namespace {

std::string GetCurrentUTC() {
  using namespace std::chrono;
  auto now = floor<seconds>(utc_clock::now());

  return std::format("{:%FT%T}Z", now);
}

} // namespace

int main(int argc, char **argv) {
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

    application::LogMessage msg{"ipv4_filter", GetCurrentUTC(), payload_str};

    auto channel = grpc::CreateChannel("localhost:50051",
                                       grpc::InsecureChannelCredentials());
    datatransfer::GrpcMessageSender sender(channel);

    sender.Send(msg);
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Unknown exception caught" << std::endl;
    return 1;
  }

  return 0;
}