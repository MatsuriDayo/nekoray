#pragma once

#ifndef NKR_NO_GRPC

#include "go/grpc_server/gen/libcore.pb.h"
#include <QString>

namespace QtGrpc {
    class Http2GrpcChannelPrivate;
}

namespace NekoGui_rpc {
    class Client {
    public:
        explicit Client(std::function<void(const QString &)> onError, const QString &target, const QString &token);

        void Exit();

        bool KeepAlive();

        // QString returns is error string

        QString Start(bool *rpcOK, const libcore::LoadConfigReq &request);

        QString Stop(bool *rpcOK);

        long long QueryStats(const std::string &tag, const std::string &direct);

        std::string ListConnections();

        libcore::TestResp Test(bool *rpcOK, const libcore::TestReq &request);

        libcore::UpdateResp Update(bool *rpcOK, const libcore::UpdateReq &request);

    private:
        std::function<std::unique_ptr<QtGrpc::Http2GrpcChannelPrivate>()> make_grpc_channel;
        std::unique_ptr<QtGrpc::Http2GrpcChannelPrivate> default_grpc_channel;
        std::function<void(const QString &)> onError;
    };

    inline Client *defaultClient;
} // namespace NekoGui_rpc
#endif
