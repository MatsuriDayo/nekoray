#include "QJS.hpp"

#ifdef _MSC_VER
#define JS_STRICT_NAN_BOXING
#endif

#include "3rdparty/qjs/nekoray_qjs.h"
#include "main/NekoGui.hpp"

namespace qjs {
#ifndef NKR_NO_QUICKJS
    namespace exception {
        static void js_dump_obj(JSContext *ctx, QString &out, JSValueConst val) {
            const char *str;

            str = JS_ToCString(ctx, val);
            if (str) {
                out.append(str);
                out.append('\n');
                JS_FreeCString(ctx, str);
            } else {
                out += "[exception]\n";
            }
        }

        static void js_std_dump_error1(JSContext *ctx, QString &out, JSValueConst exception_val) {
            JSValue val;
            auto is_error = JS_IsError(ctx, exception_val);
            js_dump_obj(ctx, out, exception_val);
            if (is_error) {
                val = JS_GetPropertyStr(ctx, exception_val, "stack");
                if (!JS_IsUndefined(val)) {
                    js_dump_obj(ctx, out, val);
                }
                JS_FreeValue(ctx, val);
            }
        }

        QString js_std_dump_error(JSContext *ctx) {
            QString result;
            JSValue exception_val;

            exception_val = JS_GetException(ctx);
            js_std_dump_error1(ctx, result, exception_val);
            JS_FreeValue(ctx, exception_val);

            return result;
        }
    } // namespace exception

    JSValue func_log(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
        QString qString;

        int i;
        const char *str;
        size_t len;

        for (i = 0; i < argc; i++) {
            if (i != 0) qString.append(' ');
            str = JS_ToCStringLen(ctx, &len, argv[i]);
            if (!str)
                return JS_EXCEPTION;
            qString.append(str);
            JS_FreeCString(ctx, str);
        }

        MW_show_log(qString);
        qDebug() << "func_log:" << qString;

        return JS_UNDEFINED;
    }
#endif

#define NEKO_CTX ((nekoray_qjs_context *) this->neko_ctx)

    QJS::QJS() {
#ifndef NKR_NO_QUICKJS
        MW_show_log("loading quickjs......");
        //
        this->neko_ctx = malloc(sizeof(nekoray_qjs_context));
        nekoray_qjs_new_arg arg;
        arg.neko_ctx = NEKO_CTX;
        arg.enable_std = NekoGui::dataStore->enable_js_hook == 2 ? 1 : 0;
        arg.func_log = func_log;
        nekoray_qjs_new(arg);
#endif
    }

    QJS::QJS(const QByteArray &jsSource) : QJS() {
        this->Eval(jsSource);
    }

    QJS::~QJS() {
#ifndef NKR_NO_QUICKJS
        nekoray_qjs_free(NEKO_CTX);
        free(this->neko_ctx);
#endif
    }

    QString QJS::Eval(const QByteArray &jsSource) const {
#ifndef NKR_NO_QUICKJS
        auto result = nekoray_qjs_eval(NEKO_CTX, jsSource.data(), jsSource.length());
        if (JS_IsException(result)) {
            MW_show_log(exception::js_std_dump_error(NEKO_CTX->ctx));
            return {};
        }
        auto cString = JS_ToCString(NEKO_CTX->ctx, result);
        QString qString(cString);
        JS_FreeCString(NEKO_CTX->ctx, cString);
        JS_FreeValue(NEKO_CTX->ctx, result);
        return qString;
#else
        return {};
#endif
    }

    QString QJS::Eval(const QString &jsSource) const {
        return this->Eval(jsSource.toUtf8());
    }

    QString QJS::EvalFile(const QString &jsPath) const {
        return this->Eval(ReadFile(jsPath));
    }

    QString QJS::EvalFunction(const QString &funcName, const QString &arg) const {
#ifndef NKR_NO_QUICKJS
        auto ba1 = arg.toUtf8();
        JSValue globalObj = JS_GetGlobalObject(NEKO_CTX->ctx);
        JSValue tempObj = JS_NewStringLen(NEKO_CTX->ctx, ba1.data(), ba1.length());
        JS_SetPropertyStr(NEKO_CTX->ctx, globalObj, "tempObj", tempObj);
        auto result = this->Eval(QString("%1(tempObj)").arg(funcName));
        JS_DeleteProperty(NEKO_CTX->ctx, globalObj, JS_NewAtom(NEKO_CTX->ctx, "tempObj"), 1); // Free tempObj
        JS_FreeValue(NEKO_CTX->ctx, globalObj);
        return result;
#else
        return {};
#endif
    }

    QByteArray ReadHookJS() {
#ifndef NKR_NO_QUICKJS
        if (NekoGui::dataStore->enable_js_hook > 0) {
            return ReadFile(QString("./hook.%1.js").arg(software_name.toLower()));
        }
#endif
        return {};
    }
} // namespace qjs
