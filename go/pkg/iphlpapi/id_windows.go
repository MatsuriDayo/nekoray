package iphlpapi

import "unsafe"

type GUID struct {
	Data1 [4]byte
	Data2 [2]byte
	Data3 [2]byte
	Data4 [8]byte
}

func Index2GUID(index uint64, guid *GUID) uint16 {
	var luid uint64
	a, _, _ := proc_ConvertInterfaceIndexToLuid.Call(uintptr(index), uintptr(unsafe.Pointer(&luid)))
	if a != 0 {
		return uint16(a)
	}
	a, _, _ = proc_ConvertInterfaceLuidToGuid.Call(uintptr(unsafe.Pointer(&luid)), uintptr(unsafe.Pointer(guid)))
	return uint16(a)
}
