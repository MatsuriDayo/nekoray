#ifndef RUNGUARD_H
#define RUNGUARD_H

#include <QObject>
#include <QSharedMemory>
#include <QSystemSemaphore>
#include <QCryptographicHash>

class RunGuard {
public:
    RunGuard(const QString &key);

    ~RunGuard();

    bool isAnotherRunning(quint64 *data_out);

    bool tryToRun(quint64 *data_in);

    void release();

private:
    const QString key;
    const QString memLockKey;
    const QString sharedmemKey;

    QSharedMemory sharedMem;
    QSystemSemaphore memLock;

    Q_DISABLE_COPY(RunGuard)
};

namespace {

    QString generateKeyHash(const QString &key, const QString &salt) {
        QByteArray data;

        data.append(key.toUtf8());
        data.append(salt.toUtf8());
        data = QCryptographicHash::hash(data, QCryptographicHash::Sha1).toHex();

        return data;
    }

} // namespace

RunGuard::RunGuard(const QString &key)
    : key(key), memLockKey(generateKeyHash(key, "_memLockKey")), sharedmemKey(generateKeyHash(key, "_sharedmemKey")), sharedMem(sharedmemKey), memLock(memLockKey, 1) {
    memLock.acquire();
    {
        QSharedMemory fix(sharedmemKey); // Fix for *nix: http://habrahabr.ru/post/173281/
        fix.attach();
    }
    memLock.release();
}

RunGuard::~RunGuard() {
    release();
}

bool RunGuard::isAnotherRunning(quint64 *data_out) {
    if (sharedMem.isAttached())
        return false;

    memLock.acquire();
    const bool isRunning = sharedMem.attach();
    if (isRunning) {
        if (data_out != nullptr) {
            memcpy(data_out, sharedMem.data(), sizeof(quint64));
        }
        sharedMem.detach();
    }
    memLock.release();

    return isRunning;
}

bool RunGuard::tryToRun(quint64 *data_in) {
    memLock.acquire();
    const bool result = sharedMem.create(sizeof(quint64));
    if (result) memcpy(sharedMem.data(), data_in, sizeof(quint64));
    memLock.release();

    if (!result) {
        release();
        return false;
    }

    return true;
}

void RunGuard::release() {
    memLock.acquire();
    if (sharedMem.isAttached())
        sharedMem.detach();
    memLock.release();
}

#endif // RUNGUARD_H
