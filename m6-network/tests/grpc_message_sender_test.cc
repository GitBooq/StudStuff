// grpc_message_sender_test.h
#include <grpcpp/grpcpp.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "sender/grpc_message_sender.h"
#include "common/log_message.h"

using testing::_;
using testing::Invoke;
using testing::Return;

class MockReceiver : public datatransfer::DataTransfer::Service {
 public:
  // mock method for
  //  grpc::Status SendData(grpc::ServerContext* context, const TransferRequest*
  //  request, TransferResponse* response) override;
  MOCK_METHOD(grpc::Status, SendData,
              (grpc::ServerContext * context,
               const datatransfer::TransferRequest* request,
               datatransfer::TransferResponse* response),
              (override));
};

class GrpcMessageSenderTest : public ::testing::Test {
 protected:
  using TransferRequest = datatransfer::TransferRequest;
  using TransferResponse = datatransfer::TransferResponse;

  void SetUp() override {
    mock_receiver_ = std::make_unique<MockReceiver>();

    grpc::ServerBuilder builder;
    builder.AddListeningPort("localhost:0", grpc::InsecureServerCredentials(),
                             &port_);
    builder.RegisterService(mock_receiver_.get());
    server_ = builder.BuildAndStart();
    ASSERT_NE(server_, nullptr);
    ASSERT_NE(port_, 0);

    auto channel = grpc::CreateChannel("localhost:" + std::to_string(port_),
                                       grpc::InsecureChannelCredentials());
    sender_ =
        std::make_unique<datatransfer::GrpcMessageSender>(channel, timeout_ms_);
  }

  void TearDown() override {
    sender_.reset();
    if (server_) {
      server_->Shutdown();
    }
    mock_receiver_.reset();
  }

  application::LogMessage CreateTestMessage() {
    application::LogMessage msg;
    msg.source_service = "test_service";
    msg.timestamp_utc = "2026-06-09T10:00:00Z";
    msg.payload = "test_payload";
    return msg;
  }

  std::unique_ptr<MockReceiver> mock_receiver_;
  std::unique_ptr<grpc::Server> server_;
  std::unique_ptr<datatransfer::GrpcMessageSender> sender_;
  int port_;
  std::size_t timeout_ms_ = 100;
};

TEST_F(GrpcMessageSenderTest, SendSuccess) {
  // Arrange
  EXPECT_CALL(*mock_receiver_, SendData(_, _, _))
      .WillOnce(
          Invoke([](grpc::ServerContext* /*ctx*/,
                    const TransferRequest* /*req*/, TransferResponse* resp) {
            resp->set_success(true);
            resp->set_message("OK");
            return grpc::Status::OK;
          }));

  // Act
  bool result = sender_->Send(CreateTestMessage());

  // Assert
  EXPECT_TRUE(result);
}

TEST_F(GrpcMessageSenderTest, SendGrpcErrorUnavailable) {
  EXPECT_CALL(*mock_receiver_, SendData(_, _, _))
      .WillOnce(Return(grpc::Status(grpc::StatusCode::UNAVAILABLE,
                                    "Service temporarily down")));

  bool result = sender_->Send(CreateTestMessage());

  EXPECT_FALSE(result);
}

TEST_F(GrpcMessageSenderTest, SendStatusOkButMsgReject) {
  EXPECT_CALL(*mock_receiver_, SendData(_, _, _))
      .WillOnce(
          Invoke([](grpc::ServerContext* /*ctx*/,
                    const TransferRequest* /*req*/, TransferResponse* resp) {
            resp->set_success(false);
            resp->set_message("Validation error: <something> invalid");
            return grpc::Status::OK;
          }));

  bool result = sender_->Send(CreateTestMessage());

  EXPECT_FALSE(result);
}

TEST_F(GrpcMessageSenderTest, SendTimeout) {
  auto wait_amount =
      std::chrono::milliseconds(timeout_ms_) + std::chrono::milliseconds(100);
  EXPECT_CALL(*mock_receiver_, SendData(_, _, _))
      .WillOnce(Invoke([wait_amount](grpc::ServerContext* context,
                                     const TransferRequest* /*req*/,
                                     TransferResponse* response) {
        // deadline is set
        EXPECT_NE(context->deadline(), std::chrono::system_clock::time_point());
        std::this_thread::sleep_for(wait_amount);
        response->set_success(true);
        return grpc::Status::OK;
      }));

  auto start = std::chrono::steady_clock::now();
  bool result = sender_->Send(CreateTestMessage());
  auto end = std::chrono::steady_clock::now();
  auto elapsed = end - start;

  EXPECT_FALSE(result);
  EXPECT_LT(elapsed, wait_amount);
}

TEST_F(GrpcMessageSenderTest, SendRequestHasCorrectData) {
  auto test_msg = CreateTestMessage();

  EXPECT_CALL(*mock_receiver_, SendData(_, _, _))
      .WillOnce(Invoke([&test_msg](grpc::ServerContext*,
                                   const TransferRequest* request,
                                   TransferResponse* response) {
        EXPECT_EQ(request->source_service(), test_msg.source_service);
        EXPECT_EQ(request->timestamp_utc(), test_msg.timestamp_utc);
        EXPECT_EQ(request->payload(), test_msg.payload);

        response->set_success(true);
        return grpc::Status::OK;
      }));

  sender_->Send(test_msg);
}