#pragma once

class QByteArray;
class QString;

namespace qjs {
    class QJS {
    public:
        QJS();
        explicit QJS(const QByteArray &jsSource);
        ~QJS();

        QString Eval(const QByteArray &jsSource) const;
        QString Eval(const QString &jsSource) const;
        QString EvalFile(const QString &jsPath) const;
        QString EvalFunction(const QString &funcName, const QString &arg) const;

    private:
        void *neko_ctx;
    };

    QByteArray ReadHookJS();
} // namespace qjs
