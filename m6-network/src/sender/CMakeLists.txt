add_library(sender_lib STATIC
    grpc_message_sender.cc
)

target_link_libraries(sender_lib
    PUBLIC
        netFilterLogger
        proto_generated
)

add_executable(sender
    sender_client.cc
)

target_link_libraries(sender
    PRIVATE
        sender_lib
        common_lib
)

target_include_directories(sender
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
