#include <grpcpp/grpcpp.h>

#include <memory>

#include "grpc_sender.h"

#include "data_transfer.grpc.pb.h"

namespace datatransfer::client {

GrpcMessageSender::GrpcMessageSender(std::shared_ptr<grpc::Channel> channel)
    : stub_(DataTransfer::NewStub(channel)) {}

bool GrpcMessageSender::Send(const LogMessage &message) {

  TransferRequest request;
  request.set_source_service(message.source_service);
  request.set_timestamp_utc(message.timestamp_utc);
  request.set_payload(message.payload);

  TransferResponse response;
  grpc::ClientContext context;
  grpc::Status status = stub_->SendData(&context, request, &response);

  if (!status.ok()) {
    return false;
  }

  return response.success();
}

} // namespace datatransfer::client