// message_sender.h

#pragma once

#include "log_message.h"

namespace datatransfer::client {

class IMessageSender {
public:
  virtual ~IMessageSender() = default;

  virtual bool Send(const LogMessage &message) = 0;
};

} // namespace datatransfer::client