#include <QByteArray>

namespace Qt515Base64 {
    enum Base64Option {
        Base64Encoding = 0,
        Base64UrlEncoding = 1,

        KeepTrailingEquals = 0,
        OmitTrailingEquals = 2,

        IgnoreBase64DecodingErrors = 0,
        AbortOnBase64DecodingErrors = 4,
    };
    Q_DECLARE_FLAGS(Base64Options, Base64Option)
    Q_DECLARE_OPERATORS_FOR_FLAGS(Base64Options)

    enum class Base64DecodingStatus {
        Ok,
        IllegalInputLength,
        IllegalCharacter,
        IllegalPadding,
    };

    class FromBase64Result {
    public:
        QByteArray decoded;
        Base64DecodingStatus decodingStatus;

        void swap(FromBase64Result &other) noexcept {
            qSwap(decoded, other.decoded);
            qSwap(decodingStatus, other.decodingStatus);
        }

        explicit operator bool() const noexcept { return decodingStatus == Base64DecodingStatus::Ok; }

#if defined(Q_COMPILER_REF_QUALIFIERS) && !defined(Q_QDOC)
        QByteArray &operator*() &noexcept { return decoded; }
        const QByteArray &operator*() const &noexcept { return decoded; }
        QByteArray &&operator*() &&noexcept { return std::move(decoded); }
#else
        QByteArray &operator*() noexcept { return decoded; }
        const QByteArray &operator*() const noexcept { return decoded; }
#endif
    };

    FromBase64Result QByteArray_fromBase64Encoding(const QByteArray &base64, Base64Options options);
} // namespace Qt515Base64
