package iphlpapi

import (
	"fmt"
	"net"
	"unsafe"
)

/*
对于路由表，预期的方法是：
查询 0.0.0.0/0 获得原始默认路由
然后为 vpn 服务器添加默认路由
之后就根据需要下发vpn路由完事。
对于0.0.0.0/0 vpn 路由，可以尝试更低的跃点数，也可以尝试分为2个。
重新连接时可以删除vpn接口的所有非链路路由表。
路由表格式：
目标网络 uint32   掩码位数 byte低6位  vpn/默认网关  byte 高1位
*/

// 太低的值添加路由时会返回 106 错误
const routeMetric = 93

type RouteRow struct {
	ForwardDest      [4]byte //目标网络
	ForwardMask      [4]byte //掩码
	ForwardPolicy    uint32  //ForwardPolicy:0x0
	ForwardNextHop   [4]byte //网关
	ForwardIfIndex   uint32  // 网卡索引 id
	ForwardType      uint32  //3 本地接口  4 远端接口
	ForwardProto     uint32  //3静态路由 2本地接口 5EGP网关
	ForwardAge       uint32  //存在时间 秒
	ForwardNextHopAS uint32  //下一跳自治域号码 0
	ForwardMetric1   uint32  //度量衡(跃点数)，根据 ForwardProto 不同意义不同。
	ForwardMetric2   uint32
	ForwardMetric3   uint32
	ForwardMetric4   uint32
	ForwardMetric5   uint32
}

func (rr *RouteRow) GetForwardDest() net.IP {
	return net.IP(rr.ForwardDest[:])
}
func (rr *RouteRow) GetForwardMask() net.IP {
	return net.IP(rr.ForwardMask[:])
}
func (rr *RouteRow) GetForwardNextHop() net.IP {
	return net.IP(rr.ForwardNextHop[:])
}

func GetRoutes() ([]RouteRow, error) {
	buf := make([]byte, 4+unsafe.Sizeof(RouteRow{}))
	buf_len := uint32(len(buf))

	proc_GetIpForwardTable.Call(uintptr(unsafe.Pointer(&buf[0])),
		uintptr(unsafe.Pointer(&buf_len)), 0)

	var r1 uintptr
	for i := 0; i < 5; i++ {
		buf = make([]byte, buf_len)
		r1, _, _ = proc_GetIpForwardTable.Call(uintptr(unsafe.Pointer(&buf[0])),
			uintptr(unsafe.Pointer(&buf_len)), 0)
		if r1 == 122 {
			continue
		}
		break
	}

	if r1 != 0 {
		return nil, fmt.Errorf("Failed to get the routing table, return value：%v", r1)
	}

	num := *(*uint32)(unsafe.Pointer(&buf[0]))
	routes := make([]RouteRow, num)
	sr := uintptr(unsafe.Pointer(&buf[0])) + unsafe.Sizeof(num)
	rowSize := unsafe.Sizeof(RouteRow{})

	// 安全检查
	if len(buf) < int((unsafe.Sizeof(num) + rowSize*uintptr(num))) {
		return nil, fmt.Errorf("System error: GetIpForwardTable returns the number is too long, beyond the buffer。")
	}

	for i := uint32(0); i < num; i++ {
		routes[i] = *((*RouteRow)(unsafe.Pointer(sr + (rowSize * uintptr(i)))))
	}

	return routes, nil
}
