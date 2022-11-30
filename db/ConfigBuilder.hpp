#pragma once

#include "ProxyEntity.hpp"
#include "sys/ExternalProcess.hpp"

namespace NekoRay {
    class BuildConfigResult {
    public:
        QString error;
        QJsonObject coreConfig;
        QStringList tryDomains;

        QList<QSharedPointer<traffic::TrafficData>> outboundStats; // all, but not including "bypass" "block"
        QSharedPointer<traffic::TrafficData> outboundStat;         // main
        QStringList ignoreConnTag;

        QList<sys::ExternalProcess *> ext;
    };

    class BuildConfigStatus {
    public:
        QList<int> globalProfiles;

        QSharedPointer<BuildConfigResult> result;
        QSharedPointer<ProxyEntity> ent;

        // xxList is V2Ray format string list

        QJsonArray domainListDNSRemote;
        QJsonArray domainListDNSDirect;
        QJsonArray domainListRemote;
        QJsonArray domainListDirect;
        QJsonArray ipListRemote;
        QJsonArray ipListDirect;
        QJsonArray domainListBlock;
        QJsonArray ipListBlock;

        // config format

        QJsonArray routingRules;
        QJsonArray inbounds;
        QJsonArray outbounds;
    };

    QSharedPointer<BuildConfigResult> BuildConfig(const QSharedPointer<ProxyEntity> &ent,
                                                  bool forTest, bool forExport);

    QSharedPointer<BuildConfigResult> BuildConfigV2Ray(const QSharedPointer<ProxyEntity> &ent,
                                                       bool forTest, bool forExport);

    QSharedPointer<BuildConfigResult> BuildConfigSingBox(const QSharedPointer<ProxyEntity> &ent,
                                                         bool forTest, bool forExport);

    QString BuildChain(int chainId, const QSharedPointer<BuildConfigStatus> &status);

    QString BuildChainInternal(int chainId, const QList<QSharedPointer<ProxyEntity>> &ents,
                               const QSharedPointer<BuildConfigStatus> &status);

    QString WriteVPNSingBoxConfig();

    QString WriteVPNLinuxScript(const QString &protectPath, const QString &configPath);
} // namespace NekoRay
