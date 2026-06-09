add_executable(receiver
    receiver_server.cc
    console_message_handler.cc
    grpc_receiver_service.cc
)

target_link_libraries(receiver
    PRIVATE
    proto_generated
    common_lib
)