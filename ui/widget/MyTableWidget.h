#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QDropEvent>
#include <QDebug>
#include <functional>
#include <utility>

class MyTableWidget : public QTableWidget {
public:
    explicit MyTableWidget(QWidget *parent = nullptr) : QTableWidget(parent) {
        // 拖拽设置
        this->setDragDropMode(QAbstractItemView::InternalMove); // 内部移动
        this->setDropIndicatorShown(true);                      // drop位置 提示
        this->setSelectionBehavior(QAbstractItemView::SelectRows);
    };

    QList<int> order;          // id sorted (save)
    std::map<int, int> id2Row; // id2Row
    QList<int> row2Id;         // row2Id: use this to refresh data

    std::function<void()> callback_save_order;
    std::function<void(int id)> refresh_data;

    void _save_order(bool saveToFile) {
        order.clear();
        id2Row.clear();
        for (int i = 0; i < this->rowCount(); i++) {
            auto id = row2Id[i];
            order += id;
            id2Row[id] = i;
        }
        if (callback_save_order != nullptr && saveToFile)
            callback_save_order();
    }

    void update_order(bool saveToFile) {
        if (order.isEmpty()) {
            _save_order(false);
            return;
        }

        // 纠错: order 里面含有不在当前表格控件的 id
        bool needSave = false;
        auto deleted_profiles = order;
        for (int i = 0; i < this->rowCount(); i++) {
            auto id = row2Id[i];
            deleted_profiles.removeAll(id);
        }
        for (auto deleted_profile: deleted_profiles) {
            needSave = true;
            order.removeAll(deleted_profile);
        }

        // map(dstRow -> srcId)
        QMap<int, int> newRows;
        for (int i = 0; i < this->rowCount(); i++) {
            auto id = row2Id[i];
            auto dst = order.indexOf(id);
            if (dst == i) continue;
            if (dst == -1) {
                // 纠错: 新的profile不需要移动
                needSave = true;
                continue;
            }
            newRows[dst] = id;
        }

        for (int i = 0; i < this->rowCount(); i++) {
            if (!newRows.contains(i)) continue;
            row2Id[i] = newRows[i];
        }

        // Then save the order
        _save_order(needSave || saveToFile);
    };

protected:
    /*
     * 2021.7.6 by gy
     * 拖拽 继承QTableWidget overwrite dropEvent事件
     * 功能：拖动一行到鼠标落下的位置
     * 注意：DragDropMode相关参数的设置
     */
    void dropEvent(QDropEvent *event) override {
        if (order.isEmpty()) order = row2Id;

        // 原行号与目标行号的确定
        int row_src, row_dst;
        row_src = this->currentRow();                        // 原行号 可加if
        auto id_src = row2Id[row_src];                       // id_src
        QTableWidgetItem *item = this->itemAt(event->pos()); // 获取落点的item
        if (item != nullptr) {
            // 判断是否为空
            row_dst = item->row(); // 不为空 获取其行号
            // Modify order
            order.removeAt(row_src);
            order.insert(row_dst, id_src);
        } else {
            // 落点没有item 说明拖动到了最下面
            return;
        }

        // Do update order & refresh
        clearSelection();
        update_order(true);
        refresh_data(-1);
    };
};
