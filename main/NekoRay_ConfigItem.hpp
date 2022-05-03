// DO NOT INCLUDE THIS

namespace NekoRay {
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
        QMap<QString, QSharedPointer<configItem>> _map;
        QList<std::function<void()>> _hooks_after_load;
        QList<std::function<void()>> _hooks_before_save;
        QString fn;
        bool debug_verbose = false;
        bool load_control_force = false;
        bool load_control_no_jsonStore = false; //不加载 json object
        bool save_control_compact = false;
        QByteArray last_save_content;

        JsonStore() = default;

        explicit JsonStore(QString fileName) {
            fn = std::move(fileName);
        }

        void _add(configItem *item);

        QSharedPointer<configItem> _get(const QString &name);

        QJsonObject ToJson();

        QByteArray ToJsonBytes();

        void FromJson(QJsonObject object);

        void FromJsonBytes(const QByteArray &data);

        bool Save();

        bool Load();
    };
}
