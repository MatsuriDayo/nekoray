#pragma once

#include "ProxyEntity.hpp"
#include "sys/ExternalProcess.hpp"

namespace NekoRay {
    class BuildConfigResult {
    public:
        QString error;
        QJsonObject coreConfig;
        QStringList tryDomains;

        QList<QSharedPointer<traffic::TrafficData>> outboundStats; // all, but not including direct
        QSharedPointer<traffic::TrafficData> outboundStat; // main

        QList<sys::ExternalProcess *> ext;
    };

    class BuildConfigStatus {
    public:
        QSharedPointer<BuildConfigResult> result;

        QJsonArray domainListDNSRemote;
        QJsonArray domainListDNSDirect;
        QJsonArray domainListRemote;
        QJsonArray domainListDirect;
        QJsonArray ipListRemote;
        QJsonArray ipListDirect;

        QJsonArray domainListBlock;
        QJsonArray ipListBlock;

        QJsonArray routingRules;
        QJsonObject hosts;

        QJsonArray inbounds;
        QJsonArray outbounds;

        QList<int> globalProfiles;
    };

    QSharedPointer<BuildConfigResult> BuildConfig(const QSharedPointer<ProxyEntity> &ent, bool forTest);

    QString BuildChain(int chainId, const QList<QSharedPointer<ProxyEntity>> &ents,
                       const QSharedPointer<BuildConfigStatus> &status);
}
