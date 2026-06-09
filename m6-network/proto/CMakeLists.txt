###########################################################################
# We have file: proto/data_transfer.proto
# From this file protoc should generate:
#   build/proto/generated/
#   data_transfer.pb.h
#   data_transfer.pb.cc
#   data_transfer.grpc.pb.h
#   data_transfer.grpc.pb.cc
# then we need to build lib proto_generated to use from services
###########################################################################

set(PROTO_FILE
    ${CMAKE_CURRENT_SOURCE_DIR}/data_transfer.proto
)

set(GENERATED_DIR
    ${CMAKE_CURRENT_BINARY_DIR}/generated
)

file(MAKE_DIRECTORY ${GENERATED_DIR})

# Find the protoc and gRPC executable installed on the host system
find_program(PROTOBUF_PROTOC protoc REQUIRED)
find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin REQUIRED)
message(STATUS "PROTOC=${PROTOBUF_PROTOC}")
message(STATUS "PLUGIN=${GRPC_CPP_PLUGIN}")

add_custom_command(
    OUTPUT
        ${GENERATED_DIR}/data_transfer.pb.cc
        ${GENERATED_DIR}/data_transfer.pb.h
        ${GENERATED_DIR}/data_transfer.grpc.pb.cc
        ${GENERATED_DIR}/data_transfer.grpc.pb.h

    COMMAND
        ${PROTOBUF_PROTOC}

    ARGS
        --grpc_out=${GENERATED_DIR}
        --cpp_out=${GENERATED_DIR}
        --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
        -I ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROTO_FILE}

    DEPENDS
        ${PROTO_FILE}
)

add_library(proto_generated
    ${GENERATED_DIR}/data_transfer.pb.cc
    ${GENERATED_DIR}/data_transfer.grpc.pb.cc
)

target_include_directories(proto_generated
    PUBLIC
    ${GENERATED_DIR}
)

target_link_libraries(proto_generated
    PUBLIC
    protobuf::libprotobuf
    gRPC::grpc++
)