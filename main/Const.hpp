#pragma once

namespace NekoRay {
    namespace DomainMatcher {
        enum DomainMatcher {
            DEFAULT,
            MPH,
        };
    }

    namespace IPv6Mode {
        enum IPv6Mode {
            DISABLE,
            ENABLE,
            PREFER,
            ONLY,
        };
    }

    namespace SniffingMode {
        enum SniffingMode {
            DISABLE,
            TO_DNS,
            TO_DESTINATION,
        };
    }
}

