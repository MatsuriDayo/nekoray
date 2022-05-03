protoc -I . --go_out=. --go_opt paths=source_relative --go-grpc_out=. --go-grpc_opt paths=source_relative libcore.proto

# protoc -I . --cpp_out=. --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` libcore.proto
