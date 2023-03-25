#pragma once

namespace Preset {
    namespace Hysteria {
        inline const char *command = "--no-check -c %config%";
        inline const char *config =
            "{\n"
            "  \"server\": \"127.0.0.1:%mapping_port%\",\n"
            "  \"server_name\": \"example.com\",\n"
            "  \"obfs\": \"fuck me till the daylight\",\n"
            "  \"up_mbps\": 10,\n"
            "  \"down_mbps\": 50,\n"
            "  \"socks5\": {\n"
            "    \"listen\": \"127.0.0.1:%socks_port%\"\n"
            "  }\n"
            "}";
    } // namespace Hysteria

    namespace SingBox {
        inline QStringList VpnImplementation = {"gvisor", "system"};
        inline QStringList DomainStrategy = {"", "ipv4_only", "ipv6_only", "prefer_ipv4", "prefer_ipv6"};
    } // namespace SingBox

    namespace Windows {
        inline QStringList system_proxy_format{"{ip}:{http_port}",
                                               "socks={ip}:{socks_port}",
                                               "http={ip}:{http_port};https={ip}:{http_port};ftp={ip}:{http_port};socks={ip}:{socks_port}",
                                               "http=http://{ip}:{http_port};https=http://{ip}:{http_port}"};
    } // namespace Windows
} // namespace Preset
