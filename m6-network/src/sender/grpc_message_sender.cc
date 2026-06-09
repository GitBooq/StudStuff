// grpc_message_sender.cc
#include <grpcpp/grpcpp.h>

#include <memory>

#include "sender/grpc_message_sender.h"

#include "data_transfer.grpc.pb.h"

namespace datatransfer {

GrpcMessageSender::GrpcMessageSender(std::shared_ptr<grpc::Channel> channel, std::size_t timeoutMs)
    : stub_(DataTransfer::NewStub(channel)), timeoutMs_(timeoutMs) {}

bool GrpcMessageSender::Send(const application::LogMessage& message) {
  TransferRequest request;
  request.set_source_service(message.source_service);
  request.set_timestamp_utc(message.timestamp_utc);
  request.set_payload(message.payload);

  TransferResponse response;
  grpc::ClientContext context;
  context.set_deadline(std::chrono::system_clock::now() + std::chrono::milliseconds(timeoutMs_));
  grpc::Status status = stub_->SendData(&context, request, &response);

  HandleResponse(status, response);

  return (status.ok() && response.success());
}

void GrpcMessageSender::HandleResponse(grpc::Status status,
                                       const TransferResponse& response) const {
  if (!status.ok()) {
    std::cout << absl::StrFormat("gRPC error. Code=%d Message=%s",
                                 status.error_code(), status.error_message())
              << std::endl;
    return;
  }

  if (!response.success()) {
    std::cout << absl::StrFormat("Receiver rejected message: %s",
                                 response.message())
              << std::endl;
    return;
  }

  std::cout << absl::StrFormat("Ok. ResponseMessage=%s", response.message())
            << std::endl;
}

}  // namespace datatransfer