cmake_minimum_required(VERSION 3.5.1)

project(FibonacciNumbers C CXX)

#
# Compiler and linker flags.
#

if(NOT DEFINED CMAKE_CXX_STANDARD)
	set(CMAKE_CXX_STANDARD 14)
endif()

#
# Requirements.
#

find_package(Threads REQUIRED)

set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${Protobuf_VERSION}")
set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION gRPC::grpc++_reflection)
find_program(_PROTOBUF_PROTOC protoc)

find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")
set(_GRPC_GRPCPP gRPC::grpc++)

find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)

set(FIBO_LIBS ${_REFLECTION} ${_GRPC_GRPCPP} ${_PROTOBUF_LIBPROTOBUF})

#
# Protocol definition.
#

get_filename_component(fibo_proto "fibo.proto" ABSOLUTE)
get_filename_component(fibo_proto_path "${fibo_proto}" PATH)

set(fibo_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/fibo.pb.cc")
set(fibo_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/fibo.pb.h")
set(fibo_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/fibo.grpc.pb.cc")
set(fibo_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/fibo.grpc.pb.h")

add_custom_command(
	OUTPUT "${fibo_proto_srcs}" "${fibo_proto_hdrs}" "${fibo_grpc_srcs}"
	       "${fibo_grpc_hdrs}"
	COMMAND ${_PROTOBUF_PROTOC}
	ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
	     --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
	     -I "${fibo_proto_path}"
	     --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
	     "${fibo_proto}"
	DEPENDS "${fibo_proto}")

# Include generated *.pb.h files
include_directories(${CMAKE_CURRENT_BINARY_DIR})

#
# Libraries
#

# Protocol definition.
add_library(fibo_grpc_proto
	${fibo_grpc_srcs}
	${fibo_grpc_hdrs}
	${fibo_proto_srcs}
	${fibo_proto_hdrs})
target_link_libraries(fibo_grpc_proto ${FIBO_LIBS})

# Client.
add_library(fibo_client fibo_client.cc)
target_link_libraries(fibo_client fibo_grpc_proto ${FIBO_LIBS})

add_executable(client fibo_client_exe.cc)
target_link_libraries(client fibo_client)

# Server.
add_library(fibo_server fibo_server.cc)
target_link_libraries(fibo_server fibo_grpc_proto ${FIBO_LIBS})

add_executable(server fibo_server_exe.cc)
target_link_libraries(server fibo_server)
