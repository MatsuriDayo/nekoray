package main

import (
	"log"
	"strings"
	"unsafe"

	core "github.com/v2fly/v2ray-core/v5"
	"github.com/v2fly/v2ray-core/v5/features/stats"
	"github.com/v2fly/v2ray-core/v5/infra/conf/serial"
	"github.com/v2fly/v2ray-core/v5/nekoutils"
)

type NekoV2RayInstance struct {
	*core.Instance

	StatsManager stats.Manager
}

func NewNekoV2rayInstance(configStr string) (*NekoV2RayInstance, error) {
	// load v4 or v5 config
	var config *core.Config
	var err error
	if content2 := strings.TrimPrefix(configStr, "matsuri-v2ray-v5"); content2 != configStr {
		config, err = core.LoadConfig("jsonv5", strings.NewReader(content2))
		if err != nil {
			log.Println(configStr, err.Error())
			return nil, err
		}
	} else {
		config, err = serial.LoadJSONConfig(strings.NewReader(configStr))
		if err != nil {
			log.Println(configStr, err.Error())
			return nil, err
		}
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
	return uintptr(unsafe.Pointer(i.Instance))
}

func (i *NekoV2RayInstance) Close() error {
	nekoutils.ResetConnections(i.CorePtr())
	nekoutils.SetConnectionPoolV2RayEnabled(i.CorePtr(), false)
	return i.Instance.Close()
}
