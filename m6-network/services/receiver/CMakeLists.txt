add_executable(receiver
    receiver_server.cc
)

target_link_libraries(receiver
    PRIVATE
    proto_generated
)