package main

import (
	"net"

	"github.com/sagernet/sing-box/adapter"
	"github.com/sagernet/sing-box/experimental/v2rayapi"
	"github.com/sagernet/sing-box/option"
	"github.com/sagernet/sing/common/network"
)

type sbV2rayServer struct {
	ss *sbV2rayStatsService
}

func NewSbV2rayServer() adapter.V2RayServer {
	options := option.V2RayStatsServiceOptions{
		Enabled:   true,
		Outbounds: []string{"proxy", "bypass"}, // TODO
	}
	return &sbV2rayServer{
		ss: &sbV2rayStatsService{v2rayapi.NewStatsService(options)},
	}
}

func (s *sbV2rayServer) Start() error                            { return nil }
func (s *sbV2rayServer) Close() error                            { return nil }
func (s *sbV2rayServer) StatsService() adapter.V2RayStatsService { return s.ss }

type sbV2rayStatsService struct {
	*v2rayapi.StatsService
}

func (s *sbV2rayStatsService) RoutedConnection(inbound string, outbound string, user string, conn net.Conn) net.Conn {
	// TODO track
	return s.StatsService.RoutedConnection(inbound, outbound, user, conn)
}

func (s *sbV2rayStatsService) RoutedPacketConnection(inbound string, outbound string, user string, conn network.PacketConn) network.PacketConn {
	return s.StatsService.RoutedPacketConnection(inbound, outbound, user, conn)
}
