// grpc_receiver_serivce.h
#pragma once

#include <grpcpp/grpcpp.h>

#include "message_handler.h"

#include "data_transfer.grpc.pb.h"

namespace datatransfer {

class GrpcReceiverServiceImpl : public DataTransfer::Service {
public:
  explicit GrpcReceiverServiceImpl(application::IMessageHandler &handler)
      : handler_(handler) {}

  grpc::Status SendData([[maybe_unused]] grpc::ServerContext *context,
                        const datatransfer::TransferRequest *request,
                        datatransfer::TransferResponse *response) override;

private:
  application::IMessageHandler &handler_;
};

} // namespace datatransfer
