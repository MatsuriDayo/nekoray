#ifndef NEKORAY_CONST_HPP
#define NEKORAY_CONST_HPP

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

#endif //NEKORAY_CONST_HPP
