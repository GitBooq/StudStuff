// log_message.h

#pragma once

#include <string>

namespace datatransfer::client {

struct LogMessage {
  std::string source_service;
  std::string timestamp_utc;
  std::string payload;
};

} // namespace datatransfer::client