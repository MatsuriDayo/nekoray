#pragma once

#include "ProxyEntity.hpp"
#include "sys/ExternalProcess.hpp"

namespace NekoRay {
    class BuildConfigResult {
    public:
        QString error;
        QJsonObject coreConfig;

        QList<QSharedPointer<traffic::TrafficData>> outboundStats; // all, but not including "bypass" "block"
        QSharedPointer<traffic::TrafficData> outboundStat;         // main
        QStringList ignoreConnTag;

        std::list<std::pair<fmt::ExternalBuildResult, QSharedPointer<sys::ExternalProcess>>> exts; // extR to extC
    };

    class BuildConfigStatus {
    public:
        QSharedPointer<BuildConfigResult> result;
        QSharedPointer<ProxyEntity> ent;
        bool forTest;
        bool forExport;

        // priv
        QList<int> globalProfiles;

        // xxList is V2Ray format string list

        QStringList domainListDNSRemote;
        QStringList domainListDNSDirect;
        QStringList domainListRemote;
        QStringList domainListDirect;
        QStringList ipListRemote;
        QStringList ipListDirect;
        QStringList domainListBlock;
        QStringList ipListBlock;

        // config format

        QJsonArray routingRules;
        QJsonArray inbounds;
        QJsonArray outbounds;
    };

    QSharedPointer<BuildConfigResult> BuildConfig(const QSharedPointer<ProxyEntity> &ent, bool forTest, bool forExport);

    void BuildConfigV2Ray(const QSharedPointer<BuildConfigStatus> &status);

    void BuildConfigSingBox(const QSharedPointer<BuildConfigStatus> &status);

    QString BuildChain(int chainId, const QSharedPointer<BuildConfigStatus> &status);

    QString BuildChainInternal(int chainId, const QList<QSharedPointer<ProxyEntity>> &ents,
                               const QSharedPointer<BuildConfigStatus> &status);

    QString WriteVPNSingBoxConfig();

    QString WriteVPNLinuxScript(const QString &protectPath, const QString &configPath);
} // namespace NekoRay
