// DO NOT INCLUDE THIS

namespace NekoGui_ConfigItem {
    // config 工具
    enum itemType {
        string,
        integer,
        integer64,
        boolean,
        stringList,
        integerList,
        jsonStore,
    };

    class configItem {
    public:
        QString name;
        void *ptr;
        itemType type;

        configItem(QString n, void *p, itemType t) {
            name = std::move(n);
            ptr = p;
            type = t;
        }
    };

    // 可格式化对象
    class JsonStore {
    public:
        QMap<QString, std::shared_ptr<configItem>> _map;

        std::function<void()> callback_after_load = nullptr;
        std::function<void()> callback_before_save = nullptr;

        QString fn;
        bool load_control_must = false; // must load from file
        bool save_control_compact = false;
        bool save_control_no_save = false;
        QByteArray last_save_content;

        JsonStore() = default;

        explicit JsonStore(QString fileName) {
            fn = std::move(fileName);
        }

        void _add(configItem *item);

        QString _name(void *p);

        std::shared_ptr<configItem> _get(const QString &name);

        void _setValue(const QString &name, void *p);

        QJsonObject ToJson(const QStringList &without = {});

        QByteArray ToJsonBytes();

        void FromJson(QJsonObject object);

        void FromJsonBytes(const QByteArray &data);

        bool Save();

        bool Load();
    };
} // namespace NekoGui_ConfigItem

using namespace NekoGui_ConfigItem;
