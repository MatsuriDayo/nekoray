#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QDropEvent>
#include <QDebug>
#include <utility>

class MyTableWidget : public QTableWidget {
public:
    explicit MyTableWidget(QWidget *parent = nullptr) : QTableWidget(parent) {
        // 拖拽设置
        this->setDragDropMode(QAbstractItemView::InternalMove);  // 内部移动
        this->setDropIndicatorShown(true);  // drop位置 提示
        this->setSelectionBehavior(QAbstractItemView::SelectRows);
    };

// takes and returns the whole row
    QList<QTableWidgetItem *> takeRow(int row) {
        QList<QTableWidgetItem *> rowItems;
        for (int col = 0; col < columnCount(); ++col) {
            rowItems << takeItem(row, col);
        }
        return rowItems;
    }

// sets the whole row
    void setRow(int row, const QList<QTableWidgetItem *> &rowItems) {
        for (int col = 0; col < columnCount(); ++col) {
            setItem(row, col, rowItems.at(col));
        }
    }

    QList<int> order; // id sorted
    std::function<void()> callback_save_order;
    std::map<int, int> id2Row;

    void _save_order(bool saveToFile) {
        order.clear();
        id2Row.clear();
        for (int i = 0; i < this->rowCount(); i++) {
            auto id = this->item(i, 0)->data(114514).toInt();
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
            auto id = this->item(i, 0)->data(114514).toInt();
            deleted_profiles.removeAll(id);
        }
        for (auto deleted_profile: deleted_profiles) {
            needSave = true;
            order.removeAll(deleted_profile);
        }

        QMap<int, QList<QTableWidgetItem *>> newRows;
        for (int i = 0; i < this->rowCount(); i++) {
            auto id = this->item(i, 0)->data(114514).toInt();
            auto dst = order.indexOf(id);
            if (dst == i) continue;
            if (dst == -1) {
                // 纠错: 新的profile不需要移动
                needSave = true;
                continue;
            }
            newRows[dst] = takeRow(i);
        }

        for (int i = 0; i < this->rowCount(); i++) {
            if (!newRows.contains(i)) continue;
            setRow(i, newRows[i]);
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
        // 原行号与目标行号的确定
        int row_src, row_dst;
        row_src = this->currentRow();// 原行号 可加if
        QTableWidgetItem *item = this->itemAt(event->pos());// 获取落点的item
        if (item != nullptr) {
            // 判断是否为空
            row_dst = item->row();// 不为空 获取其行号
            // 保证鼠标落下的位置 就是拖拽的一行最后所移动到的位置(考虑插入新行 移除原行的上下变化)
            row_src = (row_src > row_dst ? row_src + 1 : row_src);// 如果src在dst的下方(行号大)，后续插入dst会影响src的行号
            row_dst = (row_src < row_dst ? row_dst + 1 : row_dst);// 如果src在dst的上方(行号小)，后续移除src会影响dst的行号
            this->insertRow(row_dst);// 插入一行
        } else {
            // 落点没有item 说明拖动到了最下面
            row_dst = this->rowCount();// 获取行总数
            this->insertRow(row_dst);// 在最后新增一行
        }
        // 执行移动 并移除原行
        for (int i = 0; i < this->columnCount(); i++) {
            // 遍历列
            this->setItem(row_dst, i, this->takeItem(row_src, i));// 每一列item的移动
        }
        this->removeRow(row_src);// 删除原行

        // Then save the order
        _save_order(true);
    };
};
