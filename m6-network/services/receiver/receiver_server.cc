// receiver_server.cc

#include <grpcpp/grpcpp.h>

#include <iostream>
#include <memory>
#include <string>

#include "data_transfer.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

namespace datatransfer::server {

class DataTransferServiceImpl final
    : public datatransfer::DataTransfer::Service {
public:
  Status SendData([[maybe_unused]] ServerContext *context,
                  const datatransfer::TransferRequest *request,
                  datatransfer::TransferResponse *response) override {

    std::cout << "\n=== Received Data ===" << std::endl;
    std::cout << "Source: " << request->source_service() << std::endl;
    std::cout << "Timestamp: " << request->timestamp_utc() << std::endl;
    std::cout << "Payload: " << request->payload() << std::endl;
    std::cout << "===================\n" << std::endl;

    response->set_success(true);
    response->set_message("Data received successfully");
    return Status::OK;
  }
};
} // namespace datatransfer::server

int main() {
  using namespace datatransfer::server;
  
  std::string server_address("0.0.0.0:50051");
  DataTransferServiceImpl service;

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Receiver server listening on " << server_address << std::endl;

  server->Wait();
  return 0;
}