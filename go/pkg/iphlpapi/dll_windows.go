package iphlpapi

import "syscall"

var (
	proc_getIpForwardTable  *syscall.LazyProc
	proc_notifyRouteChange2 *syscall.LazyProc
)

func init() {
	iphlpapi := syscall.NewLazyDLL("iphlpapi.dll")
	proc_getIpForwardTable = iphlpapi.NewProc("GetIpForwardTable")
	proc_notifyRouteChange2 = iphlpapi.NewProc("NotifyRouteChange2")
}
