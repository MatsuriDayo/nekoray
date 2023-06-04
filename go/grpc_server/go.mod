module grpc_server

go 1.19

require (
	github.com/grpc-ecosystem/go-grpc-middleware v1.3.0
	github.com/matsuridayo/libneko v1.0.0 // replaced
	google.golang.org/grpc v1.49.0
	google.golang.org/protobuf v1.28.1
)

require (
	github.com/golang/protobuf v1.5.2 // indirect
	github.com/google/go-cmp v0.5.8 // indirect
	golang.org/x/net v0.0.0-20220624214902-1bab6f366d9e // indirect
	golang.org/x/sys v0.0.0-20220520151302-bc2c85ada10a // indirect
	golang.org/x/text v0.3.7 // indirect
	google.golang.org/genproto v0.0.0-20211223182754-3ac035c7e7cb // indirect
)

replace github.com/matsuridayo/libneko v1.0.0 => ../../../libneko
