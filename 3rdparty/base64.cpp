#include "base64.h"

#ifndef qsizetype
#define qsizetype size_t
#endif

namespace Qt515Base64 {
    namespace {
        struct fromBase64_helper_result {
            qsizetype decodedLength;
            Base64DecodingStatus status;
        };

        fromBase64_helper_result fromBase64_helper(const char *input, qsizetype inputSize,
                                                   char *output /* may alias input */,
                                                   Base64Options options) {
            fromBase64_helper_result result{0, Base64DecodingStatus::Ok};

            unsigned int buf = 0;
            int nbits = 0;

            qsizetype offset = 0;
            for (qsizetype i = 0; i < inputSize; ++i) {
                int ch = input[i];
                int d;

                if (ch >= 'A' && ch <= 'Z') {
                    d = ch - 'A';
                } else if (ch >= 'a' && ch <= 'z') {
                    d = ch - 'a' + 26;
                } else if (ch >= '0' && ch <= '9') {
                    d = ch - '0' + 52;
                } else if (ch == '+' && (options & Base64UrlEncoding) == 0) {
                    d = 62;
                } else if (ch == '-' && (options & Base64UrlEncoding) != 0) {
                    d = 62;
                } else if (ch == '/' && (options & Base64UrlEncoding) == 0) {
                    d = 63;
                } else if (ch == '_' && (options & Base64UrlEncoding) != 0) {
                    d = 63;
                } else {
                    if (options & AbortOnBase64DecodingErrors) {
                        if (ch == '=') {
                            // can have 1 or 2 '=' signs, in both cases padding base64Size to
                            // a multiple of 4. Any other case is illegal.
                            if ((inputSize % 4) != 0) {
                                result.status = Base64DecodingStatus::IllegalInputLength;
                                return result;
                            } else if ((i == inputSize - 1) ||
                                       (i == inputSize - 2 && input[++i] == '=')) {
                                d = -1; // ... and exit the loop, normally
                            } else {
                                result.status = Base64DecodingStatus::IllegalPadding;
                                return result;
                            }
                        } else {
                            result.status = Base64DecodingStatus::IllegalCharacter;
                            return result;
                        }
                    } else {
                        d = -1;
                    }
                }

                if (d != -1) {
                    buf = (buf << 6) | d;
                    nbits += 6;
                    if (nbits >= 8) {
                        nbits -= 8;
                        Q_ASSERT(offset < i);
                        output[offset++] = buf >> nbits;
                        buf &= (1 << nbits) - 1;
                    }
                }
            }

            result.decodedLength = offset;
            return result;
        }
    } // namespace

    FromBase64Result QByteArray_fromBase64Encoding(const QByteArray &base64, Base64Options options) {
        const auto base64Size = base64.size();
        QByteArray result((base64Size * 3) / 4, Qt::Uninitialized);
        const auto base64result = fromBase64_helper(base64.data(),
                                                    base64Size,
                                                    const_cast<char *>(result.constData()),
                                                    options);
        result.truncate(int(base64result.decodedLength));
        return {std::move(result), base64result.status};
    }
} // namespace Qt515Base64
