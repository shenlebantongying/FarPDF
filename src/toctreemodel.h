#ifndef TOCTREEMODEL_H
#define TOCTREEMODEL_H

#include <QAbstractItemModel>
#include <QQueue>
#include <mupdf/fitz/outline.h>

class toc_item {
    friend class tocTreeModel;

public:
    explicit toc_item(const QList<QVariant> & data, toc_item * parentItem = nullptr);

    ~toc_item();

    void appendChild(toc_item * item);

    toc_item * child(int row);

    int childCount() const;

    int columnCount() const;

    QVariant data(int column) const;

    int row() const;

    int page_number;

    toc_item * parentItem();

private:
    QList<toc_item *> m_childItems;
    QList<QVariant> m_itemData;
    toc_item * m_parent_item;
};

class tocTreeModel : public QAbstractItemModel {
    Q_OBJECT
public:
    explicit tocTreeModel(fz_outline * outline, QObject * parent = nullptr);
    ~tocTreeModel() override;

    QVariant data(const QModelIndex & index, int role) const override;

    Qt::ItemFlags flags(const QModelIndex & index) const override;

    QModelIndex index(int row, int column, const QModelIndex & parent) const override;

    QModelIndex parent(const QModelIndex & index) const override;

    int rowCount(const QModelIndex & parent) const override;

    int columnCount(const QModelIndex & parent) const override;

    static int page_num_from_index(const QModelIndex & index);

    void update_outline(fz_outline * outline);

    /**
     * @brief Add a toc title to user_toc_jumping_history which will later used to decide row color;
     * @param data <- via something like index.data()
     */
    void add_user_toc_jumping_history(const QModelIndex & data);


private:
    static void setupModelData(fz_outline * outline, toc_item * parent);

    QQueue<QModelIndex> user_toc_jumping_history;

    toc_item * rootItem;
};

#endif// TOCTREEMODEL_H
