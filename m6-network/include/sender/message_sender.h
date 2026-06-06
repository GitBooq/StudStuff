// message_sender.h
#pragma once

#include "common/log_message.h"

namespace application {

class IMessageSender {
public:
  virtual ~IMessageSender() = default;

  virtual bool Send(const LogMessage &message) = 0;
};

} // namespace application