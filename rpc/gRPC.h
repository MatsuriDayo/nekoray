#pragma once

#ifdef NKR_NO_EXTERNAL
#define NKR_NO_GRPC
#endif

#ifndef NKR_NO_GRPC

#include "go/gen/libcore.grpc.pb.h"
#include <grpc++/grpc++.h>
#include <QString>

namespace NekoRay::rpc {
    class Client {
    public:
        explicit Client(std::function<void(const QString &)> onError, const QString &target, const QString &token);

        libcore::UpdateResp Update(bool *rpcOK, const libcore::UpdateReq &request);

        void Exit();

        bool keepAlive();

        // QString returns is error string

        QString Start(bool *rpcOK, const QString &coreConfig, const QStringList &tryDomains);

        QString Stop(bool *rpcOK);

        libcore::TestResp Test(bool *rpcOK, const libcore::TestReq &request);

        long long QueryStats(const std::string &tag, const std::string &direct);

        std::string ListV2rayConnections();

    private:
        std::unique_ptr<libcore::LibcoreService::Stub> _stub;
        std::function<void(const QString &)> onError;
        QString token;
    };

    inline Client *defaultClient;
};
#endif

