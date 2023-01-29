module nekoray_core

go 1.19

require (
	github.com/jsimonetti/rtnetlink v1.2.2
	github.com/sirupsen/logrus v1.8.1
	github.com/v2fly/v2ray-core/v5 v5.0.0
	kernel.org/pub/linux/libs/security/libcap/cap v1.2.65
	libcore v1.0.0
	neko v1.0.0
)

require (
	github.com/Dreamacro/clash v1.9.0 // indirect
	github.com/Dreamacro/go-shadowsocks2 v0.1.7 // indirect
	github.com/adrg/xdg v0.4.0 // indirect
	github.com/aead/chacha20 v0.0.0-20180709150244-8b13a72661da // indirect
	github.com/aead/cmac v0.0.0-20160719120800-7af84192f0b1 // indirect
	github.com/andybalholm/brotli v1.0.4 // indirect
	github.com/boljen/go-bitmap v0.0.0-20151001105940-23cd2fb0ce7d // indirect
	github.com/dgryski/go-camellia v0.0.0-20191119043421-69a8a13fb23d // indirect
	github.com/dgryski/go-metro v0.0.0-20211217172704-adc40b04c140 // indirect
	github.com/go-task/slim-sprig v0.0.0-20210107165309-348f09dbbbc0 // indirect
	github.com/golang/mock v1.6.0 // indirect
	github.com/golang/protobuf v1.5.2 // indirect
	github.com/google/btree v1.0.1 // indirect
	github.com/google/go-cmp v0.5.9 // indirect
	github.com/google/pprof v0.0.0-20210407192527-94a9f03dee38 // indirect
	github.com/gorilla/websocket v1.5.0 // indirect
	github.com/grpc-ecosystem/go-grpc-middleware v1.3.0 // indirect
	github.com/jhump/protoreflect v1.14.0 // indirect
	github.com/josharian/native v1.0.0 // indirect
	github.com/klauspost/compress v1.15.12 // indirect
	github.com/klauspost/cpuid v1.2.3 // indirect
	github.com/klauspost/reedsolomon v1.9.3 // indirect
	github.com/lucas-clemente/quic-go v0.31.1 // indirect
	github.com/lunixbochs/struc v0.0.0-20200707160740-784aaebc1d40 // indirect
	github.com/marten-seemann/qtls-go1-18 v0.1.3 // indirect
	github.com/marten-seemann/qtls-go1-19 v0.1.1 // indirect
	github.com/mdlayher/netlink v1.6.0 // indirect
	github.com/mdlayher/socket v0.1.1 // indirect
	github.com/miekg/dns v1.1.50 // indirect
	github.com/mustafaturan/bus v1.0.2 // indirect
	github.com/mustafaturan/monoton v1.0.0 // indirect
	github.com/onsi/ginkgo/v2 v2.2.0 // indirect
	github.com/patrickmn/go-cache v2.1.0+incompatible // indirect
	github.com/pion/dtls/v2 v2.0.0-rc.7 // indirect
	github.com/pion/logging v0.2.2 // indirect
	github.com/pion/sctp v1.7.6 // indirect
	github.com/pires/go-proxyproto v0.6.2 // indirect
	github.com/pkg/errors v0.9.1 // indirect
	github.com/refraction-networking/utls v1.2.0 // indirect
	github.com/riobard/go-bloom v0.0.0-20200614022211-cdc8013cb5b3 // indirect
	github.com/sagernet/libping v0.1.1 // indirect
	github.com/secure-io/siv-go v0.0.0-20180922214919-5ff40651e2c4 // indirect
	github.com/seiflotfy/cuckoofilter v0.0.0-20220411075957-e3b120b3f5fb // indirect
	github.com/ulikunitz/xz v0.5.10 // indirect
	github.com/v2fly/BrowserBridge v0.0.0-20210430233438-0570fc1d7d08 // indirect
	github.com/v2fly/ss-bloomring v0.0.0-20210312155135-28617310f63e // indirect
	github.com/xiaokangwang/VLite v0.0.0-20220418190619-cff95160a432 // indirect
	github.com/xtaci/smux v1.5.16 // indirect
	go.starlark.net v0.0.0-20220817180228-f738f5508c12 // indirect
	go.uber.org/automaxprocs v1.4.0 // indirect
	go4.org/netipx v0.0.0-20220925034521-797b0c90d8ab // indirect
	golang.org/x/crypto v0.4.0 // indirect
	golang.org/x/exp v0.0.0-20220722155223-a9213eeb770e // indirect
	golang.org/x/mobile v0.0.0-20220722155234-aaac322e2105 // indirect
	golang.org/x/mod v0.6.0-dev.0.20220419223038-86c51ed26bb4 // indirect
	golang.org/x/net v0.4.0 // indirect
	golang.org/x/sync v0.0.0-20220907140024-f12130a52804 // indirect
	golang.org/x/sys v0.3.0 // indirect
	golang.org/x/text v0.5.0 // indirect
	golang.org/x/time v0.0.0-20211116232009-f0f3c7e86c11 // indirect
	golang.org/x/tools v0.1.12 // indirect
	golang.org/x/xerrors v0.0.0-20200804184101-5ec99f83aff1 // indirect
	google.golang.org/genproto v0.0.0-20211223182754-3ac035c7e7cb // indirect
	google.golang.org/grpc v1.51.0 // indirect
	google.golang.org/protobuf v1.28.1 // indirect
	gvisor.dev/gvisor v0.0.0 // indirect
	kernel.org/pub/linux/libs/security/libcap/psx v1.2.65 // indirect
)

replace neko v1.0.0 => ../../

replace libcore v1.0.0 => ../../../../Matsuri/libcore

replace github.com/v2fly/v2ray-core/v5 v5.0.0 => ../../../../v2ray-core

replace gvisor.dev/gvisor => github.com/sagernet/gvisor v0.0.0-20220402114650-763d12dc953e
