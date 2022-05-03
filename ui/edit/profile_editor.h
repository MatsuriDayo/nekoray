#ifndef EDIT_ALL_H
#define EDIT_ALL_H

#include "db/ProxyEntity.hpp"

class ProfileEditor {
public:
    virtual void onStart(QSharedPointer<NekoRay::ProxyEntity> ent) = 0;

    virtual bool onEnd() = 0;
};

#endif