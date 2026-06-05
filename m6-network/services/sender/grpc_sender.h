#pragma once

#include <grpcpp/grpcpp.h>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <memory>
#include <sstream>

#include "data_transfer.grpc.pb.h"

namespace datatransfer::client {
class DataSender {
private:
  std::unique_ptr<datatransfer::DataTransfer::Stub> stub_;

public:
  DataSender(std::shared_ptr<grpc::Channel> channel)
      : stub_(datatransfer::DataTransfer::NewStub(channel)) {}

  void Send(const datatransfer::TransferRequest &msg);
};

} // namespace datatransfer::client
