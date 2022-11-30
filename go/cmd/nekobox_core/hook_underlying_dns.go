package main

import (
	"context"
	"errors"
	"net"
	"reflect"
	"runtime"
	"unsafe"

	D "github.com/sagernet/sing-dns"
	"github.com/sagernet/sing/common/logger"
	M "github.com/sagernet/sing/common/metadata"
	N "github.com/sagernet/sing/common/network"
)

var underlyingDNS string

func init() {
	D.RegisterTransport([]string{"underlying"}, CreateUnderlyingTransport)
}

func CreateUnderlyingTransport(ctx context.Context, logger logger.ContextLogger, dialer N.Dialer, link string) (D.Transport, error) {
	if runtime.GOOS != "windows" {
		// Linux no resolv.conf change
		return D.CreateLocalTransport(ctx, logger, dialer, "local")
	}
	// Windows Underlying DNS hook
	t, _ := D.CreateUDPTransport(ctx, logger, dialer, link)
	udp := t.(*D.UDPTransport)
	handler_ := reflect.Indirect(reflect.ValueOf(udp)).FieldByName("handler")
	handler_ = reflect.NewAt(handler_.Type(), unsafe.Pointer(handler_.UnsafeAddr())).Elem()
	handler_.Set(reflect.ValueOf(&myTransportHandler{udp, dialer}))
	return t, nil
}

type myTransportHandler struct {
	*D.UDPTransport
	dialer N.Dialer
}

func (t *myTransportHandler) DialContext(ctx context.Context, queryCtx context.Context) (net.Conn, error) {
	if underlyingDNS == "" {
		return nil, errors.New("no underlyingDNS")
	}
	return t.dialer.DialContext(ctx, "udp", M.ParseSocksaddrHostPort(underlyingDNS, 53))
}
