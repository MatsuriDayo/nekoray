#pragma once

#include "ProxyEntity.hpp"

namespace NekoGui {
    class ProfileFilter {
    public:
        static void Uniq(
            const QList<std::shared_ptr<ProxyEntity>> &in,
            QList<std::shared_ptr<ProxyEntity>> &out,
            bool by_address = false, // def by bean
            bool keep_last = false   // def keep first
        );

        static void Common(
            const QList<std::shared_ptr<ProxyEntity>> &src,
            const QList<std::shared_ptr<ProxyEntity>> &dst,
            QList<std::shared_ptr<ProxyEntity>> &outSrc,
            QList<std::shared_ptr<ProxyEntity>> &outDst,
            bool by_address = false // def by bean
        );

        static void OnlyInSrc(
            const QList<std::shared_ptr<ProxyEntity>> &src,
            const QList<std::shared_ptr<ProxyEntity>> &dst,
            QList<std::shared_ptr<ProxyEntity>> &out,
            bool by_address = false // def by bean
        );

        static void OnlyInSrc_ByPointer(
            const QList<std::shared_ptr<ProxyEntity>> &src,
            const QList<std::shared_ptr<ProxyEntity>> &dst,
            QList<std::shared_ptr<ProxyEntity>> &out);
    };
} // namespace NekoGui
