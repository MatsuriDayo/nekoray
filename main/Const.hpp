#pragma once

namespace NekoGui {
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

    namespace CoreType {
        enum CoreType {
            V2RAY,
            SING_BOX,
        };
    }
} // namespace NekoGui
