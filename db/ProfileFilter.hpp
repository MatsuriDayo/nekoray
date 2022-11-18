#pragma once

#include "ProxyEntity.hpp"

namespace NekoRay {
    class ProfileFilter {
    public:
        static void Uniq(
            const QList<QSharedPointer<ProxyEntity>> &in,
            QList<QSharedPointer<ProxyEntity>> &out,
            bool by_address = false, // def by bean
            bool keep_last = false   // def keep first
        );

        static void Common(
            const QList<QSharedPointer<ProxyEntity>> &src,
            const QList<QSharedPointer<ProxyEntity>> &dst,
            QList<QSharedPointer<ProxyEntity>> &out,
            bool by_address = false, // def by bean
            bool keep_last = false   // def keep first
        );

        static void OnlyInSrc(
            const QList<QSharedPointer<ProxyEntity>> &src,
            const QList<QSharedPointer<ProxyEntity>> &dst,
            QList<QSharedPointer<NekoRay::ProxyEntity>> &out,
            bool by_address = false // def by bean
        );

        static void OnlyInSrc_ByPointer(
            const QList<QSharedPointer<ProxyEntity>> &src,
            const QList<QSharedPointer<ProxyEntity>> &dst,
            QList<QSharedPointer<ProxyEntity>> &out);
    };
} // namespace NekoRay
