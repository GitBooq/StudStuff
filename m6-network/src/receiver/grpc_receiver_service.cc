//grpc_receiver_service.cc
#include <grpcpp/grpcpp.h>

#include "receiver/grpc_receiver_service.h"

namespace datatransfer {

grpc::Status
GrpcReceiverServiceImpl::SendData([[maybe_unused]] grpc::ServerContext *context,
                                  const datatransfer::TransferRequest *request,
                                  datatransfer::TransferResponse *response) {
  application::LogMessage message{request->source_service(),
                                  request->timestamp_utc(), request->payload()};
  handler_.Handle(message);

  response->set_success(true);
  response->set_message("Data received successfully");
  return grpc::Status::OK;
}

} // namespace datatransfer