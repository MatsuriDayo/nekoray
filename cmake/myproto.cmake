find_package(Protobuf CONFIG REQUIRED)

set(PROTO_FILES
        go/grpc_server/gen/libcore.proto
        )

add_library(myproto STATIC ${PROTO_FILES})
target_link_libraries(myproto
        PUBLIC
        protobuf::libprotobuf
        )
target_include_directories(myproto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

protobuf_generate(TARGET myproto LANGUAGE cpp)
