#include "ProfileFilter.hpp"

namespace NekoGui {
    void ProfileFilter::Uniq(const QList<std::shared_ptr<ProxyEntity>> &in,
                             QList<std::shared_ptr<ProxyEntity>> &out,
                             bool by_address, bool keep_last) {
        QMap<QString, std::shared_ptr<ProxyEntity>> hashMap;

        for (const auto &ent: in) {
            QString key = by_address ? (ent->bean->DisplayAddress() + ent->bean->DisplayType())
                                     : ent->bean->ToJsonBytes();
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
                               QList<std::shared_ptr<ProxyEntity>> &out,
                               bool by_address, bool keep_last) {
        QMap<QString, std::shared_ptr<ProxyEntity>> hashMap;

        for (const auto &ent: src) {
            QString key = by_address ? (ent->bean->DisplayAddress() + ent->bean->DisplayType())
                                     : ent->bean->ToJsonBytes();
            hashMap[key] = ent;
        }
        for (const auto &ent: dst) {
            QString key = by_address ? (ent->bean->DisplayAddress() + ent->bean->DisplayType())
                                     : ent->bean->ToJsonBytes();
            if (hashMap.contains(key)) {
                if (keep_last) {
                    out += ent;
                } else {
                    out += hashMap[key];
                }
            }
        }
    }

    void ProfileFilter::OnlyInSrc(const QList<std::shared_ptr<ProxyEntity>> &src,
                                  const QList<std::shared_ptr<ProxyEntity>> &dst,
                                  QList<std::shared_ptr<ProxyEntity>> &out,
                                  bool by_address) {
        QMap<QString, bool> hashMap;

        for (const auto &ent: dst) {
            QString key = by_address ? (ent->bean->DisplayAddress() + ent->bean->DisplayType())
                                     : ent->bean->ToJsonBytes();
            hashMap[key] = true;
        }
        for (const auto &ent: src) {
            QString key = by_address ? (ent->bean->DisplayAddress() + ent->bean->DisplayType())
                                     : ent->bean->ToJsonBytes();
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