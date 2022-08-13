#pragma once

namespace Preset {
    namespace Hysteria {
        inline const char *command = "--no-check -c %config%";
        inline const char *config = "{\n"
                                    "  \"server\": \"127.0.0.1:%mapping_port%\",\n"
                                    "  \"obfs\": \"fuck me till the daylight\",\n"
                                    "  \"up_mbps\": 10,\n"
                                    "  \"down_mbps\": 50,\n"
                                    "  \"server_name\": \"real.name.com\",\n"
                                    "  \"socks5\": {\n"
                                    "    \"listen\": \"127.0.0.1:%socks_port%\"\n"
                                    "  }\n"
                                    "}";
    }
}
