// grpc_sender.h

#pragma once

#include <grpcpp/grpcpp.h>

#include <memory>

#include "message_sender.h"

#include "data_transfer.grpc.pb.h"

namespace datatransfer::client {

class GrpcMessageSender : public IMessageSender {
public:
  explicit GrpcMessageSender(std::shared_ptr<grpc::Channel> channel);

  bool Send(const LogMessage &message) override;

private:
  std::unique_ptr<DataTransfer::Stub> stub_;
};

} // namespace datatransfer::client
