package grpc_server

import (
	"bufio"
	"context"
	"flag"
	"fmt"
	"log"
	"neko/gen"
	"neko/pkg/neko_auth"
	"neko/pkg/neko_common"
	"net"
	"os"
	"strconv"
	"strings"
	"time"

	grpc_auth "github.com/grpc-ecosystem/go-grpc-middleware/auth"
	"google.golang.org/grpc"
)

type BaseServer struct {
	gen.LibcoreServiceServer
}

var last time.Time

func (s *BaseServer) KeepAlive(ctx context.Context, in *gen.EmptyReq) (*gen.EmptyResp, error) {
	last = time.Now()
	return &gen.EmptyResp{}, nil
}

func (s *BaseServer) Exit(ctx context.Context, in *gen.EmptyReq) (out *gen.EmptyResp, _ error) {
	out = &gen.EmptyResp{}

	// Connection closed
	os.Exit(0)
	return
}

func RunCore(setupCore func(), server gen.LibcoreServiceServer) {
	_token := flag.String("token", "", "")
	_port := flag.Int("port", 19810, "")
	_debug := flag.Bool("debug", false, "")
	flag.CommandLine.Parse(os.Args[2:])

	neko_common.Debug = *_debug

	go func() {
		t := time.NewTicker(time.Second * 10)
		for {
			<-t.C
			if last.Add(time.Second * 10).Before(time.Now()) {
				fmt.Println("Exit due to inactive")
				os.Exit(0)
			}
		}
	}()

	// Libcore
	setupCore()

	// GRPC
	lis, err := net.Listen("tcp", "127.0.0.1:"+strconv.Itoa(*_port))
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}

	token := *_token
	if token == "" {
		os.Stderr.WriteString("Please set a token: ")
		s := bufio.NewScanner(os.Stdin)
		if s.Scan() {
			token = strings.TrimSpace(s.Text())
		}
	}
	if token == "" {
		fmt.Println("You must set a token")
		os.Exit(0)
	}
	os.Stderr.WriteString("token is set\n")

	auther := neko_auth.Authenticator{
		Token: token,
	}

	s := grpc.NewServer(
		grpc.StreamInterceptor(grpc_auth.StreamServerInterceptor(auther.Authenticate)),
		grpc.UnaryInterceptor(grpc_auth.UnaryServerInterceptor(auther.Authenticate)),
	)
	gen.RegisterLibcoreServiceServer(s, server)

	name := "nekoray_core"
	if neko_common.RunMode == neko_common.RunMode_NekoBox_Core {
		name = "nekobox_core"
	}

	log.Printf("%s grpc server listening at %v\n", name, lis.Addr())
	if err := s.Serve(lis); err != nil {
		log.Fatalf("failed to serve: %v", err)
	}
}
