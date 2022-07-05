if (NKR_PROTOBUF_USE_CONFIG)
    find_package(Protobuf CONFIG REQUIRED)
else ()
    find_package(Protobuf REQUIRED)
endif ()
find_package(gRPC CONFIG REQUIRED)

#
# Protobuf/Grpc source files
#
set(PROTO_FILES
        go/gen/libcore.proto
        )

if (NKR_PROTOBUF_NO_AUTOGEN)
    # 手动生成 -> go/gen
    list(APPEND PROTO_FILES
            go/gen/libcore.pb.cc
            go/gen/libcore.grpc.pb.cc
            )
endif ()

#
# Add Library target with protobuf sources
#
add_library(myproto ${PROTO_FILES})
target_link_libraries(myproto
        PUBLIC
        gRPC::grpc++
        )
target_include_directories(myproto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

if (NOT NKR_PROTOBUF_NO_AUTOGEN)
    # 自动生成 -> CMAKE_CURRENT_BINARY_DIR
    get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
    protobuf_generate(TARGET myproto LANGUAGE cpp)
    protobuf_generate(TARGET myproto LANGUAGE grpc GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")
endif ()
