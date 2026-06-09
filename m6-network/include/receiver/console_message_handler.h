// console_message_handler.h
#pragma once

#include "message_handler.h"

namespace application {

class ConsoleMessageHandler : public IMessageHandler {
public:
  void Handle(const LogMessage &msg) override;
};

} // namespace application