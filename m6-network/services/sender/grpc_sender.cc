#include <grpcpp/grpcpp.h>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <memory>
#include <sstream>

#include "grpc_sender.h"

#include "data_transfer.grpc.pb.h"

namespace datatransfer::client {
void DataSender::Send(const datatransfer::TransferRequest &msg) {
  datatransfer::TransferResponse response;
  grpc::ClientContext context;
  grpc::Status status = stub_->SendData(&context, msg, &response);
  if (!status.ok()) {
    std::cerr << "gRPC send error [" << status.error_code() << "] "
              << status.error_message() << "\n";
  }
}
} // namespace datatransfer::client