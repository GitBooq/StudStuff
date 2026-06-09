// grpc_message_sender.h

#pragma once

#include <grpcpp/grpcpp.h>

#include <memory>

#include "message_sender.h"

#include "data_transfer.grpc.pb.h"

namespace datatransfer {

class GrpcMessageSender : public application::IMessageSender {
public:
  explicit GrpcMessageSender(std::shared_ptr<grpc::Channel> channel, std::size_t timeoutMs = 3000);

  bool Send(const application::LogMessage &message) override;

private:
  void HandleResponse(grpc::Status status, const TransferResponse& response) const;

  std::unique_ptr<DataTransfer::Stub> stub_;
  std::size_t timeoutMs_;
};

} // namespace datatransfer
