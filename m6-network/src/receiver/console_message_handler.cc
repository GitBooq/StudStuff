// console_message_handler.cc
#include "receiver/console_message_handler.h"

#include <format>
#include <iostream>

namespace application {

void ConsoleMessageHandler::Handle(const LogMessage &message) {
  std::cout << std::format("\n=== Received Data ===\n"
                           "Source: {}\n"
                           "Timestamp: {}\n"
                           "Payload: {}\n"
                           "=====================",
                           message.source_service, message.timestamp_utc,
                           message.payload)
            << std::endl;
}

} // namespace application