package iphlpapi

import (
	"syscall"
	"unsafe"
)

func notifyRouteChange2(family uint16, callback uintptr, callerContext uintptr, initialNotification bool, notificationHandle *syscall.Handle) (ret error) {
	var _p0 uint32
	if initialNotification {
		_p0 = 1
	}
	r0, _, _ := syscall.Syscall6(proc_NotifyRouteChange2.Addr(), 5, uintptr(family), uintptr(callback), uintptr(callerContext), uintptr(_p0), uintptr(unsafe.Pointer(notificationHandle)), 0)
	if r0 != 0 {
		ret = syscall.Errno(r0)
	}
	return
}

func RegisterNotifyRouteChange2(callback func(callerContext uintptr, row uintptr, notificationType uint32) uintptr, initialNotification bool) (handle syscall.Handle) {
	notifyRouteChange2(syscall.AF_UNSPEC, syscall.NewCallback(callback), 0, initialNotification, &handle)
	return
}
