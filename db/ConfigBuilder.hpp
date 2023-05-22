#pragma once

#include "ProxyEntity.hpp"
#include "sys/ExternalProcess.hpp"

namespace NekoGui {
    class BuildConfigResult {
    public:
        QString error;
        QJsonObject coreConfig;

        QList<std::shared_ptr<NekoGui_traffic::TrafficData>> outboundStats; // all, but not including "bypass" "block"
        std::shared_ptr<NekoGui_traffic::TrafficData> outboundStat;         // main
        QStringList ignoreConnTag;

        std::list<std::shared_ptr<NekoGui_fmt::ExternalBuildResult>> extRs;
    };

    class BuildConfigStatus {
    public:
        std::shared_ptr<BuildConfigResult> result;
        std::shared_ptr<ProxyEntity> ent;
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

    std::shared_ptr<BuildConfigResult> BuildConfig(const std::shared_ptr<ProxyEntity> &ent, bool forTest, bool forExport);

    void BuildConfigV2Ray(const std::shared_ptr<BuildConfigStatus> &status);

    void BuildConfigSingBox(const std::shared_ptr<BuildConfigStatus> &status);

    QString BuildChain(int chainId, const std::shared_ptr<BuildConfigStatus> &status);

    QString BuildChainInternal(int chainId, const QList<std::shared_ptr<ProxyEntity>> &ents,
                               const std::shared_ptr<BuildConfigStatus> &status);

    QString WriteVPNSingBoxConfig();

    QString WriteVPNLinuxScript(const QString &protectPath, const QString &configPath);
} // namespace NekoGui
