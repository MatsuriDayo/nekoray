#include "ProfileFilter.hpp"

namespace NekoGui {

    QString ProfileFilter_ent_key(const std::shared_ptr<NekoGui::ProxyEntity> &ent, bool by_address) {
        by_address &= ent->type != "custom";
        return by_address ? (ent->bean->DisplayAddress() + ent->bean->DisplayType())
                          : QJsonObject2QString(ent->bean->ToJson({"c_cfg", "c_out"}), true) + ent->bean->DisplayType();
    }

    void ProfileFilter::Uniq(const QList<std::shared_ptr<ProxyEntity>> &in,
                             QList<std::shared_ptr<ProxyEntity>> &out,
                             bool by_address, bool keep_last) {
        QMap<QString, std::shared_ptr<ProxyEntity>> hashMap;

        for (const auto &ent: in) {
            QString key = ProfileFilter_ent_key(ent, by_address);
            if (hashMap.contains(key)) {
                if (keep_last) {
                    out.removeAll(hashMap[key]);
                    hashMap[key] = ent;
                    out += ent;
                }
            } else {
                hashMap[key] = ent;
                out += ent;
            }
        }
    }

    void ProfileFilter::Common(const QList<std::shared_ptr<ProxyEntity>> &src,
                               const QList<std::shared_ptr<ProxyEntity>> &dst,
                               QList<std::shared_ptr<ProxyEntity>> &outSrc,
                               QList<std::shared_ptr<ProxyEntity>> &outDst,
                               bool by_address) {
        QMap<QString, std::shared_ptr<ProxyEntity>> hashMap;

        for (const auto &ent: src) {
            QString key = ProfileFilter_ent_key(ent, by_address);
            hashMap[key] = ent;
        }
        for (const auto &ent: dst) {
            QString key = ProfileFilter_ent_key(ent, by_address);
            if (hashMap.contains(key)) {
                outDst += ent;
                outSrc += hashMap[key];
            }
        }
    }

    void ProfileFilter::OnlyInSrc(const QList<std::shared_ptr<ProxyEntity>> &src,
                                  const QList<std::shared_ptr<ProxyEntity>> &dst,
                                  QList<std::shared_ptr<ProxyEntity>> &out,
                                  bool by_address) {
        QMap<QString, bool> hashMap;

        for (const auto &ent: dst) {
            QString key = ProfileFilter_ent_key(ent, by_address);
            hashMap[key] = true;
        }
        for (const auto &ent: src) {
            QString key = ProfileFilter_ent_key(ent, by_address);
            if (!hashMap.contains(key)) out += ent;
        }
    }

    void ProfileFilter::OnlyInSrc_ByPointer(const QList<std::shared_ptr<ProxyEntity>> &src,
                                            const QList<std::shared_ptr<ProxyEntity>> &dst,
                                            QList<std::shared_ptr<ProxyEntity>> &out) {
        for (const auto &ent: src) {
            if (!dst.contains(ent)) out += ent;
        }
    }

} // namespace NekoGui