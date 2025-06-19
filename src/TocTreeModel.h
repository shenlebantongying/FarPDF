#pragma once

#include <QAbstractItemModel>
#include <QQueue>
#include <mupdf/fitz/outline.h>

class TocItem {
    friend class TocTreeModel;

public:
    explicit TocItem(const QList<QVariant>& data, TocItem* parentItem = nullptr);

    ~TocItem();

    void appendChild(TocItem* item);

    TocItem* child(int row);

    qsizetype childCount() const;

    qsizetype columnCount() const;

    QVariant data(int column) const;

    int row() const;

    int page_number = 0;

    TocItem* parentItem();

private:
    QList<TocItem*> m_childItems;
    QList<QVariant> m_itemData;
    TocItem* m_parent_item;
};

class TocTreeModel : public QAbstractItemModel {
    Q_OBJECT
public:
    explicit TocTreeModel(fz_outline* outline, QObject* parent = nullptr);
    ~TocTreeModel() override;

    QVariant data(const QModelIndex& index, int role) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;

    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent) const override;

    int columnCount(const QModelIndex& parent) const override;

    static int page_num_from_index(const QModelIndex& index);

    void update_outline(fz_outline* outline);

    /**
     * @brief Add a toc title to user_toc_jumping_history which will later used to decide row color;
     * @param data <- via something like index.data()
     */
    void add_user_toc_jumping_history(const QModelIndex& data);

private:
    static void setupModelData(fz_outline* outline, TocItem* parent);

    QQueue<QModelIndex> user_toc_jumping_history;

    TocItem* rootItem;
};
