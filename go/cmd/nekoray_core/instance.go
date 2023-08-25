package main

import (
	"log"
	"strings"

	core "github.com/xtls/xray-core/core"
	"github.com/xtls/xray-core/features/stats"
	"github.com/xtls/xray-core/nekoutils"
)

type NekoV2RayInstance struct {
	*core.Instance

	StatsManager stats.Manager
}

func NewNekoV2rayInstance(configStr string) (*NekoV2RayInstance, error) {
	// load v4 or v5 config
	var config *core.Config
	var err error

	config, err = core.LoadConfig("json", strings.NewReader(configStr))
	if err != nil {
		log.Println(configStr, err.Error())
		return nil, err
	}

	c, err := core.New(config)
	if err != nil {
		return nil, err
	}

	instance := &NekoV2RayInstance{
		Instance: c,
	}
	instance.StatsManager = c.GetFeature(stats.ManagerType()).(stats.Manager)

	return instance, nil
}

func (i *NekoV2RayInstance) CorePtr() uintptr {
	return 0
}

func (i *NekoV2RayInstance) Close() error {
	nekoutils.ResetConnections(i.CorePtr())
	nekoutils.SetConnectionPoolV2RayEnabled(i.CorePtr(), false)
	return i.Instance.Close()
}
