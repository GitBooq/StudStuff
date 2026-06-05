add_executable(sender
    sender_client.cc
    grpc_sender.cc
)

target_link_libraries(sender
    PRIVATE
    ipv4filter
    proto_generated
)

target_include_directories(sender
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)