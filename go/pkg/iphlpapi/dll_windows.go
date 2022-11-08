package iphlpapi

import "syscall"

var (
	proc_GetIpForwardTable           *syscall.LazyProc
	proc_NotifyRouteChange2          *syscall.LazyProc
	proc_ConvertInterfaceIndexToLuid *syscall.LazyProc
	proc_ConvertInterfaceLuidToGuid  *syscall.LazyProc
)

func init() {
	iphlpapi := syscall.NewLazyDLL("iphlpapi.dll")
	proc_GetIpForwardTable = iphlpapi.NewProc("GetIpForwardTable")
	proc_NotifyRouteChange2 = iphlpapi.NewProc("NotifyRouteChange2")
	proc_ConvertInterfaceIndexToLuid = iphlpapi.NewProc("ConvertInterfaceIndexToLuid")
	proc_ConvertInterfaceLuidToGuid = iphlpapi.NewProc("ConvertInterfaceLuidToGuid")
}
