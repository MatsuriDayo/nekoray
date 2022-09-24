#pragma once

namespace NekoRay {
    namespace DomainMatcher {
        enum DomainMatcher {
            DEFAULT,
            MPH,
        };
    }

    namespace SniffingMode {
        enum SniffingMode {
            DISABLE,
            FOR_ROUTING,
            FOR_DESTINATION,
        };
    }

    namespace SystemProxyMode {
        enum SystemProxyMode {
            DISABLE,
            SYSTEM_PROXY,
            VPN,
        };
    }

    namespace CoreType {
        enum CoreType {
            V2RAY,
            SING_BOX,
        };
    }
}

