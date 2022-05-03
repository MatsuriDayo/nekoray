#ifndef NEKORAY_GROUP
#define NEKORAY_GROUP

#include "NekoRay.hpp"
#include "ProxyEntity.hpp"

namespace NekoRay {

    // implement in mainwindow
    namespace GroupSortMethod {
        enum GroupSortMethod {
            Raw,
            ByType,
            ByAddress,
            ByName,
            ByLatency,
            ById,
        };
    }

    struct GroupSortAction {
        GroupSortMethod::GroupSortMethod method = GroupSortMethod::Raw;
        bool save_sort = false; //保存到文件
        bool descending = false; //默认升序，开这个就是降序
    };

    // Group Header

    class Group : public JsonStore {
    public:
        int id = -1;
        QString name = "";
        QList<int> order;
        QString url = "";

        Group() {
            _add(new configItem("id", &id, itemType::integer));
            _add(new configItem("name", &this->name, itemType::string));
            _add(new configItem("order", &order, itemType::integerList));
            _add(new configItem("url", &this->url, itemType::string));
        }

        [[nodiscard]] bool IsSubscription() const {
            return !url.isEmpty();
        }

        [[nodiscard]] QList<QSharedPointer<ProxyEntity>> Profiles() const;
    };
}

#endif