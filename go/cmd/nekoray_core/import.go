package main

import (
	_ "github.com/v2fly/v2ray-core/v5/main/distro/all"

	_ "github.com/v2fly/v2ray-core/v5/proxy/vless/inbound"
	_ "github.com/v2fly/v2ray-core/v5/proxy/vless/outbound"
	_ "github.com/v2fly/v2ray-core/v5/proxy/vlite/inbound"
	_ "github.com/v2fly/v2ray-core/v5/proxy/vlite/outbound"

	_ "nekoray_core/v2ray_sip"
)
