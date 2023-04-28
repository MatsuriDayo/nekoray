#pragma once

namespace Preset {
    namespace V2Ray {
        inline QStringList UtlsFingerPrint = {"", "randomized", "randomizedalpn", "randomizednoalpn", "firefox_auto", "firefox_55", "firefox_56", "firefox_63", "firefox_65", "firefox_99", "firefox_102", "firefox_105", "chrome_auto", "chrome_58", "chrome_62", "chrome_70", "chrome_72", "chrome_83", "chrome_87", "chrome_96", "chrome_100", "chrome_102", "ios_auto", "ios_11_1", "ios_12_1", "ios_13", "ios_14", "android_11_okhttp", "edge_auto", "edge_85", "edge_106", "safari_auto", "safari_16_0", "360_auto", "360_7_5", "360_11_0", "qq_auto", "qq_11_1"};
    } // namespace V2Ray

    namespace SingBox {
        inline QStringList VpnImplementation = {"gvisor", "system"};
        inline QStringList DomainStrategy = {"", "ipv4_only", "ipv6_only", "prefer_ipv4", "prefer_ipv6"};
        inline QStringList UtlsFingerPrint = {"", "chrome", "firefox", "edge", "safari", "360", "qq", "ios", "android", "random", "randomized"};
    } // namespace SingBox

    namespace Windows {
        inline QStringList system_proxy_format{"{ip}:{http_port}",
                                               "socks={ip}:{socks_port}",
                                               "http={ip}:{http_port};https={ip}:{http_port};ftp={ip}:{http_port};socks={ip}:{socks_port}",
                                               "http=http://{ip}:{http_port};https=http://{ip}:{http_port}"};
    } // namespace Windows
} // namespace Preset
