#include "V2RayStreamSettings.hpp"
#include "ShadowSocksBean.hpp"
#include "VMessBean.hpp"
#include "TrojanVLESSBean.hpp"
#include "SocksHttpBean.hpp"

namespace NekoRay::fmt {
    QString AbstractBean::DisplayInsecureHint() {
        if (!dataStore->insecure_hint) return {};
        auto insecure_hint = InsecureHint();
        auto stream = GetStreamSettings(this);
        if (stream != nullptr) insecure_hint += "\n" + stream->InsecureHint();
        return insecure_hint.trimmed();
    }

    QString V2rayStreamSettings::InsecureHint() const {
        if (allow_insecure) {
            return QObject::tr(
                    "The configuration (insecure) can be detected and identified, the transmission is fully visible to the censor and is not resistant to man-in-the-middle tampering with the content of the communication."
            );
        }
        return {};
    }

    QString ShadowSocksBean::InsecureHint() {
        if (method.contains("-poly") || method.contains("-gcm")) {
            return {};
        }
        return QObject::tr(
                "This configuration (Shadowsocks streaming cipher) can be accurately proactively detected and decrypted by censors without requiring a password, and cannot be mitigated by turning on IV replay filters on the server side.\n"
                "\n"
                "Learn more: https://github.com/net4people/bbs/issues/24"
        );
    }

    QString VMessBean::InsecureHint() {
        if (security == "none" || security == "zero") {
            if (stream->security.isEmpty()) {
                return QObject::tr(
                        "This profile is cleartext, don't use it if the server is not in your local network.");
            }
        }
        if (aid > 0) {
            return QObject::tr(
                    "This configuration (VMess MD5 authentication) has been deprecated by upstream because of its questionable resistance to tampering and concealment.\n"
                    "\n"
                    "As of January 1, 2022, compatibility with MD5 authentication information will be disabled on the server side by default. Any client using MD5 authentication information will not be able to connect to a server with VMess MD5 authentication information disabled."
            );
        }
        return {};
    }

    QString TrojanVLESSBean::InsecureHint() {
        if (stream->security.isEmpty()) {
            return QObject::tr("This profile is cleartext, don't use it if the server is not in your local network.");
        }
        return {};
    }

    QString SocksHttpBean::InsecureHint() {
        if (stream->security.isEmpty()) {
            return QObject::tr("This profile is cleartext, don't use it if the server is not in your local network.");
        }
        return {};
    }
}

