#include "gRPC.h"

#include <utility>

#ifndef NO_GRPC

namespace NekoRay::rpc {
    Client::Client(std::function<void(const QString &)> onError, const QString &target, const QString &token) {
        auto channel = grpc::CreateChannel(target.toStdString(), grpc::InsecureChannelCredentials());
        this->_stub = libcore::LibcoreService::NewStub(channel);
        this->onError = std::move(onError);
        this->token = token;
    }

#define MAKE_CONTEXT auto context = grpc::ClientContext(); context.AddMetadata("nekoray_auth", token.toStdString());

    // TODO 太多复制代码

    void Client::Exit() {
        MAKE_CONTEXT
        libcore::EmptyReq request;
        libcore::ErrorResp reply;
        grpc::Status status = _stub->Stop(&context, request, &reply);
    }

    QString Client::Start(bool *rpcOK, const QString &coreConfig) {
        {
            MAKE_CONTEXT
            libcore::LoadConfigReq request;
            request.set_coreconfig(coreConfig.toStdString());

            libcore::ErrorResp reply;
            grpc::Status status = _stub->Start(&context, request, &reply);

            if (status.ok()) {
                *rpcOK = true;
                return {reply.error().c_str()};
            } else {
                *rpcOK = false;
                onError(
                        QString("error code: %1, error message: %2\n").arg(status.error_code()).arg(
                                status.error_message().c_str())
                );
                return "";
            }
        }
    }

    QString Client::Stop(bool *rpcOK) {
        {
            MAKE_CONTEXT
            libcore::EmptyReq request;

            libcore::ErrorResp reply;
            grpc::Status status = _stub->Stop(&context, request, &reply);

            if (status.ok()) {
                *rpcOK = true;
                return {reply.error().c_str()};
            } else {
                *rpcOK = false;
                onError(
                        QString("error code: %1, error message: %2\n").arg(status.error_code()).arg(
                                status.error_message().c_str())
                );
                return "";
            }
        }
    }

    bool Client::keepAlive() {
        MAKE_CONTEXT
        libcore::EmptyReq request;

        libcore::EmptyResp reply;
        grpc::Status status = _stub->KeepAlive(&context, request, &reply);

        if (status.ok()) {
            return true;
        }
        return false;
    }

    libcore::TestResp Client::Test(bool *rpcOK, const libcore::TestReq &request) {
        MAKE_CONTEXT
        libcore::TestResp reply;
        grpc::Status status = _stub->Test(&context, request, &reply);

        if (!status.ok()) {
            *rpcOK = false;
            onError(
                    QString("error code: %1, error message: %2\n").arg(status.error_code()).arg(
                            status.error_message().c_str())
            );
            return reply;
        }

        *rpcOK = true;
        return reply;
    }

    long long Client::QueryStats(bool *rpcOK, const std::string &tag, const std::string &direct) {
        MAKE_CONTEXT
        libcore::QueryStatsReq request;
        request.set_tag(tag);
        request.set_direct(direct);

        libcore::QueryStatsResp reply;
        grpc::Status status = _stub->QueryStats(&context, request, &reply);

        if (!status.ok()) {
            *rpcOK = false;
            onError(
                    QString("error code: %1, error message: %2\n").arg(status.error_code()).arg(
                            status.error_message().c_str())
            );
            return 0;
        }

        *rpcOK = true;
        return reply.traffic();
    }
}

#endif
