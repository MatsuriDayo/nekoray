#pragma once

#ifdef NKR_NO_EXTERNAL
#define NKR_NO_GRPC
#endif

#ifndef NKR_NO_GRPC

#include "go/gen/libcore.pb.h"
#include <QString>

namespace QtGrpc {
    class Http2GrpcChannelPrivate;
}

namespace NekoRay::rpc {
    class Client {
    public:
        explicit Client(std::function<void(const QString &)> onError, const QString &target, const QString &token);

        void Exit();

        bool KeepAlive();

        // QString returns is error string

        QString Start(bool *rpcOK, const QString &coreConfig, const QStringList &tryDomains);

        QString Stop(bool *rpcOK);

        long long QueryStats(const std::string &tag, const std::string &direct);

        std::string ListV2rayConnections();

        libcore::TestResp Test(bool *rpcOK, const libcore::TestReq &request);

        libcore::UpdateResp Update(bool *rpcOK, const libcore::UpdateReq &request);

    private:
        std::unique_ptr<QtGrpc::Http2GrpcChannelPrivate> grpc_channel;
        std::function<void(const QString &)> onError;
    };

    inline Client *defaultClient;
}
#endif

