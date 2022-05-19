find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(Threads)

#
# Protobuf/Grpc source files
#
set(PROTO_FILES
        go/gen/libcore.proto
        )

#
# Add Library target with protobuf sources
#
add_library(myproto ${PROTO_FILES})
target_link_libraries(myproto
        PUBLIC
        gRPC::grpc++
        )
target_include_directories(myproto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

#
# Compile protobuf and grpc files in myproto target to cpp
#
get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
protobuf_generate(TARGET myproto LANGUAGE cpp)
protobuf_generate(TARGET myproto LANGUAGE grpc GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")

get_target_property(SOURCES myproto SOURCES)
set_property(SOURCE ${SOURCES} PROPERTY SKIP_AUTOGEN ON)
