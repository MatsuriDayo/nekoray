#ifndef NEKORAY_GRPC_H
#define NEKORAY_GRPC_H

// MINGW gRPC build not ok now
#ifdef __MINGW32__
#define NO_GRPC
#else

#include "rpc/libcore.grpc.pb.h"
#include <grpc++/grpc++.h>
#include <QString>

namespace NekoRay::rpc {
    class Client {
    public:
        explicit Client(std::function<void(const QString &)> onError, const QString &target, const QString &token);

        void Exit();

        bool keepAlive();

        // QString returns is error string

        QString Start(bool *rpcOK, const QString &coreConfig);

        QString Stop(bool *rpcOK);

        libcore::TestResp Test(bool *rpcOK, const libcore::TestReq &request);

        long long QueryStats(bool *rpcOK, const std::string &tag, const std::string &direct);

    private:
        std::unique_ptr<libcore::LibcoreService::Stub> _stub;
        std::function<void(const QString &)> onError;
        QString token;
    };

    inline Client *defaultClient;
};
#endif

#endif //NEKORAY_GRPC_H
