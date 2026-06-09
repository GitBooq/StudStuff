// message_handler.h
#pragma once

#include "common/log_message.h"

namespace application {

class IMessageHandler {
public:
  virtual ~IMessageHandler() = default;

  virtual void Handle(const LogMessage &message) = 0;
};

} // namespace application