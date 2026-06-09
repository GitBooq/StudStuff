// receiver_server.cc

#include <grpcpp/grpcpp.h>

#include <iostream>
#include <memory>
#include <string>

#include "receiver/console_message_handler.h"
#include "receiver/grpc_receiver_service.h"
#include "common/helpers.h"

int main() {
  using namespace datatransfer;
  auto port = helpers::GetServerPortOr50051();
  std::string server_address("0.0.0.0:" + port);

  application::ConsoleMessageHandler handler;
  GrpcReceiverServiceImpl service(handler);

  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Receiver server listening on " << server_address << std::endl;

  server->Wait();
  return 0;
}